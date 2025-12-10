#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "TutorialGame.h"
#include "logging/logger.h"
#include "networking/GameServer.h"
#include "networking/NetworkBase.h"
#include "networking/NetworkObject.h"

#include "Player.h"
#include <levels.h>
#include <map>
#include <vector>

namespace NCL::CSC8503 {

using ClientId = int;

struct NetworkClient {
  ENetPeer *peer;
  ClientId clientID; // Duplicate of peer ID for convenience
  Player *playerObj;
  int lastReceivedStateID;
};

class ClientDir {
public:
  bool contains(ClientId clientID) const {
    return clients.find(clientID) != clients.end();
  }
  NetworkClient &get(ClientId clientID) { return clients.at(clientID); }
  const NetworkClient &get(ClientId clientID) const {
    return clients.at(clientID);
  }

  std::map<ClientId, NetworkClient>::iterator find(ClientId clientID) {
    return clients.find(clientID);
  }
  std::map<ClientId, NetworkClient>::const_iterator
  find(ClientId clientID) const {
    return clients.find(clientID);
  }

  std::map<ClientId, NetworkClient>::iterator begin() {
    return clients.begin();
  }
  std::map<ClientId, NetworkClient>::iterator end() { return clients.end(); }
  std::map<ClientId, NetworkClient>::const_iterator begin() const {
    return clients.begin();
  }
  std::map<ClientId, NetworkClient>::const_iterator end() const {
    return clients.end();
  }

  std::map<ClientId, NetworkClient>::const_iterator cbegin() {
    return clients.cbegin();
  }
  std::map<ClientId, NetworkClient>::const_iterator cend() {
    return clients.cend();
  }

  ClientDir &insert(NetworkClient client) {
    clients.emplace(client.clientID, client);
    return *this;
  }

  ClientDir &erase(ClientId clientID) {
    clients.erase(clientID);
    return *this;
  }

  ClientDir &updateLastReceivedStateID(ClientId clientID, int stateID) {
    auto client = clients.find(clientID);
    if (client == clients.end()) {
      NET_ERROR("ClientDir::updateLastReceivedStateID: Client ID {} not found.",
                clientID);
      return *this;
    }
    auto lastStateID = client->second.lastReceivedStateID;
    if (stateID > lastStateID) {
      client->second.lastReceivedStateID = stateID;
    }
    return *this;
  }

  int getMinimumLastReceivedStateID() const {
    int minID = INT_MAX;
    for (const auto &i : clients) {
      minID = std::min(minID, i.second.lastReceivedStateID);
    }
    return minID;
  }

protected:
  std::map<ClientId, NetworkClient> clients = {};
};

class ServerCore : public PacketReceiver {
public:
  ServerCore(uint16_t port, int maxClients, TutorialGame &game);
  ~ServerCore() {
    net.SendGlobalPacket(GamePacket(BasicNetworkMessages::Shutdown));
  }

  void RegisterPacketHandler(GamePacketType type, PacketReceiver *receiver) {
    net.RegisterPacketHandler(type, receiver);
  }

  void UpdateServer() { net.UpdateServer(); }

  void Update(float dt, GameWorld &world);
  void ReceivePacket(GamePacketType type, GamePacket *payload,
                     int source) override;
  void BroadcastSnapshot(bool deltaFrame, ::NCL::CSC8503::GameWorld &world);
  void UpdateMinimumState(::NCL::CSC8503::GameWorld &world);
  int GetPacketsToSnapshot() const { return packetsToSnapshot; }

  const GameServer &GetServer() const { return net; }
  const ClientDir &GetClients() const { return clients; }

  void OnLevelUpdate(Level level);
  void OnLevelEnd();

  Level &GetCurrentLevel() { return currentLevel; }

  Player *SpawnHostPlayer() {
    Player *player = game.SpawnPlayer(-1);
    clients.insert(NetworkClient{.peer = nullptr,
                                 .clientID = -1,
                                 .playerObj = player,
                                 .lastReceivedStateID = -1});
    net.SendGlobalPacket(PlayerConnectedPacket(-1));

    return player;
  }

  Player *GetClientPlayer(ClientId clientID) {
    if (!clients.contains(clientID))
      return nullptr;
    return clients.get(clientID).playerObj;
  }

protected:
  const int PACKETS_PER_SNAPSHOT = 5;
  const int SNAPSHOTS_PER_STATEUPDATE = 5;

  GameServer net;
  int packetsToSnapshot = 0;
  int snapshotsToStateUpdate = SNAPSHOTS_PER_STATEUPDATE;
  ClientDir clients;

  Level currentLevel = Level::Default;

  TutorialGame &game;
};
} // namespace NCL::CSC8503