#pragma once

#include "IScene.h"

class DefaultScene : public IScene {
public:
  DefaultScene(TutorialGame &game) : IScene(game) {}

  void OnInit() override {
    IScene::OnAwake();
    game.InitWorld();
  }
};
