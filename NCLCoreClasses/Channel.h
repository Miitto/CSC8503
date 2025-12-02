#pragma once

#include "Mutex.h"
#include <memory>
#include <optional>
#include <queue>

namespace NCL {
template <typename T> class Channel {
public:
  Channel() : items(std::shared_ptr<Mutex<std::queue<T>>>()) {}

  void send(const T &item) {
    auto items = items.lock();
    items->push_pack(item);
  }

  template <typename... Args> void emplace(Args &&...args) {
    auto items = items->lock();
    items->emplace_back(std::forward<Args>(args)...);
  }

  std::optional<T> receive() {
    auto items = items->lock();
    if (items->empty()) {
      return std::nullopt;
    }
    T item = std::move(items->front());
    items.erase(items->begin());
    return item;
  }

  std::optional<T> try_receive() {
    auto lock = items->try_lock();
    if (!lock)
      return std::nullopt;

    if (items->empty())
      return std::nullopt;

    T item = std::move(items->front());
    items.erase(items->begin());
    return item;
  }

  bool has_items() const {
    auto lock = items->try_lock();
    if (!lock)
      return false;
    return !items->empty();
  }

protected:
  mutable std::shared_ptr<Mutex<std::queue<T>>> items;
};
} // namespace NCL