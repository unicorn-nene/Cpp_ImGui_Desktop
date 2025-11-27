#include <cstdio>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>

namespace LightYears
{
    template <typename T>
    using unique = std::unique_ptr<T>;

    template <typename T>
    using shared = std::shared_ptr<T>;

    template <typename T>
    using weak = std::weak_ptr<T>;

    template <typename T>
    using List = std::vector<T>;

    template <typename keyType, typename valType, typename Pr = std::less<keyType>> // Pr = predicate 谓词
    using Map = std::map<keyType, valType, Pr>;

    template <typename keyType, typename valType, typename hasher = std::hash<keyType>>
    using Dictionary = std::unordered_map<keyType, valType, hasher>;

// A marco
#define LOG(M, ...) printf(M "\n", ##__VA_ARGS__) // ##__VA_ARGS__ 是 token-pasting operator（粘贴操作符), 当可变操作为空时, 自动去掉前面的','符号.
}