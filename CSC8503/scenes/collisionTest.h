#pragma once

#include "IScene.h"

class CollisionTestScene : public IScene {
public:
  CollisionTestScene(TutorialGame &game) : IScene(game) {}

  void OnInit() override {
    IScene::OnAwake();
    game.InitCollisionTest();
  }
};
