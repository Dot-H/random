#pragma once

#include "utils/bench/BenchUtils.h"
#include "utils/bench/BenchTree.h"
#include "utils/bench/BenchInfos.h"

#include <stdio.h>

#include <functional>
#include <unordered_map>
#include <thread>
#include <sstream>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace bench {
   
template <typename Infos>
class BenchEnv;

template <typename Infos>
static BenchEnv<Infos>& getEnvInstance();

// -------------------
// Environment classes
// -------------------
#if ALGOLIA_PROFILING == 1
template <typename Infos>
class BenchEnv {
    friend BenchEnv<Infos>& getEnvInstance<Infos>();
public:

    BenchEnv(BenchEnv&) = delete;
    BenchEnv(BenchEnv&&) = delete;
    BenchEnv& operator=(const BenchEnv&) = delete;
    BenchEnv& operator=(BenchEnv&&) = delete;

    template <typename Iterator>
    void eraseMapCell(const Iterator& it) {
        _benchesPerThread.erase(it);
    }

    /**
     * Dump the current content of the local tree containing the benches of
     * the thread represented by `tid` into `_out`. 
     */
    auto dumpMapCell(const std::thread::id& tid) {
        const auto threadBench_it = _benchesPerThread.find(tid);
        if (threadBench_it != end(_benchesPerThread)) {
            std::lock_guard<std::mutex> lock(_outMutex);
            threadBench_it->second->dump(_writer);
        }

        return threadBench_it;
    }

    /**
     * Dump the content of all the threads into `_out` and flush it using `func`.
     * Using a functor aims to make the way of registering the bench user side.
     * 
     * @Note after this function, the `_out` buffer is cleared.
     */
    template <typename Functor>
    void flushBench(Functor& func) {
        std::lock_guard<std::mutex> lock(_outMutex);

        for (auto &threadBench : _benchesPerThread) {
            threadBench.second->dump(_writer);
        }

        func(_out);
        _out.resize(0);
        _writer.reset();
    }

    struct BenchLocalEnv {
        BenchLocalEnv(const BenchLocalEnv&) = delete;
        BenchLocalEnv(BenchLocalEnv&&) = delete;
        BenchLocalEnv& operator=(const BenchLocalEnv&) = delete;
        BenchLocalEnv& operator=(BenchLocalEnv&&) = delete;

        ~BenchLocalEnv() {
            auto& env = getEnvInstance<Infos>();
            const auto& it = env.dumpMapCell(std::this_thread::get_id());
            env.eraseMapCell(it);
        }

        static BenchLocalEnv& getInstance() {
            static thread_local BenchLocalEnv env;
            return env;
        }

        void dump(JSONWriter& writer) const {
            writer.pushMapStart();

            writer.pushMapKey(threadIdToStr(std::this_thread::get_id()));
            writer.pushArrayStart();

            DumpVisitor<Infos> v{ writer };
            benchTree.accept(v);

            writer.pushArrayEnd();

            writer.pushRaw("}", 1, false);
        }

        Tree<Infos> benchTree;

    private:
        BenchLocalEnv() = default;
    };

    /**
     * Iterates over all the internal nodes of the local tree in order to increase
     * their elapsed value by the argument `elapsed`.
     * This function is called when adding a leaf.
     */
    template <class Duration>
    void increaseParentBench(BenchLocalEnv& localEnv, Duration elapsed) {
        for (auto* tail = localEnv.benchTree.getTail(); tail->father;
             tail = tail->father)
        {
            tail->value.elapsed += elapsed;
        }
    }

    /**
     * Create a leaf with the attributes from `infos` as value.
     * If the local tree was not already registered in the global environment, add it.
     * 
     * @return the value inside the newly created node.
     */
    Infos& addLeaf(Infos&& infos) {
        auto& localBenches = BenchLocalEnv::getInstance();
        increaseParentBench(localBenches, infos.elapsed);
        auto& ret = localBenches.benchTree.addLeaf(std::forward<Infos>(infos));
        _benchesPerThread.insert({ std::this_thread::get_id(), &localBenches });

        return ret;
    }

    /**
     * Emplace a leaf with the attributes given in argument.
     * If the local tree was not already registered in the global environment, add it.
     * 
     * @return the value inside the newly created node.
     */
    template <class Duration, typename... Args>
    Infos& emplaceLeaf(const Duration& elapsed, Args&&... args) {
        auto& localBenches = BenchLocalEnv::getInstance();
        increaseParentBench(localBenches, elapsed);
        auto& ret = localBenches.benchTree.addLeaf(elapsed, std::forward<Args>(args)...);
        _benchesPerThread.insert({ std::this_thread::get_id(), &localBenches });

        return ret;
    }

    /**
     * Add an internal node the attributes from `infos` as value.
     * If the local tree was not already registered in the global environment, add it.
     * 
     * @return the value inside the newly created node.
     */
    Infos& addSubLevel(Infos&& infos) {
        auto& localBenches = BenchLocalEnv::getInstance();
        increaseParentBench(localBenches, infos.elapsed);
        auto& ret = localBenches.benchTree.addInternal(std::forward<Infos>(infos));
        _benchesPerThread.insert({ std::this_thread::get_id(), &localBenches });

        return ret;
    }

    /**
     * Emplace an internal node with the attributes given in argument.
     * If the local tree was not already registered in the global environment, add it.
     * 
     * @return the value inside the newly created node.
     */
    template <class Duration, typename... Args>
    Infos& emplaceSubLevel(const Duration& elapsed, Args&&... args) {
        auto& localBenches = BenchLocalEnv::getInstance();
        increaseParentBench(localBenches, elapsed);
        auto& ret = localBenches.benchTree.addInternal(elapsed, std::forward<Args>(args)...);
        _benchesPerThread.insert({ std::this_thread::get_id(), &localBenches });

        return ret;
    }

    /**
     * Go up in the current local tree. Meaning that the scope of the last
     * internal node created is done.
     * 
     * @Note Typically called by the destructor of `BenchBunch`.
     */
    void endSubLevel() {
        auto& localBenches = BenchLocalEnv::getInstance();
        localBenches.benchTree.goUp();
    }

private:
    BenchEnv() 
        : _writer(_out) { }

    JSONWriter _writer;
    OutBuff_t _out;

    std::unordered_map<std::thread::id, const BenchLocalEnv* > _benchesPerThread;
    std::mutex _outMutex;
};
#else
template <typename Infos>
class BenchEnv {
    friend BenchEnv<Infos>& getEnvInstance<Infos>();
public:

    BenchEnv(BenchEnv&) = delete;
    BenchEnv(BenchEnv&&) = delete;
    BenchEnv& operator=(const BenchEnv&) = delete;
    BenchEnv& operator=(BenchEnv&&) = delete;

    template <typename Iterator>
    void eraseMapCell(const Iterator&) {}
    auto dumpMapCell(const std::thread::id&) {}

    template <typename Functor>
    void flushBench(Functor& func) {}

    struct BenchLocalEnv {
        BenchLocalEnv(const BenchLocalEnv&) = delete;
        BenchLocalEnv(BenchLocalEnv&&) = delete;
        BenchLocalEnv& operator=(const BenchLocalEnv&) = delete;
        BenchLocalEnv& operator=(BenchLocalEnv&&) = delete;

        BenchLocalEnv() = default;

        static BenchLocalEnv& getInstance() {
            static thread_local BenchLocalEnv env;
            return env;
        }

        void dump(JSONWriter& writer) const {}
    };

    template <class Duration>
    void increaseParentBench(BenchLocalEnv&, Duration) {}

    void addLeaf(Infos&&) {}

    template <class Duration, typename... Args>
    void emplaceLeaf(const Duration&, Args&&...) {}

    void addSubLevel(Infos&&) {}

    template <class Duration, typename... Args>
    void emplaceSubLevel(const Duration&, Args&&...) {}

    void endSubLevel() {}

private:
    BenchEnv() = default;
};
#endif

/**
 * @Note from https://en.cppreference.com/w/cpp/language/storage_duration:
 * if multiple threads attempt to initialize the same static local variable
 * concurrently, the initialization occurs exactly once.
 */
template <typename Infos = BenchInfos>
static BenchEnv<Infos>& getEnvInstance() {
    static BenchEnv<Infos> env;
    return env;
}


} // Namespace bench
