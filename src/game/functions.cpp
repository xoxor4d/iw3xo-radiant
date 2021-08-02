#include "std_include.hpp"

namespace ggui
{
	imgui_state_t state = imgui_state_t();
	bool mainframe_menubar_visible = true;
	
}

namespace game
{
	namespace glob
	{
		// Init
		std::string loadedModules;

		bool radiant_floatingWindows;
		bool radiant_initiated;
		bool radiant_config_loaded;
		bool radiant_config_not_found;

		CWnd* m_pCamWnd_ref;

		// Misc
		game::TrackWorldspawn track_worldspawn = game::TrackWorldspawn();

		// Live Link
		game::ProcessServerCommands cServerCmd = game::ProcessServerCommands();
		bool live_connected;

		// Renderer
		IDirect3DDevice9* d3d9_device = nullptr;
		
	}

	// radiant globals
	int&		g_nScaleHow = *reinterpret_cast<int*>(0x23F16DC);
	//CPrefsDlg*	g_PrefsDlg = reinterpret_cast<CPrefsDlg*>(0x73C704);
	game::qeglobals_t* g_qeglobals = reinterpret_cast<game::qeglobals_t*>(0x25F39C0);
	
	int*	g_nUpdateBitsPtr = reinterpret_cast<int*>(0x25D5A74);
	int&	g_nUpdateBits = *reinterpret_cast<int*>(0x25D5A74);
	bool&	g_bScreenUpdates = *reinterpret_cast<bool*>(0x739B0F);
	double& g_time = *reinterpret_cast<double*>(0x2665678);
	double& g_oldtime = *reinterpret_cast<double*>(0x2665670);
	bool&	g_region_active = *reinterpret_cast<bool*>(0x23F1744);

	game::filter_material_t* filter_surfacetype_array = reinterpret_cast<game::filter_material_t*>(0x73AF80);
	game::filter_material_t* filter_locale_array = reinterpret_cast<game::filter_material_t*>(0x73A780);
	game::filter_material_t* filter_usage_array = reinterpret_cast<game::filter_material_t*>(0x739F80);
	std::uint8_t& texWndGlob_surfaceTypeFilter = *reinterpret_cast<std::uint8_t*>(0x25D799E); // current selected index
	std::uint8_t& texWndGlob_localeFilter = *reinterpret_cast<std::uint8_t*>(0x25D799D); // current selected index
	std::uint8_t& texWndGlob_usageFilter = *reinterpret_cast<std::uint8_t*>(0x25D799C); // current selected index
	int& texWndGlob_localeCount = *reinterpret_cast<int*>(0x25D7998); // amount of loaded locale filters
	int& texWndGlob_usageCount = *reinterpret_cast<int*>(0x25D7994); // amount of loaded usage filters
	
	CPrefsDlg* g_PrefsDlg()
	{
		const auto prefs = reinterpret_cast<CPrefsDlg*>(*(DWORD*)0x73C704);
		return prefs;
	}
	
	game::undo_s* g_lastundo()
	{
		const auto undo = reinterpret_cast<game::undo_s*>(*(DWORD*)0x23F162C);
		return undo;
	}
	
	game::undo_s* g_lastredo()
	{
		const auto redo = reinterpret_cast<game::undo_s*>(*(DWORD*)0x23F15CC);
		return redo;
	}
	
	game::DxGlobals* dx = reinterpret_cast<game::DxGlobals*>(0x1365684);

	int* dvarCount = reinterpret_cast<int*>(0x242394C);
	game::dvar_s* dvarPool = reinterpret_cast<game::dvar_s*>(0x2427DA4); // dvarpool + 1 dvar size
	game::dvar_s* dvarPool_FirstEmpty = reinterpret_cast<game::dvar_s*>(0x242C14C); // first empty dvar 
	DWORD* sortedDvars = reinterpret_cast<DWORD*>(0x2423958); // sorted dvar* list
	DWORD* sortedDvarsAddons = reinterpret_cast<DWORD*>(0x2423CEC); // first empty pointer
	int sortedDvarsAddonsCount = 0;

	//game::selbrush_t *selected_brushes = reinterpret_cast<game::selbrush_t*>(0x23F1864);
	//game::selbrush_t *selected_brushes_next = reinterpret_cast<game::selbrush_t*>(0x23F1868);

	//entity_s* edit_entity = reinterpret_cast<entity_s*>(0x240A108); // add structs
	//entity_s* world_entity = reinterpret_cast<entity_s*>(0x25D5B30); // add structs

	Com_Error_t Com_Error = Com_Error_t(0x499F50);
	OnCtlColor_t OnCtlColor = OnCtlColor_t(0x587907);

	
	// -----------------------------------------------------------
	// DVARS

	void Dvar_SetString(game::dvar_s *dvar /*esi*/, const char *string /*ebx*/)
	{
		const static uint32_t Dvar_SetString_Func = 0x4B38D0;
		
		__asm pushad
		__asm mov		ebx, [string]
		__asm mov		esi, [dvar]
		__asm Call		Dvar_SetString_Func
		__asm popad
	}

	const char* Dvar_DisplayableValue(game::dvar_s* dvar)
	{
		const static uint32_t Dvar_DisplayableValue_Func = 0x4AFAA0;
		__asm
		{
			mov		eax, dvar
			Call	Dvar_DisplayableValue_Func
		}
	}

	game::dvar_s * Dvar_SetFromStringFromSource(const char *string /*ecx*/, game::dvar_s *dvar /*esi*/, int source)
	{
		const static uint32_t Dvar_SetFromStringFromSource_Func = 0x4B3910;
		__asm
		{
			mov		esi, dvar
			push	source
			mov		ecx, string

			Call	Dvar_SetFromStringFromSource_Func
			add		esp, 4h
		}
	}

	__declspec(naked) game::dvar_s* Dvar_FindVar(const char* /*dvar*/)
	{
		__asm
		{
			push eax
			pushad

			mov ebx, [esp + 28h]
			mov eax, 4B0F00h // Dvar_FindMalleableVar Addr.
			call eax

			mov[esp + 20h], eax
			popad

			pop eax
			retn
		}
	}

	void console_error(const std::string &msg)
	{
		std::string err = "[!] " + msg + "\n";
		printf(err.c_str());
	}

	void FS_ScanForDir(const char* directory, const char* search_path, int localized)
	{
		const static uint32_t FS_ScanForDir_Func = 0x4A1E80;
		__asm
		{
			pushad;
			push	localized;
			mov		edx, directory;
			mov     ecx, search_path;
			call	FS_ScanForDir_Func;
			add		esp, 4;
			popad;
		}
	}

	game::GfxImage* Image_FindExisting(const char* name)
	{
		const static uint32_t Image_FindExisting_Func = 0x513200;
		__asm
		{
			//pushad;
			mov     edi, name;
			call	Image_FindExisting_Func;
			//popad;
		}
	}

	game::GfxImage* Image_RegisterHandle(const char* name)
	{
		game::GfxImage* image = game::Image_FindExisting(name);
		
		if (!image)
		{
			// Image_FindExisting_LoadObj
			image = utils::function<game::GfxImage* (const char* name, int, int)>(0x54FFC0)(name, 1, 0);
		}

		return image;
	}
	
}
