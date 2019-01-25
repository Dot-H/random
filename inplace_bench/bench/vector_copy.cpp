#include <benchmark/benchmark.h>

static void BM_VectorCopyPush(benchmark::State& state) {
  std::vector<char> v(state.range(0), 'a');
  std::vector<char> c;
  for (auto _ : state) {
      c.resize(v.size());
      for (std::size_t i = 0; i < v.size(); ++i) {
          c[i] = v[i];
      }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_VectorCopyPush)->Range(8, 8<<15);

// Define another benchmark
static void BM_VectorCopyAssignment(benchmark::State& state) {
  std::vector<char> v(state.range(0), 'a');
  std::vector<char> c;
  for (auto _ : state) {
      c = v;
  }
}

BENCHMARK(BM_VectorCopyAssignment)->Range(8, 8<<15);

BENCHMARK_MAIN();

