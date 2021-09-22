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
	// pass input to the xywindow
	xywnd::__on_lbutton_down(pThis, nFlags, point);
}

void __fastcall cxywnd::on_lbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// pass input to the xywindow
	xywnd::__on_lbutton_up(pThis, nFlags, point);
}



// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall cxywnd::on_rbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// pass input to the xywindow
	xywnd::__on_rbutton_down(pThis, nFlags, point);
}

void __fastcall cxywnd::on_rbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// pass input to the xywindow
	xywnd::__on_rbutton_up(pThis, nFlags, point);
}



// *
// | ----------------- Middle Mouse Button ---------------------
// *

void __fastcall cxywnd::on_mbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// pass input to the xywindow
	xywnd::__on_mbutton_down(pThis, nFlags, point);
}

void __fastcall cxywnd::on_mbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// pass input to the xywindow
	xywnd::__on_mbutton_up(pThis, nFlags, point);
}



// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall cxywnd::on_mouse_move(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	xywnd::__on_mouse_move(pThis, nFlags, point);
}



// *
// | ----------------- Keys Up/Down ---------------------
// *

void __stdcall cxywnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyDown (detoured)
	xywnd::__on_keydown(nChar, nRepCnt, nFlags);
}

void __stdcall cxywnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyUp (detoured)
	xywnd::__on_keyup(nChar, nRepCnt, nFlags);
}



// *
// | ----------------- Mouse Scrolling ---------------------
// *

BOOL __stdcall cxywnd::on_scroll(UINT nFlags, std::int16_t zDelta, [[maybe_unused]] CPoint point)
{
	return xywnd::__on_scroll(nFlags, zDelta, point);
}


typedef void(__thiscall* on_cxywnd_zoom)(cmainframe*);
	on_cxywnd_zoom __on_zoomin;
	on_cxywnd_zoom __on_zoomout;


void __fastcall cxywnd::on_view_zoomin(cmainframe* pThis)
{
	if (pThis->m_pXYWnd)
	{
		if (pThis->m_pXYWnd->m_bActive)
		{
			pThis->m_pXYWnd->m_fScale = pThis->m_pXYWnd->m_fScale * 1.25f;

			if (pThis->m_pXYWnd->m_fScale > 160.0f) {
				pThis->m_pXYWnd->m_fScale = 160.0f;
			}
			
			// huh
			//flt_25D5A90 = this->m_pXYWnd->m_fScale;
		}
	}
	game::g_nUpdateBits |= W_XY_OVERLAY | W_XY;
	game::g_nUpdateBits |= W_Z_OVERLAY | W_Z;
	
	zwnd->scale = 1.25f * zwnd->scale;
	if (zwnd->scale > 160.0f) {
		zwnd->scale = 160.0f;
	}
	
	//__on_zoomin(pThis);
}

void __fastcall cxywnd::on_view_zoomout(cmainframe* pThis)
{
	__on_zoomout(pThis);
}

// *
// | ----------------- Windowproc ---------------------
// *

LRESULT WINAPI cxywnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// we only need the char event
	if (Msg == WM_CHAR)
	{
		// temp
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

	__on_zoomin					= reinterpret_cast<on_cxywnd_zoom>(utils::hook::detour(0x424750, cxywnd::on_view_zoomin, HK_JUMP));
	__on_zoomout				= reinterpret_cast<on_cxywnd_zoom>(utils::hook::detour(0x4247E0, cxywnd::on_view_zoomout, HK_JUMP));
}