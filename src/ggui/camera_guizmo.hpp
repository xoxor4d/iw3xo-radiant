#pragma once
#include "_ggui.hpp"
#include <imgui.h>

namespace ggui::camera_guizmo
{
	extern void get_selection_center_movepoints(float* center_point);
	extern void guizmo(const ImVec2& camera_size, bool& accepted_dragdrop);
}
