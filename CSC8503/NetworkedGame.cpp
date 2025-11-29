#include "NetworkedGame.h"
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

NetworkedGame::NetworkedGame(GameWorld &gameWorld,
                             GameTechRendererInterface &renderer,
                             PhysicsSystem &physics)
    : TutorialGame(gameWorld, renderer, physics) {

  NetworkBase::Initialise();
  timeToNextPacket = 0.0f;
}

NetworkedGame::~NetworkedGame() {}

void NetworkedGame::UpdateGame(float dt) {
  timeToNextPacket -= dt;
  if (timeToNextPacket < 0) {
    NetworkUpdate(dt);
    timeToNextPacket += 1.0f / 20.0f; // 20hz server/client update
  }

  TutorialGame::UpdateGame(dt);
}