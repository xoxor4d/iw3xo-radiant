#pragma once
#include "common/mainframe.hpp"
#include "fx_system/fx_structs.hpp"

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

#define FOR_ALL_SELECTED_BRUSHES(B) for (auto (B) = game::g_selected_brushes_next(); (DWORD*)(B) != game::currSelectedBrushes; (B) = (B)->next)
#define FOR_ALL_ACTIVE_BRUSHES(B) for (auto (B) = game::g_active_brushes_next(); (DWORD*)(B) != game::active_brushes_ptr; (B) = (B)->next)
#define FOR_ALL_ENTITY_INSTS(E) for (auto (E) = game::get_entity_insts_next(); (DWORD*)(E) != game::entity_insts_ptr; (E) = (E)->next)

// custom brush iter (eg. prefab brushes) - (is_single_brush :: a prefab with a single brush will reference itself)
//#define FOR_ALL_BRUSHES(B, B_CURR, B_NEXT) \
//	int is_single_brush = (DWORD*)(B_NEXT) == (DWORD*)(B_CURR); \
//	for (auto (B) = (B_NEXT); (DWORD*)(B) != (DWORD*)(&B_CURR) || --is_single_brush == 0; (B) = (B)->next)

#define FOR_ALL_BRUSHES(B, B_CURR, B_NEXT) \
	for (auto (B) = (B_NEXT); (DWORD*)(B) != (DWORD*)(&B_CURR); (B) = (B)->next)

#define mainframe_thiscall(return_val, addr)	\
		utils::hook::call<return_val(__fastcall)(cmainframe*)>(addr)(cmainframe::activewnd)

#define mainframe_cdeclcall(return_val, addr)	\
		utils::hook::call<return_val(__cdecl)(cmainframe*)>(addr)(cmainframe::activewnd)

#define mainframe_stdcall(return_val, addr)	\
		utils::hook::call<return_val(__stdcall)(cmainframe*)>(addr)(cmainframe::activewnd)

#define cdeclcall(return_val, addr)	\
		utils::hook::call<return_val(__cdecl)()>(addr)()



namespace game
{
	typedef void(__cdecl* cdecl_no_args_t)();

	namespace glob
	{
		// init
		extern bool command_thread_running;
		extern std::vector<std::string> loadedModules;
		
		extern bool radiant_initiated;
		extern bool radiant_config_loaded;
		extern bool radiant_config_not_found;
		extern float frames_per_second;
		extern bool ccamwindow_realtime;

		// misc
		extern game::TrackWorldspawn track_worldspawn;

		extern bool debug_sundir;
		extern game::vec3_t debug_sundir_startpos;
		extern float debug_sundir_length;

		extern bool is_loading_map;
		extern bool in_shutdown;

		// update check
		extern std::string gh_update_releases_json;
		extern std::string gh_update_tag;
		extern std::string gh_update_date;
		extern std::string gh_update_title;
		extern std::string gh_update_desc;
		extern std::string gh_update_link;
		extern std::string gh_update_zip_name;
		extern bool gh_update_avail;

		// live-link
		extern game::ProcessServerCommands cServerCmd;
		extern bool live_connected;

		// renderer
		extern IDirect3DDevice9* d3d9_device;
		
	}

	static DWORD* frontEndDataOut_ptr = (DWORD*)(0x73D480);  // frontEndDataOut pointer
	static DWORD* backEndDataOut_ptr = (DWORD*)(0x174F970);  // backendEndDataOut pointer
	static DWORD* s_cmdList_ptr = (DWORD*)(0x73D4A0);
	static DWORD* active_brushes_ptr = (DWORD*)(0x23F189C);
	static DWORD* active_brushes_next_ptr = (DWORD*)(0x23F18A0);
	static DWORD* currSelectedBrushes = (DWORD*)(0x23F1864); // (selected_brushes array pointer)
	static DWORD* worldEntity_ptr = (DWORD*)(0x25D5B30); // holds pointer to worldEntity
	static DWORD* g_pParentWnd_ptr = (DWORD*)(0x25D5A70);
	static DWORD* entity_insts_ptr = (DWORD*)(0x23F1748); // (entity instances array pointer)

	extern game::vec3_t vec3_origin;
	extern game::vec4_t color_white;

	extern IDirect3DTexture9* framebuffer_test;
	
	// radiant globals
	extern int&		g_nScaleHow;
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

	extern float*	g_vRotateOrigin;
	extern int&		g_prefab_stack_level;

	extern bool&	g_bDoCone;
	extern bool&	g_bDoSphere;
	
	extern game::SCommandInfo*	g_Commands;
	extern int		g_nCommandCount;

	extern const char* current_map_filepath;
	
	extern game::filter_material_t* filter_surfacetype_array;
	extern game::filter_material_t* filter_locale_array;
	extern game::filter_material_t* filter_usage_array;
	extern std::uint8_t& texWndGlob_surfaceTypeFilter;
	extern std::uint8_t& texWndGlob_localeFilter;
	extern std::uint8_t& texWndGlob_usageFilter;
	extern int& texWndGlob_localeCount;
	extern int& texWndGlob_usageCount;

	extern bool& r_initiated;
	extern game::GfxBackEndData* gfx_frontend_data;
	extern game::GfxBackEndData* gfx_backend_data;

	extern game::GfxCmdBufSourceState* gfxCmdBufSourceState;
	extern game::GfxCmdBufState* gfxCmdBufState;
	extern game::GfxCmdBufInput* gfxCmdBufInput;

	extern game::r_globals_t* rg;
	extern game::r_global_permanent_t* rgp;
	extern game::GfxScene* scene;
	extern game::DxGlobals* dx;
	extern game::GfxImage** imageGlobals;

	extern game::ComWorld* comworld;
	extern game::GfxWorld* s_world;

	extern game::GfxBackEndData* get_backenddata();
	extern game::GfxBackEndData* get_frontenddata();
	extern game::GfxCmdArray* get_cmdlist();

	extern game::entity_s* g_world_entity();
	extern game::selbrush_def_t* g_active_brushes();
	extern game::selbrush_def_t* g_active_brushes_next();
	extern game::selbrush_def_t* g_selected_brushes();
	extern game::selbrush_def_t* g_selected_brushes_next();

	extern game::entity_s* get_entity_insts();
	extern game::entity_inst_s* get_entity_insts_next();

	extern const int& g_selected_faces_count;
	extern game::selface_t* g_selected_faces();
	extern game::entity_s_def* g_edit_entity();
	extern int& multiple_edit_entities;
	extern HWND* entitywnd_hwnds;

	extern CLayerDlg* layer_dlg;
	
	extern game::eclass_t* g_eclass();
	extern CSurfaceDlg* get_surfacedialog();
	extern CPrefsDlg* g_PrefsDlg();
	extern void CPrefsDlg_SavePrefs();

	extern game::undo_s* g_lastundo();
	extern game::undo_s* g_lastredo();
	extern int& g_undoMaxSize;
	extern int& g_undoId;
	extern int& g_undoSize;

	inline auto Undo_ClearRedo = reinterpret_cast<void (*)()>(0x45DF20);
	void Undo_GeneralStart(const char* operation /*eax*/);
	void Undo_AddBrushList(void* sb); //(game::selbrush_def_t* sb /*edi*/);
	void Undo_AddBrushList_Selected();
	void Undo_EndBrushList(void* sb); //(game::selbrush_def_t* sb /*esi*/);
	void Undo_EndBrushList_Selected();
	void Undo_AddEntity_W(game::entity_s* ent /*eax*/);
	void Undo_SetIdForEntity(game::entity_s* ent /*edx*/);
	inline auto Undo_AddBrush = reinterpret_cast<void (*)(game::brush_t_with_custom_def*)>(0x45E680);
	inline auto Undo_End = reinterpret_cast<void (*)()>(0x45EA20);

	extern bool is_single_brush_selected(bool print_warning = false);
	extern bool is_any_brush_selected();

	inline auto Prefab_Enter = reinterpret_cast<void (*)()>(0x42BF70); // CMainFrame::OnPrefabEnter
	inline auto Prefab_Leave = reinterpret_cast<void (*)()>(0x42BF80); // CMainFrame::OnPrefabLeave
	inline auto Drag_MouseUp = reinterpret_cast<void (*)(unsigned int flags)>(0x4802A0);

	void Texwnd_SelectMaterial(int x, int y /*ecx*/);

	void DeleteKey(game::epair_t*& epair /*eax*/, const char* key /*ebx*/);
	void Checkkey_Model(entity_s* ent /*esi*/, const char* key);
	void Checkkey_Color(entity_s* ent /*eax*/, const char* key /*ebx*/);

	void selection_rotate_axis(int axis, int deg);
	void Select_ApplyMatrix_SelectedBrushes(bool snap /*ebx*/, float* rotate_axis, float degree, int unk);
	void Select_ApplyMatrix(float* rotate_axis /*eax*/, void* brush, int snap, float degree, int unk /*bool*/);
	void Select_RotateAxis(int axis /*eax*/, float degree, float* rotate_axis);
	void Select_GetMid(float* midpoint /*esi*/);
	void Select_FuncGroup(float* trace_dir /*ecx*/, float* trace_start /*edx*/, int contents /*eax*/);
	void Select_RotateFixedSize(game::entity_s* owner, brush_t_with_custom_def* def, float(*mid_point)[3]);

	inline void Selection_Copy() { mainframe_thiscall(void, 0x4286B0); } // CMainFrame::OnEditCopybrush
	inline void Selection_Paste() { mainframe_thiscall(void, 0x4286D0); } // CMainFrame::OnEditPastebrush

	inline void clipboard_clear(cxywnd* gridwnd)
	{

		// unknown_libname_310 @ 0x593B2D
		DWORD* g_Clipboard = reinterpret_cast<DWORD*>(0x25EB210);

		// actually a thiscall but we can call it using fastcall if we pass a second unused argument ;)
		utils::hook::call<void(__fastcall)(DWORD* pthis, void* unused, unsigned int a2, int a3)>(0x593B2D)(g_Clipboard, nullptr, 0, 0);

		const auto clipformat = RegisterClipboardFormatA("RadiantClippings");

		if (clipformat)
		{
			if (OpenClipboard(gridwnd->GetWindow()))
			{
				EmptyClipboard();

				if (void* handle = GlobalAlloc(0x2042u, 0); handle)
				{
					SetClipboardData(clipformat, handle);
					CloseClipboard();
				}
			}
		}
	}


	inline auto Select_Deselect = reinterpret_cast<void (*)(bool)>(0x48E800);
	inline auto Select_Delete = reinterpret_cast<void (*)()>(0x48E760);
	inline auto Select_Invert = reinterpret_cast<void (*)()>(0x493F10);
	inline auto Select_Hide = reinterpret_cast<void (*)()>(0x493D50); // part of CMainFrame::OnHideSelected - hides all selected entities / brushes

	void SetSpawnFlags(int flag);
	void SetMaterial(const char* name /*edi*/, game::patchMesh_material* def /*esi*/);
	void UpdateSel(int wParam, game::eclass_t* e_class);
	bool FilterBrush(game::selbrush_def_t* sb /*esi*/, int always_null);

	void Patch_SelectRow(int row /*eax*/, game::patchMesh_t* p /*edi*/, int multi);
	void Patch_UpdateSelected(game::patchMesh_t* p /*esi*/, int always_true);
	void Patch_SetTextureInfo(game::texdef_sub_t* texdef /*ebx*/);
	void Patch_ShiftTexture(game::patchMesh_t* def, float shift_horz, float shift_vert);
	inline auto Patch_Lightmap_Texturing = reinterpret_cast<void (*)()>(0x448110);
	void Patch_Lightmap_Texturing_dirty(game::patchMesh_t* p /*esi*/);
	inline auto Patch_NaturalizeSelected = reinterpret_cast<void (*)(bool unk, bool cap, float x, float y)>(0x447FD0);
	inline auto Patch_SetTexturing = reinterpret_cast<void (*)(float x, float y, int mode)>(0x446B60);
	void Patch_CalcBounds(game::patchMesh_t* p, game::vec3_t& vMin, game::vec3_t& vMax);
	void Select_GetBounds(float* mins, float* maxs);
	void Patch_Adjust(game::patchMesh_t* p, bool insert, bool column, bool flag);
	game::patchMesh_t* Patch_Duplicate(game::patchMesh_t* p /*edi*/);
	game::selbrush_def_t* Patch_Cap(patchMesh_t* pm /*ecx*/, int bByColumn, int bFirst);
	game::selbrush_def_t* Patch_CapSpecial(patchMesh_t* pm /*ecx*/, int nType, int bFirst);
	void Patch_Invert(game::patchMesh_t* p /*ebx*/);
	void Patch_Rebuild(game::patchMesh_t* p /*esi*/, int reset);
	void Patch_InsertColumn(patchMesh_t* p, bool flag /*cl*/);
	void Patch_InsertRow(patchMesh_t* p, bool flag /*dl*/);
	void Patch_RemoveColumn(patchMesh_t* p /*eax*/, bool flag /*cl*/);
	void Patch_RemoveRow(patchMesh_t* p /*eax*/, bool flag /*cl*/);
	inline auto Patch_MeshNormals = reinterpret_cast<void (*)(game::patchMesh_t*)>(0x437C80);
	inline auto Patch_GenericMesh = reinterpret_cast<game::selbrush_def_t* (*)(int nWidth, int nHeight, int nOrientation, bool bDeleteSource, bool bOverwrite)>(0x43B310);
	inline auto Create_Terrain = reinterpret_cast<game::selbrush_def_t* (*)(int nWidth, int nHeight, int nOrientation)>(0x43B660);
	inline auto Patch_Thicken = reinterpret_cast<void (*)(int thickness, bool create_seams)>(0x448700);

	void Brush_Move(const float* delta, game::brush_t_with_custom_def* def, int snap);
	int  Brush_MoveVertex(const float* delta /*eax*/, game::brush_t_with_custom_def* def, float* move_points, float* end);
	void Brush_Create(float* maxs /*edx*/, float* mins /*ecx*/, game::brush_t_with_custom_def* brush, int unk);
	void Brush_BuildWindings(game::brush_t_with_custom_def* brush /*ecx*/, int snap);
	void Entity_LinkBrush(game::brush_t_with_custom_def* brush /*eax*/, game::entity_s* world /*edi*/);
	game::brush_t_with_custom_def* Brush_AddToList(game::brush_t_with_custom_def* brush /*eax*/, game::entity_s* world);
	void Brush_AddToList2(game::brush_t_with_custom_def* brush /*eax*/);
	void Brush_RemoveFromList(game::selbrush_def_t* brush /*eax*/);
	void Brush_Deselect(game::brush_t* b /*esi*/);
	void Brush_Select(game::selbrush_def_t* b /*ecx*/, bool select_connected, bool update_status, bool center_grid_on_selection);
	bool Patch_DragScale(game::patchMesh_t* pm, const float* bounds /*eax*/, const float* dist_vec);
	void Brush_SideSelect(game::brush_t_with_custom_def* def, const float* trace_start /*eax*/, const float* trace_dir, int shear);
	inline auto QE_SingleBrush = reinterpret_cast<bool (*)()>(0x48C8B0); // use is_single_brush_selected when used in loops
	inline auto Brush_MakeFacePlanes = reinterpret_cast<void (*)(game::brush_t_with_custom_def* b)>(0x470A50); // calculate normal and dist from planepts
	inline auto Brush_MakeSidedCone = reinterpret_cast<void (*)(int num_sides)>(0x47BC10);
	inline auto Brush_MakeSided_Axis = reinterpret_cast<void (*)(int num_sides, bool snap)>(0x4735E0);
	inline auto Brush_SetSampleSize = reinterpret_cast<void (*)(int sample_size)>(0x48F800);
	inline auto Brush_FitTexture = reinterpret_cast<void (*)(float x, float y, int bounds)>(0x4939E0);
	inline auto Brush_RebuildBrush = reinterpret_cast<void (*)(game::brush_t_with_custom_def * b, float* mins, float* maxs)>(0x438760);
	int CM_ForEachBrushPlaneIntersection(game::brush_t_with_custom_def* b /*esi*/, game::BrushPt_t* brush_pts);
	
	inline auto SetKeyValue = reinterpret_cast<void (*)(game::entity_s * ent, const char* key, const char* value)>(0x483690);
	inline auto SetKeyValuePairs = reinterpret_cast<void (*)()>(0x496CF0);
	inline auto CreateEntity = reinterpret_cast<void (*)()>(0x497300);
	game::eclass_t* Eclass_ForName(const char* name /*ecx*/, int has_brushes);
	void CreateEntityFromClassname(void* cxywnd /*edi*/, const char* name /*esi*/, int x, int y);


	inline bool CreateEntityFromName_DisableUndo = false;
	// encupsule in game::CreateEntityFromName_DisableUndo true/false to disable AddBrushList/EndBrushList
	inline auto CreateEntityFromName = reinterpret_cast<void (*)(const char* name)>(0x465CC0);


	void CreateEntityBrush(int height /*eax*/, int x /*ecx*/, void* cxywnd);
	game::entity_s* Entity_Create(eclass_t* eclass /*eax*/);

	inline auto Test_Ray = reinterpret_cast<void (*)(float* start, float* dir, int contents, game::trace_t * trace, int num_traces)>(0x48D7C0);
	game::trace_t* Trace_AllDirectionsIfFailed(float* cam_origin /*ebx*/, void* trace_result, float* dir, int contents);
	inline auto R_GetXModelBounds = reinterpret_cast<void (*)(game::XModel * model, float* axis, float* mins, float* maxs)>(0x4C9150); // world bounds, not local

	void map_load_from_file(const char* path);
	void map_save_file(const char* path /*ecx*/, int is_reg, int save_to_perforce);
	void map_write_selection(const char* path);
	void mru_new_item(game::LPMRUMENU* mru, const char* item_str);
	void mru_insert_item(game::LPMRUMENU* mru, HMENU menu);


	// *
	// * --------------------- renderer / math ------------------------------
	
	inline auto Byte4PackPixelColor = reinterpret_cast<void (*)(float* from, game::GfxColor * to)>(0x402AC0);
	inline auto R_BeginFrame = reinterpret_cast<void (*)()>(0x4FCB10);
	inline auto R_EndFrame = reinterpret_cast<void (*)()>(0x4FCBC0);
	inline auto R_ReloadImages = reinterpret_cast<void (*)()>(0x513D70);
	inline auto R_SetupViewParms = reinterpret_cast<game::GfxViewParms* (*)()>(0x4FB540);
	inline auto R_SetupProjection = reinterpret_cast<void (*)(game::GfxMatrix*, float halfx, float halfy, float znear)>(0x4A78E0);
	inline auto R_SetupRenderCmd = reinterpret_cast<void (*)(game::GfxSceneDef*, game::GfxViewParms*)>(0x4FC3A0);
	inline auto R_Clear = reinterpret_cast<void (*)(int whichToClear, const float* color, float depth, bool)>(0x4FCC70);
	inline auto R_ClearScreen = reinterpret_cast<void (*)(IDirect3DDevice9*, int whichToClear, const float* color, float depth, bool stencil, game::GfxViewport*)>(0x539AA0);
	inline auto R_IssueRenderCommands = reinterpret_cast<void (*)(int)>(0x4FD630);
	inline auto R_SortMaterials = reinterpret_cast<void (*)()>(0x4FD910);
	inline auto R_SetupRendertarget = reinterpret_cast<bool (*)(game::GfxCmdBufSourceState*, game::GfxRenderTargetId)>(0x539670);
	inline auto R_SetRenderTarget = reinterpret_cast<bool (*)(game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxRenderTargetId)>(0x5397A0);
	inline auto R_SetupRendertarget_CheckDevice = reinterpret_cast<bool (*)(HWND)>(0x501A70);
	inline auto R_InitRendererForWindow = reinterpret_cast<void (*)(HWND)>(0x5011D0);
	inline auto R_CheckTargetWindow = reinterpret_cast<bool (*)(HWND)>(0x500660);
	inline auto R_AddDebugBox = reinterpret_cast<void (*)(game::DebugGlobals * debugGlobalsEntry, const float* mins, const float* maxs, const float* color)>(0x528710);
	inline auto R_CmdBufSet3D = reinterpret_cast<void (*)(game::GfxCmdBufSourceState*)>(0x53CFB0);
	inline auto R_SetGameTime = reinterpret_cast<void (*)(game::GfxCmdBufSourceState*, float)>(0x55A4A0);
	inline auto R_SortWorldSurfaces = reinterpret_cast<void (*)()>(0x52E9F0);
	inline auto R_ClearScene = reinterpret_cast<void (*)(int clientnum)>(0x505770);
	inline auto R_SetLodOrigin = reinterpret_cast<void (*)(game::refdef_s* refdef)>(0x505F90);
	inline auto CL_RenderScene = reinterpret_cast<void (*)(game::refdef_s * refdef)>(0x506030);
	inline auto R_GenerateReflectionImages = reinterpret_cast<void (*)(GfxReflectionProbe * probes, DiskGfxReflectionProbe * probeRawGeneratedData, unsigned int probeCount, int mip)>(0x550F00);

	// verts - 4 vertices defining a quad
	void R_DrawSelectionbox(const float* verts);

	// sampler_index = the index used in shader_vars.h
	inline auto R_SetSampler = reinterpret_cast<void (*)(int unused, game::GfxCmdBufState * state, int sampler_index, char sampler_state, game::GfxImage * img)>(0x538D70);
	inline auto R_AddCmdDrawFullScreenColoredQuad = reinterpret_cast<bool (*)(float s0, float t0, float s1, float t1, float* color, game::Material * mtl)>(0x4FC260);
	inline auto R_SetMaterialColor = reinterpret_cast<void (*)(const float* color)>(0x4FC2C0);
	inline auto R_DrawSurfs = reinterpret_cast<void (*)(game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxCmdBufState*, game::GfxDrawSurfListInfo*)>(0x5324E0);
	inline auto R_ShowTris = reinterpret_cast<void (*)(game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxDrawSurfListInfo*)>(0x55B100);
	inline auto R_InitCmdBufSourceState = reinterpret_cast<void (*)(game::GfxCmdBufSourceState*, game::GfxCmdBufInput * input, int)>(0x53CB20);

	inline auto Com_BitCheckAssert = reinterpret_cast<BOOL (*)(const unsigned int* array, unsigned int bitNum, int size)>(0x501BA0);
	inline auto RB_SunShadowMaps = reinterpret_cast<void (*)(game::GfxBackEndData * backend, const game::GfxViewInfo * viewinfo)>(0x56E380);
	inline auto RB_SpotShadowMaps = reinterpret_cast<void (*)(game::GfxBackEndData * backend, const game::GfxViewInfo * viewinfo)>(0x56E100);

	game::GfxCmdHeader* R_GetCommandBuffer(std::uint32_t bytes /*ebx*/, int render_cmd /*edi*/);
	void R_AddCmdSetViewportValues(int x, int y, int width, int height);
	void R_Hwnd_Resize(HWND__* hwnd, int display_width, int display_height);

	inline auto MatrixForViewer = reinterpret_cast<void (*)(float(*mtx)[4], const float* origin, const float* axis)>(0x4A7A70);
	inline auto MatrixMultiply44 = reinterpret_cast<void (*)(game::GfxViewParms * view_params, game::GfxMatrix * a, game::GfxMatrix * b)>(0x4A6180);
	inline auto MatrixInverse44 = reinterpret_cast<void (*)(game::GfxMatrix * a, game::GfxMatrix * b)>(0x4A6670);

	inline auto CopyAxis = reinterpret_cast<void (*)(float* src, float* dest)>(0x4A8860);
	inline auto AnglesToAxis = reinterpret_cast<void (*)(float* angles, float* axis)>(0x4ABEB0);
	inline auto AnglesToQuat = reinterpret_cast<void (*)(const float* angles, float* quat)>(0x4AC050);

	void AxisToAngles(const float(*axis)[3], float* angles);
	void vectoangles(float* vec /*esi*/, float* angles /*edi*/);
	inline auto AngleVectors = reinterpret_cast<void (*)(float* _angles, float* _vpn, float* _right, float* _up)>(0x4ABD70);
	inline auto PerpendicularVector = reinterpret_cast<void (*)(float* src, float* dst)>(0x4A5340);
	inline auto OrientationConcatenate = reinterpret_cast<void (*)(const game::orientation_t * orFirst, const game::orientation_t * orSecond, game::orientation_t * out)>(0x4BA7D0);

	inline auto AlignEntityToFace = reinterpret_cast<void (*)(game::entity_s_def * ent, float* normal)>(0x485AD0);


	// no error but doesnt reload everything
	inline auto DX_ResetDevice = reinterpret_cast<void (*)()>(0x5015F0);
	inline auto Hunk_Alloc = reinterpret_cast<int* (*)(size_t)>(0x5104E0);
	inline auto Z_Malloc = reinterpret_cast<int* (*)(size_t)>(0x4AC330);
	inline auto Z_Free = reinterpret_cast<void (*)(void*)>(0x4AC2A0);
	inline auto Z_VirtualAlloc = reinterpret_cast<void* (*)(size_t)>(0x4AC380);


	// *
	// * --------------------- dvars ------------------------------

	extern int* dvarCount;
	extern game::dvar_s* dvarPool;
	extern game::dvar_s* dvarPool_FirstEmpty;
	extern DWORD* sortedDvars;
	extern DWORD* sortedDvarsAddons;
	extern int sortedDvarsAddonsCount;

	inline auto Dvar_SetBool = reinterpret_cast<void (*)(game::dvar_s* dvar, bool value)>(0x4B37F0);
	inline auto Dvar_SetInt = reinterpret_cast<void (*)(game::dvar_s* dvar, std::int32_t value)>(0x4B3810);
	inline auto Dvar_SetFloat = reinterpret_cast<void (*)(game::dvar_s* dvar, float value)>(0x4B3830);
	inline auto Dvar_SetColor = reinterpret_cast<void (*)(game::dvar_s* dvar, float r, float g, float b, float a)>(0x4B38E0);
	inline auto Dvar_SetVec2 = reinterpret_cast<void (*)(game::dvar_s* dvar, float x, float y)>(0x4B3850);
	inline auto Dvar_SetVec3 = reinterpret_cast<void (*)(game::dvar_s* dvar, float x, float y, float z)>(0x4B3870);
	inline auto Dvar_SetVec4 = reinterpret_cast<void (*)(game::dvar_s* dvar, float x, float y, float z, float w)>(0x4B38A0);
	inline auto Dvar_SetIntByName = reinterpret_cast<void (*)(int value, const char* name)>(0x4B3A60);

	inline auto Dvar_RegisterBool = reinterpret_cast<game::dvar_s* (*)(const char* dvar_name, char value, __int16 flags, const char* description)>(0x4B25F0);
	inline auto Dvar_RegisterInt = reinterpret_cast<game::dvar_s* (*)(const char* dvar_name, int value, int mins, int maxs, __int16 flags, const char* description)>(0x4B2660);
	inline auto Dvar_RegisterFloat = reinterpret_cast<game::dvar_s* (*)(const char* dvar_name, float value, float mins, float maxs, __int16 flags, const char* description)>(0x4B26D0);
	inline auto Dvar_RegisterVec2 = reinterpret_cast<game::dvar_s* (*)(const char* dvar_name, float x, float y, float mins, float maxs, __int16 flags, const char* description)>(0x4B2750);
	inline auto Dvar_RegisterVec3 = reinterpret_cast<game::dvar_s* (*)(const char* dvar_name, float x, float y, float z, float mins, float maxs, __int16 flags, const char* description)>(0x4B27D0);
	game::dvar_s* Dvar_RegisterVec4(const char* dvar_name /*ecx*/, float x, float y, float z, float w, float mins, float maxs, __int16 flags /*di/edi*/, const char* description);
	inline auto Dvar_RegisterString = reinterpret_cast<game::dvar_s* (*)(const char* dvarName, const char* value, __int16 flags, const char* description)>(0x4B28E0);
	
	const char* Dvar_DisplayableValue(game::dvar_s* dvar);
	const char* Dvar_DomainToString_Internal(signed int buffer_len /*eax*/, const char* buffer_out /*ebx*/, int dvar_type, int* enum_lines_count, float mins, float maxs);
	void Dvar_SetString(game::dvar_s *dvar /*esi*/, const char *string /*ebx*/);
	game::dvar_s* Dvar_FindVar(const char* dvar);
	game::dvar_s* Dvar_SetFromStringFromSource(const char *string /*ecx*/, game::dvar_s *dvar /*esi*/, int source);
	

	// *
	// * --------------------- console / warnings / error ------------------------------

	inline auto Com_Error = reinterpret_cast<void (*)(const char* error, ...)>(0x499F50);
	int printf_to_console_internal(const char* _format, va_list va);
	int printf_to_console(_In_z_ _Printf_format_string_ char const* const _format, ...);
	void com_printf_to_console(int channel, const char* _format, ...);
	void console_error(const std::string& msg);
	

	// *
	// * --------------------- fs / io ------------------------------

	extern game::fileData_s** com_fileDataHashTable;

	inline auto FS_ReadFile = reinterpret_cast<unsigned int (*)(const char*, void**)>(0x4A0240);
	inline auto FS_FreeFile = reinterpret_cast<void (*)(void*)>(0x4A0300);
	inline auto FS_Read = reinterpret_cast<int (*)(void* buffer, size_t len, signed int h)>(0x49FE30);
	inline auto FS_FOpenFileRead = reinterpret_cast<std::uint32_t (*)(const char* filename, int* file)>(0x49FAF0);
	inline auto FS_FCloseFile = reinterpret_cast<void (*)(int h)>(0x49EAD0);
	void FS_ScanForDir(const char* directory, const char* search_path, int localized);
	const char** FS_ListFilteredFilesWrapper(const char* path /*edx*/, const char* null /*esi*/, int* file_count);
	std::uint32_t FS_HashFileName(const char* fname, int hash_size);
	void* Hunk_FindDataForFileInternal(int hash /*eax*/, int data_type /*ebx*/, const char* name /*edi*/);
	int FS_OpenFileOverwrite(const char* path /*esi*/);
	inline auto FS_Write = reinterpret_cast<int (*)(const void* buffer, size_t len, int h)>(0x49FF10);

	inline auto Image_Alloc = reinterpret_cast<game::GfxImage* (*)(const char* name, char category, char semantic, char imageTrack)>(0x5128B0);
	inline auto R_LoadJpeg = reinterpret_cast<bool (*)(game::GfxImage * img, const char* img_path)>(0x54FD40);
	inline auto R_ReloadImage = reinterpret_cast<bool (*)(game::GfxImage* img)>(0x513490); // release and reload (iwi only)
	inline auto Image_Release = reinterpret_cast<void (*)(game::GfxImage* img)>(0x5125D0); // release img
	game::GfxImage* Image_FindExisting(const char* name);
	game::GfxImage* Image_RegisterHandle(const char* name);
	void Material_Add(int idx/*eax*/, game::Material* material/*esi*/);
	inline auto Material_Load = reinterpret_cast<game::Material* (*)(const char* name, int img_track)>(0x51B690);
	inline auto Material_GetHashIndex = reinterpret_cast<void (*)(const char* name, std::uint16_t * index, bool* exists)>(0x510E10);
	inline auto Material_RegisterHandle = reinterpret_cast<game::Material* (*)(const char* name, int)>(0x511BE0);
	inline auto R_RegisterModel = reinterpret_cast<game::XModel* (*)(const char* name)>(0x51D450);
	inline auto R_RegisterLightDef = reinterpret_cast<game::GfxLightDef* (*)(const char* name)>(0x53D510);
	inline auto R_RegisterFont = reinterpret_cast<game::Font_s* (*)(const char* name, int one)>(0x511ED0);
	inline auto R_HashAssetName = reinterpret_cast<int (*)(const char* name)>(0x528D50);

	PhysPreset* FX_RegisterPhysPreset(const char* name);
	void DObjCreate(game::DObjModel_s* dobjModels /*edi*/, game::DObj_s* obj /*esi*/, size_t numModels, game::XAnimTree_s* tree, int entnum);

	// *
	// * --------------------- fx ------------------------------

	extern int& g_processCodeMesh;

	extern int I_strncmp(const char* s0, const char* s1, int n);
	extern int I_strcmp(const char* s1, const char* s2);
	extern void I_strncpyz(char* Dest, const char* Source, int destsize);
	inline auto I_stricmp = reinterpret_cast<int (*)(const char* a, const char* b)>(0x4B9490);

	inline auto Com_BeginParseSession = reinterpret_cast<void (*)(const char*)>(0x4B78D0);
	inline auto Com_EndParseSession = reinterpret_cast<void (*)()>(0x4B79A0);
	inline auto Com_SetSpaceDelimited = reinterpret_cast<void (*)(int)>(0x4B79D0);
	inline auto Com_SetParseNegativeNumbers = reinterpret_cast<void (*)()>(0x4B7A30);
	inline auto Com_Parse = reinterpret_cast<char* (*)(const char**)>(0x4B8390);
	inline auto Com_MatchToken = reinterpret_cast<int (*)(const char**, const char*, int)>(0x4B8430);
	inline auto Com_UngetToken = reinterpret_cast<void (*)()>(0x4B7C90);
}
