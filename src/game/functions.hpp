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

namespace game
{
	namespace glob
	{
		// Init
		extern bool command_thread_running;
		extern std::vector<std::string> loadedModules;
		
		extern bool radiant_floatingWindows;
		extern bool radiant_initiated;
		extern bool radiant_config_loaded;
		extern bool radiant_config_not_found;
		extern int  frametime_ms;
		extern bool ccamwindow_realtime;

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
	extern game::qeglobals_t* g_qeglobals;

	extern float&	g_zoomLevel;
	extern int*		g_nUpdateBitsPtr;
	extern int&		g_nUpdateBits;
	extern bool&	g_bScreenUpdates;
	extern double&	g_time;
	extern double&	g_oldtime;
	extern bool&	g_region_active;
	extern bool&	g_bCrossHairs;
	extern bool&	g_bClipMode;
	extern bool&	g_bRotateMode;
	extern bool&	g_bScaleMode;
	extern int&		g_nLastLen;
	extern int&		g_undoMaxSize;

	extern float*	g_vRotateOrigin;
	
	extern game::SCommandInfo*	g_Commands;
	extern int		g_nCommandCount;
	
	extern game::filter_material_t* filter_surfacetype_array;
	extern game::filter_material_t* filter_locale_array;
	extern game::filter_material_t* filter_usage_array;
	extern std::uint8_t& texWndGlob_surfaceTypeFilter;
	extern std::uint8_t& texWndGlob_localeFilter;
	extern std::uint8_t& texWndGlob_usageFilter;
	extern int& texWndGlob_localeCount;
	extern int& texWndGlob_usageCount;

	extern bool& r_initiated;
	extern game::GfxBackEndData* gfx_backend_data;
	extern game::GfxCmdBufSourceState* gfx_cmd_buf_source_state;
	extern game::r_globals_t* rg;
	extern game::r_global_permanent_t* rgp;
	extern game::GfxScene* scene;
	extern game::DxGlobals* dx;

	extern game::entity_s* g_world_entity();
	extern game::selbrush_t* g_active_brushes();
	extern game::selbrush_t* g_active_brushes_next();
	extern game::selbrush_t* g_selected_brushes();
	extern game::selbrush_t* g_selected_brushes_next();
	extern game::entity_s_def* g_edit_entity();
	extern int& multiple_edit_entities;
	extern HWND* entitywnd_hwnds;
	
	extern game::eclass_t* g_eclass();
	extern CPrefsDlg* g_PrefsDlg();
	extern void CPrefsDlg_SavePrefs();

	extern game::undo_s* g_lastundo();
	extern game::undo_s* g_lastredo();
	static utils::function<void()> Undo_ClearRedo = 0x45DF20;
	void Undo_GeneralStart(const char* operation /*eax*/);
	void Undo_AddEntity_W(game::entity_s* ent /*eax*/);
	static utils::function<void()> Undo_End = 0x45EA20;

	void DeleteKey(game::epair_t*& epair /*eax*/, const char* key /*ebx*/);
	void Checkkey_Model(entity_s* ent /*esi*/, const char* key);
	void Checkkey_Color(entity_s* ent /*eax*/, const char* key /*ebx*/);

	void SetSpawnFlags(int flag);
	void UpdateSel(int wParam, game::eclass_t* e_class);
	static utils::function<void(bool)> Select_Deselect = 0x48E800;
	void Brush_Move(const float* delta, game::brush_t* def, int snap);
	int  Brush_MoveVertex(const float* delta /*eax*/, game::brush_t* def, float* move_points, float* end);

	static utils::function<void(game::entity_s* ent, const char* key, const char* value)> SetKeyValue = 0x483690;
	static utils::function<void()> SetKeyValuePairs = 0x496CF0;
	static utils::function<void()> CreateEntity = 0x497300;

	// world bounds, not local
	static utils::function<void(game::XModel* model, float* axis, float* mins, float* maxs)> R_GetXModelBounds = 0x4C9150;
	static utils::function<game::XModel*(const char*)> R_RegisterModel = 0x51D450;
	
	const char** FS_ListFilteredFilesWrapper(const char* path /*edx*/, const char* null /*esi*/, int* file_count);
	void CreateEntityBrush(int height /*eax*/, int x /*ecx*/, void* cxywnd);
	game::trace_t* Trace_AllDirectionsIfFailed(float* cam_origin /*ebx*/, void* trace_result, float* dir, int contents);
	
	extern int* dvarCount;
	extern game::dvar_s* dvarPool;
	extern game::dvar_s* dvarPool_FirstEmpty;
	extern DWORD* sortedDvars;
	extern DWORD* sortedDvarsAddons;
	extern int sortedDvarsAddonsCount;

	static DWORD* frontEndDataOut_ptr = (DWORD*)(0x73D480);  // frontEndDataOut pointer
	static DWORD* active_brushes_ptr = (DWORD*)(0x23F189C);
	static DWORD* active_brushes_next_ptr = (DWORD*)(0x23F18A0);
	static DWORD* currSelectedBrushes = (DWORD*)(0x23F1864); // (selected_brushes array pointer)
	static DWORD* worldEntity_ptr = (DWORD*)(0x25D5B30); // holds pointer to worldEntity
	static DWORD* g_pParentWnd_ptr = (DWORD*)(0x25D5A70);

	// -----------------------------------------------------------

	typedef void(__cdecl* cdecl_no_args_t)();
	
	typedef void(*Com_Error_t)(const char *error, ...);
		extern Com_Error_t Com_Error;

	typedef HBRUSH(__thiscall* OnCtlColor_t)(void* thisptr, class CDC* pDC, class CWnd* pWnd, UINT nCtlColor);
		extern OnCtlColor_t OnCtlColor;

	static utils::function<bool(bool bQuiet)> QE_SingleBrush = 0x48C8B0; // no warnings when bQuiet
	static utils::function<void(game::DebugGlobals *debugGlobalsEntry, const float *mins, const float *maxs, const float *color)> R_AddDebugBox = 0x528710;

	//bool IsBrushSelected(game::brush_t* bSel);
	static utils::function<void(unsigned int flags)> Drag_MouseUp = 0x4802A0;
	
	// *
	// renderer
	static utils::function<void()> R_BeginFrame = 0x4FCB10;
	static utils::function<void()> R_EndFrame = 0x4FCBC0;
	static utils::function<void()> R_ReloadImages = 0x513D70;
	static utils::function<void(float* from, game::GfxColor* to)> Byte4PackPixelColor = 0x402AC0;
	static utils::function<game::GfxViewParms*()> R_SetupViewParms = 0x4FB540;
	static utils::function<void(game::GfxMatrix*, float halfx, float halfy, float znear)> R_SetupProjection = 0x4A78E0;
	static utils::function<void(game::GfxSceneDef*, game::GfxViewParms*)> R_SetupRenderCmd = 0x4FC3A0;
	static utils::function<void(int, const float*, float, bool)> R_Clear = 0x4FCC70;
	static utils::function<void(int)> R_IssueRenderCommands = 0x4FD630;
	static utils::function<void()> R_SortMaterials = 0x4FD910;
	static utils::function<bool(HWND)> R_SetupRendertarget_CheckDevice = 0x501A70;
	static utils::function<bool(HWND)> R_CheckTargetWindow = 0x500660;
	
	typedef void(*MatrixForViewer_t)(float(*mtx)[4], const float* origin, const float* axis);
		extern MatrixForViewer_t MatrixForViewer;
	
	typedef void(*MatrixMultiply44_t)(game::GfxViewParms* view_params, game::GfxMatrix* a, game::GfxMatrix* b);
		extern MatrixMultiply44_t MatrixMultiply44;
	
	typedef void(*MatrixInverse44_t)(game::GfxMatrix* a, game::GfxMatrix* b);
		extern MatrixInverse44_t MatrixInverse44;

	typedef void(*CopyAxis_t)(float* src, float* dest);
		extern CopyAxis_t CopyAxis;

	typedef void(*AnglesToAxis_t)(float*, float*);
		extern AnglesToAxis_t AnglesToAxis;

	typedef void(*AngleVectors_t)(float* _angles, float* _vpn, float* _right, float* _up);
		extern AngleVectors_t AngleVectors;

	typedef void(*OrientationConcatenate_t)(const game::orientation_t* orFirst, const game::orientation_t* orSecond, game::orientation_t* out);
		extern OrientationConcatenate_t OrientationConcatenate;

	// no error but doesnt reload everything
	static utils::function<void()> DX_ResetDevice = 0x5015F0;
	static utils::function<int*(size_t)> Hunk_Alloc = 0x5104E0;

	// *
	// dvars

	static utils::function< void (game::dvar_s* dvar, bool value)>							Dvar_SetBool = 0x4B37F0;
	static utils::function< void (game::dvar_s* dvar, std::int32_t value)>					Dvar_SetInt = 0x4B3810;
	static utils::function< void (game::dvar_s* dvar, float value)>							Dvar_SetFloat = 0x4B3830;
	static utils::function< void (game::dvar_s* dvar, float r, float g, float b, float a)>	Dvar_SetColor = 0x4B38E0;

	static utils::function< void (game::dvar_s* dvar, float x, float y)>						Dvar_SetVec2 = 0x4B3850;
	static utils::function< void (game::dvar_s* dvar, float x, float y, float z)>				Dvar_SetVec3 = 0x4B3870;
	static utils::function< void (game::dvar_s* dvar, float x, float y, float z, float w)>		Dvar_SetVec4 = 0x4B38A0;

	static utils::function< game::dvar_s* (const char* dvar_name, char value, __int16 flags, const char* description)> Dvar_RegisterBool = 0x4B25F0;
	static utils::function< game::dvar_s* (const char* dvar_name, int value, int mins, int maxs, __int16 flags, const char* description)> Dvar_RegisterInt = 0x4B2660;
	static utils::function< game::dvar_s* (const char* dvar_name, float value, float mins, float maxs, __int16 flags, const char* description)> Dvar_RegisterFloat = 0x4B26D0;
	
	static utils::function< game::dvar_s* (const char* dvar_name, float x, float y, float mins, float maxs, __int16 flags, const char* description)> Dvar_RegisterVec2 = 0x4B2750;
	static utils::function< game::dvar_s* (const char* dvar_name, float x, float y, float z, float mins, float maxs, __int16 flags, const char* description)> Dvar_RegisterVec3 = 0x4B27D0;
	//static utils::function< game::dvar_s* (const char* dvar_name, float x, float y, float z, float w, float mins, float maxs, __int16 flags, const char* description)> Dvar_RegisterVec4 = 0x4B2860;
	game::dvar_s* Dvar_RegisterVec4(const char* dvar_name /*ecx*/, float x, float y, float z, float w, float mins, float maxs, __int16 flags /*di/edi*/, const char* description);
	
	static utils::function< game::dvar_s* (const char *dvarName, const char *value, __int16 flags, const char *description)> Dvar_RegisterString = 0x4B28E0;

	// ASM
	const char* Dvar_DisplayableValue(game::dvar_s* dvar);
	const char* Dvar_DomainToString_Internal(signed int buffer_len /*eax*/, const char* buffer_out /*ebx*/, int dvar_type, int* enum_lines_count, float mins, float maxs);
	void Dvar_SetString(game::dvar_s *dvar /*esi*/, const char *string /*ebx*/);
	game::dvar_s* Dvar_FindVar(const char* dvar);
	game::dvar_s* Dvar_SetFromStringFromSource(const char *string /*ecx*/, game::dvar_s *dvar /*esi*/, int source);

	int printf_to_console_internal(const char* _format, va_list va);
	int printf_to_console(_In_z_ _Printf_format_string_ char const* const _format, ...);
	void com_printf_to_console(int channel, const char* _format, ...);
	void console_error(const std::string& msg);

	void FS_ScanForDir(const char* directory, const char* search_path, int localized);
	game::GfxImage* Image_FindExisting(const char* name);
	game::GfxImage* Image_RegisterHandle(const char* name);

	game::GfxCmdHeader* R_GetCommandBuffer(int bytes /*ebx*/, int render_cmd /*edi*/);
	void R_Hwnd_Resize(HWND__* hwnd, int display_width, int display_height);
	
}
