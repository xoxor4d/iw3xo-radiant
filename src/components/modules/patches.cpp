#include "std_include.hpp"

namespace components
{
	void __declspec(naked) fix_subdivision_loop_bug_stub()
	{
		const static uint32_t retn_addr = 0x44CEC9;
		const static uint32_t retn_addr_break_loop = 0x44CEE7;
		__asm
		{
			add		[esi + 0x14], ebx			 // og
			cmp     dword ptr[esi + 0x14], 1024; // check arbitrary large number only caused by the loop bug
			jl		OG_RETURN;
			mov		[esi + 0x14], 1;
			jmp		retn_addr_break_loop;

		OG_RETURN:
			cmp     dword ptr[esi + 0x14], 1;	// og
			jmp		retn_addr;
		}
	}

	void on_assert()
	{
		effects::stop_all();
		cfxwnd::get()->stop_effect();
	}

	void __declspec(naked) on_assert_stub()
	{
		const static uint32_t retn_addr = 0x49CEAB;
		__asm
		{
			pushad;
			call	on_assert;
			popad;

			push    ecx; // og
			mov     ecx, [ebp + 0x14]; // og
			push    esi; // og
			jmp		retn_addr;
		}
	}


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
		utils::hook(0x44CEC2, fix_subdivision_loop_bug_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x49CEA6, on_assert_stub, HOOK_JUMP).install()->quick();

		// return nullptr on assert: mtlRaw->info.sortKey doesn't index 1 << MTL_SORT_PRIMARY_SORT_KEY_BITS %i not in [0, 64]
		utils::hook(0x51B0E8, assert_mtlraw_sortkey_stub, HOOK_JUMP).install()->quick();

		// do not load "_glow" fonts (qerfont_glow)
		utils::hook::nop(0x552806, 5);

		// nop com_math.cpp "det" line:1775 assert (MatrixInverse44)
		utils::hook::nop(0x4A6BC9, 5);

		// *
		// *

		// should no longer happen, hopefully
		components::exec::on_gui_recurring([]
		{
			if (const auto w = game::g_world_entity(); w && w->firstActive)
			{
				const auto egui = GET_GUI(ggui::entity_dialog);

				if (egui->has_key_value_pair((game::entity_s_def*)w->firstActive, "angles"))
				{
					imgui::Toast(ImGuiToastType_Warning, "Invalid Worldspawn Key-Value Pair", "'angles' is not a valid kvp and will cause lighting issues!\nRemoving kvp ...", 3000);
					game::DeleteKey(w->firstActive->epairs, "angles");
				}
				else if (egui->has_key_value_pair((game::entity_s_def*)w->firstActive, "origin"))
				{
					imgui::Toast(ImGuiToastType_Warning, "Invalid Worldspawn Key-Value Pair", "'origin' is not a valid kvp and will cause lighting issues!\nRemoving kvp ...", 3000);
					game::DeleteKey(w->firstActive->epairs, "origin");
				}
			}
		}, 5000ms);
	}

	patches::~patches()
	{ }
}
