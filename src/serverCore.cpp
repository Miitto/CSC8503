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
ServerCore::ServerCore(uint16_t port, int maxClients, NetworkedGame &game)
    : GameServer(port, maxClients), game(game) {
  LOG("Server Core starting on port {}", port);
  RegisterPacketHandler(GamePacketType(BasicNetworkMessages::PlayerState),
                        this);
  RegisterPacketHandler(BasicNetworkMessages::Received_State, this);
  RegisterPacketHandler(BasicNetworkMessages::Hello, this);
  RegisterPacketHandler(BasicNetworkMessages::Ping, this);
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
  case BasicNetworkMessages::PlayerState: {
    if (!clients.contains(source))
      break;
    ClientPacket &input = ClientPacket::as<ClientPacket>(*payload);
    input.playerId = source;

    SendGlobalPacket(input);

    auto &client = clients.get(source);
    if (client.playerObj) {
      auto now = std::chrono::high_resolution_clock::now();
      auto deltaTime =
          std::chrono::duration<float>(now - client.lastInputTime).count();
      client.lastInputTime = now;
      client.playerObj->Input(deltaTime, input);
    }
    break;
  }
  case BasicNetworkMessages::Received_State: {
    auto packet = GamePacket::as<AckPacket>(payload);
    clients.updateLastReceivedStateID(source, packet->receivedID);
    break;
  }
  case BasicNetworkMessages::Hello: {
    if (clients.contains(source)) {
      NET_WARN("Player {} attempted to connect but is already connected.",
               source);
      break;
    }
    NET_INFO("Player {} connected.", source);
    Player *player = game.SpawnPlayer(source);
    clients.insert(NetworkClient{.peer = GetPeer(source),
                                 .clientID = source,
                                 .playerObj = player,
                                 .lastReceivedStateID = -1});
    SendGlobalPacket(PlayerConnectedPacket(source));
    SendPacketToClient(source, HelloPacket(source));
    SendPacketToClient(source,
                       LevelChangePacket(static_cast<uint8_t>(currentLevel)));

    for (auto &client : clients) {
      if (client.first == source) {
        continue;
      }
      SendPacketToClient(source, PlayerConnectedPacket(client.first));
    }

    break;
  }
  case static_cast<uint16_t>(BasicNetworkMessages::Ping): {
    SendPacketToClient(source,
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
      if (player.first == -1) {
        // skip host player
        continue;
      }

      if (o->WritePacket(&newPacket, deltaFrame,
                         player.second.lastReceivedStateID)) {
        SendPacketToClient(player.first, *newPacket);
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
  SendGlobalPacket(LevelChangePacket(static_cast<uint8_t>(level)));
  currentLevel = level;

  for (auto &player : clients) {
    if (player.first == -1)
      continue;
    player.second.playerObj = game.SpawnPlayer(player.first);
    player.second.lastInputTime = std::chrono::high_resolution_clock::now();
  }
}

void ServerCore::OnLevelEnd() {
  NET_INFO("Level ended on server.");
  for (auto &player : clients) {
    player.second.playerObj = nullptr;
    player.second.lastReceivedStateID = -1;
  }
}

} // namespace NCL::CSC8503