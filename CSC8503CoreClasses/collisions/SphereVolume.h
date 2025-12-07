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

  Vector3
  GetBottomPoint(const NCL::CSC8503::Transform &transform) const override {
    return transform.GetPosition() - Vector3(0, radius, 0);
  }

protected:
  float radius;
};
} // namespace NCL
