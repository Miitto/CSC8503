#include "serverCore.h"

#include "GameObject.h"
#include "GameWorld.h"
#include "logging/log.h"
#include "logging/logger.h"
#include "networking/GameServer.h"
#include "networking/NetworkBase.h"
#include "networking/NetworkObject.h"

#include <levels.h>
#include <map>
#include <vector>

namespace NCL::CSC8503 {
ServerCore::ServerCore(uint16_t port, int maxClients)
    : net(::NCL::CSC8503::GameServer(port, maxClients)) {
  LOG("Server Core starting on port {}", port);
  net.RegisterPacketHandler(BasicNetworkMessages::Received_State, this);
  net.RegisterPacketHandler(BasicNetworkMessages::Hello, this);
  net.RegisterPacketHandler(BasicNetworkMessages::Shutdown, this);
  net.RegisterPacketHandler(BasicNetworkMessages::Ping, this);
}

void ServerCore::Update(float dt, GameWorld &world) {
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

void ServerCore::ReceivePacket(GamePacketType type, GamePacket *payload,
                               int source) {
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
    net.SendPacketToClient(source, HelloPacket(source));
    net.SendPacketToClient(
        source, LevelChangePacket(static_cast<uint8_t>(currentLevel)));
    break;
  }
  case static_cast<uint16_t>(BasicNetworkMessages::Shutdown): {
    NET_INFO("Player {} disconnected.", source);
    clients.erase(source);
    break;
  }
  case static_cast<uint16_t>(BasicNetworkMessages::Ping): {
    net.SendPacketToClient(source,
                           GamePacketType(BasicNetworkMessages::Ping_Response));
    break;
  }
  }
}

void ServerCore::BroadcastSnapshot(bool deltaFrame,
                                   ::NCL::CSC8503::GameWorld &world) {
  ::std::vector<::NCL::CSC8503::GameObject *>::const_iterator first;
  ::std::vector<::NCL::CSC8503::GameObject *>::const_iterator last;

  world.GetObjectIterators(first, last);

  for (auto i = first; i != last; ++i) {
    ::NCL::CSC8503::NetworkObject *o = (*i)->GetNetworkObject();
    if (!o) {
      continue;
    }

    GamePacket *newPacket = nullptr;
    for (auto &player : clients) {
      if (o->WritePacket(&newPacket, deltaFrame,
                         player.second.lastReceivedStateID)) {
        net.SendPacketToClient(player.first, *newPacket);
      }
    }
  }
}

void ServerCore::UpdateMinimumState(::NCL::CSC8503::GameWorld &world) {
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

void ServerCore::OnLevelUpdate(Level level) {
  NET_INFO("Changing to level {}", level);
  net.SendGlobalPacket(LevelChangePacket(static_cast<uint8_t>(level)));
  currentLevel = level;
}
} // namespace NCL::CSC8503