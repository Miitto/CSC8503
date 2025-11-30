#include "Window.h"

#include "Debug.h"

#include "ai/state_machine/State.h"
#include "ai/state_machine/StateMachine.h"
#include "ai/state_machine/StateTransition.h"

#include "networking/GameClient.h"
#include "networking/GameServer.h"

#include "ai/pathfinding/NavigationGrid.h"

#include "ClientGame.h"

#include "ai/automata/PushdownMachine.h"

#include "ai/automata/PushdownState.h"

#include "ai/behaviour_trees/BehaviourAction.h"
#include "ai/behaviour_trees/BehaviourNode.h"
#include "ai/behaviour_trees/BehaviourSelector.h"
#include "ai/behaviour_trees/BehaviourSequence.h"

#include "physics/PhysicsSystem.h"

#include "menus/menu.h"

#include "logging/log.h"

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
#include <networking/NetworkObject.h>
#include <thread>

#pragma region Test Functions

#define RUN_TESTS 0

namespace {
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
          timer = static_cast<float>(rand() % 100);
          return BehaviourState::Ongoing;
        } else if (state == BehaviourState::Ongoing) {
          timer -= dt;
          if (timer <= 0.0f) {
            std::cout << "Found Key!\n";
            return BehaviourState::Success;
          }
        }
        return state;
      });

  BehaviourAction *goToRoom = new BehaviourAction(
      "Go To Room", [&](float dt, BehaviourState state) -> BehaviourState {
        if (state == BehaviourState::Initialise) {
          std::cout << "Going to Room..." << std::endl;
          return BehaviourState::Ongoing;
        } else if (state == BehaviourState::Ongoing) {
          distatnceToTarget -= dt;
          if (distatnceToTarget <= 0.0f) {
            std::cout << "Reached Room!\n";
            return BehaviourState::Success;
          }
        }
        return state;
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
        return state;
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
        return state;
      });

  BehaviourSequence *sequence = new BehaviourSequence("Room Sequence");
  sequence->AddChild(findKey).AddChild(goToRoom).AddChild(openDoor);

  BehaviourSelector *selector = new BehaviourSelector("Loot Selection");
  selector->AddChild(lookForTreasure).AddChild(lookForItems);

  BehaviourSequence *root = new BehaviourSequence("Root Sequence");
  root->AddChild(sequence).AddChild(selector);

  for (int i = 0; i < 5; ++i) {
    std::cout << "---- Behaviour Tree Run " << i << " ----\n";
    timer = 0.0f;
    distatnceToTarget = static_cast<float>(rand() % 250);
    root->Reset();
    BehaviourState result = BehaviourState::Ongoing;

    while (result == BehaviourState::Ongoing) {
      result = root->Execute(1.0f);
    }

    if (result == BehaviourState::Success) {
      std::cout << "Behaviour Tree Succeeded!\n";
    } else {
      std::cout << "Behaviour Tree Failed.\n";
    }

    std::cout << "---- End Behaviour Tree Run " << i << " ----\n\n";
  }
}

#pragma region Networking Tests
class TestPacketReceiver : public PacketReceiver {
public:
  TestPacketReceiver(std::string name) : name(name) {}

  void ReceivePacket(GamePacketType type, GamePacket *payload,
                     int source = -1) override {
    if (type == BasicNetworkMessages::String_Message) {
      auto str = StringPacket::as<StringPacket>(payload);

      LOG("{} recieved: {}", name, str->view());
    }
  }

protected:
  std::string name;
};

void TestNetworking() {
  NetworkBase::Initialise();
  TestPacketReceiver serverReceiver("Server");
  TestPacketReceiver clientReceiver("Client");

  constexpr uint16_t port = NetworkBase::GetDefaultPort();

  GameServer *server = new GameServer(port, 1);
  GameClient *client = new GameClient();

  server->RegisterPacketHandler(BasicNetworkMessages::String_Message,
                                &serverReceiver);
  client->RegisterPacketHandler(BasicNetworkMessages::String_Message,
                                &clientReceiver);

  if (!client->Connect({127, 0, 0, 1, port})) {
    std::cerr << "Client failed to connect to server!" << std::endl;
    return;
  }

  for (int i = 0; i < 100; ++i) {
    server->SendGlobalPacket(
        StringPacket("Server says hello: " + std::to_string(i)));
    client->SendPacket(StringPacket("Client says hello: " + std::to_string(i)));

    server->UpdateServer();
    client->UpdateClient();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  NetworkBase::Destroy();
}
#pragma endregion
} // namespace
#pragma endregion

PushdownMachine makeMenuPushdownAutomata(ClientGame &game) {
  PushdownState *mainMenuState = new MainMenu(game);
  PushdownMachine menuMachine(mainMenuState);

  return menuMachine;
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

  ClientGame *g = new ClientGame(*world, *renderer, *physics);

  auto menuAutomata = makeMenuPushdownAutomata(*g);

#if RUN_TESTS
  TestPathfinding();
  TestBehaviourTree();
  TestNetworking();
#endif

  w->GetTimer().GetTimeDeltaSeconds(); // Clear the timer so we don't get a
                                       // larget first dt!
  while (w->UpdateWindow()) {
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
#if RUN_TESTS
    DisplayPathfinding();
#endif

    menuAutomata.Update(dt);
    g->UpdateGame(dt);

    world->UpdateWorld(dt);
    physics->Update(dt);
    renderer->Update(dt);
    renderer->Render();

    Debug::UpdateRenderables(dt);
  }
  Window::DestroyGameWindow();
}