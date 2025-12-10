#pragma once
#include "Bitflag.h"
#include "Transform.h"
#include "collisions/CollisionVolume.h"
#include "macros.h"

using std::vector;

namespace NCL::CSC8503 {
class NetworkObject;
class RenderObject;
class PhysicsObject;

class GameObject {
public:
  enum Tag {
    Player = BIT(0),
    Pane = BIT(1),
    Enemy = BIT(2),
    Inactive = BIT(3),
  };

  enum Layer {
    RaycastIgnore = BIT(1),
  };

  GameObject(const std::string &name = "");
  ~GameObject();

  void SetBoundingVolume(CollisionVolume *vol) { boundingVolume = vol; }

  bool IsActive() const { return !tags.has(Tag::Inactive); }

  const CollisionVolume *GetBoundingVolume() const { return boundingVolume; }

  const Bitflag<Tag> &GetTags() const { return tags; }
  Bitflag<Tag> &GetTags() { return tags; }

  const Bitflag<Layer> &GetLayers() const { return layers; }
  Bitflag<Layer> &GetLayers() { return layers; }

  Transform &GetTransform() { return transform; }
  const Transform &GetTransform() const { return transform; }

  void SetResetTransform(const Transform &t) { resetTransform = t; }
  void SetCurrentTransformAsReset() { resetTransform = transform; }

  virtual void Reset();
  RenderObject *GetRenderObject() const { return renderObject; }

  PhysicsObject *GetPhysicsObject() const { return physicsObject; }

  NetworkObject *GetNetworkObject() const { return networkObject; }

  void SetRenderObject(RenderObject *newObject) { renderObject = newObject; }

  void SetPhysicsObject(PhysicsObject *newObject) { physicsObject = newObject; }

  const std::string &GetName() const { return name; }

  virtual void OnCollisionBegin(GameObject *otherObject) {
    // std::cout << "OnCollisionBegin event occured!\n";
  }

  virtual void OnCollisionEnd(GameObject *otherObject) {
    // std::cout << "OnCollisionEnd event occured!\n";
  }

  virtual void Update(float dt) {}

  bool GetBroadphaseAABB(Vector3 &outsize) const;

  void UpdateBroadphaseAABB();

  void SetWorldID(int newID) { worldID = newID; }

  int GetWorldID() const { return worldID; }

protected:
  Transform transform;
  Transform resetTransform;

  CollisionVolume *boundingVolume;
  PhysicsObject *physicsObject;
  RenderObject *renderObject;
  NetworkObject *networkObject;

  Bitflag<Tag> tags;
  Bitflag<Layer> layers;

  int worldID;
  std::string name;

  Vector3 broadphaseAABB;
};
} // namespace NCL::CSC8503
