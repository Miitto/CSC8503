

#include "OffsetTiedConstraint.h"
#include "GameObject.h"
#include "physics/PhysicsObject.h"

using namespace NCL;
using namespace Maths;
using namespace CSC8503;

Vector3 OffsetTiedConstraint::Obj::GetOffsetPos() const {
  auto pos = object->GetTransform().GetPosition();
  auto rot = object->GetTransform().GetOrientation();

  return pos + (rot * offset);
}

void OffsetTiedConstraint::UpdateConstraint(float dt) {
  if (!active)
    return;
  auto offsetPosA = objectA.GetOffsetPos();
  auto offsetPosB = objectB.GetOffsetPos();

  auto relPos = offsetPosA - offsetPosB;

  float currentDistance = Vector::Length(relPos);

  float offset = distance - currentDistance;

  if (offset >= 0.0f) {
    return;
  }

  auto offsetDir = Vector::Normalise(relPos);

  auto physA = objectA.object->GetPhysicsObject();
  auto physB = objectB.object->GetPhysicsObject();

  auto relV = physA->GetLinearVelocity() - physB->GetLinearVelocity();

  float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

  if (totalMass <= 0) {
    return;
  }

  float vDot = Vector::Dot(relV, offsetDir);

  constexpr float biasFactor = 0.01f;
  const float bias = -(biasFactor / dt) * offset;

  float lambda = -(vDot + bias) / totalMass;

  auto aJ = offsetDir * lambda;
  auto bJ = -offsetDir * lambda;

  physA->ApplyLinearImpulse(aJ);
  physB->ApplyLinearImpulse(bJ);

  auto localPosA = offsetPosA - objectA.object->GetTransform().GetPosition();
  auto localPosB = offsetPosB - objectB.object->GetTransform().GetPosition();

  physA->ApplyAngularImpulse(Vector::Cross(localPosA, aJ));
  physB->ApplyAngularImpulse(Vector::Cross(localPosB, bJ));
}
