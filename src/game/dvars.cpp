#include "std_include.hpp"

namespace dvars
{
	game::dvar_s* gui_menubar_bg_color = nullptr;
	game::dvar_s* gui_dockedwindow_bg_color = nullptr;
	game::dvar_s* gui_window_bg_color = nullptr;
	game::dvar_s* gui_floating_toolbar = nullptr;
	game::dvar_s* gui_resize_dockspace = nullptr;
	game::dvar_s* gui_mainframe_background = nullptr;
	
	//
	game::dvar_s* mainframe_show_console = nullptr;
	game::dvar_s* mainframe_show_zview = nullptr;
	game::dvar_s* mainframe_show_toolbar = nullptr;
	game::dvar_s* mainframe_show_menubar = nullptr;

	// radiant-live
	game::dvar_s* radiant_live = nullptr;
	game::dvar_s* radiant_livePort = nullptr;
	game::dvar_s* radiant_liveDebug = nullptr;

	// stock dvars (not registered but ptr assigned)
	game::dvar_s* fs_homepath = nullptr;
	
	// ---------------------------------------------

	game::dvar_s* register_int(const char* dvar_name, int value, int mins, int maxs, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterInt(dvar_name, value, mins, maxs, flags, description);
		game::printf_to_console(utils::va("|-> %s <int>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_bool(const char* dvar_name, char value, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterBool(dvar_name, value, flags, description);
		game::printf_to_console(utils::va("|-> %s <bool>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_float(const char* dvar_name, float value, float mins, float maxs, __int16 flags, const char* description)
	{
		game::dvar_s*  dvar = game::Dvar_RegisterFloat(dvar_name, value, mins, maxs, flags, description);
		game::printf_to_console(utils::va("|-> %s <float>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_vec4(const char* dvar_name, float x, float y, float z, float w, float mins, float maxs, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterVec4(dvar_name, x, y, z, w, mins, maxs, flags, description);
		game::printf_to_console(utils::va("|-> %s <vec4>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	void set_bool(game::dvar_s* dvar, bool value)
	{
		game::Dvar_SetBool(dvar, value);
	}

	void set_int(game::dvar_s* dvar, int value)
	{
		game::Dvar_SetInt(dvar, value);
	}

	void set_float(game::dvar_s* dvar, float value)
	{
		game::Dvar_SetFloat(dvar, value);
	}

	// --------------------------------------------------

	// register all new dvars here (exec. after config was loaded)
	void register_addon_dvars()
	{
		game::printf_to_console("[Dvars]: register_addon_dvars() start ...\n");

		components::gui::register_dvars();
		components::remote_net::register_dvars();
		cmainframe::register_dvars();
		ggui::toolbar::register_dvars();

		
		game::printf_to_console("\n");
	}
}