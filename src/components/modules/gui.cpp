#include "std_include.hpp"
#include <iomanip>

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define mainframe_thiscall(return_val, addr)	\
	utils::hook::call<return_val(__fastcall)(cmainframe*)>(addr)(cmainframe::activewnd)

#define mainframe_cdeclcall(return_val, addr)	\
	utils::hook::call<return_val(__cdecl)(cmainframe*)>(addr)(cmainframe::activewnd)

#define mainframe_stdcall(return_val, addr)	\
	utils::hook::call<return_val(__stdcall)(cmainframe*)>(addr)(cmainframe::activewnd)

#define cdeclcall(return_val, addr)	\
	utils::hook::call<return_val(__cdecl)()>(addr)()


#define IMGUI_REGISTER_TOGGLEABLE_MENU(menu, function, function_on_close) \
    if(menu.menustate) {		\
        function;				\
        menu.was_open = true;	\
    }							\
    else if(menu.was_open) {	\
		function_on_close;		\
		menu.was_open = false;	\
    }

// left label menu widget, eg. "dragfloat"
#define IMGUI_MENU_WIDGET_SINGLE(label, func)                                                       \
    ImGui::Text(label); ImGui::SameLine();                                                          \
    const ImGuiMenuColumns* offsets = &ImGui::GetCurrentWindow()->DC.MenuColumns;                   \
	ImGui::SetCursorPosX(static_cast<float>(offsets->OffsetShortcut + 5));                          \
	ImGui::PushItemWidth(static_cast<float>(offsets->Widths[2] + offsets->Widths[3] + 5));          \
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));   \
	func; ImGui::PopItemWidth(); ImGui::PopStyleVar();

// -------------------------------------------------------------------

// show tooltip after x seconds
#define TTDELAY 0.5f 

// tooltip with delay
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) ImGui::SetTooltip(tooltip)

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing) 


namespace components
{

	// *
	// initialize the imgui camerawnd context
	void imgui_init_ccamerawnd()
	{
		// get the device from d3d9ex::_d3d9/Ex::CreateDevice
		IDirect3DDevice9* device = game::glob::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");
		
		//ASSERT_MSG(cmainframe::activewnd->m_pCamWnd, "cmainframe::activewnd->m_pCamWnd == nullptr");
		if (!cmainframe::activewnd->m_pCamWnd)
		{
			return;
		}
		
		ggui::state.ccamerawnd.context = ImGui::CreateContext();
		ASSERT_MSG(ggui::state.ccamerawnd.context, "ccamerawnd.context == nullptr");
		
		// set context
		ImGui::SetCurrentContext(ggui::state.ccamerawnd.context);

        ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		//io.MouseDrawCursor = true;

		// font (see Fonts.cpp)
        io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

		// implementation
		ImGui_ImplWin32_Init(cmainframe::activewnd->m_pCamWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();

		// fully initialized
		ggui::state.ccamerawnd.context_initialized = true;
		ggui::state.ccamerawnd.dx_window = &game::dx->windows[ggui::CCAMERAWND];
	}

	
	// *
	// initialize the imgui xywnd context
	void imgui_init_cxywnd()
	{
		// get the device from d3d9ex::_d3d9/Ex::CreateDevice
		IDirect3DDevice9* device = game::glob::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");

		//ASSERT_MSG(cmainframe::activewnd->m_pXYWnd, "cmainframe::activewnd->m_pXYWnd == nullptr");
		if(!cmainframe::activewnd->m_pXYWnd)
		{
			return;
		}

		ggui::state.cxywnd.context = ImGui::CreateContext();
		ASSERT_MSG(ggui::state.cxywnd.context, "cxywnd.context == nullptr");

		// set context
		ImGui::SetCurrentContext(ggui::state.cxywnd.context);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.MouseDrawCursor = true;
		
		// font (see Fonts.cpp)
		io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

		// implementation
		ImGui_ImplWin32_Init(cmainframe::activewnd->m_pXYWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();
		
		// fully initialized
		ggui::state.cxywnd.context_initialized = true;
		ggui::state.cxywnd.dx_window = &game::dx->windows[ggui::CXYWND];
	}

	
	/*
	 * auto vtable = reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable);
	 * reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable)->SetStatusText(&cmainframe::activewnd->m_wndStatusBar, 0x75);
	 *
	 * auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);
	 * vtable->RecalcLayout(&cmainframe::activewnd->m_wndSplit);
	 */

	
	const float GRID_SIZE[11] =
	{ 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f, 256.0f, 512.0f };

	enum GRID_E : int
	{
		GRID_05,
		GRID_1,
		GRID_2,
		GRID_4,
		GRID_8,
		GRID_16,
		GRID_32,
		GRID_64,
		GRID_128,
		GRID_256,
		GRID_512,
	};

	void set_grid_size(const GRID_E size)
	{
		game::g_qeglobals->d_gridsize = size;
		if (game::g_PrefsDlg()->m_bSnapTToGrid)
		{
			game::g_qeglobals->d_gridsize_float = GRID_SIZE[size];
		}
		
		mainframe_thiscall(void, 0x428A00); // CMainFrame::SetGridStatus
		game::g_nUpdateBits |= W_Z | W_XY;
	}

	
	enum RENDER_METHOD_E : int
	{
		RM_WIREFRAME,
		RM_FULLBRIGHT,
		RM_NORMALFAKELIGHT,
		RM_VIEWFAKELIGHT,
		RM_CASETEXTURES,
	};
	
	void set_render_method(const RENDER_METHOD_E meth)
	{
		switch(meth)
		{
			case RM_WIREFRAME: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80DE; break;
			case RM_FULLBRIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80DF; break;
			case RM_NORMALFAKELIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E0; break;
			case RM_VIEWFAKELIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E1; break;
			case RM_CASETEXTURES: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E2; break;
		}

		if (cmainframe::activewnd->m_pCamWnd->camera.draw_mode != meth)
		{
			cmainframe::activewnd->m_pCamWnd->camera.draw_mode = meth;
			game::g_nUpdateBits |= W_CAMERA;
		}
	}

	void set_texture_resolution(int picmip)
	{
		game::g_qeglobals->d_picmip = picmip;

		// CMainFrame::PicMip
		mainframe_stdcall(void, 0x420860); // sets the stock menu ..

		// R_UpdateMipMap
		cdeclcall(void, 0x5139A0);

		// R_ReloadImages
		game::R_ReloadImages();

		game::g_nUpdateBits = -1;
	}

	void color_menu(ggui::imgui_context_menu& menu)
	{
		ImGui::Begin("Colors##xywnd", &menu.menustate, ImGuiWindowFlags_NoCollapse);

		ImGui::ColorEdit4("Camera Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_CAMERABACK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Texture Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_TEXTUREBACK], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);
		
		ImGui::ColorEdit4("Grid Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBACK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Minor", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMINOR], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Major", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMAJOR], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Block", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBLOCK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Text", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDTEXT], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);
		
		ImGui::ColorEdit4("Unselected Brushes", game::g_qeglobals->d_savedinfo.colors[game::COLOR_BRUSHES], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Selected Brushes", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELBRUSHES], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Selected Brushes Cam", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELBRUSHES_CAMERA], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Selected Face Cam", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELFACE_CAMERA], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);
		
		ImGui::ColorEdit4("Detail", game::g_qeglobals->d_savedinfo.colors[game::COLOR_DETAIL_BRUSH], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Non-Colliding", game::g_qeglobals->d_savedinfo.colors[game::COLOR_NONCOLLIDING], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Weapon Clip", game::g_qeglobals->d_savedinfo.colors[game::COLOR_WEAPON_CLIP], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);
		
		ImGui::ColorEdit4("Size Info", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SIZE_INFO], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Clipper", game::g_qeglobals->d_savedinfo.colors[game::COLOR_CLIPPER], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Model", game::g_qeglobals->d_savedinfo.colors[game::COLOR_MODEL], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Wireframe", game::g_qeglobals->d_savedinfo.colors[game::COLOR_WIREFRAME], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Frozen Layers", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FROZEN_LAYERS], ImGuiColorEditFlags_Float);
		
		SEPERATORV(0.0f);
		
		ImGui::ColorEdit4("Viewname", game::g_qeglobals->d_savedinfo.colors[game::COLOR_VIEWNAME], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Func Group", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_GROUP], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Func Cull Group", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_CULLGROUP], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Entity", game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITY], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Entity Unkown", game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITYUNK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Togglesurfs", game::g_qeglobals->d_savedinfo.colors[game::COLOR_DRAW_TOGGLESUFS], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Unkown", game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN2], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Unkown", game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN3], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Gui Menubar Bg", dvars::gui_menubar_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Docked Bg", dvars::gui_dockedwindow_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Undocked Bg", dvars::gui_window_bg_color->current.vector, ImGuiColorEditFlags_Float);

		ImGui::End();
	}

	// -----

	struct commandbinds
	{
		std::string cmd_name;
		std::string keys;
		int modifier_alt;
		int modifier_ctrl;
		int modifier_shift;
		std::string modifier_key;
	};

	std::vector<commandbinds> cmd_binds;

	const char* radiant_modifier_alt[] =
	{
		"",
		"ALT"
	};

	const char* radiant_modifier_ctrl[] =
	{
		"",
		"CTRL"
	};

	const char* radiant_modifier_shift[] =
	{
		"",
		"SHIFT"
	};

	const char* radiant_keybind_array[] =
	{
		"",
		"Space",
		"Backspace",
		"Escape",
		"End",
		"Insert",
		"Delete",
		"PageUp",
		"PageDown",
		"Up",
		"Down", // 10
		"Left",
		"Right",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8", // 20
		"F9",
		"F10",
		"F11",
		"F12",
		"Tab",
		"Return",
		"Comma",
		"Period",
		"Plus",
		"Multiply", // 30
		"Subtract",
		"NumPad0",
		"NumPad1",
		"NumPad2",
		"NumPad3",
		"NumPad4",
		"NumPad5",
		"NumPad6",
		"NumPad7",
		"NumPad8", // 40
		"NumPad9",
		"Minus",
		"[",
		"]",
		"\\",
		"~",
		"LWin", // 47
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
	};

	// get ascii fror keybind key
	int cmdbinds_key_to_ascii(std::string key)
	{
		if(key.length() > 1)
		{
			if (key == "Space"s)		return 0x20;
			if (key == "Backspace"s)	return 0x8;
			if (key == "Escape"s)		return 0x1B;
			if (key == "End"s)			return 0x23;
			if (key == "Insert"s)		return 0x2D;
			if (key == "Delete"s)		return 0x2E;
			if (key == "PageUp"s)		return 0x21;
			if (key == "PageDown"s)		return 0x22;
			if (key == "Up"s)			return 0x26;
			if (key == "Down"s)			return 0x28;
			if (key == "Left"s)			return 0x25;
			if (key == "Right"s)		return 0x27;
			if (key == "F1"s)			return 0x70;
			if (key == "F2"s)			return 0x71;
			if (key == "F3"s)			return 0x72;
			if (key == "F4"s)			return 0x73;
			if (key == "F5"s)			return 0x74;
			if (key == "F6"s)			return 0x75;
			if (key == "F7"s)			return 0x76;
			if (key == "F8"s)			return 0x77;
			if (key == "F9"s)			return 0x78;
			if (key == "F10"s)			return 0x79;
			if (key == "F11"s)			return 0x7A;
			if (key == "F12"s)			return 0x7B;
			if (key == "Tab"s)			return 0x9;
			if (key == "Return"s)		return 0x0D;
			if (key == "Comma"s)		return 0x0BC;
			if (key == "Period"s)		return 0x0BE;
			if (key == "Plus"s)			return 0x6B;
			if (key == "Multiply"s)		return 0x6A;
			if (key == "Subtract"s)		return 0x6D;
			if (key == "NumPad0"s)		return 0x60;
			if (key == "NumPad1"s)		return 0x61;
			if (key == "NumPad2"s)		return 0x62;
			if (key == "NumPad3"s)		return 0x63;
			if (key == "NumPad4"s)		return 0x64;
			if (key == "NumPad5"s)		return 0x65;
			if (key == "NumPad6"s)		return 0x66;
			if (key == "NumPad7"s)		return 0x67;
			if (key == "NumPad8"s)		return 0x68;
			if (key == "NumPad9"s)		return 0x69;
			if (key == "Minus"s)		return 0x0BD;
			if (key == "["s)			return 0x0DB;
			if (key == "]"s)			return 0x0DD;
			if (key == "\\"s)			return 0x0DC;
			if (key == "~"s)			return 0x0C0;
			if (key == "LWin"s)			return 0x5B;

			printf("[Hotkeys] Unkown key '%s'", key.c_str());
			return 0;
		}

		return key[0];
	}

	// get ascii fror keybind key
	std::string cmdbinds_ascii_to_keystr(int key)
	{
		if (key == 0x20)  return "Space"s;
		if (key == 0x8)   return "Backspace"s;
		if (key == 0x1B)  return "Escape"s;
		if (key == 0x23)  return "End"s;
		if (key == 0x2D)  return "Insert"s;
		if (key == 0x2E)  return "Delete"s;
		if (key == 0x21)  return "PageUp"s;
		if (key == 0x22)  return "PageDown"s;
		if (key == 0x26)  return "Up"s;
		if (key == 0x28)  return "Down"s;
		if (key == 0x25)  return "Left"s;
		if (key == 0x27)  return "Right"s;
		if (key == 0x70)  return "F1"s;
		if (key == 0x71)  return "F2"s;
		if (key == 0x72)  return "F3"s;
		if (key == 0x73)  return "F4"s;
		if (key == 0x74)  return "F5"s;
		if (key == 0x75)  return "F6"s;
		if (key == 0x76)  return "F7"s;
		if (key == 0x77)  return "F8"s;
		if (key == 0x78)  return "F9"s;
		if (key == 0x79)  return "F10"s;
		if (key == 0x7A)  return "F11"s;
		if (key == 0x7B)  return "F12"s;
		if (key == 0x9)   return "Tab"s;
		if (key == 0x0D)  return "Return"s;
		if (key == 0x0BC) return "Comma"s;
		if (key == 0x0BE) return "Period"s;
		if (key == 0x6B)  return "Plus"s;
		if (key == 0x6A)  return "Multiply"s;
		if (key == 0x6D)  return "Subtract"s;
		if (key == 0x60)  return "NumPad0"s;
		if (key == 0x61)  return "NumPad1"s;
		if (key == 0x62)  return "NumPad2"s;
		if (key == 0x63)  return "NumPad3"s;
		if (key == 0x64)  return "NumPad4"s;
		if (key == 0x65)  return "NumPad5"s;
		if (key == 0x66)  return "NumPad6"s;
		if (key == 0x67)  return "NumPad7"s;
		if (key == 0x68)  return "NumPad8"s;
		if (key == 0x69)  return "NumPad9"s;
		if (key == 0x0BD) return "Minus"s;
		if (key == 0x0DB) return "["s;
		if (key == 0x0DD) return "]"s;
		if (key == 0x0DC) return "\\"s;
		if (key == 0x0C0) return "~"s;
		if (key == 0x5B)  return "LWin"s;

		std::string out; out += (char)key;
		return out;
	}

	
	std::string get_hotkey_for_command(const char* command)
	{
		// find command in cmd_binds (ini)
		for (const auto& bind : cmd_binds)
		{
			if (!_strcmpi(command, bind.cmd_name.c_str()))
			{
				return	(bind.modifier_shift == 1 ? "SHIFT-"s : "") +
						(bind.modifier_alt	 == 1 ? "ALT-"s : "") +
						(bind.modifier_ctrl  == 1 ? "CTRL-"s : "") +
						 bind.modifier_key;
			}
		}

		// get the hotkey from the default command list if there is no ini or the command is missing
		// note: we overwrite g_Commands with our own hotkeys -> always check g_Commands?
		for (auto i = 0; i < game::g_nCommandCount; i++)
		{
			if (!_strcmpi(game::g_Commands[i].m_strCommand, command))
			{
				const unsigned int o_key = game::g_Commands[i].m_nKey;
				const unsigned int o_mod = game::g_Commands[i].m_nModifiers;

				std::string mod;
				mod += (o_mod & 1 ? "SHIFT-"s : "");
				mod += (o_mod & 2 ? "ALT-"s : "");
				mod += (o_mod & 4 ? "CTRL-"s : "");
				mod += cmdbinds_ascii_to_keystr(o_key);

				return mod;
			}
		}

		return "";
	}

	
	// populates std::vector<commandbinds> cmd_binds
	bool cmdbinds_load_from_file(std::string file)
	{
		cmd_binds.clear();
		std::string home_path;

		const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
		if(fs_homepath)
		{
			home_path = fs_homepath->current.string;
		}
		else
		{
			char buffer[512];
			if(!GetModuleFileNameA(nullptr, buffer, 512))
			{
				printf("[Hotkeys] could not get the base directory.\n");
				return false;
			}

			std::string path = buffer;
			home_path = path.substr(0, path.find_last_of("\\/"));
		}

		std::ifstream ini;

		std::string ini_path = home_path;
					ini_path += "\\" + file;

		ini.open(ini_path.c_str());

		if (!ini.is_open())
		{
			printf("[Hotkeys] Could not find \"iw3r_hotkeys.ini\" in \"%s\"\n", home_path.c_str());
			return false;
		}

		std::string input;
		std::vector<std::string> args;

		// read line by line
		while (std::getline(ini, input))
		{
			if (input.find("[Commands]") != std::string::npos)
			{
				//printf("[Hotkeys] Ignored '%s'\n", input.c_str());
				continue;
			}

			// ignore comments
			if (input.find(';') != std::string::npos)
			{
				printf("[Hotkeys] Ignored '%s'\n", input.c_str());
				continue;
			}

			// ignore lines not containing '='
			if (input.find(" =") == std::string::npos)
			{
				printf("[Hotkeys] Ignored '%s' => missing '='\n", input.c_str());
				continue;
			}

			// split the string on = (gets us 2 args)
			args = utils::split(input, '=');

			// remove the leftover space on the command name
			utils::rtrim(args[0]);

			// trim leading tabs and spaces on the key-bind
			utils::ltrim(args[1]);

			// split keys on space
			std::vector<std::string> keys;
			if (args[1].find(' ') != std::string::npos)
			{
				// multiple keys
				keys = utils::split(args[1], ' ');
			}
			else
			{
				// single key
				keys.push_back(args[1]);
			}

			cmd_binds.push_back(
				commandbinds
				{
					args[0],
					args[1],
					input.find("+alt") != std::string::npos,
					input.find("+ctrl") != std::string::npos,
					input.find("+shift") != std::string::npos,
					keys[keys.size() - 1]
				});
		}

		return true;
	}

	
	// g_commandmap m_nModifiers
	// 1u = shift
	// 2u = alt
	// 4u = ctrl
	// 8u = lwin

	// overwrite hardcoded hotkeys with our own
	void load_commandmap()
	{
		if (!cmdbinds_load_from_file("iw3r_hotkeys.ini"s))
		{
			// update 'std::map' unkown commandmap (uses g_Commands)
			cdeclcall(void, 0x420140); // the func that would normally be called
			
			return;
		}

		int commands_overwritten = 0;
		printf("[Hotkeys] Loading '%d' hotkeys from 'iw3r_hotkeys.ini'\n", cmd_binds.size());
		
		for(auto i = 0; i < game::g_nCommandCount; i++)
		{
			for (commandbinds& bind : cmd_binds)
			{
				if(!_strcmpi(game::g_Commands[i].m_strCommand, bind.cmd_name.c_str()))
				{
					const unsigned int o_key = game::g_Commands[i].m_nKey;
					const unsigned int o_mod = game::g_Commands[i].m_nModifiers;
					
					//printf("overwriting command '%s'\n", game::g_Commands[i].m_strCommand);
					//printf("|-> m_nKey '%d' to ", game::g_Commands[i].m_nKey);

					game::g_Commands[i].m_nKey = cmdbinds_key_to_ascii(bind.modifier_key);
					
					//printf("'%d'\n", game::g_Commands[i].m_nKey);
					//printf("|-> m_nModifiers '%d' to ", game::g_Commands[i].m_nModifiers);

					game::g_Commands[i].m_nModifiers = 
						   bind.modifier_shift
						| (bind.modifier_alt == 1 ? 2 : 0)
						| (bind.modifier_ctrl == 1 ? 4 : 0);

					//printf("'%d'\n\n", game::g_Commands[i].m_nModifiers);
					
					if(o_key != game::g_Commands[i].m_nKey || 
						o_mod != game::g_Commands[i].m_nModifiers) 
					{
						printf("|-> modified hotkey '%s'\n", bind.cmd_name.c_str());
						commands_overwritten++;
					}

					break;
				}
			}
		}

		printf("|-> modified '%d' commands\n\n", commands_overwritten);

		// there is a second commandmap (vector/unsorted map) ... whatever
		// clear ^ and set command count to 0
		cdeclcall(void, 0x42C7A0);

		// update 'std::map' unkown commandmap (uses g_Commands)
		cdeclcall(void, 0x420140);
	}

	
	// load the default commandmap if we did not load our own (ini)
	void load_default_commandmap()
	{
		if(cmd_binds.empty())
		{
			cdeclcall(void, 0x421230); // CMainFrame::LoadCommandMap
		}
	}

	
	// triggered when the imgui hotkey menu gets closed
	void cmdbinds_on_close()
	{
		if(cmd_binds.empty())
		{
			return;
		}
		
		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
			fs_homepath)
		{
			std::ofstream ini;

			std::string ini_path = fs_homepath->current.string;
						ini_path += "\\iw3r_hotkeys.ini";

			ini.open(ini_path.c_str());
			
			if (!ini.is_open())
			{
				printf("[Hotkeys] Could not write to \"iw3r_hotkeys.ini\" in \"%s\"", fs_homepath->current.string);
				return;
			}

			ini << "[Commands]" << std::endl;

			for (commandbinds& bind : cmd_binds)
			{
				ini << std::left << std::setw(26) << bind.cmd_name << " = ";

				ini << (bind.modifier_shift == 0 ? "" : "+shift ");
				ini << (bind.modifier_alt	== 0 ? "" : "+alt ");
				ini << (bind.modifier_ctrl	== 0 ? "" : "+ctrl ");
				ini <<  bind.modifier_key << std::endl;
			}

			load_commandmap();
		}
	}

	
	// show help text in case there is no hotkeys file
	void cmdbinds_helper_menu(ggui::imgui_context_menu& menu)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(450, 160), ImVec2(450, 160));
		ImGui::Begin("Hotkeys Helper##xywnd", &menu.menustate, ImGuiWindowFlags_NoCollapse);

		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
			fs_homepath)
		{
			const char* apply_hint = utils::va("Could not find file 'iw3r_hotkeys.ini' in\n'%s'.", fs_homepath->current.string);
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(apply_hint).x) * 0.5f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetWindowHeight() * 0.5f - ImGui::CalcTextSize(apply_hint).y);
			ImGui::TextUnformatted(apply_hint);
		}
		
		ImGui::End();
	}

	bool cmdbinds_check_dupe(commandbinds& bind, std::string& o_dupebind)
	{
		for (commandbinds& binds : cmd_binds)
		{
			// ignore "self"
			if(bind.cmd_name == binds.cmd_name) {
				continue;
			}

			// ignore non-assigned
			if (!binds.modifier_shift && !binds.modifier_ctrl && !binds.modifier_alt
				&& binds.modifier_key.empty())
			{
				continue;
			}

			if(		bind.modifier_shift == binds.modifier_shift
				 && bind.modifier_ctrl	== binds.modifier_ctrl
				 && bind.modifier_alt	== binds.modifier_alt
				 && bind.modifier_key	== binds.modifier_key)
			{
				o_dupebind = binds.cmd_name;
				return true;
			}
		}

		return false;
	}
	
	// hotkey menu
	void cmdbinds_menu(ggui::imgui_context_menu& menu)
	{
		// on first open, load ini
		if(!menu.was_open)
		{
			if(!cmdbinds_load_from_file("iw3r_hotkeys.ini"s))
			{
				gui::toggle(ggui::state.cxywnd.m_cmdbinds_helper, 0, true);
				menu.menustate = false;
			}
		}

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(1.0f, 4.0f));
		ImGui::SetNextWindowSizeConstraints(ImVec2(450, 320), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin("Hotkeys##xywnd", &menu.menustate, ImGuiWindowFlags_NoCollapse);

		const char* apply_hint = "Changes will apply upon closing the window.";
		ImGui::SetCursorPosX((ImGui::GetColumnWidth() - ImGui::CalcTextSize(apply_hint).x) * 0.5f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
		ImGui::TextUnformatted(apply_hint);
		
		if (ImGui::BeginTable("bind_table", 5, 
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("                  Command", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 200.0f);
			ImGui::TableSetupColumn("Shift", ImGuiTableColumnFlags_WidthFixed, 34.0f);
			ImGui::TableSetupColumn("  Alt", ImGuiTableColumnFlags_WidthFixed, 34.0f);
			ImGui::TableSetupColumn(" Ctrl", ImGuiTableColumnFlags_WidthFixed, 34.0f);
			ImGui::TableSetupColumn(" Key", ImGuiTableColumnFlags_WidthStretch, 120.0f);
			ImGui::TableHeadersRow();

			int row = 0;
			for (commandbinds& bind : cmd_binds)
			{
				std::string str_dupe_bind = bind.cmd_name;
				bool found_dupe = cmdbinds_check_dupe(bind, str_dupe_bind);
				
				// unique widget id's for each row (we get collisions otherwise)
				ImGui::PushID(row); row++;
				ImGui::TableNextRow();
				
				for (int column = 0; column < 5; column++)
				{
					ImGui::PushID(column);
					ImGui::TableNextColumn();
					
					switch (column)
					{
					case 0:
						ImGui::SetCursorPosX((ImGui::GetColumnWidth() - ImGui::CalcTextSize(bind.cmd_name.c_str()).x) * 0.5f);
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

						if (found_dupe)
						{
							ImGui::TextColored(ImVec4(0.9f, 0.1f, 0.1f, 1.0f), bind.cmd_name.c_str());
							ImGui::SameLine();
							ImGui::HelpMarker(utils::va("bind conflicts with '%s'", str_dupe_bind.c_str()));
						}
						else
						{
							ImGui::TextUnformatted(bind.cmd_name.c_str());
						}

						break;
					case 1:
						ImGui::Checkbox("##2", (bool*)&bind.modifier_shift);
						break;
					case 2:
						ImGui::Checkbox("##0", (bool*)&bind.modifier_alt);
						break;
					case 3:
						ImGui::Checkbox("##1", (bool*)&bind.modifier_ctrl);
						break;
					case 4:
						float w = ImGui::GetColumnWidth();//ImGui::CalcItemWidth();
						ImGui::PushItemWidth(w - 6.0f);
						
						if (ImGui::BeginCombo("##combokey", bind.modifier_key.c_str(), ImGuiComboFlags_NoArrowButton)) // The second parameter is the label previewed before opening the combo.
						{
							for (int n = 0; n < IM_ARRAYSIZE(radiant_keybind_array); n++)
							{
								const bool is_selected = !_stricmp(bind.modifier_key.c_str(), radiant_keybind_array[n]); // You can store your selection however you want, outside or inside your objects
								if (is_selected)
								{
									ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
								}
								
								if (ImGui::Selectable(radiant_keybind_array[n], is_selected))
								{
									bind.modifier_key = radiant_keybind_array[n];
									if (is_selected)
									{
										ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
									}	
								}
							}
							ImGui::EndCombo();
						}
						
						ImGui::PopItemWidth();
						break;
					}

					// column
					ImGui::PopID();
				}

				// row
				ImGui::PopID();
			}
			
			ImGui::EndTable();
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_CellPadding
		ImGui::End();
	}

	
	void cxywnd_gui(ggui::imgui_context_cxy& context)
	{
		ImGuiIO& io = ImGui::GetIO();

		int _stylevars = 0;
		int _stylecolors = 0;
		
		// styles that need to be set before the dockspace is created
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(800, 36));	_stylevars++;


		// *
		// create main dockspace

		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::ToImVec4(dvars::gui_menubar_bg_color->current.vector));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ToImVec4(dvars::gui_dockedwindow_bg_color->current.vector));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));	_stylecolors++;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("dockspace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		
		// *
		// menu bar within dockspace
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 6));
		
		if (ImGui::BeginMenuBar()) 
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Map")) {
					mainframe_cdeclcall(void, 0x423AA0); //cmainframe::OnFileNew
				}

				if (ImGui::MenuItem("Open", get_hotkey_for_command("FileOpen").c_str())) {
					mainframe_cdeclcall(void, 0x423AE0); //cmainframe::OnFileOpen
				}

				if (ImGui::MenuItem("Save", get_hotkey_for_command("FileSave").c_str())) {
					mainframe_cdeclcall(void, 0x423B80); //cmainframe::OnFileSave
				}

				if (ImGui::MenuItem("Save As")) {
					mainframe_cdeclcall(void, 0x423BC0); //cmainframe::OnFileSaveas
				}

				if (ImGui::MenuItem("Save Selected")) {
					mainframe_thiscall(void, 0x4293A0); //cmainframe::OnFileExportmap
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Open Recent", game::g_qeglobals->d_lpMruMenu->wNbItemFill))
				{
					// itemfill = amount of strings inside "lpMRU"
					for (std::uint16_t i = 0; i < game::g_qeglobals->d_lpMruMenu->wNbItemFill; i++)
					{
						// "lpMRU" strings are 128chars wide
						const char* mru_item_str = game::g_qeglobals->d_lpMruMenu->lpMRU + (game::g_qeglobals->d_lpMruMenu->wMaxSizeLruItem * i);

						if (mru_item_str && ImGui::MenuItem(mru_item_str))
						{
							typedef  BOOL(__thiscall* CMainFrame_OnMru_t)(cmainframe* pThis, std::uint16_t nID);
							CMainFrame_OnMru_t CMainFrame_OnMru = reinterpret_cast<CMainFrame_OnMru_t>(0x423FE0);

							//mru id's start at 8000 (wIdMru) + 1 for the first item as DoMru() subtracts 1
							CMainFrame_OnMru(cmainframe::activewnd, i + 1 + game::g_qeglobals->d_lpMruMenu->wIdMru);
						}
					}

					ImGui::EndMenu(); // Open Recent
				}
				
				if (ImGui::BeginMenu("Misc")) 
				{
					if (ImGui::MenuItem("Save Region", "", nullptr, game::g_region_active)) {
						mainframe_cdeclcall(void, 0x429020); //cmainframe::OnFileSaveregion
					}

					if (ImGui::MenuItem("Pointfile")) {
						mainframe_cdeclcall(void, 0x423B20); //cmainframe::OnPointfileOpen
					}

					if (ImGui::MenuItem("Errorfile")) {
						mainframe_cdeclcall(void, 0x423B40); //cmainframe::OnErrorFile
					}
					
					ImGui::EndMenu(); // Misc
				}
				
				if (ImGui::BeginMenu("Project Settings"))
				{
					if (ImGui::MenuItem("New Project")){
						mainframe_cdeclcall(void, 0x426E80); //cmainframe::OnFileNewproject
					}

					if (ImGui::MenuItem("Set Startup Project")){
						mainframe_cdeclcall(void, 0x427010); //cmainframe::OnSetStartupProject
					}

					if (ImGui::MenuItem("Project Settings")) {
						mainframe_cdeclcall(void, 0x428DE0); //cmainframe::OnFileProjectsettings
					}
					
					ImGui::EndMenu(); // Project Settings
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Exit", get_hotkey_for_command("Quit").c_str()))
				{
					PostMessage(cmainframe::activewnd->GetWindow(), WM_CLOSE, 0, 0L);
				}

				ImGui::EndMenu(); // File
			}

			// ----------------------------
			
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", get_hotkey_for_command("Undo").c_str(), nullptr, game::g_lastundo() && game::g_lastundo()->done)) {
					cdeclcall(void, 0x428730); //cmainframe::OnEditUndo / OnUpdateEditUndo
				}
				
				if (ImGui::MenuItem("Redo", get_hotkey_for_command("Redo").c_str(), nullptr, game::g_lastredo())) {
					cdeclcall(void, 0x428740); //cmainframe::OnEditRedo / OnUpdateEditRedo
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Copy", get_hotkey_for_command("Copy").c_str())) {
					mainframe_thiscall(void, 0x4286B0); //cmainframe::OnEditCopybrush
				}

				if (ImGui::MenuItem("Paste", get_hotkey_for_command("Paste").c_str())) {
					mainframe_thiscall(void, 0x4286D0); //cmainframe::OnEditPastebrush
				}
				
				if (ImGui::MenuItem("Delete", get_hotkey_for_command("DeleteSelection").c_str())) {
					cdeclcall(void, 0x425690); //cmainframe::OnSelectionDelete
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Map Info", get_hotkey_for_command("MapInfo").c_str())) {
					cdeclcall(void, 0x426C60); //cmainframe::OnEditMapinfo
				}

				if (ImGui::MenuItem("Entity Info")) {
					cdeclcall(void, 0x426D40); //cmainframe::OnEditEntityinfo
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Enter Prefab", get_hotkey_for_command("EnterPrefab").c_str())) {
					cdeclcall(void, 0x42BF70); //cmainframe::OnPrefabEnter
				}

				if (ImGui::MenuItem("Leave Prefab", get_hotkey_for_command("LeavePrefab").c_str())) {
					cdeclcall(void, 0x42BF80); //cmainframe::OnPrefabLeave
				}

				SEPERATORV(0.0f);
				
				if (ImGui::MenuItem("Preferences", get_hotkey_for_command("Preferences").c_str())) {
					mainframe_thiscall(void, 0x426950); //cmainframe::OnPrefs
				}

				ImGui::EndMenu(); // Edit
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::BeginMenu("Toggle"))
				{
					if (ImGui::MenuItem("Camera View", get_hotkey_for_command("ToggleCamera").c_str(), nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426A40); // cmainframe::OnTogglecamera
					}

					if (ImGui::MenuItem("Console View", get_hotkey_for_command("ViewConsole").c_str(), nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426A90); // cmainframe::OnToggleconsole
					}

					if (ImGui::MenuItem("XY View", get_hotkey_for_command("ToggleView").c_str(), nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426AE0); // cmainframe::OnToggleview
					}
					
					if (ImGui::MenuItem("Z View", get_hotkey_for_command("ToggleZ").c_str(), nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426B30); // cmainframe::OnTogglez
					}

					if (ImGui::MenuItem("XY Crosshair", get_hotkey_for_command("ToggleCrosshairs").c_str(), game::g_bCrossHairs)) {
						game::g_bCrossHairs ^= 1;
					}

					if (ImGui::MenuItem("Selected Outlines", get_hotkey_for_command("ToggleOutlineDraw").c_str(), !game::g_qeglobals->dontDrawSelectedOutlines)) {
						game::g_qeglobals->dontDrawSelectedOutlines ^= 1;
					}

					if (ImGui::MenuItem("Selected Tint", get_hotkey_for_command("ToggleTintDraw").c_str(), !game::g_qeglobals->dontDrawSelectedTint)) {
						game::g_qeglobals->dontDrawSelectedTint ^= 1;
					}

					if (ImGui::MenuItem("Entity View", get_hotkey_for_command("ViewEntityInfo").c_str())) {
						mainframe_thiscall(void, 0x423F00); // cmainframe::OnViewEntity
					}

					if (ImGui::MenuItem("Layers", get_hotkey_for_command("ToggleLayers").c_str())) {
						cdeclcall(void, 0x42BD10); // cmainframe::OnLayersDlg
					}

					if (ImGui::MenuItem("Filter Settings", get_hotkey_for_command("ViewFilters").c_str())) {
						mainframe_thiscall(void, 0x42B7A0); // cmainframe::OnFilterDlg
					}

					ImGui::EndMenu(); // Toggle
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Center", get_hotkey_for_command("CenterView").c_str())) {
					mainframe_thiscall(void, 0x423C50); // cmainframe::OnViewCenter
				}

				if (ImGui::MenuItem("Center 2D On Camera", get_hotkey_for_command("Center2DOnCamera").c_str())) {
					cdeclcall(void, 0x42A2D0); // cmainframe::OnCenter2DOnCamera
				}

				if (ImGui::MenuItem("Up Floor", get_hotkey_for_command("UpFloor").c_str())) {
					mainframe_thiscall(void, 0x424700); // cmainframe::OnViewUpfloor
				} TT("move up one floor, if there is one");

				if (ImGui::MenuItem("Down Floor", get_hotkey_for_command("DownFloor").c_str())) {
					mainframe_thiscall(void, 0x423ED0); // cmainframe::OnViewDownfloor
				} TT("move down one floor, if there is one");

				SEPERATORV(0.0f);
				
				if (ImGui::MenuItem("Next (XY, YZ, XZ)", get_hotkey_for_command("NextView").c_str())) {
					mainframe_thiscall(void, 0x426DB0); // cmainframe::OnViewNextview
				}

				if (ImGui::BeginMenu("XY Window"))
				{
					if (ImGui::MenuItem("XY")) {
						mainframe_thiscall(void, 0x424710); // cmainframe::OnViewXy
					}

					if (ImGui::MenuItem("YZ")) {
						mainframe_thiscall(void, 0x423FB0); // cmainframe::OnViewYz
					}

					if (ImGui::MenuItem("XZ")) {
						mainframe_thiscall(void, 0x424A80); // cmainframe::OnViewXz
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Show Names", 0, (game::g_qeglobals->d_xyShowFlags & 8) == 0)) {
						mainframe_thiscall(void, 0x42BA40); // cmainframe::OnSelectNames
					}

					if (ImGui::MenuItem("Show Angles", 0, (game::g_qeglobals->d_xyShowFlags & 2) == 0)) {
						mainframe_thiscall(void, 0x42BAA0); // cmainframe::OnSelectAngles
					}

					if (ImGui::MenuItem("Show Grid Blocks", 0, (game::g_qeglobals->d_xyShowFlags & 16) == 0)) {
						mainframe_thiscall(void, 0x42BB00); // cmainframe::OnSelectBlocks
					}

					if (ImGui::MenuItem("Show Connections", 0, (game::g_qeglobals->d_xyShowFlags & 4) == 0)) {
						mainframe_thiscall(void, 0x42BBC0); // cmainframe::OnSelectConnections
					}
					
					if (ImGui::MenuItem("Show Coordinates", 0, (game::g_qeglobals->d_xyShowFlags & 32) == 0)) {
						mainframe_thiscall(void, 0x42BB60); // cmainframe::OnSelectCoordinates
					}
					
					ImGui::EndMenu(); // XY Window
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Hide Selected", get_hotkey_for_command("HideSelected").c_str())) {
					cdeclcall(void, 0x42B6A0); // cmainframe::OnHideSelected
				}

				if (ImGui::MenuItem("Hide Unselected", get_hotkey_for_command("HideUnSelected").c_str())) {
					cdeclcall(void, 0x42B6C0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Last Hidden", get_hotkey_for_command("ShowLastHidden").c_str())) {
					cdeclcall(void, 0x42B6E0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Hidden", get_hotkey_for_command("ShowHidden").c_str())) {
					cdeclcall(void, 0x42B6D0); // cmainframe::OnShowHidden
				}
				
				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Draw Entities As"))
				{
					if (ImGui::MenuItem("Bounding Box", 0, game::g_PrefsDlg()->m_nEntityShowState == game::ENTITY_BOXED)) {
						mainframe_thiscall(void, 0x42B320); // cmainframe::OnViewEntitiesasBoundingbox
					}

					if (ImGui::MenuItem("Wireframe", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B410); // cmainframe::OnViewEntitiesasWireframe
					}

					if (ImGui::MenuItem("Selected Wireframe", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SELECTED_ONLY | game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B380); // cmainframe::OnViewEntitiesasSelectedwireframe
					}

					if (ImGui::MenuItem("Selected Skinned", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SELECTED_ONLY | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B350); // cmainframe::OnViewEntitiesasSelectedskinned
					}

					if (ImGui::MenuItem("Skinned", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3B0); // cmainframe::OnViewEntitiesasSkinned
					}

					if (ImGui::MenuItem("Skinned And Boxed", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_BOXED | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3E0); // cmainframe::OnViewEntitiesasSkinnedandboxed
					}
					
					ImGui::EndMenu(); // Draw Entities As
				}

				if (ImGui::BeginMenu("Light Preview"))
				{
					if (ImGui::MenuItem("Enable Light Preview", get_hotkey_for_command("LightPreviewToggle").c_str(), game::g_PrefsDlg()->enable_light_preview)) {
						mainframe_thiscall(void, 0x4240C0); // cmainframe::OnEnableLightPreview
					}

					if (ImGui::MenuItem("Enable Sun Preview", get_hotkey_for_command("LightPreviewSun").c_str(), game::g_PrefsDlg()->preview_sun_aswell)) {
						mainframe_thiscall(void, 0x424060); // cmainframe::OnPreviewSun
					}

					SEPERATORV(0.0f);
					
					if (ImGui::MenuItem("Start Previewing Selected", get_hotkey_for_command("LightPreviewStart").c_str())) {
						mainframe_thiscall(void, 0x424120); // cmainframe::OnStartPreviewSelected
					}

					if (ImGui::MenuItem("Stop Previewing Selected", get_hotkey_for_command("LightPreviewStop").c_str())) {
						mainframe_thiscall(void, 0x424170); // cmainframe::OnStopPreviewSelected
					}

					if (ImGui::MenuItem("Clear Preview List", get_hotkey_for_command("LightPreviewClear").c_str())) {
						mainframe_thiscall(void, 0x4241C0); // cmainframe::OnClearPreviewList
					}

					SEPERATORV(0.0f);
					
					if (ImGui::MenuItem("Preview At Max Intensity", get_hotkey_for_command("MaxLightIntensity").c_str(), !game::g_qeglobals->preview_at_max_intensity)) {
						cdeclcall(void, 0x425670); // cmainframe::OnPreviewAtMaxIntensity
					}

					if (ImGui::MenuItem("Show Regions For Selected", get_hotkey_for_command("LightPreviewRegions").c_str())) {
						mainframe_thiscall(void, 0x4241E0); // cmainframe::OnShowRegionsForSelected
					}

					ImGui::EndMenu(); // Light Preview
				}

				SEPERATORV(0.0f);

				//if (ImGui::MenuItem("Cubic Clip Zoom In", "CTRL-[")) {
				//	mainframe_thiscall(void, 0x428F10); // cmainframe::OnViewCubein
				//}

				//if (ImGui::MenuItem("Cubic Clip Zoom Out", "CTRL-]")) {
				//	mainframe_thiscall(void, 0x428F50); // cmainframe::OnViewCubeout
				//}

				IMGUI_MENU_WIDGET_SINGLE("Cubic Scale", ImGui::DragInt("", &game::g_PrefsDlg()->m_nCubicScale, 1, 1, 220));

				if (ImGui::MenuItem("Cubic Clipping", get_hotkey_for_command("ToggleCubicClip").c_str(), game::g_PrefsDlg()->m_bCubicClipping)) {
					mainframe_thiscall(void, 0x428F90); // cmainframe::OnViewCubicclipping
				}

				ImGui::EndMenu(); // View
			}

			if (ImGui::BeginMenu("Selection"))
			{
				if (ImGui::MenuItem("Drag Edges", get_hotkey_for_command("DragEdges").c_str())) {
					cdeclcall(void, 0x4257D0); // CMainFrame::OnSelectionDragedges
				}

				if (ImGui::MenuItem("Drag Vertices", get_hotkey_for_command("DragVertices").c_str())) {
					cdeclcall(void, 0x425840); // CMainFrame::OnSelectionDragVertices
				}

				SEPERATORV(0.0f);
				
				if (ImGui::MenuItem("Clone", get_hotkey_for_command("CloneSelection").c_str())) {
					cdeclcall(void, 0x425480); // CMainFrame::OnSelectionClone
				}

				if (ImGui::MenuItem("Deselect", get_hotkey_for_command("UnSelectSelection").c_str())) {
					mainframe_thiscall(void, 0x425740); // CMainFrame::OnSelectionDeselect
				}

				if (ImGui::MenuItem("Invert", get_hotkey_for_command("InvertSelection").c_str())) {
					cdeclcall(void, 0x42B6F0); // CMainFrame::OnSelectionInvert
				}
				
				SEPERATORV(0.0f);
				
				if (ImGui::BeginMenu("Flip"))
				{
					if (ImGui::MenuItem("Flip X")) {
						cdeclcall(void, 0x4250A0); // CMainFrame::OnBrushFlipx
					}

					if (ImGui::MenuItem("Flip Y")) {
						cdeclcall(void, 0x4250C0); // CMainFrame::OnBrushFlipy
					}

					if (ImGui::MenuItem("Flip Z")) {
						cdeclcall(void, 0x4250E0); // CMainFrame::OnBrushFlipz
					}

					ImGui::EndMenu(); // Flip
				}

				if (ImGui::BeginMenu("Rotate"))
				{
					if (ImGui::MenuItem("Rotate X")) {
						cdeclcall(void, 0x425100); // CMainFrame::OnBrushRotatex
					}

					if (ImGui::MenuItem("Rotate Y")) {
						cdeclcall(void, 0x425190); // CMainFrame::OnBrushRotatey
					}

					if (ImGui::MenuItem("Rotate Z", get_hotkey_for_command("RotateZ").c_str())) {
						cdeclcall(void, 0x425220); // CMainFrame::OnBrushRotatez
					}

					if (ImGui::MenuItem("Free Rotation", get_hotkey_for_command("MouseRotate").c_str())) {
						mainframe_thiscall(void, 0x428570); // CMainFrame::OnSelectMouserotate
					}

					if (ImGui::MenuItem("Arbitrary Rotation")) {
						cdeclcall(void, 0x425300); // CMainFrame::OnSelectionArbitraryrotation
					}
	
					ImGui::EndMenu(); // Rotate
				}

				if (ImGui::BeginMenu("Scale"))
				{
					if (ImGui::MenuItem("Lock X", 0, (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6))) {
						mainframe_thiscall(LRESULT, 0x428BC0); // CMainFrame::OnScalelockX
					}
					
					if (ImGui::MenuItem("Lock Y", 0, (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5))) {
						mainframe_thiscall(LRESULT, 0x428B60); // CMainFrame::OnScalelockY
					}

					if (ImGui::MenuItem("Lock Z", 0, (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3))) {
						mainframe_thiscall(LRESULT, 0x428B90); // CMainFrame::OnScalelockZ
					}

					if (ImGui::MenuItem("Arbitrary Scale")) {
						cdeclcall(void, 0x4283D0); // CMainFrame::OnSelectScale
					}
					
					ImGui::EndMenu(); // Scale
				}
				

				if (ImGui::MenuItem("Maya Export")) {
					cdeclcall(void, 0x4263C0); // CMainFrame::OnExportToMaya
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("CSG"))
				{
					if (ImGui::MenuItem("Hollow")) {
						cdeclcall(void, 0x425570); // CMainFrame::OnSelectionMakehollow
					}

					if (ImGui::MenuItem("Merge", get_hotkey_for_command("CSGMerge").c_str())) {
						cdeclcall(void, 0x4255D0); // CMainFrame::OnSelectionCsgmerge
					}

					if (ImGui::MenuItem("Auto Caulk", get_hotkey_for_command("AutoCaulk").c_str())) {
						cdeclcall(void, 0x425600); // CMainFrame::OnSelectionAutoCaulk
					}

					ImGui::EndMenu(); // CSG
				}

				if (ImGui::BeginMenu("Clipper"))
				{
					if (ImGui::MenuItem("Toggle Clipper", get_hotkey_for_command("ToggleClipper").c_str())) {
						mainframe_thiscall(void, 0x426510); // CMainFrame::OnViewClipper
					}

					if (ImGui::MenuItem("Clip Selected", get_hotkey_for_command("ClipSelected").c_str())) {
						mainframe_thiscall(void, 0x427170); // CMainFrame::OnClipSelected
					}

					if (ImGui::MenuItem("Split Selected", get_hotkey_for_command("SplitSelected").c_str())) {
						mainframe_thiscall(void, 0x4271D0); // CMainFrame::OnSplitSelected
					}

					if (ImGui::MenuItem("Flip Clip Orientation", get_hotkey_for_command("FlipClip").c_str())) {
						mainframe_thiscall(void, 0x427140); // CMainFrame::OnFlipClip
					}

					ImGui::EndMenu(); // Clipper
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Select Targetname")) {
					cdeclcall(void, 0x426390); // CMainFrame::OnSelectionTargetname
				}

				if (ImGui::MenuItem("Select Classname")) {
					cdeclcall(void, 0x4263A0); // CMainFrame::OnSelectionClassname
				}

				if (ImGui::MenuItem("Select By Key/Value", get_hotkey_for_command("SelectionKeyValue").c_str())) {
					cdeclcall(void, 0x4263B0); // CMainFrame::OnSelectionKeyValue
				}

				if (ImGui::MenuItem("Select Connected", get_hotkey_for_command("SelectConnectedEntities").c_str())) {
					cdeclcall(void, 0x425550); // CMainFrame::OnSelectConneted
				}

				if (ImGui::BeginMenu("Select"))
				{
					if (ImGui::MenuItem("Select Complete Tall")) {
						cdeclcall(void, 0x426340); // CMainFrame::OnSelectionSelectcompletetall
					}

					if (ImGui::MenuItem("Select Partial Tall")) {
						cdeclcall(void, 0x426360); // CMainFrame::OnSelectionSelectpartialtall
					}

					if (ImGui::MenuItem("Select Touching")) {
						cdeclcall(void, 0x426370); // CMainFrame::OnSelectionSelecttouching
					}

					if (ImGui::MenuItem("Select Inside")) {
						cdeclcall(void, 0x426350); // CMainFrame::OnSelectionSelectinside
					}

					ImGui::EndMenu(); // Clipper
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Connect Entities", get_hotkey_for_command("ConnectSelection").c_str())) {
					cdeclcall(void, 0x425510); // CMainFrame::OnSelectionConnect
				}

				if (ImGui::MenuItem("Link Entities", get_hotkey_for_command("LinkSelected").c_str())) {
					cdeclcall(void, 0x425500); // CMainFrame::OnLinkSelected
				}

				if (ImGui::MenuItem("Ungroup Entity")) {
					cdeclcall(void, 0x426380); // CMainFrame::OnSelectionUngroupentity
				}

				if (ImGui::MenuItem("Cycle Group Selection", get_hotkey_for_command("Patch TAB").c_str())) {
					cdeclcall(void, 0x42A9E0); // CMainFrame::OnPatchTab
				} TT("Cycle between all brushes/patches of a selected group/brushmodel");

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Make Detail", get_hotkey_for_command("MakeDetail").c_str())) {
					cdeclcall(void, 0x4261C0); // CMainFrame::OnSelectionMakeDetail
				}

				if (ImGui::MenuItem("Make Structural", get_hotkey_for_command("MakeStructural").c_str())) {
					cdeclcall(void, 0x426200); // CMainFrame::OnSelectionMakeStructural
				}
				
				if (ImGui::MenuItem("Make Weapon Clip", get_hotkey_for_command("MakeWeaponClip").c_str())) {
					cdeclcall(void, 0x426240); // CMainFrame::OnSelectionMakeWeaponclip
				}
				
				if (ImGui::MenuItem("Make Non-Colliding", get_hotkey_for_command("MakeNonColliding").c_str())) {
					cdeclcall(void, 0x426280); // CMainFrame::OnSelectionMakeNonColliding
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Make Split Coplanar Geo")) {
					cdeclcall(void, 0x4262C0); // CMainFrame::OnSelectionMakeSplitCoplanar
				}

				if (ImGui::MenuItem("Make Don't Split Coplanar Geo")) {
					cdeclcall(void, 0x426300); // CMainFrame::OnSelectionMakeDontSplitCoplanar
				}
				
				ImGui::EndMenu(); // Selection
			}

			if (ImGui::BeginMenu("Grid"))
			{
				if (ImGui::MenuItem("Grid 0.5", get_hotkey_for_command("SetGridPointFive").c_str(), game::g_qeglobals->d_gridsize == GRID_05)) {
					set_grid_size(GRID_05);
				}

				if (ImGui::MenuItem("Grid 1", get_hotkey_for_command("SetGrid1").c_str(), game::g_qeglobals->d_gridsize == GRID_1)) {
					set_grid_size(GRID_1);
				}

				if (ImGui::MenuItem("Grid 2", get_hotkey_for_command("SetGrid2").c_str(), game::g_qeglobals->d_gridsize == GRID_2)) {
					set_grid_size(GRID_2);
				}

				if (ImGui::MenuItem("Grid 4", get_hotkey_for_command("SetGrid4").c_str(), game::g_qeglobals->d_gridsize == GRID_4)) {
					set_grid_size(GRID_4);
				}

				if (ImGui::MenuItem("Grid 8", get_hotkey_for_command("SetGrid8").c_str(), game::g_qeglobals->d_gridsize == GRID_8)) {
					set_grid_size(GRID_8);
				}

				if (ImGui::MenuItem("Grid 16", get_hotkey_for_command("SetGrid16").c_str(), game::g_qeglobals->d_gridsize == GRID_16)) {
					set_grid_size(GRID_16);
				}

				if (ImGui::MenuItem("Grid 32", get_hotkey_for_command("SetGrid32").c_str(), game::g_qeglobals->d_gridsize == GRID_32)) {
					set_grid_size(GRID_32);
				}

				if (ImGui::MenuItem("Grid 64", get_hotkey_for_command("SetGrid64").c_str(), game::g_qeglobals->d_gridsize == GRID_64)) {
					set_grid_size(GRID_64);
				}

				if (ImGui::MenuItem("Grid 128", 0, game::g_qeglobals->d_gridsize == GRID_128)) {
					set_grid_size(GRID_128);
				}

				if (ImGui::MenuItem("Grid 256", get_hotkey_for_command("SetGrid256").c_str(), game::g_qeglobals->d_gridsize == GRID_256)) {
					set_grid_size(GRID_256);
				}

				if (ImGui::MenuItem("Grid 512", get_hotkey_for_command("SetGrid512").c_str(), game::g_qeglobals->d_gridsize == GRID_512)) {
					set_grid_size(GRID_512);
				}

				if (ImGui::MenuItem("Snap To Grid", get_hotkey_for_command("ToggleSnapToGrid").c_str(), !game::g_PrefsDlg()->m_bNoClamp)) {
					mainframe_thiscall(void, 0x428380); // CMainFrame::OnSnaptogrid
				}

				ImGui::EndMenu(); // Grid
			}
			

			if (ImGui::BeginMenu("Textures"))
			{
				if (ImGui::MenuItem("Show All", get_hotkey_for_command("ShowAllTextures").c_str())) {
					cdeclcall(void, 0x42B440); // CMainFrame::OnTexturesShowall
				}

				if (ImGui::MenuItem("Show In Use", get_hotkey_for_command("ShowTexturesInUse").c_str())) {
					mainframe_thiscall(void, 0x424B20); // CMainFrame::OnTexturesShowinuse
				}

				if (ImGui::MenuItem("Surface Inspector", get_hotkey_for_command("SurfaceInspector").c_str())) {
					cdeclcall(void, 0x424B60); // CMainFrame::OnTexturesInspector
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Find / Replace")) {
					cdeclcall(void, 0x428B40); // CMainFrame::OnTextureReplaceall
				}

				if (ImGui::BeginMenu("Render Method"))
				{
					if (ImGui::MenuItem("Wireframe", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_WIREFRAME))  {
						set_render_method(RM_WIREFRAME);
					}

					if (ImGui::MenuItem("Fullbright", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_FULLBRIGHT)) {
						set_render_method(RM_FULLBRIGHT);
					}

					if (ImGui::MenuItem("Normal-Based Fake Lighting", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_NORMALFAKELIGHT)) {
						set_render_method(RM_NORMALFAKELIGHT);
					}

					if (ImGui::MenuItem("View-Based Fake Lighting", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_VIEWFAKELIGHT)) {
						set_render_method(RM_VIEWFAKELIGHT);
					}

					if (ImGui::MenuItem("Case Textures", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_CASETEXTURES)) {
						set_render_method(RM_CASETEXTURES);
					}
					
					ImGui::EndMenu(); // Render Method
				}

				if (ImGui::BeginMenu("Texture Filter"))
				{
					if (ImGui::MenuItem("Unchanged", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 0)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 0);
					}

					if (ImGui::MenuItem("Force Trilinear", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 1)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 1);
					}

					if (ImGui::MenuItem("Force Bilinear", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 2)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 2);
					}

					if (ImGui::MenuItem("Force Mipmaps Off", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 3)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 3);
					}
					
					ImGui::EndMenu(); // Texture Filter
				}

				if (ImGui::BeginMenu("Texture Resolution"))
				{
					if (ImGui::MenuItem("Maximum", 0, game::g_qeglobals->d_picmip == 0)) {
						set_texture_resolution(0);
					}

					if (ImGui::MenuItem("High", 0, game::g_qeglobals->d_picmip == 1)) {
						set_texture_resolution(1);
					}
					
					if (ImGui::MenuItem("Normal", 0, game::g_qeglobals->d_picmip == 2)) {
						set_texture_resolution(2);
					}

					if (ImGui::MenuItem("Low", 0, game::g_qeglobals->d_picmip == 3)) {
						set_texture_resolution(3);
					}
					
					ImGui::EndMenu(); // Texture Resolution
				}

				if (ImGui::BeginMenu("Texture Window Scale"))
				{
					if (ImGui::MenuItem("200%", 0, game::g_PrefsDlg()->m_nTextureWindowScale == 200)) {
						mainframe_thiscall(void, 0x42B020); // CMainFrame::OnTexturesTexturewindowscale200
					}

					if (ImGui::MenuItem("100%", 0, game::g_PrefsDlg()->m_nTextureWindowScale == 100)) {
						mainframe_thiscall(void, 0x42B000); // CMainFrame::OnTexturesTexturewindowscale100
					}

					if (ImGui::MenuItem("50%", 0, game::g_PrefsDlg()->m_nTextureWindowScale == 50)) {
						mainframe_thiscall(void, 0x42B060); // CMainFrame::OnTexturesTexturewindowscale50
					}

					if (ImGui::MenuItem("25%", 0, game::g_PrefsDlg()->m_nTextureWindowScale == 25)) {
						mainframe_thiscall(void, 0x42B040); // CMainFrame::OnTexturesTexturewindowscale25
					}

					if (ImGui::MenuItem("10%", 0, game::g_PrefsDlg()->m_nTextureWindowScale == 10)) {
						mainframe_thiscall(void, 0x42AFE0); // CMainFrame::OnTexturesTexturewindowscale10
					}
					
					ImGui::EndMenu(); // Texture Window Scale
				}

				if (ImGui::BeginMenu("Texture Lock"))
				{
					if (ImGui::MenuItem("Moves", get_hotkey_for_command("ToggleTexMoveLock").c_str(), game::g_PrefsDlg()->m_bTextureLock == 1)) {
						mainframe_thiscall(void, 0x426B80); // CMainFrame::OnToggleLockMoves
					}

					if (ImGui::MenuItem("Rotations", get_hotkey_for_command("ToggleTexRotateLock").c_str(), game::g_PrefsDlg()->m_bRotateLock == 1)) {
						mainframe_thiscall(void, 0x429230); // CMainFrame::OnToggleLockRotations
					}

					if (ImGui::MenuItem("Lightmaps", get_hotkey_for_command("ToggleLightmapLock").c_str(), game::g_PrefsDlg()->m_bLightmapLock == 1)) {
						mainframe_thiscall(void, 0x426BF0); // CMainFrame::OnToggleLockLightmap
					}
					
					ImGui::EndMenu(); // Texture Lock
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Layered Materials"))
				{
					if (ImGui::MenuItem("Toogle Tool Window", get_hotkey_for_command("ToggleLayeredMaterialWnd").c_str())) {
						cdeclcall(void, 0x42BFE0); // CMainFrame::OnToggleLayeredMaterials
					}

					if (ImGui::MenuItem("Save", get_hotkey_for_command("SaveLayeredMaterials").c_str())) {
						cdeclcall(void, 0x42C020); // CMainFrame::OnSaveLayeredMaterials
					}
					
					ImGui::EndMenu(); // Layered Materials
				}

				if (ImGui::BeginMenu("Edit Layer"))
				{
					if (ImGui::MenuItem("Cycle", get_hotkey_for_command("TexLayerCycle").c_str())) {
						cdeclcall(void, 0x424010); // CMainFrame::OnEditLayerCycle
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Material", get_hotkey_for_command("TexLayerMaterial").c_str(), game::g_qeglobals->current_edit_layer == 0)) {
						cdeclcall(void, 0x424030); // CMainFrame::OnEditLayerMaterial
					}

					if (ImGui::MenuItem("Lightmap", get_hotkey_for_command("TexLayerLightmap").c_str(), game::g_qeglobals->current_edit_layer == 1)) {
						cdeclcall(void, 0x424040); // CMainFrame::OnEditLayerLightmap
					}

					if (ImGui::MenuItem("Smoothing", 0, game::g_qeglobals->current_edit_layer == 2)) {
						cdeclcall(void, 0x424050); // CMainFrame::OnEditLayerSmoothing
					}
					
					ImGui::EndMenu(); // Edit Layer
				}

				if (ImGui::MenuItem("Refresh Textures", get_hotkey_for_command("RefreshTextures").c_str())) {
					cdeclcall(void, 0x428B50); // CMainFrame::OnTextureRefresh
				}

				if (ImGui::BeginMenu("Usage Filter"))
				{
					for (std::uint8_t i = 0; i < game::texWndGlob_usageCount; i++)
					{
						const char* name = game::filter_usage_array[i].name;
						if (name)
						{
							if (ImGui::MenuItem(name, 0, game::texWndGlob_usageFilter == i))
							{
								game::texWndGlob_usageFilter = i;
								game::g_nUpdateBits |= W_TEXTURE;
							}
						}
						else if (game::filter_usage_array[i].index == -1)
						{
							SEPERATORV(0.0f);
						}
					}
					
					ImGui::EndMenu(); // Usage Filter
				}

				if (ImGui::BeginMenu("Locale Filter"))
				{
					for (std::uint8_t i = 0; i < game::texWndGlob_localeCount; i++)
					{
						const char* name = game::filter_locale_array[i].name;
						if (name)
						{
							if (ImGui::MenuItem(name, 0, game::texWndGlob_localeFilter == i))
							{
								game::texWndGlob_localeFilter = i;
								game::g_nUpdateBits |= W_TEXTURE;
							}
						}
						else if(game::filter_locale_array[i].index == -1)
						{
							SEPERATORV(0.0f);
						}
					}
					
					ImGui::EndMenu(); // Locale Filter
				}

				if (ImGui::BeginMenu("Surface Type Filter"))
				{
					for(std::uint8_t i = 0; i < 29; i++) // hardcoded value
					{
						const char* name = game::filter_surfacetype_array[i].name;
						if (name)
						{
							if (ImGui::MenuItem(name, 0, game::texWndGlob_surfaceTypeFilter == i))
							{
								game::texWndGlob_surfaceTypeFilter = i;
								game::g_nUpdateBits |= W_TEXTURE;
							}
						}
					}
					
					ImGui::EndMenu(); // Surface Type Filter
				}

				ImGui::EndMenu(); // Textures
			}

			
			if (ImGui::BeginMenu("Misc"))
			{
				if (ImGui::MenuItem("Colors")) {
					gui::toggle(context.m_colors, 0, true);
				}

				if (ImGui::MenuItem("Find Brush")) {
					cdeclcall(void, 0x424B80); // CMainFrame::OnMiscFindbrush
				}

				if (ImGui::MenuItem("Got To Position")) {
					cdeclcall(void, 0x424BA0); // CMainFrame::OnMiscGoToPosition
				}

				if (ImGui::MenuItem("-> Leak Spot / Error", get_hotkey_for_command("NextLeakSpot").c_str())) {
					cdeclcall(void, 0x424BC0); // CMainFrame::OnMiscNextleakspot
				}

				if (ImGui::MenuItem("<- Leak Spot / Error", get_hotkey_for_command("PrevLeakSpot").c_str())) {
					cdeclcall(void, 0x424BE0); // CMainFrame::OnMiscPreviousleakspot
				}

				if (ImGui::MenuItem("Print XY View")) {
					cdeclcall(void, 0x424C00); // CMainFrame::OnMiscPrintxy
				}

				if (ImGui::MenuItem("Entity Color Dialog", get_hotkey_for_command("EntityColor").c_str())) {
					cdeclcall(void, 0x424C10); // CMainFrame::OnMiscSelectentitycolor
				}

				if (ImGui::MenuItem("Script Group Dialog")) {
					cdeclcall(void, 0x424E20); // CMainFrame::OnScriptGroup
				}

				if (ImGui::MenuItem("Delete Exportables")) {
					cdeclcall(void, 0x424E30); // CMainFrame::OnDeleteExportables
				}

				if (ImGui::MenuItem("Vehicle Group Dialog", get_hotkey_for_command("VehicleGroup").c_str())) {
					cdeclcall(void, 0x42BD50); // CMainFrame::OnMiscVehicleGroup
				}

				if (ImGui::MenuItem("Dyn Entity Dialog", get_hotkey_for_command("DynEntities").c_str())) {
					cdeclcall(void, 0x42BD90); // CMainFrame::OnMiscDynEntities
				}
				
				if (ImGui::MenuItem("Models Replace Dialog")) {
					cdeclcall(void, 0x42BF00); // CMainFrame::OnReplaceModels
				}

				// no clue what that does
				if (ImGui::MenuItem("Cycle Preview Models", get_hotkey_for_command("TogglePreviewModels").c_str())) {
					mainframe_thiscall(void, 0x42BDD0); // CMainFrame::OnMiscCyclePreviewModels
				}
				
				ImGui::EndMenu(); // Misc
			}

			
			if (ImGui::BeginMenu("Region"))
			{
				if(ImGui::MenuItem("Region Off")) {
					cdeclcall(void, 0x4252B0); // CMainFrame::OnRegionOff
				}

				if (ImGui::MenuItem("Set XY")) {
					cdeclcall(void, 0x4252F0); // CMainFrame::OnRegionSetxy
				}

				if (ImGui::MenuItem("Set Tall Brush")) {
					cdeclcall(void, 0x4252E0); // CMainFrame::OnRegionSettallbrush
				}

				if (ImGui::MenuItem("Set Brush")) {
					cdeclcall(void, 0x4252C0); // CMainFrame::OnRegionSetbrush
				}

				if (ImGui::MenuItem("Set Selected Brush")) {
					cdeclcall(void, 0x4252D0); // CMainFrame::OnRegionSetselection
				}
				
				ImGui::EndMenu(); // Region
			}

			
			if (ImGui::BeginMenu("Brush"))
			{
				if (ImGui::MenuItem("Arbitrary Sided Cylinder")) {
					cdeclcall(void, 0x424EE0); // CMainFrame::OnBrushArbitrarysided
				}

				if (ImGui::MenuItem("Cone")) {
					cdeclcall(void, 0x429170); // CMainFrame::OnBrushMakecone
				}

				if (ImGui::MenuItem("Sphere")) {
					cdeclcall(void, 0x42B630); // CMainFrame::OnBrushPrimitivesSphere
				}

				if (ImGui::MenuItem("Box")) {
					cdeclcall(void, 0x429200); // CMainFrame::OnMakePhysBox
				}

				if (ImGui::MenuItem("Cylinder")) {
					cdeclcall(void, 0x4291D0); // CMainFrame::OnMakePhysCylinder
				}
				
				ImGui::EndMenu(); // Brush
			}

			
			if (ImGui::BeginMenu("Patch"))
			{
				if (ImGui::MenuItem("Simple Curve Patch")) {
					cdeclcall(void, 0x429A20); // CMainFrame::OnCurveSimplepatchmesh
				}

				if (ImGui::MenuItem("Simple Terrain Patch")) {
					cdeclcall(void, 0x429DA0); // CMainFrame::OnTerrainSimplepatchmesh
				}

				if (ImGui::MenuItem("Curve To Terrain")) {
					cdeclcall(void, 0x429B30); // CMainFrame::OnCurveToTerrain
				}
				
				if (ImGui::MenuItem("Faces To Terrain")) {
					cdeclcall(void, 0x429BE0); // CMainFrame::OnFaceToTerrain
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Primitives"))
				{
					if (ImGui::MenuItem("Bevel")) {
						cdeclcall(void, 0x42A450); // CMainFrame::OnCurvePatchbevel
					}

					if (ImGui::MenuItem("Square Bevel")) {
						cdeclcall(void, 0x42B5E0); // CMainFrame::OnCuveSquareBevel
					}

					if (ImGui::MenuItem("End Cap")) {
						cdeclcall(void, 0x42A4A0); // CMainFrame::OnCurvePatchendcap
					}

					if (ImGui::MenuItem("Square End Cap")) {
						cdeclcall(void, 0x42B590); // CMainFrame::OnCurveSquareEndCap
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Cylinder")) {
						cdeclcall(void, 0x42A3B0); // CMainFrame::OnCurvePatchtube
					}

					if (ImGui::MenuItem("Dense Cylinder")) {
						cdeclcall(void, 0x42AB90); // CMainFrame::OnCurvePatchdensetube
					}

					if (ImGui::MenuItem("Very Dense Cylinder")) {
						cdeclcall(void, 0x42AC40); // CMainFrame::OnCurvePatchverydensetube
					}

					if (ImGui::MenuItem("Square Cylinder")) {
						cdeclcall(void, 0x42AF00); // CMainFrame::OnCurvePatchsquare
					}

					if (ImGui::MenuItem("Cone")) {
						cdeclcall(void, 0x42A360); // CMainFrame::OnCurvePatchcone
					}

					// not implemented
					//if (ImGui::MenuItem("Sphere")) {
					//	cdeclcall(void, 0x1); // CMainFrame::
					//}
					
					ImGui::EndMenu(); // Primitives
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Advanced Edit Dialog", get_hotkey_for_command("AdvancedCurveEdit").c_str())) {
					cdeclcall(void, 0x42BC90); // CMainFrame::OnAdvancedEditDlg
				}

				if (ImGui::BeginMenu("Insert"))
				{
					if (ImGui::MenuItem("Insert 2 Columns")) {
						cdeclcall(void, 0x42A740); // CMainFrame::OnCurveInsertInsertcolumn
					}

					if (ImGui::MenuItem("Insert 2 Rows")) {
						cdeclcall(void, 0x42A790); // CMainFrame::OnCurveInsertInsertrow
					}

					if (ImGui::MenuItem("Add 2 Columns", get_hotkey_for_command("IncPatchColumn").c_str())) {
						cdeclcall(void, 0x42A6A0); // CMainFrame::OnCurveInsertAddcolumn
					}

					if (ImGui::MenuItem("Add 2 Rows", get_hotkey_for_command("IncPatchRow").c_str())) {
						cdeclcall(void, 0x42A6F0); // CMainFrame::OnCurveInsertAddrow
					}

					if (ImGui::MenuItem("Add Terrain Row / Column", get_hotkey_for_command("AddTerrainRow").c_str())) {
						cdeclcall(void, 0x42B080); // CMainFrame::OnAddTerrainRowColumn
					}

					if (ImGui::MenuItem("Extrude Terrain Row", get_hotkey_for_command("ExtrudeTerrainRow").c_str())) {
						cdeclcall(void, 0x42B0A0); // CMainFrame::ExtrudeTerrainRow
					}
					
					ImGui::EndMenu(); // Insert
				}

				if (ImGui::BeginMenu("Delete"))
				{
					if (ImGui::MenuItem("First 2 Columns", get_hotkey_for_command("DecPatchColumn").c_str())) {
						cdeclcall(void, 0x42A810); // CMainFrame::OnCurveDeleteFirstcolumn
					}

					if (ImGui::MenuItem("First 2 Rows", get_hotkey_for_command("DecPatchRow").c_str())) {
						cdeclcall(void, 0x42A860); // CMainFrame::OnCurveDeleteFirstrow
					}

					if (ImGui::MenuItem("Last 2 Columns")) {
						cdeclcall(void, 0x42A8B0); // CMainFrame::OnCurveDeleteLastcolumn
					}

					if (ImGui::MenuItem("Last 2 Rows")) {
						cdeclcall(void, 0x42A900); // CMainFrame::OnCurveDeleteLastrow
					}

					if (ImGui::MenuItem("Remove Terrain Row / Column", get_hotkey_for_command("RemoveTerrainRow").c_str())) {
						cdeclcall(void, 0x42B0B0); // CMainFrame::OnRemoveTerrainRowColumn
					}
					
					ImGui::EndMenu(); // Delete
				}

				if (ImGui::BeginMenu("Matrix"))
				{
					if (ImGui::MenuItem("Transpose", get_hotkey_for_command("PatchMatrixTranspose").c_str())) {
						cdeclcall(void, 0x42B1E0); // CMainFrame::OnCurveMatrixTranspose
					}
					
					if (ImGui::MenuItem("Re-disperse Vertices", get_hotkey_for_command("RedisperseVertices").c_str())) {
						cdeclcall(void, 0x42A270); // CMainFrame::OnRedistPatchPoints
					}
					
					if (ImGui::MenuItem("Re-disperse Columns", get_hotkey_for_command("RedisperseCols").c_str())) {
						cdeclcall(void, 0x42AD80); // CMainFrame::OnCurveRedisperseCols
					}

					if (ImGui::MenuItem("Re-disperse Rows", get_hotkey_for_command("RedisperseRows").c_str())) {
						cdeclcall(void, 0x42AD90); // CMainFrame::OnCurveRedisperseRows
					}

					ImGui::EndMenu(); // Matrix
				}

				if (ImGui::MenuItem("Split", get_hotkey_for_command("SplitPatch").c_str())) {
					cdeclcall(void, 0x42B0C0); // CMainFrame::OnSplitPatch
				}

				// TODO: add 'InvertCurveTextureX' and 'InvertCurveTextureY'
				if (ImGui::MenuItem("Invert", get_hotkey_for_command("InvertCurve").c_str())) {
					cdeclcall(void, 0x42A7E0); // CMainFrame::OnCurveNegative
				}

				if (ImGui::MenuItem("Thicken", get_hotkey_for_command("ThickenPatch").c_str())) {
					cdeclcall(void, 0x42B0D0); // CMainFrame::OnCurveThicken
				}

				if (ImGui::MenuItem("Cap", get_hotkey_for_command("CapCurrentCurve").c_str())) {
					cdeclcall(void, 0x42AD40); // CMainFrame::OnCurveCap
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Turn Terrain Edges")) {
					cdeclcall(void, 0x4294E0); // CMainFrame::OnTurnTerrainEdges
				}

				if (ImGui::MenuItem("Lock Vertices", get_hotkey_for_command("ToggleLockPatchVertices").c_str())) {
					mainframe_thiscall(void, 0x42B4F0); // CMainFrame::ToggleLockPatchVertMode
				}

				if (ImGui::MenuItem("Unlock Vertices", get_hotkey_for_command("ToggleUnlockPatchVertices").c_str())) {
					mainframe_thiscall(void, 0x42B510); // CMainFrame::ToggleUnlockPatchVertMode
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Cycle Cap Texture", get_hotkey_for_command("CycleCapTexturePatch").c_str())) {
					cdeclcall(void, 0x42B1A0); // CMainFrame::OnCurveCyclecap
				}
				
				if (ImGui::MenuItem("Vert Edit Dialog", get_hotkey_for_command("VertEdit").c_str())) {
					cdeclcall(void, 0x42BCD0); // CMainFrame::OnVertexEditDlg
				}

				if (ImGui::MenuItem("Cap Texture", get_hotkey_for_command("ApplyPatchCap").c_str())) {
					cdeclcall(void, 0x42AE50); // CMainFrame::OnPatchCap
				}
				
				ImGui::EndMenu(); // Patch
			}
			
		
			if (ImGui::BeginMenu("Help"))
			{
				// TODO! implement help

				if (ImGui::MenuItem("Command List")) {
					cdeclcall(void, 0x426E00); // CMainFrame::OnHelpCommandlist
				}

				if (ImGui::MenuItem("Command Binds")) {
					gui::toggle(context.m_cmdbinds, 0, true);
				}

				// TODO! implement about
				
				
				ImGui::EndMenu(); // Help
			}

			ImGui::PopStyleVar(2); // ImGuiStyleVar_WindowPadding | ImGuiStyleVar_ItemSpacing
			ImGui::EndMenuBar();
		}

		
		// *
		// create default docking layout

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("cxywnd_dockspace_layout");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode); // | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode);

			// init dockspace once
			if (!context.m_toolbar.one_time_init)
			{
				context.m_toolbar.one_time_init = true;

				// clear any previous layout
				ImGui::DockBuilderRemoveNode(dockspace_id);
				ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

				// create toolbar dockspace
				auto dock_toolbar = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.01f, nullptr, &dockspace_id);
				ImGui::DockBuilderSetNodeSize(dock_toolbar, ImVec2(viewport->Size.x, 16)); // 36

				// ^ undockable toolbar without tabbar
				ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_toolbar);
				node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoDockingSplitMe | ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoResizeY;
				
				// split the resulting node (dockspace_id) -> dockspace_id will be everything to the right of the split
				const float split_ratio_left = viewport->Size.x < 1000.0f ? 0.5f : (1.0f / viewport->Size.x) * 500.0f;
				auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, split_ratio_left, nullptr, &dockspace_id);
				
				// we now dock our windows into the docking node we made above
				ImGui::DockBuilderDockWindow("toolbar_xywnd", dock_toolbar);
				ImGui::DockBuilderDockWindow("Colors##xywnd", dock_id_left);
				ImGui::DockBuilderDockWindow("Hotkeys##xywnd", dock_id_left);
				
				ImGui::DockBuilderFinish(dockspace_id);
			}
		}

		ImGui::End();
		
		// ----------------------------

		// *
		// create toolbar window

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 4.0f));		_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));	_stylevars++;

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));		_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(100, 100, 100, 70));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));		_stylecolors++;

		//ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 36.0f), ImVec2(FLT_MAX, 36.0f));
		
		ImGui::Begin("toolbar_xywnd", nullptr, 
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar
		);

		
		// *
		// sizes (not really needed because the toolbar is pre-docked)

		auto toolbar_menu = context.m_toolbar;
		
		toolbar_menu.position[0] = viewport->Pos.x;
		toolbar_menu.position[1] = viewport->Pos.y;
		toolbar_menu.size[0] = viewport->Size.x; //static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width); //ImGui::GetWindowWidth();
		toolbar_menu.size[1] = 40.0f;

		//ImGui::SetWindowPos(ImVec2(toolbar_menu.position[0], toolbar_menu.position[1]), ImGuiCond_FirstUseEver);
		//ImGui::SetWindowSize(ImVec2(toolbar_menu.size[0], toolbar_menu.size[1]), ImGuiCond_FirstUseEver);

		
		// *
		// gui elements

		const ImVec2 toolbar_imagebutton_size = ImVec2(28.0f, 28.0f);

		
		// lock x
		if (const auto image = game::Image_RegisterHandle("lock_x"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// cmainframe::OnScalelockX
				mainframe_thiscall(LRESULT, 0x428BC0);
			} TT("lock grid along the x-axis");
		}

		
		// lock y
		if (const auto image = game::Image_RegisterHandle("lock_y"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// cmainframe::OnScalelockY
				mainframe_thiscall(LRESULT, 0x428B60);
			} TT("lock grid along the y-axis");
		}

		
		// lock z
		if (const auto image = game::Image_RegisterHandle("lock_z"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// cmainframe::OnScalelockZ
				mainframe_thiscall(LRESULT, 0x428B90);
			} TT("lock grid along the z-axis");
		}

		ImGui::SameLine();
		if (ImGui::Button("Reload Images")) { game::R_ReloadImages(); }
		
		// TODO! - remove me
		ImGui::SameLine();
		if (ImGui::Button("Lock X")) { mainframe_thiscall(LRESULT, 0x428BC0); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Y")) { mainframe_thiscall(LRESULT, 0x428B60); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Z")) { mainframe_thiscall(LRESULT, 0x428B90); }


		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if(ImGui::Button("Switch Console <-> Splitter"))
		{
			const auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);
			
			const auto pTop = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 0, 0);
			const auto pBottom = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 1, 0);

			if(!pTop || !pBottom)
			{
				goto END_GUI;
			}

			const auto _top = pTop->m_hWnd;
			const auto idTop = GetWindowLongA(_top, GWL_ID);

			const auto _bottom = pBottom->m_hWnd;
			const auto idBottom = GetWindowLongA(_bottom, GWL_ID);

			SetWindowLongA(_top, GWL_ID, idBottom);
			SetWindowLongA(_bottom, GWL_ID, idTop);

			vtable->RecalcLayout(&cmainframe::activewnd->m_wndSplit);
		}

		// TODO! - remove me
		ImGui::SameLine();
		if (ImGui::Button("Set Statustext"))
		{
			const auto vtable = reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable);
			vtable->SetStatusText(&cmainframe::activewnd->m_wndStatusBar, 0x75);
		}

		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if(ImGui::Button("Toggle Toolbar"))
		{
			typedef void(__thiscall* CFrameWnd_ShowControlBar_t)(CFrameWnd*, CControlBar*, BOOL bShow, BOOL bDelay);
			CFrameWnd_ShowControlBar_t CFrameWnd_ShowControlBar = reinterpret_cast<CFrameWnd_ShowControlBar_t>(0x59E9DD);

			auto vtable = reinterpret_cast<CToolBar_vtbl*>(cmainframe::activewnd->m_wndToolBar.__vftable);
			CFrameWnd_ShowControlBar(cmainframe::activewnd, &cmainframe::activewnd->m_wndToolBar, vtable->IsVisible(&cmainframe::activewnd->m_wndToolBar) ? 0 : 1, 1);
		}

		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if (ImGui::Button("Toggle Menubar"))
		{
			if (!ggui::mainframe_menubar_enabled)
			{
				command::execute("menubar_show");
			}
			else
			{
				command::execute("menubar_hide");
			}

			game::CPrefsDlg_SavePrefs();
		}

		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if (ImGui::Button("Reload Commandmap"))
		{
			// CMainFrame::LoadCommandMap
			cdeclcall(void, 0x421230);
		}

	END_GUI:
		ImGui::PopStyleColor(_stylecolors);
		ImGui::PopStyleVar(_stylevars);
		ImGui::End();
	}

	
	// *
	// main rendering loop (d3d9ex::d3d9device::EndScene())
	void gui::render_loop()
	{
		// *
		// menus

		//gui::any_open_menus();

		/* - radiant draws multiple windows using d3d => multiple endscene / present calls
		 * - each window should have its own imgui context
		 * - use dx->targetWindowIndex to distinguish between windows
		 */
		
		if (game::dx->targetWindowIndex == ggui::CCAMERAWND)
		{
			if(!ggui::state.ccamerawnd.context_initialized)
			{
				imgui_init_ccamerawnd();

				// allow 1 frame to pass
				return;
			}

			// set context
			IMGUI_BEGIN_CCAMERAWND;

			// begin context frame
			gui::begin_frame();

			// TODO! always show demo window (for now)
			//ImGui::ShowDemoWindow(nullptr);

			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.ccamerawnd.m_demo,
				ImGui::ShowDemoWindow(&ggui::state.ccamerawnd.m_demo.menustate), nullptr);
			
			// end the current context frame
			goto END_FRAME;
		}

		// ------------------------------------------------------
		
		if (game::dx->targetWindowIndex == ggui::CXYWND) //if (game::glob::gui_present.cxywnd)
		{
			if (!ggui::state.cxywnd.context_initialized)
			{
				imgui_init_cxywnd();

				// allow 1 frame to pass
				return;
			}

			IMGUI_BEGIN_CXYWND;

			// global style vars for current context
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameBorderSize = 0.0f;
			style.Colors[ImGuiCol_WindowBg] = ImGui::ToImVec4(dvars::gui_window_bg_color->current.vector);

			// begin context frame
			gui::begin_frame();

			// create all dockable windows in cxywnd_gui
			cxywnd_gui(ggui::state.cxywnd);

			//color_menu(ggui::state.cxywnd.m_colors); // always open
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_colors, 
				color_menu(ggui::state.cxywnd.m_colors), nullptr);

			// toggleable command bind menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_cmdbinds, 
				cmdbinds_menu(ggui::state.cxywnd.m_cmdbinds), cmdbinds_on_close());

			// toggleable command bind helper menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_cmdbinds_helper,
				cmdbinds_helper_menu(ggui::state.cxywnd.m_cmdbinds_helper), nullptr);
			
			// toggleable demo menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_demo,
					ImGui::ShowDemoWindow(&ggui::state.cxywnd.m_demo.menustate), nullptr);

			// end the current context frame
			goto END_FRAME;
		}

		return;

		// ------------

	END_FRAME:
		gui::end_frame();

		ImGuiIO& io = ImGui::GetIO();

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	// *
	// shutdown imgui (d3d9ex::d3d9device::Release)
	void gui::shutdown()
	{
		if(ggui::state.ccamerawnd.context_initialized)
		{
			IMGUI_BEGIN_CCAMERAWND;
			// Shutdown calls d3dDevice->Release D:
			// ImGui_ImplDX9_Shutdown();
			
			ImGui::DestroyPlatformWindows();
			ImGui_ImplDX9_InvalidateDeviceObjects();

			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}

		if (ggui::state.cxywnd.context_initialized)
		{
			IMGUI_BEGIN_CXYWND;
			// Shutdown calls d3dDevice->Release D:
			// ImGui_ImplDX9_Shutdown();

			ImGui::DestroyPlatformWindows();
			ImGui_ImplDX9_InvalidateDeviceObjects();
			
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}

		memset(&ggui::state, 0, sizeof(ggui::imgui_state_t));
	}

	
	bool gui::all_contexts_ready()
	{
		if(ggui::state.ccamerawnd.context_initialized 
			&& ggui::state.cxywnd.context_initialized)
		{
			return true;
		}

		return false;
	}

	// *
	// toggle a imgui menu by command (or key (scheduler))
	void gui::toggle(ggui::imgui_context_menu& menu, [[maybe_unused]] int keycatcher, bool onCommand = false)
	{
		if(!gui::all_contexts_ready())
		{
			return;
		}

		// TODO! fix me
		// toggle menu by key or command
		if (onCommand)
		{
			menu.menustate = !menu.menustate;

			// on close
			if (!menu.menustate)
			{
			}
		}
	}


	// *
	void gui::begin_frame()
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	
	// *
	void gui::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	
	// *
	void gui::register_dvars()
	{
		dvars::gui_menubar_bg_color = dvars::register_vec4(
			/* name		*/ "gui_menubar_bg_color",
			/* x		*/ 0.30f,
			/* y		*/ 0.01f,
			/* z		*/ 0.02f,
			/* w		*/ 0.6f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui menubar background color");
		
		dvars::gui_dockedwindow_bg_color = dvars::register_vec4(
			/* name		*/ "gui_dockedwindow_bg_color",
			/* x		*/ 0.050f,
			/* y		*/ 0.050f,
			/* z		*/ 0.050f,
			/* w		*/ 0.65f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui toolbar background color");

		dvars::gui_window_bg_color = dvars::register_vec4(
			/* name		*/ "gui_window_bg_color",
			/* x		*/ 0.050f,
			/* y		*/ 0.050f,
			/* z		*/ 0.050f,
			/* w		*/ 0.65f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui background color");
	}

	
	// *
	gui::gui()
	{
		// replace hardcoded hotkeys with our own (ini)
		utils::hook(0x420A4F, load_commandmap, HOOK_CALL).install()->quick();
		
		// load/skip the original commandmap (depends if iw3r_hotkeys.ini exists or not)
		utils::hook(0x4210BF, load_default_commandmap, HOOK_CALL).install()->quick();

		
		command::register_command("xydemo"s, [](std::vector<std::string> args)
		{
			gui::toggle(ggui::state.cxywnd.m_demo, 0, true);
		});

		command::register_command("camdemo"s, [](std::vector<std::string> args)
		{
			gui::toggle(ggui::state.ccamerawnd.m_demo, 0, true);
		});

		command::register_command("menubar_show"s, [](std::vector<std::string> args)
		{
			const auto menubar = LoadMenu(cmainframe::activewnd->m_pModuleState->m_hCurrentInstanceHandle, MAKEINTRESOURCE(0xD6)); // 0xD6 = IDR_MENU_QUAKE3
			SetMenu(cmainframe::activewnd->GetWindow(), menubar);
			//dvars::set_bool(dvars::radiant_enable_menubar, true);
			ggui::mainframe_menubar_enabled = true;
		});
		
		
		command::register_command("menubar_hide"s, [](std::vector<std::string> args)
		{
			// destroy the menu or radiant crashes on shutdown when its trying to get the menubar style
			// GetMenuFromHandle
			if (const auto menu = utils::hook::call<CMenu * (__fastcall)(cmainframe*)>(0x42EE20)(cmainframe::activewnd); menu)
			{
				// CMenu::DestroyMenu
				utils::hook::call<void(__fastcall)(CMenu*)>(0x58A908)(menu);
			}

			SetMenu(cmainframe::activewnd->GetWindow(), nullptr);
			//dvars::set_bool(dvars::radiant_enable_menubar, false);
			ggui::mainframe_menubar_enabled = false;
		});
	}

	gui::~gui()
	{ }
}