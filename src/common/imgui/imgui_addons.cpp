#include "std_include.hpp"
#include "imgui_internal.h"

static const float NODE_SLOT_RADIUS = 4.0f;

bool set_cursor_pos_w(int x, int y)
{
	return SetCursorPos(x, y);
}

namespace ImGui
{
	bool IsVertScollbarVisible()
	{
		ImGuiWindow* window = GImGui->CurrentWindow;
		return window->ScrollbarY;
	}

	bool Checkbox_FxElemFlag(const char* name, fx_system::FxEditorElemDef* elem, fx_system::FX_ED_FLAG_ flag, bool* result)
	{
		bool flag_wrapper = elem->editorFlags & flag;
		if (ImGui::Checkbox(name, &flag_wrapper))
		{
			if (flag_wrapper)
			{
				elem->editorFlags |= flag;
			}
			else
			{
				elem->editorFlags &= ~flag;
			}

			if (result)
			{
				*result = true;
			}

			return true;
		}

		if (result)
		{
			*result = flag_wrapper;
		}

		return false;
	}

	bool Checkbox_FxElemFlag(const char* name, fx_system::FxEditorElemDef* elem, fx_system::FX_ELEM_ flag, bool* result)
	{
		bool flag_wrapper = elem->flags & flag;
		if (ImGui::Checkbox(name, &flag_wrapper))
		{
			if (flag_wrapper)
			{
				elem->flags |= flag;
			}
			else
			{
				elem->flags &= ~flag;
			}

			if(result)
			{
				*result = true;
			}

			return true;
		}

		if (result)
		{
			*result = flag_wrapper;
		}

		return false;
	}

	bool DragFloat2_FxFloatRange(const char* name, fx_system::FxFloatRange* range, float speed, float min, float max, const char* format)
	{
		float range_wrapper[2] =
		{
			range->base,
			range->base + range->amplitude
		};

		if (ImGui::DragFloat2(name, range_wrapper, speed, min, max, format))
		{
			range->base = range_wrapper[0];
			range->amplitude = range_wrapper[1] - range_wrapper[0];

			return true;
		}

		return false;
	}

	static ImVec2 node_pos;
	static ImGuiID last_node_id;


	float toRange2( float OLD_MIN, float OLD_MAX,	// original range
					float NEW_MIN, float NEW_MAX,	// desired range
					float value)					// value to convert
	{
		float old_range = (value - OLD_MIN) / (OLD_MAX - OLD_MIN);
		return ((NEW_MAX - NEW_MIN) * old_range) + NEW_MIN;
	}

	// https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930
	int CurveEditor(const char* label, float* values, int points_count, const ImVec2& editor_size, ImU32 flags, int* new_count)
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

		static ImVec2 start_pan;
		int hovered_idx = -1;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
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

		ImVec2 points_min(0.0f, 0.0f);
		ImVec2 points_max(1.0f, 1.0f);

		float from_x = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_X, points_min.x) ;
		float from_y = window->StateStorage.GetFloat((ImGuiID)StorageValues::FROM_Y, points_min.y);
		float width  = window->StateStorage.GetFloat((ImGuiID)StorageValues::WIDTH, points_max.x - points_min.x);
		float height = window->StateStorage.GetFloat((ImGuiID)StorageValues::HEIGHT, points_max.y - points_min.y);

		if (width > 1.0f) {
			width = 1.0f;
		}

		if (height > 1.0f) {
			height = 1.0f;
		}

		if (from_x < 0.0f) {
			from_x = 0.0f;
		}
		else if (from_x + width > 1.0f) {
			from_x = 1.0f - width;
		}

		if (from_y < 0.0f) {
			from_y = 0.0f;
		}
		else if (from_y + height > 1.0f) {
			from_y = 1.0f - height;
		}

		window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_X, from_x);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::FROM_Y, from_y);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::WIDTH, width);
		window->StateStorage.SetFloat((ImGuiID)StorageValues::HEIGHT, height);

		const ImRect inner_bb = window->InnerRect;

		auto transform = [&](const ImVec2& pos) -> ImVec2
		{
			float x = (pos.x - from_x) / width;
			float y = (pos.y - from_y) / height;

			return ImVec2(inner_bb.Min.x * (1 - x) + inner_bb.Max.x * x, inner_bb.Min.y * y + inner_bb.Max.y * (1 - y));
		};

		auto invTransform = [&](const ImVec2& pos) -> ImVec2
		{
			float x = (pos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x);
			float y = (inner_bb.Max.y - pos.y) / (inner_bb.Max.y - inner_bb.Min.y);

			return ImVec2(from_x + width * x, from_y + height * y);
		};

		if (flags & (int)CurveEditorFlags::SHOW_GRID)
		{
			int exp;
			frexp(width / 5.0f, &exp);

			float step_x = ldexp(1.0f, exp);
			int cell_cols = static_cast<int>(width / step_x);
			float x = step_x * static_cast<int>((from_x / step_x));

			for (int i = -1; i < cell_cols + 2; ++i)
			{
				ImVec2 a = transform({ x + i * step_x, from_y });
				ImVec2 b = transform({ x + i * step_x, from_y + height });
				window->DrawList->AddLine(a, b, grid_color);
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

			const float step_y	= ldexp(1.0f, exp);
			const int cell_rows = static_cast<int>(height / step_y);
			const float y		= step_y * static_cast<float>( static_cast<int>(from_y / step_y) );

			for (int i = -1; i < cell_rows + 2; ++i)
			{
				ImVec2 a = transform({ from_x, y + i * step_y });
				ImVec2 b = transform({ from_x + width, y + i * step_y });

				window->DrawList->AddLine(a, b, grid_color);
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

				if(pos.x - (SIZE * 0.75f) < inner_bb.Min.x)
				{
					pos.x += (SIZE * 0.75f);
				}
				if (pos.y - (SIZE * 0.75f) < inner_bb.Min.y)
				{
					pos.y += (SIZE * 0.75f);
				}

				if (pos.x + (SIZE * 0.75f) > inner_bb.Max.x)
				{
					pos.x -= (SIZE * 0.75f);
				}
				if (pos.y + (SIZE * 0.75f) > inner_bb.Max.y)
				{
					pos.y -= (SIZE * 0.75f);
				}

				SetCursorScreenPos(pos - ImVec2(SIZE, SIZE));
				PushID(idx);
				InvisibleButton("", ImVec2(2 * NODE_SLOT_RADIUS, 2 * NODE_SLOT_RADIUS));

				const ImU32 col = IsItemActive() || IsItemHovered() ? GetColorU32(ImGuiCol_PlotLinesHovered)
					: GetColorU32(ImGuiCol_PlotLines);

				window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0,  SIZE), col);
				window->DrawList->AddLine(pos + ImVec2( SIZE, 0), pos + ImVec2(0,  SIZE), col);
				window->DrawList->AddLine(pos + ImVec2( SIZE, 0), pos + ImVec2(0, -SIZE), col);
				window->DrawList->AddLine(pos + ImVec2(-SIZE, 0), pos + ImVec2(0, -SIZE), col);

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
						pos.x = inner_bb.Max.x - position_str_size.x;
					}

					if (pos.y - position_str_size.y < inner_bb.Min.y)
					{
						pos.y = inner_bb.Min.y + position_str_size.y;
					}

					char tmp[64];
					ImFormatString(tmp, sizeof(tmp), position_str);
					window->DrawList->AddText({ pos.x, pos.y - GetTextLineHeight() }, point_hover_text_color, tmp);
				}

				if (IsItemActive() && IsMouseDragging(0))
				{
					pos.x = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_X, pos.x);
					pos.y = window->StateStorage.GetFloat((ImGuiID)StorageValues::POINT_START_Y, pos.y);
					pos += ImGui::GetMouseDragDelta();

					ImVec2 v = invTransform(pos);

					if(idx == 0)
					{
						p = ImClamp(v, ImVec2(0.0f, 0.0f), ImVec2(0.0f, 1.0f));
					}
					else if(_first_point)
					{
						p = ImClamp(v, ImVec2(1.0f, 0.0f), ImVec2(1.0f, 1.0f));
					}
					else
					{
						p = ImClamp(v, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
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

		if (hovered_idx >= 0 && ImGui::IsMouseDoubleClicked(0) && new_count && points_count > 2)
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

	ImGuiID FindNodeByID(ImGuiID id)
	{
		if(const auto node = (ImGuiDockNode*)GImGui->DockContext.Nodes.GetVoidPtr(id);
					  node)
		{
			return node->ID;
		}
		
		return 0;
	}
	
	void DockBuilderDockWindow_FirstUseOrSaved(const char* window_name, ImGuiID node_id)
	{
		ImGuiID window_id = ImHashStr(window_name);
		ImGuiWindowSettings* settings = FindWindowSettings(window_id);
		
		if (ImGuiWindow* window = FindWindowByID(window_id))
		{
			if(settings)
			{
				SetWindowDock(window, settings->DockId, ImGuiCond_FirstUseEver);
				window->DockOrder = settings->DockOrder;
			}
			else
			{
				// Apply to created window
				SetWindowDock(window, node_id, ImGuiCond_FirstUseEver);
				window->DockOrder = -1;
			}
		}
		else
		{
			if (settings == nullptr)
			{
				settings = CreateNewWindowSettings(window_name);
				settings->DockId = node_id;
				settings->DockOrder = -1;
			}
		}
	}

	void PushStyleCompact()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
	}

	void PopStyleCompact()
	{
		ImGui::PopStyleVar(2);
	}

	bool BeginTabItem_SmallGap(const char* label, bool* p_open, ImGuiTabItemFlags flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(1, 4));
		const bool result = ImGui::BeginTabItem(label, p_open, flags);
		ImGui::PopStyleVar();
		return result;
	}
	
	bool IsItemHoveredDelay(float delay_in_seconds)
	{
		ImGuiContext& g = *GImGui;

		if (ImGui::IsItemHovered() && !g.IO.MouseDown[0] && GImGui->HoveredIdTimer > delay_in_seconds)
		{
			return true;
		}

		return false;
	}

	bool IsResizing()
	{
		switch (ImGui::GetMouseCursor())
		{
		case ImGuiMouseCursor_ResizeAll:
		case ImGuiMouseCursor_ResizeNS:
		case ImGuiMouseCursor_ResizeEW:
		case ImGuiMouseCursor_ResizeNESW:
		case ImGuiMouseCursor_ResizeNWSE:
			return true;
		}

		return false;
	}

	// "custom" ImGui_ImplWin32_WndProcHandler
	// * hook a wndclass::function handling input and call this function with the corrosponding WM_ msg
	void HandleKeyIO(HWND hwnd, UINT key, SHORT zDelta, UINT nChar)
	{
		if (ImGui::GetCurrentContext() == NULL)
			return;

		ImGuiIO& io = ImGui::GetIO();

		switch (key)
		{
		case WM_SETFOCUS:
			std::fill_n(io.KeysDown, 512, 0);
			return;

		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		{
			int button = 0;
			if (key == WM_LBUTTONDOWN || key == WM_LBUTTONDBLCLK) { button = 0; }
			if (key == WM_RBUTTONDOWN || key == WM_RBUTTONDBLCLK) { button = 1; }
			if (key == WM_MBUTTONDOWN || key == WM_MBUTTONDBLCLK) { button = 2; }
			if (!ImGui::IsAnyMouseDown() && ::GetCapture() == nullptr)
				::SetCapture(hwnd);
			io.MouseDown[button] = true;
			return;
		}

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			int button = 0;
			if (key == WM_LBUTTONUP) { button = 0; }
			if (key == WM_RBUTTONUP) { button = 1; }
			if (key == WM_MBUTTONUP) { button = 2; }
			io.MouseDown[button] = false;
			if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
				::ReleaseCapture();
			return;
		}

		case WM_MOUSEWHEEL:
			io.MouseWheel += static_cast<float>(zDelta) / 120.0f; // WHEEL_DELTA
			return;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (nChar < 256)
				io.KeysDown[nChar] = true;
			return;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (nChar < 256)
				io.KeysDown[nChar] = false;
			return;
		}
	}

	ImVec4 ToImVec4(const float* in)
	{
		if(in)
		{
			return ImVec4(in[0], in[1], in[2], in[3]);
		}

		return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void PushFontFromIndex(int font)
	{
		ImGuiIO& io = ImGui::GetIO();

		if(io.Fonts->Fonts[font])
		{
			ImGui::PushFont(io.Fonts->Fonts[font]);
		}
		else
		{
			ImGui::PushFont(ImGui::GetDefaultFont());
		}
	}

	void AddUnterline(ImColor col)
	{
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		min.y = max.y;
		ImGui::GetWindowDrawList()->AddLine(min, max, col, 1.0f);
	}

	void TextURL(const char* name, const char* url)
	{
		ImGui::TextUnformatted(name);

		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseClicked(0))
			{
				ShellExecuteA(0, 0, url, 0, 0, SW_SHOW);
			}

			AddUnterline(ImGui::GetStyle().Colors[ImGuiCol_TabHovered]);
			ImGui::SetTooltip("Open in browser\n[%s]", url);
		}
		else
		{
			AddUnterline(ImGui::GetStyle().Colors[ImGuiCol_Button]);
		}
	}

	void SetCursorForCenteredText(const char* text)
	{
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text).x) * 0.5f);
	}

	void title_with_background(const char* title_text, const ImVec2& pos, const float width, const float height, const float* bg_color, const float* border_color, bool pre_spacing, const float text_indent)
	{
		if (bg_color && border_color)
		{
			if (pre_spacing)
			{
				SPACING(0.0f, 8.0f);
			}

			// GetForegroundDrawList

			ImVec2 text_pos = ImGui::GetCursorScreenPos();
			text_pos.x += text_indent;

			ImGui::PushFontFromIndex(ggui::BOLD_18PX);
			text_pos.y = text_pos.y + (height * 0.5f - ImGui::CalcTextSize(title_text).y * 0.5f);

			ImVec2 max = ImVec2(pos.x + width, pos.y + height);
			ImGui::GetWindowDrawList()->AddRectFilled(pos, max, ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(bg_color)), 0.0f);
			ImGui::GetWindowDrawList()->AddRect(pos, max, ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(border_color)), 0.0f);
			//ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::GetWindowDrawList()->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), title_text);
			ImGui::PopFont();

			SPACING(0.0f, 40.0f);
		}
	}

	void title_with_seperator(const char* title_text, bool pre_spacing, float width, float height, float post_spacing)
	{
		if (pre_spacing)
		{
			SPACING(0.0f, 12.0f);
		}

		if (width == 0.0f)
		{
			width = ImGui::GetContentRegionAvailWidth() - 16.0f;
		}

		ImGui::PushFontFromIndex(ggui::BOLD_18PX);
		ImGui::TextUnformatted(title_text);
		ImGui::PopFont();
		//ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

		const ImVec2 seperator_pos = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddLine(seperator_pos, ImVec2(seperator_pos.x + width, seperator_pos.y + height), ImGui::GetColorU32(ImGuiCol_Separator));

		SPACING(0.0f, post_spacing);
	}

	bool InputScalarDir(const char* label, ImGuiDataType data_type, void* p_data, int* dir, void* p_step, const void* p_step_fast, bool display_p_step, const char* format, ImGuiInputTextFlags flags)
	{
		if (GetCurrentWindow()->SkipItems)
		{
			return false;
		}

		ImGuiContext& g = *GImGui;
		ImGuiStyle& style = g.Style;

		if (format == nullptr)
		{
			format = DataTypeGetInfo(data_type)->PrintFmt;
		}

		char buf[64];
		DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, p_data, format);

		bool value_changed = false;

		if ((flags & (ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
		{
			flags |= ImGuiInputTextFlags_CharsDecimal;
		}
		flags |= ImGuiInputTextFlags_AutoSelectAll;
		flags |= ImGuiInputTextFlags_NoMarkEdited;  // We call MarkItemEdited() ourselves by comparing the actual data rather than the string.

		if (p_step != nullptr)
		{
			const float dragfloat_size = 50.0f;
			const float button_size = GetFrameHeight();

			BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
			PushID(label);

			SetNextItemWidth(ImMax(68.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2 - dragfloat_size + style.ItemInnerSpacing.x));
			
			if (InputText("", buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
			{
				value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);
			}

			// Step buttons
			const ImVec2 backup_frame_padding = style.FramePadding;
			style.FramePadding.x = style.FramePadding.y;
			ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;

			SameLine(0, style.ItemInnerSpacing.x);
			if (ButtonEx("-", ImVec2(button_size, button_size), button_flags))
			{
				DataTypeApplyOp(data_type, '-', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
				value_changed = true;
				if (dir)
				{
					*dir = -1;
				}
			}

			SameLine(0, style.ItemInnerSpacing.x);
			if (ButtonEx("+", ImVec2(button_size, button_size), button_flags))
			{
				DataTypeApplyOp(data_type, '+', p_data, p_data, g.IO.KeyCtrl && p_step_fast ? p_step_fast : p_step);
				value_changed = true;

				if (dir)
				{
					*dir = 1;
				}
			}

			if(display_p_step)
			{
				SameLine(0, style.ItemInnerSpacing.x);

				PushStyleColor(ImGuiCol_FrameBg, GetColorU32(ImGuiCol_Button));
				PushStyleColor(ImGuiCol_FrameBgHovered, GetColorU32(ImGuiCol_ButtonHovered));
				PushStyleColor(ImGuiCol_FrameBgActive, GetColorU32(ImGuiCol_ButtonActive));

				SetNextItemWidth(dragfloat_size);
				ImGui::DragFloat("##amount", static_cast<float*>(p_step), 0.5f, 0.1f, 256.0f, "%.1f");
				TT("Inc/Dec Amount");

				PopStyleColor(3);
			}

			const char* label_end = FindRenderedTextEnd(label);
			if (label != label_end)
			{
				SameLine(0, style.ItemInnerSpacing.x);
				TextEx(label, label_end);
			}

			style.FramePadding = backup_frame_padding;

			PopID();
			EndGroup();
		}
		else
		{
			if (InputText(label, buf, IM_ARRAYSIZE(buf), flags))
				value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);
		}

		if (value_changed)
		{
			MarkItemEdited(g.LastItemData.ID);
		}

		return value_changed;
	}
	
	void debug_table_entry_vec3(const char* label, const float* vec3)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		for (auto i = 0; i < 3; i++)
		{
			ImGui::TableNextColumn();
			ImGui::Text("%.2f", vec3[i]);
		}
	}

	void debug_table_entry_float(const char* label, const float* val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		ImGui::TableNextColumn();
		ImGui::Text("%.2f", val);
	}

	void debug_table_entry_int(const char* label, const int val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		ImGui::TableNextColumn();
		ImGui::Text("%d", val);
	}

	void debug_table_entry_int2(const char* label, const int* val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);

		for (auto i = 0; i < 2; i++)
		{
			ImGui::TableNextColumn();
			ImGui::Text("%d", val[i]);
		}
	}

	void StyleColorsDevgui(ImGuiStyle* dst)
	{
		ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		style->AntiAliasedLines = true;
		style->AntiAliasedFill = true;

		// Main
		style->DisplaySafeAreaPadding.y = 18.0f; // fix child menus going out of view
		style->WindowPadding.x = 5.0f;
		style->WindowPadding.y = 5.0f;
		style->PopupRounding = 2.0f;
		style->FramePadding.x = 10.0f;
		style->FramePadding.y = 5.0f;
		style->ItemSpacing.x = 10.0f;
		style->ItemSpacing.y = 5.0f;
		style->ItemInnerSpacing.x = 10.0f;
		style->ItemInnerSpacing.y = 4.0f;
		style->TouchExtraPadding.x = 0.0f;
		style->TouchExtraPadding.y = 0.0f;
		style->IndentSpacing = 30.0f;
		style->ScrollbarSize = 14.0f;
		style->GrabMinSize = 10.0f;

		// Borders
		style->WindowBorderSize = 1.0f;
		style->ChildBorderSize = 1.0f;
		style->PopupBorderSize = 1.0f;
		style->FrameBorderSize = 1.0f;
		style->TabBorderSize = 0.0f;

		// Rounding
		style->WindowRounding = 2.0f;
		style->ChildRounding = 2.0f;

		// 04.10.21
		//style->FrameRounding = 2.0f;
		style->FrameRounding = 5.0f;
		
		style->ScrollbarRounding = 2.0f;
		style->GrabRounding = 2.0f;

		// 04.10.21
		//style->TabRounding = 2.0f;	
		style->TabRounding = 6.0f;
		
		// Alignment
		style->WindowTitleAlign.x = 0.5f;
		style->WindowTitleAlign.y = 0.5f;
		style->Alpha = 1.0f;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.84f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 0.64f);

		// 04.10.21
		//colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.82f);						
		colors[ImGuiCol_WindowBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);

		// 04.10.21
		//colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);						
		colors[ImGuiCol_ChildBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);	
		
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.93f);
		colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 0.55f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.55f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.44f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);

		// 04.10.21
		//colors[ImGuiCol_TitleBg] = ImVec4(0.30f, 0.01f, 0.02f, 0.35f);						
		//colors[ImGuiCol_TitleBgActive] = ImVec4(0.30f, 0.01f, 0.02f, 0.51f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.66f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.11f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.24f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.99f, 1.00f, 1.00f, 0.24f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);

		//colors[ImGuiCol_SliderGrab] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		//colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.59f, 0.59f, 0.59f, 0.21f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		
		colors[ImGuiCol_Button] = ImVec4(0.24f, 0.24f, 0.24f, 0.55f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.39f, 0.39f, 0.39f, 0.73f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.55f, 0.55f, 0.55f, 0.74f);
		colors[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.21f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.36f);
		colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.36f);

		// 04.10.21
		//colors[ImGuiCol_Separator] = ImVec4(1.00f, 1.00f, 1.00f, 0.09f);						
		colors[ImGuiCol_Separator] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		
		colors[ImGuiCol_SeparatorHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.09f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);

		// 04.10.21
		//colors[ImGuiCol_Tab] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
		//colors[ImGuiCol_TabHovered] = ImVec4(0.52f, 0.01f, 0.02f, 0.63f);
		//colors[ImGuiCol_TabActive] = ImVec4(0.52f, 0.01f, 0.02f, 0.45f);
		//colors[ImGuiCol_TabUnfocused] = ImVec4(0.42f, 0.00f, 0.02f, 0.00f);
		//colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.52f, 0.01f, 0.02f, 0.45f);
		colors[ImGuiCol_Tab] = ImVec4(0.17f, 0.17f, 0.17f, 0.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.49f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.49f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.17f, 0.17f, 0.17f, 0.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
		
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.70f, 0.62f, 0.30f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.27f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.34f);
	}
}
