#pragma once
#include "CollisionVolume.h"

namespace NCL {
class SphereVolume : public CollisionVolume {
public:
  SphereVolume(float sphereRadius = 1.0f) {
    type = VolumeType::Sphere;
    radius = sphereRadius;
  }
  ~SphereVolume() = default;

  float GetMaxExtent() const override { return radius; }

  float GetRadius() const { return radius; }

protected:
  float radius;
};
} // namespace NCL
