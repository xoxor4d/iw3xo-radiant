#pragma once
#include "_ggui.hpp"
#include <imgui.h>

namespace ggui::camera_guizmo
{
	void guizmo(const ImVec2& camera_size, bool& accepted_dragdrop);

}
