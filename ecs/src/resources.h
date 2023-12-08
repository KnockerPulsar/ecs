#pragma once

#include <functional>
#include <typeindex>
#include <optional>
#include <any>

namespace ecs {
struct Resources {
  std::unordered_map<std::type_index, std::any> r;

  template <typename R>
  void addResource(R initialValue) {
    r.insert({typeid(R), initialValue});
  }

  template <typename R>
  std::optional<std::reference_wrapper<R>> getResource() {
    if (!r.contains(typeid(R)))
      return std::nullopt;

    return std::any_cast<R &>(r.find(typeid(R))->second);
  }

  template <typename R>
  std::optional<R> consumeResource() {
    if(!r.contains(typeid(R))) {
      return {};
    }

    auto resIter = r.find(typeid(R));
    auto resCopy = R(std::any_cast<R>(resIter->second));
    r.erase(resIter);

    return resCopy;
  }
};
}
