#include "Player.h"

#include "Bitflag.h"
#include "physics/PhysicsObject.h"

namespace NCL::CSC8503 {

void Player::Update(float dt) {
  if (GetTransform().GetPosition().y < -50.0f) {
    Reset();
  }

  auto pos = GetTransform().GetPosition();
  camera.SetPosition(pos + Vector3(0, 1.25, 0));
  auto rot = Quaternion::EulerAnglesToQuaternion(0, camera.GetYaw(), 0);
  GetTransform().SetOrientation(rot);
}

void Player::Input(float dt, ClientPacket input) {
  constexpr Vector3 UP{0, 1, 0};
  constexpr Vector3 RIGHT{1, 0, 0};
  constexpr Vector3 FORWARD{0, 0, -1};

  auto forwardQ = Quaternion::AxisAngleToQuaterion(UP, camera.GetYaw());

  const Vector3 forward = forwardQ * FORWARD;
  const Vector3 right = Vector::Cross(forward, UP);

  auto &phys = *GetPhysicsObject();

  Bitflag<Actions> flags(input.actions);

  constexpr float speed = 10.f;

  if (flags.has(Actions::MoveForward)) {
    phys.AddForce(forward * speed);
  }

  if (flags.has(Actions::MoveBackward)) {
    phys.AddForce(-forward * speed);
  }

  if (flags.has(Actions::MoveLeft)) {
    phys.AddForce(-right * speed);
  }

  if (flags.has(Actions::MoveRight)) {
    phys.AddForce(right * speed);
  }

  if (flags.has(Actions::Jump) && world->IsOnGround(this)) {
    phys.ApplyLinearImpulse(UP * 50.0f);
  }

  if (pane) {
    if (flags.has(Actions::AttachFrontLeftCorner)) {
      pane->AttachCorner(Pane::Corner::FrontLeft);
    }

    if (flags.has(Actions::AttachFrontRightCorner)) {
      pane->AttachCorner(Pane::Corner::FrontRight);
    }

    if (flags.has(Actions::AttachBackLeftCorner)) {
      pane->AttachCorner(Pane::Corner::BackLeft);
    }

    if (flags.has(Actions::AttachBackRightCorner)) {
      pane->AttachCorner(Pane::Corner::BackRight);
    }

    if (flags.has(Actions::ExtendFrontLeftCorner)) {
      pane->ExtendCorner(Pane::Corner::FrontLeft, dt);
    }

    if (flags.has(Actions::ExtendFrontRightCorner)) {
      pane->ExtendCorner(Pane::Corner::FrontRight, dt);
    }

    if (flags.has(Actions::ExtendBackLeftCorner)) {
      pane->ExtendCorner(Pane::Corner::BackLeft, dt);
    }

    if (flags.has(Actions::ExtendBackRightCorner)) {
      pane->ExtendCorner(Pane::Corner::BackRight, dt);
    }

    if (flags.has(Actions::RetractFrontLeftCorner)) {
      pane->RetractCorner(Pane::Corner::FrontLeft, dt);
    }

    if (flags.has(Actions::RetractFrontRightCorner)) {
      pane->RetractCorner(Pane::Corner::FrontRight, dt);
    }

    if (flags.has(Actions::RetractBackLeftCorner)) {
      pane->RetractCorner(Pane::Corner::BackLeft, dt);
    }

    if (flags.has(Actions::RetractBackRightCorner)) {
      pane->RetractCorner(Pane::Corner::BackRight, dt);
    }

    if (flags.has(Actions::DetachFrontLeftCorner)) {
      pane->DetachCorner(Pane::Corner::FrontLeft);
    }

    if (flags.has(Actions::DetachFrontRightCorner)) {
      pane->DetachCorner(Pane::Corner::FrontRight);
    }

    if (flags.has(Actions::DetachBackLeftCorner)) {
      pane->DetachCorner(Pane::Corner::BackLeft);
    }

    if (flags.has(Actions::DetachBackRightCorner)) {
      pane->DetachCorner(Pane::Corner::BackRight);
    }
  }
}

void Player::OnCollisionBegin(GameObject *otherObject) {
  if (otherObject->GetTags().has(Tag::Enemy)) {
    Reset();
  }
}
} // namespace NCL::CSC8503
