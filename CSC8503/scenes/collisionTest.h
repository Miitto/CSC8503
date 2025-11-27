#pragma once

#include "IScene.h"

class CollisionTestScene : public IScene {
public:
  CollisionTestScene(TutorialGame &game) : IScene(game) {}

  void OnAwake() override {
    IScene::OnAwake();
    game.InitCollisionTest();
  }
};
