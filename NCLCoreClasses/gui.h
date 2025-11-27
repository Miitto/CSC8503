#pragma once

#include <imgui/imgui.h>

namespace NCL::gui {
class Frame {
public:
  Frame(const char *const name, bool *p_open = nullptr,
        ImGuiWindowFlags flags = 0) {
    ImGui::Begin(name, p_open, flags);
  }

  template <typename... Args>
  void text(const char *const fmt, Args... args) const {
    ImGui::Text(fmt, args...);
  }

  bool button(const char *const label,
              const ImVec2 &size = ImVec2(0, 0)) const {
    return ImGui::Button(label, size);
  }

  ~Frame() { ImGui::End(); }
};
} // namespace NCL::gui