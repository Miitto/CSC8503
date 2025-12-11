#pragma once

#include "Camera.h"
#include "Debug.h"
#include "GameObject.h"
#include "GamePlayer.h"
#include "GameWorld.h"
#include "constraints/OffsetTiedConstraint.h"
#include "networking/NetworkObject.h"
#include "physics/PhysicsObject.h"

class Pane : public NCL::CSC8503::GameObject {
public:
  enum class Corner { FrontLeft, FrontRight, BackLeft, BackRight };
  Pane(NCL::CSC8503::GameWorld *world, GameObject *player)
      : GameObject("Pane"), world(world), player(player) {
    GetTags().set(Tag::Pane);

    // -2 since no client will get it. -1 may be the host client
    networkObject = new NCL::CSC8503::NetworkObject(*this, -2);
  }

  void Update(float dt) override {
    if (GetTransform().GetPosition().y < -50.0f) {
      Reset();
    }

    const NCL::CSC8503::OffsetTiedConstraint *constraints[4] = {
        ropes.fl.constraint, ropes.fr.constraint, ropes.bl.constraint,
        ropes.br.constraint};

    const NCL::Maths::Vector4 colors[4] = {
        NCL::Maths::Vector4(1, 0, 0, 1), NCL::Maths::Vector4(0, 1, 0, 1),
        NCL::Maths::Vector4(0, 0, 1, 1), NCL::Maths::Vector4(1, 1, 0, 1)};

    for (int i = 0; i < 4; ++i) {
      const NCL::CSC8503::OffsetTiedConstraint *constraint = constraints[i];
      if (constraint && constraint->IsActive()) {
        Vector4 color = colors[i];

        float constraintDistance = constraints[i]->GetDistance();
        Vector3 aPos = constraints[i]->GetAAttachPos();
        Vector3 bPos = constraints[i]->GetBAttachPos();

        Vector3 dir = bPos - aPos;
        float distSq = NCL::Maths::Vector::Dot(dir, dir);
        float constraintDistSq = constraintDistance * constraintDistance;

        float percent = distSq / constraintDistSq;

        color.w *= std::clamp(percent, 0.0f, 1.0f);

        NCL::Debug::DrawLine(aPos, bPos, color);
      }
    }
  }

  void OnCollisionBegin(NCL::CSC8503::GameObject *otherObject) override {
    Vector3 relativeVel = otherObject->GetPhysicsObject()->GetLinearVelocity() -
                          GetPhysicsObject()->GetLinearVelocity();

    constexpr float collisionThreshold = 25.0f;

    float mag = Vector::Dot(relativeVel, relativeVel);

    if (otherObject->GetPhysicsObject()->GetInverseMass() == 0.f &&
        mag > collisionThreshold) {
      Reset();
    }
  }

  void Reset() override {
    GameObject::Reset();
    ropes.fl.constraint->deactivate();
    ropes.fr.constraint->deactivate();
    ropes.bl.constraint->deactivate();
    ropes.br.constraint->deactivate();

    GetPhysicsObject()->SetAxisLocks(
        NCL::CSC8503::PhysicsObject::AxisLock::LinearX |
        NCL::CSC8503::PhysicsObject::AxisLock::LinearY |
        NCL::CSC8503::PhysicsObject::AxisLock::LinearZ);
  }

  void AttachCorner(Corner currentCorner, NCL::Camera &cam) {
    GetPhysicsObject()->SetAxisLocks(0);

    std::vector<NCL::CSC8503::GameObject *> ignores;
    for (auto &p : world->GetPlayerRange()) {
      GameObject *player = p.second;
      ignores.push_back(player);
    }
    ignores.push_back(this);

    Quaternion camRot =
        Quaternion::EulerAnglesToQuaternion(cam.GetPitch(), cam.GetYaw(), 0.f);
    auto forward = camRot * NCL::Maths::Vector3(0, 0, -1);
    Ray ray(cam.GetPosition(), forward);

    RayCollision closestCollision;
    if (world->Raycast(ray, closestCollision, std::nullopt, ignores)) {
      auto node =
          static_cast<NCL::CSC8503::GameObject *>(closestCollision.node);
      auto offset =
          closestCollision.collidedAt - node->GetTransform().GetPosition();

      NCL::CSC8503::OffsetTiedConstraint *toActivate =
          GetRope(currentCorner).constraint;
      NCL::Maths::Vector3 pos =
          GetTransform().GetPosition() +
          (GetTransform().GetOrientation() * GetCornerOffset(currentCorner));

      auto rel = pos - closestCollision.collidedAt;
      auto dist = NCL::Maths::Vector::Length(rel);

      toActivate->SetObjB({node, offset});
      toActivate->SetDistance(dist);
      toActivate->activate();
    }
  }

  void ExtendCorner(Corner currentCorner, float dt) {
    NCL::CSC8503::OffsetTiedConstraint *toActivate =
        GetRope(currentCorner).constraint;

    constexpr float extendSpeed = 5.0f;
    float newDist = toActivate->GetDistance() + extendSpeed * dt;
    toActivate->SetDistance(newDist);
  }

  void RetractCorner(Corner currentCorner, float dt) {
    NCL::CSC8503::OffsetTiedConstraint *toDeactivate =
        GetRope(currentCorner).constraint;
    constexpr float retractSpeed = 5.0f;
    float newDist = toDeactivate->GetDistance() - retractSpeed * dt;

    toDeactivate->SetDistance(newDist);
  }

  void DetachCorner(Corner currentCorner) {
    Rope rope = GetRope(currentCorner);
    rope.constraint->deactivate();
  }

  void SetupConstraints(NCL::CSC8503::GameWorld &world) {
    auto scale = GetTransform().GetScale();

    ropes.fl.constraint = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(-scale.x / 2, 0.0f, -scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(-0.5f, 5.0f, -0.5f)}, 0.0f);

    ropes.fr.constraint = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(scale.x / 2, 0.0f, -scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(0.5f, 5.0f, -0.5f)}, 0.0f);

    ropes.bl.constraint = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(-scale.x / 2, 0.0f, scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(-0.5f, 5.0f, 0.5f)}, 0.0f);

    ropes.br.constraint = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(scale.x / 2, 0.0f, scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(0.5f, 5.0f, 0.5f)}, 0.0f);

    ropes.fl.constraint->deactivate();
    ropes.fr.constraint->deactivate();
    ropes.bl.constraint->deactivate();
    ropes.br.constraint->deactivate();

    world.AddConstraint(ropes.fl.constraint);
    world.AddConstraint(ropes.fr.constraint);
    world.AddConstraint(ropes.bl.constraint);
    world.AddConstraint(ropes.br.constraint);
  }

protected:
  NCL::CSC8503::GameWorld *world;
  GameObject *player;

  struct Rope {
    NCL::CSC8503::OffsetTiedConstraint *constraint = nullptr;
  };

  struct Ropes {
    Rope fl = {};
    Rope fr = {};
    Rope bl = {};
    Rope br = {};
  };

  Ropes ropes;

  Rope GetRope(Corner corner) {
    switch (corner) {
    case Corner::FrontLeft:
      return ropes.fl;
    case Corner::FrontRight:
      return ropes.fr;
    case Corner::BackLeft:
      return ropes.bl;
    case Corner::BackRight:
      return ropes.br;
    }
  }

  NCL::Maths::Vector3 GetCornerOffset(Corner corner) {
    auto scale = GetTransform().GetScale();
    switch (corner) {
    case Corner::FrontLeft:
      return NCL::Maths::Vector3(-scale.x / 2, 0.0f, -scale.z / 2);
    case Corner::FrontRight:
      return NCL::Maths::Vector3(scale.x / 2, 0.0f, -scale.z / 2);
    case Corner::BackLeft:
      return NCL::Maths::Vector3(-scale.x / 2, 0.0f, scale.z / 2);
    case Corner::BackRight:
      return NCL::Maths::Vector3(scale.x / 2, 0.0f, scale.z / 2);
    }
  }
};
