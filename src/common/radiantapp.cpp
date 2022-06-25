#include "std_include.hpp"

typedef int(__thiscall* CWinApp_WriteProfileInt_t)(CWinApp* pthis, LPCSTR lpAppName, LPCSTR lpValueName, int Data);
						CWinApp_WriteProfileInt_t CWinApp_WriteProfileInt = reinterpret_cast<CWinApp_WriteProfileInt_t>(0x59853E);

typedef int(__thiscall* CWinApp_GetProfileIntA_t)(CWinApp* pthis, LPCSTR cbData, LPCSTR lpValueName, int nDefault);
						CWinApp_GetProfileIntA_t CWinApp_GetProfileIntA = reinterpret_cast<CWinApp_GetProfileIntA_t>(0x5984D5);

// init stubs in order:
// * on_init_radiant_instance
// * MFCCreate
// * on_load_project (after mainframe visible)
// * radiantapp::on_create_client (after material load, right before the first frame)

// force global preferences on init (too early for dvars) (CRadiantApp::InitInstance)
void on_init_radiant_instance()
{
	const auto prefs = game::g_PrefsDlg();

	// force split view
	prefs->m_nView = 1;

	// disable detatched windows
	prefs->detatch_windows = false;

	// this can really kill performance without the user knowing why
	prefs->preview_sun_aswell = false;
}

// stub after FS-dvars are registered (QE_LoadProject)
void on_load_project()
{
	components::config::load_dvars();
	components::effects::radiant_init_fx();
	ctexwnd::init();
}

void radiantapp::on_create_client()
{
	components::d3dbsp::force_dvars();

	// disable r_vsync
	if (const auto& r_vsync = game::Dvar_FindVar("r_vsync");
		r_vsync && r_vsync->current.enabled)
	{
		dvars::set_bool(r_vsync, false);
	}

	// disable debug plumes drawing (only effect xmodels)
	if (const auto& r_showTriCounts = game::Dvar_FindVar("r_showTriCounts");
		r_showTriCounts && r_showTriCounts->current.enabled)
	{
		dvars::set_bool(r_showTriCounts, false);
	}

	if (const auto& r_showVertCounts = game::Dvar_FindVar("r_showVertCounts");
		r_showVertCounts && r_showVertCounts->current.enabled)
	{
		dvars::set_bool(r_showVertCounts, false);
	}

	if (const auto& r_showSurfCounts = game::Dvar_FindVar("r_showSurfCounts");
		r_showSurfCounts && r_showSurfCounts->current.enabled)
	{
		dvars::set_bool(r_showSurfCounts, false);
	}
}

// called from cmainframe::on_destroy
void radiantapp::on_shutdown()
{
	// restore states filter states
	if (components::gameview::p_this->get_all_geo_state())		components::gameview::p_this->toggle_all_geo(false);
	if (components::gameview::p_this->get_all_ents_state())		components::gameview::p_this->toggle_all_entities(false);
	if (components::gameview::p_this->get_all_triggers_state()) components::gameview::p_this->toggle_all_triggers(false);
	if (components::gameview::p_this->get_all_others_state())	components::gameview::p_this->toggle_all_others(false);

	if (dvars::radiant_gameview->current.enabled)
	{
		components::gameview::p_this->set_state(false);
	}

	components::remote_net::on_shutdown();
	components::config::write_dvars();

	GET_GUI(ggui::toolbar_dialog)->save_settings_ini();
	GET_GUI(ggui::hotkey_dialog)->on_close();
}

// stub before shutting down the fs and dvar system
bool on_exit_instance()
{
	// no logic referencing any win32 ui because the mainframe is already destroyed
	// > use cmainframe::on_destroy() instead

	// asm stub logic
	return game::r_initiated;
}

void registery_save([[maybe_unused]] CPrefsDlg* prefs)
{
	const auto state = afx::get_module_state();
	CWinApp_WriteProfileInt(state->m_pCurrentWinApp, "Prefs", "iw3x_mainframe_menubar_enabled", ggui::mainframe_menubar_enabled);
	CWinApp_WriteProfileInt(state->m_pCurrentWinApp, "Prefs", "lights_max_intensity", game::g_qeglobals->preview_at_max_intensity);
	CWinApp_WriteProfileInt(state->m_pCurrentWinApp, "Prefs", "force_zero_dropheight", prefs->m_bForceZeroDropHeight);
	CWinApp_WriteProfileInt(state->m_pCurrentWinApp, "Prefs", "discord_rpc", components::discord::g_enable_discord_rpc);
}

void registery_load()
{
	const auto state = afx::get_module_state();
	ggui::mainframe_menubar_enabled = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "iw3x_mainframe_menubar_enabled", 0);

	// put the undo levels variable to use (g_undoMaxSize always defaults to 64 otherwise)
	game::g_undoMaxSize = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "UndoLevels", 0);

	// always bothered me ..
	game::g_qeglobals->preview_at_max_intensity = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "lights_max_intensity", 0);

	if(const auto prefs = game::g_PrefsDlg(); prefs)
	{
		prefs->m_bForceZeroDropHeight = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "force_zero_dropheight", 1);
	}

	components::discord::g_enable_discord_rpc = CWinApp_GetProfileIntA(state->m_pCurrentWinApp, "Prefs", "discord_rpc", 1);
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
		RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\iw\\IW3xRadiant\\IW3xRadiant", 0, KEY_READ, &hKey);
	}

	RegQueryValueExA(hKey, pszName, NULL, (unsigned long*)&lType, (unsigned char*)pvBuf, (unsigned long*)plSize);
	RegCloseKey(hKey);
	
	return TRUE;
}

void radiantapp::set_default_savedinfo_colors()
{
	utils::vector::set_vec4(game::g_qeglobals->d_savedinfo.colors[game::COLOR_TEXTUREBACK], 0.22f, 0.22f, 0.22f, 1.0f); // 0
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
	long savedinfo_size = sizeof(game::g_qeglobals->d_savedinfo);
	LoadRegistryInfo("SavedInfo", &game::g_qeglobals->d_savedinfo, &savedinfo_size);

	const int old_size = game::g_qeglobals->d_savedinfo.iSize;
	if (game::g_qeglobals->d_savedinfo.iSize != sizeof(game::g_qeglobals->d_savedinfo))
	{
		game::g_qeglobals->d_savedinfo.iSize = sizeof(game::g_qeglobals->d_savedinfo);
		game::g_qeglobals->d_savedinfo.iTextMenu = 32992; // NORMALFAKELIGHT
		game::g_qeglobals->d_savedinfo.d_gridsize = 1.0f;
		game::g_qeglobals->d_savedinfo.d_picmip = 0;
		
		radiantapp::set_default_savedinfo_colors();

		if(old_size < sizeof(game::g_qeglobals->d_savedinfo))
		{
			savedinfo_size = old_size;
			LoadRegistryInfo("SavedInfo", &game::g_qeglobals->d_savedinfo, &savedinfo_size);
		}
	}

	if (const HMENU hMenu = GetMenu(game::g_qeglobals->d_hwndMain); 
					hMenu)
	{
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & game::GRID_FLAG_CONNECTIONS) != 0)
		{
			CheckMenuItem(hMenu, 0x84B6, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & game::GRID_FLAG_NAMES) != 0)
		{
			CheckMenuItem(hMenu, 0x84B3, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & game::GRID_FLAG_BLOCKS) != 0)
		{
			CheckMenuItem(hMenu, 0x84B5, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & game::GRID_FLAG_COORDINATES) != 0)
		{
			CheckMenuItem(hMenu, 0x84B7, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & game::GRID_FLAG_ANGLES) != 0)
		{
			CheckMenuItem(hMenu, 0x84B4, 0);
		}
		
		if ((game::g_qeglobals->d_savedinfo.d_xyShowFlags & game::GRID_FLAG_REVERSE_FILTER) != 0)
		{
			CheckMenuItem(hMenu, 0x8D1F, 0);
		}
	}
}

__declspec(naked) void on_init_radiant_instance_stub()
{
	const static uint32_t retn_addr = 0x450735;
	__asm
	{
		push    ecx; // og
		lea     ecx, [esp + 1Ch]; // og

		pushad;
		call	on_init_radiant_instance;
		popad;

		jmp		retn_addr;
	}
}

__declspec(naked) void on_load_project_stub()
{
	const static uint32_t R_BeginRegistrationInternal_func = 0x416510;
	const static uint32_t retn_addr = 0x48BCF2; // next op

	__asm
	{
		pushad;
		call	on_load_project;
		popad;

		call	R_BeginRegistrationInternal_func;
		jmp		retn_addr;
	}
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

__declspec(naked) void registery_save_stub()
{
	const static uint32_t AfxGetModuleState_addr = 0x59390E;
	const static uint32_t retn_addr = 0x44F299;
	__asm
	{
		pushad;
		push	esi; // CPrefsDlg*
		call	registery_save;
		add		esp, 4;
		popad;

		call	AfxGetModuleState_addr;
		jmp		retn_addr;
	}
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

void radiantapp::hooks()
{
	// force global preferences on init (CRadiantApp::InitInstance)
	utils::hook(0x450730, on_init_radiant_instance_stub, HOOK_JUMP).install()->quick();

	// stub after FS-dvars are registered (QE_LoadProject)
	utils::hook(0x48BCED, on_load_project_stub, HOOK_JUMP).install()->quick();

	// stub before shutting down the fs and dvar system
	utils::hook(0x450A27, on_exit_instance_stub, HOOK_JUMP).install()->quick();

	// registery saving stub :: CPrefsDlg::SavePrefs
	utils::hook(0x44F294, registery_save_stub, HOOK_JUMP).install()->quick();

	// always save preferences (even if loaded map = unnamed.map) (CMainFrame::OnDestroy)
	utils::hook::nop(0x422115, 2);

	// registery loading stub :: CPrefsDlg::LoadPrefs
	utils::hook(0x44E38C, registery_load_stub, HOOK_JUMP).install()->quick();

	// default savedinfo values, mainly colors (CMainFrame::OnCreate)
	utils::hook(0x420A39, MFCCreate, HOOK_CALL).install()->quick();
	
	// note:
	// we need to save the menubar state within the registery because dvars are not yet initialized when we need the menubar state
	
	// show/hide mainframe menubar on startup 01 :: CRadiantApp::InitInstance
	utils::hook(0x4507CA, menubar_stub_01, HOOK_JUMP).install()->quick();

	// show/hide mainframe menubar on startup 02 :: CMainFrame::OnCreate
	utils::hook(0x42104A, menubar_stub_02, HOOK_JUMP).install()->quick();

	// show/hide mainframe menubar on startup 03 :: CMainFrame::OnCreate
	utils::hook(0x421057, menubar_stub_03, HOOK_JUMP).install()->quick();

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