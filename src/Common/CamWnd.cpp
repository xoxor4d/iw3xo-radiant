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


// *
// | ----------------- Left Mouse Button ---------------------
// *

void __fastcall CCamWnd::on_lbutton_down(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	ImGui::SetCurrentContext(Game::Globals::_context_camera);
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_LBUTTONDOWN);

#if !CCAMWND_REALTIME
    pThis->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_lbutton_down(pThis, nFlags, point);
    }
}

void __fastcall CCamWnd::on_lbutton_up(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	ImGui::SetCurrentContext(Game::Globals::_context_camera);
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_LBUTTONUP);

#if !CCAMWND_REALTIME
    pThis->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif

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
	ImGui::SetCurrentContext(Game::Globals::_context_camera);
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_RBUTTONDOWN);

#if !CCAMWND_REALTIME
    pThis->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_rbutton_down(pThis, nFlags, point);
    }
}

void __fastcall CCamWnd::on_rbutton_up(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	ImGui::SetCurrentContext(Game::Globals::_context_camera);
	
    Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_RBUTTONUP);

#if !CCAMWND_REALTIME
    pThis->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
#endif

    // do not pass the msg if mouse is inside an imgui window
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        return __on_rbutton_up(pThis, nFlags, point);
    }
}


// *
// | ----------------------------------------------------------
// *

void CCamWnd::on_endframe()
{
    Game::Globals::gui_present.CCamWnd = true;
    Game::R_EndFrame();
}

// *
// *

BOOL WINAPI CCamWnd::wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
#if !CCAMWND_REALTIME
	if(Msg == WM_SETCURSOR && Game::Globals::d3d9_device)
	{
        CCamWnd::ActiveWindow->RedrawWindow();
	}
#endif

	if(Game::Globals::_context_camera)
	{
		ImGui::SetCurrentContext(Game::Globals::_context_camera);
	}

    if (GGUI_READY)
    {
        // handle mouse cursor for open menus
        for (auto menu = 0; menu < GGUI_MENU_COUNT; menu++)
        {
            if (Game::Globals::gui.menus[menu].menustate)
            {
                if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
                {
                    if (ImGui::GetIO().WantCaptureMouse)
                    {
                        ShowCursor(0);
                        ImGui::GetIO().MouseDrawCursor = 1;
                        return true;
                    }
                }
            }
        }

        ShowCursor(1);
        ImGui::GetIO().MouseDrawCursor = 0;
    }
	
	// => og CamWndProc
    return Utils::Hook::Call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x402D90)(hWnd, Msg, wParam, lParam);
}

// *
// *

void CCamWnd::main()
{
	// hook CCamWnd message handler
    //Utils::Hook::Set(0x402E86 + 4, CCamWnd::wndproc);
	
	// endframe hook to set imgui present bool
    Utils::Hook(0x40305C, CCamWnd::on_endframe, HOOK_CALL).install()->quick();
	
	__on_lbutton_down   = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x403160, CCamWnd::on_lbutton_down, HK_JUMP));
    __on_lbutton_up     = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x4031D0, CCamWnd::on_lbutton_up, HK_JUMP));

    __on_rbutton_down   = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x4032B0, CCamWnd::on_rbutton_down, HK_JUMP));
    __on_rbutton_up     = reinterpret_cast<on_ccamwnd_msg>(Utils::Hook::Detour(0x403310, CCamWnd::on_rbutton_up, HK_JUMP));
}