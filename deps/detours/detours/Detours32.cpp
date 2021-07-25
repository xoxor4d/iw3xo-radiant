#include "stdafx.h"

extern "C" int DetoursInitZydis32();
extern "C" int DetoursInitZydis64();
extern "C" int DetoursGetNextInstructionLength(void *Instruction);

#ifdef _M_IX86
namespace Detours
{
	namespace X86
	{
		#define HEADER_MAGIC		'@D32'

		#define MAX_INSTRUCT_SIZE	0x08

		#define JUMP_LENGTH_32		0x05	// jmp <addr>
		#define CALL_LENGTH_32		0x05	// call <addr>
		#define JUMP_EAX_LENGTH_32	0x07	// mov eax, <addr>; jmp eax
		#define JUMP_PTR_LENGTH_32	0x06	// jmp dword ptr <addr>
		#define PUSH_RET_LENGTH_32	0x06	// push <addr>; retn

		#define ALIGN_32(x)			DetourAlignAddress((uint64_t)(x), 0x4);
		#define BREAK_ON_ERROR()	{ if (GetGlobalOptions() & OPT_BREAK_ON_FAIL) __debugbreak(); }

		uintptr_t DetourFunction(uintptr_t Target, uintptr_t Detour, X86Option Options)
		{
			if (!Target || !Detour)
			{
				BREAK_ON_ERROR();
				return 0;
			}

			// Init decoder exactly once
			static bool decoderInit = []()
			{
				return DetoursInitZydis64() == -1 ? false : true;
			}();

			if (!decoderInit)
			{
				BREAK_ON_ERROR();
				return 0;
			}

			// Decode the actual assembly
			uint32_t neededSize = DetourGetHookLength(Options);
			uint32_t totalInstrSize = 0;

			for (int len = 0; len != -1; len = DetoursGetNextInstructionLength(reinterpret_cast<void *>(Target + totalInstrSize)))
			{
				totalInstrSize += len;

				if (totalInstrSize >= neededSize)
					break;
			}

			// Unable to find a needed length
			if (neededSize == 0 || totalInstrSize < neededSize)
			{
				BREAK_ON_ERROR();
				return 0;
			}

			// Allocate the trampoline data
			uint32_t allocSize = 0;
			allocSize += sizeof(JumpTrampolineHeader);	// Base structure
			allocSize += totalInstrSize;				// Size of the copied instructions
			allocSize += MAX_INSTRUCT_SIZE;				// Maximum instruction size
			allocSize += MAX_INSTRUCT_SIZE;				// Maximum instruction size
			allocSize += 0x64;							// Padding for any memory alignment

			uint8_t *jumpTrampolinePtr = (uint8_t *)VirtualAlloc(nullptr, allocSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			if (!jumpTrampolinePtr)
			{
				BREAK_ON_ERROR();
				return 0;
			}

			// Fill out the header
			auto header					= reinterpret_cast<JumpTrampolineHeader *>(jumpTrampolinePtr);

			header->Magic				= HEADER_MAGIC;
			header->Random				= GetCurrentProcessId() + GetCurrentThreadId() + totalInstrSize;

			header->CodeOffset			= Target;
			header->DetourOffset		= Detour;

			header->InstructionLength	= totalInstrSize;
			header->InstructionOffset	= ALIGN_32(jumpTrampolinePtr + sizeof(JumpTrampolineHeader));

			header->TrampolineLength	= JUMP_LENGTH_32;
			header->TrampolineOffset	= ALIGN_32(header->InstructionOffset + header->InstructionLength + JUMP_LENGTH_32 + header->TrampolineLength);

			// Copy the old instructions over
			DetourCopyMemory(header->InstructionOffset, header->CodeOffset, header->InstructionLength);

			// Write the assembly in the allocation block
			DetourWriteStub(header);

			// Write padding nops if the hook splices the middle of an instruction
			if ((GetGlobalOptions() & OPT_DO_NOT_PAD_NOPS) == 0)
			{
				std::vector<uint8_t> nops;

				for (uint32_t i = 0; i < header->InstructionLength; i++)
					nops.push_back(0x90);

				DetourCopyMemory(header->CodeOffset, reinterpret_cast<uintptr_t>(nops.data()), nops.size());
			}

			bool result = false;

			switch (Options)
			{
			case X86Option::USE_JUMP:		result = DetourWriteJump(header);		break;
			case X86Option::USE_CALL:		result = DetourWriteCall(header);		break;
			case X86Option::USE_PUSH_RET:	result = DetourWritePushRet(header);	break;
			}

			// If an operation failed free the memory and exit
			if (!result)
			{
				VirtualFree(jumpTrampolinePtr, 0, MEM_RELEASE);

				BREAK_ON_ERROR();
				return 0;
			}

			// Force flush any possible CPU cache
			DetourFlushCache(Target, totalInstrSize);
			DetourFlushCache(reinterpret_cast<uintptr_t>(jumpTrampolinePtr), allocSize);

			// Set read/execution on the page
			DWORD dwOld = 0;
			VirtualProtect(jumpTrampolinePtr, allocSize, PAGE_EXECUTE_READ, &dwOld);

			return header->InstructionOffset;
		}

		bool DetourRemove(uint8_t *Trampoline)
		{
			if (!Trampoline)
			{
				BREAK_ON_ERROR();
				return false;
			}

			auto header = reinterpret_cast<JumpTrampolineHeader *>(Trampoline - sizeof(JumpTrampolineHeader));

			if (header->Magic != HEADER_MAGIC)
			{
				BREAK_ON_ERROR();
				return false;
			}

			// Rewrite the backed-up code
			if (!DetourCopyMemory(header->CodeOffset, header->InstructionOffset, header->InstructionLength))
			{
				BREAK_ON_ERROR();
				return false;
			}

			DetourFlushCache(header->CodeOffset, header->InstructionLength);
			VirtualFree(header, 0, MEM_RELEASE);

			return true;
		}

		uintptr_t DetourVTable(uintptr_t Target, uintptr_t Detour, uint32_t TableIndex)
		{
			// Each function is stored in an array
			auto virtualPointer = reinterpret_cast<void *>(Target + (TableIndex * sizeof(void *)));

			DWORD dwOld = 0;
			if (!VirtualProtect(virtualPointer, sizeof(void *), PAGE_EXECUTE_READWRITE, &dwOld))
				return 0;

			auto original = InterlockedExchangePointer(reinterpret_cast<void **>(virtualPointer), reinterpret_cast<void *>(Detour));

			VirtualProtect(virtualPointer, sizeof(void *), dwOld, &dwOld);
			return reinterpret_cast<uintptr_t>(original);
		}

		bool VTableRemove(uintptr_t Target, uintptr_t Function, uint32_t TableIndex)
		{
			// Reverse VTable detour
			return DetourVTable(Target, Function, TableIndex) != 0;
		}

		void DetourWriteStub(JumpTrampolineHeader *Header)
		{
			/********** Allocated code block modifications **********/
			uint8_t buffer[5];
			
			auto unhookStart = Header->CodeOffset + Header->InstructionLength;		// Determine where the 'unhooked' part of the function starts
			auto binstrPtr = Header->InstructionOffset + Header->InstructionLength;// Jump to hooked function (Backed up instructions)

			buffer[0] = 0xE9;
			*reinterpret_cast<int32_t *>(&buffer[1]) = static_cast<int32_t>(unhookStart - (binstrPtr + 5));

			DetourCopyMemory(binstrPtr, reinterpret_cast<uintptr_t>(&buffer), sizeof(buffer));

			// Jump to user function (Write the trampoline)
			buffer[0] = 0xE9;
			*reinterpret_cast<int32_t *>(&buffer[1]) = static_cast<int32_t>(Header->DetourOffset - (Header->TrampolineOffset + 5));

			DetourCopyMemory(Header->TrampolineOffset, reinterpret_cast<uintptr_t>(&buffer), sizeof(buffer));
		}

		bool DetourWriteJump(JumpTrampolineHeader *Header)
		{
			// Relative JUMP
			uint8_t buffer[5];

			buffer[0] = 0xE9;
			*reinterpret_cast<int32_t *>(&buffer[1]) = static_cast<int32_t>(Header->TrampolineOffset - (Header->CodeOffset + 5));

			return DetourCopyMemory(Header->CodeOffset, reinterpret_cast<uintptr_t>(&buffer), sizeof(buffer));
		}

		bool DetourWriteCall(JumpTrampolineHeader *Header)
		{
			// Relative CALL
			uint8_t buffer[5];

			buffer[0] = 0xE8;
			*reinterpret_cast<int32_t *>(&buffer[1]) = static_cast<int32_t>(Header->TrampolineOffset - (Header->CodeOffset + 5));

			return DetourCopyMemory(Header->CodeOffset, reinterpret_cast<uintptr_t>(&buffer), sizeof(buffer));
		}

		bool DetourWritePushRet(JumpTrampolineHeader *Header)
		{
			// RET-Jump to trampoline
			uint8_t buffer[6];

			// push 0xXXXXX
			buffer[0] = 0x68;
			*reinterpret_cast<uint32_t *>(&buffer[1]) = Header->TrampolineOffset;

			// retn
			buffer[5] = 0xC3;

			return DetourCopyMemory(Header->CodeOffset, reinterpret_cast<uintptr_t>(&buffer), sizeof(buffer));
		}

		uint32_t DetourGetHookLength(X86Option Options)
		{
			uint32_t size = 0;

			switch (Options)
			{
			case X86Option::USE_JUMP:		size += JUMP_LENGTH_32;		break;
			case X86Option::USE_CALL:		size += CALL_LENGTH_32;		break;
			case X86Option::USE_PUSH_RET:	size += PUSH_RET_LENGTH_32;	break;
			}

			return size;
		}
	}
}
#endif // _M_IX86