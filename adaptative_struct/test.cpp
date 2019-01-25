#include <iostream>

template <std::size_t N>
constexpr uint8_t nBytes() {
    return (sizeof(N) + 1) - x86_lzcnt(N);
}

template <std::size_t N>
struct Size {
    constexpr std::size_t size() const { nBytes<N>(); }
};

int main(int argc, char* argv[])
{
    Size<128> s;
    std::cout << s.size() << std::endl;

    return 0;
}
