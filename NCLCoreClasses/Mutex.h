#pragma once

#include <mutex>
#include <optional>

namespace NCL {

template <typename T> class Mutex;

template <typename T> class LockGuard {
public:
  LockGuard(Mutex<T> &m) : mutex(m), lock(m.mutex) {}

  T &operator*() { return mutex.value; }
  const T &operator*() const { return mutex.value; }

  T *operator->() { return &mutex.value; }
  const T *operator->() const { return &mutex.value; }

  T &value() { return mutex.value; }
  const T &value() const { return mutex.value; }

  std::unique_lock<std::mutex> &getLock() { return lock; }

protected:
  Mutex<T> &mutex;
  std::unique_lock<std::mutex> lock;
};

template <typename T> class Mutex {
  friend class LockGuard<T>;

public:
  Mutex() = default;
  Mutex(T initialValue) : value(initialValue) {}

  LockGuard<T> lock() {
    mutex.lock();
    return LockGuard<T>(*this);
  }

  LockGuard<const T> lock() const { return LockGuard<const T>(*this); }

  std::optional<LockGuard<T>> try_lock() {
    if (mutex.try_lock()) {
      return LockGuard<T>(*this);
    }
    return std::nullopt;
  }

protected:
  T value;
  std::mutex mutex;
};
} // namespace NCL