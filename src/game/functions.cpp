#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace game
{
	namespace glob
	{
		// Init
		bool command_thread_running;
		std::vector<std::string> loadedModules;

		bool radiant_initiated;
		bool radiant_config_loaded;
		bool radiant_config_not_found;
		float frames_per_second;
		bool ccamwindow_realtime;

		// Misc
		game::TrackWorldspawn track_worldspawn = game::TrackWorldspawn();

		// Live Link
		game::ProcessServerCommands cServerCmd = game::ProcessServerCommands();
		bool live_connected;

		// Renderer
		IDirect3DDevice9* d3d9_device = nullptr;
		
	}

	game::vec3_t vec3_origin = { 0.0f, 0.0f, 0.0f };
	game::vec4_t color_white = { 1.0f, 1.0f, 1.0f, 1.0f };

	IDirect3DTexture9* framebuffer_test = nullptr;
	
	// radiant globals
	int&		g_nScaleHow = *reinterpret_cast<int*>(0x23F16DC);
	game::qeglobals_t* g_qeglobals = reinterpret_cast<game::qeglobals_t*>(0x25F39C0);

	float&	g_zoomLevel = *reinterpret_cast<float*>(0x25D5A90);
	int*	g_nUpdateBitsPtr = reinterpret_cast<int*>(0x25D5A74);
	int&	g_nUpdateBits = *reinterpret_cast<int*>(0x25D5A74);
	bool&	g_bScreenUpdates = *reinterpret_cast<bool*>(0x739B0F);
	double& g_time = *reinterpret_cast<double*>(0x2665678);
	double& g_oldtime = *reinterpret_cast<double*>(0x2665670);
	bool&	g_region_active = *reinterpret_cast<bool*>(0x23F1744);
	bool&	g_bCrossHairs = *reinterpret_cast<bool*>(0x25D5B06);
	bool&	g_bClipMode = *reinterpret_cast<bool*>(0x23F16D8);
	bool&	g_bRotateMode = *reinterpret_cast<bool*>(0x23F16D9);
	bool&	g_bScaleMode = *reinterpret_cast<bool*>(0x23F16DA);
	int&	g_nLastLen = *reinterpret_cast<int*>(0x25D5B14);
	int&	g_undoMaxSize = *reinterpret_cast<int*>(0x739F6C);

	float*	g_vRotateOrigin = reinterpret_cast<float*>(0x23F1658);
	
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

	HWND* entitywnd_hwnds = reinterpret_cast<HWND*>(0x240A118);

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

	bool is_single_brush_selected(bool print_warning)
	{
		if ((DWORD*)g_selected_brushes_next() == game::currSelectedBrushes || (DWORD*)g_selected_brushes_next()->next != game::currSelectedBrushes)
		{
			if(print_warning)
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

	void Undo_GeneralStart(const char* operation /*eax*/)
	{
#ifdef DEBUG
		game::printf_to_console("Undo_GeneralStart :: %s", operation);
#endif
		
		const static uint32_t func_addr = 0x45E3F0;
		__asm
		{
			pushad;
			mov		eax, operation;
			call	func_addr;
			popad;
		}
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

	void Patch_UpdateSelected(game::patchMesh_t* p /*esi*/, bool unk)
	{
		int unkown = unk;

		const static uint32_t patch_update_selected_func_addr = 0x438D80;
		__asm
		{
			mov		esi, p;
			push	unkown;
			call	patch_update_selected_func_addr;
			add     esp, 4;
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

	void Brush_Select(game::brush_t* b /*ecx*/, bool some_overwrite, bool update_status, bool center_grid_on_selection)
	{
		const int overwrite = some_overwrite;
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

	MatrixForViewer_t MatrixForViewer = reinterpret_cast<MatrixForViewer_t>(0x4A7A70);
	MatrixMultiply44_t MatrixMultiply44 = reinterpret_cast<MatrixMultiply44_t>(0x4A6180);
	MatrixInverse44_t MatrixInverse44 = reinterpret_cast<MatrixInverse44_t>(0x4A6670);

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

	CopyAxis_t CopyAxis = reinterpret_cast<CopyAxis_t>(0x4A8860);
	AnglesToAxis_t AnglesToAxis = reinterpret_cast<AnglesToAxis_t>(0x4ABEB0);
	AngleVectors_t AngleVectors = reinterpret_cast<AngleVectors_t>(0x4ABD70);
	OrientationConcatenate_t OrientationConcatenate = reinterpret_cast<OrientationConcatenate_t>(0x4BA7D0);


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

	// -----------------------------------------------------------
	// DVARS

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

	const char* Dvar_DomainToString_Internal(signed int buffer_len /*eax*/, const char* buffer_out /*ebx*/, int dvar_type, int* enum_lines_count, float mins, float maxs) //DvarLimits limit)
	{
		const static uint32_t Dvar_DomainToString_Internal_Func = 0x4B0220;
		__asm
		{
			push	maxs;
			push	mins;
			
			push	enum_lines_count;
			push	dvar_type;
			mov		ebx, buffer_out;
			mov		eax, buffer_len;
			call	Dvar_DomainToString_Internal_Func;
			add		esp, 16;
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

	// do not call manually :x
	int printf_to_console_internal(const char* _format, va_list va)
	{
		int _result;
		char text_out[32772];

		vsprintf(text_out, _format, va);
		_result = _vfprintf_l(stdout, _format, NULL, va);

		if (ggui::_console)
		{
			ggui::_console->addline_no_format(text_out);
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

		if(ggui::_console)
		{
			ggui::_console->addline_no_format(text_out);
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

		if (ggui::_console)
		{
			ggui::_console->addline_no_format(text_out);
		}
	}

	void console_error(const std::string &msg)
	{
		std::string err = "[!] " + msg + "\n";
		printf(err.c_str());

		if (ggui::_console)
		{
			ggui::_console->addline_no_format(err.c_str());
		}
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

	game::GfxCmdHeader* R_GetCommandBuffer(std::uint32_t bytes /*ebx*/, int render_cmd /*edi*/)
	{
		const static uint32_t R_RenderBufferCmdCheck_Func = 0x4FAEB0;
		__asm
		{
			mov		ebx, bytes;
			mov		edi, render_cmd;
			call	R_RenderBufferCmdCheck_Func;
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
				for (auto i = game::dx->windows; i->hwnd != hwnd; ++i)
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

				auto swapchain = &game::dx->windows[wnd_count].swapChain;
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

				game::dx->windows[wnd_count].width = display_width;
				game::dx->windows[wnd_count].height = display_height;
			}
		}
	}

}
