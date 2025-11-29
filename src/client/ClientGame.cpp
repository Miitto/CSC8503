#include "ClientGame.h"
#include "GameWorld.h"
#include "NetworkPlayer.h"
#include "Window.h"
#include "networking/GameClient.h"
#include "networking/GameServer.h"
#include "networking/NetworkObject.h"

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

void ClientGame::Disconnect() {
  if (net) {
    net->SendPacket(BasicNetworkMessages::Shutdown);
    net = std::nullopt;
  }
}

void ClientGame::InitServer(uint16_t port, int maxClients) {
  serverNet.emplace(port, maxClients);
}

void ClientGame::ShutdownServer() { serverNet = std::nullopt; }

void ClientGame::NetworkUpdate(float dt) {
  if (serverNet) {
    serverNet->Update(dt, world);
  }

  if (!net)
    return;

  ClientPacket newPacket;

  if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
    // fire button pressed!
    newPacket.buttonstates[0] = 1;
    newPacket.lastID = lastFullSync;
  }
  net->SendPacket(newPacket);
}

void ClientGame::PingCheck(float dt) {
  if (pingInfo) {
    pingInfo->timeSinceLastPing += dt;

    constexpr float PING_TIMEOUT = 3.0f;

    if (pingInfo->timeSinceLastPing >= PING_TIMEOUT) {
      if (pingInfo->attemptsLeft > 0) {
        net->SendPacket(BasicNetworkMessages::Ping);
        pingInfo->lastPingSentTime = std::chrono::high_resolution_clock::now();
        pingInfo->timeSinceLastPing = 0.0f;
        pingInfo->attemptsLeft--;
      } else {
        NET_LOG("Connection to server lost: No ping response.");
        pingInfo->cb(ConnectionFailure::NoPingResponse);
        Disconnect();
        pingInfo.reset();
      }
    }
  }
}

void ClientGame::StartLevel() {}

void ClientGame::ReceivePacket(GamePacketType type, GamePacket *payload,
                               int source) {
  switch (type.type) {
  case static_cast<uint16_t>(BasicNetworkMessages::Full_State): {
    auto fs = GamePacket::as<FullPacket>(payload);
    lastFullSync = fs->fullState.stateID;
    break;
  }
  case static_cast<uint16_t>(BasicNetworkMessages::Ping_Response): {
    if (pingInfo) {
      pingInfo->cb(ConnectionFailure::None);
      pingInfo = std::nullopt;
    }
    break;
  }
  }

  for (auto i : networkObjects) {
    i->ReadPacket(*payload);
  }

  net->SendPacket(AckPacket(lastFullSync));
}
