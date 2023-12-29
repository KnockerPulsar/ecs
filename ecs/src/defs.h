#pragma once

#include <any>
#include <cstdint>
#include <optional>
#include <vector>

using u32 = uint32_t;
using u8  = uint8_t;
using f32 = float;

namespace ecs {

using Entity = std::uint32_t;

template <typename T>
using Vector = typename std::vector<std::optional<T>>;

template <typename T>
using OptIter = typename Vector<T>::iterator;

template <typename T>
using Iter = typename std::vector<T>::iterator;

template <typename... Ts>
using Query = std::tuple<Ts...>;

// Forward decl
template <typename... Ts>
struct MultiIterator;

template <typename... Ts>
using ComponentIter = MultiIterator<Ts...>;

template <typename Fn, typename Return, typename... Args>
concept MatchSignature = std::is_invocable_r_v<Return, Fn, Args...>;

} // namespace ecs
