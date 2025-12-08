#include "Enemy.h"
#include "Player.h"
#include "logging/log.h"
#include "physics/PhysicsObject.h"

#include "ai/behaviour_trees/BehaviourAction.h"

namespace NCL::CSC8503 {
Enemy::Enemy(const GameWorld &w, const std::string name, float viewDistance)
    : GameObject(name), world(w), rootBehaviour(name),
      viewDistance(viewDistance) {
  InitializeBehaviours();
}

void Enemy::InitializeBehaviours() {
  auto canSeePlayer = [this]() {
    float distance = std::numeric_limits<float>::infinity();
    std::optional<const NCL::CSC8503::Player *> goal = std::nullopt;

    Vector3 pos = GetTransform().GetPosition();

    for (auto &player : world.GetPlayerRange()) {
      Vector3 pPos = player.second->GetTransform().GetPosition();

      Vector3 dir = Vector::Normalise(pPos - pos);

      if (Vector::Dot(dir, GetTransform().GetOrientation() * Vector3(0, 0, 1)) <
          cosf(Maths::DegreesToRadians(viewAngle))) {
        continue;
      }

      Ray ray(pos, dir);
      RayCollision hit;
      if (world.Raycast(ray, hit, viewDistance, this)) {
        if (hit.node == player.second) {
          if (distance < hit.rayDistance) {
            distance = hit.rayDistance;
            goal = reinterpret_cast<const NCL::CSC8503::Player *const>(
                player.second);
          }
        }
      }
    }

    return goal;
  };

  std::shared_ptr gotoNextWaypoint = std::make_shared<BehaviourAction>(
      "GotoNextWaypoint", [this, canSeePlayer](float dt, BehaviourState state) {
        if (!nav.has_value()) {
          return BehaviourState::Failure;
        }
        if (NavigateTo(nav->waypoint)) {
          if (!nav->next()) {
            nav.reset();
            return BehaviourState::Success;
          }
        }
        return BehaviourState::Ongoing;
      });

  rootBehaviour.AddChild(gotoNextWaypoint);
}

void Enemy::Update(float dt) {
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
      nav = Nav{.path = std::move(path), .waypoint = first};
    } else {
      WARN("Enemy pathfinding failed with error {}", res.unwrap_err());
      nav.reset();
    }
  }
}

bool Enemy::NavigateTo(const Vector3 &targetPos) {
  Vector3 currentPos = GetTransform().GetPosition();

  Vector3 toTarget = targetPos - currentPos;

  Vector3 moveDir = Vector::Normalise(toTarget);

  GetPhysicsObject()->AddForce(moveDir * speed);

  constexpr float waypointThreshold = 1.0f;
  return Vector::Dot(toTarget, toTarget) < waypointThreshold;
}
} // namespace NCL::CSC8503