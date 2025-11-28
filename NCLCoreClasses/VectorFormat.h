#pragma once

#include "Vector.h"
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/ranges.h>

template <typename T, uint32_t n>
struct fmt::formatter<NCL::Maths::VectorTemplate<T, n>>
    : formatter<std::string> {
  template <typename FormatContext>
  auto format(const NCL::Maths::VectorTemplate<T, n> &vec,
              FormatContext &ctx) const {
    std::vector<T> values(vec.array, vec.array + n);
    std::string formatted =
        fmt::format("{}{}{}", "{", fmt::join(values, ","), "}");
    return fmt::formatter<std::string>::format(formatted, ctx);
  }
};
