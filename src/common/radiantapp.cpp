#include "std_include.hpp"

typedef int(__thiscall* CWinApp_WriteProfileInt_t)(CWinApp* pthis, LPCSTR lpAppName, LPCSTR lpValueName, int Data);
						CWinApp_WriteProfileInt_t CWinApp_WriteProfileInt = reinterpret_cast<CWinApp_WriteProfileInt_t>(0x59853E);

typedef int(__thiscall* CWinApp_GetProfileIntA_t)(CWinApp* pthis, LPCSTR cbData, LPCSTR lpValueName, int nDefault);
						CWinApp_GetProfileIntA_t CWinApp_GetProfileIntA = reinterpret_cast<CWinApp_GetProfileIntA_t>(0x5984D5);


void registery_save([[maybe_unused]] CPrefsDlg* prefs)
{
	const auto state = afx::get_module_state();
	CWinApp_WriteProfileInt(state->m_pCurrentWinApp, "Prefs", "iw3x_mainframe_menubar_enabled", ggui::mainframe_menubar_enabled);
}

__declspec(naked) void registery_save_stub()
{
	const static uint32_t AfxGetModuleState_addr = 0x59390E;
	const static uint32_t retn_pt = 0x44F299;
	__asm
	{
		pushad;
		push	esi; // CPrefsDlg*
		call	registery_save;
		add		esp, 4;
		popad;

		call	AfxGetModuleState_addr;
		jmp		retn_pt;
	}
}


void registery_load()
{
	const auto state = afx::get_module_state();
	ggui::mainframe_menubar_enabled = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "iw3x_mainframe_menubar_enabled", 0);
}

__declspec(naked) void registery_load_stub()
{
	const static uint32_t AfxGetModuleState_addr = 0x59390E;
	const static uint32_t retn_pt = 0x44E391;
	__asm
	{
		pushad;
		call	registery_load;
		popad;

		call	AfxGetModuleState_addr;
		jmp		retn_pt;
	}
}


__declspec(naked) void menubar_stub_01()
{
	const static uint32_t retn_pt = 0x4507CF;
	__asm
	{
		cmp		ggui::mainframe_menubar_enabled, 1;
		jne		MENUBAR_DISABLED;

		push    214; // IDR_MENU_QUAKE3
		jmp		retn_pt;

	MENUBAR_DISABLED:
		push	0; // NULL menu
		jmp		retn_pt;
	}
}

__declspec(naked) void menubar_stub_02()
{
	const static uint32_t DestroyMenu_addr = 0x58A908;
	const static uint32_t retn_pt = 0x42104F;
	__asm
	{
		cmp		ggui::mainframe_menubar_enabled, 1;
		jne		MENUBAR_DISABLED;

		call	DestroyMenu_addr;
		jmp		retn_pt;

	MENUBAR_DISABLED:
		jmp		retn_pt;
	}
}

__declspec(naked) void menubar_stub_03()
{
	const static uint32_t retn_pt = 0x42105C;
	__asm
	{
		cmp		ggui::mainframe_menubar_enabled, 1;
		jne		MENUBAR_DISABLED;

		push    214; // IDR_MENU_QUAKE3
		jmp		retn_pt;

	MENUBAR_DISABLED:
		push	0; // NULL menu
		jmp		retn_pt;
	}
}

void radiantapp::main()
{
	// registery saving stub :: CPrefsDlg::SavePrefs
	utils::hook(0x44F294, registery_save_stub, HOOK_JUMP).install()->quick();

	// registery loading stub :: CPrefsDlg::LoadPrefs
	utils::hook(0x44E38C, registery_load_stub, HOOK_JUMP).install()->quick();

	// note:
	// we need to save the menubar state within the registery because dvars are not yet initialized when we need the menubar state
	
	// show/hide mainframe menubar on startup 01 :: CRadiantApp::InitInstance
	utils::hook(0x4507CA, menubar_stub_01, HOOK_JUMP).install()->quick();

	// show/hide mainframe menubar on startup 02 :: CMainFrame::OnCreate
	utils::hook(0x42104A, menubar_stub_02, HOOK_JUMP).install()->quick();

	// show/hide mainframe menubar on startup 03 :: CMainFrame::OnCreate
	utils::hook(0x421057, menubar_stub_03, HOOK_JUMP).install()->quick();

	
#ifdef HIDE_MAINFRAME_MENUBAR
	// -----------------------------------------------------------------------
	// disable mainframe menubar

	// create mainframe without menu
	utils::hook::set<BYTE>(0x4507CA + 1, 0x0);

	// cmainframe::OnCreate :: nop CMenu::DestroyMenu call (nullptr exception)
	utils::hook::nop(0x42104A, 5);

	// cmainframe::OnCreate :: make LoadMenuA load a null menu
	utils::hook::set<BYTE>(0x421057 + 1, 0x0);

	// -----------------------------------------------------------------------
#endif
	
}