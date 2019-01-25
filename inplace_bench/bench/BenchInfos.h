#pragma once

#include "utils/bench/BenchUtils.h"

#include <string>
#include <chrono>

namespace bench {

// -------------
// Infos classes
// -------------

struct BenchInfos {
    using Key_t = std::string;
    using Value_t = std::string;
    using Comment_t  = std::pair<Key_t, Value_t>;

    std::chrono::nanoseconds elapsed;
    const std::string name;
    std::vector<Comment_t> comments;

    template <typename Key, typename Value>
    void addComment(Key&& key, Value&& value) {
        static_assert(std::is_convertible_v<Key, std::string>,
                      "Invalid Key type");
        static_assert(std::is_arithmetic_v<std::remove_reference_t<Value>>
                      || std::is_convertible_v<Value, std::string>,
                      "Invalid Value type");
        if constexpr (std::is_arithmetic_v<std::remove_reference_t<Value>>) {
            comments.emplace_back(std::forward<Key>(key),
                                  std::to_string(value));
        } else {
            comments.emplace_back(std::forward<Key>(key),
                                  std::forward<Value>(value));
        }
    }
};

} // namespace bench
