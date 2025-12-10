#pragma once

#include "GameObject.h"
#include "TutorialGame.h"
#include "logging/log.h"

namespace NCL::CSC8503 {
class LevelEnd : public GameObject {
public:
  LevelEnd(TutorialGame &game) : game(game) {
    GetLayers().set(Layer::RaycastIgnore);
  }

  void OnCollisionBegin(GameObject *otherObject) override {
    DEBUG("LevelEnd collided with " + otherObject->GetName());
    if (otherObject->GetTags().has(Tag::Pane)) {
      LOG("Ending Level");
      game.EndLevel();
    }
  }

protected:
  TutorialGame &game;
};
} // namespace NCL::CSC8503