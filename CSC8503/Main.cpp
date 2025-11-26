#include "Window.h"

#include "Debug.h"

#include "ai/state_machine/State.h"
#include "ai/state_machine/StateMachine.h"
#include "ai/state_machine/StateTransition.h"

#include "networking/GameClient.h"
#include "networking/GameServer.h"

#include "ai/pathfinding/NavigationGrid.h"
#include "ai/pathfinding/NavigationMesh.h"

#include "NetworkedGame.h"
#include "TutorialGame.h"

#include "ai/automata/PushdownMachine.h"

#include "ai/automata/PushdownState.h"

#include "ai/behaviour_trees/BehaviourAction.h"
#include "ai/behaviour_trees/BehaviourNode.h"
#include "ai/behaviour_trees/BehaviourSelector.h"
#include "ai/behaviour_trees/BehaviourSequence.h"

#include "physics/PhysicsSystem.h"

#ifdef USEOPENGL
#include "GameTechRenderer.h"
#define CAN_COMPILE
#endif
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#define CAN_COMPILE
#endif

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <sstream>
#include <thread>

void TestStateMachine() {
  StateMachine *testMachine = new StateMachine();
  int data = 0;

  auto A = std::make_unique<State>([&](float dt) {
    std::cout << "State A: " << data << std::endl;
    ++data;
  });

  auto B = std::make_unique<State>([&](float dt) {
    std::cout << "State B: " << data << std::endl;
    --data;
  });

  auto stateAToB = std::make_unique<StateTransition>(
      A.get(), B.get(), [&]() { return data > 10; });
  auto stateBToA = std::make_unique<StateTransition>(
      B.get(), A.get(), [&]() { return data < 0; });

  testMachine->AddState(std::move(A));
  testMachine->AddState(std::move(B));
  testMachine->AddTransition(std::move(stateAToB));
  testMachine->AddTransition(std::move(stateBToA));
  for (int i = 0; i < 100; ++i) {
    testMachine->Update(1.f);
  }
}

std::vector<Vector3> testNodes;
void TestPathfinding() {
  NavigationGrid grid("TestGrid1.txt");

  NavigationPath outPath;

  Vector3 startPos(80, 0, 10);
  Vector3 endPos(80, 0, 80);

  bool found = grid.FindPath(startPos, endPos, outPath);

  Vector3 pos;
  while (outPath.PopWaypoint(pos)) {
    testNodes.push_back(pos);
  }
}

void DisplayPathfinding() {
  for (int i = 1; i < testNodes.size(); ++i) {
    Vector3 a = testNodes[i - 1];
    Vector3 b = testNodes[i];

    Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
  }
}

void TestBehaviourTree() {
  float timer = 0.0f;
  float distatnceToTarget;

  BehaviourAction *findKey = new BehaviourAction(
      "Find Key", [&](float dt, BehaviourState state) -> BehaviourState {
        if (state == BehaviourState::Initialise) {
          std::cout << "Finding Key..." << std::endl;
          timer = rand() % 100;
          state = BehaviourState::Ongoing;
        } else if (state == BehaviourState::Ongoing) {
          timer -= dt;
          if (timer <= 0.0f) {
            std::cout << "Found Key!\n";
            return BehaviourState::Success;
          }
          return state;
        }
      });

  BehaviourAction *goToRoom = new BehaviourAction(
      "Go To Room", [&](float dt, BehaviourState state) -> BehaviourState {
        if (state == BehaviourState::Initialise) {
          std::cout << "Going to Room..." << std::endl;
          state = BehaviourState::Ongoing;
        } else if (state == BehaviourState::Ongoing) {
          distatnceToTarget -= dt;
          if (distatnceToTarget <= 0.0f) {
            std::cout << "Reached Room!\n";
            return BehaviourState::Success;
          }
          return state;
        }
      });

  BehaviourAction *openDoor = new BehaviourAction(
      "Open Door", [&](float dt, BehaviourState state) -> BehaviourState {
        if (state == BehaviourState::Initialise) {
          std::cout << "Opening Door..." << std::endl;
          return BehaviourState::Success;
        }
        return state;
      });

  BehaviourAction *lookForTreasure = new BehaviourAction(
      "Look For Treasure",
      [&](float dt, BehaviourState state) -> BehaviourState {
        if (state == BehaviourState::Initialise) {
          std::cout << "Looking for Treasure..." << std::endl;
          return BehaviourState::Ongoing;
        } else if (state == BehaviourState::Ongoing) {
          bool found = rand() % 2;
          if (found) {
            std::cout << "Found Treasure!\n";
            return BehaviourState::Success;
          } else {
            std::cout << "Couldn't find Treasure.\n";
            return BehaviourState::Failure;
          }
        }
      });

  BehaviourAction *lookForItems = new BehaviourAction(
      "Look For Items", [&](float dt, BehaviourState state) -> BehaviourState {
        if (state == BehaviourState::Initialise) {
          std::cout << "Looking for Items..." << std::endl;
          return BehaviourState::Ongoing;
        } else if (state == BehaviourState::Ongoing) {
          bool found = rand() % 2;
          if (found) {
            std::cout << "Found Items!\n";
            return BehaviourState::Success;
          } else {
            std::cout << "Couldn't find Items.\n";
            return BehaviourState::Failure;
          }
        }
      });

  BehaviourSequence *sequence = new BehaviourSequence("Room Sequence")
}

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead.

This time, we've added some extra functionality to the window class - we can
hide or show the

*/
int main() {
  WindowInitialisation initInfo;
  initInfo.width = 1280;
  initInfo.height = 720;
  initInfo.windowTitle = "FLOAT CSC8503 Game technology!";

  Window *w = Window::CreateGameWindow(initInfo);

  if (!w->HasInitialised()) {
    return -1;
  }

  w->ShowOSPointer(false);
  w->LockMouseToWindow(true);

  GameWorld *world = new GameWorld();
  PhysicsSystem *physics = new PhysicsSystem(*world);

#ifdef USEVULKAN
  GameTechVulkanRenderer *renderer = new GameTechVulkanRenderer(*world);
#elif USEOPENGL
  GameTechRenderer *renderer = new GameTechRenderer(*world);
#endif

  TutorialGame *g = new TutorialGame(*world, *renderer, *physics);

  TestPathfinding();

  w->GetTimer().GetTimeDeltaSeconds(); // Clear the timer so we don't get a
                                       // larget first dt!
  while (w->UpdateWindow() &&
         !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
    float dt = w->GetTimer().GetTimeDeltaSeconds();
    if (dt > 0.1f) {
      std::cout << "Skipping large time delta" << std::endl;
      continue; // must have hit a breakpoint or something to have a 1 second
                // frame time!
    }
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
      w->ShowConsole(true);
    }
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
      w->ShowConsole(false);
    }

    if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
      w->SetWindowPosition(0, 0);
    }

    w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

    renderer->StartFrame();
    DisplayPathfinding();

    g->UpdateGame(dt);

    world->UpdateWorld(dt);
    physics->Update(dt);
    renderer->Update(dt);
    renderer->Render();

    Debug::UpdateRenderables(dt);
  }
  Window::DestroyGameWindow();
}