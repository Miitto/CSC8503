#include "ServerGame.h"
#include "physics/PhysicsSystem.h"
#include <DummyRenderer.h>

using namespace NCL;
using namespace CSC8503;

int main() {
  GameWorld *world = new GameWorld();
  PhysicsSystem *physics = new PhysicsSystem(*world);

  DummyRenderer renderer = DummyRenderer();

  ServerGame *g = new ServerGame(*world, renderer, *physics);

  auto lastFrameTime = std::chrono::high_resolution_clock::now();

  auto getDeltaSeconds = [&]() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = now - lastFrameTime;
    lastFrameTime = now;
    return delta.count();
  };

  while (true) {
    float dt = getDeltaSeconds();
    if (dt > 0.1f) {
      std::cout << "Skipping large time delta" << std::endl;
      continue; // must have hit a breakpoint or something to have a 1 second
                // frame time!
    }

    g->UpdateGame(dt);

    world->UpdateWorld(dt);
    physics->Update(dt);
  }
}