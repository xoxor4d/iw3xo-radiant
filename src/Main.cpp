#include "STDInclude.hpp"

namespace Main
{
	static Utils::Hook EntryPointHook;

	void Initialize()
	{
		Main::EntryPointHook.uninstall();
		Components::Loader::Initialize();
	}

	void Uninitialize()
	{
		Components::Loader::Uninitialize();
	}
}

__declspec(naked) void EntryPoint()
{
    __asm
    {
        // This has to be called, otherwise the hook is not uninstalled and we're deadlocking
        call Main::Initialize

		// same address as EntryPointHook.initialize
        mov eax, 5C4299h
        jmp eax
    }
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DWORD oldProtect;
		VirtualProtect(GetModuleHandle(nullptr), 0x234D000, PAGE_EXECUTE_READWRITE, &oldProtect); // find this with CFF Explorer - Optinal Headers - SizeOfImage

		Beep(523, 100);

		// Adress is the entry adress found in IDA under Exports
		Main::EntryPointHook.initialize(0x5C4299, EntryPoint)->install();
	}

	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		Main::Uninitialize();
	}

	return TRUE;
}
