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

static void BM_Toto(benchmark::State& state) {
  for (auto _ : state) {
      Toto t;
      benchmark::DoNotOptimize(t);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_Toto);

static void BM_Tata(benchmark::State& state) {
  for (auto _ : state) {
      Tata t;
      benchmark::DoNotOptimize(t);
  }
}

BENCHMARK(BM_Tata);

static void BM_TotoVal(benchmark::State& state) {
  for (auto _ : state) {
      Toto t{42, {1, 2, 3}, "toto"};
      benchmark::DoNotOptimize(t);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_TotoVal);

static void BM_TataVal(benchmark::State& state) {
  for (auto _ : state) {
      Tata t(42, {1, 2, 3}, "toto");
      benchmark::DoNotOptimize(t);
  }
}

BENCHMARK(BM_TataVal);

BENCHMARK_MAIN();
