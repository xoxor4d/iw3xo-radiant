#include "STDInclude.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * (CXYWnd member functions are only used if the xy subwindow is focused)
 * - Directly clicking onto an imgui window will not focus the subwindow behind it
 * - IO will instead be handled by CMainFrame member functions
 * + Mouse scrolling handled by CMainFrame::on_mscroll
 * + Char events handled by CXYWnd::wndproc / CCamWnd::wndproc or CMainFrame::windowproc (depends on focused window)
 */

typedef void(__thiscall* on_cxywnd_msg)(CXYWnd*, UINT, CPoint);
	on_cxywnd_msg __on_lbutton_down;
	on_cxywnd_msg __on_lbutton_up;
	on_cxywnd_msg __on_rbutton_down;
	on_cxywnd_msg __on_rbutton_up;
	on_cxywnd_msg __on_mouse_move;


typedef void(__stdcall* on_cxywnd_key)(UINT nChar, UINT nRepCnt, UINT nFlags);
	on_cxywnd_key __on_keydown;
	on_cxywnd_key __on_keyup;


// *
// | ----------------- Left Mouse Button ---------------------
// *

void __fastcall CXYWnd::on_lbutton_down(CXYWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// set cxy context
	IMGUI_BEGIN_CXYWND;

	// check if mouse cursor is above any cxy imgui window
	if (ImGui::GetIO().WantCaptureMouse)
	{
		// handle input, don't pass input to the xywindow
		Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_LBUTTONDOWN);
	}
	else // pass input to the xywindow
    {
        return __on_lbutton_down(pThis, nFlags, point);
    }
}

void __fastcall CXYWnd::on_lbutton_up(CXYWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_LBUTTONUP);
	}
	else
    {
        return __on_lbutton_up(pThis, nFlags, point);
    }
}


// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall CXYWnd::on_rbutton_down(CXYWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_RBUTTONDOWN);
	}
	else
    {
        return __on_rbutton_down(pThis, nFlags, point);
    }
}

void __fastcall CXYWnd::on_rbutton_up(CXYWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		Game::ImGui_HandleKeyIO(pThis->GetWindow(), WM_RBUTTONUP);
	}
	else
    {
        return __on_rbutton_up(pThis, nFlags, point);
    }
}



// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall CXYWnd::on_mouse_move(CXYWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;

	// block xywindow input if mouse cursor is above any cxy imgui window
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		return __on_mouse_move(pThis, nFlags, point);
	}
}



// *
// | ----------------- Keys Up/Down ---------------------
// *

void __stdcall CXYWnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		Game::ImGui_HandleKeyIO(CMainFrame::ActiveWindow->m_pXYWnd->GetWindow(), WM_KEYDOWN, 0, nChar);
	}
	else
	{
		// CMainFrame::OnKeyDown
		return __on_keydown(nChar, nRepCnt, nFlags);
	}
}

void __stdcall CXYWnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		Game::ImGui_HandleKeyIO(CMainFrame::ActiveWindow->m_pXYWnd->GetWindow(), WM_KEYUP, 0, nChar);
	}
	else
	{
		// CMainFrame::OnKeyUp
		return __on_keyup(nChar, nRepCnt, nFlags);
	}
}


//void CXYWnd::on_endframe()
//{
//	Game::R_EndFrame();
//}


// *
// | ----------------- Windowproc ---------------------
// *

LRESULT WINAPI CXYWnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Components::Gui::all_contexts_ready())
	{
		// we only need the char event
		if (Msg == WM_CHAR)
		{
			// handle char inputs when xy window is focused
			IMGUI_BEGIN_CXYWND;
			if (ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}

			// handle char inputs if xy window is focused but cursor is within the camera window, over an imgui menu
			IMGUI_BEGIN_CCAMERAWND;
			if (ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
	}
	
	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

void __declspec(naked) wndproc_stub()
{
	const static uint32_t AfxRegisterClass_Func = 0x58A0A1;
	const static uint32_t retn_pt = 0x463A09;
	__asm
	{
		mov     dword ptr[esp + 10h], offset CXYWnd::windowproc;
		call	AfxRegisterClass_Func;

		jmp		retn_pt;
	}
}


// *
// | ----------------- Main ---------------------
// *

void CXYWnd::main()
{
	// custom windowproc
	Utils::Hook::Nop(0x463A00, 9); Utils::Hook(0x463A00, wndproc_stub, HOOK_JUMP).install()->quick();

	// endframe hook
	//Utils::Hook(0x465C0E, CXYWnd::on_endframe, HOOK_CALL).install()->quick();

	__on_lbutton_down	= reinterpret_cast<on_cxywnd_msg>(Utils::Hook::Detour(0x463F70, CXYWnd::on_lbutton_down, HK_JUMP));
	__on_lbutton_up		= reinterpret_cast<on_cxywnd_msg>(Utils::Hook::Detour(0x464860, CXYWnd::on_lbutton_up, HK_JUMP));

	__on_rbutton_down	= reinterpret_cast<on_cxywnd_msg>(Utils::Hook::Detour(0x4647B0, CXYWnd::on_rbutton_down, HK_JUMP));
	__on_rbutton_up		= reinterpret_cast<on_cxywnd_msg>(Utils::Hook::Detour(0x464990, CXYWnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_cxywnd_msg>(Utils::Hook::Detour(0x464B10, CXYWnd::on_mouse_move, HK_JUMP));

	__on_keydown		= reinterpret_cast<on_cxywnd_key>(Utils::Hook::Detour(0x465C90, CXYWnd::on_keydown, HK_JUMP));
	__on_keyup			= reinterpret_cast<on_cxywnd_key>(Utils::Hook::Detour(0x46E510, CXYWnd::on_keyup, HK_JUMP));
}