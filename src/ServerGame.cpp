#include "ServerGame.h"
#include "GameWorld.h"
#include "NetworkedGame.h"
#include "networking/GameServer.h"
#include "networking/NetworkObject.h"
#include <GameTechRendererInterface.h>
#include <physics/PhysicsSystem.h>

using namespace NCL;
using namespace CSC8503;

ServerGame::ServerGame(GameWorld &gameWorld,
                       GameTechRendererInterface &renderer,
                       PhysicsSystem &physics)
    : NetworkedGame(gameWorld, renderer, physics),
      net({NetworkBase::GetDefaultPort(), 4, *this}) {

  NetworkBase::Initialise();
  timeToNextPacket = 0.0f;

  net.RegisterPacketHandler(BasicNetworkMessages::Hello, this);
  net.RegisterPacketHandler(BasicNetworkMessages::LevelChange, this);
}

ServerGame::~ServerGame() {}

void ServerGame::NetworkUpdate(float dt) { net.Update(dt, world); }

void ServerGame::UpdateMinimumState() { net.UpdateMinimumState(world); }

void ServerGame::StartLevel(Level level) {
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

  net.OnLevelUpdate(level);

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

void ServerGame::EndLevel() {
  net.OnLevelEnd();
  Level nextLevel = Level::Default;

  switch (net.GetCurrentLevel()) {
  case Level::Default:
    nextLevel = Level::CollisionTest;
    break;
  case Level::CollisionTest:
    nextLevel = Level::Default;
    break;
  }

  StartLevel(nextLevel);
  net.OnLevelUpdate(nextLevel);
}

void ServerGame::ReceivePacket(GamePacketType type, GamePacket *payload,
                               int source) {
  switch (type.type) {
  case BasicNetworkMessages::Hello: {
    if (!started) {
      StartLevel(Level::Default);
      started = true;
    }
    break;
  }
  case BasicNetworkMessages::LevelChange: {
    auto packet = GamePacket::as<LevelChangePacket>(payload);
    auto level = static_cast<Level>(packet->level);
    StartLevel(level);
  }
  }
}
