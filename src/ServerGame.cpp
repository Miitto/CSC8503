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
      net({NetworkBase::GetDefaultPort(), 4, *this, &gameWorld}) {

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

  net.OnLevelUpdate(level);

  for (auto &obj : world) {
    NCL::CSC8503::NetworkObject *netObj = obj->GetNetworkObject();
    if (netObj) {
      networkObjects.push_back(netObj);
    }
  }
}

void ServerGame::EndLevel() {
  Level nextLevel = Level::One;

  switch (net.GetCurrentLevel()) {
  case Level::One:
    nextLevel = Level::Two;
    break;
  case Level::Two:
    nextLevel = Level::One;
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
      StartLevel(Level::One);
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
