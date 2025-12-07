#pragma once

namespace NCL {
template <typename T> class IteratorRange {
  T b;
  T e;

public:
  IteratorRange(T begin, T end) : b(begin), e(end) {}

  T begin() const { return b; }
  T end() const { return e; }
};
} // namespace NCL