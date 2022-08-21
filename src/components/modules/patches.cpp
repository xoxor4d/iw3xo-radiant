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
