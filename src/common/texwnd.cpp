#include "std_include.hpp"

texwnd_s* g_texwnd = reinterpret_cast<texwnd_s*>(0x25D7990);

void ctexwnd::on_mousebutton_down(UINT nFlags)
{
	const static uint32_t CTexWnd_OnButtonDown_Func = 0x45C9A0;
	__asm
	{
		pushad;
		push	ggui::rtt_texwnd.cursor_pos_pt.y;
		push	ggui::rtt_texwnd.cursor_pos_pt.x;
		mov		eax, nFlags;
		call	CTexWnd_OnButtonDown_Func;
		add     esp, 8;
		popad;
	}
}

void ctexwnd::on_mousebutton_up(UINT nFlags)
{
	if ((nFlags & (MK_MBUTTON | MK_RBUTTON | MK_LBUTTON)) == 0)
	{
		// CTexWnd::OnButtonUp
		cdeclcall(void, 0x45CA30);

		ReleaseCapture();
	}
}

void ctexwnd::on_mousemove(UINT nFlags)
{
	const static uint32_t CTexWnd__OnMouseFirst_Func = 0x45CA60;
	__asm
	{
		pushad;
		mov		eax, nFlags;
		call	CTexWnd__OnMouseFirst_Func;
		popad;
	}
}

BOOL __fastcall ctexwnd::on_paint(ctexwnd* pThis)
{
	PAINTSTRUCT Paint;
	BeginPaint(pThis->GetWindow(), &Paint);

	// R_CheckHwnd_or_Device
	if (!utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(pThis->GetWindow()))
	{
		return EndPaint(pThis->GetWindow(), &Paint);
	}

	// R_BeginFrame
	cdeclcall(void, 0x4FCB10);

	// R_Clear
	utils::hook::call<void(__cdecl)(int, const float*, float, char)>(0x4FCC70)(7, game::g_qeglobals->d_savedinfo.colors[0], 1.0f, 0);

	// SetProjectionType ??
	cdeclcall(game::GfxCmdHeader*, 0x4FD390);

	// R_DrawTexWnd
	cdeclcall(void, 0x45D0F0);
	
	// R_EndFrame
	cdeclcall(void, 0x4FCBC0);

	// R_IssueRenderCommands
	utils::hook::call<void(__cdecl)(int)>(0x4FD630)(-1);

	// still no clue what this is
	cdeclcall(void, 0x4FD910);

	// R_CheckTargetWindow
	utils::hook::call<void(__cdecl)(HWND)>(0x500660)(pThis->GetWindow());

	int& random_dword01 = *reinterpret_cast<int*>(0x25D5B88);
	int& random_dword02 = *reinterpret_cast<int*>(0x242293C);
	int& random_dword03 = *reinterpret_cast<int*>(0x2422940);

	if (!random_dword01)
	{
		__debugbreak();
	}

	random_dword03 = random_dword02;

	// nice meme IW
	return EndPaint(pThis->GetWindow(), &Paint);
}

void ctexwnd::main()
{
	// do not set parent window for texture window
	utils::hook::nop(0x4228C1, 8);

	// disable texture tab insertion in entitywnd :: CTabCtrl::InsertItem(&g_wndTabsEntWnd, 1u, 1u, "&Textures", 0, 0);
	utils::hook::nop(0x49672A, 23);

	// TODO! :: why does the default OnPaint function induces lag on all windows (even outside radiant) calling it at 250fps
	// -- rewritten one runs fine
	utils::hook::detour(0x45DB20, ctexwnd::on_paint, HK_JUMP);
}