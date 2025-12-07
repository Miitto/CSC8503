#pragma once

#include <spdlog/fmt/bundled/format.h>
#include <string_view>

#include "macros.h"
#include <Transform.h>

namespace NCL {
enum class VolumeType {
  Invalid = 0,
  AABB = BIT(1),
  OBB = BIT(2),
  Sphere = BIT(3),
  Mesh = BIT(4),
  Capsule = BIT(5),
  Compound = BIT(6),
};

class CollisionVolume {
public:
  CollisionVolume() { type = VolumeType::Invalid; }
  ~CollisionVolume() = default;

  virtual float GetMaxExtent() const = 0;
  virtual Maths::Vector3
  GetBottomPoint(const NCL::CSC8503::Transform &transform) const = 0;

  VolumeType type;
};
} // namespace NCL

template <>
struct fmt::formatter<NCL::VolumeType> : formatter<std::string_view> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const NCL::VolumeType &type, FormatContext &ctx) const {
    const char *typeStr = "Unknown";
    switch (type) {
    case NCL::VolumeType::AABB:
      typeStr = "AABB";
      break;
    case NCL::VolumeType::OBB:
      typeStr = "OBB";
      break;
    case NCL::VolumeType::Sphere:
      typeStr = "Sphere";
      break;
    case NCL::VolumeType::Mesh:
      typeStr = "Mesh";
      break;
    case NCL::VolumeType::Capsule:
      typeStr = "Capsule";
      break;
    case NCL::VolumeType::Compound:
      typeStr = "Compound";
      break;
    default:
      typeStr = "Invalid";
      break;
    }
    return fmt::formatter<std::string_view>::format(typeStr, ctx);
  }
};
