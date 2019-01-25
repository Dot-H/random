#include <iostream>

struct Verbose {
    Verbose(const Verbose&) { std::cout << "Verbose copy\n"; }

    Verbose(Verbose&&) { std::cout << "Verbose move\n"; }

    Verbose() { std::cout << "Verbose create\n"; }

    ~Verbose() { std::cout << "Verbose delete\n"; }
};

struct Toto {
    Toto(const Verbose& a_)
        : a(a_) {}

    const Verbose& a;
};

template <class T, typename... Args>
T* create_fwd(Args&&... args) { return new T(std::forward<Args...>(args...)); }

template <class T, typename... Args>
T* create(Args... args) { return new T(std::forward<Args...>(args...)); }

int main()
{
    Verbose a;
//    Toto* t = create<Toto>(a); // Useless copy made when passing arguments to create
    Toto* t = create_fwd<Toto>(a); // No copy made

    delete t;
    return 0;
}

