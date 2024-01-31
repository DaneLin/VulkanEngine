#ifndef __ARC_UTILS_H__
#define __ARC_UTILS_H__

#include <functional>

namespace arc
{
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t &seed, const T &v, const Rest &...rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    }
}

#endif // __ARC_UTILS_H__