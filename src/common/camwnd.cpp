#include "std_include.hpp"

ccamwnd* ccamwnd::activewnd;
IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define RTT_CAMERA_USE_PADDING

// render to texture - camera window
void ccamwnd::rtt_camera_window()
{
	int p_styles = 0;
	int p_colors = 0;

	const auto IO = ImGui::GetIO();
	const auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));
	ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

	float	window_padding = 0.0f;
	if(dvars::gui_rtt_padding_enabled && dvars::gui_rtt_padding_enabled->current.enabled) {
			window_padding = dvars::gui_rtt_padding_size ? dvars::gui_rtt_padding_size->current.integer : 6.0f;
	}
	
	const ImVec2 window_padding_both = ImVec2(window_padding * 2.0f, window_padding * 2.0f);
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(window_padding, window_padding)); p_styles++;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TabUnfocusedActive)); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ToImVec4(dvars::gui_rtt_padding_color->current.vector)); p_colors++;
	
	auto camerawnd = ggui::get_rtt_camerawnd();
	if (camerawnd->should_set_focus)
	{
		ImGui::SetNextWindowFocus();
		camerawnd->should_set_focus = false;
	}

	ImGui::Begin("Camera Window##rtt", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	if (camerawnd->scene_texture)
	{
		bool tabbar_visible = true;
		const auto wnd = ImGui::GetCurrentWindow();

		if(ImGui::IsWindowDocked() && wnd)
		{
			if(wnd->DockNode && wnd->DockNode->IsHiddenTabBar())
			{
				tabbar_visible = false;
			}
		}

		const float frame_height = tabbar_visible ? ImGui::GetFrameHeightWithSpacing() : 0.0f;
		camerawnd->scene_size_imgui = ImGui::GetWindowSize() - ImVec2(0.0f, frame_height) - window_padding_both;

		// hack to disable left mouse window movement
		ImGui::BeginChild("scene_child", ImVec2(camera_size.x, camera_size.y + frame_height) + window_padding_both, false, ImGuiWindowFlags_NoMove);
		{
			const auto screenpos = ImGui::GetCursorScreenPos();
			SetWindowPos(cmainframe::activewnd->m_pCamWnd->GetWindow(), HWND_BOTTOM, (int)screenpos.x, (int)screenpos.y, (int)camerawnd->scene_size_imgui.x, (int)camerawnd->scene_size_imgui.y, SWP_NOZORDER);

			const auto pre_image_cursor = ImGui::GetCursorPos();

			ImGui::Image(camerawnd->scene_texture, camera_size);
			camerawnd->window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
			
			// pop ItemSpacing
			ImGui::PopStyleVar(); p_styles--;

			ImGui::SetCursorPos(pre_image_cursor);
			const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

			camerawnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
			camerawnd->cursor_pos_pt = CPoint((LONG)camerawnd->cursor_pos.x, (LONG)camerawnd->cursor_pos.y);

			ggui::FixDockingTabbarTriangle(wnd, camerawnd);
			ImGui::EndChild();
		}
	}
	ImGui::PopStyleColor(p_colors);
	ImGui::PopStyleVar(p_styles);
	ImGui::End();
}

void ccamwnd::mouse_control(float dtime)
{
	const static uint32_t Cam_MouseControl_Func = 0x403950;
	__asm
	{
		push	esi;
		mov		esi, ecx; // esi = this
		push	dtime;
		call	Cam_MouseControl_Func; // __userpurge :: automatically fixes the stack
		pop		esi;
	}

	return;
}

void ccamwnd::mouse_up(ccamwnd* cam, int flags)
{
	cam->m_nCambuttonstate = 0;
	game::Drag_MouseUp(flags);
	cam->prob_some_cursor = 0;
	cam->x47 = 0;
	cam->x48 = 1; // prob. int16

	int sw_cur;
	do {
		sw_cur = ShowCursor(1);
	} while (sw_cur < 0);
}

void ccamwnd::mouse_moved(ccamwnd* wnd, int buttons, int x, int y)
{
	const static uint32_t CCamWnd__Cam_MouseMoved_Func = 0x404FC0;
	__asm
	{
		pushad;

		mov		eax, buttons;
		push	y;
		push	x;
		mov		ecx, wnd;
		call	CCamWnd__Cam_MouseMoved_Func; // cleans the stack

		popad;
	}
}


// *
// | -------------------- MSG typedefs ------------------------
// *


typedef void(__thiscall* on_ccamwnd_msg)(ccamwnd*, UINT, CPoint);

	// mouse scrolling handled in cmainframe
    on_ccamwnd_msg __on_lbutton_down;
    on_ccamwnd_msg __on_lbutton_up;
    on_ccamwnd_msg __on_rbutton_down;
    on_ccamwnd_msg __on_rbutton_up;
	on_ccamwnd_msg __on_mouse_move;


typedef void(__stdcall* on_ccamwnd_key)(UINT nChar, UINT nRepCnt, UINT nFlags);

	on_ccamwnd_key __on_keydown_cam;
	on_ccamwnd_key __on_keyup_cam;


// *
// | ----------------- Left Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_lbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_lbutton_down(pThis, nFlags, point);
}

void __fastcall ccamwnd::on_lbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_lbutton_up(pThis, nFlags, point);
}


// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_rbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_rbutton_down(pThis, nFlags, point);
}

void __fastcall ccamwnd::on_rbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_rbutton_up(pThis, nFlags, point);
}


// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall ccamwnd::on_mouse_move(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_mouse_move(pThis, nFlags, point);
}


// *
// | ----------------- Keys Up/Down ---------------------
// *

void __stdcall ccamwnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyDown
	__on_keydown_cam(nChar, nRepCnt, nFlags);
}

void __stdcall ccamwnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyUp
	__on_keyup_cam(nChar, nRepCnt, nFlags);
}


// *
// | ----------------- Windowproc ---------------------
// *

BOOL WINAPI ccamwnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// handle text input when the imgui camera window is active (selected) 
	// but the user is within a textbox in some other imgui window
	if (Msg == WM_CHAR || Msg == WM_KEYDOWN || Msg == WM_KEYUP)
	{
		if (ggui::cz_context_ready())
		{
			// set cz context (in-case we use multiple imgui context's)
			IMGUI_BEGIN_CZWND;

			if (!ggui::get_rtt_camerawnd()->window_hovered && ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
	}
	
	// => og CamWndProc
    return utils::hook::call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x402D90)(hWnd, Msg, wParam, lParam);
}


void __declspec(naked) camwnd_set_child_window_style()
{
	const static uint32_t retn_pt = 0x402EF7;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD; //WS_VISIBLE;
		jmp		retn_pt;
	}
}

void __declspec(naked) camwnd_set_detatched_child_window_style()
{
	const static uint32_t retn_pt = 0x402ED1;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_POPUP;
		jmp		retn_pt;
	}
}


// *
// *

void ccamwnd::hooks()
{
	// hook ccamwnd message handler
    utils::hook::set(0x402E86 + 4, ccamwnd::windowproc);

#if 1
	// ccamwnd::precreatewindow -> change window style for child windows (split view, not detatched)
	utils::hook::nop(0x402EEE, 2);
	utils::hook::nop(0x402EF0, 7);
		 utils::hook(0x402EF0, camwnd_set_child_window_style, HOOK_JUMP).install()->quick();

	// ccamwnd::precreatewindow -> change window style for detatched windows (split view, detatched)
	utils::hook::nop(0x402ECA, 7);
		 utils::hook(0x402ECA, camwnd_set_detatched_child_window_style, HOOK_JUMP).install()->quick();
#endif
	
	__on_lbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403160, ccamwnd::on_lbutton_down, HK_JUMP));
    __on_lbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4031D0, ccamwnd::on_lbutton_up, HK_JUMP));

    __on_rbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4032B0, ccamwnd::on_rbutton_down, HK_JUMP));
    __on_rbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403310, ccamwnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403100, ccamwnd::on_mouse_move, HK_JUMP));

	__on_keydown_cam	= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x402F60, ccamwnd::on_keydown, HK_JUMP));
	__on_keyup_cam		= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x408B70, ccamwnd::on_keyup, HK_JUMP));
}