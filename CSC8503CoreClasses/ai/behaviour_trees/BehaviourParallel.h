#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourParallel : public BehaviourNodeWithChildren {
public:
  BehaviourParallel(const std::string &nodeName)
      : BehaviourNodeWithChildren(nodeName) {}
  ~BehaviourParallel() {}

  BehaviourState Execute(float dt) override {
    bool success = false;
    for (auto &i : childNodes) {
      BehaviourState nodeState = i->Execute(dt);
      switch (nodeState) {
      case BehaviourState::Failure:
        break;
      case BehaviourState::Success:
        success = true;
        break;
      case BehaviourState::Ongoing: {
        currentState = BehaviourState::Ongoing;
        break;
      }
      }
    }
    if (currentState == BehaviourState::Ongoing) {
      return currentState;
    }

    currentState = success ? BehaviourState::Success : BehaviourState::Failure;
    return currentState;
  }
};