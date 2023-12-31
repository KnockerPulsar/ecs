#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <optional>
#include <typeindex>

namespace ecs {
struct Resources {
  std::unordered_map<std::type_index, std::any> r;

  template <typename R>
  void addResource(R&& initialValue) {
    if(r.contains(typeid(R))) {
      std::cerr<< "Resource already exists\n";
    }
    r.insert_or_assign(typeid(R), std::move(initialValue));
  }

  template <typename R>
  std::optional<std::reference_wrapper<R>> getResource() {
    if (!r.contains(typeid(R)))
      return std::nullopt;

    return std::any_cast<R &>(r.find(typeid(R))->second);
  }

  template <typename R>
  std::optional<R> consumeResource() {
    if (!r.contains(typeid(R))) {
      return {};
    }

    auto resIter = r.find(typeid(R));
    auto resCopy = R(std::any_cast<R>(resIter->second));
    r.erase(resIter);

    return resCopy;
  }

  void clear() { r.clear(); }
};

struct ResourceBundle {
  Resources &global, &level;
};

} // namespace ecs
