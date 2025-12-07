#include "Enemy.h"
#include "logging/log.h"
#include "physics/PhysicsObject.h"

namespace NCL::CSC8503 {
Enemy::Enemy(const GameWorld &w, const std::string name, float viewDistance)
    : GameObject(name), world(w), rootBehaviour(name),
      viewDistance(viewDistance) {
  InitializeBehaviours();
}

void Enemy::InitializeBehaviours() {
  auto canSeePlayer = [this](float dt) {
    float distance = std::numeric_limits<float>::infinity();
    std::optional<Vector3> goal = std::nullopt;

    Vector3 pos = GetTransform().GetPosition();

    for (auto &player : world.GetPlayerRange()) {
      Vector3 pPos = player->GetTransform().GetPosition();

      Vector3 dir = (pPos - pos);

      Ray ray(pos, Vector::Normalise(dir));
      RayCollision hit;
      if (world.Raycast(ray, hit, viewDistance, this)) {
        if (hit.node == player) {
          if (distance < hit.rayDistance) {
            distance = hit.rayDistance;
            goal = pPos;
          }
        }
      }
    }

    return goal;
  };
}

void Enemy::Update(float dt) {
  rootBehaviour.Execute(dt);
  CheckNavRequest();

  if (nav.has_value() && NavigateTo(nav->waypoint)) {
    if (!nav->next()) {
      nav.reset();
    }
  }
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