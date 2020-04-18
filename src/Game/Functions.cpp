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
	}

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
}
