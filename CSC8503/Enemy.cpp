#include "Enemy.h"
#include "Player.h"
#include "logging/log.h"
#include "physics/PhysicsObject.h"

#include "Debug.h"

#include "ai/behaviour_trees/BehaviourAction.h"
#include "ai/behaviour_trees/BehaviourSequence.h"
#include "ai/state_machine/State.h"
#include "ai/state_machine/StateMachine.h"
#include "ai/state_machine/StateTransition.h"

namespace NCL::CSC8503 {
Enemy::Enemy(const GameWorld &w, int id, const std::string name,
             float viewDistance)
    : GameObject(name), world(w), rootBehaviour(name),
      viewDistance(viewDistance) {
  GetTags().set(Tag::Enemy);
  InitializeBehaviours();

  networkObject = new NetworkObject(*this, id);
}

void Enemy::InitializeBehaviours() {
  auto canSeePlayer = [this]() {
    float distance = std::numeric_limits<float>::infinity();
    std::optional<const NCL::CSC8503::Player *> goal = std::nullopt;

    Vector3 pos = GetTransform().GetPosition();

    for (auto &player : world.GetPlayerRange()) {
      Vector3 pPos = player.second->GetTransform().GetPosition();

      Vector3 dir = Vector::Normalise(pPos - pos);

      Ray ray(pos, dir);
      RayCollision hit;
      if (world.Raycast(ray, hit, std::numeric_limits<float>::max(), this)) {
        if (hit.node == player.second && distance > hit.rayDistance) {
          distance = hit.rayDistance;
          goal = reinterpret_cast<const NCL::CSC8503::Player *const>(
              player.second);
        }
      }
    }

    return goal;
  };

  enum class WaypointState { Ongoing, Finished, Failed };

  auto gotoNextWaypoint = [this](float dt) {
    if (!nav.has_value()) {
      return WaypointState::Failed;
    }
    if (NavigateTo(dt, nav->waypoint)) {
      if (!nav->next()) {
        nav.reset();
        return WaypointState::Finished;
      }
    }
    return WaypointState::Ongoing;
  };

  std::unique_ptr chasingPlayer =
      std::make_unique<State>([this, gotoNextWaypoint, canSeePlayer](float dt) {
        gotoNextWaypoint(dt);
        auto seenPlayer = canSeePlayer();

        if (seenPlayer.has_value()) {
          timeSinceSeenPlayer = 0.0f;
          Vector3 currentPlayerPos =
              seenPlayer.value()->GetTransform().GetPosition();

          Vector3 to = currentPlayerPos - lastSeenPlayerPos;

          if (!navRequest.has_value() && Vector::Dot(to, to) > 5.f) {
            navRequest = world.pathfind().requestPath(
                GetTransform().GetPosition(), lastSeenPlayerPos, true);
            lastSeenPlayerPos =
                seenPlayer.value()->GetTransform().GetPosition();
          }
        }
      });

  std::unique_ptr patrolling =
      std::make_unique<State>([this, gotoNextWaypoint](float dt) {
        WaypointState state = gotoNextWaypoint(dt);
        switch (state) {
        case WaypointState::Ongoing:
          break;
        case WaypointState::Failed:
        case WaypointState::Finished:
          if (patrolPoints.empty()) {
            return;
          }

          currentPatrolPoint = (currentPatrolPoint + 1) % patrolPoints.size();
          if (currentPatrolPoint >= patrolPoints.size()) {
            currentPatrolPoint = 0;
          }
          navRequest = world.pathfind().requestPath(
              GetTransform().GetPosition(), patrolPoints[currentPatrolPoint],
              true);
        }
      });

  std::unique_ptr shouldChasePlayer = std::make_unique<StateTransition>(
      patrolling.get(), chasingPlayer.get(), [this, canSeePlayer]() {
        auto playerOpt = canSeePlayer();
        if (playerOpt.has_value()) {
          timeSinceSeenPlayer = 0.0f;
          lastSeenPlayerPos = playerOpt.value()->GetTransform().GetPosition();

          navRequest = world.pathfind().requestPath(
              GetTransform().GetPosition(), lastSeenPlayerPos, true);
          return true;
        }
        return false;
      });

  std::unique_ptr shouldPatrol = std::make_unique<StateTransition>(
      chasingPlayer.get(), patrolling.get(), [this]() {
        if (timeSinceSeenPlayer > 5.0f) {
          UpdateToClosestPatrolPoint();
          return true;
        }
        return false;
      });

  pathfindingMachine.AddTransition(std::move(shouldChasePlayer));
  pathfindingMachine.AddTransition(std::move(shouldPatrol));
  pathfindingMachine.AddState(std::move(patrolling));
  pathfindingMachine.AddState(std::move(chasingPlayer));

  std::shared_ptr pathfind = std::make_unique<BehaviourAction>(
      "Pathfinding Update", [this](float dt, BehaviourState state) {
        pathfindingMachine.Update(dt);
        return BehaviourState::Ongoing;
      });

  rootBehaviour.AddChild(pathfind);
}

void Enemy::Update(float dt) {
  timeSinceSeenPlayer += dt;
  rootBehaviour.Execute(dt);
  CheckNavRequest();
}

void Enemy::CheckNavRequest() {
  if (navRequest.has_value() && navRequest.value().valid()) {
    auto res = navRequest.value().get();
    if (res.is_ok()) {
      auto path = res.unwrap();
      Vector3 first;
      path.PopWaypoint(first);
      path.PopWaypoint(first);
      nav = Nav{.path = std::move(path), .waypoint = first};
      navRequest.reset();
      DEBUG("Enemy pathfinding succeeded");
    } else {
      WARN("Enemy pathfinding failed with error {}", res.unwrap_err());
      nav.reset();
    }
  }
}

bool Enemy::NavigateTo(float dt, const Vector3 &targetPos) {
  Vector3 currentPos = GetTransform().GetPosition();

  Vector3 toTarget = targetPos - currentPos;
  toTarget.y = 0.f;

  Vector3 moveDir = Vector::Normalise(toTarget);

  GetPhysicsObject()->AddForce(moveDir * speed * dt);

  constexpr float waypointThreshold = 2.0f;
  return Vector::Dot(toTarget, toTarget) < waypointThreshold;
}

void Enemy::UpdateToClosestPatrolPoint() {
  if (patrolPoints.empty()) {
    return;
  }
  Vector3 currentPos = GetTransform().GetPosition();
  size_t closestPoint = currentPatrolPoint;
  float closestDist = std::numeric_limits<float>::infinity();
  for (size_t i = 0; i < patrolPoints.size(); ++i) {
    float dist =
        Vector::Dot(patrolPoints[i] - currentPos, patrolPoints[i] - currentPos);
    if (dist < closestDist) {
      closestDist = dist;
      closestPoint = i;
    }
  }
  currentPatrolPoint = closestPoint;
}
} // namespace NCL::CSC8503