#include "ClientGame.h"
#include "GameWorld.h"
#include "Window.h"
#include "networking/GameClient.h"
#include "networking/GameServer.h"
#include "networking/NetworkObject.h"
#include <array>
#include <levels.h>
#include <logging/log.h>

using namespace NCL;
using namespace CSC8503;

ClientGame::ClientGame(GameWorld &gameWorld,
                       GameTechRendererInterface &renderer,
                       PhysicsSystem &physics)
    : NetworkedGame(gameWorld, renderer, physics) {
  NetworkBase::Initialise();
}

void ClientGame::SetupPacketHandlers() {
  constexpr std::array<uint16_t, 9> handledMessages = {
      BasicNetworkMessages::Full_State,
      BasicNetworkMessages::Delta_State,
      BasicNetworkMessages::Ping_Response,
      BasicNetworkMessages::Player_Connected,
      BasicNetworkMessages::Player_Disconnected,
      BasicNetworkMessages::Shutdown,
      BasicNetworkMessages::Hello,
      BasicNetworkMessages::LevelChange,
      BasicNetworkMessages::PlayerState,
  };

  for (auto msgID : handledMessages) {
    net->RegisterPacketHandler(msgID, this);
  }
}

void ClientGame::Disconnect() {
  if (net) {
    NET_DEBUG("Client sending shutdown");
    net->SendPacket(GamePacket(BasicNetworkMessages::Shutdown));
    net->UpdateClient(); // Need to run an update to send the packet
    net = std::nullopt;
  }
}

void ClientGame::InitServer(uint16_t port, int maxClients) {
  serverNet.emplace(port, maxClients, *this);
}

void ClientGame::ShutdownServer() { serverNet = std::nullopt; }

void ClientGame::NetworkUpdate(float dt) {
  if (serverNet) {
    serverNet->Update(dt, world);
    if (player)
      serverNet->SendGlobalPacket(player->CreateInputPacket());
  }

  if (!net)
    return;

  if (!serverNet) {
    if (player)
      net->SendPacket(player->CreateInputPacket());
  }
}

void ClientGame::PingCheck(float dt) {
  if (pingInfo) {
    pingInfo->timeSinceLastPing += dt;

    constexpr float PING_TIMEOUT = 3.0f;

    if (pingInfo->timeSinceLastPing >= PING_TIMEOUT) {
      if (pingInfo->attemptsLeft > 0) {
        net->SendPacket(*pingInfo->packet);
        pingInfo->lastPingSentTime = std::chrono::high_resolution_clock::now();
        pingInfo->timeSinceLastPing = 0.0f;
        pingInfo->attemptsLeft--;
      } else {
        NET_INFO("Connection to server lost: No ping response.");
        pingInfo->cb(ConnectionFailure::NoPingResponse);
        Disconnect();
        pingInfo.reset();
      }
    }
  }
}

void ClientGame::SelectLevel(Level level) {
  switch (GetServerState()) {
  case ServerState::Host:
    serverNet->OnLevelUpdate(level);
    [[fallthrough]];
  case ServerState::Singleplayer:
    StartLevel(level);
    break;
  case ServerState::Client:
    LevelChangePacket packet(static_cast<uint16_t>(level));
    net->SendPacket(packet);
    break;
  }
}

void ClientGame::StartLevel(Level level) {
  switch (level) {
  case Level::One:
    InitLvlOne();
    break;
  case Level::Two:
    InitLvlTwo();
    break;
  default:
    NET_WARN("Unknown level requested: {}", static_cast<int>(level));
    return;
  }

  if (!serverNet) {
    player = SpawnPlayer(ourPlayerId);

    if (net) {
      for (auto id : connectedPlayers) {
        if (id != ourPlayerId) {
          SpawnPlayer(id);
        }
      }
    }
  } else {
    serverNet->OnLevelUpdate(level);
    player = serverNet->SpawnHostPlayer();
  }

  world.SetMainCamera(&player->GetCamera());

  for (auto &obj : world) {
    NCL::CSC8503::NetworkObject *netObj = obj->GetNetworkObject();
    if (netObj && std::find(networkObjects.begin(), networkObjects.end(),
                            netObj) == networkObjects.end()) {
      DEBUG("Syncing object {}", obj->GetName());
      networkObjects.push_back(netObj);
    }
  }
}

void ClientGame::EndLevel() {
  Level nextLevel = Level::One;

  switch (currentLevel) {
  case Level::One:
    nextLevel = Level::Two;
    break;
  case Level::Two:
    nextLevel = Level::One;
    break;
  }

  if (serverNet) {
    TutorialGame::EndLevel();
    serverNet->OnLevelEnd();
    serverNet->OnLevelUpdate(nextLevel);
  } else if (!net) {
    TutorialGame::EndLevel();
    SelectLevel(nextLevel);
  }
}

void ClientGame::UpdateGame(float dt) {
  if (net) {
    net->UpdateClient();
  }

  if (serverNet) {
    serverNet->UpdateServer();
  }

  if (player)
    player->ClientInput(dt);

  NetworkedGame::UpdateGame(dt);

  if (net) {
    net->UpdateClient();
  }

  if (serverNet) {
    serverNet->UpdateServer();
  }
}

void ClientGame::ReceivePacket(GamePacketType type, GamePacket *payload,
                               int source) {
  int packetId = -1;
  switch (type.type) {
  case BasicNetworkMessages::Full_State: {
    auto fs = GamePacket::as<FullPacket>(payload);
    lastFullSync = fs->fullState.stateID;
    packetId = lastFullSync;
    break;
  }
  case BasicNetworkMessages::Delta_State: {
    auto ds = GamePacket::as<DeltaPacket>(payload);
    packetId = ds->fullID;
    break;
  }
  case BasicNetworkMessages::Ping_Response: {
    NET_DEBUG("Received ping response from server.");
    if (pingInfo && pingInfo->packet->type == BasicNetworkMessages::Ping) {
      pingInfo->cb(ConnectionFailure::None);
      pingInfo = std::nullopt;
    }
    break;
  }
  case BasicNetworkMessages::Player_Connected: {
    auto pc = GamePacket::as<PlayerConnectedPacket>(payload);
    NET_INFO("Player {} has connected.", pc->id);
    connectedPlayers.push_back(pc->id);
    if (!serverNet) {
      SpawnPlayer(pc->id);
    }
    break;
  }
  case BasicNetworkMessages::Player_Disconnected: {
    auto pd = GamePacket::as<PlayerDisconnectedPacket>(payload);
    NET_INFO("Player {} has disconnected.", pd->id);
    connectedPlayers.erase(
        std::remove(connectedPlayers.begin(), connectedPlayers.end(), pd->id),
        connectedPlayers.end());
    RemovePlayer(pd->id);
    break;
  }
  case BasicNetworkMessages::Shutdown: {
    NET_INFO("Server has shutdown the connection.");
    break;
  }
  case BasicNetworkMessages::Hello: {
    NET_DEBUG("Received hello from server.");
    auto helloPacket = GamePacket::as<HelloPacket>(payload);
    ourPlayerId = helloPacket->id;
    if (pingInfo && pingInfo->packet->type == BasicNetworkMessages::Hello) {
      pingInfo->cb(ConnectionFailure::None);
      pingInfo = std::nullopt;
    }
    break;
  }
  case BasicNetworkMessages::LevelChange: {
    NET_DEBUG("Received level change from server.");
    auto packet = GamePacket::as<LevelChangePacket>(payload);
    auto level = static_cast<Level>(packet->level);
    NET_INFO("Changing to level {}", level);
    StartLevel(level);
    break;
  }
  case BasicNetworkMessages::PlayerState: {
    auto &playerPacket = *GamePacket::as<ClientPacket>(payload);

    if (playerPacket.playerId == ourPlayerId) {
      // Ignore our own packets
      break;
    }

    Player *player =
        reinterpret_cast<Player *>(world.GetPlayer(playerPacket.playerId));
    if (player) {
      float deltaTime = player->GetInputDeltaTime();
      player->Input(deltaTime, playerPacket);
    }

    break;
  }
  }

  if (NCL::CSC8503::NetworkObject::wantsPacket(*payload)) {
    NET_ASSERT(packetId != -1, "Received network state packet without method "
                               "of extracting packetID for NetworkObject.");
    for (auto i : networkObjects) {
      if (i->ReadPacket(*payload)) {
        net->SendPacket(AckPacket(packetId));
        break;
      }
    }
  }
}
