#pragma once

#include "Mutex.h"
#include "Result.h"
#include "RwLock.h"
#include <condition_variable>
#include <memory>
#include <optional>
#include <queue>

namespace NCL {
namespace mpsc {
template <typename T> class Sender;
template <typename T> class Receiver;
} // namespace mpsc

template <typename T> class ChannelBase {

protected:
  using Lock = std::unique_lock<std::mutex>;
  ChannelBase() = default;
  ChannelBase(const ChannelBase<T> &other) = default;
  ChannelBase<T> &operator=(const ChannelBase<T> &other) = default;
  ChannelBase(ChannelBase<T> &&other) noexcept
      : inner(std::move(other.inner)), counts(std::move(other.counts)) {
    other.movedFrom = true;
  }
  ChannelBase<T> &operator=(ChannelBase<T> &&other) noexcept {
    if (this != &other) {
      inner = std::move(other.inner);
      counts = std::move(other.counts);
      other.movedFrom = true;
    }
    return *this;
  }

  struct Inner {
    std::queue<T> items = {};
    std::mutex mutex = {};
    std::condition_variable cv = {};
    bool dataAvailable = false;
  };

  std::shared_ptr<Inner> inner = std::make_shared<Inner>();

  struct Counts {
    int senderCount = 0;
    int receiverCount = 0;
  };
  std::shared_ptr<RwLock<Counts>> counts =
      std::make_shared<RwLock<Counts>>(Counts());

  bool movedFrom = false;
};

namespace mpsc {

template <typename T> class Receiver : public ChannelBase<T> {
protected:
  using Lock = std::unique_lock<std::mutex>;

  bool isClosed() {
    auto lock = this->counts->read();
    return lock->senderCount == 0;
  }

public:
  Receiver() : ChannelBase<T>() { this->counts->write()->receiverCount++; }
  Receiver(ChannelBase<T> &&other) : ChannelBase<T>(std::move(other)) {
    this->counts->write()->receiverCount++;
  }

  ~Receiver() {
    if (this->movedFrom)
      return;
    auto lock = this->counts->write();
    lock->receiverCount--;
  }

  Receiver(const Receiver &other) = delete;
  Receiver(Receiver &&other) noexcept : ChannelBase<T>(std::move(other)) {}
  Receiver &operator=(const Receiver &other) = delete;
  Receiver &operator=(Receiver &&other) noexcept {
    if (this != &other) {
      ChannelBase<T>::operator=(std::move(other));
    }
    return *this;
  }

  enum class ReceiveError { Closed };

  Result<T, ReceiveError> receive() {
    if (isClosed() && this->inner->items.empty()) {
      return Result<T, ReceiveError>::err(ReceiveError::Closed);
    }

    Lock lock(this->inner->mutex);
    if (!this->inner->dataAvailable) {
      this->inner->cv.wait(lock, [&]() { return this->inner->dataAvailable; });
    }
    if (this->inner->items.empty()) {
      if (isClosed()) {
        return Result<T, ReceiveError>::err(ReceiveError::Closed);
      } else {
        // Spurious wakeup, continue waiting
        this->inner->dataAvailable = false;
        return receive();
      }
    }

    T item = std::move(this->inner->items.front());
    this->inner->items.pop();

    if (this->inner->items.empty()) {
      this->inner->dataAvailable = false;
    }

    return item;
  }
};

template <typename T> class Sender : public ChannelBase<T> {
protected:
  using Lock = std::unique_lock<std::mutex>;

  bool isClosed() {
    auto lock = this->counts->read();
    return lock->receiverCount == 0;
  }

public:
  Sender() : ChannelBase<T>() { this->counts->write()->senderCount++; }
  Sender(ChannelBase<T> &&other) : ChannelBase<T>(std::move(other)) {
    this->counts->write()->senderCount++;
  }

  Sender(const Sender &other) : ChannelBase<T>(other) {
    this->counts->write()->senderCount++;
  }
  Sender(const Receiver<T> &other) : ChannelBase<T>(other) {
    this->counts->write()->senderCount++;
  }
  Sender &operator=(const Sender &other) {
    if (this != &other) {
      ChannelBase<T>::operator=(other);
      this->counts->write()->senderCount++;
    }
    return *this;
  }
  Sender(Sender<T> &&other) noexcept : ChannelBase<T>(std::move(other)) {}
  Sender &operator=(Sender<T> &&other) noexcept {
    if (this != &other) {
      ChannelBase<T>::operator=(std::move(other));
    }
    return *this;
  }

  ~Sender() {
    if (this->movedFrom)
      return;
    auto lock = this->counts->write();
    lock->senderCount--;
    if (lock->senderCount == 0) {
      Lock innerLock(this->inner->mutex);
      this->inner->dataAvailable = true;
      this->inner->cv.notify_all();
    }
  }

  enum class SendResult { Success, Closed };

  SendResult send(const T &item) {
    if (isClosed()) {
      return SendResult::Closed;
    }

    Lock lock(this->inner->mutex);
    this->inner->items.push(item);
    this->inner->dataAvailable = true;
    this->inner->cv.notify_all();

    return SendResult::Success;
  }

  std::optional<std::pair<T, SendResult>> send(T &&item) {
    if (isClosed()) {
      return std::move(std::pair(std::move(item), SendResult::Closed));
    }

    Lock lock(this->inner->mutex);
    this->inner->items.push(std::move(item));
    this->inner->dataAvailable = true;
    this->inner->cv.notify_all();

    return std::nullopt;
  }
};

template <typename T> std::pair<Sender<T>, Receiver<T>> create() {
  Receiver<T> receiver;
  Sender<T> sender(receiver);

  return {std::move(sender), std::move(receiver)};
}
} // namespace mpsc
} // namespace NCL