#include <iostream>
#include <array>
#include <utility>

struct Uncopiable {
    Uncopiable(int val_ = 0)
        : val(val_) {};
    Uncopiable(const Uncopiable&) =delete;
    Uncopiable& operator=(const Uncopiable&) =delete;
    Uncopiable(Uncopiable&&) =delete;
    Uncopiable& operator=(Uncopiable&&) =delete;

    int val;
};


template <typename... Args>
constexpr std::array<Uncopiable, sizeof...(Args)> fill(Args&&... args) {
    return { args... };   
}

template <class Tuple, std::size_t... I, std::size_t... J>
constexpr auto fill_with_impl(Tuple&& t, std::index_sequence<I...>, std::index_sequence<J...>) {
    return fill(std::get<I - J>(std::forward<Tuple>(t))...);
}

template <int N,
          typename Indices = std::make_index_sequence<N>>
constexpr auto fill_with(int val) {
    auto tuple = std::make_tuple(val);
    return fill_with_impl(tuple, Indices{}, Indices{});
}

template <int N>
struct S {
    template <class Tuple, std::size_t... I, std::size_t... J>
    constexpr Ringbuffer(Tuple&& t, std::index_sequence<I...>, std::index_sequence<J...>)
        : _buffer{std::get<I - J>(std::forward<Tuple>(t))...} {}

            template <typename Indices = std::make_index_sequence<N>>
                : Ringbuffer(std::make_tuple(val), Indices{}, Indices{}) {}
    S(int val)
        : S{fill_with<N>(val)} {}

    std::array<Uncopiable, N> array;
};

int main(int argc, char* argv[])
{
    auto s = S<10>(3);
    a = { { 3}, { 3 }, { 3 } }
}


