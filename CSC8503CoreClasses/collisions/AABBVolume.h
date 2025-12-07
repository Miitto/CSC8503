#pragma once
#include "CollisionVolume.h"

namespace NCL {
using namespace NCL::Maths;
class AABBVolume : public CollisionVolume {
public:
  AABBVolume(const Vector3 &halfDims) {
    type = VolumeType::AABB;
    halfSizes = halfDims;
  }
  ~AABBVolume() = default;

  float GetMaxExtent() const override {
    return Maths::Vector::GetAbsMaxElement(halfSizes) * 1.4f;
  }

  Vector3
  GetBottomPoint(const NCL::CSC8503::Transform &transform) const override {
    return transform.GetPosition() - Vector3(0, halfSizes.y, 0);
  }

  Vector3 GetHalfDimensions() const { return halfSizes; }

protected:
  Vector3 halfSizes;
};
} // namespace NCL
