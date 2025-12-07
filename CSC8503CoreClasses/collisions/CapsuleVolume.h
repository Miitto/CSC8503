#pragma once
#include "CollisionVolume.h"

namespace NCL {
class CapsuleVolume : public CollisionVolume {
public:
  CapsuleVolume(float halfHeight, float radius) {
    this->halfHeight = halfHeight;
    this->radius = radius;
    this->type = VolumeType::Capsule;
  };
  ~CapsuleVolume() = default;

  float GetRadius() const { return radius; }

  float GetHalfHeight() const { return halfHeight; }

  float GetMaxExtent() const override { return halfHeight + radius; }

  Vector3
  GetBottomPoint(const NCL::CSC8503::Transform &transform) const override {
    Vector3 pos = transform.GetPosition();

    Vector3 a = Vector3(0, halfHeight, 0);
    Vector3 b = Vector3(0, halfHeight, 0);

    Vector3 rotatedA = transform.GetOrientation() * a;
    Vector3 rotatedB = transform.GetOrientation() * b;

    if (rotatedA.y < rotatedB.y) {
      return pos + rotatedA - Vector3(0, radius, 0);
    } else {
      return pos + rotatedB - Vector3(0, radius, 0);
    }
  }

protected:
  float radius;
  float halfHeight;
};
} // namespace NCL
