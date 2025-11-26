#pragma once

#include "macros.h"

namespace NCL {
enum class VolumeType {
  AABB = BIT(1),
  OBB = BIT(2),
  Sphere = BIT(3),
  Mesh = BIT(4),
  Capsule = BIT(5),
  Compound = BIT(6),
  Invalid = BIT(7)
};

class CollisionVolume {
public:
  CollisionVolume() { type = VolumeType::Invalid; }
  ~CollisionVolume() = default;

  virtual float GetMaxExtent() const = 0;

  VolumeType type;
};
} // namespace NCL