#pragma once
#include "BehaviourNode.h"

class BehaviourNodeWithChildren : public BehaviourNode {
public:
  BehaviourNodeWithChildren(const std::string &nodeName)
      : BehaviourNode(nodeName) {};
  ~BehaviourNodeWithChildren() {
    for (auto &i : childNodes) {
      delete i;
    }
  }
  BehaviourNodeWithChildren &AddChild(BehaviourNode *n) {
    childNodes.emplace_back(n);
    return *this;
  }

  void Reset() override {
    currentState = BehaviourState::Initialise;
    for (auto &i : childNodes) {
      i->Reset();
    }
  }

protected:
  std::vector<BehaviourNode *> childNodes;
};