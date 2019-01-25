#include <bitset>
#include <benchmark/benchmark.h>
#include <climits>

constexpr inline int MAX_FIELD_ID = 65;
// Maximum value stored in the position list of the inverted list
constexpr inline int INVERTED_LIST_POSITION_BITSET_SIZE = 65536;
// type used to store the biset of position fields

static void BM_NaifSet(benchmark::State& state) {
    using PositionBitSet = std::bitset<INVERTED_LIST_POSITION_BITSET_SIZE>;
    using FieldBitSet = std::bitset<MAX_FIELD_ID>;

    PositionBitSet positionBitSet;
    for (auto _ : state) {
         for (std::size_t i = 0; i < state.range(0); ++i) {
              std::size_t first = i * 1000;
              std::size_t last = (i + 1) * 1000;
              for (std::size_t j = first; j < last && j < INVERTED_LIST_POSITION_BITSET_SIZE; ++j) {
                  positionBitSet[j] = true;
              }
         }
    }
}
// Register the function as a benchmark
BENCHMARK(BM_NaifSet)->Range(1, 65);

static void BM_YoloSet(benchmark::State& state) {
    using PositionBitSet = std::bitset<INVERTED_LIST_POSITION_BITSET_SIZE>;
    using FieldBitSet = std::bitset<MAX_FIELD_ID>;

    PositionBitSet positionBitSet;
    for (auto _ : state) {
         for (std::size_t i = 0; i < state.range(0); ++i) {
             PositionBitSet tmp(ULLONG_MAX);
              tmp <<= i * 1000;
              positionBitSet |= tmp;
              for (int i = 1; i < 15; ++i) {
                  tmp <<= 64;
                  positionBitSet |= tmp;
              }
         }
    }
}
// Register the function as a benchmark
BENCHMARK(BM_YoloSet)->Range(1, 65);

BENCHMARK_MAIN();

