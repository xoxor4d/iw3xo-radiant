#pragma once

#define GET_PARENTWND (CMainFrame*) *(DWORD*)(game::g_pParentWnd_ptr)

// Taken directly from q3radiant
// https://github.com/id-Software/Quake-III-Arena
#define W_CAMERA		0x0001
#define W_XY			0x0002
#define W_XY_OVERLAY	0x0004
#define W_Z				0x0008
#define W_TEXTURE		0x0010
#define W_Z_OVERLAY		0x0020
#define W_CONSOLE		0x0040
#define W_ENTITY		0x0080
#define W_CAMERA_IFON	0x0100
#define W_XZ			0x0200  //--| only used for patch vertex manip stuff
#define W_YZ			0x0400  //--|
#define W_GROUP			0x0800 
#define W_MEDIA			0x1000 
#define W_ALL			0xFFFFFFFF

namespace ggui
{
	extern imgui_state_t state;
	extern bool mainframe_menubar_visible;
	
}

namespace game
{
	namespace glob
	{
		// Init
		extern std::string loadedModules;

		extern bool radiant_floatingWindows;
		extern bool radiant_initiated;
		extern bool radiant_config_loaded;
		extern bool radiant_config_not_found;

		extern CWnd* m_pCamWnd_ref;

		// Misc
		extern game::TrackWorldspawn track_worldspawn;

		// Live Link
		extern game::ProcessServerCommands cServerCmd;
		extern bool live_connected;

		// Renderer
		extern IDirect3DDevice9* d3d9_device;
		
	}
	
	// radiant globals
	extern int&			g_nScaleHow;
	//extern CPrefsDlg*	g_PrefsDlg;
	extern game::qeglobals_t* g_qeglobals;

	extern int*		g_nUpdateBitsPtr;
	extern int&		g_nUpdateBits;
	extern bool&	g_bScreenUpdates;
	extern double&	g_time;
	extern double&	g_oldtime;
	extern bool&	g_region_active;
	extern bool&	g_bCrossHairs;
	
	extern game::filter_material_t* filter_surfacetype_array;
	extern game::filter_material_t* filter_locale_array;
	extern game::filter_material_t* filter_usage_array;
	extern std::uint8_t& texWndGlob_surfaceTypeFilter;
	extern std::uint8_t& texWndGlob_localeFilter;
	extern std::uint8_t& texWndGlob_usageFilter;
	extern int& texWndGlob_localeCount;
	extern int& texWndGlob_usageCount;
	
	extern CPrefsDlg* g_PrefsDlg();
	extern game::undo_s* g_lastundo();
	extern game::undo_s* g_lastredo();

	//static DWORD* g_lastundo_ptr = (DWORD*)(0x23F162C);
	//static DWORD* g_lastredo_ptr = (DWORD*)(0x23F15CC);
	
	extern game::DxGlobals* dx;

	extern int* dvarCount;
	extern game::dvar_s* dvarPool;
	extern game::dvar_s* dvarPool_FirstEmpty;
	extern DWORD* sortedDvars;
	extern DWORD* sortedDvarsAddons;
	extern int sortedDvarsAddonsCount;

	static DWORD* frontEndDataOut_ptr = (DWORD*)(0x73D480);  // frontEndDataOut pointer
	static DWORD* currSelectedBrushes = (DWORD*)(0x23F1864); // (selected_brushes array pointer)
	static DWORD* worldEntity_ptr = (DWORD*)(0x25D5B30); // holds pointer to worldEntity
	static DWORD* g_pParentWnd_ptr = (DWORD*)(0x25D5A70);

	// -----------------------------------------------------------

	typedef void(*Com_Error_t)(const char *error, ...);
		extern Com_Error_t Com_Error;

	typedef HBRUSH(__thiscall* OnCtlColor_t)(void* thisptr, class CDC* pDC, class CWnd* pWnd, UINT nCtlColor);
		extern OnCtlColor_t OnCtlColor;

	static utils::function<bool(bool bQuiet)> QE_SingleBrush = 0x48C8B0; // no warnings when bQuiet
	static utils::function<void(game::DebugGlobals *debugGlobalsEntry, const float *mins, const float *maxs, const float *color)> R_AddDebugBox = 0x528710;

	//bool IsBrushSelected(game::brush_t* bSel);

	
	// *
	// renderer
	static utils::function<void()> R_EndFrame = 0x4FCBC0;
	static utils::function<void()> R_ReloadImages = 0x513D70;
	
	// no error but doesnt reload everything
	static utils::function< void()>	DX_ResetDevice = 0x5015F0;


	// *
	// dvars

	static utils::function< void (game::dvar_s* dvar, bool value)>							Dvar_SetBool = 0x4B37F0;
	static utils::function< void (game::dvar_s* dvar, std::int32_t value)>					Dvar_SetInt = 0x4B3810;
	static utils::function< void (game::dvar_s* dvar, float value)>							Dvar_SetFloat = 0x4B3830;
	static utils::function< void (game::dvar_s* dvar, float r, float g, float b, float a)>	Dvar_SetColor = 0x4B38E0;

	static utils::function< void (game::dvar_s* dvar, float x, float y)>						Dvar_SetVec2 = 0x4B3850;
	static utils::function< void (game::dvar_s* dvar, float x, float y, float z)>				Dvar_SetVec3 = 0x4B3870;
	static utils::function< void (game::dvar_s* dvar, float x, float y, float z, float w)>		Dvar_SetVec4 = 0x4B38A0;

	static utils::function< game::dvar_s* (const char* dvarName, int value, int mins, int maxs, __int16 flags, const char* description)> Dvar_RegisterInt = 0x4B2660;
	static utils::function< game::dvar_s* (const char* dvarName, char value, __int16 flags, const char* description)> Dvar_RegisterBool = 0x4B25F0;
	static utils::function< game::dvar_s* (const char* dvarName, float value, float mins, float maxs, __int16 flags, const char* description)> Dvar_RegisterFloat = 0x4B26D0;
	static utils::function< game::dvar_s* (const char *dvarName, const char *value, __int16 flags, const char *description)> Dvar_RegisterString = 0x4B28E0;

	// ASM
	const char* Dvar_DisplayableValue(game::dvar_s* dvar);
	void Dvar_SetString(game::dvar_s *dvar /*esi*/, const char *string /*ebx*/);
	game::dvar_s* Dvar_FindVar(const char* dvar);
	game::dvar_s* Dvar_SetFromStringFromSource(const char *string /*ecx*/, game::dvar_s *dvar /*esi*/, int source);

	void console_error(const std::string& msg);

	void FS_ScanForDir(const char* directory, const char* search_path, int localized);
	game::GfxImage* Image_FindExisting(const char* name);
	game::GfxImage* Image_RegisterHandle(const char* name);
	
}
