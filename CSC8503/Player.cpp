#include "Player.h"

#include "Bitflag.h"
#include "physics/PhysicsObject.h"

namespace {
constexpr float ORIENTATION_DELTA_SCALE =
    std::numeric_limits<uint16_t>::max() / 360.f;
} // namespace

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

void Player::ClientInput(float dt) {
  auto &phys = *GetPhysicsObject();

  float pitch = camera.GetPitch();
  float yaw = camera.GetYaw();

  pitch -= controller->GetNamedAxis("YLook");
  yaw -= controller->GetNamedAxis("XLook");

  pitch = std::min(pitch, 90.0f);
  pitch = std::max(pitch, -90.0f);

  if (yaw < 0) {
    yaw += 360.0f;
  }
  if (yaw > 360.0f) {
    yaw -= 360.0f;
  }
  camera.SetYaw(yaw);
  camera.SetPitch(pitch);

  float forward = controller->GetNamedAxis("Forward");
  float sidestep = controller->GetNamedAxis("Sidestep");

  constexpr Vector3 UP{0, 1, 0};
  constexpr Vector3 RIGHT{1, 0, 0};
  constexpr Vector3 FORWARD{0, 0, -1};

  auto forwardQ = Quaternion::AxisAngleToQuaterion(UP, camera.GetYaw());

  const Vector3 forwardV = forwardQ * FORWARD;
  const Vector3 rightV = Vector::Cross(forwardV, UP);

  auto &kb = *Window::GetKeyboard();

  constexpr float baseSpeed = 1000.f;
  float speed = baseSpeed * dt;

  phys.AddForce(forwardV * forward * speed);
  phys.AddForce(rightV * sidestep * speed);

  ClientPacket input = CreateInputPacket();
  Input(dt, input, true);
}

void Player::Input(float dt, ClientPacket input, bool skipPosRot) {
  auto &phys = *GetPhysicsObject();

  Bitflag<Actions> flags(input.actions);

  if (!skipPosRot) {
    auto pos = GetTransform().GetPosition();
    pos.x = input.pos[0];
    pos.y = input.pos[1];
    pos.z = input.pos[2];
    GetTransform().SetPosition(pos);

    float xRot = static_cast<float>(input.rot[0]) / ORIENTATION_DELTA_SCALE;
    float yRot = static_cast<float>(input.rot[1]) / ORIENTATION_DELTA_SCALE;

    camera.SetYaw(yRot);
    camera.SetPitch(xRot);
  }

  constexpr Vector3 UP{0, 1, 0};
  if (flags.has(Actions::Jump) && world->IsOnGround(this)) {
    phys.ApplyLinearImpulse(UP * 50.0f);
  }

  if (pane) {
    if (flags.has(Actions::AttachFrontLeftCorner)) {
      pane->AttachCorner(Pane::Corner::FrontLeft, camera);
    }

    if (flags.has(Actions::AttachFrontRightCorner)) {
      pane->AttachCorner(Pane::Corner::FrontRight, camera);
    }

    if (flags.has(Actions::AttachBackLeftCorner)) {
      pane->AttachCorner(Pane::Corner::BackLeft, camera);
    }

    if (flags.has(Actions::AttachBackRightCorner)) {
      pane->AttachCorner(Pane::Corner::BackRight, camera);
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

ClientPacket Player::CreateInputPacket() {
  Bitflag<Player::Actions> actions;

  auto &kb = *Window::GetKeyboard();

  if (kb.KeyDown(KeyCodes::SPACE))
    actions.set(Player::Actions::Jump);

  bool shiftDown = kb.KeyDown(KeyCodes::SHIFT);
  bool ctrlDown = kb.KeyDown(KeyCodes::CONTROL);

  bool attaching = !shiftDown && !ctrlDown;
  bool extending = !shiftDown && ctrlDown;
  bool retracting = !ctrlDown && shiftDown;
  bool detaching = shiftDown && ctrlDown;

  if (kb.KeyDown(KeyCodes::NUM1) && attaching)
    actions.set(Player::Actions::AttachFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && attaching)
    actions.set(Player::Actions::AttachFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && attaching)
    actions.set(Player::Actions::AttachBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && attaching)
    actions.set(Player::Actions::AttachBackRightCorner);

  if (kb.KeyDown(KeyCodes::NUM1) && extending)
    actions.set(Player::Actions::ExtendFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && extending)
    actions.set(Player::Actions::ExtendFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && extending)
    actions.set(Player::Actions::ExtendBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && extending)
    actions.set(Player::Actions::ExtendBackRightCorner);

  if (kb.KeyDown(KeyCodes::NUM1) && retracting)
    actions.set(Player::Actions::RetractFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && retracting)
    actions.set(Player::Actions::RetractFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && retracting)
    actions.set(Player::Actions::RetractBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && retracting)
    actions.set(Player::Actions::RetractBackRightCorner);

  if (kb.KeyDown(KeyCodes::NUM1) && detaching)
    actions.set(Player::Actions::DetachFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && detaching)
    actions.set(Player::Actions::DetachFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && detaching)
    actions.set(Player::Actions::DetachBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && detaching)
    actions.set(Player::Actions::DetachBackRightCorner);

  ClientPacket p;
  p.actions = actions.flags;

  p.rot[0] = static_cast<int16_t>(camera.GetPitch() * ORIENTATION_DELTA_SCALE);
  p.rot[1] = static_cast<int16_t>(camera.GetYaw() * ORIENTATION_DELTA_SCALE);

  auto pos = GetTransform().GetPosition();

  p.pos[0] = pos.x;
  p.pos[1] = pos.y;
  p.pos[2] = pos.z;

  return p;
}

} // namespace NCL::CSC8503
