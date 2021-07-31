#include "std_include.hpp"

namespace dvars
{
	// radiant-live
	game::dvar_s* radiant_live = nullptr;
	game::dvar_s* radiant_livePort = nullptr;
	game::dvar_s* radiant_liveDebug = nullptr;

	// stock dvars (not registered but ptr assigned)
	game::dvar_s* fs_homepath = nullptr;

	// ---------------------------------------------

	game::dvar_s* register_int(const char* dvarName, int value, int mins, int maxs, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterInt(dvarName, value, mins, maxs, flags, description);

		printf(utils::va("|-> %s <int>\n", dvarName));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_bool(const char* dvarName, char value, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterBool(dvarName, value, flags, description);
		
		printf(utils::va("|-> %s <bool>\n", dvarName));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_float(const char* dvarName, float value, float mins, float maxs, __int16 flags, const char* description)
	{
		game::dvar_s*  dvar = game::Dvar_RegisterFloat(dvarName, value, mins, maxs, flags, description);

		printf(utils::va("|-> %s <float>\n", dvarName));

		// return a pointer to our dvar
		return dvar;
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