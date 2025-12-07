#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "ai/behaviour_trees/BehaviourSelector.h"
#include "ai/pathfinding/PathfindingService.h"
#include <optional>

namespace NCL::CSC8503 {
class Enemy : public GameObject {
public:
  Enemy(const GameWorld &world, const std::string name = "Enemy",
        float viewDist = 100);

  void Update(float dt) override;

protected:
  void InitializeBehaviours();

  void CheckNavRequest();
  bool NavigateTo(const Vector3 &targetPos);

  const GameWorld &world;
  std::optional<PathfindingService::Request> navRequest;

  struct Nav {
    NavigationPath path;
    Vector3 waypoint;

    bool next() { return path.PopWaypoint(waypoint); }
  };

  std::optional<Nav> nav;
  BehaviourSelector rootBehaviour;
  float viewDistance;
};
} // namespace NCL::CSC8503