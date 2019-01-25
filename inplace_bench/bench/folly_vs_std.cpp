#include <benchmark/benchmark.h>
#include <folly/FBVector.h>
#include <folly/FBString.h>

struct toto {
    size_t i;
    size_t a;
    size_t b;
    folly::fbstring c;
};

namespace folly {
    template <>
  struct IsRelocatable<toto> : std::true_type {};
}

static void BM_VectorPush(benchmark::State& state) {
  std::vector<toto> v;
  for (auto _ : state) {
      for (std::size_t i = 0; i < state.range(0); ++i) {
          v.push_back({1, 2, 3, "toto"});
      }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_VectorPush)->Range(8, 8<<20);

static void BM_VectorPushScalar(benchmark::State& state) {
  std::vector<std::size_t> v;
  for (auto _ : state) {
      for (std::size_t i = 0; i < state.range(0); ++i) {
          v.push_back('a');
      }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_VectorPushScalar)->Range(8, 8<<20);

static void BM_FollyPushScalar(benchmark::State& state) {
  folly::fbvector<std::size_t> v;
  for (auto _ : state) {
      for (std::size_t i = 0; i < state.range(0); ++i) {
          v.push_back('a');
      }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_FollyPushScalar)->Range(8, 8<<20);

// Define another benchmark
static void BM_FollyPush(benchmark::State& state) {
  folly::fbvector<toto> v;
  for (auto _ : state) {
      for (std::size_t i = 0; i < state.range(0); ++i) {
          v.push_back({1, 2, 3, "toto"});
      }
  }
}


BENCHMARK(BM_FollyPush)->Range(8, 8<<20);

BENCHMARK_MAIN();

