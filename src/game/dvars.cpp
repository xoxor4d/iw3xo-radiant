#include "std_include.hpp"

namespace dvars
{
	game::dvar_s* gui_menubar_bg_color = nullptr;
	game::dvar_s* gui_dockedwindow_bg_color = nullptr;
	game::dvar_s* gui_window_bg_color = nullptr;
	
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
		printf(utils::va("|-> %s <int>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_bool(const char* dvar_name, char value, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterBool(dvar_name, value, flags, description);
		printf(utils::va("|-> %s <bool>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_float(const char* dvar_name, float value, float mins, float maxs, __int16 flags, const char* description)
	{
		game::dvar_s*  dvar = game::Dvar_RegisterFloat(dvar_name, value, mins, maxs, flags, description);
		printf(utils::va("|-> %s <float>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_vec4(const char* dvar_name, float x, float y, float z, float w, float mins, float maxs, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterVec4(dvar_name, x, y, z, w, mins, maxs, flags, description);
		printf(utils::va("|-> %s <vec4>\n", dvar_name));

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
		printf("[dvars]: register_addon_dvars() start ...\n");

		components::gui::register_dvars();

		dvars::radiant_live = dvars::register_bool(
			/* name		*/ "radiant_live",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "enables radiant <-> game link.");

		dvars::radiant_livePort = dvars::register_int(
			/* name		*/	"radiant_livePort",
			/* default	*/	3700,
			/* mins		*/	0,
			/* maxs		*/	99999,
			/* flags	*/	game::dvar_flags::saved,
			/* desc		*/	"port to be used for live-link.");

		dvars::radiant_liveDebug = dvars::register_bool(
			/* name		*/ "radiant_liveDebug",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "enables debug prints.");

		printf("\n");
	}
}