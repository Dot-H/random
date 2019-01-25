#include <benchmark/benchmark.h>

struct Toto {
    size_t i = 0;
    std::vector<int> v = {};
    std::string s = {};
};

struct Tata {
    Tata()
        : i(0), v(), s() {}
    Tata(std::size_t i_, const std::vector<int>& v_, const std::string& s_) {
        i = i_;
        v = v_;
        s = s_;
    }

    size_t i;
    std::vector<int> v;
    std::string s;
};

static void BM_EmplaceBack(benchmark::State& state) {
    std::vector<Tata> v;
    benchmark::DoNotOptimize(v);
    for (auto _ : state) {
        v.emplace_back(42, std::vector<int>{1, 2, 3}, "toto");
    }
}
// Register the function as a benchmark
BENCHMARK(BM_EmplaceBack);

static void BM_PushBack(benchmark::State& state) {
    std::vector<Tata> v;
    benchmark::DoNotOptimize(v);
    for (auto _ : state) {
        v.push_back({42, std::vector<int>{1, 2, 3}, "toto"});
    }
}
// Register the function as a benchmark
BENCHMARK(BM_PushBack);

BENCHMARK_MAIN();
