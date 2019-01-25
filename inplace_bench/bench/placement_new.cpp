#include <benchmark/benchmark.h>
#include <memory>

struct Toto {
    size_t i = 0;
    std::vector<int> v = {};
    std::string s = {};
};

struct Tata {
    Tata(size_t i_, const std::vector<int>& v_, std::string s_)
        : i(i_), v(v_), s(s_) {}

    size_t i;
    std::vector<int> v;
    std::string s;
};

static void BM_Allocator(benchmark::State& state) {
    std::allocator<Tata> a;
    using alloc_traits = std::allocator_traits<std::allocator<Tata>>;
    for (auto _ : state) {
        auto* elt = alloc_traits::allocate(a, 1);
        alloc_traits::construct(a, elt, 42, std::vector<int>{1,2,3}, "toto");
        benchmark::DoNotOptimize(elt);
        delete elt;
    }
}
// Register the function as a benchmark
BENCHMARK(BM_Allocator);

static void BM_New(benchmark::State& state) {
    std::allocator<Toto> a;
    using alloc_traits = std::allocator_traits<std::allocator<Toto>>;
    for (auto _ : state) {
        Toto* elt = alloc_traits::allocate(a, 1);
        elt = new (elt) Toto{42, {1,2,3}, "toto"};
        benchmark::DoNotOptimize(elt);
        delete elt;
    }
}

BENCHMARK(BM_New);

BENCHMARK_MAIN();
