#pragma once

#include <type_traits>

namespace NCL {
template <typename T>
  requires std::is_enum_v<T>
struct Bitflag {
  using Underlying = typename std::underlying_type<T>::type;
  constexpr inline Bitflag() : flags(0) {}
  constexpr inline Bitflag(T flag) : flags(static_cast<Underlying>(flag)) {}
  constexpr explicit inline Bitflag(Underlying flag) : flags(flag) {}

  constexpr inline Bitflag<T> &operator|=(T flag) {
    flags |= static_cast<Underlying>(flag);
    return *this;
  }

  constexpr inline Bitflag<T> &operator&=(T flag) {
    flags &= static_cast<Underlying>(flag);
    return *this;
  }

  constexpr inline Bitflag<T> &operator^=(T flag) {
    flags ^= static_cast<Underlying>(flag);
    return *this;
  }

  constexpr inline Bitflag<T> &operator|=(Bitflag<T> other) {
    flags |= other.flags;
    return *this;
  }

  constexpr inline Bitflag<T> &operator&=(Bitflag<T> other) {
    flags &= other.flags;
    return *this;
  }

  constexpr inline Bitflag<T> &operator^=(Bitflag<T> other) {
    flags ^= other.flags;
    return *this;
  }

  constexpr inline operator bool() const { return flags != 0; }
  constexpr inline bool operator!() const { return flags == 0; }

  constexpr explicit inline operator Underlying() const { return flags; }

  constexpr bool inline operator==(Bitflag<T> other) const {
    return flags == other.flags;
  }
  constexpr bool inline operator!=(Bitflag<T> other) const {
    return flags != other.flags;
  }

  constexpr inline Bitflag<T> operator~() const { return Bitflag<T>(~flags); }

  constexpr inline Bitflag &set(T flag) {
    flags |= static_cast<Underlying>(flag);
    return *this;
  }

  constexpr inline bool has(T flag) const {
    return (flags & static_cast<Underlying>(flag)) != 0;
  }

  constexpr inline Bitflag &clear(T flag) {
    flags &= ~static_cast<Underlying>(flag);
    return *this;
  }

  Underlying flags;
};

template <typename T>
  requires std::is_enum_v<T>
constexpr inline Bitflag<T> operator|(Bitflag<T> lhs, T rhs) {
  lhs |= rhs;
  return lhs;
}
template <typename T>
  requires std::is_enum_v<T>
constexpr inline Bitflag<T> operator&(Bitflag<T> lhs, T rhs) {
  lhs &= rhs;
  return lhs;
}

template <typename T>
  requires std::is_enum_v<T>
constexpr inline Bitflag<T> operator^(Bitflag<T> lhs, T rhs) {
  lhs ^= rhs;
  return lhs;
}

template <typename T>
  requires std::is_enum_v<T> && std::constructible_from<Bitflag<T>, T>
constexpr inline Bitflag<T> operator|(T lhs, T rhs) {
  return Bitflag<T>(lhs) | rhs;
}

template <typename T>
  requires std::is_enum_v<T> && std::constructible_from<Bitflag<T>, T>
constexpr inline Bitflag<T> operator&(T lhs, T rhs) {
  return Bitflag<T>(lhs) & rhs;
}

template <typename T>
  requires std::is_enum_v<T> && std::constructible_from<Bitflag<T>, T>
constexpr inline Bitflag<T> operator^(T lhs, T rhs) {
  return Bitflag<T>(lhs) ^ rhs;
}
} // namespace NCL