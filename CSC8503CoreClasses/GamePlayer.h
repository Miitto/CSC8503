#pragma once

#include "Camera.h"
#include "GameObject.h"
#include "networking/packets.h"

namespace NCL::CSC8503 {
class GamePlayer : public GameObject {
public:
  GamePlayer(int id) : id(id) { GetTags().set(Tag::Player); }

  virtual void Input(float dt, ClientPacket input) = 0;

  const int GetId() const { return id; }

private:
  int id;
};
} // namespace NCL::CSC8503