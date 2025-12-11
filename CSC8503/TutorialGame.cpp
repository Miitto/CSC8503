#include "TutorialGame.h"
#include "GameWorld.h"
#include "LevelEnd.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "gui.h"
#include "physics/PhysicsObject.h"
#include "physics/PhysicsSystem.h"

#include "Oscillator.h"
#include "StateGameObject.h"
#include "constraints/OrientationConstraint.h"
#include "constraints/PositionConstraint.h"
#include "constraints/TiedConstraint.h"

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "Debug.h"

#include "KeyboardMouseController.h"

#include "GameTechRendererInterface.h"

#include "collisions/Ray.h"

#include <imgui/imgui.h>

#include "logging/log.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame(GameWorld &inWorld,
                           GameTechRendererInterface &inRenderer,
                           PhysicsSystem &inPhysics)
    : world(inWorld), renderer(inRenderer), physics(inPhysics) {
  useGravity = true;
  freeCursor = false;

  world.SetSunPosition({-200.0f, 60.0f, -200.0f});
  world.SetSunColour({0.8f, 0.8f, 0.5f});

  cubeMesh = renderer.LoadMesh("cube.msh");
  sphereMesh = renderer.LoadMesh("sphere.msh");
  enemyMesh = renderer.LoadMesh("ORIGAMI_Chat.msh");
  kittenMesh = renderer.LoadMesh("Kitten.msh");

  playerMesh = renderer.LoadMesh("Keeper.msh");

  bonusMesh = renderer.LoadMesh("19463_Kitten_Head_v1.msh");
  capsuleMesh = renderer.LoadMesh("capsule.msh");

  defaultTex = renderer.LoadTexture("One.png");
  checkerTex = renderer.LoadTexture("checkerboard.png");
  glassTex = renderer.LoadTexture("stainedglass.tga");
  paleGreenTex = renderer.LoadTexture("PaleGreen.png");
  crosshairTex = renderer.LoadTexture("Crosshair.png");

  checkerMaterial.type = MaterialType::Opaque;
  checkerMaterial.diffuseTex = checkerTex;

  glassMaterial.type = MaterialType::Transparent;
  glassMaterial.diffuseTex = glassTex;
}

TutorialGame::~TutorialGame() {}

void TutorialGame::EndLevel() { Clear(); }

void TutorialGame::UpdateGame(float dt) {
  if (!active)
    return;

  if (shouldEndLevel) {
    EndLevel();
    shouldEndLevel = false;
  }

  if (updateCamera && !freeCursor) {
    auto *cam = world.GetMainCamera();
    if (cam)
      cam->UpdateCamera(dt);
  }

  if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
    freeCursor = !freeCursor;
    if (freeCursor) {
      Window::GetWindow()->ShowOSPointer(true);
      Window::GetWindow()->LockMouseToWindow(false);
    } else {
      Window::GetWindow()->ShowOSPointer(false);
      Window::GetWindow()->LockMouseToWindow(true);
    }
  }

  Debug::DrawTex(*crosshairTex, Vector2(50, 50), Vector2(1, 1));
  Debug::Print(
      fmt::format("Press Q to {} the cursor", freeCursor ? "lock" : "free"),
      Vector2(2, 95));

  DebugUi();

  world.OperateOnContents([dt](GameObject *o) { o->Update(dt); });
}

void TutorialGame::DebugUi() {
  if (!showUi)
    return;
  {
    auto frame = NCL::gui::Frame("Tutorial Game Debug");
    if (ImGui::Checkbox("Use Gravity", &useGravity))
      physics.UseGravity(useGravity);

    if (useGravity) {
      ImGui::InputFloat3("Gravity", &physics.GetGravity().x);
    }
  }

  {
    auto frame = NCL::gui::Frame("Camera");
    auto *cam = world.GetMainCamera();
    if (cam) {
      auto pos = cam->GetPosition();
      frame.text("Camera Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    }
  }
}

void TutorialGame::Clear() {
  world.ClearAndErase();
  physics.Clear();

  pane = nullptr;
  player = nullptr;
  world.SetMainCamera(nullptr);
}

void TutorialGame::InitLvlOne() {
  Clear();

  pane = AddPaneToWorld(Vector3(10, 5, 0), Vector2(4, 2), .5f);
  pane->SetupConstraints(world);

  NavigationMesh navMesh("lvl1.navmesh");
  world.pathfind().add(std::move(navMesh));

  struct Floor {
    Vector3 position;
    Vector3 scale;
  };

  constexpr Floor floors[] = {
      {.position = {0, 0, 0}, .scale = {20, 1, 20}},
      {.position = {20, 0, 0}, .scale = {20, 1, 2}},
      {.position = {31, 0, -9}, .scale = {2, 1, 20}},
      {.position = {42, 0, -18}, .scale = {20, 1, 2}},
      {.position = {53, 0, -9}, .scale = {2, 1, 20}},
      {.position = {64, 0, 0}, .scale = {20, 1, 2}},
      {.position = {84, 0, 0}, .scale = {20, 1, 20}},
      {.position = {-10.5, 9.5, 0}, .scale = {1, 20, 2}},
      {.position = {94.5, 9.5, 0}, .scale = {1, 20, 2}},
      {.position = {42, 20, 0}, .scale = {106, 1, 2}},
  };

  for (const Floor &floor : floors) {
    AddCubeToWorld(floor.position, floor.scale, 0.0f);
  }

  AddLevelEndToWorld(Vector3(84, 5, 0), Vector3(5, 5, 5));

  GameObject *pendulumAttachment =
      AddCubeToWorld({42, 50, -18}, {5, 2, 5}, 0.0f);
  GameObject *pendulum = AddSphereToWorld({42, 50, -70}, 5.0f, 0.0001f);
  pendulum->GetPhysicsObject()->GetMaterial().SetLinearDamping(0);
  pendulum->GetPhysicsObject()->SetAxisLocks(PhysicsObject::AxisLock::LinearX);
  world.AddConstraint(new TiedConstraint(pendulumAttachment, pendulum, 44));
}

void TutorialGame::InitLvlTwo() {
  Clear();

  AddFloorToWorld(Vector3(0, -5, 0));
  AddFloorToWorld(Vector3(0, 10, 0));

  NavigationGrid grid("lvl2.txt", {-50, -5.5, -90});
  AddNavigationGridToWorld(grid);
  world.pathfind().add(std::move(grid));

  pane = AddPaneToWorld(Vector3(0, 3.5, -12), Vector2(4, 2), .5f);
  pane->SetupConstraints(world);

  AddLevelEndToWorld(Vector3(-35, 2, -80), Vector3(5, 5, 5));

  {
    Enemy *e = AddEnemyToWorld({32.5, 0, -67.5}, 500);
    e->AddPatrolPoint({32.5, 0, -67.5});
    e->AddPatrolPoint({-37.5, 0, -22.5});
  }

  {
    Enemy *e = AddEnemyToWorld({17.5, 0, -37.5}, 501);
    e->AddPatrolPoint({17.5, 0, -37.5});
    e->AddPatrolPoint({-2.5, 0, -47.5});
  }
}

Player *TutorialGame::SpawnPlayer(int id) {
  DEBUG("Spawning player with ID {}", id);
  Vector3 pos = Vector3(0, 1, id * 2);
  return AddPlayerToWorld(pos, id);
}

void TutorialGame::RemovePlayer(int id) {
  for (auto &player : world.GetPlayerRange()) {
    if (player.first == id) {
      world.RemovePlayerObject(player.second, true);
      DEBUG("Removed player with ID {}", id);
      return;
    }
  }
}

#pragma region World Building Functions
/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject *TutorialGame::AddFloorToWorld(const Vector3 &position) {
  GameObject *floor = new GameObject();

  Vector3 floorSize = Vector3(200, 2, 200);
  AABBVolume *volume = new AABBVolume(floorSize);
  floor->SetBoundingVolume(volume);
  floor->GetTransform().SetScale(floorSize * 2.0f).SetPosition(position);

  floor->SetRenderObject(
      new RenderObject(floor->GetTransform(), cubeMesh, checkerMaterial));
  floor->SetPhysicsObject(
      new PhysicsObject(floor->GetTransform(), floor->GetBoundingVolume()));

  floor->GetPhysicsObject()->SetInverseMass(0);
  floor->GetPhysicsObject()->InitCubeInertia();

  world.AddGameObject(floor);

  return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding
sphere for its rigid body representation. This and the cube function will let
you build a lot of 'simple' physics worlds. You'll probably need another
function for the creation of OBB cubes too.

*/
GameObject *TutorialGame::AddSphereToWorld(const Vector3 &position,
                                           float radius, float inverseMass) {
  GameObject *sphere = new GameObject();

  Vector3 sphereSize = Vector3(radius, radius, radius);
  SphereVolume *volume = new SphereVolume(radius);
  sphere->SetBoundingVolume(volume);

  sphere->GetTransform().SetScale(sphereSize).SetPosition(position);

  sphere->SetRenderObject(
      new RenderObject(sphere->GetTransform(), sphereMesh, checkerMaterial));
  sphere->SetPhysicsObject(
      new PhysicsObject(sphere->GetTransform(), sphere->GetBoundingVolume()));

  sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
  sphere->GetPhysicsObject()->InitSphereInertia();

  sphere->SetCurrentTransformAsReset();

  world.AddGameObject(sphere);

  return sphere;
}

GameObject *TutorialGame::AddCubeToWorld(const Vector3 &position,
                                         Vector3 dimensions, float inverseMass,
                                         GameObject *cube) {

  AABBVolume *volume = new AABBVolume(dimensions * 0.5f);
  cube->SetBoundingVolume(volume);

  cube->GetTransform().SetPosition(position).SetScale(dimensions);

  cube->SetRenderObject(
      new RenderObject(cube->GetTransform(), cubeMesh, checkerMaterial));
  cube->SetPhysicsObject(
      new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));

  cube->GetPhysicsObject()->SetInverseMass(inverseMass);
  cube->GetPhysicsObject()->InitCubeInertia();

  cube->SetCurrentTransformAsReset();

  world.AddGameObject(cube);

  return cube;
}

GameObject *TutorialGame::AddOBBToWorld(const Vector3 &position,
                                        Vector3 dimensions,
                                        Quaternion orientation,
                                        float inverseMass) {
  GameObject *obb = new GameObject();
  OBBVolume *volume = new OBBVolume(dimensions);
  obb->SetBoundingVolume(volume);
  obb->GetTransform()
      .SetPosition(position)
      .SetScale(dimensions * 2.0f)
      .SetOrientation(orientation);
  obb->SetRenderObject(
      new RenderObject(obb->GetTransform(), cubeMesh, checkerMaterial));
  obb->SetPhysicsObject(
      new PhysicsObject(obb->GetTransform(), obb->GetBoundingVolume()));
  obb->GetPhysicsObject()->SetInverseMass(inverseMass);
  obb->GetPhysicsObject()->InitCubeInertia();
  world.AddGameObject(obb);
  obb->SetCurrentTransformAsReset();
  return obb;
}

GameObject *TutorialGame::AddCapsuleToWorld(const Vector3 &position,
                                            float halfHeight, float radius,
                                            Quaternion orientation,
                                            float inverseMass) {
  GameObject *capsule = new GameObject();
  CapsuleVolume *volume = new CapsuleVolume(halfHeight, radius);
  capsule->SetBoundingVolume(volume);
  capsule->GetTransform()
      .SetPosition(position)
      .SetScale(Vector3(radius * 2, halfHeight * 2, radius * 2))
      .SetOrientation(orientation);
  capsule->SetRenderObject(
      new RenderObject(capsule->GetTransform(), capsuleMesh, checkerMaterial));
  capsule->SetPhysicsObject(
      new PhysicsObject(capsule->GetTransform(), capsule->GetBoundingVolume()));
  capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
  capsule->GetPhysicsObject()->InitSphereInertia();
  world.AddGameObject(capsule);

  capsule->SetCurrentTransformAsReset();
  return capsule;
}

StateGameObject *TutorialGame::AddStateObjectToWorld(const Vector3 &position) {
  StateGameObject *apple = new StateGameObject();

  SphereVolume *volume = new SphereVolume(0.5f);
  apple->SetBoundingVolume(volume);
  apple->GetTransform().SetScale(Vector3(2, 2, 2)).SetPosition(position);

  apple->SetRenderObject(
      new RenderObject(apple->GetTransform(), bonusMesh, glassMaterial));
  apple->SetPhysicsObject(
      new PhysicsObject(apple->GetTransform(), apple->GetBoundingVolume()));

  apple->GetPhysicsObject()->SetInverseMass(1.0f);
  apple->GetPhysicsObject()->InitSphereInertia();

  world.AddGameObject(apple);

  return apple;
}

Player *TutorialGame::AddPlayerToWorld(const Vector3 &position, int id) {
  float meshSize = 1.0f;
  float inverseMass = 0.5f;

  auto character = new Player(id, &world, pane);
  auto volume = new SphereVolume(0.75f * meshSize);

  character->SetBoundingVolume(volume);

  character->GetTransform()
      .SetScale(Vector3(meshSize, meshSize, meshSize))
      .SetPosition(position);

  character->SetRenderObject(
      new RenderObject(character->GetTransform(), playerMesh, notexMaterial));
  character->SetPhysicsObject(new PhysicsObject(
      character->GetTransform(), character->GetBoundingVolume()));

  character->GetPhysicsObject()->SetInverseMass(inverseMass);
  character->GetPhysicsObject()->InitSphereInertia();

  character->GetPhysicsObject()->GetMaxLinearVelocity() = 20.f;
  character->GetPhysicsObject()->GetMaxAngularVelocity() = 10.f;

  world.AddPlayerObject(character);

  character->SetCurrentTransformAsReset();

  return character;
}

Enemy *TutorialGame::AddEnemyToWorld(const Vector3 &position, int id) {
  float meshSize = 2.0f;
  float inverseMass = 5.f;

  Enemy *character = new Enemy(world, id);

  SphereVolume *volume = new SphereVolume(0.1f * meshSize);
  character->SetBoundingVolume(volume);

  character->GetTransform()
      .SetScale(Vector3(meshSize, meshSize, meshSize))
      .SetPosition(position);

  character->SetRenderObject(
      new RenderObject(character->GetTransform(), enemyMesh, notexMaterial));
  character->SetPhysicsObject(new PhysicsObject(
      character->GetTransform(), character->GetBoundingVolume()));

  character->GetPhysicsObject()->SetInverseMass(inverseMass);
  character->GetPhysicsObject()->InitSphereInertia();
  character->GetPhysicsObject()->GetMaterial().SetLinearDamping(5.f);

  character->SetCurrentTransformAsReset();

  world.AddGameObject(character);

  return character;
}

GameObject *TutorialGame::AddBonusToWorld(const Vector3 &position) {
  GameObject *apple = new GameObject();

  SphereVolume *volume = new SphereVolume(0.5f);
  volume->SetTrigger();
  apple->SetBoundingVolume(volume);
  apple->GetTransform().SetScale(Vector3(2, 2, 2)).SetPosition(position);

  apple->SetRenderObject(
      new RenderObject(apple->GetTransform(), bonusMesh, glassMaterial));
  apple->SetPhysicsObject(
      new PhysicsObject(apple->GetTransform(), apple->GetBoundingVolume()));

  apple->GetPhysicsObject()->SetInverseMass(1.0f);
  apple->GetPhysicsObject()->InitSphereInertia();

  world.AddGameObject(apple);

  return apple;
}

Pane *TutorialGame::AddPaneToWorld(const Vector3 &position,
                                   const Vector2 &dimensions, float invMass) {
  Pane *pane = new Pane(&world, player);
  auto *volume =
      new OBBVolume(Vector3(dimensions.x * 0.5f, 0.1f, dimensions.y * 0.5f));
  pane->SetBoundingVolume(volume);
  pane->GetTransform()
      .SetScale(Vector3(dimensions.x, 0.1f, dimensions.y))
      .SetPosition(position);
  GameTechMaterial paneMaterial;
  paneMaterial.type = MaterialType::Transparent;
  paneMaterial.diffuseTex = glassTex;
  pane->SetRenderObject(
      new RenderObject(pane->GetTransform(), cubeMesh, paneMaterial));
  pane->SetPhysicsObject(
      new PhysicsObject(pane->GetTransform(), pane->GetBoundingVolume()));
  pane->GetPhysicsObject()->SetInverseMass(invMass);
  pane->GetPhysicsObject()->InitCubeInertia();
  pane->GetPhysicsObject()->SetAxisLocks(PhysicsObject::AxisLock::LinearX |
                                         PhysicsObject::AxisLock::LinearY |
                                         PhysicsObject::AxisLock::LinearZ);

  pane->SetCurrentTransformAsReset();

  world.AddGameObject(pane);
  return pane;
}

GameObject *TutorialGame::AddLevelEndToWorld(const Vector3 &position,
                                             const Vector3 &scale) {
  LevelEnd *levelEnd = new LevelEnd(*this);
  auto *volume = new AABBVolume(scale * 0.5f);
  volume->SetTrigger();
  levelEnd->SetBoundingVolume(volume);
  levelEnd->GetTransform().SetPosition(position).SetScale(scale);

  GameTechMaterial levelEndMaterial;
  levelEndMaterial.type = MaterialType::Transparent;
  levelEndMaterial.diffuseTex = paleGreenTex;

  levelEnd->SetRenderObject(
      new RenderObject(levelEnd->GetTransform(), cubeMesh, levelEndMaterial));
  levelEnd->SetPhysicsObject(new PhysicsObject(levelEnd->GetTransform(),
                                               levelEnd->GetBoundingVolume()));
  levelEnd->GetPhysicsObject()->SetInverseMass(0.0f);
  world.AddGameObject(levelEnd);

  return levelEnd;
}

void TutorialGame::AddNavigationGridToWorld(const NavigationGrid &grid) {
  int chunkCount = (grid.GetHeight() / 64) + ((grid.GetHeight() % 64) ? 1 : 0);

  std::vector<std::vector<uint64_t>> chunks;

  for (int chunk = 0; chunk <= grid.GetHeight() / 64; ++chunk) {
    auto &columns = chunks.emplace_back(grid.GetWidth(), 0);
    int maxY = std::min(64, grid.GetHeight() - (chunk * 64));
    for (uint64_t x = 0; x < grid.GetWidth(); ++x) {
      for (uint64_t y = 0; y < maxY; ++y) {
        if (!grid.isNodeWalkable(y, x)) {
          columns[x] |= (static_cast<unsigned long long>(1u) << y);
        }
      }
    }
  }

  for (auto &columnData : chunks) {
    for (auto &data : columnData) {
      DEBUG("{:b}", data);
    }
  }

  for (int chunk = 0; chunk < chunkCount; ++chunk) {
    auto &columns = chunks[chunk];
    int maxY = std::min(64, grid.GetHeight() - (chunk * 64));
    for (int row = 0; row < grid.GetWidth(); ++row) {
      uint64_t columnData = columns[row];
      if (columnData == 0)
        continue;

      int y = 0;

      while (y < maxY) {
        y += std::countr_zero(columns[row] >> y);

        if (y >= maxY)
          break;

        int h = std::countr_one(columnData >> y);

        int hMask = h > 63 ? ~0 : (1 << h) - 1;
        int mask = hMask << y;

        int w = 1;

        while (row + w < grid.GetWidth()) {
          uint64_t nextColumnData = columns[row + w];
          uint64_t nextColumn = (nextColumnData >> y) & hMask;

          if (nextColumn != hMask) {
            break;
          }

          columns[row + w] &= ~mask;

          w += 1;
        }

        if (w > 0 && h > 0) {
          float xPos = grid.GetOrigin().z + row * grid.GetNodeSize();
          float yPos = grid.GetOrigin().y;
          float zPos =
              grid.GetOrigin().x + (chunk * 64 + y) * grid.GetNodeSize();
          float width = static_cast<float>(w * grid.GetNodeSize());
          float height = static_cast<float>(h * grid.GetNodeSize());
          Vector3 pos(zPos + height * 0.5f, yPos + (grid.GetNodeSize()),
                      xPos + width * 0.5f);
          AddCubeToWorld(
              pos, {height, static_cast<float>(grid.GetNodeSize()), width},
              0.f);
        }
        y += h;
      }
    }
  }
}
#pragma endregion

#pragma region Examples and Tests
void TutorialGame::CreateSphereGrid(int numRows, int numCols, float rowSpacing,
                                    float colSpacing, float radius) {
  for (int x = 0; x < numCols; ++x) {
    for (int z = 0; z < numRows; ++z) {
      Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
      AddSphereToWorld(position, radius, 1.0f);
    }
  }
  AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::CreatedMixedGrid(int numRows, int numCols, float rowSpacing,
                                    float colSpacing) {
  float sphereRadius = 1.0f;
  Vector3 cubeDims = Vector3(1, 1, 1);

  for (int x = 0; x < numCols; ++x) {
    for (int z = 0; z < numRows; ++z) {
      Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

      if (rand() % 2) {
        AddCubeToWorld(position, cubeDims);
      } else {
        AddSphereToWorld(position, sphereRadius);
      }
    }
  }
}

void TutorialGame::CreateAABBGrid(int numRows, int numCols, float rowSpacing,
                                  float colSpacing, const Vector3 &cubeDims) {
  for (int x = 1; x < numCols + 1; ++x) {
    for (int z = 1; z < numRows + 1; ++z) {
      Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
      AddCubeToWorld(position, cubeDims, 1.0f);
    }
  }
}

void TutorialGame::BridgeConstraintTest() {
  const Vector3 cubeDims = Vector3(8, 8, 8);

  constexpr float invCubeMass = 5.0f;
  constexpr int numLinks = 10;
  constexpr float maxDistance = 30.f;
  constexpr float cubeDistance = 20.f;

  const Vector3 startPos(300, 50, 300);

  auto start = AddCubeToWorld(startPos, cubeDims, 0.0f);
  auto end = AddCubeToWorld(
      startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeDims, 0.0f);

  auto prev = start;

  for (int i = 0; i < numLinks; ++i) {
    auto link = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0),
                               cubeDims, invCubeMass);
    auto constraint = new TiedConstraint(prev, link, maxDistance);
    world.AddConstraint(constraint);
    prev = link;
  }

  auto constraint = new TiedConstraint(prev, end, maxDistance);
  world.AddConstraint(constraint);
}
#pragma endregion
