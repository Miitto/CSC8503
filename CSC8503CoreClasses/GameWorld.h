#pragma once
#include "./Camera.h"
#include "collisions/Ray.h"

namespace NCL {
namespace Maths {
class Ray;
} // namespace Maths
class Camera;
class PerspectiveCamera;

namespace CSC8503 {
class GameObject;
class Constraint;

typedef std::function<void(GameObject *)> GameObjectFunc;
typedef std::vector<GameObject *>::const_iterator GameObjectIterator;

class GameWorld {
public:
  GameWorld();
  ~GameWorld();

  void Clear();
  void ClearAndErase();

  void AddGameObject(GameObject *o);
  void RemoveGameObject(GameObject *o, bool andDelete = false);

  void AddConstraint(Constraint *c);
  void RemoveConstraint(Constraint *c, bool andDelete = false);

  PerspectiveCamera &GetMainCamera() { return mainCamera; }

  void ShuffleConstraints(bool state) { shuffleConstraints = state; }

  void ShuffleObjects(bool state) { shuffleObjects = state; }

  bool Raycast(Ray &r, RayCollision &collision,
               std::optional<float> maxDist = std::nullopt,
               const GameObject *const ignore = nullptr) const;

  bool RaycastHitCheck(Ray &r, std::optional<float> maxDist = std::nullopt,
                       const GameObject *const ignore = nullptr) const;

  struct LookingAt {
    GameObject *object;
    RayCollision collision;

    operator bool() const { return object != nullptr; }
  };

  LookingAt ObjectLookAt(GameObject *object,
                         std::optional<float> maxDist = std::nullopt) const;

  virtual void UpdateWorld(float dt);

  void OperateOnContents(GameObjectFunc f);

  void GetObjectIterators(GameObjectIterator &first,
                          GameObjectIterator &last) const;

  void
  GetConstraintIterators(std::vector<Constraint *>::const_iterator &first,
                         std::vector<Constraint *>::const_iterator &last) const;

  std::vector<GameObject *>::iterator begin() { return gameObjects.begin(); }
  std::vector<GameObject *>::iterator end() { return gameObjects.end(); }
  std::vector<GameObject *>::const_iterator begin() const noexcept {
    return gameObjects.begin();
  }
  std::vector<GameObject *>::const_iterator end() const noexcept {
    return gameObjects.end();
  }
  std::vector<GameObject *>::const_iterator cbegin() const noexcept {
    return gameObjects.cbegin();
  }
  std::vector<GameObject *>::const_iterator cend() const noexcept {
    return gameObjects.cend();
  }

  int GetWorldStateID() const { return worldStateCounter; }

  void SetSunPosition(const Vector3 &pos) { sunPosition = pos; }

  Vector3 GetSunPosition() const { return sunPosition; }

  void SetSunColour(const Vector3 &col) { sunColour = col; }

  Vector3 GetSunColour() const { return sunColour; }

protected:
  std::vector<GameObject *> gameObjects;
  std::vector<Constraint *> constraints;

  PerspectiveCamera mainCamera;

  bool shuffleConstraints;
  bool shuffleObjects;
  int worldIDCounter;
  int worldStateCounter;

  Vector3 sunPosition;
  Vector3 sunColour;
};
} // namespace CSC8503
} // namespace NCL
