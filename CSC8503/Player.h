#pragma once

#include "Bitflag.h"
#include "Camera.h"
#include "GamePlayer.h"
#include "GameWorld.h"
#include "KeyboardMouseController.h"
#include "Pane.h"
#include "Window.h"
#include "networking/NetworkObject.h"

namespace NCL::CSC8503 {
class Player : public GamePlayer {
public:
  enum class Actions : uint64_t {
    Jump = BIT(0),
    AttachFrontLeftCorner = BIT(1),
    AttachFrontRightCorner = BIT(2),
    AttachBackLeftCorner = BIT(3),
    AttachBackRightCorner = BIT(4),
    MoveForward = BIT(5),
    MoveBackward = BIT(6),
    MoveLeft = BIT(7),
    MoveRight = BIT(8),
    ExtendFrontLeftCorner = BIT(9),
    ExtendFrontRightCorner = BIT(10),
    ExtendBackLeftCorner = BIT(11),
    ExtendBackRightCorner = BIT(12),
    RetractFrontLeftCorner = BIT(13),
    RetractFrontRightCorner = BIT(14),
    RetractBackLeftCorner = BIT(15),
    RetractBackRightCorner = BIT(16),
    DetachFrontLeftCorner = BIT(17),
    DetachFrontRightCorner = BIT(18),
    DetachBackLeftCorner = BIT(19),
    DetachBackRightCorner = BIT(20)
  };

  using ActionFlags = Bitflag<Actions>;

  Player(int id, GameWorld *world, ::Pane *pane)
      : GamePlayer(id), world(world), pane(pane) {
    camera.SetNearPlane(0.1f);
    camera.SetFarPlane(500.0f);

    controller =
        new KeyboardMouseController(*NCL::Window::GetWindow()->GetKeyboard(),
                                    *NCL::Window::GetWindow()->GetMouse());

    camera.SetController(*controller);

    controller->MapAxis(0, "Sidestep");
    controller->MapAxis(1, "UpDown");
    controller->MapAxis(2, "Forward");

    controller->MapAxis(3, "XLook");
    controller->MapAxis(4, "YLook");

    networkObject = new NCL::CSC8503::NetworkObject(*this, id);
  }

  ~Player() { delete controller; }

  void Update(float dt) override;
  void Input(float dt, ClientPacket input) override;

  void OnCollisionBegin(GameObject *otherObject) override;

  PerspectiveCamera &GetCamera() { return camera; }

protected:
  GameWorld *world;
  ::Pane *pane;
  PerspectiveCamera camera;
  Controller *controller;
};
} // namespace NCL::CSC8503
