#pragma once

namespace NCL {
template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
} // namespace NCL