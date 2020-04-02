#pragma once

#define STOCK_DVARCOUNT 229

namespace Dvars
{
	// radiant-live
	extern Game::dvar_s* radiant_live;
	extern Game::dvar_s* radiant_livePort;
	extern Game::dvar_s* radiant_liveDebug;

	// stock dvars (not registered but ptr assigned)
	extern Game::dvar_s* fs_homepath;

	// -----------------------------------------------------

	Game::dvar_s* Register_AddonInt(const char* dvarName, int value, int mins, int maxs, __int16 flags, const char* description);
	Game::dvar_s* Register_AddonBool(const char* dvarName, char value, __int16 flags, const char* description);
	Game::dvar_s* Register_AddonFloat(const char* dvarName, float value, float mins, float maxs, __int16 flags, const char* description);

	// -----------------------------------------------------

	void Register_AddonDvars(); // registers all addon dvars
}