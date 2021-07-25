#include "stdafx.h"

namespace Detours
{
	uint32_t GlobalOptions;

	void SetGlobalOptions(uint32_t Options)
	{
		InterlockedExchange(&GlobalOptions, Options & OPT_MASK);
	}

	uint32_t GetGlobalOptions()
	{
		return GlobalOptions;
	}

	uint64_t DetourAlignAddress(uint64_t Address, uint8_t Align)
	{
		if (Address % Align != 0)
			Address += Align - Address % 8;

		return Address;
	}

	bool DetourCopyMemory(uintptr_t Target, uintptr_t Memory, size_t Length)
	{
		auto pvTarget = reinterpret_cast<void *>(Target);
		auto pvMemory = reinterpret_cast<void *>(Memory);

		DWORD dwOld = 0;
		if (!VirtualProtect(pvTarget, Length, PAGE_EXECUTE_READWRITE, &dwOld))
			return false;

		memcpy(pvTarget, pvMemory, Length);

		// Ignore if this fails, the memory was copied either way
		VirtualProtect(pvTarget, Length, dwOld, &dwOld);
		return true;
	}

	bool DetourFlushCache(uintptr_t Target, size_t Length)
	{
		return FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void *>(Target), Length) != FALSE;
	}

	uintptr_t IATThunkHook(uintptr_t Module, PIMAGE_THUNK_DATA NameTable, PIMAGE_THUNK_DATA ImportTable, const char *API, uintptr_t Detour)
	{
		for (; NameTable->u1.Ordinal != 0; ++NameTable, ++ImportTable)
		{
			if (!IMAGE_SNAP_BY_ORDINAL(NameTable->u1.Ordinal))
			{
				auto importName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(Module + NameTable->u1.ForwarderString);
				auto funcName = &importName->Name[0];

				// If this is the function name we want, hook it
				if (!strcmp(funcName, API))
				{
					// Copy the pointer variable itself, not the function bytes
					uintptr_t originalFunc = ImportTable->u1.AddressOfData;
					uintptr_t newPointer = Detour;

					if (!DetourCopyMemory(reinterpret_cast<uintptr_t>(&ImportTable->u1.AddressOfData), reinterpret_cast<uintptr_t>(&newPointer), sizeof(ImportTable->u1.AddressOfData)))
						return 0;

					// Done
					return originalFunc;
				}
			}
		}

		return 0;
	}

	uintptr_t IATHook(uintptr_t Module, const char *ImportModule, const char *API, uintptr_t Detour)
	{
		auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(Module);
		auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(Module + dosHeader->e_lfanew);

		// Validate PE Header and (64-bit|32-bit) module type
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
			return 0;

		// Get the load configuration section which holds the imports
		auto dataDirectory = ntHeaders->OptionalHeader.DataDirectory;
		auto sectionRVA = dataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		auto sectionSize = dataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

		if (sectionRVA == 0 || sectionSize == 0)
			return 0;

		// https://jpassing.com/2008/01/06/using-import-address-table-hooking-for-testing/
		// https://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/interception/interception_win.cc
		//
		// Iterate over each import descriptor
		auto importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(Module + sectionRVA);

		for (size_t i = 0; importDescriptor[i].Name != 0; i++)
		{
			auto dllName = reinterpret_cast<PSTR>(Module + importDescriptor[i].Name);

			// Is this the specific module the user wants?
			if (!_stricmp(dllName, ImportModule))
			{
				if (!importDescriptor[i].FirstThunk)
					return 0;

				auto nameTable = reinterpret_cast<PIMAGE_THUNK_DATA>(Module + importDescriptor[i].OriginalFirstThunk);
				auto importTable = reinterpret_cast<PIMAGE_THUNK_DATA>(Module + importDescriptor[i].FirstThunk);

				auto originalFunc = IATThunkHook(Module, nameTable, importTable, API, Detour);

				if (!originalFunc)
					continue;

				return originalFunc;
			}
		}

		// API or module name wasn't found
		return 0;
	}

	uintptr_t IATDelayedHook(uintptr_t Module, const char *ImportModule, const char *API, uintptr_t Detour)
	{
		auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(Module);
		auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(Module + dosHeader->e_lfanew);

		// Validate PE Header and (64-bit|32-bit) module type
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
			return 0;

		// Get the load configuration section which holds the imports
		auto dataDirectory = ntHeaders->OptionalHeader.DataDirectory;
		auto sectionRVA = dataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress;
		auto sectionSize = dataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size;

		if (sectionRVA == 0 || sectionSize == 0)
			return 0;

		// Iterate over each delayed import descriptor
		auto importDescriptor = reinterpret_cast<PIMAGE_DELAYLOAD_DESCRIPTOR>(Module + sectionRVA);

		for (size_t i = 0; importDescriptor[i].DllNameRVA != 0; i++)
		{
			auto dllName = reinterpret_cast<PSTR>(Module + importDescriptor[i].DllNameRVA);

			// Is this the specific module the user wants?
			if (!_stricmp(dllName, ImportModule))
			{
				auto nameTable = reinterpret_cast<PIMAGE_THUNK_DATA>(Module + importDescriptor[i].ImportNameTableRVA);
				auto importTable = reinterpret_cast<PIMAGE_THUNK_DATA>(Module + importDescriptor[i].ImportAddressTableRVA);

				auto originalFunc = IATThunkHook(Module, nameTable, importTable, API, Detour);

				if (!originalFunc)
					continue;

				return originalFunc;
			}
		}

		// API or module name wasn't found
		return 0;
	}
}