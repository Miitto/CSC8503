
#pragma once
#include "Constraint.h"

namespace NCL::CSC8503 {
class GameObject;

class OffsetTiedConstraint : public Constraint {
public:
  struct Obj {
    GameObject *object;
    NCL::Maths::Vector3 offset;

    NCL::Maths::Vector3 GetOffsetPos() const;
  };

  OffsetTiedConstraint(Obj a, Obj b, float d)
      : objectA(a), objectB(b), distance(d) {}
  ~OffsetTiedConstraint() = default;

  void UpdateConstraint(float dt) override;

  void SetObjA(Obj a) { objectA = a; }
  void SetObjB(Obj b) { objectB = b; }

  float GetDistance() const { return distance; }
  void SetDistance(float d) { distance = d; }

  NCL::Maths::Vector3 GetAAttachPos() const { return objectA.GetOffsetPos(); }
  NCL::Maths::Vector3 GetBAttachPos() const { return objectB.GetOffsetPos(); }

protected:
  Obj objectA;
  Obj objectB;

  float distance;
};
} // namespace NCL::CSC8503
