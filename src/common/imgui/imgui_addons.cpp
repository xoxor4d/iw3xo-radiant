#include "std_include.hpp"
#include "imgui_internal.h"

namespace ImGui
{
	// https://github.com/ocornut/imgui/issues/1901 (zfedoran)
	bool Spinner(const char* label, float radius, float thickness, const ImU32& color)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (window->SkipItems)
		{
			return false;
		}

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const ImVec2 pos = window->DC.CursorPos;
		const ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ItemSize(bb, style.FramePadding.y);

		if (!ItemAdd(bb, id))
		{
			return false;
		}
			

		// Render
		window->DrawList->PathClear();

		const int num_segments = 30;
		const auto num_segments_f = static_cast<float>(num_segments);

		const int start = static_cast<int>(abs(ImSin(static_cast<float>(g.Time * 1.8)) * (num_segments - 5)));

		const float a_min = IM_PI * 2.0f * static_cast<float>(start) / num_segments_f;
		const float a_max = IM_PI * 2.0f * (num_segments_f - 3.0f) / num_segments_f;

		const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

		for (int i = 0; i < num_segments; i++) 
		{
			const float a = a_min + ((float)i / num_segments_f) * (a_max - a_min);
			window->DrawList->PathLineTo(
				ImVec2(centre.x + ImCos(a + static_cast<float>(g.Time * 8.0)) * radius,
				centre.y + ImSin(a + static_cast<float>(g.Time * 8.0)) * radius));
		}

		window->DrawList->PathStroke(color, false, thickness);

		return true;
	}

	bool IsVertScollbarVisible()
	{
		ImGuiWindow* window = GImGui->CurrentWindow;
		return window->ScrollbarY;
	}

	void left_label_drag(const char* label, const float text_y_offset, const float sameline_offset)
	{
		const auto og_cursor_y = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + text_y_offset);
		ImGui::TextUnformatted(label);
		ImGui::SameLine(sameline_offset);
		ImGui::SetCursorPosY(og_cursor_y);
		ImGui::SetNextItemWidth(-ImGui::GetStyle().FramePadding.x);
	}

	void left_label_checkbox(const char* label, const float text_y_offset)
	{
		const auto og_cursor_y = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + text_y_offset);
		ImGui::TextUnformatted(label);
		ImGui::SameLine();
		ImGui::SetCursorPosY(og_cursor_y);
	};

	void DvarBool_External(const char* checkbox_label, const char* dvar_name)
	{
		if (const auto handle = game::Dvar_FindVar(dvar_name); handle)
		{
			if (ImGui::Checkbox(checkbox_label, &handle->current.enabled))
			{
				//dvars::set_bool(handle, handle->current.enabled);
				handle->modified = true;
			} TT(utils::va("%s :: %s", dvar_name, handle->description));
		}
	}

	void DvarInt_External(const char* checkbox_label, const char* dvar_name)
	{
		if (const auto handle = game::Dvar_FindVar(dvar_name); handle)
		{
			if (ImGui::DragInt(checkbox_label, &handle->current.integer, 0.01f, handle->domain.integer.min, handle->domain.integer.max))
			{
				handle->modified = true;
			} TT(utils::va("%s :: %s", dvar_name, handle->description));
		}
	}

	void DvarFloat_External(const char* checkbox_label, const char* dvar_name)
	{
		if (const auto handle = game::Dvar_FindVar(dvar_name); handle)
		{
			if (ImGui::DragFloat(checkbox_label, &handle->current.value, 0.01f, handle->domain.value.min, handle->domain.value.max, "%.2f"))
			{
				handle->modified = true;
			} TT(utils::va("%s :: %s", dvar_name, handle->description));
		}
	}

	void DvarEnum_External(const char* label, const char* dvar_name)
	{
		const auto handle = game::Dvar_FindVar(dvar_name);

		if (ImGui::SliderInt(label, &handle->current.integer, 0, handle->domain.enumeration.stringCount - 1, handle->domain.enumeration.strings[handle->current.integer]))
		{
			handle->modified = true;
		} TT(utils::va("%s :: %s", dvar_name, handle->description));
	}
		
	bool Checkbox_FxElemFlag(const char* name, fx_system::FxEditorElemDef* elem, fx_system::FX_ED_FLAG_ flag, bool* result, bool invert_selected)
	{
		bool flag_wrapper = elem->editorFlags & flag;

		if(invert_selected)
		{
			flag_wrapper = !flag_wrapper;
		}

		if (ImGui::Checkbox(name, &flag_wrapper))
		{
			if (!invert_selected ? flag_wrapper : !flag_wrapper)
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

	static bool IsRootOfOpenMenuSet()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if ((g.OpenPopupStack.Size <= g.BeginPopupStack.Size) || (window->Flags & ImGuiWindowFlags_ChildMenu))
			return false;

		// Initially we used 'OpenParentId' to differentiate multiple menu sets from each others (e.g. inside menu bar vs loose menu items) based on parent ID.
		// This would however prevent the use of e.g. PuhsID() user code submitting menus.
		// Previously this worked between popup and a first child menu because the first child menu always had the _ChildWindow flag,
		// making  hovering on parent popup possible while first child menu was focused - but this was generally a bug with other side effects.
		// Instead we don't treat Popup specifically (in order to consistently support menu features in them), maybe the first child menu of a Popup
		// doesn't have the _ChildWindow flag, and we rely on this IsRootOfOpenMenuSet() check to allow hovering between root window/popup and first chilld menu.
		const ImGuiPopupData* upper_popup = &g.OpenPopupStack[g.BeginPopupStack.Size];
		return (/*upper_popup->OpenParentId == window->IDStack.back() &&*/ upper_popup->Window && (upper_popup->Window->Flags & ImGuiWindowFlags_ChildMenu));
	}

	bool MenuItemFlags(const char* label, bool selected, bool enabled, ImGuiSelectableFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		ImGuiStyle& style = g.Style;
		ImVec2 pos = window->DC.CursorPos;
		ImVec2 label_size = CalcTextSize(label, NULL, true);

		const bool menuset_is_open = IsRootOfOpenMenuSet();
		ImGuiWindow* backed_nav_window = g.NavWindow;
		if (menuset_is_open)
			g.NavWindow = window;

		// We've been using the equivalent of ImGuiSelectableFlags_SetNavIdOnHover on all Selectable() since early Nav system days (commit 43ee5d73),
		// but I am unsure whether this should be kept at all. For now moved it to be an opt-in feature used by menus only.
		bool pressed;
		PushID(label);
		if (!enabled)
			BeginDisabled();

		const ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SelectOnRelease | ImGuiSelectableFlags_SetNavIdOnHover | flags;
		const ImGuiMenuColumns* offsets = &window->DC.MenuColumns;
		if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
		{
			// Mimic the exact layout spacing of BeginMenu() to allow MenuItem() inside a menu bar, which is a little misleading but may be useful
			// Note that in this situation: we don't render the shortcut, we render a highlight instead of the selected tick mark.
			float w = label_size.x;
			window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * 0.5f);
			ImVec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
			PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
			pressed = Selectable("", selected, selectable_flags, ImVec2(w, 0.0f));
			PopStyleVar();
			RenderText(text_pos, label);
			window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
		}
		else
		{
			// Menu item inside a vertical menu
			// (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
			//  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.
			float icon_w = 0.0f;
			float shortcut_w = 0.0f;
			float checkmark_w = IM_FLOOR(g.FontSize * 1.20f);
			float min_w = window->DC.MenuColumns.DeclColumns(icon_w, label_size.x, shortcut_w, checkmark_w); // Feedback for next frame
			float stretch_w = ImMax(0.0f, GetContentRegionAvail().x - min_w);
			pressed = Selectable("", false, selectable_flags | ImGuiSelectableFlags_SpanAvailWidth, ImVec2(min_w, 0.0f));
			RenderText(pos + ImVec2(offsets->OffsetLabel, 0.0f), label);

			if (selected)
				RenderCheckMark(window->DrawList, pos + ImVec2(offsets->OffsetMark + stretch_w + g.FontSize * 0.40f, g.FontSize * 0.134f * 0.5f), GetColorU32(ImGuiCol_Text), g.FontSize * 0.866f);
		}

		IMGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (selected ? ImGuiItemStatusFlags_Checked : 0));

		if (!enabled)
			EndDisabled();

		PopID();

		if (menuset_is_open)
			g.NavWindow = backed_nav_window;

		return pressed;
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

	struct ImGui_ImplWin32_Data // he
	{
		HWND                        hWnd;
		HWND                        MouseHwnd;
		bool                        MouseTracked;
		int                         MouseButtonsDown;
		INT64                       Time;
		INT64                       TicksPerSecond;
		ImGuiMouseCursor            LastMouseCursor;
		bool                        HasGamepad;
		bool                        WantUpdateHasGamepad;
		bool                        WantUpdateMonitors;
	};

	static bool IsVkDown(int vk)
	{
		return (::GetKeyState(vk) & 0x8000) != 0;
	}

	static void ImGui_ImplWin32_AddKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode = -1)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(key, down);
		io.SetKeyEventNativeData(key, native_keycode, native_scancode); // To support legacy indexing (<1.87 user code)
		IM_UNUSED(native_scancode);
	}

	// Map VK_xxx to ImGuiKey_xxx.
	static ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam)
	{
		switch (wParam)
		{
		case VK_TAB: return ImGuiKey_Tab;
		case VK_LEFT: return ImGuiKey_LeftArrow;
		case VK_RIGHT: return ImGuiKey_RightArrow;
		case VK_UP: return ImGuiKey_UpArrow;
		case VK_DOWN: return ImGuiKey_DownArrow;
		case VK_PRIOR: return ImGuiKey_PageUp;
		case VK_NEXT: return ImGuiKey_PageDown;
		case VK_HOME: return ImGuiKey_Home;
		case VK_END: return ImGuiKey_End;
		case VK_INSERT: return ImGuiKey_Insert;
		case VK_DELETE: return ImGuiKey_Delete;
		case VK_BACK: return ImGuiKey_Backspace;
		case VK_SPACE: return ImGuiKey_Space;
		case VK_RETURN: return ImGuiKey_Enter;
		case VK_ESCAPE: return ImGuiKey_Escape;
		case VK_OEM_7: return ImGuiKey_Apostrophe;
		case VK_OEM_COMMA: return ImGuiKey_Comma;
		case VK_OEM_MINUS: return ImGuiKey_Minus;
		case VK_OEM_PERIOD: return ImGuiKey_Period;
		case VK_OEM_2: return ImGuiKey_Slash;
		case VK_OEM_1: return ImGuiKey_Semicolon;
		case VK_OEM_PLUS: return ImGuiKey_Equal;
		case VK_OEM_4: return ImGuiKey_LeftBracket;
		case VK_OEM_5: return ImGuiKey_Backslash;
		case VK_OEM_6: return ImGuiKey_RightBracket;
		case VK_OEM_3: return ImGuiKey_GraveAccent;
		case VK_CAPITAL: return ImGuiKey_CapsLock;
		case VK_SCROLL: return ImGuiKey_ScrollLock;
		case VK_NUMLOCK: return ImGuiKey_NumLock;
		case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
		case VK_PAUSE: return ImGuiKey_Pause;
		case VK_NUMPAD0: return ImGuiKey_Keypad0;
		case VK_NUMPAD1: return ImGuiKey_Keypad1;
		case VK_NUMPAD2: return ImGuiKey_Keypad2;
		case VK_NUMPAD3: return ImGuiKey_Keypad3;
		case VK_NUMPAD4: return ImGuiKey_Keypad4;
		case VK_NUMPAD5: return ImGuiKey_Keypad5;
		case VK_NUMPAD6: return ImGuiKey_Keypad6;
		case VK_NUMPAD7: return ImGuiKey_Keypad7;
		case VK_NUMPAD8: return ImGuiKey_Keypad8;
		case VK_NUMPAD9: return ImGuiKey_Keypad9;
		case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
		case VK_DIVIDE: return ImGuiKey_KeypadDivide;
		case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case VK_ADD: return ImGuiKey_KeypadAdd;
		case VK_LSHIFT: return ImGuiKey_LeftShift;
		case VK_LCONTROL: return ImGuiKey_LeftCtrl;
		case VK_LMENU: return ImGuiKey_LeftAlt;
		case VK_LWIN: return ImGuiKey_LeftSuper;
		case VK_RSHIFT: return ImGuiKey_RightShift;
		case VK_RCONTROL: return ImGuiKey_RightCtrl;
		case VK_RMENU: return ImGuiKey_RightAlt;
		case VK_RWIN: return ImGuiKey_RightSuper;
		case VK_APPS: return ImGuiKey_Menu;
		case '0': return ImGuiKey_0;
		case '1': return ImGuiKey_1;
		case '2': return ImGuiKey_2;
		case '3': return ImGuiKey_3;
		case '4': return ImGuiKey_4;
		case '5': return ImGuiKey_5;
		case '6': return ImGuiKey_6;
		case '7': return ImGuiKey_7;
		case '8': return ImGuiKey_8;
		case '9': return ImGuiKey_9;
		case 'A': return ImGuiKey_A;
		case 'B': return ImGuiKey_B;
		case 'C': return ImGuiKey_C;
		case 'D': return ImGuiKey_D;
		case 'E': return ImGuiKey_E;
		case 'F': return ImGuiKey_F;
		case 'G': return ImGuiKey_G;
		case 'H': return ImGuiKey_H;
		case 'I': return ImGuiKey_I;
		case 'J': return ImGuiKey_J;
		case 'K': return ImGuiKey_K;
		case 'L': return ImGuiKey_L;
		case 'M': return ImGuiKey_M;
		case 'N': return ImGuiKey_N;
		case 'O': return ImGuiKey_O;
		case 'P': return ImGuiKey_P;
		case 'Q': return ImGuiKey_Q;
		case 'R': return ImGuiKey_R;
		case 'S': return ImGuiKey_S;
		case 'T': return ImGuiKey_T;
		case 'U': return ImGuiKey_U;
		case 'V': return ImGuiKey_V;
		case 'W': return ImGuiKey_W;
		case 'X': return ImGuiKey_X;
		case 'Y': return ImGuiKey_Y;
		case 'Z': return ImGuiKey_Z;
		case VK_F1: return ImGuiKey_F1;
		case VK_F2: return ImGuiKey_F2;
		case VK_F3: return ImGuiKey_F3;
		case VK_F4: return ImGuiKey_F4;
		case VK_F5: return ImGuiKey_F5;
		case VK_F6: return ImGuiKey_F6;
		case VK_F7: return ImGuiKey_F7;
		case VK_F8: return ImGuiKey_F8;
		case VK_F9: return ImGuiKey_F9;
		case VK_F10: return ImGuiKey_F10;
		case VK_F11: return ImGuiKey_F11;
		case VK_F12: return ImGuiKey_F12;
		default: return ImGuiKey_None;
		}
	}

	// "custom" ImGui_ImplWin32_WndProcHandler
	// * hook a wndclass::function handling input and call this function with the corrosponding WM_ msg
	void HandleKeyIO(HWND hwnd, UINT key, SHORT zDelta, UINT nChar)
	{
		if (ImGui::GetCurrentContext() == NULL)
			return;

		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplWin32_Data* bd = ImGui::GetCurrentContext() ? (ImGui_ImplWin32_Data*)ImGui::GetIO().BackendPlatformUserData : NULL;

		switch (key)
		{
			case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
			{
				int button = 0;
				if (key == WM_LBUTTONDOWN || key == WM_LBUTTONDBLCLK) { button = 0; }
				if (key == WM_RBUTTONDOWN || key == WM_RBUTTONDBLCLK) { button = 1; }
				if (key == WM_MBUTTONDOWN || key == WM_MBUTTONDBLCLK) { button = 2; }
				if (bd->MouseButtonsDown == 0 && ::GetCapture() == NULL)
					::SetCapture(hwnd);
				bd->MouseButtonsDown |= 1 << button;
				io.AddMouseButtonEvent(button, true);
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
				bd->MouseButtonsDown &= ~(1 << button);
				if (bd->MouseButtonsDown == 0 && ::GetCapture() == hwnd)
					::ReleaseCapture();
				io.AddMouseButtonEvent(button, false);
				return;
			}

			case WM_MOUSEWHEEL:
				io.AddMouseWheelEvent(0.0f, static_cast<float>(zDelta) / 120.0f);
				return;

			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			{
				const bool is_key_down = (key == WM_KEYDOWN || key == WM_SYSKEYDOWN);
				if (nChar < 256)
				{
					// Submit modifiers
					io.AddKeyEvent(ImGuiKey_ModCtrl, IsVkDown(VK_CONTROL));
					io.AddKeyEvent(ImGuiKey_ModShift, IsVkDown(VK_SHIFT));
					io.AddKeyEvent(ImGuiKey_ModAlt, IsVkDown(VK_MENU));
					io.AddKeyEvent(ImGuiKey_ModSuper, IsVkDown(VK_APPS));

					// Obtain virtual key code
					// (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
					int vk = nChar;

					// Submit key event
					const ImGuiKey keyx = ImGui_ImplWin32_VirtualKeyToImGuiKey(vk);
					if (keyx != ImGuiKey_None)
						ImGui_ImplWin32_AddKeyEvent(keyx, is_key_down, vk);
				}
				return;
			}

			case WM_CHAR:
				// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
				if (nChar > 0 && nChar < 0x10000)
					io.AddInputCharacterUTF16((unsigned short)nChar);
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
				// fix stuck left mouse button
				ImGuiIO& io = ImGui::GetIO();
				io.AddMouseButtonEvent(0, false);

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
			width = ImGui::GetContentRegionAvail().x - 16.0f;
		}

		ImGui::PushFontFromIndex(ggui::BOLD_18PX);
		ImGui::TextUnformatted(title_text);
		ImGui::PopFont();
		//ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

		const ImVec2 seperator_pos = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddLine(seperator_pos, ImVec2(seperator_pos.x + width, seperator_pos.y + height), ImGui::GetColorU32(ImGuiCol_Separator));

		SPACING(0.0f, post_spacing);
	}

	void title_with_seperator_helpmark(const char* title_text, bool pre_spacing, float width, float height, float post_spacing, const char* helper_text)
	{
		if (pre_spacing)
		{
			SPACING(0.0f, 12.0f);
		}

		if (width == 0.0f)
		{
			width = ImGui::GetContentRegionAvail().x - 16.0f;
		}

		ImGui::PushFontFromIndex(ggui::BOLD_18PX);
		ImGui::TextUnformatted(title_text);
		ImGui::PopFont();

		const ImVec2 seperator_pos = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddLine(seperator_pos, ImVec2(seperator_pos.x + width, seperator_pos.y + height), ImGui::GetColorU32(ImGuiCol_Separator));

		ImGui::SameLine(width - 6.0f);
		ImGui::HelpMarker(helper_text);

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
				//value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, p_data, format);
				value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
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
			{
				value_changed = DataTypeApplyFromText(buf, data_type, p_data, format);
			}
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
		
		colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.93f);
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
