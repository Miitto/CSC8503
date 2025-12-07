#include "GameWorld.h"
#include "Camera.h"
#include "GameObject.h"
#include "collisions/CollisionDetection.h"
#include "constraints/Constraint.h"

#include <random>

#include "collisions/CollisionDetection.h"
#include "collisions/QuadTree.h"
#include "collisions/Ray.h"

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld() {
  shuffleConstraints = false;
  shuffleObjects = false;
  worldIDCounter = 0;
  worldStateCounter = 0;
}

GameWorld::~GameWorld() {}

void GameWorld::Clear() {
  gameObjects.clear();
  constraints.clear();
  worldIDCounter = 0;
  worldStateCounter = 0;
}

void GameWorld::ClearAndErase() {
  for (auto &i : gameObjects) {
    delete i;
  }
  for (auto &i : constraints) {
    delete i;
  }
  Clear();
}

void GameWorld::AddGameObject(GameObject *o) {
  gameObjects.emplace_back(o);
  o->SetWorldID(worldIDCounter++);
  worldStateCounter++;
}

void GameWorld::AddPlayerObject(GameObject *o, bool addToGameList) {
  players.emplace_back(o);
  if (addToGameList)
    AddGameObject(o);
}

void GameWorld::RemoveGameObject(GameObject *o, bool andDelete) {
  gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o),
                    gameObjects.end());
  if (andDelete) {
    delete o;
  }
  worldStateCounter++;
}

void GameWorld::RemovePlayerObject(GameObject *o, bool andDelete,
                                   bool removeFromGameObjects) {
  players.erase(std::remove(players.begin(), players.end(), o), players.end());
  if (removeFromGameObjects) {
    RemoveGameObject(o, andDelete);
  } else {
    if (andDelete)
      delete o;
    worldStateCounter++;
  }
}

void GameWorld::GetObjectIterators(GameObjectIterator &first,
                                   GameObjectIterator &last) const {

  first = gameObjects.begin();
  last = gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
  for (GameObject *g : gameObjects) {
    f(g);
  }
}

void GameWorld::UpdateWorld(float dt) {
  auto rng = std::default_random_engine{};

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine e(seed);

  if (shuffleObjects) {
    std::shuffle(gameObjects.begin(), gameObjects.end(), e);
  }

  if (shuffleConstraints) {
    std::shuffle(constraints.begin(), constraints.end(), e);
  }
}

bool GameWorld::Raycast(Ray &r, RayCollision &collision,
                        std::optional<float> maxDist,
                        const GameObject *const ignoreThis) const {
  auto distanceCheck = [&](const RayCollision &c) {
    if (maxDist.has_value()) {
      return c.rayDistance <= maxDist.value();
    }
    return true;
  };

  Vector3 rayPos = r.GetPosition();
  float maxDistSq = maxDist.has_value() ? maxDist.value() * maxDist.value()
                                        : std::numeric_limits<float>::max();

  for (auto &i : gameObjects) {
    if (!i->GetBoundingVolume()) { // objects might not be collideable etc...
      continue;
    }
    if (i == ignoreThis) {
      continue;
    }

    // Distance pre check
    Vector3 objPos = i->GetTransform().GetPosition();
    Vector3 rel = objPos - rayPos;
    float maxExtent = i->GetBoundingVolume()->GetMaxExtent();
    float distSqToObj =
        Vector::Dot(rel, rel) +
        (maxExtent * maxExtent); // Take into account max extent.
    if (distSqToObj > maxDistSq) {
      continue;
    }

    RayCollision thisCollision;
    if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
      if (thisCollision.rayDistance < collision.rayDistance &&
          distanceCheck(thisCollision)) {
        thisCollision.node = i;
        collision = thisCollision;
      }
    }
  }
  if (collision.node) {
    return true;
  }
  return false;
}

bool GameWorld::RaycastHitCheck(Ray &r, std::optional<float> maxDist,
                                const GameObject *const ignoreThis) const {
  RayCollision collision;

  auto distanceCheck = [&](const RayCollision &c) {
    if (maxDist.has_value()) {
      return c.rayDistance <= maxDist.value();
    }
    return true;
  };

  for (auto &i : gameObjects) {
    if (!i->GetBoundingVolume()) { // objects might not be collideable etc...
      continue;
    }
    if (i == ignoreThis) {
      continue;
    }

    RayCollision thisCollision;
    if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
      if (distanceCheck(thisCollision)) {
        return true;
      }
    }
  }
  return false;
}

GameWorld::LookingAt
GameWorld::ObjectLookAt(GameObject *object,
                        std::optional<float> maxDist) const {

  Vector3 rayPos = object->GetTransform().GetPosition();
  Vector3 rayDir = object->GetTransform().GetOrientation() * Vector3(0, 0, -1);

  Ray r = Ray(rayPos, rayDir);
  RayCollision collision;
  if (Raycast(r, collision, maxDist, object)) {
    LookingAt lookingAt;
    lookingAt.object = (GameObject *)collision.node;
    lookingAt.collision = collision;
    return lookingAt;
  }

  return {nullptr, RayCollision()};
}

/*
Constraint Tutorial Stuff
*/

void GameWorld::AddConstraint(Constraint *c) { constraints.emplace_back(c); }

void GameWorld::RemoveConstraint(Constraint *c, bool andDelete) {
  constraints.erase(std::remove(constraints.begin(), constraints.end(), c),
                    constraints.end());
  if (andDelete) {
    delete c;
  }
}

void GameWorld::GetConstraintIterators(
    std::vector<Constraint *>::const_iterator &first,
    std::vector<Constraint *>::const_iterator &last) const {
  first = constraints.begin();
  last = constraints.end();
}