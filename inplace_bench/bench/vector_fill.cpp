#include <benchmark/benchmark.h>

struct node {
    node(char chr_, std::size_t value_, std::size_t value1_)
        : chr(chr_), value(value_), value1(value1_) {}

    char chr;
    std::size_t value;
    std::size_t value1;
    std::size_t value2;
    std::size_t value3;
    std::size_t son;
};

static void BM_VectorPush(benchmark::State& state) {
  std::vector<node> v;
  v.reserve(state.range(0));
  for (auto _ : state) {
      for (std::size_t i = 0; i < state.range(0); ++i) {
          v.push_back(node(1,2,3));
      }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_VectorPush)->Range(8, 8<<15);

// Define another benchmark
static void BM_VectorEmplace(benchmark::State& state) {
  std::vector<node> v;
  v.reserve(state.range(0));
  for (auto _ : state) {
      for (std::size_t i = 0; i < state.range(0); ++i) {
          v.emplace_back(1,2,3);
      }
  }
}

BENCHMARK(BM_VectorEmplace)->Range(8, 8<<15);

BENCHMARK_MAIN();
