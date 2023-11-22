#pragma once

#include <any>
#include <cstdint>
#include <optional>
#include <vector>

using u32 = uint32_t;

namespace ecs {

using Entity = std::uint32_t;

template <typename T>
using Vector = typename std::vector<std::optional<T>>;
template <typename T>
using OptIter = typename Vector<T>::iterator;
template <typename T>
using Iter = typename std::vector<T>::iterator;
} // namespace ecs
