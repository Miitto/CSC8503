#pragma once

#include <spdlog/fmt/bundled/format.h>

enum class Level : uint8_t {
  Default,
  CollisionTest,
};

template <> struct fmt::formatter<Level> : fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const Level &level, FormatContext &ctx) const {
    std::string_view name = "Unknown";
    switch (level) {
    case Level::Default:
      name = "Default";
      break;
    case Level::CollisionTest:
      name = "CollisionTest";
      break;
    }
    return fmt::formatter<std::string_view>::format(name, ctx);
  }
};
