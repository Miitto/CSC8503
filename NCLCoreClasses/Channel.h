#pragma once

#include "Mutex.h"
#include <condition_variable>
#include <memory>
#include <optional>
#include <queue>

namespace NCL {
template <typename T> class Channel {
  using Lock = std::unique_lock<std::mutex>;

public:
  Channel() : inner(std::make_shared<Inner>()) {}

  void send(const T &item) {
    Lock lock(inner->mutex);
    inner->items.push(item);
    inner->dataAvailable = true;
    inner->cv.notify_all();
  }

  void send(T &&item) {
    Lock lock(inner->mutex);
    inner->items.push(std::move(item));
    inner->dataAvailable = true;
    inner->cv.notify_all();
  }

  std::optional<T> receive() {
    Lock lock(inner->mutex);
    if (!inner->dataAvailable) {
      inner->cv.wait(lock, [&]() { return inner->dataAvailable; });
    }

    T item = std::move(inner->items.front());
    inner->items.pop();

    if (inner->items.empty()) {
      inner->dataAvailable = false;
    }

    return item;
  }

protected:
  struct Inner {
    std::queue<T> items = {};
    std::mutex mutex = {};
    std::condition_variable cv = {};
    bool dataAvailable = false;
  };

  std::shared_ptr<Inner> inner;
};
} // namespace NCL