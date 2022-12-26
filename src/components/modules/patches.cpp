#include "std_include.hpp"

namespace components
{
	patches* patches::p_this = nullptr;

	void cefn_undo_addbrushlist()
	{
		if (!game::CreateEntityFromName_DisableUndo)
		{
			game::Undo_AddBrushList_Selected();
		}
	}

	void cefn_undo_endbrushlist()
	{
		if (!game::CreateEntityFromName_DisableUndo)
		{
			game::Undo_EndBrushList_Selected();
		}
	}

	// on Undo_AddBrushList
	void __declspec(naked) create_entity_from_name_stub01()
	{
		const static uint32_t retn_addr = 0x465D43;
		__asm
		{
			pushad;
			call	cefn_undo_addbrushlist;
			popad;
			jmp		retn_addr;
		}
	}

	// on Undo_EndBrushList
	void __declspec(naked) create_entity_from_name_stub02()
	{
		const static uint32_t retn_addr = 0x46627A;
		__asm
		{
			pushad;
			call	cefn_undo_endbrushlist;
			popad;
			jmp		retn_addr;
		}
	}

	// #

	void __declspec(naked) fix_subdivision_loop_bug_stub()
	{
		const static uint32_t retn_addr = 0x44CEC9;
		const static uint32_t retn_addr_break_loop = 0x44CEE7;
		__asm
		{
			add		[esi + 0x14], ebx			 // og
			cmp     dword ptr[esi + 0x14], 1024; // check arbitrary large number only caused by the loop bug
			jl		OG_RETURN;
			mov		[esi + 0x14], 1;			 // force patch->subDivType to 1
			jmp		retn_addr_break_loop;		 // break subdivide loop

		OG_RETURN:
			cmp     dword ptr[esi + 0x14], 1;	// og
			jmp		retn_addr;
		}
	}

	// #

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

	// #

	void DECLSPEC_NORETURN on_invalid_hwnd_assert()
	{
		game::Com_Error("Failed to initialize renderer. This might indicate that you have not installed the cod4 modtools or are missing parts of it.\n");
		exit(0);
	}

	void __declspec(naked) on_invalid_hwnd_assert_stub()
	{
		__asm
		{
			pushad;
			call	on_invalid_hwnd_assert;
			popad;
		}
	}

	// #

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
		patches::p_this = this;

		// add ability to disable undo_add/endbrushlist in 'CreateEntityFromName' by using helper bool 'CreateEntityFromName_DisableUndo'
		utils::hook(0x465D3E, create_entity_from_name_stub01, HOOK_JUMP).install()->quick();
		utils::hook(0x466275, create_entity_from_name_stub02, HOOK_JUMP).install()->quick();

		// fix infinite loop that can occur when trying to subdivide 2 curve patches at the same time
		utils::hook(0x44CEC2, fix_subdivision_loop_bug_stub, HOOK_JUMP).install()->quick();

		// stop effects on asserts
		utils::hook(0x49CEA6, on_assert_stub, HOOK_JUMP).install()->quick();

		// custom "invalid hwnd" assert msg with hint to check correct installation of modtools
		utils::hook(0x501B18, on_invalid_hwnd_assert_stub, HOOK_JUMP).install()->quick();

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
		}, 30000ms);
	}

	patches::~patches()
	{ }
}
