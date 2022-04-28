#include "std_include.hpp"

texwnd_s* g_texwnd = reinterpret_cast<texwnd_s*>(0x25D7990);

void ctexwnd::on_mousebutton_down(UINT nFlags)
{
	auto point = GET_GUI(ggui::texture_dialog)->rtt_get_cursor_pos_cpoint();
	const static uint32_t CTexWnd_OnButtonDown_func = 0x45C9A0;
	__asm
	{
		pushad;
		push	point.y;
		push	point.x;
		mov		eax, nFlags;
		call	CTexWnd_OnButtonDown_func;
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
	const static uint32_t CTexWnd__OnMouseFirst_func = 0x45CA60;
	__asm
	{
		pushad;
		mov		eax, nFlags;
		call	CTexWnd__OnMouseFirst_func;
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

	if(GET_GUI(ggui::texture_dialog)->is_active())
	{
		game::R_BeginFrame();
		game::R_Clear(7, game::g_qeglobals->d_savedinfo.colors[0], 1.0f, 0);

		// SetProjection 2D
		cdeclcall(game::GfxCmdHeader*, 0x4FD390);

		// R_DrawTexWnd
		cdeclcall(void, 0x45D0F0);

		game::R_EndFrame();
		game::R_IssueRenderCommands(-1);
		game::R_SortMaterials();
	}

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

bool texwnd_textfilter(const char* iter_material_name)
{
	const auto tex = GET_GUI(ggui::texture_dialog);
	const auto& filter = tex->get_filter();

	if (tex->get_filter_length())
	{
		if (std::string(iter_material_name).find(filter.InputBuf) != std::string::npos)
		{
			return true;
		}
	}

	return false;
}

// asm helper function
int texwnd_get_filter_length()
{
	return GET_GUI(ggui::texture_dialog)->get_filter_length();
}

void __declspec(naked) texwnd_listmaterials_intercept()
{
	const static uint32_t no_filter_pt = 0x45BD89;
	const static uint32_t break_pt = 0x45BD11;
	const static uint32_t goto_pt = 0x45BD8F;
	__asm
	{
		// test if textbox filter is enabled (textlen > 0)
		call	texwnd_get_filter_length;
		test	eax, eax;
		jz		NO_FILTER;

		// filter active
		mov     eax, [ebp - 28h]; // iter_material_name
		push	eax;
		call	texwnd_textfilter;
		add		esp, 4;

		test	al, al;
		jnz		BREAK_F;
		jmp		goto_pt;

	BREAK_F:
		jmp		break_pt;

	NO_FILTER:
		jmp		no_filter_pt;
	}
}

// *
// detours

// CMainFrame::OnViewTexture
void on_viewtextures_command()
{
	const auto tex = GET_GUI(ggui::texture_dialog);

	if(tex->is_inactive_tab() && tex->is_active())
	{
		tex->set_bring_to_front(true);
		return;
	}

	tex->toggle();
}

// CMainFrame::OnTexturesShowinuse
void on_textures_show_in_use_command()
{
	const auto tex = GET_GUI(ggui::texture_dialog);
	tex->set_bring_to_front(true);
	tex->open();

	// Texture_ShowInuse
	cdeclcall(void, 0x45B850);
}


// CMainFrame::OnTexturesShowall (intercept: no logic besides showing the menu)
void on_textures_show_all_command_intercept()
{
	const auto tex = GET_GUI(ggui::texture_dialog);
	tex->set_bring_to_front(true);
	tex->open();
}

void __declspec(naked) on_textures_show_all_command_stub()
{
	const static uint32_t sub_453E50 = 0x453E50;
	const static uint32_t retn_pt = 0x42B445;
	__asm
	{
		pushad;
		call	on_textures_show_all_command_intercept;
		popad;
		
		call    sub_453E50; // overwritten
		jmp     retn_pt; // jump back to "test al, al"
	}
}

void ctexwnd::register_dvars()
{
	dvars::gui_texwnd_draw_scrollbar = dvars::register_bool(
		/* name		*/ "gui_texwnd_draw_scrollbar",
		/* default	*/ true,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Draw texturewindow scrollbar");

	dvars::gui_texwnd_draw_scrollpercent = dvars::register_bool(
		/* name		*/ "gui_texwnd_draw_scrollpercent",
		/* default	*/ false,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Draw texturewindow scroll in percent (position in % / 100%)");
}

void ctexwnd::hooks()
{
	// do not set parent window for texture window
	utils::hook::nop(0x4228C1, 8);

	// ignore crashing texture searchbar destructor :x
	utils::hook::nop(0x627990, 5);
	utils::hook::set<BYTE>(0x627990, 0xC3);

	utils::hook::nop(0x45BCDD, 8);
	utils::hook(0x45BCDD, texwnd_listmaterials_intercept, HOOK_JUMP).install()->quick();

	// disable texture tab insertion in entitywnd :: CTabCtrl::InsertItem(&g_wndTabsEntWnd, 1u, 1u, "&Textures", 0, 0);
	utils::hook::nop(0x49672A, 23);

	// TODO! :: why does the default OnPaint function induces lag on all windows (even outside radiant) when calling it 250 times a second?
	// -- rewritten one runs fine (EndPaint?)
	utils::hook::detour(0x45DB20, ctexwnd::on_paint, HK_JUMP);

	// detour the view textures hotkey (CMainFrame::OnViewTexture) to open the imgui texture window
	utils::hook::detour(0x424440, on_viewtextures_command, HK_JUMP);

	// detour the show textures in use hotkey (CMainFrame::OnTexturesShowinuse) to open the imgui texture window
	utils::hook::detour(0x424B20, on_textures_show_in_use_command, HK_JUMP);

	// detour fails here .. so doing it manually -> make the show all textures hotkey (CMainFrame::OnTexturesShowall) open the imgui texture window
	utils::hook(0x42B440, on_textures_show_all_command_stub, HOOK_JUMP).install()->quick();
	
}