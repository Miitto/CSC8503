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

  void StartAsServer();
  void StartAsClient(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

  void UpdateGame(float dt) override;

  void SpawnPlayer();

  void StartLevel();

  void ReceivePacket(GamePacketType type, GamePacket *payload,
                     int source) override;

  void OnPlayerCollision(NetworkPlayer *a, NetworkPlayer *b);

protected:
  void UpdateAsServer(float dt);
  void UpdateAsClient(float dt);

  void BroadcastSnapshot(bool deltaFrame);
  void UpdateMinimumState();
  std::map<int, int> stateIDs;

  GameServer *thisServer;
  GameClient *thisClient;
  float timeToNextPacket;
  int packetsToSnapshot;

  std::vector<NetworkObject *> networkObjects;

  std::map<int, GameObject *> serverPlayers;
  GameObject *localPlayer;
};
} // namespace NCL::CSC8503