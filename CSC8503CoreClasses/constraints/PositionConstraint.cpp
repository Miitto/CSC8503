#include "PositionConstraint.h"
#include "GameObject.h"
#include "physics/PhysicsObject.h"

using namespace NCL;
using namespace Maths;
using namespace CSC8503;

PositionConstraint::PositionConstraint(GameObject *a, GameObject *b, float d)
    : objectA(a), objectB(b), distance(d) {}

// a simple constraint that stops objects from being more than <distance> away
// from each other...this would be all we need to simulate a rope, or a ragdoll
void PositionConstraint::UpdateConstraint(float dt) {
  auto relPos = objectA->GetTransform().GetPosition() -
                objectB->GetTransform().GetPosition();

  float currentDistance = Vector::Length(relPos);

  float offset = distance - currentDistance;

  if (abs(offset) <= 0.0f) {
    return;
  }

  auto offsetDir = Vector::Normalise(relPos);

  auto physA = objectA->GetPhysicsObject();
  auto physB = objectB->GetPhysicsObject();

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
}
