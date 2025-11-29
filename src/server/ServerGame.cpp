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
  short playerID;
  short messageID;

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
}

ServerGame::~ServerGame() {}

void ServerGame::NetworkUpdate(float dt) { net.Update(dt, world); }

void ServerGame::UpdateMinimumState() { net.UpdateMinimumState(world); }

void ServerGame::StartLevel() {}

void ServerGame::ReceivePacket(GamePacketType type, GamePacket *payload,
                               int source) {}
