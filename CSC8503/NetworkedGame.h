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

  void Clear() override {
    networkObjects.clear();
    TutorialGame::Clear();
  }

protected:
  virtual void NetworkUpdate(float dt) = 0;

  float timeToNextPacket;
  float timeSinceLastNetUpdate = 0.0f;

  std::vector<NetworkObject *> networkObjects;
};
} // namespace NCL::CSC8503