#pragma once

enum class BehaviourState { Initialise, Failure, Success, Ongoing };

class BehaviourNode {
public:
  BehaviourNode(const std::string &nodeName) {
    currentState = BehaviourState::Initialise;
    name = nodeName;
  }
  virtual ~BehaviourNode() {}
  virtual BehaviourState Execute(float dt) = 0;

  // BehaviourState	GetState() const {return currentState; }
  virtual void Reset() { currentState = BehaviourState::Initialise; }

protected:
  BehaviourState currentState;
  std::string name;
};