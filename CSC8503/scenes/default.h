#pragma once

#include "IScene.h"

class DefaultScene : public IScene {
public:
  DefaultScene(TutorialGame &game) : IScene(game) {}

  void OnAwake() override {
    IScene::OnAwake();
    game.InitWorld();
  }
};
