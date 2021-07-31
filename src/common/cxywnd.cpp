#include "std_include.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * (cxywnd member functions are only used if the xy subwindow is focused)
 * - Directly clicking onto an imgui window will not focus the subwindow behind it
 * - IO will instead be handled by cmainframe member functions
 * + Mouse scrolling handled by cmainframe::on_mscroll
 * + Char events handled by cxywnd::wndproc / ccamwnd::wndproc or cmainframe::windowproc (depends on focused window)
 */

typedef void(__thiscall* on_cxywnd_msg)(cxywnd*, UINT, CPoint);
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

void __fastcall cxywnd::on_lbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// set cxy context
	IMGUI_BEGIN_CXYWND;

	// check if mouse cursor is above any cxy imgui window
	if (ImGui::GetIO().WantCaptureMouse)
	{
		// handle input, don't pass input to the xywindow
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONDOWN);
	}
	else // pass input to the xywindow
    {
        return __on_lbutton_down(pThis, nFlags, point);
    }
}

void __fastcall cxywnd::on_lbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONUP);
	}
	else
    {
        return __on_lbutton_up(pThis, nFlags, point);
    }
}


// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall cxywnd::on_rbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONDOWN);
	}
	else
    {
        return __on_rbutton_down(pThis, nFlags, point);
    }
}

void __fastcall cxywnd::on_rbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONUP);
	}
	else
    {
        return __on_rbutton_up(pThis, nFlags, point);
    }
}



// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall cxywnd::on_mouse_move(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
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

void __stdcall cxywnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(cmainframe::activewnd->m_pXYWnd->GetWindow(), WM_KEYDOWN, 0, nChar);
	}
	else
	{
		// cmainframe::OnKeyDown
		return __on_keydown(nChar, nRepCnt, nFlags);
	}
}

void __stdcall cxywnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(cmainframe::activewnd->m_pXYWnd->GetWindow(), WM_KEYUP, 0, nChar);
	}
	else
	{
		// cmainframe::OnKeyUp
		return __on_keyup(nChar, nRepCnt, nFlags);
	}
}


//void cxywnd::on_endframe()
//{
//	game::R_EndFrame();
//}


// *
// | ----------------- Windowproc ---------------------
// *

LRESULT WINAPI cxywnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (components::gui::all_contexts_ready())
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

void __declspec(naked) windowproc_stub()
{
	const static uint32_t AfxRegisterClass_Func = 0x58A0A1;
	const static uint32_t retn_pt = 0x463A09;
	__asm
	{
		mov     dword ptr[esp + 10h], offset cxywnd::windowproc;
		call	AfxRegisterClass_Func;

		jmp		retn_pt;
	}
}


// *
// | ----------------- Main ---------------------
// *

void cxywnd::main()
{
	// custom windowproc
	utils::hook::nop(0x463A00, 9);
		 utils::hook(0x463A00, windowproc_stub, HOOK_JUMP).install()->quick();

	// endframe hook
	//utils::hook(0x465C0E, cxywnd::on_endframe, HOOK_CALL).install()->quick();

	__on_lbutton_down	= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x463F70, cxywnd::on_lbutton_down, HK_JUMP));
	__on_lbutton_up		= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x464860, cxywnd::on_lbutton_up, HK_JUMP));

	__on_rbutton_down	= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x4647B0, cxywnd::on_rbutton_down, HK_JUMP));
	__on_rbutton_up		= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x464990, cxywnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x464B10, cxywnd::on_mouse_move, HK_JUMP));

	__on_keydown		= reinterpret_cast<on_cxywnd_key>(utils::hook::detour(0x465C90, cxywnd::on_keydown, HK_JUMP));
	__on_keyup			= reinterpret_cast<on_cxywnd_key>(utils::hook::detour(0x46E510, cxywnd::on_keyup, HK_JUMP));
}