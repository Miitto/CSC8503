#pragma once

#include "GameObject.h"
#include "ai/behaviour_trees/BehaviourSelector.h"
#include "ai/pathfinding/NavigationPath.h"
#include <future>
#include <variant>

namespace NCL::CSC8503 {
class Enemy : public GameObject {
public:
  Enemy(const GameWorld &world, std::string name = "Enemy");

  void Update(float dt) override;

protected:
  GameWorld &world;
  std::varaint<std::future<NavigationPath>, NavigationPath> nav;
  BehaviourSelector rootBehaviour;
};
} // namespace NCL::CSC8503