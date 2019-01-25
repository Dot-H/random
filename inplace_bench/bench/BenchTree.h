#pragma once

#include <functional>
#include <memory>

#include "utils/varvector.h"

namespace bench {

template <class T>
class Tree {

struct InternalNode; 
struct Node;

template <class U>
friend class DumpVisitor;

public:
    using ProcessFunc = std::function<void(const T&)>;

    Tree() : _root(new InternalNode()) { _tail = _root; }
    ~Tree() { delete _root; }

    Tree(Tree&&) = default;
    Tree& operator=(Tree&&) = default;
    Tree(const Tree&) = default;
    Tree& operator=(const Tree&) = default;

    // -----------------
    // Modifiers methods
    // -----------------
    template <typename... Args> // Supports both emplace and move constructor
    T& addInternal(Args&&... args) {
        _tail->children.template emplace_back<InternalNode>(_tail, std::forward<Args>(args)...);
        _tail = &(_tail->children.template back<InternalNode>());

        ++_size;
        return _tail->value;
    }

    template <typename... Args> // Supports both emplace and move constructor
    T& addLeaf(Args&&... args) {
        _tail->children.template emplace_back<Node>(_tail, std::forward<Args>(args)...);

        ++_size;
        return _tail->children.template back<Node>().value;
    }

    void goUp() { assert(_tail->father); _tail = _tail->father; }

    // ---------
    // Accessors
    // ---------
    AUInt size() const { return _size; } 
    bool isRoot() const { return _tail->father == nullptr; }
    InternalNode* getTail() const { return _tail; }

    // -------
    // Process
    // -------
    template <typename Underlying>
    struct Visitor : crtp<Underlying, Visitor> {
        void visit(Node& n) { this->underlying().visit(n); }
        void visit(const Node& n) { this->underlying().visit(n); }
        void visit(InternalNode& n) { this->underlying().visit(n); }
        void visit(const InternalNode& n) { this->underlying().visit(n); }

    private:
        // This is a compile time security to prevent a class which is not
        // `crtpType` to use this crtp instanciation
        Visitor(){}
        friend Underlying;
    };

    template <typename V>
    void accept(V&& v) const { 
        _root->children.foreach([&v](auto&& child) {
            child.accept(std::forward<V>(v));
        });
    }

private:
    struct Node {
        Node(InternalNode* father_, T&& infos)
            : father(father_), value(std::forward<T>(infos)) {}

        template <typename... Args>
        Node(InternalNode* father_, Args&&... args) // Emplace constructor
            : father(father_), value{std::forward<Args>(args)...} {}

        template <typename V>
        void accept(V&& v) const {  v.visit(*this); }

        InternalNode* father;
        T value;
    };

    struct InternalNode {
        template <typename... Args>
        InternalNode(InternalNode* father_, Args&&... args) // Emplace constructor
            : father(father_), value{std::forward<Args>(args)...} {}

        InternalNode() = default; // Root constructor

        template <typename V>
        void accept(V&& v) const {  v.visit(*this); }

        InternalNode* father = nullptr;
        T value;
        stable_varvector<Node, InternalNode> children;
    };

    /**
     * @Note By definition, a node can only be added to the tail's children.
     *       Therefore, the children of the tail's father cannot be resized and
     *       it is safe to store the tail as a pointer.
     */
    InternalNode* _root; // Sentinel
    InternalNode* _tail;
    AUInt _size;
};

// -------------
// Tree visitors
// -------------
template <class Infos>
struct DumpVisitor : public Tree<Infos>::template Visitor<DumpVisitor<Infos>>  {
    using Node = typename Tree<Infos>::Node;
    using InternalNode = typename Tree<Infos>::InternalNode;
    
    DumpVisitor(JSONWriter& writer_)
        : writer(writer_) {}

    void visit(const Node& n) {
        writer.pushMapStart();
        writer.pushMapKeyConst("name");
        writer.pushString(n.value.name);
        for (const auto& comment : n.value.comments) {
            writer.pushMapKeyRaw(comment.first);
            writer.pushString(comment.second);
        }
        writer.pushMapKeyConst("elapsed");
        writer.pushString(prettyPrint(n.value.elapsed.count())); 

        writer.pushMapEnd();
    }

    void visit(const InternalNode& n) {
        writer.pushMapStart();
        writer.pushMapKeyConst("name");
        writer.pushString(n.value.name);
        for (const auto& comment : n.value.comments) {
            writer.pushMapKeyRaw(comment.first);
            writer.pushString(comment.second);
        }
        writer.pushMapKeyRaw("elapsed");
        writer.pushString(prettyPrint(n.value.elapsed.count()));

        writer.pushMapKeyConst("sub");
        writer.pushArrayStart();
        n.children.foreach([&](auto&& child) {
            child.accept(*this);
        });
        writer.pushArrayEnd(); // benches
        writer.pushMapEnd(); // current sub `name`
    }

    JSONWriter& writer;
};

} // namespace bench
