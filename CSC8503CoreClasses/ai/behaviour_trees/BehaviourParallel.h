#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourParallel : public BehaviourNodeWithChildren {
public:
  BehaviourParallel(const std::string &nodeName)
      : BehaviourNodeWithChildren(nodeName) {}
  ~BehaviourParallel() {}

  BehaviourState Execute(float dt) override {
    for (auto &i : childNodes) {
      BehaviourState nodeState = i->Execute(dt);
      switch (nodeState) {
      case BehaviourState::Failure:
        if (currentState != BehaviourState::Ongoing)
          currentState = BehaviourState::Failure;
        continue;
      case BehaviourState::Success:
      case BehaviourState::Ongoing: {
        currentState = BehaviourState::Ongoing;
        return currentState;
      }
      }
    }
    return BehaviourState::Failure;
  }
};