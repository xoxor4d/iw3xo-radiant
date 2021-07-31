#include "std_include.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
	IMGUI_BEGIN_CCAMERAWND;
	
    ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONDOWN);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_lbutton_down(pThis, nFlags, point);
    }
}

void __fastcall ccamwnd::on_lbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;
	
    ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONUP);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_lbutton_up(pThis, nFlags, point);
    }
}

// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_rbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;
	
    ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONDOWN);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_rbutton_down(pThis, nFlags, point);
    }
}

void __fastcall ccamwnd::on_rbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;
	
    ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONUP);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_rbutton_up(pThis, nFlags, point);
    }
}

void __fastcall ccamwnd::on_mouse_move(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		return __on_mouse_move(pThis, nFlags, point);
	}
}


void __stdcall ccamwnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CCAMERAWND;

	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(cmainframe::activewnd->m_pCamWnd->GetWindow(), WM_KEYDOWN, 0, nChar);
	}
	else
	{
		// cmainframe::OnKeyDown
		return __on_keydown_cam(nChar, nRepCnt, nFlags);
	}
}

void __stdcall ccamwnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CCAMERAWND;

	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(cmainframe::activewnd->m_pCamWnd->GetWindow(), WM_KEYUP, 0, nChar);
	}
	else
	{
		// cmainframe::OnKeyUp
		return __on_keyup_cam(nChar, nRepCnt, nFlags);
	}
}

// *
// | ----------------------------------------------------------
// *

//void ccamwnd::on_endframe()
//{
//    game::R_EndFrame();
//}

// *
// *

BOOL WINAPI ccamwnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (components::gui::all_contexts_ready())
	{
		// fix mouse cursor for imgui windows
		//if (Msg == WM_NCHITTEST)
		{
			// save current context
			const auto imgui_context_old = ImGui::GetCurrentContext();

			IMGUI_BEGIN_CCAMERAWND;
			ImGuiIO& io = ImGui::GetIO();
			
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
		}

		
		// only process the char event, else we get odd multi context behaviour
		if (Msg == WM_CHAR)
		{
			// handle char inputs if camera window is focused
			IMGUI_BEGIN_CCAMERAWND;
			if (ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}

			// handle char inputs if camera window is focused but cursor is within the xy window, over an imgui menu
			IMGUI_BEGIN_CXYWND;
			if (ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
	}

	// => og CamWndProc
    return utils::hook::call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x402D90)(hWnd, Msg, wParam, lParam);
}

// *
// *

void ccamwnd::main()
{
	// hook ccamwnd message handler
    utils::hook::set(0x402E86 + 4, ccamwnd::windowproc);
	
	// endframe hook
    //utils::hook(0x40305C, ccamwnd::on_endframe, HOOK_CALL).install()->quick();
	
	__on_lbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403160, ccamwnd::on_lbutton_down, HK_JUMP));
    __on_lbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4031D0, ccamwnd::on_lbutton_up, HK_JUMP));

    __on_rbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4032B0, ccamwnd::on_rbutton_down, HK_JUMP));
    __on_rbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403310, ccamwnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403100, ccamwnd::on_mouse_move, HK_JUMP));

	__on_keydown_cam	= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x402F60, ccamwnd::on_keydown, HK_JUMP));
	__on_keyup_cam		= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x408B70, ccamwnd::on_keyup, HK_JUMP));
}