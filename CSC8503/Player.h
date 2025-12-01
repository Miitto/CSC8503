#pragma once

#include "Camera.h"
#include "GameObject.h"

namespace NCL::CSC8503 {
class Player : public GameObject {
public:
  Player(Camera &camera);

  void Update(float dt) override;

protected:
  Camera &camera;
};
} // namespace NCL::CSC8503