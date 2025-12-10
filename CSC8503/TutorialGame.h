#pragma once
#include "Enemy.h"
#include "Pane.h"
#include "Player.h"
#include "RenderObject.h"
#include "StateGameObject.h"

namespace NCL {
class Controller;

namespace Rendering {
class Mesh;
class Texture;
class Shader;
} // namespace Rendering
namespace CSC8503 {
class GameTechRendererInterface;
class PhysicsSystem;
class GameWorld;
class GameObject;

class TutorialGame {
public:
  TutorialGame(GameWorld &gameWorld, GameTechRendererInterface &renderer,
               PhysicsSystem &physics);
  ~TutorialGame();

  virtual void UpdateGame(float dt);

  void RequestEndLevel() { shouldEndLevel = true; }

  void Clear();
  void InitWorld();
  void InitCollisionTest();

  Player *SpawnPlayer(int id);

  void SetActive(bool state) { active = state; }
  void SetCameraActive(bool state) { updateCamera = state; }
  void SetShowUi(bool state) { showUi = state; }

protected:
  void InitCamera();
  virtual void EndLevel();

  /*
  These are some of the world/object creation functions I created when testing
  the functionality in the module. Feel free to mess around with them to see
  different objects being created in different test scenarios (constraints,
  collision types, and so on).
  */
  void CreateSphereGrid(int numRows, int numCols, float rowSpacing,
                        float colSpacing, float radius);
  void CreatedMixedGrid(int numRows, int numCols, float rowSpacing,
                        float colSpacing);
  void CreateAABBGrid(int numRows, int numCols, float rowSpacing,
                      float colSpacing, const NCL::Maths::Vector3 &cubeDims);

  void BridgeConstraintTest();

  void DebugUi();

  GameObject *AddFloorToWorld(const NCL::Maths::Vector3 &position);
  GameObject *AddSphereToWorld(const NCL::Maths::Vector3 &position,
                               float radius, float inverseMass = 10.0f);
  GameObject *AddCubeToWorld(const NCL::Maths::Vector3 &position,
                             NCL::Maths::Vector3 dimensions,
                             float inverseMass = 10.0f,
                             GameObject *obj = new GameObject());
  GameObject *AddOBBToWorld(const NCL::Maths::Vector3 &position,
                            NCL::Maths::Vector3 dimensions,
                            NCL::Maths::Quaternion orientation,
                            float inverseMass = 10.0f);

  GameObject *AddCapsuleToWorld(const NCL::Maths::Vector3 &position,
                                float halfHeight, float radius,
                                Quaternion orientation,
                                float inverseMass = 10.0f);

  GameObject *AddLevelEndToWorld(const NCL::Maths::Vector3 &position,
                                 const NCL::Maths::Vector3 &dimensions);

  void AddNavigationGridToWorld(const NavigationGrid &grid);

  StateGameObject *AddStateObjectToWorld(const NCL::Maths::Vector3 &position);

  Player *AddPlayerToWorld(const NCL::Maths::Vector3 &position, int id);
  GameObject *AddEnemyToWorld(const NCL::Maths::Vector3 &position);
  GameObject *AddBonusToWorld(const NCL::Maths::Vector3 &position);

  Pane *AddPaneToWorld(const NCL::Maths::Vector3 &position,
                       const NCL::Maths::Vector2 &dimensions, float invMass);

  bool active = true;
  bool updateCamera = true;
  bool showUi = true;

  GameWorld &world;
  GameTechRendererInterface &renderer;
  PhysicsSystem &physics;
  Controller *controller;

  bool useGravity;
  bool freeCursor;
  bool shouldEndLevel = false;

  Rendering::Mesh *capsuleMesh = nullptr;
  Rendering::Mesh *cubeMesh = nullptr;
  Rendering::Mesh *sphereMesh = nullptr;

  Rendering::Texture *defaultTex = nullptr;
  Rendering::Texture *checkerTex = nullptr;
  Rendering::Texture *glassTex = nullptr;
  Rendering::Texture *paleGreenTex = nullptr;
  Rendering::Texture *crosshairTex = nullptr;

  // Coursework Meshes
  Rendering::Mesh *playerMesh = nullptr;
  Rendering::Mesh *kittenMesh = nullptr;
  Rendering::Mesh *enemyMesh = nullptr;
  Rendering::Mesh *bonusMesh = nullptr;

  GameTechMaterial checkerMaterial;
  GameTechMaterial glassMaterial;
  GameTechMaterial notexMaterial;

  // Coursework Additional functionality
  Pane *pane;
  Player *player;
};
} // namespace CSC8503
} // namespace NCL