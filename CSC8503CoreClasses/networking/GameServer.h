#pragma once
#include "NetworkBase.h"

typedef struct _ENetPeer ENetPeer;

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

  bool SendPacketToClient(int clientID, GamePacketType type);
  bool SendPacketToClient(int clientID, GamePacket &packet);
  bool SendPacketToClient(int clientID, GamePacket &&packet);

  bool SendGlobalPacket(GamePacketType type);
  bool SendGlobalPacket(GamePacket &packet);
  bool SendGlobalPacket(GamePacket &&packet);

  virtual void UpdateServer();

protected:
  ENetPeer *GetPeer(int id);

  uint16_t port;
  int clientMax;
  int clientCount;
  GameWorld *gameWorld;

  int incomingDataRate;
  int outgoingDataRate;
};
} // namespace CSC8503
} // namespace NCL
