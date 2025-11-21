#pragma once

#include <Camera.h>
#include <GameObject.h>
#include <GameWorld.h>

class Player : public NCL::CSC8503::GameObject {
public:
  Player(NCL::CSC8503::GameWorld *world) : GameObject("Player"), world(world) {}

  void Update(float dt) override;

  void CamToPlayer(NCL::Camera &cam) const {
    constexpr Vector3 lockedOffset{0, 0.55f, 0.55f};
    Vector3 objPos = GetTransform().GetPosition();
    Quaternion objOr = GetTransform().GetOrientation();

    auto euler = objOr.ToEuler();

    if (euler.y < 0) {
      euler.y += 360.f;
    }
    if (euler.y > 360.f) {
      euler.y -= 360.f;
    }

    euler.x = pitch;

    Quaternion camRot = Quaternion::EulerAnglesToQuaternion(0, euler.y, 0);

    Vector3 offset = camRot * lockedOffset;

    Vector3 camPos = objPos + offset;

    Vector3 fwd = objOr * Vector3(0, 0, -1);

    Matrix4 temp = Matrix::View(camPos, camPos - fwd, Vector3(0, 1, 0));

    Matrix4 modelMat = Matrix::Inverse(temp);

    Quaternion q(modelMat);
    Vector3 angles = q.ToEuler(); // nearly there now!

    cam.SetPosition(camPos);
    cam.SetPitch(pitch);
    cam.SetYaw(angles.y);
  }

protected:
  float pitch = 0.f;
  NCL::CSC8503::GameWorld *world;
};