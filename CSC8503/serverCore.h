#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "networking/GameServer.h"
#include "networking/NetworkBase.h"
#include "networking/NetworkObject.h"

#include <map>
#include <vector>

namespace NCL::CSC8503 {
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

  void Update(float dt, GameWorld &world) {
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
      auto curr = stateIDs[source];
      if (packet->receivedID > curr)
        stateIDs[source] = packet->receivedID;
      break;
    }
    case static_cast<uint16_t>(BasicNetworkMessages::Hello): {
      NET_LOG("Client {} connected.", source);
      stateIDs.emplace(source, 0);
      break;
    }
    case static_cast<uint16_t>(BasicNetworkMessages::Shutdown): {
      NET_LOG("Client {} disconnected.", source);
      stateIDs.erase(source);
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
    int minID = INT_MAX;
    int maxID = 0; // we could use this to see if a player is lagging behind?
    for (auto i : stateIDs) {
      minID = std::min(minID, i.second);
      maxID = std::max(maxID, i.second);
    }
    // every client has acknowledged reaching at least state minID
    // so we can get rid of any old states!
    std::vector<::NCL::CSC8503::GameObject *>::const_iterator first;
    std::vector<::NCL::CSC8503::GameObject *>::const_iterator last;
    world.GetObjectIterators(first, last);
    for (auto i = first; i != last; ++i) {
      auto *o = (*i)->GetNetworkObject();
      if (!o) {
        continue;
      }
      o->UpdateStateHistory(
          minID); // clear out old states so they arent taking up memory...
    }
  }

  int GetClientStateID(int clientID) {
    auto i = stateIDs.find(clientID);
    if (i != stateIDs.end()) {
      return i->second;
    }
    return -1;
  }

  int GetPacketsToSnapshot() const { return packetsToSnapshot; }

  const GameServer &GetServer() const { return net; }

protected:
  const int PACKETS_PER_SNAPSHOT = 5;
  const int SNAPSHOTS_PER_STATEUPDATE = 5;

  GameServer net;
  int packetsToSnapshot = 0;
  int snapshotsToStateUpdate = SNAPSHOTS_PER_STATEUPDATE;
  std::map<int, int> stateIDs;
};
} // namespace NCL::CSC8503