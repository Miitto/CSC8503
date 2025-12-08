#pragma once
#include "BehaviourNode.h"

class BehaviourNodeWithChildren : public BehaviourNode {
public:
  BehaviourNodeWithChildren(const std::string &nodeName)
      : BehaviourNode(nodeName) {};

  BehaviourNodeWithChildren &AddChild(std::shared_ptr<BehaviourNode> n) {
    childNodes.emplace_back(std::move(n));
    return *this;
  }

  void Reset() override {
    currentState = BehaviourState::Initialise;
    for (auto &i : childNodes) {
      i->Reset();
    }
  }

protected:
  std::vector<std::shared_ptr<BehaviourNode>> childNodes;
};