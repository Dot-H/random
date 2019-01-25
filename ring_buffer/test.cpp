#include <iostream>
#include "Ringbuffer.h"

#define N 5

struct Verbose {
    Verbose() {}

    Verbose(int i)
        : id(i) {
            std::cout << "Verbose " << id << " create\n"; 
        }

    Verbose(const Verbose&) { std::cout << "Verbose " << id << " copy\n"; }
    Verbose& operator=(const Verbose&) { std::cout << "Verbose " << id << " copy assignment\n";  return *this;}

    Verbose(Verbose&&) { std::cout << "Verbose " << id << " move\n"; }
    Verbose& operator=(Verbose&&) { std::cout << "Verbose " << id << " move assignment\n"; return *this;}

    ~Verbose() { std::cout << "Verbose " << id << " delete\n"; }

    void hello() { std::cout << "Hello" << std::endl; }

    int id;
};

int main(void) {
    Ringbuffer<Verbose, N> rb(1);
    for (int i = 0; i < N; ++i) {
        Verbose v(i);
        rb.tryAdd(std::move(v));
    }

    for (int i = 0; i < N + 1; ++i) {
        std::cout << '\n';
    }

    return 0;
}