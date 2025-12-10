#pragma once

#include "Bitflag.h"
#include "GamePlayer.h"
#include "GameWorld.h"
#include "Pane.h"
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

  Player(int id, GameWorld *world, ::Pane *pane, Camera &camera)
      : GamePlayer(id), world(world), pane(pane), camera(camera) {
    networkObject = new NCL::CSC8503::NetworkObject(*this, id);
  }

  void Update(float dt) override;
  void Input(float dt, ClientPacket input) override;

  void OnCollisionBegin(GameObject *otherObject) override;

protected:
  GameWorld *world;
  ::Pane *pane;
  Camera &camera;
};
} // namespace NCL::CSC8503
