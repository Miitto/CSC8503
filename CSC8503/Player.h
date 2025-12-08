#pragma once

#include "Bitflag.h"
#include "GamePlayer.h"
#include "GameWorld.h"
#include "Pane.h"

namespace NCL::CSC8503 {
class Player : public GamePlayer {
public:
  enum class Actions : uint64_t {
    Jump = BIT(0),
    AttachFrontLeftCorner = BIT(1),
    AttachFrontRightCorner = BIT(2),
    AttachBackLeftCorner = BIT(3),
    AttachBackRightCorner = BIT(4),
  };

  using ActionFlags = Bitflag<Actions>;

  Player(int id, GameWorld *world, Pane *pane, Camera &camera)
      : GamePlayer(id), world(world), pane(pane), camera(camera) {}

  void Update(float dt) override;
  void Input(float dt, ClientPacket input) override;

protected:
  GameWorld *world;
  Pane *pane;
  Camera &camera;
};
} // namespace NCL::CSC8503
