#pragma once

namespace NCL {
namespace CSC8503 {
class Constraint {
public:
  Constraint() {}
  virtual ~Constraint() = default;

  virtual void UpdateConstraint(float dt) = 0;

  void SetActive(bool state) { active = state; }
  void activate() { active = true; }
  void deactivate() { active = false; }

protected:
  bool active = true;
};
} // namespace CSC8503
} // namespace NCL