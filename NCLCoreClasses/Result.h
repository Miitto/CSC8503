#pragma once

#include <optional>

namespace NCL {

template <typename Ok, typename Err> class Result {
  Result(const Err &err, int sen) : isOk(false), errValue(err) {}
  Result(Err &&err, int sen) : isOk(false), errValue(std::move(err)) {}

public:
  Result(const Ok &value) : isOk(true), okValue(value) {}
  Result(Ok &&value) : isOk(true), okValue(std::move(value)) {}

  ~Result() {
    if (isOk) {
      okValue.~Ok();
    } else {
      errValue.~Err();
    }
  }

  template <typename OkC = Ok, typename ErrC = Err>
    requires std::is_same_v<OkC, Ok> && std::is_same_v<ErrC, Err> &&
             (std::is_copy_constructible_v<OkC> &&
              std::is_copy_constructible_v<ErrC>)
  Result(const Result<OkC, ErrC> &other) : isOk(other.is_ok()) {
    if (isOk) {
      new (&okValue) Ok(other.unwrap());
    } else {
      new (&errValue) Err(other.unwrap_err());
    }
  }

  template <typename OkM = Ok, typename ErrM = Err>
    requires std::is_same_v<OkM, Ok> && std::is_same_v<ErrM, Err> &&
             std::is_move_constructible_v<OkM> &&
             std::is_move_constructible_v<ErrM>
  Result(Result<OkM, ErrM> &&other) noexcept : isOk(other.is_ok()) {
    if (isOk) {
      new (&okValue) Ok(std::move(other.unwrap()));
    } else {
      new (&errValue) Err(std::move(other.unwrap_err()));
    }
  }

  static inline Result<Ok, Err> err(const Err &value) {
    return Result<Ok, Err>(value, 0);
  }

  static inline Result<Ok, Err> err(Err &&value) {
    return Result<Ok, Err>(std::move(value), 0);
  }

  bool is_ok() const { return isOk; }
  bool is_err() const { return !isOk; }

  Ok unwrap() const {
    if (!isOk) {
      throw std::runtime_error("Called unwrap on an Err Result");
    }
    return okValue;
  }

  Ok &unwrap_ref() {
    if (!isOk) {
      throw std::runtime_error("Called unwrap_ref on an Err Result");
    }
    return okValue;
  }

  Err unwrap_err() const {
    if (isOk) {
      throw std::runtime_error("Called unwrap_err on an Ok Result");
    }
    return errValue;
  }

  std::optional<Ok> ok() const {
    if (isOk) {
      return okValue;
    }
    return std::nullopt;
  }

protected:
  bool isOk;
  union {
    Ok okValue;
    Err errValue;
  };
};

} // namespace NCL