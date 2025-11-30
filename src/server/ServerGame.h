#pragma once

#include "NetworkedGame.h"
#include "serverCore.h"

class GameServer;
class GameClient;
class NetworkPlayer;
class NetworkObject;

class ServerGame : public NCL::CSC8503::NetworkedGame {
public:
  ServerGame(NCL::CSC8503::GameWorld &gameWorld,
             NCL::CSC8503::GameTechRendererInterface &renderer,
             NCL::CSC8503::PhysicsSystem &physics);
  ~ServerGame();

  void ReceivePacket(GamePacketType type, GamePacket *payload,
                     int source) override;

  void StartLevel() override;

  void UpdateGame(float dt) override {
    net.UpdateServer();
    NCL::CSC8503::NetworkedGame::UpdateGame(dt);
  }

protected:
  virtual void NetworkUpdate(float dt) override;

  void BroadcastSnapshot(bool deltaFrame);
  void UpdateMinimumState();

  NCL::CSC8503::ServerCore net;
};