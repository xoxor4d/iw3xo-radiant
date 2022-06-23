#include "std_include.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ccamwnd::calculate_ray_direction(int x, int y, float* dir)
{
	const auto cam = cmainframe::activewnd->m_pCamWnd;
	
	const float tan_half_y = tan(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f);
	const float tan_half_x = (tan_half_y * 0.75f + tan_half_y * 0.75f) / (float)cam->camera.height;

	const float xa = tan_half_x * (float)(y - cam->camera.height / 2);
	const float xb = tan_half_x * (float)(x - cam->camera.width / 2);

	dir[0] = cam->camera.vup[0] * xa + cam->camera.vright[0] * xb + cam->camera.vpn[0];
	dir[1] = cam->camera.vup[1] * xa + cam->camera.vright[1] * xb + cam->camera.vpn[1];
	dir[2] = cam->camera.vup[2] * xa + cam->camera.vright[2] * xb + cam->camera.vpn[2];

	utils::vector::normalize(dir);
}

// CTRL + SHIFT + RMButton
void cam_rotate()
{
	auto cam = cmainframe::activewnd->m_pCamWnd;
	auto prefs = game::g_PrefsDlg();
	
	CPoint point;
	GetCursorPos(&point);

	if (point.x != cam->m_ptCursor.x || point.y != cam->m_ptCursor.y)
	{
		// use anglespeed
		cam->camera.angles[1] = cam->camera.angles[1] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
		cam->camera.angles[0] = cam->camera.angles[0] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.y - cam->m_ptCursor.y);

		// org
		//cam->camera.angles[1] = cam->camera.angles[1] - prefs->m_nMoveSpeed / 500.0 * (point.x - cam->m_ptCursor.x);
		//cam->camera.angles[0] = cam->camera.angles[0] - prefs->m_nMoveSpeed / 500.0 * (point.y - cam->m_ptCursor.y);

		SetCursorPos(cam->m_ptCursor.x, cam->m_ptCursor.y);
		cam->cursor_visible = false;

		ShowCursor(0);
	}
}

void cam_positiondrag()
{
	auto cam = cmainframe::activewnd->m_pCamWnd;
	auto prefs = game::g_PrefsDlg();

	CPoint point;
	GetCursorPos(&point);
	
	if (point.x != cam->m_ptCursor.x || point.y != cam->m_ptCursor.y)
	{
		// use anglespeed
		cam->camera.angles[1] = cam->camera.angles[1] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
		
		// org
		//cam->camera.angles[1] = cam->camera.angles[1] - prefs->m_nMoveSpeed / 500.0 * (point.x - cam->m_ptCursor.x);

		const float delta = static_cast<float>(prefs->m_nMoveSpeed) / -250.0f * static_cast<float>(point.y - cam->m_ptCursor.y);
		utils::vector::normalize(cam->camera.vpn);
		
		cam->camera.origin[0] = cam->camera.vpn[0] * delta + cam->camera.origin[0];
		cam->camera.origin[1] = cam->camera.vpn[1] * delta + cam->camera.origin[1];
		//cam->camera.origin[2] = cam->camera.origin[2];

		SetCursorPos(cam->m_ptCursor.x, cam->m_ptCursor.y);
		cam->cursor_visible = false;
		
		ShowCursor(0);
	}
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
	cam->cursor_visible = true; // prob. int16

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
	on_ccamwnd_msg __on_mbutton_up;
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
	__on_lbutton_down(pThis, nFlags, point); // not in use?
}

void __fastcall ccamwnd::on_lbutton_up([[maybe_unused]] ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	//__on_lbutton_up(pThis, nFlags, point);

	// use unsed CPoint in czwnd::on_lbutton_up to differentiate which function called it
	point.x = 251;

	czwnd::on_lbutton_up(cmainframe::activewnd->m_pZWnd, nullptr, nFlags, point); // redirect
}


// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_rbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_rbutton_down(pThis, nFlags, point); // not in use?
}

void __fastcall ccamwnd::on_rbutton_up([[maybe_unused]] ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	//__on_rbutton_up(pThis, nFlags, point);

	// use unsed CPoint in czwnd::on_rbutton_up to differentiate which function called it
	point.x = 253;

	czwnd::on_rbutton_up(cmainframe::activewnd->m_pZWnd, nullptr, nFlags, point); // redirect
}


// *
// | ----------------- Middle Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_mbutton_up([[maybe_unused]] ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// use unsed CPoint in czwnd::on_mbutton_up to differentiate which function called it
	point.x = 255;

	czwnd::on_mbutton_up(cmainframe::activewnd->m_pZWnd, nullptr, nFlags, point);
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
#ifdef DEBUG_KEYS
	game::printf_to_console("cam keydown: %s -> mainframe", ggui::hotkeys::cmdbinds_ascii_to_keystr(nChar).c_str());
#endif

	// original function :: CMainFrame::OnKeyDown
	__on_keydown_cam(nChar, nRepCnt, nFlags);
}

void __stdcall ccamwnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
#ifdef DEBUG_KEYS
	game::printf_to_console("cam keyup: %s -> mainframe", ggui::hotkeys::cmdbinds_ascii_to_keystr(nChar).c_str());
#endif

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
		if (ggui::is_ggui_initialized())
		{
			// set cz context (in-case we use multiple imgui context's)
			IMGUI_BEGIN_CZWND;

			// not sure why I did this
			if (!GET_GUI(ggui::camera_dialog)->rtt_is_hovered() && ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}

			// handle text input 
			if(ImGui::GetIO().WantTextInput)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
	}

#if 0
	if(Msg == WM_SIZE)
	{
		// disable postfx when resizing
		components::renderer::postfx::set_disable_duration(0.45f);
		components::renderer::postfx::disable();
	}
#endif
	
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

bool should_move_selection()
{
	if(dvars::guizmo_enable->current.enabled)
	{
		if(ImGuizmo::IsOver() || ImGuizmo::IsUsing())
		{
			return false;
		}
		
		// disable brush dragging within the camera window
		/*if (ggui::get_rtt_camerawnd()->window_hovered && game::g_qeglobals->d_select_mode == game::select_t::sel_brush)
		{
			return false;
		}*/
	}

    return true;
}

void __declspec(naked) move_selection_stub()
{
    const static uint32_t move_selection_func = 0x47F0C0;
    const static uint32_t retn_pt = 0x48023D;
    __asm
    {
		pushad; // new
        call    should_move_selection;
        test    al, al;
		popad;	// new
        
        je      SKIP_MOVE;
        call    move_selection_func;

    SKIP_MOVE:
        jmp		retn_pt;
    }
}

void __declspec(naked) drag_setup_stub()
{
	const static uint32_t cmp_addr = 0x47E45A;
	const static uint32_t retn_pt = 0x47E448;
	__asm
	{
		shr     eax, 0xF;	// og
		test    al, 1;		// og
		jz		JZ_ADDR;
		jmp		retn_pt;

	JZ_ADDR:
		pushad; // new
		call    should_move_selection;
		test    al, al;
		popad;	// new

		je      SKIP_ADDR;
		jmp		cmp_addr;

	SKIP_ADDR:
		jmp		retn_pt;
	}
}

// *
// *

void ccamwnd::register_dvars()
{
	dvars::guizmo_enable = dvars::register_bool(
		/* name		*/ "guizmo_enable",
		/* default	*/ true,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Enable guizmo");

	dvars::guizmo_snapping = dvars::register_bool(
		/* name		*/ "guizmo_snapping",
		/* default	*/ true,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Guizmo: Enable grid-snapping");

	//dvars::guizmo_brush_mode = dvars::register_bool(
	//	/* name		*/ "guizmo_brush_mode",
	//	/* default	*/ true,
	//	/* flags	*/ game::dvar_flags::saved,
	//	/* desc		*/ "Guizmo: Enable brush mode");

	dvars::gui_camera_toolbar_defaultopen = dvars::register_bool(
		/* name		*/ "gui_camera_toolbar_defaultopen",
		/* default	*/ false,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Open the camera toolbar by default");

	
	dvars::gui_toolbox_integrate_cam_toolbar = dvars::register_bool(
		/* name		*/ "gui_toolbox_integrate_cam_toolbar",
		/* default	*/ false,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "integrate camera window toolbar into toolbox");
}

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

	utils::hook(0x405346, cam_rotate, HOOK_CALL).install()->quick();
	utils::hook(0x405321, cam_positiondrag, HOOK_CALL).install()->quick();

	// disable entity dragging with mouse in camerawnd
	//utils::hook::set<BYTE>(0x40539D, 0xEB);
	utils::hook(0x480238, move_selection_stub, HOOK_JUMP).install()->quick();

	// disable selection box when using the guizmo to translate vertices -> fixes deselection of vertices after translation
	utils::hook::nop(0x47E441, 5);
	utils::hook(0x47E441, drag_setup_stub, HOOK_JUMP).install()->quick();

	// disable original context menu ~ handled in czwnd
	//utils::hook::nop(0x403340, 5);
	
	__on_lbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403160, ccamwnd::on_lbutton_down, HK_JUMP));
    __on_lbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4031D0, ccamwnd::on_lbutton_up, HK_JUMP));

    __on_rbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4032B0, ccamwnd::on_rbutton_down, HK_JUMP));
    __on_rbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403310, ccamwnd::on_rbutton_up, HK_JUMP));

	__on_mbutton_up		= reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403270, ccamwnd::on_mbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403100, ccamwnd::on_mouse_move, HK_JUMP));

	__on_keydown_cam	= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x402F60, ccamwnd::on_keydown, HK_JUMP));
	__on_keyup_cam		= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x408B70, ccamwnd::on_keyup, HK_JUMP));

	components::command::register_command_with_hotkey("center_camera_on_selection"s, [](auto)
	{
		const auto eent = game::g_edit_entity();

		if (eent && eent->eclass && eent->eclass->name)
		{
			if (utils::str_to_lower(eent->eclass->name) != "worldspawn")
			{
				utils::vector::copy(eent->origin, cmainframe::activewnd->m_pCamWnd->camera.origin);
			}
			else
			{
				const auto sel = game::g_selected_brushes();
				if(components::remote_net::selection_is_brush(sel->def))
				{
					// get center of the brush
					for (int j = 0; j < 3; j++)
					{
						cmainframe::activewnd->m_pCamWnd->camera.origin[j] = 
							sel->def->mins[j] + abs((sel->def->maxs[j] - sel->def->mins[j]) * 0.5f);
					}
				}
			}
		}
	});
}