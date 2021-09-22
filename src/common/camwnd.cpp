#include "std_include.hpp"

ccamwnd* ccamwnd::activewnd;

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
	/* // maybe useful later
	// fix mouse cursor for imgui windows
	// disable win32 cursor if we hover imgui windows
	if (ImGui::GetIO().WantCaptureMouse) 
	{
		// get info for current cursor
		CURSORINFO ci = { sizeof(CURSORINFO) };
		if (GetCursorInfo(&ci))
		{
			auto size_ns = LoadCursor(0, IDC_SIZENS);
			auto size_we = LoadCursor(0, IDC_SIZEWE);

			// do not hide the size arrow when moving splitters across imgui windows
			if(ci.hCursor != size_ns && ci.hCursor != size_we)
			{
				io.MouseDrawCursor = true;
				SetCursor(nullptr);
			}
		}
	}
	else
	{
		io.MouseDrawCursor = false;
	}

	// restore context
	ImGui::SetCurrentContext(imgui_context_old);
	*/
	
	// only process the char event, else we get odd multi context behaviour
	if (Msg == WM_CHAR)
	{
		// temp
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

void ccamwnd::main()
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