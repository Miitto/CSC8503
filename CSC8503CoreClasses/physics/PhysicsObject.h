#pragma once

#include "macros.h"

using namespace NCL::Maths;

namespace NCL {
class CollisionVolume;

namespace CSC8503 {
class Transform;

class PhysicsObject {
public:
  enum AxisLock : uint8_t {
    LinearX = BIT(1),
    LinearY = BIT(2),
    LinearZ = BIT(3),
    AngularX = BIT(4),
    AngularY = BIT(5),
    AngularZ = BIT(6)
  };

  PhysicsObject(Transform &parentTransform,
                const CollisionVolume *parentVolume);
  ~PhysicsObject() = default;

  void ClampVelocities();

  void Reset() {
    linearVelocity = {};
    angularVelocity = {};
    force = {};
    torque = {};
  }

  Vector3 GetLinearVelocity() const { return linearVelocity; }
  std::optional<float> &GetMaxLinearVelocity() { return maxLinearVelocity; }

  Vector3 GetAngularVelocity() const { return angularVelocity; }
  std::optional<float> &GetMaxAngularVelocity() { return maxAngularVelocity; }

  Vector3 GetTorque() const { return torque; }

  Vector3 GetForce() const { return force; }

  void SetInverseMass(float invMass) { inverseMass = invMass; }

  float GetInverseMass() const { return inverseMass; }

  void ApplyAngularImpulse(const Vector3 &force);
  void ApplyLinearImpulse(const Vector3 &force);

  void AddForce(const Vector3 &force);

  void AddForceAtPosition(const Vector3 &force, const Vector3 &position);

  void AddTorque(const Vector3 &torque);

  void ClearForces();

  void SetLinearVelocity(const Vector3 &v) { linearVelocity = v; }

  void SetAngularVelocity(const Vector3 &v) { angularVelocity = v; }

  void InitCubeInertia();
  void InitSphereInertia();

  void UpdateInertiaTensor();

  Matrix3 GetInertiaTensor() const { return inverseInteriaTensor; }

protected:
  const CollisionVolume *volume;
  Transform &transform;

  float inverseMass;
  float elasticity;
  float friction;

  uint8_t axisLocks;

  // linear stuff
  Vector3 linearVelocity;
  Vector3 force;
  std::optional<float> maxLinearVelocity;

  // angular stuff
  Vector3 angularVelocity;
  Vector3 torque;
  Vector3 inverseInertia;
  Matrix3 inverseInteriaTensor;
  std::optional<float> maxAngularVelocity;
};
} // namespace CSC8503
} // namespace NCL
