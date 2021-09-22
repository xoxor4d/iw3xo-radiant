#include "std_include.hpp"

enum VIEWTYPE
{
	YZ = 0x0,
	XZ = 0x1,
	XY = 0x2,
};

void reposition_viewtype_hint(const char* text, game::Font_s* font, [[maybe_unused]] float* origin, float* pixel_step_x, float* pixel_step_y, float* color)
{
	float new_org[3];

	const float w = static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nWidth / 2) / cmainframe::activewnd->m_pXYWnd->m_fScale;
	const float h = static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nHeight / 2) / cmainframe::activewnd->m_pXYWnd->m_fScale;

	const int nDim1 = cmainframe::activewnd->m_pXYWnd->m_nViewType == YZ;
	const int nDim2 = (cmainframe::activewnd->m_pXYWnd->m_nViewType != XY) + 1;
	const int nDim3 = 3 - nDim2 - nDim1;

	float hc = 0.0f;
	if ((cmainframe::activewnd->m_pXYWnd->m_nWidth & 1) == 0) {
		hc = 0.5f / cmainframe::activewnd->m_pXYWnd->m_fScale;
	}

	float hd = 0.0f;
	if ((cmainframe::activewnd->m_pXYWnd->m_nHeight & 1) == 0) {
		hd = 0.5f / cmainframe::activewnd->m_pXYWnd->m_fScale;
	}

	new_org[nDim1] = (cmainframe::activewnd->m_pXYWnd->m_vOrigin[nDim1] - w + 28.0f / cmainframe::activewnd->m_pXYWnd->m_fScale) + hc;
	new_org[nDim2] = (cmainframe::activewnd->m_pXYWnd->m_vOrigin[nDim2] - h + 28.0f / cmainframe::activewnd->m_pXYWnd->m_fScale) + hd;
	new_org[nDim3] = 0.0f;

	// ----

	if (text && *text)
	{
		const size_t t_size = strlen(text);
		auto cmd = reinterpret_cast<game::GfxCmdDrawText3D*>(game::R_RenderBufferCmdCheck((t_size + 0x34) & 0xFFFFFFFC, 16));
		if (cmd)
		{
			cmd->org[0] = new_org[0];
			cmd->org[1] = new_org[1];
			cmd->org[2] = new_org[2];
			cmd->font = font;
			cmd->xPixelStep[0] = pixel_step_x[0];
			cmd->xPixelStep[1] = pixel_step_x[1];
			cmd->xPixelStep[2] = pixel_step_x[2];
			cmd->yPixelStep[0] = pixel_step_y[0];
			cmd->yPixelStep[1] = pixel_step_y[1];
			cmd->yPixelStep[2] = pixel_step_y[2];

			// R_ConvertColorToBytes
			utils::function<void(float*, game::GfxColor*)>(0x528EA0)(color, &cmd->color);

			memcpy(cmd->text, text, t_size);
			cmd->text[t_size] = 0;
		}
	}
}


void reposition_top_grid_hint(const char* text, game::Font_s* font, [[maybe_unused]] float* origin, float* pixel_step_x, float* pixel_step_y, float* color)
{
	const int nDim1 = cmainframe::activewnd->m_pXYWnd->m_nViewType == YZ;
	const int nDim2 = (cmainframe::activewnd->m_pXYWnd->m_nViewType != XY) + 1;
	const int nDim3 = 3 - nDim2 - nDim1;
	
	const float h = static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nHeight - 24) / cmainframe::activewnd->m_pXYWnd->m_fScale;

	float new_org[3];
	new_org[nDim1] = origin[nDim1];
	new_org[nDim2] = origin[nDim2] - h;
	new_org[nDim3] = origin[nDim3];

	// ----

	if (text && *text)
	{
		const size_t t_size = strlen(text);
		auto cmd = reinterpret_cast<game::GfxCmdDrawText3D*>(game::R_RenderBufferCmdCheck((t_size + 0x34) & 0xFFFFFFFC, 16));
		if (cmd)
		{
			cmd->org[0] = new_org[0];
			cmd->org[1] = new_org[1];
			cmd->org[2] = new_org[2];
			cmd->font = font;
			cmd->xPixelStep[0] = pixel_step_x[0];
			cmd->xPixelStep[1] = pixel_step_x[1];
			cmd->xPixelStep[2] = pixel_step_x[2];
			cmd->yPixelStep[0] = pixel_step_y[0];
			cmd->yPixelStep[1] = pixel_step_y[1];
			cmd->yPixelStep[2] = pixel_step_y[2];

			// R_ConvertColorToBytes
			utils::function<void(float*, game::GfxColor*)>(0x528EA0)(color, &cmd->color);

			memcpy(cmd->text, text, t_size);
			cmd->text[t_size] = 0;
		}
	}
}

// *
// | -------------------- gui i/o ------------------------
// *

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * (cxywnd member functions are only used if the xy subwindow is focused)
 * - Directly clicking onto an imgui window will not focus the subwindow behind it
 * - IO will instead be handled by cmainframe member functions
 * + Mouse scrolling handled by cmainframe::on_mscroll
 * + Char events handled by cxywnd::wndproc / ccamwnd::wndproc or cmainframe::windowproc (depends on focused window)
 */

namespace xywnd
{
	on_cxywnd_msg __on_lbutton_down;
	on_cxywnd_msg __on_lbutton_up;
	on_cxywnd_msg __on_rbutton_down;
	on_cxywnd_msg __on_rbutton_up;
	on_cxywnd_msg __on_mbutton_down;
	on_cxywnd_msg __on_mbutton_up;
	on_cxywnd_msg __on_mouse_move;

	on_cxywnd_key __on_keydown;
	on_cxywnd_key __on_keyup;

	on_cxywnd_scroll __on_scroll;
}


// *
// | ----------------- Left Mouse Button ---------------------
// *

void __fastcall cxywnd::on_lbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	if(ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_lbutton_down(pThis, nFlags, ggui::rtt_gridwnd.cursor_pos_pt);
			return;
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			typedef  void(__thiscall* CamWnd__DropModelsToPlane_t)(ccamwnd*, int x, int y, int buttons);
			const auto CamWnd__DropModelsToPlane = reinterpret_cast<CamWnd__DropModelsToPlane_t>(0x403D30);

			const auto ccam = cmainframe::activewnd->m_pCamWnd;

			// ------

			ccam->m_ptLastCursor = ggui::rtt_camerawnd.cursor_pos_pt;
			CamWnd__DropModelsToPlane(ccam, ccam->m_ptLastCursor.x, ccam->camera.height - ccam->m_ptLastCursor.y - 1, nFlags);

			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			// handle input, don't pass input to the xywindow
			ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONDOWN);
			return;
		}
	}

	// pass input to the xywindow
	xywnd::__on_lbutton_down(pThis, nFlags, point);
}

void __fastcall cxywnd::on_lbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_lbutton_up(pThis, nFlags, ggui::rtt_gridwnd.cursor_pos_pt);
			return;
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			ccamwnd::mouse_up(cmainframe::activewnd->m_pCamWnd, nFlags);
			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONUP);
			return;
		}
	}

	// pass input to the xywindow
	xywnd::__on_lbutton_up(pThis, nFlags, point);
}



// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall cxywnd::on_rbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_rbutton_down(pThis, nFlags, ggui::rtt_gridwnd.cursor_pos_pt);
			return;
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			typedef  void(__thiscall* CamWnd__DropModelsToPlane_t)(ccamwnd*, int x, int y, int buttons);
			const auto CamWnd__DropModelsToPlane = reinterpret_cast<CamWnd__DropModelsToPlane_t>(0x403D30);

			const auto ccam = cmainframe::activewnd->m_pCamWnd;
			CamWnd__DropModelsToPlane(ccam, ggui::rtt_camerawnd.cursor_pos_pt.x, ccam->camera.height - ggui::rtt_camerawnd.cursor_pos_pt.y - 1, nFlags);

			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONDOWN);
			return;
		}
	}

	// pass input to the xywindow
	xywnd::__on_rbutton_down(pThis, nFlags, point);
}

void __fastcall cxywnd::on_rbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_rbutton_up(pThis, nFlags, ggui::rtt_gridwnd.cursor_pos_pt);
			return;
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			const auto ccam = cmainframe::activewnd->m_pCamWnd;
			const int cursor_point_y = ccam->camera.height - ggui::rtt_camerawnd.cursor_pos_pt.y - 1;

			// context menu
			const static uint32_t CCamWnd__ContextMenu_Func = 0x404D40;
			__asm
			{
				pushad;

				push	cursor_point_y;
				push	ggui::rtt_camerawnd.cursor_pos_pt.x;
				mov     ebx, ccam;
				call	CCamWnd__ContextMenu_Func; // cleans the stack

				popad;
			}

			ccamwnd::mouse_up(cmainframe::activewnd->m_pCamWnd, nFlags);
			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONUP);
			return;
		}
	}
	
	// pass input to the xywindow
	xywnd::__on_rbutton_up(pThis, nFlags, point);
}



// *
// | ----------------- Middle Mouse Button ---------------------
// *

void __fastcall cxywnd::on_mbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_mbutton_down(pThis, nFlags, ggui::rtt_gridwnd.cursor_pos_pt);
			return;
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			typedef  void(__thiscall* CamWnd__DropModelsToPlane_t)(ccamwnd*, int x, int y, int buttons);
			const auto CamWnd__DropModelsToPlane = reinterpret_cast<CamWnd__DropModelsToPlane_t>(0x403D30);

			const auto ccam = cmainframe::activewnd->m_pCamWnd;
			CamWnd__DropModelsToPlane(ccam, ggui::rtt_camerawnd.cursor_pos_pt.x, ccam->camera.height - ggui::rtt_camerawnd.cursor_pos_pt.y - 1, nFlags);

			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::HandleKeyIO(pThis->GetWindow(), WM_MBUTTONDOWN);
			return;
		}
	}

	// pass input to the xywindow
	xywnd::__on_mbutton_down(pThis, nFlags, point);
}

void __fastcall cxywnd::on_mbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_mbutton_up(pThis, nFlags, ggui::rtt_gridwnd.cursor_pos_pt);
			return;
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			ccamwnd::mouse_up(cmainframe::activewnd->m_pCamWnd, nFlags);
			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::HandleKeyIO(pThis->GetWindow(), WM_MBUTTONUP);
			return;
		}
	}
	
	// pass input to the xywindow
	xywnd::__on_mbutton_up(pThis, nFlags, point);
}



// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall cxywnd::on_mouse_move(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_mouse_move(pThis, nFlags, ggui::rtt_gridwnd.cursor_pos_pt);
			return;
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			const auto ccam = cmainframe::activewnd->m_pCamWnd;
			const int cursor_point_y = ccam->camera.height - ggui::rtt_camerawnd.cursor_pos_pt.y - 1;

			const static uint32_t CCamWnd__Cam_MouseMoved_Func = 0x404FC0;
			__asm
			{
				pushad;

				mov		eax, nFlags;
				push	cursor_point_y;
				push	ggui::rtt_camerawnd.cursor_pos_pt.x;
				mov		ecx, ccam;
				call	CCamWnd__Cam_MouseMoved_Func; // cleans the stack

				popad;
			}

			return;
		}

		// block xywindow input if mouse cursor is above any cxy imgui window
		if(ImGui::GetIO().WantCaptureMouse)
		{
			return;
		}
	}

	// original function
	xywnd::__on_mouse_move(pThis, nFlags, point);
}



// *
// | ----------------- Keys Up/Down ---------------------
// *

void __stdcall cxywnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_keydown(nChar, nRepCnt, nFlags);
			return;
		}

		// handle imgui-camera window (only triggers when xywnd is focused)
		// cmainframe::on_keydown handles input if imgui-camera window is focused 
		if (ggui::rtt_camerawnd.window_hovered)
		{
			// calls the original on_keydown function
			mainframe::__on_keydown(cmainframe::activewnd, nChar, nRepCnt, nFlags);
			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::HandleKeyIO(cmainframe::activewnd->m_pXYWnd->GetWindow(), WM_KEYDOWN, 0, nChar);
			return;
		}
	}
	
	// original function :: CMainFrame::OnKeyDown (detoured)
	xywnd::__on_keydown(nChar, nRepCnt, nFlags);
}

void __stdcall cxywnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			xywnd::__on_keyup(nChar, nRepCnt, nFlags);
			return;
		}

		// handle imgui-camera window (only triggers when xywnd is focused)
		// cmainframe::on_keyup handles input if imgui-camera window is focused 
		if (ggui::rtt_camerawnd.window_hovered)
		{
			// calls the original on_keyup function
			mainframe::__on_keyup(cmainframe::activewnd, nChar);
			return;
		}

		// if mouse cursor above any cxy imgui window
		if (ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::HandleKeyIO(cmainframe::activewnd->m_pXYWnd->GetWindow(), WM_KEYUP, 0, nChar);
			return;
		}
	}
	
	// original function :: CMainFrame::OnKeyUp (detoured)
	xywnd::__on_keyup(nChar, nRepCnt, nFlags);
}



// *
// | ----------------- Mouse Scrolling ---------------------
// *

BOOL __stdcall cxywnd::on_scroll(UINT nFlags, std::int16_t zDelta, [[maybe_unused]] CPoint point)
{
	if (ggui::cxy_context_ready())
	{
		// set cxy context
		IMGUI_BEGIN_CXYWND;

		// if mouse is inside imgui-cxy window
		if (ggui::rtt_gridwnd.window_hovered)
		{
			return xywnd::__on_scroll(nFlags, zDelta, ggui::rtt_gridwnd.cursor_pos_pt);
		}

		// if mouse is inside imgui-camera window
		if (ggui::rtt_camerawnd.window_hovered)
		{
			float scroll_dir = zDelta <= 0 ? 1.0f : -1.0f;

			const static uint32_t CCamWnd__Scroll_Func = 0x4248A0;
			__asm
			{
				pushad;

				mov		edi, cmainframe::activewnd; // yes .. :r
				push	cmainframe::activewnd;		// ^
				fld		scroll_dir;
				fstp    dword ptr[esp];
				call	CCamWnd__Scroll_Func; // cleans the stack

				popad;
			}

			return 1;
		}
	}

	return xywnd::__on_scroll(nFlags, zDelta, point);
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
	// we only need the char event
	if (Msg == WM_CHAR)
	{
		if(ggui::cxy_context_ready())
		{
			// if mouse is inside imgui-cxy window
			if (ggui::rtt_gridwnd.window_hovered)
			{
				return DefWindowProcA(hWnd, Msg, wParam, lParam);
			}
			
			// handle char inputs when xy window is focused
			IMGUI_BEGIN_CXYWND;
			if (ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
		
		if(ggui::camera_context_ready())
		{
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

//void __declspec(naked) set_window_style()
//{
//	const static uint32_t retn_pt = 0x422827;
//	__asm
//	{
//		mov		ecx, 0; // WS_VISIBLE 
//		jmp		retn_pt;
//	}
//}

void __declspec(naked) set_child_window_style()
{
	const static uint32_t retn_pt = 0x463A6D;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD; // WS_VISIBLE //WS_TABSTOP | WS_GROUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZE | WS_DLGFRAME | WS_BORDER | WS_CLIPCHILDREN | WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;
		jmp		retn_pt;
	}
}

void __declspec(naked) set_detatched_child_window_style()
{
	const static uint32_t retn_pt = 0x463A47;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_POPUP; // WS_VISIBLE
		jmp		retn_pt;
	}
}

// *
// *

void cxywnd::main()
{
	// reposition xy-xz-yz text
	utils::hook(0x4690C5, reposition_viewtype_hint, HOOK_CALL).install()->quick();

	// reposition top grid -> bottom
	utils::hook(0x468E50, reposition_top_grid_hint, HOOK_CALL).install()->quick();

#if 1
	// cmainframe::oncreateclient -> change window styles on cxywnd->Create()
	//utils::hook::nop(0x422817, 16);
	//	 utils::hook(0x422817, set_window_style, HOOK_JUMP).install()->quick();
	
	// cxywnd::precreatewindow -> change window style for child windows (split view, not detatched)
	utils::hook::nop(0x463A64, 2);
	utils::hook::nop(0x463A66, 7);
		 utils::hook(0x463A66, set_child_window_style, HOOK_JUMP).install()->quick();

	// cxywnd::precreatewindow -> change window style for detatched windows (split view, detatched)
	utils::hook::nop(0x463A40, 7);
		 utils::hook(0x463A40, set_detatched_child_window_style, HOOK_JUMP).install()->quick();
#endif
	
	// custom windowproc
	utils::hook::nop(0x463A00, 9);
		 utils::hook(0x463A00, windowproc_stub, HOOK_JUMP).install()->quick();
	

	xywnd::__on_lbutton_down	= reinterpret_cast<xywnd::on_cxywnd_msg>(utils::hook::detour(0x463F70, cxywnd::on_lbutton_down, HK_JUMP));
	xywnd::__on_lbutton_up		= reinterpret_cast<xywnd::on_cxywnd_msg>(utils::hook::detour(0x464860, cxywnd::on_lbutton_up, HK_JUMP));

	xywnd::__on_rbutton_down	= reinterpret_cast<xywnd::on_cxywnd_msg>(utils::hook::detour(0x4647B0, cxywnd::on_rbutton_down, HK_JUMP));
	xywnd::__on_rbutton_up		= reinterpret_cast<xywnd::on_cxywnd_msg>(utils::hook::detour(0x464990, cxywnd::on_rbutton_up, HK_JUMP));

	xywnd::__on_mbutton_down	= reinterpret_cast<xywnd::on_cxywnd_msg>(utils::hook::detour(0x463FF0, cxywnd::on_mbutton_down, HK_JUMP));
	xywnd::__on_mbutton_up		= reinterpret_cast<xywnd::on_cxywnd_msg>(utils::hook::detour(0x464950, cxywnd::on_mbutton_up, HK_JUMP));
	
	xywnd::__on_mouse_move		= reinterpret_cast<xywnd::on_cxywnd_msg>(utils::hook::detour(0x464B10, cxywnd::on_mouse_move, HK_JUMP));

	xywnd::__on_keydown			= reinterpret_cast<xywnd::on_cxywnd_key>(utils::hook::detour(0x465C90, cxywnd::on_keydown, HK_JUMP));
	xywnd::__on_keyup			= reinterpret_cast<xywnd::on_cxywnd_key>(utils::hook::detour(0x46E510, cxywnd::on_keyup, HK_JUMP));

	xywnd::__on_scroll			= reinterpret_cast<xywnd::on_cxywnd_scroll>(utils::hook::detour(0x46E5C0, cxywnd::on_scroll, HK_JUMP));
}