#include "TutorialGame.h"
#include "GameWorld.h"
#include "RenderObject.h"
#include "TextureLoader.h"
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

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame(GameWorld &inWorld,
                           GameTechRendererInterface &inRenderer,
                           PhysicsSystem &inPhysics)
    : world(inWorld), renderer(inRenderer), physics(inPhysics) {

  forceMagnitude = 10.0f;
  useGravity = false;
  inSelectionMode = false;

  controller = new KeyboardMouseController(*Window::GetWindow()->GetKeyboard(),
                                           *Window::GetWindow()->GetMouse());

  world.GetMainCamera().SetController(*controller);

  world.SetSunPosition({-200.0f, 60.0f, -200.0f});
  world.SetSunColour({0.8f, 0.8f, 0.5f});

  controller->MapAxis(0, "Sidestep");
  controller->MapAxis(1, "UpDown");
  controller->MapAxis(2, "Forward");

  controller->MapAxis(3, "XLook");
  controller->MapAxis(4, "YLook");

  cubeMesh = renderer.LoadMesh("cube.msh");
  sphereMesh = renderer.LoadMesh("sphere.msh");
  catMesh = renderer.LoadMesh("ORIGAMI_Chat.msh");
  kittenMesh = renderer.LoadMesh("Kitten.msh");

  enemyMesh = renderer.LoadMesh("Keeper.msh");

  bonusMesh = renderer.LoadMesh("19463_Kitten_Head_v1.msh");
  capsuleMesh = renderer.LoadMesh("capsule.msh");

  defaultTex = renderer.LoadTexture("Default.png");
  checkerTex = renderer.LoadTexture("checkerboard.png");
  glassTex = renderer.LoadTexture("stainedglass.tga");

  checkerMaterial.type = MaterialType::Opaque;
  checkerMaterial.diffuseTex = checkerTex;

  glassMaterial.type = MaterialType::Transparent;
  glassMaterial.diffuseTex = glassTex;

  InitCamera();
  InitWorld();
}

TutorialGame::~TutorialGame() {}

void TutorialGame::UpdateGame(float dt) {
  if (!inSelectionMode) {
    world.GetMainCamera().UpdateCamera(dt);
  }
  if (pane) {
    auto camPos = world.GetMainCamera().GetPosition();
    auto pos = pane->GetTransform().GetPosition();

    auto relPos = pos - camPos;
    auto dir = Vector::Normalise(relPos);

    auto distSq = Vector::Dot(relPos, relPos);

    Ray ray(camPos, dir);
    RayCollision rayCollision;
    world.Raycast(ray, rayCollision, true, pane);
    if (rayCollision.rayDistance * rayCollision.rayDistance < distSq) {
      camPos = rayCollision.collidedAt + dir * 0.5f;
      world.GetMainCamera().SetPosition(camPos);
    }

    NCL::Debug::DrawLine(camPos, pos, NCL::Debug::GREEN);
  }

  if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
    InitWorld(); // We can reset the simulation at any time with F1
    selectionObject = nullptr;
  }

  if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
    InitCamera(); // F2 will reset the camera to a specific default place
  }

  if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
    useGravity = !useGravity; // Toggle gravity!
    physics.UseGravity(useGravity);
  }
  // Running certain physics updates in a consistent order might cause some
  // bias in the calculations - the same objects might keep 'winning' the
  // constraint allowing the other one to stretch too much etc. Shuffling the
  // order so that it is random every frame can help reduce such bias.
  if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
    world.ShuffleConstraints(true);
  }
  if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
    world.ShuffleConstraints(false);
  }

  if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
    world.ShuffleObjects(true);
  }
  if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
    world.ShuffleObjects(false);
  }

  DebugObjectMovement();

  if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
    auto obj = world.ObjectLookAt(selectionObject);
    if (obj) {
      if (objClosest) {
        objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
      }
      objClosest = obj.object;

      objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
    }
  }

  // This year we can draw debug textures as well!
  Debug::DrawTex(*defaultTex, Vector2(10, 10), Vector2(5, 5), Debug::WHITE);
  Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

  if (useGravity) {
    Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
  } else {
    Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
  }

  SelectObject();
  MoveSelectedObject();

  DebugUi();

  world.OperateOnContents([dt](GameObject *o) { o->Update(dt); });
}

void TutorialGame::DebugUi() {
  ImGui::Begin("Tutorial Game Debug");
  if (ImGui::Checkbox("Use (G)ravity", &useGravity))
    physics.UseGravity(useGravity);

  if (useGravity) {
    ImGui::InputFloat3("Gravity", &physics.GetGravity().x);
  }

  ImGui::End();

  if (selectionObject) {
    ImGui::Begin("Selected Object");
    Vector3 pos = selectionObject->GetTransform().GetPosition();
    if (ImGui::InputFloat3("P", &pos.x)) {
      selectionObject->GetTransform().SetPosition(pos);
      selectionObject->GetPhysicsObject()->SetLinearVelocity(Vector3());
      selectionObject->GetPhysicsObject()->SetAngularVelocity(Vector3());
    }
    Vector3 vel = selectionObject->GetPhysicsObject()->GetLinearVelocity();
    if (ImGui::InputFloat3("V", &vel.x)) {
      selectionObject->GetPhysicsObject()->SetLinearVelocity(vel);
    }
    Vector3 angVel = selectionObject->GetPhysicsObject()->GetAngularVelocity();
    if (ImGui::InputFloat3("W", &angVel.x)) {
      selectionObject->GetPhysicsObject()->SetAngularVelocity(angVel);
    }
    ImGui::Text("Inv Mass: %.3f",
                selectionObject->GetPhysicsObject()->GetInverseMass());

    constexpr const char *const colliderNames[] = {"None", "Sphere ", "AABB",
                                                   "OBB", "Capsule"};
    int idx = 0;
    switch (selectionObject->GetBoundingVolume()->type) {
    case VolumeType::Sphere:
      idx = 1;
      break;
    case VolumeType::AABB:
      idx = 2;
      break;
    case VolumeType::OBB:
      idx = 3;
      break;
    case VolumeType::Capsule:
      idx = 4;
      break;
    }

    ImGui::Text("Collider: %s", colliderNames[idx]);

    ImGui::End();
  }
}

void TutorialGame::InitCamera() {
  world.GetMainCamera().SetNearPlane(0.1f);
  world.GetMainCamera().SetFarPlane(500.0f);
  world.GetMainCamera().SetPitch(-15.0f);
  world.GetMainCamera().SetYaw(315.0f);
  world.GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

void TutorialGame::Clear() {
  world.ClearAndErase();
  physics.Clear();

  selectionObject = nullptr;
  objClosest = nullptr;
  pane = nullptr;
}

void TutorialGame::InitWorld() {
  Clear();

  CreatedMixedGrid(15, 15, 5.f, 5.f);

  AddFloorToWorld(Vector3(0, -20, 0));

  AddPlayerToWorld(Vector3(0, -15, 0));

  BridgeConstraintTest();

  AddStateObjectToWorld(Vector3(50, 50, 50));
  pane = AddPaneToWorld(Vector3(0, 10, -20), Vector2(4, 2), .5f);
  pane->SetupConstraints(world);

  AddCubeToWorld(Vector3(0, 20, -20), Vector3(2, 2, 2), 0.0f,
                 new Oscillator({{0, 20, -20}, {}}, {{0, 40, -20}, {}}, 10.f));
}

void TutorialGame::InitCollisionTest() {
  Clear();

  AddFloorToWorld(Vector3(0, -20, 0));
  AddSphereToWorld(Vector3(0, 0, 0), 1.0f, 10.0f);
  AddSphereToWorld(Vector3(0, 5, 0), 1.0f, 10.0f);
  AddSphereToWorld(Vector3(0, 10, 0), 1.0f, 10.0f);
  AddSphereToWorld(Vector3(5, 0, 0), 1.0f, 10.0f);
  AddSphereToWorld(Vector3(10, 0, 0), 1.0f, 10.0f);
  AddSphereToWorld(Vector3(15, 0, 0), 1.0f, 10.0f);

  AddOBBToWorld(Vector3(0, 0, 5), Vector3(1, 1, 1), 10.0f);
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

  world.AddGameObject(sphere);

  return sphere;
}

GameObject *TutorialGame::AddCubeToWorld(const Vector3 &position,
                                         Vector3 dimensions, float inverseMass,
                                         GameObject *cube) {

  AABBVolume *volume = new AABBVolume(dimensions);
  cube->SetBoundingVolume(volume);

  cube->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);

  cube->SetRenderObject(
      new RenderObject(cube->GetTransform(), cubeMesh, checkerMaterial));
  cube->SetPhysicsObject(
      new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));

  cube->GetPhysicsObject()->SetInverseMass(inverseMass);
  cube->GetPhysicsObject()->InitCubeInertia();

  world.AddGameObject(cube);

  return cube;
}

GameObject *TutorialGame::AddOBBToWorld(const Vector3 &position,
                                        Vector3 dimensions, float inverseMass) {
  GameObject *obb = new GameObject();
  OBBVolume *volume = new OBBVolume(dimensions);
  obb->SetBoundingVolume(volume);
  obb->GetTransform().SetPosition(position).SetScale(dimensions * 2.0f);
  obb->SetRenderObject(
      new RenderObject(obb->GetTransform(), cubeMesh, checkerMaterial));
  obb->SetPhysicsObject(
      new PhysicsObject(obb->GetTransform(), obb->GetBoundingVolume()));
  obb->GetPhysicsObject()->SetInverseMass(inverseMass);
  obb->GetPhysicsObject()->InitCubeInertia();
  world.AddGameObject(obb);
  return obb;
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

GameObject *TutorialGame::AddPlayerToWorld(const Vector3 &position) {
  float meshSize = 1.0f;
  float inverseMass = 0.5f;

  auto *character = new GameObject();
  SphereVolume *volume = new SphereVolume(1.0f);

  character->SetBoundingVolume(volume);

  character->GetTransform()
      .SetScale(Vector3(meshSize, meshSize, meshSize))
      .SetPosition(position);

  character->SetRenderObject(
      new RenderObject(character->GetTransform(), catMesh, notexMaterial));
  character->SetPhysicsObject(new PhysicsObject(
      character->GetTransform(), character->GetBoundingVolume()));

  character->GetPhysicsObject()->SetInverseMass(inverseMass);
  character->GetPhysicsObject()->InitSphereInertia();

  world.AddGameObject(character);

  return character;
}

GameObject *TutorialGame::AddEnemyToWorld(const Vector3 &position) {
  float meshSize = 3.0f;
  float inverseMass = 0.5f;

  GameObject *character = new GameObject();

  AABBVolume *volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
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

  world.AddGameObject(character);

  return character;
}

GameObject *TutorialGame::AddBonusToWorld(const Vector3 &position) {
  GameObject *apple = new GameObject();

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

Pane *TutorialGame::AddPaneToWorld(const Vector3 &position,
                                   const Vector2 &dimensions, float invMass) {
  Pane *pane = new Pane(&world);
  auto *volume = new OBBVolume(Vector3(dimensions.x, 0.1f, dimensions.y));
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
  world.AddGameObject(pane);
  return pane;
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

/*
Every frame, this code will let you perform a raycast, to see if there's an
object underneath the cursor, and if so 'select it' into a pointer, so that it
can be manipulated later. Pressing Q will let you toggle between this
behaviour and instead letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
  if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
    inSelectionMode = !inSelectionMode;
    if (inSelectionMode) {
      Window::GetWindow()->ShowOSPointer(true);
      Window::GetWindow()->LockMouseToWindow(false);
    } else {
      Window::GetWindow()->ShowOSPointer(false);
      Window::GetWindow()->LockMouseToWindow(true);
    }
  }

  Debug::Print(
      "Camera Pos:" + std::to_string(world.GetMainCamera().GetPosition().x) +
          "," + std::to_string(world.GetMainCamera().GetPosition().y) + "," +
          std::to_string(world.GetMainCamera().GetPosition().z),
      Vector2(5, 5), Debug::WHITE);

  if (inSelectionMode) {
    Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

    if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
      if (selectionObject) { // set colour to deselected;
        selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
        selectionObject = nullptr;
      }

      Ray ray = CollisionDetection::BuildRayFromMouse(world.GetMainCamera());

      RayCollision closestCollision;
      if (world.Raycast(ray, closestCollision, true)) {
        selectionObject = (GameObject *)closestCollision.node;

        selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
        return true;
      } else {
        return false;
      }
    }
  } else {
    Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
  }
  return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button,
by an amount determined by the scroll wheel. In the first tutorial this won't
do anything, as we haven't added linear motion into our physics system. After
the second tutorial, objects will move in a straight line - after the third,
they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
  Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
  forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

  if (!selectionObject) {
    return; // we haven't selected anything!
  }

  // Push the selected object!
  if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
    Ray ray = CollisionDetection::BuildRayFromMouse(world.GetMainCamera());

    RayCollision closestCollision;
    if (world.Raycast(ray, closestCollision, true)) {
      if (closestCollision.node == selectionObject) {
        selectionObject->GetPhysicsObject()->AddForceAtPosition(
            ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
      }
    }
  }

  auto pitch = world.GetMainCamera().GetPitch();
  auto roll = world.GetMainCamera().GetYaw();

  Quaternion q;

  if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
    selectionObject->GetPhysicsObject()->AddForce(Vector3(-10, 0, 0));
  }
  if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
    selectionObject->GetPhysicsObject()->AddForce(Vector3(10, 0, 0));
  }
  if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
    selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 10, 0));
  }
  if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
    selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
  }
}

void TutorialGame::DebugObjectMovement() {
  // If we've selected an object, we can manipulate it with some key presses
  if (inSelectionMode && selectionObject) {
    // Twist the selected object!
    if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
      selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
      selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
      selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
      selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
      selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
      selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
      selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
      selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
    }
  }
}
