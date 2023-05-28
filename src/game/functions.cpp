#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace game
{
	namespace glob
	{
		// init
		bool command_thread_running;
		std::vector<std::string> loadedModules;

		bool radiant_initiated;
		bool radiant_config_loaded;
		bool radiant_config_not_found;
		float frames_per_second;
		bool ccamwindow_realtime;

		// misc
		game::TrackWorldspawn track_worldspawn = game::TrackWorldspawn();

		bool debug_sundir = false;
		game::vec3_t debug_sundir_startpos = {};
		float debug_sundir_length = 500.0f;

		bool is_loading_map = false;
		bool in_shutdown = false;


		// update check
		std::string gh_update_releases_json;
		std::string gh_update_tag;
		std::string gh_update_date;
		std::string gh_update_title;
		std::string gh_update_desc;
		std::string gh_update_link;
		std::string gh_update_zip_name;
		bool gh_update_avail = false;

		// live-link
		game::ProcessServerCommands cServerCmd = game::ProcessServerCommands();
		bool live_connected;

		// renderer
		IDirect3DDevice9* d3d9_device = nullptr;

	}

	game::vec3_t vec3_origin = { 0.0f, 0.0f, 0.0f };
	game::vec4_t color_white = { 1.0f, 1.0f, 1.0f, 1.0f };

	IDirect3DTexture9* framebuffer_test = nullptr;

	// radiant globals
	int& g_nScaleHow = *reinterpret_cast<int*>(0x23F16DC);
	game::qeglobals_t* g_qeglobals = reinterpret_cast<game::qeglobals_t*>(0x25F39C0);

	float& g_zoomLevel = *reinterpret_cast<float*>(0x25D5A90);
	int* g_nUpdateBitsPtr = reinterpret_cast<int*>(0x25D5A74);
	int& g_nUpdateBits = *reinterpret_cast<int*>(0x25D5A74);
	bool& g_bScreenUpdates = *reinterpret_cast<bool*>(0x739B0F);
	double& g_time = *reinterpret_cast<double*>(0x2665678);
	double& g_oldtime = *reinterpret_cast<double*>(0x2665670);
	bool& g_region_active = *reinterpret_cast<bool*>(0x23F1744);
	bool& g_bCrossHairs = *reinterpret_cast<bool*>(0x25D5B06);
	bool& g_bClipMode = *reinterpret_cast<bool*>(0x23F16D8);
	bool& g_bRotateMode = *reinterpret_cast<bool*>(0x23F16D9);
	bool& g_bScaleMode = *reinterpret_cast<bool*>(0x23F16DA);
	int& g_nLastLen = *reinterpret_cast<int*>(0x25D5B14);

	float* g_vRotateOrigin = reinterpret_cast<float*>(0x23F1658);
	int& g_prefab_stack_level = *reinterpret_cast<int*>(0x25D5B34);

	bool& g_bDoCone = *reinterpret_cast<bool*>(0x25D5B38);
	bool& g_bDoSphere = *reinterpret_cast<bool*>(0x25D5B39);

	game::SCommandInfo* g_Commands = reinterpret_cast<game::SCommandInfo*>(0x73B240);
	int		g_nCommandCount = 187;

	const char* current_map_filepath = reinterpret_cast<const char*>(0x23F18D8);

	game::filter_material_t* filter_surfacetype_array = reinterpret_cast<game::filter_material_t*>(0x73AF80);
	game::filter_material_t* filter_locale_array = reinterpret_cast<game::filter_material_t*>(0x73A780);
	game::filter_material_t* filter_usage_array = reinterpret_cast<game::filter_material_t*>(0x739F80);
	std::uint8_t& texWndGlob_surfaceTypeFilter = *reinterpret_cast<std::uint8_t*>(0x25D799E); // current selected index
	std::uint8_t& texWndGlob_localeFilter = *reinterpret_cast<std::uint8_t*>(0x25D799D); // current selected index
	std::uint8_t& texWndGlob_usageFilter = *reinterpret_cast<std::uint8_t*>(0x25D799C); // current selected index
	int& texWndGlob_localeCount = *reinterpret_cast<int*>(0x25D7998); // amount of loaded locale filters
	int& texWndGlob_usageCount = *reinterpret_cast<int*>(0x25D7994); // amount of loaded usage filters

	bool& r_initiated = *reinterpret_cast<bool*>(0x25D5A68);
	game::GfxBackEndData* gfx_frontend_data = reinterpret_cast<game::GfxBackEndData*>(0x73D480);
	game::GfxBackEndData* gfx_backend_data = reinterpret_cast<game::GfxBackEndData*>(0x73D500);

	game::GfxCmdBufSourceState* gfxCmdBufSourceState = reinterpret_cast<game::GfxCmdBufSourceState*>(0x174D760);
	game::GfxCmdBufState* gfxCmdBufState = reinterpret_cast<game::GfxCmdBufState*>(0x174E660);
	game::GfxCmdBufInput* gfxCmdBufInput = reinterpret_cast<game::GfxCmdBufInput*>(0x174F070);

	game::r_globals_t* rg = reinterpret_cast<game::r_globals_t*>(0x13683F0);
	game::r_global_permanent_t* rgp = reinterpret_cast<game::r_global_permanent_t*>(0x136C700);
	game::GfxScene* scene = reinterpret_cast<game::GfxScene*>(0x1370980);
	game::DxGlobals* dx = reinterpret_cast<game::DxGlobals*>(0x1365684);
	game::GfxImage** imageGlobals = reinterpret_cast<game::GfxImage**>(0x14C6CF8);

	game::ComWorld* comworld = reinterpret_cast<game::ComWorld*>(0x241DDC8);
	game::GfxWorld* s_world = reinterpret_cast<game::GfxWorld*>(0x174F688);

	HWND* entitywnd_hwnds = reinterpret_cast<HWND*>(0x240A118);

	CLayerDlg* layer_dlg = reinterpret_cast<CLayerDlg*>(0x25d6ea0);

	game::GfxBackEndData* get_backenddata()
	{
		const auto out = reinterpret_cast<game::GfxBackEndData*>(*game::backEndDataOut_ptr);
		return out;
	}

	game::GfxBackEndData* get_frontenddata()
	{
		const auto out = reinterpret_cast<game::GfxBackEndData*>(*game::frontEndDataOut_ptr);
		return out;
	}

	game::GfxCmdArray* get_cmdlist()
	{
		const auto out = reinterpret_cast<game::GfxCmdArray*>(*game::s_cmdList_ptr);
		return out;
	}

	game::entity_s* g_world_entity()
	{
		const auto ent = reinterpret_cast<game::entity_s*>(*game::worldEntity_ptr);
		return ent;
	}

	game::selbrush_def_t* g_active_brushes()
	{
		const auto brush = reinterpret_cast<game::selbrush_def_t*>(*game::active_brushes_ptr);
		return brush;
	}

	game::selbrush_def_t* g_active_brushes_next()
	{
		const auto brush = reinterpret_cast<game::selbrush_def_t*>(*game::active_brushes_next_ptr);
		return brush;
	}

	game::selbrush_def_t* g_selected_brushes()
	{
		const auto brush = reinterpret_cast<game::selbrush_def_t*>(*game::currSelectedBrushes);
		return brush;
	}

	game::selbrush_def_t* g_selected_brushes_next()
	{
		const auto brush = reinterpret_cast<game::selbrush_def_t*>(*(DWORD*)0x23F1868);
		return brush;
	}

	game::entity_s* get_entity_insts()
	{
		const auto entity = reinterpret_cast<game::entity_s*>(*(DWORD*)0x23F1748);
		return entity;
	}

	game::entity_inst_s* get_entity_insts_next()
	{
		const auto entity = reinterpret_cast<game::entity_inst_s*>(*(DWORD*)0x23F174C);
		return entity;
	}

	const int& g_selected_faces_count = *reinterpret_cast<const int*>(0x73C714);

	// g_selected_faces is a CArray
	game::selface_t* g_selected_faces()
	{
		const auto selfaces = reinterpret_cast<game::selface_t*>(*(DWORD*)0x73C710);
		return selfaces;
	}

	// pot. unsafe, do not use in loops
	game::entity_s_def* g_edit_entity()
	{
		const auto ent = reinterpret_cast<game::entity_s_def*>(*(DWORD*)0x240A108);
		return ent;
	}

	int& multiple_edit_entities = *reinterpret_cast<int*>(0x240A10C);

	game::eclass_t* g_eclass()
	{
		const auto eclass = reinterpret_cast<game::eclass_t*>(*(DWORD*)0x25D5B20);
		return eclass;
	}

	CSurfaceDlg* get_surfacedialog()
	{
		const auto g_dlgSurface = reinterpret_cast<CSurfaceDlg*>(0x25D7668);
		return g_dlgSurface;
	}

	CPrefsDlg* g_PrefsDlg()
	{
		const auto prefs = reinterpret_cast<CPrefsDlg*>(*(DWORD*)0x73C704);
		return prefs;
	}

	void CPrefsDlg_SavePrefs()
	{
		const static uint32_t SavePrefs_addr = 0x44F280;
		__asm
		{
			call	game::g_PrefsDlg;
			mov		esi, eax;
			call	SavePrefs_addr;
		}
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

	int& g_undoMaxSize = *reinterpret_cast<int*>(0x739F6C);
	int& g_undoId = *reinterpret_cast<int*>(0x739F74);
	int& g_undoSize = *reinterpret_cast<int*>(0x25D5B18);

	void Undo_GeneralStart(const char* operation /*eax*/)
	{
//#ifdef DEBUG
//		game::printf_to_console("Undo_GeneralStart :: %s", operation);
//#endif

		const static uint32_t func_addr = 0x45E3F0;
		__asm
		{
			pushad;
			mov		eax, operation;
			call	func_addr;
			popad;
		}
	}

	// game::currSelectedBrushes
	void Undo_AddBrushList(void* sb)
	{
#ifdef DEBUG
		game::printf_to_console("Undo_AddBrushList");
#endif

		const static uint32_t func_addr = 0x45E7C0;
		__asm
		{
			pushad;
			mov		edi, sb;
			call	func_addr;
			popad;
		}
	}

	void Undo_AddBrushList_Selected()
	{
		game::Undo_AddBrushList(game::currSelectedBrushes);
	}

	// game::currSelectedBrushes
	void Undo_EndBrushList(void* sb)
	{
#ifdef DEBUG
		game::printf_to_console("Undo_EndBrushList");
#endif

		const static uint32_t func_addr = 0x45E870;
		__asm
		{
			pushad;
			mov		esi, sb;
			call	func_addr;
			popad;
		}
	}

	void Undo_EndBrushList_Selected()
	{
		game::Undo_EndBrushList(game::currSelectedBrushes);
	}

	void Undo_AddEntity_W(game::entity_s* ent /*eax*/)
	{
		const static uint32_t func_addr = 0x45E990;
		__asm
		{
			pushad;
			mov		eax, ent;
			call	func_addr;
			popad;
		}
	}

	void Undo_SetIdForEntity(game::entity_s* ent /*edx*/)
	{
		const static uint32_t func_addr = 0x45E9E0;
		__asm
		{
			pushad;
			mov		edx, ent;
			call	func_addr;
			popad;
		}
	}

	bool is_single_brush_selected(bool print_warning)
	{
		if ((DWORD*)g_selected_brushes_next() == game::currSelectedBrushes || (DWORD*)g_selected_brushes_next()->next != game::currSelectedBrushes)
		{
			if (print_warning)
			{
				game::printf_to_console("Error: you must have a single brush selected");
			}

			return false;
		}

		if (auto n = g_selected_brushes_next(); n && n->owner && n->owner->firstActive && n->owner->firstActive->eclass->fixedsize)
		{
			if (print_warning)
			{
				game::printf_to_console("Error: you cannot manipulate fixed size entities");
			}

			return false;
		}

		return true;
	}

	bool is_any_brush_selected()
	{
		return (DWORD*)game::g_selected_brushes_next() != game::currSelectedBrushes;
	}

	void Texwnd_SelectMaterial(int x, int y /*ecx*/)
	{
		const static uint32_t func_addr = 0x45C520;
		__asm
		{
			pushad;
			mov		ecx, y;
			push	x;
			call	func_addr;
			add		esp, 4;
			popad;
		}
	}

	void DeleteKey(game::epair_t*& epair /*eax*/, const char* key /*ebx*/)
	{
		const static uint32_t func_addr = 0x483720;
		__asm
		{
			pushad;
			mov		esi, epair;
			lea		eax, [esi];
			mov		ebx, key;
			call	func_addr;
			popad;
		}
	}

	void Checkkey_Model(entity_s* ent /*esi*/, const char* key)
	{
		const static uint32_t func_addr = 0x482F70;
		__asm
		{
			pushad;
			push	key;
			mov		esi, ent;
			call	func_addr;
			add     esp, 4;
			popad;
		}
	}

	void Checkkey_Color(entity_s* ent /*eax*/, const char* key /*ebx*/)
	{
		const static uint32_t func_addr = 0x483210;
		__asm
		{
			pushad;
			mov		ebx, key;
			mov		eax, ent;
			call	func_addr;
			popad;
		}
	}

	// arbitary rotate selection around axis (xyz)
	void selection_rotate_axis(int axis, int deg)
	{
		class rot_helper
		{
		public:
			char pad[16];
			int deg;
			int unused;

			rot_helper(int deg)
			{
				this->deg = deg;
				this->unused = 0;
			}
		}; STATIC_ASSERT_OFFSET(rot_helper, deg, 0x10);

		rot_helper helper(deg);

		switch (axis)
		{
		case 0:
			utils::hook::call<void(__stdcall)(rot_helper*, int*)>(0x450EF0)(&helper, &helper.unused);
			break;

		case 1:
			utils::hook::call<void(__stdcall)(rot_helper*, int*)>(0x450F80)(&helper, &helper.unused);
			break;

		case 2:
			utils::hook::call<void(__stdcall)(rot_helper*, int*)>(0x451010)(&helper, &helper.unused);
			break;
		}
	}

	void Select_ApplyMatrix_SelectedBrushes(bool snap /*ebx*/, float* rotate_axis, float degree, int unk)
	{
		const static uint32_t func_addr = 0x48FD10;
		__asm
		{
			pushad;

			push	unk;
			push	degree;
			push	rotate_axis;
			mov     bl, snap;
			call	func_addr;
			add		esp, 12;

			popad;
		}
	}

	void Select_ApplyMatrix(float* rotate_axis /*eax*/, void* brush, int snap, float degree, int unk /*bool*/)
	{
		const static uint32_t func_addr = 0x47CDE0;
		__asm
		{
			pushad;
			push	unk;
			push	0; // ecx?

			fld		degree;
			fstp    dword ptr[esp];

			push	snap;
			push	brush;

			mov		eax, rotate_axis; // lea

			call	func_addr;
			add		esp, 16;
			popad;
		}
	}

	void Select_RotateAxis(int axis /*eax*/, float degree, float* rotate_axis)
	{
		const static uint32_t func_addr = 0x48FF40;
		__asm
		{
			pushad;
			push	rotate_axis;
			push	0; // ecx?

			mov		eax, axis;

			fld		degree;
			fstp    dword ptr[esp];

			call	func_addr;
			add		esp, 8;
			popad;
		}
	}

	// turn on g_PrefsDlg->m_bNoClamp before calling to calc. midpoint without snapping
	void Select_GetMid(float* midpoint /*esi*/)
	{
		const static uint32_t func_addr = 0x48FC70;
		__asm
		{
			pushad;
			mov		esi, midpoint;
			call	func_addr;
			popad;
		}
	}

	// not sure about the name or what it does (use brush_select to select groups)
	void Select_FuncGroup(float* trace_dir /*ecx*/, float* trace_start /*edx*/, int contents /*eax*/)
	{
		const static uint32_t func_addr = 0x48E340;
		__asm
		{
			pushad;

			mov		eax, contents;
			mov		edx, trace_start;
			mov		ecx, trace_dir;
			call	func_addr;
			popad;
		}
	}

	void Select_RotateFixedSize(game::entity_s* owner, brush_t_with_custom_def* def, float(*mid_point)[3])
	{
		if (!def->owner)
		{
			def->owner = owner->firstActive;
			imgui::Toast(ImGuiToastType_Warning, "dynamic prefabs - Select_RotateFixedSize", "trying to fix brush with 'def->owner = owner->firstActive'");
		}

		float offset_axis[3][3];
		float axis[3][3];
		float rot_trans[3];
		float origin[3];
		float move_delta[3];

		entity_s* first_active = owner->firstActive;
		move_delta[0] = first_active->origin[0] - (*mid_point)[0];
		move_delta[1] = first_active->origin[1] - (*mid_point)[1];
		move_delta[2] = first_active->origin[2] - (*mid_point)[2];

		if (utils::vector::length_squared(move_delta) != 0.0f)
		{
			utils::vector::vec3_rotate_transpose(move_delta, &(*mid_point)[3], rot_trans);
			utils::vector::subtract(rot_trans, move_delta, move_delta);
			game::Brush_Move(move_delta, def, true);
		}

		const auto egui = GET_GUI(ggui::entity_dialog);
		if (!egui->get_vec3_for_key_from_entity(first_active, origin, "angles"))
		{
			origin[0] = 0.0f;
			origin[1] = 0.0f;
			origin[2] = 0.0f;
		}

		AnglesToAxis(origin, axis[0]);
		utils::vector::matrix_multiply(&(*mid_point)[3], axis[0], offset_axis[0]);
		AxisToAngles(offset_axis, origin); // offset_axis

		SetKeyValue(first_active, "angles", utils::va("%g %g %g", origin[0], origin[1], origin[2]));

		Brush_BuildWindings(def, true);

		/*if (g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge)
		{
			SetupVertexSelection();
		}

		MarkMapModified();*/

		def->version++;
	}

	void SetSpawnFlags(int flag)
	{
		const static uint32_t func_addr = 0x496F00;
		__asm
		{
			pushad;
			mov		ebx, flag;
			call	func_addr;
			popad;
		}
	}

	// change material of the current selection (only tested on a single patch)
	void SetMaterial(const char* name /*edi*/, game::patchMesh_material* def /*esi*/)
	{
		const static uint32_t func_addr = 0x4315C0;
		__asm
		{
			pushad;
			mov		esi, def;
			mov     edi, name;
			call	func_addr;
			popad;
		}
	}

	void UpdateSel(int wParam, game::eclass_t* e_class)
	{
		const static uint32_t func_addr = 0x497180;
		__asm
		{
			pushad;
			mov		ecx, wParam;
			mov		eax, e_class;
			call	func_addr;
			popad;
		}
	}

	bool FilterBrush(game::selbrush_def_t* sb /*esi*/, int always_null)
	{
		const static uint32_t func_addr = 0x46A1F0;
		auto return_value = false;
		__asm
		{
			pushad;
			push	always_null;
			mov		esi, sb;
			call	func_addr;
			add		esp, 4;
			mov		return_value, al;
			popad;
		}

		return return_value;
	}

	// select a complete row of a selected patch
	void Patch_SelectRow(int row /*eax*/, game::patchMesh_t* p /*edi*/, int multi)
	{
		const static uint32_t func_addr = 0x43C710;
		__asm
		{
			pushad;
			push	multi;
			mov		eax, row;
			mov		edi, p;
			call	func_addr;
			add		esp, 4;
			popad;
		}
	}

	// update / rebuild the patch (also visually)
	void Patch_UpdateSelected(game::patchMesh_t* p /*esi*/, int always_true)
	{
		const static uint32_t func_addr = 0x438D80;
		__asm
		{
			pushad;
			push	always_true;
			mov		esi, p;
			call	func_addr;
			add     esp, 4;
			popad;
		}
	}

	void Patch_SetTextureInfo(game::texdef_sub_t* texdef /*ebx*/)
	{
		const static uint32_t func_addr = 0x447760;
		__asm
		{
			pushad;
			mov		ebx, texdef;
			call	func_addr;
			popad;
		}
	}

	void Patch_ShiftTexture(game::patchMesh_t* def, float shift_horz, float shift_vert)
	{
		const static uint32_t func_addr = 0x446170;
		__asm
		{
			pushad;
			sub		esp, 8;

			mov		edi, def;

			fld		shift_vert;
			fstp    dword ptr[esp + 4];

			fld		shift_horz;
			fstp    dword ptr[esp];

			call	func_addr;
			add		esp, 8;
			popad;
		}
	}

	// no checks, no undo
	void Patch_Lightmap_Texturing_dirty(game::patchMesh_t* p /*esi*/)
	{
		const static uint32_t func_addr = 0x4397B0;
		__asm
		{
			pushad;
			mov		esi, p;
			call	func_addr;
			popad;
		}
	}

	// calculate mins and maxs for the given patch
	void Patch_CalcBounds(game::patchMesh_t* p, game::vec3_t& vMin, game::vec3_t& vMax)
	{
		vMin[0] = vMin[1] = vMin[2] = 99999.0f;
		vMax[0] = vMax[1] = vMax[2] = -99999.0f;

		//p->bDirty = true;
		for (int w = 0; w < p->width; w++)
		{
			for (int h = 0; h < p->height; h++)
			{
				for (int j = 0; j < 3; j++)
				{
					const float f = p->ctrl[w][h].xyz[j];
					if (f < vMin[j])
					{
						vMin[j] = f;
					}

					if (f > vMax[j])
					{
						vMax[j] = f;
					}
				}
			}
		}
	}

	void Select_GetBounds(float* mins, float* maxs)
	{
		mins[0] = 131072.0f;
		mins[1] = 131072.0f;
		mins[2] = 131072.0f;
		maxs[0] = -131072.0f;
		maxs[1] = -131072.0f;
		maxs[2] = -131072.0f;

		FOR_ALL_SELECTED_BRUSHES(sb)
		{
			utils::vector::clamp_vec3(sb->def->mins, sb->def->maxs, mins, maxs);
		}
	}

	// insert/delete, column/row
	void Patch_Adjust(game::patchMesh_t* p, bool insert, bool column, bool flag)
	{
		if (insert)
		{
			if (column)
			{
				game::Patch_InsertColumn(p, flag);
			}
			else
			{
				game::Patch_InsertRow(p, flag);
			}
		}
		else if (column)
		{
			game::Patch_RemoveColumn(p, flag);
		}
		else
		{
			game::Patch_RemoveRow(p, flag);
		}
	}

	game::patchMesh_t* Patch_Duplicate(game::patchMesh_t* p /*edi*/)
	{
		const static uint32_t func_addr = 0x4486A0;
		__asm
		{
			mov		edi, p;
			call	func_addr;
		}
	}

	game::selbrush_def_t* Patch_Cap(patchMesh_t* pm /*ecx*/, int bByColumn, int bFirst)
	{
		const static uint32_t func_addr = 0x439C00;
		__asm
		{
			push	bFirst;
			push	bByColumn;
			mov		ecx, pm;
			call	func_addr;
			add		esp, 8;
		}
	}

	game::selbrush_def_t* Patch_CapSpecial(patchMesh_t* pm /*ecx*/, int nType, int bFirst)
	{
		const static uint32_t func_addr = 0x43A170;
		__asm
		{
			push	bFirst;
			push	nType;
			mov		ecx, pm;
			call	func_addr;
			add		esp, 8;
		}
	}

	void Patch_Invert(game::patchMesh_t* p /*ebx*/)
	{
		const static uint32_t func_addr = 0x446480;
		__asm
		{
			mov		ebx, p;
			call	func_addr;
		}
	}

	void Patch_Rebuild(game::patchMesh_t* p /*esi*/, int reset)
	{
		const static uint32_t func_addr = 0x438D80;
		__asm
		{
			push	reset;
			mov		esi, p;
			call	func_addr;
			add		esp, 4;
		}
	}

	// flag :: [true] first column, [false] last column
	void Patch_InsertColumn(patchMesh_t* p, bool flag /*cl*/)
	{
		const static uint32_t func_addr = 0x443410;
		__asm
		{
			mov		cl, flag;
			push	p;
			call	func_addr;
			add		esp, 4;
		}
	}

	// flag :: [true] first row, [false] last row
	void Patch_InsertRow(patchMesh_t* p, bool flag /*dl*/)
	{
		const static uint32_t func_addr = 0x443830;
		__asm
		{
			mov		dl, flag;
			push	p;
			call	func_addr;
			add		esp, 4;
		}
	}

	// flag :: [true] first column, [false] last column
	void Patch_RemoveColumn(patchMesh_t* p /*eax*/, bool flag /*cl*/)
	{
		const static uint32_t func_addr = 0x443C90;
		__asm
		{
			mov		cl, flag;
			mov		eax, p;
			call	func_addr;
		}
	}

	// flag :: [true] first row, [false] last row
	void Patch_RemoveRow(patchMesh_t* p /*eax*/, bool flag /*cl*/)
	{
		const static uint32_t func_addr = 0x443B60;
		__asm
		{
			mov		cl, flag;
			mov		eax, p;
			call	func_addr;
		}
	}

	void Brush_Move(const float* delta, game::brush_t_with_custom_def* def, int snap)
	{
		const static uint32_t func_addr = 0x47BA40;
		__asm
		{
			pushad;
			push	snap;
			push	def;
			mov		ebx, delta;
			call	func_addr;
			add     esp, 8;
			popad;
		}
	}

	int Brush_MoveVertex(const float* delta /*eax*/, game::brush_t_with_custom_def* def, float* move_points, float* end)
	{
		const static uint32_t func_addr = 0x471C30;
		__asm
		{
			//pushad;
			push	end;
			push	move_points;
			push	def;
			mov		eax, delta;
			call	func_addr;
			add     esp, 12;
			//popad;
		}
	}

	void Brush_Create(float* maxs /*edx*/, float* mins /*ecx*/, game::brush_t_with_custom_def* brush, int unk)
	{
		const static uint32_t func_addr = 0x475300;
		__asm
		{
			pushad;

			push	unk;
			push	brush;
			mov		ecx, mins;
			mov		edx, maxs;

			call	func_addr;
			add		esp, 8;

			popad;
		}
	}

	void Brush_BuildWindings(game::brush_t_with_custom_def* brush /*ecx*/, int snap)
	{
		const static uint32_t func_addr = 0x477AC0;
		__asm
		{
			pushad;
			push	snap;
			mov		ecx, brush;
			call	func_addr;
			add		esp, 4;
			popad;
		}
	}

	void Entity_LinkBrush(game::brush_t_with_custom_def* brush /*eax*/, game::entity_s* world /*edi*/)
	{
		const static uint32_t func_addr = 0x484FC0;
		__asm
		{
			pushad;

			mov		edi, world;
			mov		eax, brush;

			call	func_addr;

			popad;
		}
	}

	game::brush_t_with_custom_def* Brush_AddToList(game::brush_t_with_custom_def* brush /*eax*/, game::entity_s* world)
	{
		const static uint32_t func_addr = 0x475980;
		__asm
		{
			//pushad;
			push	world;
			mov		eax, brush;

			call	func_addr;
			add		esp, 4;
			//popad;
		}
	}

	void Brush_AddToList2(game::brush_t_with_custom_def* brush /*eax*/)
	{
		const static uint32_t func_addr = 0x4765A0;
		__asm
		{
			pushad;
			mov		eax, brush;
			call	func_addr;
			popad;
		}
	}

	void Brush_RemoveFromList(game::selbrush_def_t* brush /*eax*/)
	{
		const static uint32_t func_addr = 0x476680;
		__asm
		{
			pushad;
			mov		eax, brush;
			call	func_addr;
			popad;
		}
	}

	void Brush_Deselect(game::brush_t* b /*esi*/)
	{
		const static uint32_t func_addr = 0x48DC60;
		__asm
		{
			pushad;
			mov		esi, b;
			call	func_addr;
			popad;
		}
	}

	/**
	 * @param b							brush to select
	 * @param select_connected			select connected brushes (func_group, brush-model etc) 
	 * @param update_status				legacy, not needed
	 * @param center_grid_on_selection	move grid view to center of selection
	 */
	void Brush_Select(game::selbrush_def_t* b /*ecx*/, bool select_connected, bool update_status, bool center_grid_on_selection)
	{
		const int overwrite = select_connected;
		const int status = update_status;
		const int center_grid = center_grid_on_selection;

		const static uint32_t func_addr = 0x48DCC0;
		__asm
		{
			pushad;
			mov		ecx, b;
			push	center_grid;
			push	status;
			push	overwrite;
			call	func_addr;
			add		esp, 12;
			popad;
		}
	}

	// https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/q3radiant/PMESH.CPP#L3002
	bool Patch_DragScale(game::patchMesh_t* pm, const float* bounds /*eax*/, const float* dist_vec)
	{
		bool return_value = false;
		const static uint32_t func_addr = 0x442B90;

		__asm
		{
			pushad;
			push	dist_vec;
			mov		eax, bounds;
			push	pm;
			call	func_addr;
			add		esp, 8;
			mov		return_value, al;
			popad;
		}

		return return_value;
	}

	// used to select brush faces (vertices) that are going to be extruded when dragging next to a selected brush (g_qeglobals.d_num_move_points)
	void Brush_SideSelect(game::brush_t_with_custom_def* def, const float* trace_start /*eax*/, const float* trace_dir, int shear)
	{
		const static uint32_t func_addr = 0x4777D0;
		__asm
		{
			pushad;
			push	shear;
			push	trace_dir;
			mov		eax, trace_start;
			push	def;
			call	func_addr;
			add		esp, 12;
			popad;
		}
	}

	int CM_ForEachBrushPlaneIntersection(game::brush_t_with_custom_def* b /*esi*/, game::BrushPt_t* brush_pts)
	{
		const static uint32_t func_addr = 0x470880;
		__asm
		{
			push	brush_pts;
			mov		esi, b;
			call	func_addr;
			add     esp, 4;
		}
	}

	void CreateEntityBrush(int height /*eax*/, int x /*ecx*/, void* wnd)
	{
		const static uint32_t func_addr = 0x466290;
		__asm
		{
			pushad;
			push	wnd;
			mov		ecx, x;
			mov		eax, height;
			call	func_addr;
			add     esp, 4;
			popad;
		}
	}

	game::entity_s* Entity_Create(eclass_t* eclass /*eax*/)
	{
		const static uint32_t func_addr = 0x484980;
		__asm
		{
			mov		eax, eclass;
			call	func_addr;
		}
	}

	game::eclass_t* Eclass_ForName(const char* name /*ecx*/, int has_brushes)
	{
		const static uint32_t func_addr = 0x482190;
		__asm
		{
			mov		ecx, name;
			push	has_brushes;
			call	func_addr;
			add     esp, 4;
		}
	}

	// also adds an undo
	void CreateEntityFromClassname(void* cxywnd /*edi*/, const char* name /*esi*/, int x, int y)
	{
		const static uint32_t func_addr = 0x466480;
		__asm
		{
			pushad;
			push	y;
			push	x;
			mov		esi, name;
			mov		edi, cxywnd;
			call	func_addr;
			add     esp, 8;
			popad;
		}
	}

	game::trace_t* Trace_AllDirectionsIfFailed(float* cam_origin /*ebx*/, void* trace_result, float* dir, int contents)
	{
		const static uint32_t func_addr = 0x48DAA0;
		__asm
		{
			mov		ebx, cam_origin;
			push	contents;
			push	dir;
			push	trace_result;
			call	func_addr;
			add     esp, 12;
		}
	}

	void map_load_from_file(const char* path)
	{
		const static uint32_t func_addr = 0x486680;
		__asm
		{
			pushad;
			mov		ecx, path;
			call	func_addr;
			popad;
		}
	}

	void map_save_file(const char* path /*ecx*/, int is_reg, int save_to_perforce)
	{
		const static uint32_t func_addr = 0x486C00;
		__asm
		{
			pushad;
			mov		ecx, path;
			push	save_to_perforce;
			push	is_reg;
			call	func_addr;
			add		esp, 8;
			popad;
		}
	}

	void map_write_selection(const char* path)
	{
		const static uint32_t func_addr = 0x488EB0;
		__asm
		{
			pushad;
			mov		edi, path;
			call	func_addr;
			popad;
		}
	}

	void mru_new_item(game::LPMRUMENU* mru, const char* item_str)
	{
		const static uint32_t func_addr = 0x48A2C0;
		__asm
		{
			pushad;
			mov		esi, mru;
			push	item_str;
			call	func_addr;
			add     esp, 4;
			popad;
		}
	}

	void mru_insert_item(game::LPMRUMENU* mru, HMENU menu)
	{
		const static uint32_t func_addr = 0x48A400;
		__asm
		{
			pushad;
			mov		edi, mru;
			push	menu;
			call	func_addr;
			add     esp, 4;
			popad;
		}
	}


	// *
	// * --------------------- renderer ------------------------------

	void R_DrawSelectionbox(const float* verts)
	{
		const static uint32_t func_addr = 0x40CC50;
		__asm
		{
			pushad;
			mov		esi, verts;
			call	func_addr;
			popad;
		}
	}

	game::GfxCmdHeader* R_GetCommandBuffer(std::uint32_t bytes /*ebx*/, int render_cmd /*edi*/)
	{
		const static uint32_t R_RenderBufferCmdCheck_func = 0x4FAEB0;
		__asm
		{
			mov		ebx, bytes;
			mov		edi, render_cmd;
			call	R_RenderBufferCmdCheck_func;
		}
	}

	void R_AddCmdSetViewportValues(int x, int y, int width, int height)
	{
		if (width <= 0) {
			game::Com_Error("R_AddCmdSetViewportValues :: width");
		}

		if (height <= 0) {
			game::Com_Error("R_AddCmdSetViewportValues :: height");
		}

		// RC_SET_VIEWPORT
		if (auto cmd = reinterpret_cast<game::GfxCmdSetViewport*>(game::R_GetCommandBuffer(20, 7));
			cmd)
		{
			cmd->viewport.height = height;
			cmd->viewport.x = x;
			cmd->viewport.y = y;
			cmd->viewport.width = width;
		}
		else
		{
			game::Com_Error("R_AddCmdSetViewportValues :: cmd");
		}
	}

	void R_SetD3DPresentParameters(_D3DPRESENT_PARAMETERS_* d3dpp, game::GfxWindowParms* wnd, [[maybe_unused]] int window_count)
	{
		ASSERT_MSG(d3dpp, "invalid D3DPRESENT_PARAMETERS d3dpp");
		ASSERT_MSG(wnd, "invalid GfxWindowParms wnd");
		ASSERT_MSG(wnd->hwnd, "invalid HWND wnd->hwnd");

		//R_SetupAntiAliasing(wnd, window_count);
		memset(d3dpp, 0, sizeof(_D3DPRESENT_PARAMETERS_));
		d3dpp->BackBufferWidth = wnd->displayWidth;
		d3dpp->BackBufferHeight = wnd->displayHeight;
		d3dpp->BackBufferFormat = D3DFMT_A8R8G8B8;
		d3dpp->BackBufferCount = 1;
		d3dpp->MultiSampleType = _D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE; // (D3DMULTISAMPLE_TYPE)game::dx->multiSampleType;
		d3dpp->MultiSampleQuality = 0; // game::dx->multiSampleQuality
		d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp->EnableAutoDepthStencil = 1; // test
		d3dpp->AutoDepthStencilFormat = static_cast<D3DFORMAT>(game::dx->depthStencilFormat);
		d3dpp->PresentationInterval = 0x80000000; //r_vsync->current.enabled ? 1 : 0x80000000;
		d3dpp->hDeviceWindow = wnd->hwnd;
		d3dpp->Flags = 0;

		if (wnd->fullscreen)
		{
			d3dpp->Windowed = 0;
			d3dpp->FullScreen_RefreshRateInHz = wnd->hz;
		}
		else
		{
			d3dpp->FullScreen_RefreshRateInHz = 0;
			d3dpp->Windowed = 1;
		}
	}

	void R_Hwnd_Resize(HWND__* hwnd, int display_width, int display_height)
	{
		ASSERT_MSG(hwnd, "invalid hwnd");

		_D3DPRESENT_PARAMETERS_ d3dpp{};
		game::GfxWindowParms wnd{};

		if (display_width && display_height)
		{
			if (game::dx->windowCount > 0)
			{
				int wnd_count = 0;
				for (auto i = components::renderer::windows; i->hwnd != hwnd; ++i)
				{
					if (++wnd_count >= game::dx->windowCount)
					{
						return;
					}
				}

				wnd.hwnd = hwnd;
				wnd.fullscreen = false;
				wnd.displayWidth = display_width;
				wnd.displayHeight = display_height;
				wnd.sceneWidth = display_width;
				wnd.sceneHeight = display_height;
				wnd.aaSamples = 1;

				R_SetD3DPresentParameters(&d3dpp, &wnd, game::dx->windowCount);

				const auto gfx_window = components::renderer::get_window(static_cast<components::renderer::GFXWND_>(wnd_count));

				//auto swapchain = &game::dx->windows[wnd_count].swapChain;
				auto swapchain = &gfx_window->swapChain;
				auto old_swapchain = *swapchain;

				if (*swapchain == nullptr)
				{
					ASSERT_MSG(1, "var");
				}

				*swapchain = nullptr;

				if (old_swapchain->Release())
				{
					ASSERT_MSG(0, "release failed, leaks ...");
				}

				if (auto hr = game::dx->device->CreateAdditionalSwapChain(&d3dpp, (IDirect3DSwapChain9**)swapchain);
					hr < 0)
				{
					// g_disableRendering;
					ASSERT_MSG(0, "CreateAdditionalSwapChain failed ...");
				}

				gfx_window->width = display_width;
				gfx_window->height = display_height;
			}
		}
	}

	void AxisToAngles(const float(*axis)[3], float* angles)
	{
		const static uint32_t func_addr = 0x4A8A00;
		__asm
		{
			pushad;
			mov		eax, angles;
			mov		ecx, axis;
			call	func_addr;
			popad;
		}
	}

	void vectoangles(float* vec /*esi*/, float* angles /*edi*/)
	{
		const static uint32_t func_addr = 0x4A5020;
		__asm
		{
			pushad;
			mov		edi, angles;
			mov		esi, vec;
			call	func_addr;
			popad;
		}
	}
	
	
	// *
	// * --------------------- dvars ------------------------------

	int* dvarCount = reinterpret_cast<int*>(0x242394C);
	game::dvar_s* dvarPool = reinterpret_cast<game::dvar_s*>(0x2427DA4); // dvarpool + 1 dvar size
	game::dvar_s* dvarPool_FirstEmpty = reinterpret_cast<game::dvar_s*>(0x242C14C); // first empty dvar 
	DWORD* sortedDvars = reinterpret_cast<DWORD*>(0x2423958); // sorted dvar* list
	DWORD* sortedDvarsAddons = reinterpret_cast<DWORD*>(0x2423CEC); // first empty pointer
	int sortedDvarsAddonsCount = 0;

	game::dvar_s* Dvar_RegisterVec4(const char* dvar_name /*ecx*/, float x, float y, float z, float w, float mins, float maxs, __int16 flags /*di/edi*/, const char* description)
	{
		const static uint32_t Dvar_RegisterVec4 = 0x4B2860;
		__asm
		{
			push	description;
			sub		esp, 24;
			
			fld		maxs;
			fstp    dword ptr[esp + 20];
			
			fld		mins;
			fstp    dword ptr[esp + 16];
			
			fld		w;
			fstp    dword ptr[esp + 12];
			
			fld		z;
			fstp    dword ptr[esp + 8];
			
			fld		y;
			fstp    dword ptr[esp + 4];
			
			fld		x;
			fstp    dword ptr[esp];

			xor		edi, edi;
			mov		di, flags;
			
			mov		ecx, dvar_name;
			
			call	Dvar_RegisterVec4;
			add		esp, 28;
		}
	}

	void Dvar_SetString(game::dvar_s *dvar /*esi*/, const char *string /*ebx*/)
	{
		const static uint32_t Dvar_SetString_func = 0x4B38D0;
		__asm
		{
			pushad;
			mov		ebx, [string];
			mov		esi, [dvar];
			call	Dvar_SetString_func;
			popad;
		}
	}

	const char* Dvar_DisplayableValue(game::dvar_s* dvar)
	{
		const static uint32_t Dvar_DisplayableValue_func = 0x4AFAA0;
		__asm
		{
			mov		eax, dvar
			call	Dvar_DisplayableValue_func
		}
	}

	const char* Dvar_DomainToString_Internal(signed int buffer_len /*eax*/, const char* buffer_out /*ebx*/, int dvar_type, int* enum_lines_count, float mins, float maxs) //DvarLimits limit)
	{
		const static uint32_t Dvar_DomainToString_Internal_func = 0x4B0220;
		__asm
		{
			push	maxs;
			push	mins;
			push	enum_lines_count;
			push	dvar_type;
			mov		ebx, buffer_out;
			mov		eax, buffer_len;
			call	Dvar_DomainToString_Internal_func;
			add		esp, 16;
		}
	}

	game::dvar_s * Dvar_SetFromStringFromSource(const char *string /*ecx*/, game::dvar_s *dvar /*esi*/, int source)
	{
		const static uint32_t Dvar_SetFromStringFromSource_func = 0x4B3910;
		__asm
		{
			mov		esi, dvar;
			push	source;
			mov		ecx, string;

			call	Dvar_SetFromStringFromSource_func;
			add		esp, 4;
		}
	}

	__declspec(naked) game::dvar_s* Dvar_FindVar(const char* /*dvar*/)
	{
		__asm
		{
			push eax;
			pushad;

			mov ebx, [esp + 28h];
			mov eax, 0x4B0F00; // Dvar_FindMalleableVar Addr.
			call eax;

			mov[esp + 20h], eax;
			popad;

			pop eax;
			retn;
		}
	}


	// *
	// * --------------------- console / warnings / error ------------------------------

	// do not call manually
	int printf_to_console_internal(const char* _format, va_list va)
	{
		int _result;
		char text_out[32772];

		vsprintf(text_out, _format, va);
		_result = _vfprintf_l(stdout, _format, NULL, va);

		if (const auto	con = GET_GUI(ggui::console_dialog); 
						con)
		{
			con->addline_no_format(text_out);
		}

		return _result;
	}
	
	int printf_to_console(_In_z_ _Printf_format_string_ char const* const _format, ...)
	{
		int _result;
		va_list _arglist;
		char text_out[1024];

		__crt_va_start(_arglist, _format);
		vsprintf(text_out, _format, _arglist);
		_result = _vfprintf_l(stdout, _format, NULL, _arglist);
		__crt_va_end(_arglist);

		if (const auto	con = GET_GUI(ggui::console_dialog);
						con)
		{
			con->addline_no_format(text_out);
		}
		
		return _result;
	}

	void com_printf_to_console([[maybe_unused]] int channel, const char* _format, ...)
	{
		va_list _arglist;
		char text_out[1024];

		__crt_va_start(_arglist, _format);
		vsprintf(text_out, _format, _arglist);
		_vfprintf_l(stdout, _format, NULL, _arglist);
		__crt_va_end(_arglist);

		if (const auto	con = GET_GUI(ggui::console_dialog);
						con)
		{
			con->addline_no_format(text_out);
		}
	}

	void console_error(const std::string &msg)
	{
		std::string err = "[!] " + msg + "\n";
		printf(err.c_str());

		if (const auto	con = GET_GUI(ggui::console_dialog);
						con)
		{
			con->addline_no_format(err.c_str());
		}
	}


	// *
	// * --------------------- fs / io ------------------------------

	game::fileData_s** com_fileDataHashTable = reinterpret_cast<fileData_s**>(0x2422948);

	void FS_ScanForDir(const char* directory, const char* search_path, int localized)
	{
		const static uint32_t FS_ScanForDir_func = 0x4A1E80;
		__asm
		{
			pushad;
			push	localized;
			mov		edx, directory;
			mov     ecx, search_path;
			call	FS_ScanForDir_func;
			add		esp, 4;
			popad;
		}
	}

	const char** FS_ListFilteredFilesWrapper(const char* path /*edx*/, const char* null /*esi*/, int* file_count)
	{
		const static uint32_t func_addr = 0x4A11A0;
		__asm
		{
			push	file_count;
			mov		esi, null;
			mov		edx, path;

			call	func_addr;
			add     esp, 4;
		}
	}

	std::uint32_t FS_HashFileName(const char* fname, int hash_size)
	{
		auto hash = 0u;
		for (auto i = 0u; fname[i]; ++i)
		{
			auto letter = tolower(fname[i]);
			if (letter == '.')
			{
				break;
			}

			if (letter == '\\')
			{
				letter = '/';
			}

			hash += letter * (i + 'w');
		}

		return ((hash >> 20) ^ hash ^ (hash >> 10)) & (hash_size - 1);
	}

	void* Hunk_FindDataForFileInternal(int hash /*eax*/, int data_type /*ebx*/, const char* name /*edi*/)
	{
		const static uint32_t func_addr = 0x4AC6A0;
		__asm
		{
			mov		eax, hash;
			mov		ebx, data_type;
			mov		edi, name;

			call	func_addr;
		}
	}

	int FS_OpenFileOverwrite(const char* path /*esi*/)
	{
		const static uint32_t func_addr = 0x4A05D0;
		__asm
		{
			mov		esi, path;
			call	func_addr;
		}
	}

	game::GfxImage* Image_FindExisting(const char* name)
	{
		const static uint32_t Image_FindExisting_func = 0x513200;
		__asm
		{
			mov     edi, name;
			call	Image_FindExisting_func;
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

	void Material_Add(int idx/*eax*/, game::Material* material/*esi*/)
	{
		const static uint32_t func_addr = 0x510CC0;
		__asm
		{
			pushad;
			mov     eax, idx;
			mov     esi, material;
			call	func_addr;
			popad;
		}
	}

	void* Hunk_AllocPhysPresetPrecache(size_t size)
	{
		if (size <= 0)
		{
			Assert();
		}

		return Hunk_Alloc(size);
	}

	PhysPreset* FX_RegisterPhysPreset(const char* name)
	{
		const static uint32_t func_addr = 0x4D6350;
		__asm
		{
			mov		eax, Hunk_AllocPhysPresetPrecache;
			mov		ecx, name;
			call	func_addr;
		}
	}

	void DObjCreate(game::DObjModel_s* dobjModels /*edi*/, game::DObj_s* obj /*esi*/, size_t numModels, game::XAnimTree_s* tree, int entnum)
	{
		const static uint32_t func_addr = 0x4BE9D0;
		__asm
		{
			pushad;
			mov		edi, dobjModels;
			mov		esi, obj;
			push	entnum;
			push	tree;
			push	numModels;
			call	func_addr;
			add		esp, 12;
			popad;
		}
	}

	// *
	// * --------------------- FX ----------------------------------

	int& g_processCodeMesh = *reinterpret_cast<int*>(0x174F960);

	int I_strncmp(const char* s0, const char* s1, int n)
	{
		int c0, c1;

		if (!s0 || !s1)
		{
			Assert();
			return s1 - s0;
		}
		do
		{
			c0 = *s0++;
			c1 = *s1++;

			if (!n--)
			{
				return 0;
			}

			if (c0 != c1)
			{
				return 2 * (c0 >= c1) - 1;
			}

		} while (c0);

		return 0;
	}

	int I_strcmp(const char* s1, const char* s2)
	{
		if (!s1 || !s2)
		{
			Assert();
		}

		return I_strncmp(s1, s2, 0x7FFFFFFF);
	}

	void I_strncpyz(char* Dest, const char* Source, int destsize)
	{
		if (!Source || !Dest || destsize < 1)
		{
			Assert();
		}

		strncpy(Dest, Source, destsize - 1);
		Dest[destsize - 1] = 0;
	}
}
