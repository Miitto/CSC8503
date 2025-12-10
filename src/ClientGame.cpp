#include "ClientGame.h"
#include "GameWorld.h"
#include "Window.h"
#include "networking/GameClient.h"
#include "networking/GameServer.h"
#include "networking/NetworkObject.h"
#include <array>
#include <levels.h>

#define COLLISION_MSG 30

using namespace NCL;
using namespace CSC8503;

struct MessagePacket : public GamePacket {
  short playerID;
  short messageID;

  MessagePacket() {
    type = BasicNetworkMessages::Message;
    size = sizeof(short) * 2;
  }
};

ClientGame::ClientGame(GameWorld &gameWorld,
                       GameTechRendererInterface &renderer,
                       PhysicsSystem &physics)
    : NetworkedGame(gameWorld, renderer, physics) {
  NetworkBase::Initialise();
}

void ClientGame::SetupPacketHandlers() {
  constexpr std::array<uint16_t, 8> handledMessages = {
      BasicNetworkMessages::Full_State,
      BasicNetworkMessages::Delta_State,
      BasicNetworkMessages::Ping_Response,
      BasicNetworkMessages::Player_Connected,
      BasicNetworkMessages::Player_Disconnected,
      BasicNetworkMessages::Shutdown,
      BasicNetworkMessages::Hello,
      BasicNetworkMessages::LevelChange,
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
  }

  if (!net)
    return;

  if (!serverNet)
    net->SendPacket(createClientPacket());
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
  case Level::Default:
    InitWorld();
    break;
  case Level::CollisionTest:
    InitCollisionTest();
    break;
  default:
    NET_WARN("Unknown level requested: {}", static_cast<int>(level));
    return;
  }

  if (!serverNet) {
    if (!net) {
      player = SpawnPlayer(ourPlayerId);
    }

    if (net) {
      for (auto id : connectedPlayers) {
        if (id != ourPlayerId) {
          SpawnPlayer(id);
        }
      }
      player = SpawnPlayer(ourPlayerId);
    }
  } else {
    serverNet->OnLevelUpdate(level);
    player = serverNet->SpawnHostPlayer();
  }

  world.SetMainCamera(&player->GetCamera());

  for (auto &player : world.GetPlayerRange()) {
    NCL::CSC8503::NetworkObject *netObj = player.second->GetNetworkObject();
    if (netObj) {
      networkObjects.push_back(netObj);
    }
  }

  for (auto &obj : world) {
    NCL::CSC8503::NetworkObject *netObj = obj->GetNetworkObject();
    if (netObj && std::find(networkObjects.begin(), networkObjects.end(),
                            netObj) == networkObjects.end()) {
      networkObjects.push_back(netObj);
    }
  }
}

void ClientGame::EndLevel() {
  Level nextLevel = Level::Default;

  switch (currentLevel) {
  case Level::Default:
    nextLevel = Level::CollisionTest;
    break;
  case Level::CollisionTest:
    nextLevel = Level::Default;
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

  NetworkedGame::UpdateGame(dt);

  if (player)
    player->Input(dt, createClientPacket());
}

ClientPacket ClientGame::createClientPacket() {
  Bitflag<Player::Actions> actions;

  auto &kb = *Window::GetKeyboard();

  if (kb.KeyDown(KeyCodes::W))
    actions.set(Player::Actions::MoveForward);
  if (kb.KeyDown(KeyCodes::S))
    actions.set(Player::Actions::MoveBackward);
  if (kb.KeyDown(KeyCodes::A))
    actions.set(Player::Actions::MoveLeft);
  if (kb.KeyDown(KeyCodes::D))
    actions.set(Player::Actions::MoveRight);
  if (kb.KeyDown(KeyCodes::SPACE))
    actions.set(Player::Actions::Jump);

  bool shiftDown = kb.KeyDown(KeyCodes::SHIFT);
  bool ctrlDown = kb.KeyDown(KeyCodes::CONTROL);

  bool attaching = !shiftDown && !ctrlDown;
  bool extending = !shiftDown && ctrlDown;
  bool retracting = !ctrlDown && shiftDown;
  bool detaching = shiftDown && ctrlDown;

  if (kb.KeyDown(KeyCodes::NUM1) && attaching)
    actions.set(Player::Actions::AttachFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && attaching)
    actions.set(Player::Actions::AttachFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && attaching)
    actions.set(Player::Actions::AttachBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && attaching)
    actions.set(Player::Actions::AttachBackRightCorner);

  if (kb.KeyDown(KeyCodes::NUM1) && extending)
    actions.set(Player::Actions::ExtendFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && extending)
    actions.set(Player::Actions::ExtendFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && extending)
    actions.set(Player::Actions::ExtendBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && extending)
    actions.set(Player::Actions::ExtendBackRightCorner);

  if (kb.KeyDown(KeyCodes::NUM1) && retracting)
    actions.set(Player::Actions::RetractFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && retracting)
    actions.set(Player::Actions::RetractFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && retracting)
    actions.set(Player::Actions::RetractBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && retracting)
    actions.set(Player::Actions::RetractBackRightCorner);

  if (kb.KeyDown(KeyCodes::NUM1) && detaching)
    actions.set(Player::Actions::DetachFrontLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM2) && detaching)
    actions.set(Player::Actions::DetachFrontRightCorner);
  if (kb.KeyDown(KeyCodes::NUM3) && detaching)
    actions.set(Player::Actions::DetachBackLeftCorner);
  if (kb.KeyDown(KeyCodes::NUM4) && detaching)
    actions.set(Player::Actions::DetachBackRightCorner);

  ClientPacket p;
  p.actions = actions.flags;

  return p;
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
