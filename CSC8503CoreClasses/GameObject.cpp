#include "GameObject.h"
#include "RenderObject.h"
#include "collisions/CollisionDetection.h"
#include "networking/NetworkObject.h"
#include "physics/PhysicsObject.h"

using namespace NCL::CSC8503;

GameObject::GameObject(const std::string &objectName)
    : name(objectName), worldID(-1), boundingVolume(nullptr),
      physicsObject(nullptr), renderObject(nullptr), networkObject(nullptr) {}

GameObject::~GameObject() {
  delete boundingVolume;
  delete physicsObject;
  delete renderObject;
  delete networkObject;
}

void GameObject::Sync(int id) { networkObject = new NetworkObject(*this, id); }

void GameObject::Reset() {
  transform = resetTransform;
  if (physicsObject) {
    physicsObject->Reset();
  }
}

bool GameObject::GetBroadphaseAABB(Vector3 &outSize) const {
  if (!boundingVolume) {
    return false;
  }
  outSize = broadphaseAABB;
  return true;
}

void GameObject::UpdateBroadphaseAABB() {
  if (!boundingVolume) {
    return;
  }
  switch (boundingVolume->type) {
  case VolumeType::AABB: {
    broadphaseAABB = ((AABBVolume &)*boundingVolume).GetHalfDimensions();
  } break;
  case VolumeType::Sphere: {
    float r = ((SphereVolume &)*boundingVolume).GetRadius();
    broadphaseAABB = Vector3(r, r, r);
  } break;
  case VolumeType::OBB: {
    Matrix3 mat =
        Quaternion::RotationMatrix<Matrix3>(transform.GetOrientation());
    mat = Matrix::Absolute(mat);
    Vector3 halfSizes = ((OBBVolume &)*boundingVolume).GetHalfDimensions();
    broadphaseAABB = mat * halfSizes;
  } break;
  case VolumeType::Capsule: {
    auto capsule = (CapsuleVolume &)*boundingVolume;
    auto height = capsule.GetHalfHeight() + capsule.GetRadius() / 2;
    auto width = capsule.GetRadius();

    Vector3 halfSizes = Vector3(width, height, width);

    auto rotMat = Matrix::Absolute(
        Quaternion::RotationMatrix<Matrix3>(transform.GetOrientation()));

    broadphaseAABB = rotMat * halfSizes;
  } break;
  default: {
    PHYS_WARN("Object type {} does not support Broadphase culling",
              boundingVolume->type);
  }
  }
}