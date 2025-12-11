#pragma once

#include "GameObject.h"
#include "GameWorld.h"
#include "ai/behaviour_trees/BehaviourSequence.h"
#include "ai/pathfinding/PathfindingService.h"
#include "ai/state_machine/StateMachine.h"
#include <optional>

namespace NCL::CSC8503 {
class Enemy : public GameObject {
public:
  Enemy(const GameWorld &world, int id, const std::string name = "Enemy",
        float viewDist = 100);

  void Update(float dt) override;

  Enemy &AddPatrolPoint(Vector3 point) {
    patrolPoints.push_back(point);
    return *this;
  }

protected:
  void InitializeBehaviours();

  void CheckNavRequest();
  bool NavigateTo(float dt, const Vector3 &targetPos);

  void UpdateToClosestPatrolPoint();

  const GameWorld &world;
  std::optional<PathfindingService::Request> navRequest;

  struct Nav {
    NavigationPath path;
    Vector3 waypoint;

    bool next() { return path.PopWaypoint(waypoint); }
  };

  std::optional<Nav> nav;
  BehaviourSequence rootBehaviour;
  StateMachine pathfindingMachine;

  float viewDistance;
  float reach = 2.0f;
  float viewAngle = 45.0f;
  float speed = 100.0f;

  float timeSinceSeenPlayer = 0.0f;
  Vector3 lastSeenPlayerPos;

  std::vector<Vector3> patrolPoints;
  size_t currentPatrolPoint = 0;
};
} // namespace NCL::CSC8503