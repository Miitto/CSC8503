#pragma once
#include "TutorialGame.h"
#include "networking/NetworkBase.h"

namespace NCL::CSC8503 {
class GameServer;
class GameClient;
class NetworkPlayer;
class NetworkObject;

class NetworkedGame : public TutorialGame, public PacketReceiver {
public:
  NetworkedGame(GameWorld &gameWorld, GameTechRendererInterface &renderer,
                PhysicsSystem &physics);
  ~NetworkedGame();

  void UpdateGame(float dt) override;

  virtual void StartLevel() = 0;

protected:
  virtual void NetworkUpdate(float dt) = 0;

  float timeToNextPacket;

  std::vector<NetworkObject *> networkObjects;
};
} // namespace NCL::CSC8503