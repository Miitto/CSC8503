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
    ExtendFrontLeftCorner = BIT(5),
    ExtendFrontRightCorner = BIT(6),
    ExtendBackLeftCorner = BIT(7),
    ExtendBackRightCorner = BIT(8),
    RetractFrontLeftCorner = BIT(9),
    RetractFrontRightCorner = BIT(10),
    RetractBackLeftCorner = BIT(11),
    RetractBackRightCorner = BIT(12),
    DetachFrontLeftCorner = BIT(13),
    DetachFrontRightCorner = BIT(14),
    DetachBackLeftCorner = BIT(15),
    DetachBackRightCorner = BIT(16)
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

  /// @brief Directly applies client input to the player, such as movement
  /// @param dt
  void ClientInput(float dt);
  /// @brief Applies input received from the client over the network
  /// @param dt
  /// @param input
  void Input(float dt, ClientPacket input, bool skipPosRot = false) override;

  void OnCollisionBegin(GameObject *otherObject) override;

  PerspectiveCamera &GetCamera() { return camera; }

  ClientPacket CreateInputPacket();

  float GetInputDeltaTime() {
    auto now = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration<float>(now - lastInputTime).count();
    lastInputTime = now;
    return delta;
  }

protected:
  GameWorld *world;
  ::Pane *pane;
  PerspectiveCamera camera;
  Controller *controller;
  std::chrono::steady_clock::time_point lastInputTime =
      std::chrono::high_resolution_clock::now();
};
} // namespace NCL::CSC8503
