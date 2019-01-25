#include <iostream>

namespace detail {

template <std::size_t S, std::size_t... I>
constexpr std::index_sequence<S + I...> incrSeq(std::index_sequence<I...>) {
    return {};
}

template <typename Tuple, std::size_t... I>
void printTupleImpl(const Tuple& tuple, std::index_sequence<I...>) {
    ((std::cout << std::get<I>(tuple) << ' '), ...);
    std::cout << std::endl;
}

template <typename Tuple,
          typename Indices = std::make_index_sequence<std::tuple_size<Tuple>::value>>
void printTuple(Tuple tuple) {
    printTupleImpl(tuple, Indices{});
}

template <std::size_t S, std::size_t E,
         typename Indices = std::make_index_sequence<E-S>>
constexpr auto range() {
    return incrSeq<S>(Indices{});
}

template <std::size_t I, typename Arg, typename... Args>
constexpr auto get(Arg&& arg, Args&&... args)
    -> std::enable_if_t<I == 0, Arg>
{
    return arg;
}

template <std::size_t I, typename Arg, typename... Args>
constexpr auto get(Arg&& arg, Args&&... args) 
    -> std::enable_if_t<(I > 0), typename std::tuple_element<I, std::tuple<Arg, Args...>>::type>
{
    return get<I - 1>(std::forward<Args>(args)...);
}

}// namespace detail

template <typename... Args,
          std::size_t... I>
void test_impl(std::index_sequence<I...>, Args&&... args) {
    std::cout << detail::get<0>(std::forward<Args>(args)...);
    ((std::cout << detail::get<I>(std::forward<Args>(args)...)),...);
    std::cout << detail::get<sizeof...(Args) - 1>(std::forward<Args>(args)...);
}

template <typename... Args>
void test(Args&&... args) {
    test_impl(detail::range<1,sizeof...(args)-1>(), std::forward<Args>(args)...);
}

int main(int argc, char* argv[])
{
    test(1, 'b', 'c', 3);
    return 0;
}
