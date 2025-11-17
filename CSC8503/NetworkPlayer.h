#pragma once
#include "GameObject.h"
#include "networking/GameClient.h"

namespace NCL {
namespace CSC8503 {
class NetworkedGame;

class NetworkPlayer : public GameObject {
public:
  NetworkPlayer(NetworkedGame *game, int num);
  ~NetworkPlayer();

  void OnCollisionBegin(GameObject *otherObject) override;

  int GetPlayerNum() const { return playerNum; }

protected:
  NetworkedGame *game;
  int playerNum;
};
} // namespace CSC8503
} // namespace NCL
