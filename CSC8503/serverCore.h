#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "logging/logger.h"
#include "networking/GameServer.h"
#include "networking/NetworkBase.h"
#include "networking/NetworkObject.h"

#include <map>
#include <vector>

namespace NCL::CSC8503 {

using ClientId = int;

struct NetworkClient {
  ENetPeer *peer;
  ClientId clientID; // Duplicate of peer ID for convenience
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

  std::map<ClientId, NetworkClient>::const_iterator begin() const {
    return clients.cbegin();
  }
  std::map<ClientId, NetworkClient>::const_iterator end() const {
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
  ServerCore(uint16_t port, int maxClients)
      : net(::NCL::CSC8503::GameServer(port, maxClients)) {
    net.RegisterPacketHandler(BasicNetworkMessages::Received_State, this);
    net.RegisterPacketHandler(BasicNetworkMessages::Hello, this);
    net.RegisterPacketHandler(BasicNetworkMessages::Shutdown, this);
    net.RegisterPacketHandler(BasicNetworkMessages::Ping, this);
  }

  ~ServerCore() { net.SendGlobalPacket(BasicNetworkMessages::Shutdown); }

  void UpdateServer() { net.UpdateServer(); }

  void Update(float dt, GameWorld &world) {
    NET_TRACE("Server Update at {}hz", 1 / dt);
    packetsToSnapshot--;
    if (packetsToSnapshot < 0) {
      --snapshotsToStateUpdate;
      BroadcastSnapshot(false, world);
      packetsToSnapshot = PACKETS_PER_SNAPSHOT;

      if (snapshotsToStateUpdate <= 0) {
        UpdateMinimumState(world);
        snapshotsToStateUpdate = SNAPSHOTS_PER_STATEUPDATE;
      }
    } else {
      BroadcastSnapshot(true, world);
    }
  }

  void ReceivePacket(GamePacketType type, GamePacket *payload,
                     int source) override {
    switch (type.type) {
    case static_cast<uint16_t>(BasicNetworkMessages::Received_State): {
      auto packet = GamePacket::as<AckPacket>(payload);
      clients.updateLastReceivedStateID(source, packet->receivedID);
      break;
    }
    case static_cast<uint16_t>(BasicNetworkMessages::Hello): {
      NET_INFO("Player {} connected.", source);
      clients.insert(NetworkClient{.peer = net.GetPeer(source),
                                   .clientID = source,
                                   .lastReceivedStateID = -1});
      net.SendGlobalPacket(PlayerConnectedPacket(source));
      net.SendPacketToClient(source, BasicNetworkMessages::Hello);
      break;
    }
    case static_cast<uint16_t>(BasicNetworkMessages::Shutdown): {
      NET_INFO("Player {} disconnected.", source);
      clients.erase(source);
      break;
    }
    case static_cast<uint16_t>(BasicNetworkMessages::Ping): {
      net.SendPacketToClient(
          source, GamePacketType(BasicNetworkMessages::Ping_Response));
      break;
    }
    }
  }

  void BroadcastSnapshot(bool deltaFrame, ::NCL::CSC8503::GameWorld &world) {
    ::std::vector<::NCL::CSC8503::GameObject *>::const_iterator first;
    ::std::vector<::NCL::CSC8503::GameObject *>::const_iterator last;

    world.GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i) {
      ::NCL::CSC8503::NetworkObject *o = (*i)->GetNetworkObject();
      if (!o) {
        continue;
      }
      // TODO - you'll need some way of determining
      // when a player has sent the server an acknowledgement
      // and store the lastID somewhere. A map between player
      // and an int could work, or it could be part of a
      // NetworkPlayer struct.
      int playerState = 0;
      GamePacket *newPacket = nullptr;
      if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
        net.SendGlobalPacket(*newPacket);
        delete newPacket;
      }
    }
  }

  void UpdateMinimumState(::NCL::CSC8503::GameWorld &world) {
    // Periodically remove old data from the server
    int minID = clients.getMinimumLastReceivedStateID();

    for (auto i : world) {
      auto *o = i->GetNetworkObject();
      if (!o) {
        continue;
      }
      o->UpdateStateHistory(minID);
    }
  }

  int GetPacketsToSnapshot() const { return packetsToSnapshot; }

  const GameServer &GetServer() const { return net; }
  const ClientDir &GetClients() const { return clients; }

protected:
  const int PACKETS_PER_SNAPSHOT = 5;
  const int SNAPSHOTS_PER_STATEUPDATE = 5;

  GameServer net;
  int packetsToSnapshot = 0;
  int snapshotsToStateUpdate = SNAPSHOTS_PER_STATEUPDATE;
  ClientDir clients;
};
} // namespace NCL::CSC8503