#include "Player.h"

#include "Window.h"
#include "physics/PhysicsObject.h"

namespace NCL::CSC8503 {
Player::Player(Camera &inCamera) : camera(inCamera) {}

void Player::Update(float dt) {
  auto pos = GetTransform().GetPosition();
  camera.SetPosition(pos + Vector3(0, 5, 0));
  auto rot = Quaternion::EulerAnglesToQuaternion(0, camera.GetYaw(), 0);
  GetTransform().SetOrientation(rot);

  constexpr Vector3 UP{0, 1, 0};
  constexpr Vector3 RIGHT{1, 0, 0};
  constexpr Vector3 FORWARD{0, 0, -1};

  auto forwardQ = Quaternion::AxisAngleToQuaterion(UP, camera.GetYaw());

  const Vector3 forward = forwardQ * FORWARD;
  const Vector3 right = Vector::Cross(forward, UP);

  auto &phys = *GetPhysicsObject();

  if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
    phys.AddForce(forward * 50.0f);
  }
  if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
    phys.AddForce(-forward * 50.0f);
  }
  if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
    phys.AddForce(-right * 50.0f);
  }
  if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
    phys.AddForce(right * 50.0f);
  }
}
} // namespace NCL::CSC8503