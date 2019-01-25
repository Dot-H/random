#include <iostream>
#include <tuple>
#include <vector>

namespace details {

template <int N, typename Srch, typename Fst, typename... Ts>
constexpr int find_impl() {
    if constexpr (std::is_same_v<Srch, Fst>) {
        return N;
    } else if constexpr (sizeof...(Ts) == 0) {
        return -1;
    } else {
        return find_impl<N+1, Srch, Ts...>();
    }
}

template <typename Srch, typename... Ts>
constexpr int find_T() {
    return find_impl<0, Srch, Ts...>();
}

template <std::size_t N, typename Tuple, typename Func>
constexpr void foreachcont_impl(Tuple tpl, Func fn) {
    if constexpr (N < std::tuple_size_v<Tuple>) {
        const auto& container = std::get<N>(tpl);
        for (auto&& el : container) {
            fn(el);
        }
        foreachcont_impl<N+1>(tpl, fn);
    }
}

template <std::size_t N, typename Tuple, typename Func>
constexpr void foreach_impl(Tuple tpl, Func fn) {
    if constexpr (N < std::tuple_size_v<Tuple>) {
        fn(std::get<N>(tpl));
        foreach_impl<N+1>(tpl, fn);
    }
}

} // namespace details

template <typename... Ts>
struct varvector {
    template <typename T>
    constexpr void push_back(T&& value) {
        std::get<details::find_T<T, Ts...>()>(vecs).push_back(std::forward<T>(value));
    }

    template <typename Func>
    constexpr void foreach(Func fn) {
        details::foreachcont_impl<0>(vecs, fn);
    }

    constexpr std::size_t size() const {
        std::size_t n = 0;
        details::foreach_impl<0>(vecs, [&n](const auto& v) {
            n += v.size();
        });

        return n;
    }

    std::tuple<std::vector<Ts>...> vecs;
};

int main(int argc, char* argv[])
{
    varvector<int, char, bool> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    v.push_back(true);
    v.push_back(false);
    v.push_back('c');
    v.push_back('a');
    v.push_back(42);

    std::cout << "vector<int> size: " << sizeof(std::vector<int>) << std::endl;
    std::cout << "vector<char> size: " << sizeof(std::vector<char>) << std::endl;
    std::cout << "vector<bool> size: " << sizeof(std::vector<bool>) << std::endl;
    std::cout << "varvector size: " << sizeof(varvector<int, char, bool>) << std::endl;

    std::cout << "Nb elements: " << v.size() << std::endl;
    v.foreach([](const auto& el) {
        std::cout << el << '\n';
    });

    return 0;
}
