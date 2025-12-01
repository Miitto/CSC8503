#include "CollisionDetection.h"
#include "AABBVolume.h"
#include "CollisionVolume.h"
#include "Debug.h"
#include "Maths.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "VectorFormat.h"
#include "Window.h"
#include "logging/logger.h"
#include <array>
#include <span>

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray &r, const Plane &p,
                                              RayCollision &collisions) {
  float ln = Vector::Dot(p.GetNormal(), r.GetDirection());

  if (ln == 0.0f) {
    return false; // direction vectors are perpendicular!
  }

  Vector3 planePoint = p.GetPointOnPlane();

  Vector3 pointDir = planePoint - r.GetPosition();

  float d = Vector::Dot(pointDir, p.GetNormal()) / ln;

  collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);

  return true;
}

bool CollisionDetection::RayIntersection(const Ray &r, GameObject &object,
                                         RayCollision &collision) {
  bool hasCollided = false;

  if (object.GetLayers() & GameObject::Layer::RaycastIgnore) {
    return false;
  }

  const Transform &worldTransform = object.GetTransform();
  const CollisionVolume *volume = object.GetBoundingVolume();

  if (!volume) {
    return false;
  }

  switch (volume->type) {
  case VolumeType::AABB:
    hasCollided = RayAABBIntersection(r, worldTransform,
                                      (const AABBVolume &)*volume, collision);
    break;
  case VolumeType::OBB:
    hasCollided = RayOBBIntersection(r, worldTransform,
                                     (const OBBVolume &)*volume, collision);
    break;
  case VolumeType::Sphere:
    hasCollided = RaySphereIntersection(
        r, worldTransform, (const SphereVolume &)*volume, collision);
    break;

  case VolumeType::Capsule:
    hasCollided = RayCapsuleIntersection(
        r, worldTransform, (const CapsuleVolume &)*volume, collision);
    break;
  }

  return hasCollided;
}

bool CollisionDetection::RayBoxIntersection(const Ray &r, const Vector3 &boxPos,
                                            const Vector3 &boxSize,
                                            RayCollision &collision) {
  auto boxMin = boxPos - boxSize;
  auto boxMax = boxPos + boxSize;

  auto rayPos = r.GetPosition();
  auto rayDir = r.GetDirection();

  Vector3 t(-1.f, -1.f, -1.f);

  for (int i = 0; i < 3; ++i) {
    if (rayDir[i] > 0.f) {
      t[i] = (boxMin[i] - rayPos[i]) / rayDir[i];
    } else if (rayDir[i] < 0.f) {
      t[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
    }
  }

  float best = Vector::GetMaxElement(t);

  if (best < 0.f) {
    return false;
  }

  auto interesction = rayPos + rayDir * best;

  constexpr float bias = 0.0001f;

  for (int i = 0; i < 3; ++i) {
    if (interesction[i] < boxMin[i] - bias ||
        interesction[i] > boxMax[i] + bias) {
      return false;
    }
  }

  collision.collidedAt = interesction;
  collision.rayDistance = best;
  return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray &r,
                                             const Transform &worldTransform,
                                             const AABBVolume &volume,
                                             RayCollision &collision) {
  auto pos = worldTransform.GetPosition();
  auto size = volume.GetHalfDimensions();

  return RayBoxIntersection(r, pos, size, collision);
}

bool CollisionDetection::RayOBBIntersection(const Ray &r,
                                            const Transform &worldTransform,
                                            const OBBVolume &volume,
                                            RayCollision &collision) {
  auto rot = worldTransform.GetOrientation();
  auto pos = worldTransform.GetPosition();

  Matrix3 transform = Quaternion::RotationMatrix<Matrix3>(rot);
  Matrix3 invTransform = Quaternion::RotationMatrix<Matrix3>(rot.Conjugate());

  Vector3 localRayPos = r.GetPosition() - pos;
  Ray tRay(invTransform * localRayPos, invTransform * r.GetDirection());

  bool collided = RayBoxIntersection(tRay, Vector3(0, 0, 0),
                                     volume.GetHalfDimensions(), collision);

  if (collided) {
    collision.collidedAt = (transform * (collision.collidedAt)) + pos;
  }
  return collided;
}

bool CollisionDetection::RaySphereIntersection(const Ray &r,
                                               const Transform &worldTransform,
                                               const SphereVolume &volume,
                                               RayCollision &collision) {
  Vector3 spherePos = worldTransform.GetPosition();
  float sphereRadius = volume.GetRadius();

  auto dir = spherePos - r.GetPosition();

  float sphereProj = Vector::Dot(dir, r.GetDirection());

  if (sphereProj < 0) {
    return false;
  }

  Vector3 closestPoint = r.GetPosition() + (r.GetDirection() * sphereProj);

  float distToSphereSq =
      Vector::Dot(spherePos - closestPoint, spherePos - closestPoint);

  if (distToSphereSq > sphereRadius * sphereRadius) {
    return false;
  }

  float offset = sqrtf((sphereRadius * sphereRadius) - distToSphereSq);

  collision.rayDistance = sphereProj - offset;
  collision.collidedAt =
      r.GetPosition() + r.GetDirection() * collision.rayDistance;

  return true;
}

bool CollisionDetection::RayCapsuleIntersection(const Ray &r,
                                                const Transform &worldTransform,
                                                const CapsuleVolume &volume,
                                                RayCollision &collision) {
  // https://gist.github.com/jdryg/ecde24d34aa0ce2d4d87
  auto aPos =
      worldTransform.GetPosition() +
      (worldTransform.GetOrientation() * Vector3(0, volume.GetHalfHeight(), 0));
  auto bPos =
      worldTransform.GetPosition() + (worldTransform.GetOrientation() *
                                      Vector3(0, -volume.GetHalfHeight(), 0));

  auto ab = bPos - aPos;
  auto ao = r.GetPosition() - aPos;

  auto abDotDir = Vector::Dot(ab, r.GetDirection());
  auto abDotAo = Vector::Dot(ab, ao);
  auto abDotAb = Vector::Dot(ab, ab);

  auto m = abDotDir / abDotAb;
  auto n = abDotAo / abDotAb;

  auto q = r.GetDirection() - (ab * m);
  auto R = ao - (ab * n);

  auto a = Vector::Dot(q, q);
  auto b = 2.0f * Vector::Dot(q, R);
  auto c = Vector::Dot(R, R) - (volume.GetRadius() * volume.GetRadius());

  SphereVolume sphere(volume.GetRadius());
  Transform tA;

  if (a == 0.0f) {
    SphereVolume sphere(volume.GetRadius());
    tA.SetPosition(aPos);
    bool collided = RaySphereIntersection(r, tA, sphere, collision);
    RayCollision tempCollision;
    tA.SetPosition(bPos);
    bool collidedB = RaySphereIntersection(r, tA, sphere, tempCollision);
    if (collidedB && tempCollision.rayDistance < collision.rayDistance) {
      collision = tempCollision;
    }
    return collided || collidedB;
  }

  float discriminant = b * b - 4 * a * c;

  if (discriminant < 0.0f) {
    return false;
  }

  float sqrtDisc = sqrtf(discriminant);
  float t1 = (-b - sqrtDisc) / (2.0f * a);
  float t2 = (-b + sqrtDisc) / (2.0f * a);

  if (t1 > t2) {
    std::swap(t1, t2);
  }

  float k1 = m * t1 + n;
  float k2 = m * t2 + n;
  if (k1 < 0.0f || k2 < 0.0) {
    tA.SetPosition(aPos);
    return RaySphereIntersection(r, tA, sphere, collision);
  } else if (k1 > 1.0f || k2 > 1.0f) {
    tA.SetPosition(bPos);
    return RaySphereIntersection(r, tA, sphere, collision);
  }

  collision.rayDistance = t1;
  collision.collidedAt = r.GetPosition() + r.GetDirection() * t1;
  return true;
}

bool CollisionDetection::ObjectIntersection(GameObject *a, GameObject *b,
                                            CollisionInfo &collisionInfo) {
  const CollisionVolume *volA = a->GetBoundingVolume();
  const CollisionVolume *volB = b->GetBoundingVolume();

  if (!volA || !volB) {
    return false;
  }

  collisionInfo.a = a;
  collisionInfo.b = b;

  Transform &transformA = a->GetTransform();
  Transform &transformB = b->GetTransform();

  // Quick check first based on max extents
  auto relPos = transformB.GetPosition() - transformA.GetPosition();
  auto distSq = Vector::Dot(relPos, relPos);

  auto maxExtent = volA->GetMaxExtent() + volB->GetMaxExtent();

  if (distSq > maxExtent * maxExtent) {
    return false;
  }

#define A(FN, TYPEA, TYPEB)                                                    \
  FN((TYPEA &)*volA, transformA, (TYPEB &)*volB, transformB, collisionInfo);

#define B(FN, TYPEB, TYPEA)                                                    \
  collisionInfo.a = b, collisionInfo.b = a,                                    \
  FN((TYPEB &)*volB, transformB, (TYPEA &)*volA, transformA, collisionInfo);

  switch (volA->type) {
  case VolumeType::AABB:
    switch (volB->type) {
    case VolumeType::AABB:
      return A(AABBIntersection, AABBVolume, AABBVolume);
    case VolumeType::Sphere:
      return A(AABBSphereIntersection, AABBVolume, SphereVolume);
    case VolumeType::OBB:
      return A(AABBOBBIntersection, AABBVolume, OBBVolume);
    case VolumeType::Capsule:
      return A(AABBCapsuleIntersection, AABBVolume, CapsuleVolume);
    }
    break;

  case VolumeType::Sphere:
    switch (volB->type) {
    case VolumeType::AABB:
      return B(AABBSphereIntersection, AABBVolume, SphereVolume);
    case VolumeType::Sphere:
      return A(SphereIntersection, SphereVolume, SphereVolume);
    case VolumeType::OBB:
      return B(OBBSphereIntersection, OBBVolume, SphereVolume);
    case VolumeType::Capsule:
      return A(SphereCapsuleIntersection, SphereVolume, CapsuleVolume);
    }
    break;
  case VolumeType::OBB:
    switch (volB->type) {
    case VolumeType::AABB:
      return B(AABBOBBIntersection, AABBVolume, OBBVolume);
    case VolumeType::Sphere:
      return A(OBBSphereIntersection, OBBVolume, SphereVolume);
    case VolumeType::OBB:
      return A(OBBIntersection, OBBVolume, OBBVolume);
    case VolumeType::Capsule:
      return A(OBBCapsuleIntersection, OBBVolume, CapsuleVolume);
    }
    break;
  case VolumeType::Capsule:
    switch (volB->type) {
    case VolumeType::AABB:
      return B(AABBCapsuleIntersection, AABBVolume, CapsuleVolume);
    case VolumeType::Sphere:
      return B(SphereCapsuleIntersection, SphereVolume, CapsuleVolume);
    case VolumeType::OBB:
      return B(OBBCapsuleIntersection, OBBVolume, CapsuleVolume);
    case VolumeType::Capsule:
      return A(CapsuleIntersection, CapsuleVolume, CapsuleVolume);
    }
    break;
  }
  return false;
}

bool CollisionDetection::AABBTest(const Vector3 &posA, const Vector3 &posB,
                                  const Vector3 &halfSizeA,
                                  const Vector3 &halfSizeB) {
  Vector3 delta = posB - posA;
  Vector3 totalSize = halfSizeA + halfSizeB;

  if (abs(delta.x) < totalSize.x && abs(delta.y) < totalSize.y &&
      abs(delta.z) < totalSize.z) {
    return true;
  }
  return false;
}

// AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume &volumeA,
                                          const Transform &worldTransformA,
                                          const AABBVolume &volumeB,
                                          const Transform &worldTransformB,
                                          CollisionInfo &collisionInfo) {
  auto boxAPos = worldTransformA.GetPosition();
  auto boxBPos = worldTransformB.GetPosition();

  auto boxASize = volumeA.GetHalfDimensions();
  auto boxBSize = volumeB.GetHalfDimensions();

  bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);

  if (!overlap) {
    return false;
  }

  static const Vector3 faces[6] = {Vector3(-1, 0, 0), Vector3(1, 0, 0),
                                   Vector3(0, -1, 0), Vector3(0, 1, 0),
                                   Vector3(0, 0, -1), Vector3(0, 0, 1)};

  auto maxA = boxAPos + boxASize;
  auto minA = boxAPos - boxASize;

  auto maxB = boxBPos + boxBSize;
  auto minB = boxBPos - boxBSize;

  float distances[6] = {
      maxB.x - minA.x, // Left
      maxA.x - minB.x, // Right
      maxB.y - minA.y, // Bottom
      maxA.y - minB.y, // Top
      maxB.z - minA.z, // Back
      maxA.z - minB.z  // Front
  };

  float penetration = std::numeric_limits<float>::max();

  Vector3 bestAxis;
  for (int i = 0; i < 6; ++i) {
    if (distances[i] < penetration) {
      penetration = distances[i];
      bestAxis = faces[i];
    }
  }
  collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);
  return true;
}

// Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume &volumeA,
                                            const Transform &worldTransformA,
                                            const SphereVolume &volumeB,
                                            const Transform &worldTransformB,
                                            CollisionInfo &collisionInfo) {
  float radii = volumeA.GetRadius() + volumeB.GetRadius();

  Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();

  float distSq = Vector::Dot(delta, delta);

  if (distSq >= radii * radii) {
    return false;
  }

  float penetration = radii - sqrtf(distSq);
  auto normal = Vector::Normalise(delta);
  auto locA = normal * volumeA.GetRadius();
  auto locB = -normal * volumeB.GetRadius();

  collisionInfo.AddContactPoint(locA, locB, normal, penetration);
  return true;
}

bool CollisionDetection::SphereCapsuleIntersection(
    const SphereVolume &volumeA, const Transform &worldTransformA,
    const CapsuleVolume &volumeB, const Transform &worldTransformB,
    CollisionInfo &collisionInfo) {
  auto spherePos = worldTransformA.GetPosition();

  auto aPos =
      worldTransformB.GetPosition() + (worldTransformB.GetOrientation() *
                                       Vector3(0, volumeB.GetHalfHeight(), 0));
  auto pBos =
      worldTransformB.GetPosition() + (worldTransformB.GetOrientation() *
                                       Vector3(0, -volumeB.GetHalfHeight(), 0));

  auto ab = pBos - aPos;

  auto t = Vector::Dot(spherePos - aPos, ab) / Vector::Dot(ab, ab);

  t = std::clamp(t, 0.0f, 1.0f);

  auto closestPoint = aPos + ab * t;

  Transform trans = worldTransformB;
  trans.SetPosition(closestPoint);

  return SphereIntersection(volumeA, worldTransformA,
                            SphereVolume(volumeB.GetRadius()), trans,
                            collisionInfo);
}

// AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(
    const AABBVolume &volumeA, const Transform &worldTransformA,
    const SphereVolume &volumeB, const Transform &worldTransformB,
    CollisionInfo &collisionInfo) {
  auto boxSize = volumeA.GetHalfDimensions();

  auto delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();

  auto closestPoint = Vector::Clamp(delta, -boxSize, boxSize);

  auto localPoint = delta - closestPoint;

  auto radius = volumeB.GetRadius();

  float distSq = Vector::Dot(localPoint, localPoint);

  if (distSq < radius * radius) {
    float penetration = radius - sqrtf(distSq);
    auto normal = Vector::Normalise(localPoint);
    auto locA = Vector3();
    auto locB = -normal * radius;
    collisionInfo.AddContactPoint(locA, locB, normal, penetration);
    return true;
  }
  return false;
}

bool CollisionDetection::AABBOBBIntersection(const AABBVolume &volumeA,
                                             const Transform &worldTransformA,
                                             const OBBVolume &volumeB,
                                             const Transform &worldTransformB,
                                             CollisionInfo &collisionInfo) {
  OBBVolume boxA(volumeA.GetHalfDimensions());

  return OBBIntersection(boxA, worldTransformA, volumeB, worldTransformB,
                         collisionInfo);
}

bool CollisionDetection::AABBCapsuleIntersection(
    const AABBVolume &volumeA, const Transform &worldTransformA,
    const CapsuleVolume &volumeB, const Transform &worldTransformB,
    CollisionInfo &collisionInfo) {
  auto boxSize = volumeA.GetHalfDimensions();

  auto aPos =
      worldTransformB.GetPosition() + (worldTransformB.GetOrientation() *
                                       Vector3(0, volumeB.GetHalfHeight(), 0));
  auto bPos =
      worldTransformA.GetPosition() + (worldTransformB.GetOrientation() *
                                       Vector3(0, -volumeB.GetHalfHeight(), 0));

  auto ab = bPos - aPos;

  auto t = Vector::Dot(worldTransformA.GetPosition() - aPos, ab) /
           Vector::Dot(ab, ab);

  t = std::clamp(t, 0.0f, 1.0f);

  auto closestPoint = aPos + ab * t - worldTransformA.GetPosition();

  SphereVolume sphere(volumeB.GetRadius());

  Transform trans = worldTransformB;
  trans.SetPosition(closestPoint + worldTransformA.GetPosition());

  return AABBSphereIntersection(volumeA, worldTransformA, sphere, trans,
                                collisionInfo);
}

bool CollisionDetection::OBBIntersection(const OBBVolume &volumeA,
                                         const Transform &worldTransformA,
                                         const OBBVolume &volumeB,
                                         const Transform &worldTransformB,
                                         CollisionInfo &collisionInfo) {
  auto aPos = worldTransformA.GetPosition();
  auto aSize = volumeA.GetHalfDimensions();
  auto aRot = worldTransformA.GetOrientation();

  auto bPos = worldTransformB.GetPosition();
  auto bSize = volumeB.GetHalfDimensions();
  auto bRot = worldTransformB.GetOrientation();

  struct Axes {
    Vector3 right;
    Vector3 up;
    Vector3 forward;

    Axes(const Quaternion &q) {
      constexpr Vector3 UP(0, 1, 0);
      constexpr Vector3 RIGHT(1, 0, 0);
      constexpr Vector3 FORWARD(0, 0, 1);
      right = q * RIGHT;
      up = q * UP;
      forward = q * FORWARD;
    }
  };

  Axes aAxes(aRot);
  Axes bAxes(bRot);

  struct MinMax {
    float min;
    float max;

    bool contains(float value) const { return value >= min && value <= max; }
  };

  auto getMinMax = [&](const Vector3 &axis,
                       std::span<Vector3> corners) -> MinMax {
    MinMax res{
        .min = std::numeric_limits<float>::max(),
        .max = -std::numeric_limits<float>::max(),
    };

    for (const auto &corner : corners) {
      float projection = Vector::Dot(corner, axis);
      res.min = std::min(res.min, projection);
      res.max = std::max(res.max, projection);
    }

    return res;
  };

  auto getCorners = [&](const Vector3 &pos, const Vector3 &size,
                        const Axes &axes) -> std::array<Vector3, 8> {
    std::array<Vector3, 8> corners;
    int index = 0;
    for (int x = -1; x <= 1; x += 2) {
      for (int y = -1; y <= 1; y += 2) {
        for (int z = -1; z <= 1; z += 2) {
          Vector3 corner = pos + axes.right * size.x * (float)x +
                           axes.up * size.y * (float)y +
                           axes.forward * size.z * (float)z;
          corners[index++] = corner;
        }
      }
    }
    return corners;
  };

  auto aCorners = getCorners(aPos, aSize, aAxes);
  auto bCorners = getCorners(bPos, bSize, bAxes);

  std::array<Vector3, 15> testAxes = {
      aAxes.right,
      aAxes.up,
      aAxes.forward,
      bAxes.right,
      bAxes.up,
      bAxes.forward,
      Vector::Cross(aAxes.right, bAxes.right),
      Vector::Cross(aAxes.right, bAxes.up),
      Vector::Cross(aAxes.right, bAxes.forward),
      Vector::Cross(aAxes.up, bAxes.right),
      Vector::Cross(aAxes.up, bAxes.up),
      Vector::Cross(aAxes.up, bAxes.forward),
      Vector::Cross(aAxes.forward, bAxes.right),
      Vector::Cross(aAxes.forward, bAxes.up),
      Vector::Cross(aAxes.forward, bAxes.forward),
  };

  float leastPenetration = std::numeric_limits<float>::max();
  NCL::Maths::Vector3 bestAxis;

  for (const auto &axis : testAxes) {
    if (Vector::Dot(axis, axis) < 0.0001f) {
      continue;
    }
    Vector3 normAxis = Vector::Normalise(axis);
    MinMax aMinMax = getMinMax(normAxis, aCorners);
    MinMax bMinMax = getMinMax(normAxis, bCorners);
    if (!(aMinMax.contains(bMinMax.min) || bMinMax.contains(aMinMax.min))) {
      return false;
    }

    float pen1 = aMinMax.max - bMinMax.min;
    if (pen1 < leastPenetration) {
      leastPenetration = pen1;
      bestAxis = normAxis;
    }
  }

  collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis,
                                leastPenetration);

  return true;
}

bool CollisionDetection::OBBSphereIntersection(const OBBVolume &volumeA,
                                               const Transform &worldTransformA,
                                               const SphereVolume &volumeB,
                                               const Transform &worldTransformB,
                                               CollisionInfo &collisionInfo) {
  return false;
}

bool CollisionDetection::OBBCapsuleIntersection(
    const OBBVolume &volumeA, const Transform &worldTransformA,
    const CapsuleVolume &volumeB, const Transform &worldTransformB,
    CollisionInfo &collisionInfo) {
  return false;
}

bool CollisionDetection::CapsuleIntersection(const CapsuleVolume &volumeA,
                                             const Transform &worldTransformA,
                                             const CapsuleVolume &volumeB,
                                             const Transform &worldTransformB,
                                             CollisionInfo &collisionInfo) {
  auto aaPos =
      worldTransformA.GetPosition() + (worldTransformA.GetOrientation() *
                                       Vector3(0, volumeA.GetHalfHeight(), 0));

  auto abPos =
      worldTransformA.GetPosition() + (worldTransformA.GetOrientation() *
                                       Vector3(0, -volumeA.GetHalfHeight(), 0));

  auto baPos =
      worldTransformB.GetPosition() + (worldTransformB.GetOrientation() *
                                       Vector3(0, volumeB.GetHalfHeight(), 0));
  auto bbPos =
      worldTransformB.GetPosition() + (worldTransformB.GetOrientation() *
                                       Vector3(0, -volumeB.GetHalfHeight(), 0));

  Vector3 d1 = abPos - aaPos;
  Vector3 d2 = bbPos - baPos;
  Vector3 r = aaPos - baPos;

  const float EPS = 1e-6f;

  float a = Vector::Dot(d1, d1);
  float e = Vector::Dot(d2, d2);
  float f = Vector::Dot(d2, r);

  float s = 0.0f;
  float t = 0.0f;

  if (a <= EPS && e <= EPS) {
    // NOOP
  } else if (a <= EPS && e > EPS) {
    t = std::clamp(f / e, 0.0f, 1.0f);
  } else if (e <= EPS) {
    float c = Vector::Dot(d1, r);
    s = std::clamp(-c / a, 0.0f, 1.0f);
  } else {
    float b = Vector::Dot(d1, d2);
    float c = Vector::Dot(d1, r);
    float denom = a * e - b * b;

    if (denom != 0.0f) {
      s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
    } else {
      s = 0.0f;
    }

    t = (b * s + f) / e;

    if (t < 0.0f) {
      t = 0.0f;
      s = std::clamp(-c / a, 0.0f, 1.0f);
    } else if (t > 1.0f) {
      t = 1.0f;
      s = std::clamp((b - c) / a, 0.0f, 1.0f);
    }
  }

  Vector3 c1 = aaPos + d1 * s; // closest point on A segment (world)
  Vector3 c2 = baPos + d2 * t; // closest point on B segment (world)

  SphereVolume sphereA(volumeA.GetRadius());
  SphereVolume sphereB(volumeB.GetRadius());

  Transform transA = worldTransformA;
  Transform transB = worldTransformB;

  transA.SetPosition(c1);
  transB.SetPosition(c2);

  return SphereIntersection(sphereA, transA, sphereB, transB, collisionInfo);
}

Matrix4 GenerateInverseView(const Camera &c) {
  float pitch = c.GetPitch();
  float yaw = c.GetYaw();
  Vector3 position = c.GetPosition();

  Matrix4 iview = Matrix::Translation(position) *
                  Matrix::Rotation(-yaw, Vector3(0, -1, 0)) *
                  Matrix::Rotation(-pitch, Vector3(-1, 0, 0));

  return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float fov, float nearPlane,
                                  float farPlane) {
  float negDepth = nearPlane - farPlane;

  float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

  Matrix4 m;

  float h = 1.0f / tan(fov * PI_OVER_360);

  m.array[0][0] = aspect / h;
  m.array[1][1] = tan(fov * PI_OVER_360);
  m.array[2][2] = 0.0f;

  m.array[2][3] = invNegDepth; //// +PI_OVER_360;
  m.array[3][2] = -1.0f;
  m.array[3][3] = (0.5f / nearPlane) + (0.5f / farPlane);

  return m;
}

Vector3 CollisionDetection::Unproject(const Vector3 &screenPos,
                                      const PerspectiveCamera &cam) {
  Vector2i screenSize = Window::GetWindow()->GetScreenSize();

  float aspect = Window::GetWindow()->GetScreenAspect();
  float fov = cam.GetFieldOfVision();
  float nearPlane = cam.GetNearPlane();
  float farPlane = cam.GetFarPlane();

  // Create our inverted matrix! Note how that to get a correct inverse
  // matrix, the order of matrices used to form it are inverted, too.
  Matrix4 invVP = GenerateInverseView(cam) *
                  GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

  Matrix4 proj = cam.BuildProjectionMatrix(aspect);

  // Our mouse position x and y values are in 0 to screen dimensions range,
  // so we need to turn them into the -1 to 1 axis range of clip space.
  // We can do that by dividing the mouse values by the width and height of
  // the screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0
  // to 2.0) and then subtracting 1 (-1.0 to 1.0).
  Vector4 clipSpace = Vector4((screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
                              (screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
                              (screenPos.z), 1.0f);

  // Then, we multiply our clipspace coordinate by our inverted matrix
  Vector4 transformed = invVP * clipSpace;

  // our transformed w coordinate is now the 'inverse' perspective divide, so
  // we can reconstruct the final world space by dividing x,y,and z by w.
  return Vector3(transformed.x / transformed.w, transformed.y / transformed.w,
                 transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const PerspectiveCamera &cam) {
  Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
  Vector2i screenSize = Window::GetWindow()->GetScreenSize();

  // We remove the y axis mouse position from height as OpenGL is 'upside
  // down', and thinks the bottom left is the origin, instead of the top left!
  Vector3 nearPos =
      Vector3(screenMouse.x, screenSize.y - screenMouse.y, -0.99999f);

  // We also don't use exactly 1.0 (the normalised 'end' of the far plane) as
  // this causes the unproject function to go a bit weird.
  Vector3 farPos =
      Vector3(screenMouse.x, screenSize.y - screenMouse.y, 0.99999f);

  Vector3 a = Unproject(nearPos, cam);
  Vector3 b = Unproject(farPos, cam);
  Vector3 c = b - a;

  c = Vector::Normalise(c);

  return Ray(cam.GetPosition(), c);
}

// http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov,
                                                      float nearPlane,
                                                      float farPlane) {
  Matrix4 m;

  float t = tan(fov * PI_OVER_360);

  float neg_depth = nearPlane - farPlane;

  const float h = 1.0f / t;

  float c = (farPlane + nearPlane) / neg_depth;
  float e = -1.0f;
  float d = 2.0f * (nearPlane * farPlane) / neg_depth;

  m.array[0][0] = aspect / h;
  m.array[1][1] = tan(fov * PI_OVER_360);
  m.array[2][2] = 0.0f;

  m.array[2][3] = 1.0f / d;

  m.array[3][2] = 1.0f / e;
  m.array[3][3] = -c / (d * e);

  return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
  float pitch = c.GetPitch();
  float yaw = c.GetYaw();
  Vector3 position = c.GetPosition();

  Matrix4 iview = Matrix::Translation(position) *
                  Matrix::Rotation(yaw, Vector3(0, 1, 0)) *
                  Matrix::Rotation(pitch, Vector3(1, 0, 0));

  return iview;
}

/*
If you've read through the Deferred Rendering tutorial you should have a
pretty good idea what this function does. It takes a 2D position, such as the
mouse position, and 'unprojects' it, to generate a 3D world space position for
it.

Just as we turn a world space position into a clip space position by
multiplying it by the model, view, and projection matrices, we can turn a clip
space position back to a 3D position by multiply it by the INVERSE of the view
projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can
cheat the inversion process again, just like we do when we create a view
matrix using the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3 CollisionDetection::UnprojectScreenPosition(
    Vector3 position, float aspect, float fov, const PerspectiveCamera &c) {
  // Create our inverted matrix! Note how that to get a correct inverse
  // matrix, the order of matrices used to form it are inverted, too.
  Matrix4 invVP =
      GenerateInverseView(c) *
      GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());

  Vector2i screenSize = Window::GetWindow()->GetScreenSize();

  // Our mouse position x and y values are in 0 to screen dimensions range,
  // so we need to turn them into the -1 to 1 axis range of clip space.
  // We can do that by dividing the mouse values by the width and height of
  // the screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0
  // to 2.0) and then subtracting 1 (-1.0 to 1.0).
  Vector4 clipSpace = Vector4((position.x / (float)screenSize.x) * 2.0f - 1.0f,
                              (position.y / (float)screenSize.y) * 2.0f - 1.0f,
                              (position.z) - 1.0f, 1.0f);

  // Then, we multiply our clipspace coordinate by our inverted matrix
  Vector4 transformed = invVP * clipSpace;

  // our transformed w coordinate is now the 'inverse' perspective divide, so
  // we can reconstruct the final world space by dividing x,y,and z by w.
  return Vector3(transformed.x / transformed.w, transformed.y / transformed.w,
                 transformed.z / transformed.w);
}
