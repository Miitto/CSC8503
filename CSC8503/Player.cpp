#include "Player.h"

#include "Window.h"
#include "physics/PhysicsObject.h"

namespace {
constexpr Vector3 FORWARD(0, 0, -1);
constexpr Vector3 UP(0, 1, 0);
} // namespace

void Player::Update(float dt) {
  auto kb = NCL::Window::GetKeyboard();
  auto mouse = NCL::Window::GetMouse();

  auto rot = GetTransform().GetOrientation();

  auto pitchDelta = mouse->GetRelativePosition().y * -0.5f;
  auto yawDelta = mouse->GetRelativePosition().x * -0.5f;

  if (kb->KeyDown(NCL::KeyCodes::LEFT)) {
    yawDelta = 0.25f;
  }
  if (kb->KeyDown(NCL::KeyCodes::RIGHT)) {
    yawDelta = -0.25f;
  }
  if (kb->KeyDown(NCL::KeyCodes::UP)) {
    pitchDelta = 0.25f;
  }
  if (kb->KeyDown(NCL::KeyCodes::DOWN)) {
    pitchDelta = -0.25f;
  }

  pitch += pitchDelta;

  if (pitch > 89.0f) {
    pitch = 89.0f;
  }
  if (pitch < -89.0f) {
    pitch = -89.0f;
  }

  Quaternion yawQuat = Quaternion::EulerAnglesToQuaternion(0, yawDelta, 0);
  rot = (yawQuat * rot).Normalised();

  GetTransform().SetOrientation(rot);

  Vector3 forward = rot * FORWARD;
  Vector3 right = Vector::Normalise(Vector::Cross(forward, UP));

  Vector3 movement{};

  if (kb->KeyDown(NCL::KeyCodes::W)) {
    movement = movement - forward;
  }
  if (kb->KeyDown(NCL::KeyCodes::S)) {
    movement = movement + forward;
  }
  if (kb->KeyDown(NCL::KeyCodes::A)) {
    movement = movement + right;
  }
  if (kb->KeyDown(NCL::KeyCodes::D)) {
    movement = movement - right;
  }

  if (kb->KeyPressed(NCL::KeyCodes::SPACE)) {
    auto p = GetTransform().GetPosition();
    Ray r(p, -UP);
    RayCollision collision;
    if (world->Raycast(r, collision, true, this)) {
      if (collision.rayDistance < 1.15f) {
        GetPhysicsObject()->ApplyLinearImpulse(UP * 5.f);
      }
    }
  }

  GetPhysicsObject()->AddForce(Vector::Normalise(movement) * 20.0f);
}