#pragma once
#include "Constraint.h"

namespace NCL {
namespace CSC8503 {
class GameObject;

class TiedConstraint : public Constraint {
public:
  TiedConstraint(GameObject *a, GameObject *b, float d);
  ~TiedConstraint() = default;

  void UpdateConstraint(float dt) override;

protected:
  GameObject *objectA;
  GameObject *objectB;

  float distance;
};
} // namespace CSC8503
} // namespace NCL