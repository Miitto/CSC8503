#pragma once

#include "Bitflag.h"
#include <spdlog/fmt/bundled/format.h>
#include <string_view>

#include "macros.h"
#include <Transform.h>

namespace NCL {
enum class VolumeType : uint8_t {
  Invalid = 0,
  AABB = BIT(1),
  OBB = BIT(2),
  Sphere = BIT(3),
  Mesh = BIT(4),
  Capsule = BIT(5),
};

enum class VolumeFlags : uint8_t {
  Trigger = BIT(0),
};

class CollisionVolume {
public:
  CollisionVolume() { type = VolumeType::Invalid; }
  ~CollisionVolume() = default;

  virtual float GetMaxExtent() const = 0;
  virtual Maths::Vector3
  GetBottomPoint(const NCL::CSC8503::Transform &transform) const = 0;

  CollisionVolume &SetTrigger(bool state = true) {
    if (state) {
      flags.set(VolumeFlags::Trigger);
    } else {
      flags.clear(VolumeFlags::Trigger);
    }

    return *this;
  }

  bool isTrigger() const {
    if (flags.has(VolumeFlags::Trigger))
      return true;
  }

  VolumeType type;
  Bitflag<VolumeFlags> flags = Bitflag<VolumeFlags>(0);
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
    default:
      typeStr = "Invalid";
      break;
    }
    return fmt::formatter<std::string_view>::format(typeStr, ctx);
  }
};
