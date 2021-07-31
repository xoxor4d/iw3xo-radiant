#include "STDInclude.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

CCamWnd* CCamWnd::ActiveWindow;

void CCamWnd::Cam_MouseControl(float dtime)
{
	static DWORD dwCall = 0x403950;

	_asm
	{
		push esi
		mov esi, ecx // esi = this
		push dtime
		call dwCall // Automatically fixes the stack
		pop esi
	}

	return;
}


// *
// | -------------------- MSG typedefs ------------------------
// *


typedef void(__thiscall* on_ccamwnd_msg)(CCamWnd*, UINT, CPoint);

	// mouse scrolling handled in CMainFrame
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

void __fastcall CCamWnd::on_lbutton_down(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_LBUTTONDOWN);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_lbutton_down(pThis, nFlags, point);
    }
}

void __fastcall CCamWnd::on_lbutton_up(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_LBUTTONUP);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_lbutton_up(pThis, nFlags, point);
    }
}

// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall CCamWnd::on_rbutton_down(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_RBUTTONDOWN);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_rbutton_down(pThis, nFlags, point);
    }
}

void __fastcall CCamWnd::on_rbutton_up(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_RBUTTONUP);

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_rbutton_up(pThis, nFlags, point);
    }
}

void __fastcall CCamWnd::on_mouse_move(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CCAMERAWND;

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		return __on_mouse_move(pThis, nFlags, point);
	}
}


void __stdcall CCamWnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CCAMERAWND;

	if (ImGui::GetIO().WantCaptureMouse)
	{
		Game::ImGui_HandleKeyIO(CMainFrame::ActiveWindow->m_pCamWnd->GetWindow(), WM_KEYDOWN, 0, nChar);
	}
	else
	{
		// CMainFrame::OnKeyDown
		return __on_keydown_cam(nChar, nRepCnt, nFlags);
	}
}

void __stdcall CCamWnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CCAMERAWND;

	if (ImGui::GetIO().WantCaptureMouse)
	{
		Game::ImGui_HandleKeyIO(CMainFrame::ActiveWindow->m_pCamWnd->GetWindow(), WM_KEYUP, 0, nChar);
	}
	else
	{
		// CMainFrame::OnKeyUp
		return __on_keyup_cam(nChar, nRepCnt, nFlags);
	}
}

// *
// | ----------------------------------------------------------
// *

//void CCamWnd::on_endframe()
//{
//    Game::R_EndFrame();
//}

// *
// *

BOOL WINAPI CCamWnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Components::Gui::all_contexts_ready())
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
    return Utils::Hook::Call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x402D90)(hWnd, Msg, wParam, lParam);
}

// *
// *

void CCamWnd::main()
{
	// hook CCamWnd message handler
    Utils::Hook::Set(0x402E86 + 4, CCamWnd::windowproc);
	
	// endframe hook
    //Utils::Hook(0x40305C, CCamWnd::on_endframe, HOOK_CALL).install()->quick();
	
	__on_lbutton_down   = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x403160, CCamWnd::on_lbutton_down, HK_JUMP));
    __on_lbutton_up     = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x4031D0, CCamWnd::on_lbutton_up, HK_JUMP));

    __on_rbutton_down   = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x4032B0, CCamWnd::on_rbutton_down, HK_JUMP));
    __on_rbutton_up     = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x403310, CCamWnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x403100, CCamWnd::on_mouse_move, HK_JUMP));

	__on_keydown_cam	= reinterpret_cast<on_ccamwnd_key>(Utils::Hook::Detour(0x402F60, CCamWnd::on_keydown, HK_JUMP));
	__on_keyup_cam		= reinterpret_cast<on_ccamwnd_key>(Utils::Hook::Detour(0x408B70, CCamWnd::on_keyup, HK_JUMP));
}