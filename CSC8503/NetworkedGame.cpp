#include "NetworkedGame.h"
#include "GameWorld.h"
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
  timeSinceLastNetUpdate += dt;
  if (!active)
    return;

  timeToNextPacket -= dt;
  if (timeToNextPacket < 0) {
    NetworkUpdate(timeSinceLastNetUpdate);
    timeToNextPacket += 1.0f / 20.0f; // 20hz server/client update
    timeSinceLastNetUpdate = 0.0f;
  }

  TutorialGame::UpdateGame(dt);
}

Player *NetworkedGame::SpawnPlayer(int id) {
  Player *player = TutorialGame::SpawnPlayer(id);
  NetworkObject *netObj = player->GetNetworkObject();
  if (netObj) {
    networkObjects.push_back(netObj);
  }
  return player;
}

void NetworkedGame::RemovePlayer(int id) {
  TutorialGame::RemovePlayer(id);
  networkObjects.erase(
      std::remove_if(
          networkObjects.begin(), networkObjects.end(),
          [id](NetworkObject *obj) {
            return obj->GetObject().GetTags().has(GameObject::Tag::Player) &&
                   static_cast<Player &>(obj->GetObject()).GetId() == id;
          }),
      networkObjects.end());
}