#include "ServerGame.h"
#include "GameWorld.h"
#include "NetworkedGame.h"
#include "networking/GameServer.h"
#include "networking/NetworkObject.h"
#include <GameTechRendererInterface.h>
#include <physics/PhysicsSystem.h>

#define COLLISION_MSG 30

using namespace NCL;
using namespace CSC8503;

struct MessagePacket : public GamePacket {
  short playerID = -1;
  short messageID = -1;

  MessagePacket() {
    type = BasicNetworkMessages::Message;
    size = sizeof(short) * 2;
  }
};

ServerGame::ServerGame(GameWorld &gameWorld,
                       GameTechRendererInterface &renderer,
                       PhysicsSystem &physics)
    : NetworkedGame(gameWorld, renderer, physics),
      net({NetworkBase::GetDefaultPort(), 4}) {

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

  NET_INFO("Starting level {}", level);
  Clear();
  net.OnLevelUpdate(level);
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
