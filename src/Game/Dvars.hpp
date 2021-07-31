#pragma once

#define STOCK_DVARCOUNT 229

namespace dvars
{
	// radiant-live
	extern game::dvar_s* radiant_live;
	extern game::dvar_s* radiant_livePort;
	extern game::dvar_s* radiant_liveDebug;

	// stock dvars (not registered but ptr assigned)
	extern game::dvar_s* fs_homepath;

	// -----------------------------------------------------

	game::dvar_s* register_int(const char* dvarName, int value, int mins, int maxs, __int16 flags, const char* description);
	game::dvar_s* register_bool(const char* dvarName, char value, __int16 flags, const char* description);
	game::dvar_s* register_float(const char* dvarName, float value, float mins, float maxs, __int16 flags, const char* description);

	// -----------------------------------------------------

	void register_addon_dvars(); // registers all addon dvars
}