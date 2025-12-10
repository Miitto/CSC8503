#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "constraints/OffsetTiedConstraint.h"

class Pane : public NCL::CSC8503::GameObject {
public:
  enum class Corner { FrontLeft, FrontRight, BackLeft, BackRight };
  Pane(NCL::CSC8503::GameWorld *world, GameObject *player)
      : GameObject("Pane"), world(world), player(player) {
    GetTags().set(Tag::Pane);
  }

  void Update(float dt) override {
    if (GetTransform().GetPosition().y < -50.0f) {
      Reset();
    }
  }

  void Reset() override {
    GameObject::Reset();
    constraints.fl->deactivate();
    constraints.fr->deactivate();
    constraints.bl->deactivate();
    constraints.br->deactivate();
  }

  void AttachCorner(Corner currentCorner) {
    auto &cam = world->GetMainCamera();
    Quaternion camRot =
        Quaternion::EulerAnglesToQuaternion(cam.GetPitch(), cam.GetYaw(), 0.f);
    auto forward = camRot * NCL::Maths::Vector3(0, 0, -1);
    Ray ray(cam.GetPosition(), forward);

    RayCollision closestCollision;
    if (world->Raycast(ray, closestCollision, std::nullopt, player)) {
      auto node =
          static_cast<NCL::CSC8503::GameObject *>(closestCollision.node);
      auto offset =
          closestCollision.collidedAt - node->GetTransform().GetPosition();

      NCL::CSC8503::OffsetTiedConstraint *toActivate = nullptr;
      switch (currentCorner) {
      case Corner::FrontLeft:
        toActivate = constraints.fl;
        break;
      case Corner::FrontRight:
        toActivate = constraints.fr;
        break;
      case Corner::BackLeft:
        toActivate = constraints.bl;
        break;
      case Corner::BackRight:
        toActivate = constraints.br;
        break;
      }

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
    NCL::CSC8503::OffsetTiedConstraint *toActivate = nullptr;
    switch (currentCorner) {
    case Corner::FrontLeft:
      toActivate = constraints.fl;
      break;
    case Corner::FrontRight:
      toActivate = constraints.fr;
      break;
    case Corner::BackLeft:
      toActivate = constraints.bl;
      break;
    case Corner::BackRight:
      toActivate = constraints.br;
      break;
    }
    constexpr float extendSpeed = 5.0f;
    float newDist = toActivate->GetDistance() + extendSpeed * dt;
    toActivate->SetDistance(newDist);
  }

  void RetractCorner(Corner currentCorner, float dt) {
    NCL::CSC8503::OffsetTiedConstraint *toDeactivate = nullptr;
    switch (currentCorner) {
    case Corner::FrontLeft:
      toDeactivate = constraints.fl;
      break;
    case Corner::FrontRight:
      toDeactivate = constraints.fr;
      break;
    case Corner::BackLeft:
      toDeactivate = constraints.bl;
      break;
    case Corner::BackRight:
      toDeactivate = constraints.br;
      break;
    }

    constexpr float retractSpeed = 5.0f;
    float newDist = toDeactivate->GetDistance() - retractSpeed * dt;

    toDeactivate->SetDistance(newDist);
  }

  void DetachCorner(Corner currentCorner) {
    NCL::CSC8503::OffsetTiedConstraint *toDeactivate = nullptr;
    switch (currentCorner) {
    case Corner::FrontLeft:
      toDeactivate = constraints.fl;
      break;
    case Corner::FrontRight:
      toDeactivate = constraints.fr;
      break;
    case Corner::BackLeft:
      toDeactivate = constraints.bl;
      break;
    case Corner::BackRight:
      toDeactivate = constraints.br;
      break;
    }
    toDeactivate->deactivate();
  }

  void SetupConstraints(NCL::CSC8503::GameWorld &world) {
    auto scale = GetTransform().GetScale();

    constraints.fl = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(-scale.x / 2, 0.0f, -scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(-0.5f, 5.0f, -0.5f)}, 0.0f);

    constraints.fr = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(scale.x / 2, 0.0f, -scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(0.5f, 5.0f, -0.5f)}, 0.0f);

    constraints.bl = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(-scale.x / 2, 0.0f, scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(-0.5f, 5.0f, 0.5f)}, 0.0f);

    constraints.br = new NCL::CSC8503::OffsetTiedConstraint(
        {this, NCL::Maths::Vector3(scale.x / 2, 0.0f, scale.z / 2)},
        {nullptr, NCL::Maths::Vector3(0.5f, 5.0f, 0.5f)}, 0.0f);

    constraints.fl->deactivate();
    constraints.fr->deactivate();
    constraints.bl->deactivate();
    constraints.br->deactivate();

    world.AddConstraint(constraints.fl);
    world.AddConstraint(constraints.fr);
    world.AddConstraint(constraints.bl);
    world.AddConstraint(constraints.br);
  }

protected:
  NCL::CSC8503::GameWorld *world;
  GameObject *player;

  struct Constraints {
    NCL::CSC8503::OffsetTiedConstraint *fl;
    NCL::CSC8503::OffsetTiedConstraint *fr;
    NCL::CSC8503::OffsetTiedConstraint *bl;
    NCL::CSC8503::OffsetTiedConstraint *br;
  };

  Constraints constraints;

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
