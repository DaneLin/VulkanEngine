#ifndef __ARC_UTILS_H__
#define __ARC_UTILS_H__

#include <functional>

namespace arc
{
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    inline void hashCombine(std::size_t &seed, T const &v, Rest &&...rest)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (int[]){0, (hashCombine(seed, std::forward<Rest>(rest)), 0)...};
    }
}

#endif // __ARC_UTILS_H__