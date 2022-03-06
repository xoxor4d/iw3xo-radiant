#pragma once
#include "imgui_internal.h"

namespace ImGui
{
	enum class CurveEditorFlags
	{
		NO_TANGENTS = 1 << 0,
		SHOW_GRID = 1 << 1,
		RESET = 1 << 2
	};

	struct traildef_shape_s
	{
		int index;
		int offset_vertex;
		int num_vertex;
		int offset_indices;
		int num_indices;
		bool pending_deletion;
	};

	int		CurveEditor(const char* label, float* values, int points_count, const ImVec2& grid_mins, const ImVec2& grid_maxs, const ImVec2& editor_size, ImU32 flags, int* new_count);
	int		CurveEditorShapes(const char* label, float* values, traildef_shape_s* shapes, int shape_count, const ImVec2& grid_mins, const ImVec2& grid_maxs, const ImVec2& editor_size, ImU32 flags, int* hovered_point);

}