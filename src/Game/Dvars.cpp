#include "STDInclude.hpp"

namespace Dvars
{
	// radiant-live
	Game::dvar_s* radiant_live = nullptr;
	Game::dvar_s* radiant_livePort = nullptr;
	Game::dvar_s* radiant_liveDebug = nullptr;

	// stock dvars (not registered but ptr assigned)
	Game::dvar_s* fs_homepath = nullptr;

	// ---------------------------------------------

	Game::dvar_s* Register_AddonInt(const char* dvarName, int value, int mins, int maxs, __int16 flags, const char* description)
	{
		int oldDvarCount = *Game::dvarCount;

		// new dvars after stock dvars
		*Game::dvarCount = STOCK_DVARCOUNT + Game::sortedDvarsAddonsCount;
		Game::dvar_s* dvar = Game::Dvar_RegisterInt(dvarName, value, mins, maxs, flags, description);

		// increase our addon dvar count and reset the dvar count
		Game::sortedDvarsAddonsCount++;
		*Game::dvarCount = oldDvarCount;

		printf(Utils::VA("|-> %s <int>\n", dvarName));

		// return a pointer to our dvar
		return dvar;
	}

	Game::dvar_s* Register_AddonBool(const char* dvarName, char value, __int16 flags, const char* description)
	{
		int oldDvarCount = *Game::dvarCount;

		// put new dvars after dvar 228
		*Game::dvarCount = STOCK_DVARCOUNT + Game::sortedDvarsAddonsCount;
		Game::dvar_s* dvar = Game::Dvar_RegisterBool(dvarName, value, flags, description);

		// increase our addon dvar count and reset the dvar count
		Game::sortedDvarsAddonsCount++;
		*Game::dvarCount = oldDvarCount;

		printf(Utils::VA("|-> %s <bool>\n", dvarName));

		// return a pointer to our dvar
		return dvar;
	}

	Game::dvar_s* Register_AddonFloat(const char* dvarName, float value, float mins, float maxs, __int16 flags, const char* description)
	{
		int oldDvarCount = *Game::dvarCount;

		// put new dvars after dvar 228
		*Game::dvarCount = STOCK_DVARCOUNT + Game::sortedDvarsAddonsCount;
		Game::dvar_s*  dvar = Game::Dvar_RegisterFloat(dvarName, value, mins, maxs, flags, description);

		// increase our addon dvar count and reset the dvar count
		Game::sortedDvarsAddonsCount++;
		*Game::dvarCount = oldDvarCount;

		printf(Utils::VA("|-> %s <float>\n", dvarName));

		// return a pointer to our dvar
		return dvar;
	}

	// --------------------------------------------------

	// register all new dvars here (exec. after config was loaded)
	void Register_AddonDvars()
	{
		printf("[Dvars]: Register_AddonDvars() start ...\n");

		Dvars::radiant_live = Dvars::Register_AddonBool(
			/*name*/	"radiant_live",
			/*default*/	1,
			/*flag*/	Game::dvar_flags::saved,
			/*desc*/	"enables radiant <-> game link.");

		Dvars::radiant_livePort = Dvars::Register_AddonInt(
			/*name*/	"radiant_livePort",
			/*default*/	3700,
			/*mins*/	0,
			/*maxs*/	99999,
			/*flags*/	Game::dvar_flags::saved,
			/*desc*/	"port to be used for live-link.");

		Dvars::radiant_liveDebug = Dvars::Register_AddonBool(
			/*name*/	"radiant_liveDebug",
			/*default*/	1,
			/*flag*/	Game::dvar_flags::saved,
			/*desc*/	"enables debug prints.");
	}
}