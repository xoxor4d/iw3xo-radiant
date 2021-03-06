#pragma once
#include "Utils/function.hpp"

namespace Game
{
	namespace Globals
	{
		// Init
		extern std::string loadedModules;

		extern bool radiant_floatingWindows;
		extern bool radiant_initiated;
		extern bool radiant_config_loaded;
		extern bool radiant_config_not_found;

		extern CWnd* m_pCamWnd_ref;

		// Misc
		extern Game::TrackWorldspawn trackWorldspawn;

		// Live Link
		extern Game::ProcessServerCommands cServerCmd;
		extern bool live_connected;
		
	}

	extern int	*g_nUpdateBitsPtr;
	extern int	&g_nUpdateBits;
	extern bool &g_bScreenUpdates;
	extern double &g_time;
	extern double &g_oldtime;

	extern int *dvarCount;
	extern Game::dvar_s *dvarPool;
	extern Game::dvar_s* dvarPool_FirstEmpty;
	extern DWORD* sortedDvars;
	extern DWORD* sortedDvarsAddons;
	extern int sortedDvarsAddonsCount;

	static DWORD* frontEndDataOut_ptr = (DWORD*)(0x73D480);  // frontEndDataOut pointer
	static DWORD* currSelectedBrushes = (DWORD*)(0x23F1864); // (selected_brushes array pointer)

	static DWORD* worldEntity_ptr = (DWORD*)(0x25D5B30); // holds pointer to worldEntity

	// -----------------------------------------------------------

	typedef void(*Com_Error_t)(const char *error, ...);
		extern Com_Error_t Com_Error;

	typedef HBRUSH(__thiscall* OnCtlColor_t)(void* thisptr, class CDC* pDC, class CWnd* pWnd, UINT nCtlColor);
		extern OnCtlColor_t OnCtlColor;

	static utils::function<bool(bool bQuiet)> QE_SingleBrush = 0x48C8B0; // no warnings when bQuiet
	static utils::function<void(Game::DebugGlobals *debugGlobalsEntry, const float *mins, const float *maxs, const float *color)> R_AddDebugBox = 0x528710;

	//bool IsBrushSelected(Game::brush_t* bSel);

	// no error but doesnt reload everything
	static utils::function< void()>	DX_ResetDevice = 0x5015F0;

	// -----------------------------------------------------------
	// DVARS

	static utils::function< void (Game::dvar_s* dvar, bool value)>							Dvar_SetBool = 0x4B37F0;
	static utils::function< void (Game::dvar_s* dvar, std::int32_t value)>					Dvar_SetInt = 0x4B3810;
	static utils::function< void (Game::dvar_s* dvar, float value)>							Dvar_SetFloat = 0x4B3830;
	static utils::function< void (Game::dvar_s* dvar, float r, float g, float b, float a)>	Dvar_SetColor = 0x4B38E0;

	static utils::function< void (Game::dvar_s* dvar, float x, float y)>						Dvar_SetVec2 = 0x4B3850;
	static utils::function< void (Game::dvar_s* dvar, float x, float y, float z)>				Dvar_SetVec3 = 0x4B3870;
	static utils::function< void (Game::dvar_s* dvar, float x, float y, float z, float w)>		Dvar_SetVec4 = 0x4B38A0;

	static utils::function< Game::dvar_s* (const char* dvarName, int value, int mins, int maxs, __int16 flags, const char* description)> Dvar_RegisterInt = 0x4B2660;
	static utils::function< Game::dvar_s* (const char* dvarName, char value, __int16 flags, const char* description)> Dvar_RegisterBool = 0x4B25F0;
	static utils::function< Game::dvar_s* (const char* dvarName, float value, float mins, float maxs, __int16 flags, const char* description)> Dvar_RegisterFloat = 0x4B26D0;
	static utils::function< Game::dvar_s* (const char *dvarName, const char *value, __int16 flags, const char *description)> Dvar_RegisterString = 0x4B28E0;

	// ASM
	const char* Dvar_DisplayableValue(Game::dvar_s* dvar);
	void Dvar_SetString(Game::dvar_s *dvar /*esi*/, const char *string /*ebx*/);
	Game::dvar_s* Dvar_FindVar(const char* dvar);
	Game::dvar_s* Dvar_SetFromStringFromSource(const char *string /*ecx*/, Game::dvar_s *dvar /*esi*/, int source);

	void ConsoleError(const std::string& msg);
}
