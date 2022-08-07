#include "std_include.hpp"

namespace components
{
	void assert_mtlraw_sortkey(int sort_key, int sort_max)
	{
		game::printf_to_console("mtlRaw->info.sortKey doesn't index 1 << MTL_SORT_PRIMARY_SORT_KEY_BITS - %i not in [0, %i]", sort_key, sort_max);
	}

	void __declspec(naked) assert_mtlraw_sortkey_stub()
	{
		const static uint32_t retn_addr = 0x51B206; // return nullptr with stack offset 0x124
		__asm
		{
			pushad;
			push    64;
			push    edx; // sort_key
			call	assert_mtlraw_sortkey;
			add		esp, 8;
			popad;

			add		esp, 8; // og pushes - back to stack offset of 0x124
			jmp		retn_addr;
		}
	}

	patches::patches()
	{
		// return nullptr on assert: mtlRaw->info.sortKey doesn't index 1 << MTL_SORT_PRIMARY_SORT_KEY_BITS %i not in [0, 64]
		utils::hook(0x51B0E8, assert_mtlraw_sortkey_stub, HOOK_JUMP).install()->quick();
	}

	patches::~patches()
	{ }
}
