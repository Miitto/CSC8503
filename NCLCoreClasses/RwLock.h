#pragma once

#include <mutex>
#include <shared_mutex>
#include <variant>

namespace NCL {
template <typename T> class RwLock;
template <typename T> class ReadLockGuard {
  friend class RwLock<T>;

public:
  const T &operator*() const { return mutex.value; }

  const T *operator->() const { return &mutex.value; }

  const T &value() const { return mutex.value; }

  ReadLockGuard(const RwLock<T> &mutex, RwLock<T>::ReadLock &&lock)
      : mutex(mutex), lock(std::move(lock)) {}

protected:
  const RwLock<T> &mutex;
  RwLock<T>::ReadLock lock;
};

template <typename T> class WriteLockGuard {
  friend class RwLock<T>;

public:
  T &operator*() { return mutex.value; }
  const T &operator*() const { return mutex.value; }

  T *operator->() { return &mutex.value; }
  const T *operator->() const { return &mutex.value; }

  T &value() { return mutex.value; }
  const T &value() const { return mutex.value; }

  WriteLockGuard(RwLock<T> &mutex, RwLock<T>::WriteLock &&lock)
      : mutex(mutex), lock(std::move(lock)) {}

protected:
  RwLock<T> &mutex;
  RwLock<T>::WriteLock lock;
};

template <typename T> class RwLock {
  friend class ReadLockGuard<T>;
  friend class WriteLockGuard<T>;

public:
  using ReadLock = std::shared_lock<std::shared_mutex>;
  using WriteLock = std::unique_lock<std::shared_mutex>;

  RwLock(const T &initialValue) : value(initialValue) {}
  RwLock(T &&initialValue) : value(std::move(initialValue)) {}

  ReadLockGuard<T> read() {
    ReadLock lock(mutex);
    ReadLockGuard<T> l(*this, std::move(lock));
    return l;
  }

  WriteLockGuard<T> write() {
    WriteLock lock(mutex);
    WriteLockGuard<T> l(*this, std::move(lock));
    return l;
  }

protected:
  std::shared_mutex mutex = {};
  T value;
};
} // namespace NCL