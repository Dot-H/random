#include <utility>

#include "utils/bench/BenchEnv.h"

namespace bench {

#ifndef ALGOLIA_PROFILING
    #define ALGOLIA_PROFILING 0
#endif

// ------
// Timers
// ------
#if ALGOLIA_PROFILING == 1
/**
 * This class compute the time taken between its creation and the call to 
 * its destruction.
 * The elapsed time is added to the accumulator given as argument.
 * 
 * @Example ```
*       std::chrono::nanoseconds elapsed;
 *      {
 *       ScopedTimer timer(elapsed);
 *       // Do stuff
 *      } // when leaving this scope elapsed contains the time taken by `Do stuff`
 *         ```
 */
template <typename DurationT>
struct ScopedTimer {
    constexpr explicit ScopedTimer(DurationT& acc_)
        : acc(acc_)
        {
            // Initialisation order in initializer list is undefined. We setup
            // `_start` here in order to make sure it is not assigned before
            // constructing the `_infos` object.
            start = std::chrono::steady_clock::now();
        }

    FORCEINLINE ~ScopedTimer() {
        auto end = std::chrono::steady_clock::now();
        acc += end - start;
    }

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

    DurationT& acc;
    std::chrono::time_point<std::chrono::steady_clock> start;
};

// ---------------
// Deduction guide
// ---------------
template <typename DurationT>
ScopedTimer(DurationT& acc_) -> ScopedTimer<DurationT>;

// -----------------------
// Bench computing classes
// -----------------------

/**
 * Benchmark a task represented by `_task`. On destruction, the object
 * add its `BenchInfos` object to the `BenchEnv` instance.
 * 
 * @Example ```
 *      int foo(int arg1, bool arg2) { // Do stuff and return int }
 *      int main() {
 *          auto bench_foo = bench::BenchTask("foo", foo);
 *          bench_foo.addComment("N", 10);
 *          bench_foo.addComment("Predicate", "false");
 *          return bench_foo.run(10, false);
 *      }
 *          ```
 * 
 * @Note to use it on a function member, use the `std::mem_fn` to pass it to the
 *       `BenchTask` constructor and pass the object as first argument when calling
 *       `BenchTask::run`.
 * @Note Once created, a benchTask object cannot change its task. This helps
 *       the compiler inline the task and avoid having overhead from the call.
 */
template <typename Task, typename Infos = BenchInfos>
class BenchTask {
public:
    CLASS_NON_COPYABLE(BenchTask);

    template <std::size_t N>
    constexpr BenchTask(const char (&name)[N], const Task task)
        : _infos{{}, name, {}},
          _task(task) {}

    BenchTask(BenchTask&& o) = default;
    BenchTask& operator=(BenchTask&& o) = default;

    ~BenchTask() {
        auto& env = getEnvInstance<Infos>();
        env.addLeaf(std::move(_infos));
    }

    template <typename... Args>
    constexpr auto run(Args&&... args) {
        ScopedTimer timer(_infos.elapsed);
        return _task(std::forward<Args>(args)...);
    }

    template <typename Key, typename Value>
    void addComment(Key&& key, Value&& value) {
        _infos.addComment(std::forward<Key>(key), std::forward<Value>(value));
    }

private:
    Infos _infos;
    const Task _task;
};

/**
 * Benchmark a scope between its start and its end. On destruction, the object
 * add its `BenchInfos` object to the `BenchEnv` instance.
 * 
 * @Example ```
 *     for (std::size_t i = 0; i < n; ++i) {
 *        bench::BenchScope scope("push_back");

 *        std::string s;
 *        s = std::to_string(i);
 *        dst.push_back(s);
 *    }
 *          ```
 */
template <typename Infos = BenchInfos>
class BenchScope {
public:

    template<std::size_t N>
    constexpr BenchScope(const char (&name)[N])
        : _infos{{}, name, {}} {
            // Initialisation order in initializer list is undefined. We setup
            // `_start` here in order to make sure it is not assigned before
            // constructing the `_infos` object.
            _start = std::chrono::steady_clock::now(); 
        }
        
    FORCEINLINE ~BenchScope() {
        auto end = std::chrono::steady_clock::now();
        _infos.elapsed += end - _start;

        auto& env = getEnvInstance<Infos>();
        env.addLeaf(std::move(_infos));
    }

    BenchScope(const BenchScope&) = delete;
    BenchScope(BenchScope&&) = delete;
    BenchScope& operator=(const BenchScope&) = delete;
    BenchScope& operator=(BenchScope&&) = delete;

    template <typename Key, typename Value>
    constexpr void addComment(Key&& key, Value&& value) {
        // Do not take into account the time taken to add the comment
        auto end = std::chrono::steady_clock::now();
        _infos.elapsed += end - _start;

        _infos.addComment(std::forward<Key>(key), std::forward<Value>(value));
        _start = std::chrono::steady_clock::now();
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> _start;
    Infos _infos;
};

/**
 * Represent a container of bench objects. Its elapsed time is the sum of all bench
 * objects declared before the end of its scope.
 * `BenchBunch` objects can be nested.
 * 
 * @Example ```
 *     static void bad_to_string(std::vector<std::string>& dst, std::size_t n) {
*         auto bunch = bench::BenchBunch("bad_to_string");
*         bunch.addComment("N", n);
*         for (std::size_t i = 0; i < n; ++i) {
*             bench::BenchScope scope("push_back");
*
*             std::string s;
*             s = std::to_string(i);
*             dst.push_back(s);
*         }
*      }
*          ```
*/
template <typename Infos = BenchInfos>
class BenchBunch {
public:
    using Comments_t = std::vector<typename Infos::Comment_t>;

    template <std::size_t N>
    constexpr BenchBunch(const char (&name)[N]) 
        : _infos(getEnvInstance<Infos>().emplaceSubLevel(std::chrono::nanoseconds{}, name, Comments_t{})) {}

    template <typename Key, typename Value>
    void addComment(Key&& key, Value&& value) {
        _infos.addComment(std::forward<Key>(key), std::forward<Value>(value));
    }

private:
    Infos& _infos;
};

#else
template <typename DurationT>
struct ScopedTimer {};

template <typename Task, typename Infos = BenchInfos>
class BenchTask {
public:
    CLASS_NON_COPYABLE(BenchTask);

    template <std::size_t N>
    constexpr BenchTask(const char (&)[N], const Task task)
        : _task(task) {}

    BenchTask(BenchTask&& o) = default;
    BenchTask& operator=(BenchTask&& o) = default;

    template <typename... Args>
    constexpr auto run(Args&&... args) {
        return _task(std::forward<Args>(args)...);
    }

    template <typename Key, typename Value>
    void addComment(Key&& key, Value&& value) {}

private:
    const Task _task;
};

template <typename Infos = BenchInfos>
class BenchScope {
public:
    template<std::size_t N>
    BenchScope(const char (&name)[N]) {}

    BenchScope(const BenchScope&) = delete;
    BenchScope(BenchScope&&) = delete;
    BenchScope& operator=(const BenchScope&) = delete;
    BenchScope& operator=(BenchScope&&) = delete;

    template <typename Key, typename Value>
    void addComment(Key&& key, Value&& value) {}
};

// ----------------
// Deduction guides
// ----------------
template <typename Task, std::size_t N>
BenchTask(const char (&)[N], const Task)
    -> BenchTask<Task>;

template <typename Infos = BenchInfos>
class BenchBunch {
public:
    template <std::size_t N>
    BenchBunch(const char (&)[N]) {}

    template <typename Key, typename Value>
    void addComment(Key&& key, Value&& value) {}
};
#endif

/**
 * Capture and run a code snippet and store its bench with the `NAME` value.
 * 
 * @Example ```
 *      static void bench_fibo(std::size_t n) {
 *          auto bunch = bench::BenchBunch("Fibo&flush");
 *          bunch.addComment("N", n);
 *
 *          auto out = fibo(n);
 *          START_BENCH_SNIPPET("write stderr") {
 *              std::cerr << out << std::endl;
 *          } END_BENCH_SNIPPET();
 *      }
 *          ```
 */
#define START_BENCH_SNIPPET(NAME) ::bench::BenchTask(NAME, [&] 
#define END_BENCH_SNIPPET() ).run()

/**
 * Bench the function execution and store the bench with the function name.
 * 
 * @Example ```
 *      static std::size_t fibo(std::size_t n) {
 *          BENCH_THIS_FUNCTION();
 *
 *          if (n < 2) return n;
 *
 *          std::size_t n0 = 0;
 *          std::size_t n1 = 1;
 *
 *          for (std::size_t i = 1; i < n; ++i) {
 *              std::size_t tmp = n1;
 *              n1 = n0 + n1;
 *              n0 = tmp;
 *          }
 *
 *          return n1;
 *      }
 *          ```
 */
#define BENCH_THIS_FUNCTION() ::bench::BenchScope scope(__PRETTY_FUNCTION__)

} // namespace bench