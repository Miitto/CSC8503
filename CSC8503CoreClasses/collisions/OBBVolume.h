#pragma once
#include "CollisionVolume.h"

namespace NCL {
class OBBVolume : public CollisionVolume {
public:
  OBBVolume(const Maths::Vector3 &halfDims) {
    type = VolumeType::OBB;
    halfSizes = halfDims;
  }
  ~OBBVolume() = default;

  float GetMaxExtent() const override {
    return Maths::Vector::GetAbsMaxElement(halfSizes) * 1.4f;
  }

  Maths::Vector3 GetHalfDimensions() const { return halfSizes; }

  Maths::Vector3
  GetBottomPoint(const NCL::CSC8503::Transform &transform) const override {
    // TODO: Do rotation
    return transform.GetPosition() - Maths::Vector3(0, halfSizes.y, 0);
  }

protected:
  Maths::Vector3 halfSizes;
};
} // namespace NCL
