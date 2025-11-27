#pragma once
#include "NetworkBase.h"

namespace NCL {
namespace CSC8503 {
class GameWorld;
class GameServer : public NetworkBase {
public:
  GameServer(uint16_t onPort, int maxClients);
  ~GameServer();

  bool Initialise();
  void Shutdown();

  void SetGameWorld(GameWorld &g);

  bool SendGlobalPacket(GamePacketType type);
  bool SendGlobalPacket(GamePacket &packet);
  bool SendGlobalPacket(GamePacket &&packet);

  virtual void UpdateServer();

protected:
  uint16_t port;
  int clientMax;
  int clientCount;
  GameWorld *gameWorld;

  int incomingDataRate;
  int outgoingDataRate;
};
} // namespace CSC8503
} // namespace NCL
