#pragma once

#include "utils/Types.h"
#include "json/JSONWriter.h"

#include <thread>
#include <sstream>

namespace bench {

using OutBuff_t = ABuffer;

// -----
// Utils
// -----
[[maybe_unused]] static std::string prettyPrint(AULong ns) {
    static const std::vector<std::string> units = { "ns", "us", "ms", "s" };

    std::string ret = std::to_string(ns);
    AULong n = 0;
    while ((ns /= 1000) > 0 && n < units.size()) {
        ++n;
    }

    if (n > 0) {
        ret.insert(ret.size() - (3 * n), ".");
    }

    ret.append(units[n]);
    return ret;
}

[[maybe_unused]] static inline std::string threadIdToStr(const std::thread::id& tid) {
        std::stringstream threadId;
        threadId << tid;
        return threadId.str();
}

/**
 * This is a helper on the Curiously recurring template pattern taken from
 * this last episode (https://www.fluentcpp.com/2017/05/19/crtp-helper/)
 * of a serie of articles written by Jonathan Boccara about the pattern.
 */
template <typename T, template<typename> class crtpType>
struct crtp {
    T& underlying() { return static_cast<T&>(*this); }
    T const& underlying() const { return static_cast<T const&>(*this); }

private:
    // This is a compile time security to prevent a class which is not
    // `crtpType` to use this crtp instanciation
    crtp(){}
    friend crtpType<T>;
};

} // namespace bench