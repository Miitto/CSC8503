#include "RendererBase.h"

#include <imgui/backends/imgui_impl_opengl3.h>
#ifdef _WIN32
#include "Win32Window.h"
#include <imgui/backends/imgui_impl_win32.h>
#endif

using namespace NCL;
using namespace Rendering;

RendererBase::RendererBase(Window &window) : hostWindow(window) {}

RendererBase::~RendererBase() {}

void RendererBase::StartFrame() {
#ifdef _WIN32
  ImGui_ImplWin32_NewFrame();
#endif
  ImGui::NewFrame();
}
