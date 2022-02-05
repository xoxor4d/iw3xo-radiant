#include "std_include.hpp"

typedef int(__thiscall* CWinApp_WriteProfileInt_t)(CWinApp* pthis, LPCSTR lpAppName, LPCSTR lpValueName, int Data);
						CWinApp_WriteProfileInt_t CWinApp_WriteProfileInt = reinterpret_cast<CWinApp_WriteProfileInt_t>(0x59853E);

typedef int(__thiscall* CWinApp_GetProfileIntA_t)(CWinApp* pthis, LPCSTR cbData, LPCSTR lpValueName, int nDefault);
						CWinApp_GetProfileIntA_t CWinApp_GetProfileIntA = reinterpret_cast<CWinApp_GetProfileIntA_t>(0x5984D5);

void force_preferences_on_init()
{
	const auto prefs = game::g_PrefsDlg();

	// force split view
	prefs->m_nView = 1;

	// disable detatched windows
	prefs->detatch_windows = false;

	// this can really kill performance without the user knowing why
	prefs->preview_sun_aswell = false;
}

__declspec(naked) void force_preferences_on_init_stub()
{
	const static uint32_t retn_pt = 0x450735;
	__asm
	{
		push    ecx; // og
		lea     ecx, [esp + 1Ch]; // og

		pushad;
		call	force_preferences_on_init;
		popad;

		jmp retn_pt;
	}
}

void registery_save([[maybe_unused]] CPrefsDlg* prefs)
{
	const auto state = afx::get_module_state();
	CWinApp_WriteProfileInt(state->m_pCurrentWinApp, "Prefs", "iw3x_mainframe_menubar_enabled", ggui::mainframe_menubar_enabled);
	CWinApp_WriteProfileInt(state->m_pCurrentWinApp, "Prefs", "lights_max_intensity", game::g_qeglobals->preview_at_max_intensity);
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

	// put the undo levels variable to use (g_undoMaxSize always defaults to 64 otherwise)
	game::g_undoMaxSize = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "UndoLevels", 0);

	// always bothered me ..
	game::g_qeglobals->preview_at_max_intensity = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "lights_max_intensity", 0);
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

BOOL LoadRegistryInfo(const char* pszName, void* pvBuf, long* plSize)
{
	HKEY  hKey;
	long lType, lSize;

	if (plSize == nullptr)
	{
		plSize = &lSize;
	}

	if (game::g_qeglobals->use_ini)
	{
		RegOpenKeyExA(HKEY_CURRENT_USER, game::g_qeglobals->use_ini_registry, 0, KEY_READ, &hKey);
	}
	else
	{
		//RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\iw\\CoD4Radiant\\CoD4Radiant", 0, KEY_READ, &hKey);
		RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\iw\\IW3xRadiant\\IW3xRadiant", 0, KEY_READ, &hKey);
	}

	RegQueryValueExA(hKey, pszName, NULL, (unsigned long*)&lType, (unsigned char*)pvBuf, (unsigned long*)plSize);
	RegCloseKey(hKey);
	
	return TRUE;
}

enum XY_SHOW_FLAGS
{
	ANGLES = 0x2,
	CONNECTIONS = 0x4,
	NAMES = 0x8,
	BLOCKS = 0x10,
	COORDINATES = 0x20,
	REVERSE_FILTER = 0x40,
};

void radiantapp::set_default_savedinfo_colors()
{
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_TEXTUREBACK], 0.169f, 0.169f, 0.169f, 1.0f); // 0
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBACK], 0.25f, 0.25f, 0.25f, 1.0f); // 1
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMINOR], 0.233f, 0.233f, 0.233f, 1.0f); // 2
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMAJOR], 0.209f, 0.209f, 0.209f, 1.0f); // 3
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_CAMERABACK], 0.169f, 0.169f, 0.169f, 1.0f); // 4

	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITY], 0.0f, 0.5f, 0.0f, 1.0f); // 5 - not used org
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITYUNK], 1.0f, 0.1f, 0.0f, 0.4f); // 6 - not used org - now grid entity classname
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBLOCK], 0.165f, 0.165f, 0.165f, 1.0f); // 7
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDTEXT], 0.0f, 0.0f, 0.0f, 1.0f); // 8
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_BRUSHES], 0.54f, 0.54f, 0.54f, 1.0f); // 9

	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELBRUSHES], 1.0f, 0.125f, 0.0f, 1.0f); // 10
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELBRUSHES_CAMERA], 1.0f, 0.0f, 0.0f, 0.35f); // 11
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_CLIPPER], 1.0f, 0.0f, 0.0f, 1.0f); // 12
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_VIEWNAME], 1.0f, 0.35f, 0.0f, 0.6f); // 13
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_DETAIL_BRUSH], 0.0f, 0.525f, 0.18f, 1.0f); // 14
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_DRAW_TOGGLESUFS], 0.0f, 0.0f, 0.0f, 1.0f); // 15 - not used org

	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELFACE_CAMERA], 1.0f, 0.25f, 0.25f, 0.25f); // 16
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_GROUP], 0.0f, 0.5f, 0.65f, 1.0f); // 17
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_CULLGROUP], 0.0f, 0.0f, 1.0f, 1.0f); // 18
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_WEAPON_CLIP], 0.5f, 0.6f, 0.0f, 1.0f); // 19
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_SIZE_INFO], 0.72f, 0.72f, 0.72f, 1.0f); // 20

	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_MODEL], 0.05f, 0.1f, 0.15f, 1.0f); // 21
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_NONCOLLIDING], 1.0f, 0.365f, 0.16f, 1.0f); // 22
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_WIREFRAME], 0.0f, 0.0f, 0.0f, 1.0f); // 23 - not used org
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_FROZEN_LAYERS], 0.4f, 0.4f, 0.8f, 0.5f); // 24 - not used org
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN2], 0.0f, 0.0f, 0.0f, 1.0f); // 25 - not used org
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN3], 0.0f, 0.0f, 0.0f, 1.0f); // 26 - not used org
}


void MFCCreate()
{
	components::effects::radiant_init_fx();

	long savedinfo_size = sizeof(game::g_qeglobals->d_savedinfo);
	LoadRegistryInfo("SavedInfo", &game::g_qeglobals->d_savedinfo, &savedinfo_size);

	int old_size = game::g_qeglobals->d_savedinfo.iSize;
	if (game::g_qeglobals->d_savedinfo.iSize != sizeof(game::g_qeglobals->d_savedinfo))
	{
		game::g_qeglobals->d_savedinfo.iSize = sizeof(game::g_qeglobals->d_savedinfo);
		game::g_qeglobals->d_savedinfo.iTextMenu = 32993;
		game::g_qeglobals->d_savedinfo.d_gridsize = 1.0f;
		game::g_qeglobals->d_savedinfo.d_picmip = 0;
		
		radiantapp::set_default_savedinfo_colors();

		if(old_size < sizeof(game::g_qeglobals->d_savedinfo))
		{
			savedinfo_size = old_size;
			LoadRegistryInfo("SavedInfo", &game::g_qeglobals->d_savedinfo, &savedinfo_size);
		}
	}

	if (HMENU hMenu = GetMenu(game::g_qeglobals->d_hwndMain); 
			  hMenu)
	{
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & CONNECTIONS) != 0)
		{
			CheckMenuItem(hMenu, 0x84B6, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & NAMES) != 0)
		{
			CheckMenuItem(hMenu, 0x84B3, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & BLOCKS) != 0)
		{
			CheckMenuItem(hMenu, 0x84B5, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & COORDINATES) != 0)
		{
			CheckMenuItem(hMenu, 0x84B7, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & ANGLES) != 0)
		{
			CheckMenuItem(hMenu, 0x84B4, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & REVERSE_FILTER) != 0)
		{
			CheckMenuItem(hMenu, 0x8D1F, 0);
		}
	}
}

bool on_exit_instance()
{
	// no logic referencing any win32 ui because the mainframe is already destroyed
	// > use cmainframe::on_destroy() instead
	
	// asm stub logic
	return game::r_initiated;
}

__declspec(naked) void on_exit_instance_stub()
{
	const static uint32_t retn_pt_renderer_initiated = 0x450A2C;
	const static uint32_t retn_pt = 0x450A42;
	__asm
	{
		pushad;
		call	on_exit_instance;
		test	al, al;
		popad;

		push    esi;
		mov     esi, ecx;
		je      LOC_450A42;
		jmp		retn_pt_renderer_initiated;
		
		LOC_450A42:
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

void radiantapp::hooks()
{
	// force global preferences on init
	utils::hook(0x450730, force_preferences_on_init_stub, HOOK_JUMP).install()->quick();
	
	// registery saving stub :: CPrefsDlg::SavePrefs
	utils::hook(0x44F294, registery_save_stub, HOOK_JUMP).install()->quick();

	// always save preferences (even if loaded map = unnamed.map)
	utils::hook::nop(0x422115, 2);

	// registery loading stub :: CPrefsDlg::LoadPrefs
	utils::hook(0x44E38C, registery_load_stub, HOOK_JUMP).install()->quick();

	// default savedinfo values, mainly colors
	utils::hook(0x420A39, MFCCreate, HOOK_CALL).install()->quick();
	
	// note:
	// we need to save the menubar state within the registery because dvars are not yet initialized when we need the menubar state
	
	// show/hide mainframe menubar on startup 01 :: CRadiantApp::InitInstance
	utils::hook(0x4507CA, menubar_stub_01, HOOK_JUMP).install()->quick();

	// show/hide mainframe menubar on startup 02 :: CMainFrame::OnCreate
	utils::hook(0x42104A, menubar_stub_02, HOOK_JUMP).install()->quick();

	// show/hide mainframe menubar on startup 03 :: CMainFrame::OnCreate
	utils::hook(0x421057, menubar_stub_03, HOOK_JUMP).install()->quick();

	utils::hook(0x450A27, on_exit_instance_stub, HOOK_JUMP).install()->quick();

	// do not use or overwrite stock radiant registry keys - create seperate ones for IW3xRadiant
	utils::hook::write_string(0x6EBA58, R"(Software\iw\IW3xRadiant\IW3xRadiant)"s);
	utils::hook::write_string(0x6E2320, R"(iw\IW3xRadiant)"s);
	utils::hook::write_string(0x6E22F0, R"(Software\iw\IW3xRadiant\IniPrefs)"s);
	utils::hook::write_string(0x6DC1EC, R"(Software\iw\IW3xRadiant\MRU)"s);

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