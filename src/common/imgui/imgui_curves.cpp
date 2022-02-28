#include "std_include.hpp"
#include "imgui_internal.h"

// https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930

namespace ImGui
{
	int CurveEditor(const char* label, float* values, int points_count, const ImVec2& grid_mins, const ImVec2& grid_maxs, const ImVec2& editor_size, ImU32 flags, int* new_count)
	{
		enum class StorageValues : ImGuiID
		{
			FROM_X = 100,
			FROM_Y,
			WIDTH,
			HEIGHT,
			IS_PANNING,
			POINT_START_X,
			POINT_START_Y
		};

		if (new_count)
		{
			*new_count = points_count;
		}

		int color_vars = 0;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.21f, 0.21f, 0.21f, 1.0f)); color_vars++;
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.9f, 0.9f, 0.9f, 1.0f)); color_vars++;

		int style_vars = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f)); style_vars++;

		static float color_grid[4] = { 0.16f, 0.16f, 0.16f, 1.0f };
		//ImGui::ColorEdit4("grid", color_grid);

		static float color_grid_text[4] = { 0.125f, 0.125f, 0.125f, 1.0f };
		//ImGui::ColorEdit4("grid text", color_grid_text);

		static float color_point_hover_text[4] = { 0.976f, 0.435f, 0.0f, 1.0f };
		//ImGui::ColorEdit4("point hover text", color_point_hover_text);

		auto grid_color = ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(color_grid));
		auto grid_text_color = ImGui::ColorConvertFloat4ToU32(ToImVec4(color_grid_text));
		auto point_hover_text_color = ImGui::ColorConvertFloat4ToU32(ToImVec4(color_point_hover_text));

		int hovered_idx = -1;
		ImVec2 size = editor_size;

		ImGuiWindow* parent_window = GetCurrentWindow();
		ImGuiID id = parent_window->GetID(label);

		if (!BeginChildFrame(id, size, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImGui::PopStyleVar(style_vars);
			ImGui::PopStyleColor(color_vars);

			EndChild();
			return -1;
		}

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
		{
			ImGui::PopStyleVar(style_vars);
			ImGui::PopStyleColor(color_vars);

			EndChild();
			return -1;
		}

		ImVec2 points_min(grid_mins.x, grid_mins.y);
		ImVec2 points_max(grid_maxs.x, grid_maxs.y);

		float from_x = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_X, points_min.x);
		float from_y = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_Y, points_min.y);
		float width  = window->StateStorage.GetFloat((ImGuiID)StorageValues::WIDTH, points_max.x - points_min.x);
		float height = window->StateStorage.GetFloat((ImGuiID)StorageValues::HEIGHT, points_max.y - points_min.y);


		if (width > grid_maxs.x - grid_mins.x) {
			width = grid_maxs.x - grid_mins.x;
		}

		if (height > grid_maxs.y - grid_mins.y) {
			height = grid_maxs.y - grid_mins.y;
		}

		if (from_x < grid_mins.x) {
			from_x = grid_mins.x;
		}
		else if (from_x + width > grid_maxs.x) {
			from_x = grid_maxs.x - width;
		}

		if (from_y < grid_mins.y) {
			from_y = grid_mins.y;
		}
		else if (from_y + height > grid_maxs.y) {
			from_y = grid_maxs.y - height;
		}


		window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_X, from_x);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_Y, from_y);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::WIDTH, width);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::HEIGHT, height);

		const ImRect inner_bb = window->InnerRect;

		auto transform = [&](const ImVec2& pos) -> ImVec2
		{
			const float x = (pos.x - from_x) / width;
			const float y = (pos.y - from_y) / height;

			return ImVec2(inner_bb.Min.x * (1 - x) + inner_bb.Max.x * x, inner_bb.Min.y * y + inner_bb.Max.y * (1 - y));
		};

		auto invTransform = [&](const ImVec2& pos) -> ImVec2
		{
			const float x = (pos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x);
			const float y = (inner_bb.Max.y - pos.y) / (inner_bb.Max.y - inner_bb.Min.y);

			return ImVec2(from_x + width * x, from_y + height * y);
		};

		if (flags & (int)CurveEditorFlags::SHOW_GRID)
		{
			int exp;
			frexp(width / 5.0f, &exp);

			const float step_x = ldexp(1.0f, exp);
			const float x = step_x * static_cast<int>((from_x / step_x));

			const int cell_cols	= static_cast<int>(width / step_x);

			for (int i = -1; i < cell_cols + 2; ++i)
			{
				ImVec2 a = transform({ x + i * step_x, from_y });
				ImVec2 b = transform({ x + i * step_x, from_y + height });

				if(i && x + static_cast<float>(i) * step_x == 0.0f)
				{
					window->DrawList->AddLine(a, b, grid_color, 4.0f);
				}
				else
				{
					window->DrawList->AddLine(a, b, grid_color);
				}

				char buf[64];

				if (exp > 0)
				{
					ImFormatString(buf, sizeof(buf), " %d", static_cast<int>((x + static_cast<float>(i) * step_x)));
				}
				else
				{
					ImFormatString(buf, sizeof(buf), " %.2f", x + static_cast<float>(i) * step_x);
				}

				window->DrawList->AddText(b, grid_text_color, buf);
			}

			frexp(height / 5.0f, &exp);

			const float step_y = ldexp(1.0f, exp);
			const float y = step_y * static_cast<float>(static_cast<int>(from_y / step_y));

			const int cell_rows = static_cast<int>(height / step_y);

			for (int i = -1; i < cell_rows + 2; ++i)
			{
				ImVec2 a = transform({ from_x, y + i * step_y });
				ImVec2 b = transform({ from_x + width, y + i * step_y });

				if (i && y + static_cast<float>(i) * step_y == 0.0f)
				{
					window->DrawList->AddLine(a, b, grid_color, 4.0f);
				}
				else
				{
					window->DrawList->AddLine(a, b, grid_color);
				}

				char buf[64];

				if (exp > 0)
				{
					ImFormatString(buf, sizeof(buf), " %d", static_cast<int>(y + static_cast<float>(i) * step_y));
				}
				else
				{
					ImFormatString(buf, sizeof(buf), " %.2f", y + static_cast<float>(i) * step_y);
				}

				window->DrawList->AddText(a, grid_text_color, buf);
			}
		}

		int changed_idx = -1;
		bool first_point = true;

		for (int point_idx = points_count - 2; point_idx >= 0; --point_idx)
		{
			ImVec2* points = ((ImVec2*)values) + point_idx;
			ImVec2  p_prev = points[0];
			ImVec2  p = points[1];
			
			auto handlePoint = [&](ImVec2& p, int idx, bool _first_point = false) -> bool
			{
				static const float SIZE = 4;

				const ImVec2 cursor_pos = GetCursorScreenPos();
				ImVec2 pos = transform(p);

				ImVec2 pos_for_drawing = pos;

				if(pos.x - (SIZE * 0.75f) < inner_bb.Min.x)
				{
					pos_for_drawing.x += (SIZE * 0.75f);
				}
				if (pos.y - (SIZE * 0.75f) < inner_bb.Min.y)
				{
					pos_for_drawing.y += (SIZE * 0.75f);
				}

				if (pos.x + (SIZE * 0.75f) > inner_bb.Max.x)
				{
					pos_for_drawing.x -= (SIZE * 0.75f);
				}
				if (pos.y + (SIZE * 0.75f) > inner_bb.Max.y)
				{
					pos_for_drawing.y -= (SIZE * 0.75f);
				}

				const float NODE_SLOT_RADIUS = 4.0f;

				SetCursorScreenPos(pos_for_drawing - ImVec2(SIZE, SIZE));
				PushID(idx);
				InvisibleButton("", ImVec2(2 * NODE_SLOT_RADIUS, 2 * NODE_SLOT_RADIUS));

				const ImU32 col = IsItemActive() || IsItemHovered() ? GetColorU32(ImGuiCol_PlotLinesHovered)
					: GetColorU32(ImGuiCol_PlotLines);

				window->DrawList->AddLine(pos_for_drawing + ImVec2(-SIZE, 0), pos_for_drawing + ImVec2(0,  SIZE), col);
				window->DrawList->AddLine(pos_for_drawing + ImVec2( SIZE, 0), pos_for_drawing + ImVec2(0,  SIZE), col);
				window->DrawList->AddLine(pos_for_drawing + ImVec2( SIZE, 0), pos_for_drawing + ImVec2(0, -SIZE), col);
				window->DrawList->AddLine(pos_for_drawing + ImVec2(-SIZE, 0), pos_for_drawing + ImVec2(0, -SIZE), col);

				if (IsItemHovered())
				{
					hovered_idx = point_idx + idx;
				}
	
				bool changed = false;
				if (IsItemActive() && IsMouseClicked(0))
				{
					window->StateStorage.SetFloat((ImGuiID)StorageValues::POINT_START_X, pos.x);
					window->StateStorage.SetFloat((ImGuiID)StorageValues::POINT_START_Y, pos.y);
				}

				if (IsItemHovered() || IsItemActive() && IsMouseDragging(0))
				{
					const char* position_str = utils::va("%0.2f, %0.2f", p.x, p.y);
					const ImVec2 position_str_size = ImGui::CalcTextSize(position_str);

					if (pos.x + position_str_size.x > inner_bb.Max.x)
					{
						pos_for_drawing.x = inner_bb.Max.x - position_str_size.x;
					}

					if (pos.y - position_str_size.y < inner_bb.Min.y)
					{
						pos_for_drawing.y = inner_bb.Min.y + position_str_size.y;
					}

					char tmp[64];
					ImFormatString(tmp, sizeof(tmp), position_str);
					window->DrawList->AddText({ pos_for_drawing.x, pos_for_drawing.y - GetTextLineHeight() }, point_hover_text_color, tmp);
				}

				if (IsItemActive() && IsMouseDragging(0))
				{
					pos.x = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_X, pos.x);
					pos.y = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_Y, pos.y);
					pos += ImGui::GetMouseDragDelta();

					ImVec2 v = invTransform(pos);

					if (idx == 0)
					{
						// x - axis :: clamp last (right to left) point to mins 
						p = ImClamp(v, ImVec2(grid_mins.x, grid_mins.y), ImVec2(grid_mins.x, grid_maxs.y));
					}
					else if (_first_point)
					{
						// x - axis :: clamp first (right to left) point to maxs
						p = ImClamp(v, ImVec2(grid_maxs.x, grid_mins.y), ImVec2(grid_maxs.x, grid_maxs.y));
					}
					else
					{
						// clamp every other point between mins and maxs
						p = ImClamp(v, ImVec2(grid_mins.x, grid_mins.y), ImVec2(grid_maxs.x, grid_maxs.y));
					}

					changed = true;
				}
				PopID();

				SetCursorScreenPos(cursor_pos);
				return changed;
			};

			PushID(point_idx);
			{
				window->DrawList->AddLine(transform(p_prev), transform(p), GetColorU32(ImGuiCol_PlotLines), 1.0f);
				if (handlePoint(p, 1, first_point))
				{
					if (p.x <= p_prev.x)
					{
						p.x = p_prev.x + 0.001f;
					}
						
					if (point_idx < points_count - 2 && p.x >= points[2].x)
					{
						p.x = points[2].x - 0.001f;
					}

					points[1] = p;
					changed_idx = point_idx + 1;
				}
			}

			if (point_idx == 0)
			{
				if (handlePoint(p_prev, 0))
				{
					if (p.x <= p_prev.x)
					{
						p_prev.x = p.x - 0.001f;
					}
						
					points[0] = p_prev;
					changed_idx = point_idx;
				}
			}

			PopID();

			first_point = false;
		}

		SetCursorScreenPos(inner_bb.Min);

		InvisibleButton(utils::va("%s##bg", label), inner_bb.Max - inner_bb.Min);

		// add new key on double click
		if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0) && new_count)
		{
			const ImVec2 mp = ImGui::GetMousePos();
			const ImVec2 new_p = invTransform(mp);

			ImVec2* points = (ImVec2*)values;
			points[points_count] = new_p;
			++* new_count;

			auto compare = [](const void* a, const void* b) -> int
			{
				const float fa = ((const ImVec2*)a)->x;
				const float fb = ((const ImVec2*)b)->x;
				return fa < fb ? -1 : (fa > fb) ? 1 : 0;
			};

			qsort(values, points_count + 1, sizeof(ImVec2), compare);
			
		}

		// delete key on double click
		if (hovered_idx > 0 && (hovered_idx + 1 != points_count) && ImGui::IsMouseDoubleClicked(0) && new_count && points_count > 2)
		{

			ImVec2* points = (ImVec2*)values;
			--* new_count;

			for (int j = hovered_idx; j < points_count - 1; ++j)
			{
				points[j] = points[j + 1];
			}
		}

		ImGui::PopStyleVar(style_vars);
		ImGui::PopStyleColor(color_vars);
		EndChild();

		return changed_idx;
	}








	int CurveEditorShapes(const char* label, float* values, traildef_shape_s* shapes, int shape_count, const ImVec2& grid_mins, const ImVec2& grid_maxs, const ImVec2& editor_size, ImU32 flags, int* new_count)
	{
		enum class StorageValues : ImGuiID
		{
			FROM_X = 100,
			FROM_Y,
			WIDTH,
			HEIGHT,
			IS_PANNING,
			POINT_START_X,
			POINT_START_Y
		};

		if (new_count)
		{
			for(auto shape = 0; shape < shape_count; shape++)
			{
				new_count[shape] = shapes[shape].num_vertex;
			}
		}

		int color_vars = 0;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.21f, 0.21f, 0.21f, 1.0f)); color_vars++;
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.9f, 0.9f, 0.9f, 1.0f)); color_vars++;

		int style_vars = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f)); style_vars++;

		static float color_grid[4] = { 0.16f, 0.16f, 0.16f, 1.0f };
		//ImGui::ColorEdit4("grid", color_grid);

		static float color_grid_text[4] = { 0.125f, 0.125f, 0.125f, 1.0f };
		//ImGui::ColorEdit4("grid text", color_grid_text);

		static float color_point_hover_text[4] = { 0.976f, 0.435f, 0.0f, 1.0f };
		//ImGui::ColorEdit4("point hover text", color_point_hover_text);

		auto grid_color = ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(color_grid));
		auto grid_text_color = ImGui::ColorConvertFloat4ToU32(ToImVec4(color_grid_text));
		auto point_hover_text_color = ImGui::ColorConvertFloat4ToU32(ToImVec4(color_point_hover_text));

		int hovered_idx = -1;
		ImVec2 size = editor_size;

		ImGuiWindow* parent_window = GetCurrentWindow();
		ImGuiID id = parent_window->GetID(label);

		if (!BeginChildFrame(id, size, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImGui::PopStyleVar(style_vars);
			ImGui::PopStyleColor(color_vars);

			EndChild();
			return -1;
		}

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
		{
			ImGui::PopStyleVar(style_vars);
			ImGui::PopStyleColor(color_vars);

			EndChild();
			return -1;
		}

		ImVec2 points_min(grid_mins.x, grid_mins.y);
		ImVec2 points_max(grid_maxs.x, grid_maxs.y);

		float from_x = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_X, points_min.x);
		float from_y = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_Y, points_min.y);
		float width = window->StateStorage.GetFloat((ImGuiID)StorageValues::WIDTH, points_max.x - points_min.x);
		float height = window->StateStorage.GetFloat((ImGuiID)StorageValues::HEIGHT, points_max.y - points_min.y);


		if (width > grid_maxs.x - grid_mins.x) {
			width = grid_maxs.x - grid_mins.x;
		}

		if (height > grid_maxs.y - grid_mins.y) {
			height = grid_maxs.y - grid_mins.y;
		}

		if (from_x < grid_mins.x) {
			from_x = grid_mins.x;
		}
		else if (from_x + width > grid_maxs.x) {
			from_x = grid_maxs.x - width;
		}

		if (from_y < grid_mins.y) {
			from_y = grid_mins.y;
		}
		else if (from_y + height > grid_maxs.y) {
			from_y = grid_maxs.y - height;
		}


		window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_X, from_x);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_Y, from_y);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::WIDTH, width);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::HEIGHT, height);

		const ImRect inner_bb = window->InnerRect;

		auto transform = [&](const ImVec2& pos) -> ImVec2
		{
			const float x = (pos.x - from_x) / width;
			const float y = (pos.y - from_y) / height;

			return ImVec2(inner_bb.Min.x * (1 - x) + inner_bb.Max.x * x, inner_bb.Min.y * y + inner_bb.Max.y * (1 - y));
		};

		auto invTransform = [&](const ImVec2& pos) -> ImVec2
		{
			const float x = (pos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x);
			const float y = (inner_bb.Max.y - pos.y) / (inner_bb.Max.y - inner_bb.Min.y);

			return ImVec2(from_x + width * x, from_y + height * y);
		};

		if (flags & (int)CurveEditorFlags::SHOW_GRID)
		{
			int exp;
			frexp(width / 5.0f, &exp);

			const float step_x = ldexp(1.0f, exp);
			const float x = step_x * static_cast<int>((from_x / step_x));

			const int cell_cols = static_cast<int>(width / step_x);

			for (int i = 0; i < cell_cols + 1; ++i)
			{
				ImVec2 a = transform({ x + i * step_x, from_y });
				ImVec2 b = transform({ x + i * step_x, from_y + height });

				if (i && x + static_cast<float>(i) * step_x == 0.0f)
				{
					window->DrawList->AddLine(a, b, grid_color, 4.0f);
				}
				else
				{
					window->DrawList->AddLine(a, b, grid_color);
				}

				char buf[64];

				if (exp > 0)
				{
					ImFormatString(buf, sizeof(buf), " %d", static_cast<int>((x + static_cast<float>(i) * step_x)));
				}
				else
				{
					ImFormatString(buf, sizeof(buf), " %.2f", x + static_cast<float>(i) * step_x);
				}

				if(!i)
				{
					b.x += 8.0f;
				}

				window->DrawList->AddText(b, grid_text_color, buf);
			}

			frexp(height / 5.0f, &exp);

			const float step_y = ldexp(1.0f, exp);
			const float y = step_y * static_cast<float>(static_cast<int>(from_y / step_y));

			const int cell_rows = static_cast<int>(height / step_y);

			for (int i = 0; i < cell_rows + 1; ++i)
			{
				ImVec2 a = transform({ from_x, y + i * step_y });
				ImVec2 b = transform({ from_x + width, y + i * step_y });

				if (i && y + static_cast<float>(i) * step_y == 0.0f)
				{
					window->DrawList->AddLine(a, b, grid_color, 4.0f);
				}
				else
				{
					window->DrawList->AddLine(a, b, grid_color);
				}

				char buf[64];

				if (exp > 0)
				{
					ImFormatString(buf, sizeof(buf), " %d", static_cast<int>(y + static_cast<float>(i) * step_y));
				}
				else
				{
					ImFormatString(buf, sizeof(buf), " %.2f", y + static_cast<float>(i) * step_y);
				}

				if (i + 1 > cell_rows)
				{
					a.y += 12.0f;
				}

				window->DrawList->AddText(a, grid_text_color, buf);
			}
		}

		int changed_idx = -1;

		for(auto shape = 0; shape < shape_count; shape++)
		{
			bool first_point = true;

			int pp = 0;
			for (int point_idx = shapes[shape].num_vertex - 2; point_idx >= 0; --point_idx)
			{
				ImVec2* points = ((ImVec2*)values) + (shapes[shape].offset_vertex + point_idx);
				ImVec2  p_prev = points[0];
				ImVec2  p = points[1];

				auto handlePoint = [&](ImVec2& p, int idx, int idx_offset) -> bool
				{
					static const float SIZE = 4;

					const ImVec2 cursor_pos = GetCursorScreenPos();
					ImVec2 pos = transform(p);

					ImVec2 pos_for_drawing = pos;

					if (pos.x - (SIZE * 0.75f) < inner_bb.Min.x)
					{
						pos_for_drawing.x += (SIZE * 0.75f);
					}
					if (pos.y - (SIZE * 0.75f) < inner_bb.Min.y)
					{
						pos_for_drawing.y += (SIZE * 0.75f);
					}

					if (pos.x + (SIZE * 0.75f) > inner_bb.Max.x)
					{
						pos_for_drawing.x -= (SIZE * 0.75f);
					}
					if (pos.y + (SIZE * 0.75f) > inner_bb.Max.y)
					{
						pos_for_drawing.y -= (SIZE * 0.75f);
					}

					const float NODE_SLOT_RADIUS = 4.0f;

					SetCursorScreenPos(pos_for_drawing - ImVec2(SIZE, SIZE));
					PushID(idx + (shape + 0) + idx_offset);
					InvisibleButton("##inv_node_button", ImVec2(2 * NODE_SLOT_RADIUS, 2 * NODE_SLOT_RADIUS));

					const ImU32 col = IsItemActive() || IsItemHovered() ? GetColorU32(ImGuiCol_PlotLinesHovered)
						: GetColorU32(ImGuiCol_PlotLines);

					window->DrawList->AddLine(pos_for_drawing + ImVec2(-SIZE, 0), pos_for_drawing + ImVec2(0, SIZE), col);
					window->DrawList->AddLine(pos_for_drawing + ImVec2(SIZE, 0), pos_for_drawing + ImVec2(0, SIZE), col);
					window->DrawList->AddLine(pos_for_drawing + ImVec2(SIZE, 0), pos_for_drawing + ImVec2(0, -SIZE), col);
					window->DrawList->AddLine(pos_for_drawing + ImVec2(-SIZE, 0), pos_for_drawing + ImVec2(0, -SIZE), col);

					if (IsItemHovered())
					{
						//hovered_idx = point_idx + (idx + (shape + 0) + idx_offset);
						hovered_idx = idx_offset - pp;
						//game::printf_to_console("hovered idx: %d", hovered_idx);
					}

					pp++;

					bool changed = false;
					if (IsItemActive() && IsMouseClicked(0))
					{
						window->StateStorage.SetFloat((ImGuiID)StorageValues::POINT_START_X, pos.x);
						window->StateStorage.SetFloat((ImGuiID)StorageValues::POINT_START_Y, pos.y);
					}

					if (IsItemHovered() || IsItemActive() && IsMouseDragging(0))
					{
						const char* position_str = utils::va("%0.2f, %0.2f - [#%d S%d]", p.x, p.y, hovered_idx, shape);
						ImGui::PushFontFromIndex(ggui::BOLD_18PX);
						//const char* position_str = utils::va("%0.2f, %0.2f", p.x, p.y);
						const ImVec2 position_str_size = ImGui::CalcTextSize(position_str);

						if (pos.x + position_str_size.x > inner_bb.Max.x)
						{
							pos_for_drawing.x = inner_bb.Max.x - position_str_size.x;
						}

						if (pos.y - position_str_size.y < inner_bb.Min.y)
						{
							pos_for_drawing.y = inner_bb.Min.y + position_str_size.y;
						}

						char tmp[64];
						ImFormatString(tmp, sizeof(tmp), position_str);
						window->DrawList->AddText({ pos_for_drawing.x, pos_for_drawing.y - GetTextLineHeight() }, point_hover_text_color, tmp);
						ImGui::PopFont();
					}

					if (IsItemActive() && IsMouseDragging(0))
					{
						pos.x = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_X, pos.x);
						pos.y = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_Y, pos.y);
						pos += ImGui::GetMouseDragDelta();

						ImVec2 v = invTransform(pos);

						{
							// clamp every other point between mins and maxs
							p = ImClamp(v, ImVec2(grid_mins.x, grid_mins.y), ImVec2(grid_maxs.x, grid_maxs.y));
						}

						changed = true;
					}
					PopID();

					SetCursorScreenPos(cursor_pos);
					return changed;
				};

				PushID(point_idx + (shape + 0) + shapes[shape].num_indices + 1);
				{
					window->DrawList->AddLine(transform(p_prev), transform(p), GetColorU32(ImGuiCol_PlotLines), 1.0f);
					if (handlePoint(p, 1, shapes[shape].offset_vertex + shapes[shape].num_vertex - 1)) //shape > 0 ? shapes[shape].num_vertex : 0))  // num_indices + 1
					{
						points[1] = p;
						changed_idx = hovered_idx;
					}

					if (point_idx == 0)
					{
						if (handlePoint(p_prev, 0, shapes[shape].offset_vertex + shapes[shape].num_vertex - 1)) //shape > 0 ? shapes[shape].num_vertex : 0)) // // num_indices + 1
						{
							points[0] = p_prev;
							changed_idx = hovered_idx;
						}
					}
				}
				PopID();

				first_point = false;
			}
		}

		SetCursorScreenPos(inner_bb.Min);

		InvisibleButton(utils::va("%s##bg", label), inner_bb.Max - inner_bb.Min);

		ImGui::PopStyleVar(style_vars);
		ImGui::PopStyleColor(color_vars);
		EndChild();

		static int hovered_index_on_context = -1;

		if (hovered_idx >= 0 || hovered_index_on_context >= 0)
		{
			if (ImGui::BeginPopupContextItem("geotrail_graph##context"))
			{
				// save hover id on context open
				if(hovered_index_on_context < 0)
				{
					//game::printf_to_console("saved hovered idx: %d", hovered_idx);
					hovered_index_on_context = hovered_idx;
				}

				if (ImGui::MenuItem("Remove Shape"))
				{
					for (auto shape = 0; shape < shape_count; shape++)
					{
						const int pt = hovered_index_on_context;// - 1;
						if(pt < 0)
						{
							game::Com_Error("hovered_index_on_context < 0");
						}

						if(pt >= shapes[shape].offset_vertex && pt < shapes[shape].offset_vertex + shapes[shape].num_vertex)
						{
							//game::printf_to_console("about to delete shape %d using index %d", shape, hovered_index_on_context);
							shapes[shape].pending_deletion = true;
							changed_idx = hovered_index_on_context;

							break;
						}

						// delete last shape if point is out of bounds
						if(shape + 1 >= shape_count)
						{
							shapes[shape].pending_deletion = true;
							changed_idx = hovered_index_on_context;

							break;
						}
					}

					
				}
				ImGui::EndPopup();
			}
			else if(hovered_index_on_context >= 0)
			{
				//game::printf_to_console("RESET: hovered idx was: %d", hovered_index_on_context);
				hovered_index_on_context = -1;
			}
		}

		return changed_idx;
	}
}