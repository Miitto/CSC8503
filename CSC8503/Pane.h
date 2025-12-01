#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "Window.h"
#include "collisions/CollisionDetection.h"
#include "constraints/OffsetTiedConstraint.h"
#include "gui.h"

class Pane : public NCL::CSC8503::GameObject {
public:
  Pane(NCL::CSC8503::GameWorld *world) : GameObject("Pane"), world(world) {}

  void Update(float dt) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::C)) {
      NextCorner();
    }

    {
      auto frame = NCL::gui::Frame("Pane Controls");

      constexpr const char *cornerNames[] = {
          "None", "Front Left", "Front Right", "Back Left", "Back Right",
      };

      if (ImGui::BeginCombo("Current Corner",
                            cornerNames[static_cast<int>(currentCorner)])) {
        for (int i = 0; i <= BackRight; ++i) {
          bool isSelected = (currentCorner == static_cast<Corner>(i));
          if (ImGui::Selectable(cornerNames[i], isSelected)) {
            currentCorner = static_cast<Corner>(i);
          }
        }
        ImGui::EndCombo();
      }
    }

    if (NCL::Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
      Ray ray =
          NCL::CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

      RayCollision closestCollision;
      if (currentCorner != None &&
          world->Raycast(ray, closestCollision, 100.f, this)) {
        auto node =
            static_cast<NCL::CSC8503::GameObject *>(closestCollision.node);
        auto offset =
            closestCollision.collidedAt - node->GetTransform().GetPosition();

        NCL::CSC8503::OffsetTiedConstraint *toActivate = nullptr;
        switch (currentCorner) {
        case FrontLeft:
          toActivate = constraints.fl;
          break;
        case FrontRight:
          toActivate = constraints.fr;
          break;
        case BackLeft:
          toActivate = constraints.bl;
          break;
        case BackRight:
          toActivate = constraints.br;
          break;
        case None:
          break; // NOOP, unreachable
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

  struct Constraints {
    NCL::CSC8503::OffsetTiedConstraint *fl;
    NCL::CSC8503::OffsetTiedConstraint *fr;
    NCL::CSC8503::OffsetTiedConstraint *bl;
    NCL::CSC8503::OffsetTiedConstraint *br;
  };

  Constraints constraints;

  enum Corner { None = 0, FrontLeft, FrontRight, BackLeft, BackRight };

  NCL::Maths::Vector3 GetCornerOffset(Corner corner) {
    auto scale = GetTransform().GetScale();
    switch (corner) {
    case FrontLeft:
      return NCL::Maths::Vector3(-scale.x / 2, 0.0f, -scale.z / 2);
    case FrontRight:
      return NCL::Maths::Vector3(scale.x / 2, 0.0f, -scale.z / 2);
    case BackLeft:
      return NCL::Maths::Vector3(-scale.x / 2, 0.0f, scale.z / 2);
    case BackRight:
      return NCL::Maths::Vector3(scale.x / 2, 0.0f, scale.z / 2);
    case None:
    default:
      return NCL::Maths::Vector3(0.0f, 0.0f, 0.0f);
    }
  }

  Corner currentCorner = None;
  void NextCorner() {
    auto next = currentCorner + 1;
    if (next > BackRight) {
      next = None;
    }

    currentCorner = static_cast<Corner>(next);
  }
};
