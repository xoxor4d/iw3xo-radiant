#include "STDInclude.hpp"

namespace Game
{
	namespace Globals
	{
		// Init
		std::string loadedModules;

		bool radiant_floatingWindows;
		bool radiant_initiated;
		bool radiant_config_loaded;
		bool radiant_config_not_found;

		CWnd* m_pCamWnd_ref;

		// Misc
		Game::TrackWorldspawn trackWorldspawn = Game::TrackWorldspawn();

		// Live Link
		Game::ProcessServerCommands cServerCmd = Game::ProcessServerCommands();
		bool live_connected;

		// Renderer
		IDirect3DDevice9* d3d9_device = nullptr;

		// Gui
		gui_present_s gui_present = gui_present_s();
		Game::gui_t gui = Game::gui_t();
		
	}

	// radiant globals
	int& g_nScaleHow = *reinterpret_cast<int*>(0x23F16DC);

	int	*g_nUpdateBitsPtr = reinterpret_cast<int*>(0x25D5A74);
	int	&g_nUpdateBits = *reinterpret_cast<int*>(0x25D5A74);
	bool &g_bScreenUpdates = *reinterpret_cast<bool*>(0x739B0F);
	double &g_time = *reinterpret_cast<double*>(0x2665678);
	double &g_oldtime = *reinterpret_cast<double*>(0x2665670);

	int *dvarCount = reinterpret_cast<int*>(0x242394C);
	Game::dvar_s* dvarPool = reinterpret_cast<Game::dvar_s*>(0x2427DA4); // dvarpool + 1 dvar size
	Game::dvar_s* dvarPool_FirstEmpty = reinterpret_cast<Game::dvar_s*>(0x242C14C); // first empty dvar 
	DWORD* sortedDvars = reinterpret_cast<DWORD*>(0x2423958); // sorted dvar* list
	DWORD* sortedDvarsAddons = reinterpret_cast<DWORD*>(0x2423CEC); // first empty pointer
	int sortedDvarsAddonsCount = 0;

	//Game::selbrush_t *selected_brushes = reinterpret_cast<Game::selbrush_t*>(0x23F1864);
	//Game::selbrush_t *selected_brushes_next = reinterpret_cast<Game::selbrush_t*>(0x23F1868);

	//entity_s* edit_entity = reinterpret_cast<entity_s*>(0x240A108); // add structs
	//entity_s* world_entity = reinterpret_cast<entity_s*>(0x25D5B30); // add structs

	Com_Error_t Com_Error = Com_Error_t(0x499F50);
	OnCtlColor_t OnCtlColor = OnCtlColor_t(0x587907);

	
	// "custom" ImGui_ImplWin32_WndProcHandler
	// * hook a wndclass::function handling input and call this function with the corrosponding WM_ msg
	void ImGui_HandleKeyIO(HWND hwnd, UINT key, SHORT zDelta, UINT nChar)
	{
		if (ImGui::GetCurrentContext() == NULL)
			return;

		ImGuiIO& io = ImGui::GetIO();

		switch (key)
		{
		case WM_SETFOCUS:
			std::fill_n(io.KeysDown, 512, 0);
			return;

		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		{
			int button = 0;
			if (key == WM_LBUTTONDOWN || key == WM_LBUTTONDBLCLK) { button = 0; }
			if (key == WM_RBUTTONDOWN || key == WM_RBUTTONDBLCLK) { button = 1; }
			if (key == WM_MBUTTONDOWN || key == WM_MBUTTONDBLCLK) { button = 2; }
			if (!ImGui::IsAnyMouseDown() && ::GetCapture() == nullptr)
				::SetCapture(hwnd);
			io.MouseDown[button] = true;
			return;
		}

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			int button = 0;
			if (key == WM_LBUTTONUP) { button = 0; }
			if (key == WM_RBUTTONUP) { button = 1; }
			if (key == WM_MBUTTONUP) { button = 2; }
			io.MouseDown[button] = false;
			if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
				::ReleaseCapture();
			return;
		}

		case WM_MOUSEWHEEL:
			io.MouseWheel += static_cast<float>(zDelta) / 120.0f; // WHEEL_DELTA
			return;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		    if (nChar < 256)
		        io.KeysDown[nChar] = true;
		    return;
			
		case WM_KEYUP:
		case WM_SYSKEYUP:
		    if (nChar < 256)
		        io.KeysDown[nChar] = false;
		    return;
			
		//case WM_CHAR:
		//    // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		//    io.AddInputCharacter((unsigned int)wParam);
		//    return;
		}
	}
	
	
	// -----------------------------------------------------------
	// DVARS

	void Dvar_SetString(Game::dvar_s *dvar /*esi*/, const char *string /*ebx*/)
	{
		const static uint32_t Dvar_SetString_Func = 0x4B38D0;
		
		__asm pushad
		__asm mov		ebx, [string]
		__asm mov		esi, [dvar]
		__asm Call		Dvar_SetString_Func
		__asm popad
	}

	const char* Dvar_DisplayableValue(Game::dvar_s* dvar)
	{
		const static uint32_t Dvar_DisplayableValue_Func = 0x4AFAA0;
		__asm
		{
			mov		eax, dvar
			Call	Dvar_DisplayableValue_Func
		}
	}

	Game::dvar_s * Dvar_SetFromStringFromSource(const char *string /*ecx*/, Game::dvar_s *dvar /*esi*/, int source)
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

	__declspec(naked) Game::dvar_s* Dvar_FindVar(const char* /*dvar*/)
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

	void ConsoleError(const std::string &msg)
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

	Game::GfxImage* Image_FindExisting(const char* name)
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

	Game::GfxImage* Image_RegisterHandle(const char* name)
	{
		Game::GfxImage* image = Game::Image_FindExisting(name);
		
		if (!image)
		{
			// Image_FindExisting_LoadObj
			image = utils::function<Game::GfxImage* (const char* name, int, int)>(0x54FFC0)(name, 1, 0);
		}

		return image;
	}
	
}
