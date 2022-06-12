#include "std_include.hpp"

namespace components
{
	// Patch_CapCurrent :: includes undo logic
	void pmesh::cap_current(int type)
	{
		game::Undo_ClearRedo();
		game::Undo_GeneralStart("cap patch");
		game::Undo_AddBrushList_Selected();

		game::patchMesh_t* pParent = nullptr;
		game::selbrush_def_t* b[4] = {};
		game::selbrush_def_t* pCap = nullptr;

		int nIndex = 0;

		if (!game::QE_SingleBrush())
		{
			game::printf_to_console("^2 Cannot cap multiple selection. Please select a single patch.\n");
			return;
		}

		if (game::g_selected_brushes_next())
		{
			if (game::g_selected_brushes_next()->patch->def != game::g_selected_brushes_next()->def->patch)
			{
				game::printf_to_console("^1 pb->patch->def != pb->def->patch");
				return;
			}

			FOR_ALL_SELECTED_BRUSHES(pb)
			{
				pParent = pb->patch->def;

				// decide which if any ends we are going to cap
				// if any of these compares hit, it is a closed patch and as such
				// the generic capping will work.. if we do not find a closed edge 
				// then we need to ask which kind of cap to add

				if (utils::vector::compare(pParent->ctrl[0][0].xyz, pParent->ctrl[pParent->width - 1][0].xyz))
				{
					pCap = game::Patch_Cap(pParent, true, false);
					if (pCap != nullptr)
					{
						b[nIndex++] = pCap;
					}
				}

				if (utils::vector::compare(pParent->ctrl[0][pParent->height - 1].xyz, pParent->ctrl[pParent->width - 1][pParent->height - 1].xyz))
				{
					pCap = game::Patch_Cap(pParent, true, true);
					if (pCap != nullptr)
					{
						b[nIndex++] = pCap;
					}
				}

				if (utils::vector::compare(pParent->ctrl[0][0].xyz, pParent->ctrl[0][pParent->height - 1].xyz))
				{
					pCap = game::Patch_Cap(pParent, false, false);
					if (pCap != nullptr)
					{
						b[nIndex++] = pCap;
					}
				}

				if (utils::vector::compare(pParent->ctrl[pParent->width - 1][0].xyz, pParent->ctrl[pParent->width - 1][pParent->height - 1].xyz))
				{
					pCap = game::Patch_Cap(pParent, false, true);
					if (pCap != nullptr)
					{
						b[nIndex++] = pCap;
					}
				}

			}

			if (pParent)
			{
				// if we did not cap anything with the above tests
				if (nIndex == 0)
				{
					b[nIndex++] = game::Patch_CapSpecial(pParent, type, false);
					b[nIndex++] = game::Patch_CapSpecial(pParent, type, true);
				}

				if (nIndex > 0)
				{
					while (nIndex > 0)
					{
						nIndex--;
						if (b[nIndex])
						{
							game::Brush_Select(b[nIndex], true, true, false);
						}
					}

					if (game::g_selected_brushes_next()->owner == game::g_world_entity())
					{
						game::eclass_t* pecNew = game::Eclass_ForName("func_group", false);
						if (pecNew)
						{
							const auto e = Entity_Create(pecNew);
							game::SetKeyValue(e->firstActive, "type", "patchCapped");
							game::Undo_SetIdForEntity(e->firstActive);
						}
					}
				}
			}
		}

		game::Undo_EndBrushList_Selected();
		game::Undo_End();
	}

	void thicken_terrain_patch(game::patchMesh_t* p, int thickness)
	{
		std::vector<game::brush_t_with_custom_def*> brushes;
		game::patchMesh_t* seam = nullptr;

		game::printf_to_console("Thicken Terrain Patch :o");

		game::Patch_MeshNormals(p);

		// ---------------------------------
		// create backside

		auto* backside = game::Patch_Duplicate(p);
		{
			for (auto i = 0; i < p->width; i++)
			{
				for (auto j = 0; j < p->height; j++)
				{
					utils::vector::ma(p->ctrl[i][j].xyz, static_cast<float>(thickness), p->ctrl[i][j].normal, backside->ctrl[i][j].xyz);
				}
			}

			game::Patch_Rebuild(backside, 1);
			backside->type |= game::PATCH_THICK;

			auto* symbiot = game::Brush_AddToList(backside->pSymbiot, game::g_selected_brushes_next()->owner);
			game::Brush_AddToList2(symbiot);
			brushes.emplace_back(symbiot);

			//game::Patch_Invert(pNew); // invert patch after building seams
			//game::Patch_UpdateSelected(pNew, 0);
		}

		// ---------------------------------
		// left seam

		seam = game::Patch_Duplicate(p);
		{
			for (auto w = p->width - 2; w > 0; w--)
			{
				// remove columns from right to left till there are only 2 left
				game::Patch_RemoveColumn(seam, false);
			}

			for (auto i = 0; i < p->height; i++)
			{
				// move points from the right column -> left backside column
				utils::vector::copy(backside->ctrl[0][i].xyz, seam->ctrl[1][i].xyz);
			}

			game::Patch_Rebuild(seam, 1);
			seam->type |= game::PATCH_SEAM;

			auto* symbiot = game::Brush_AddToList(seam->pSymbiot, game::g_selected_brushes_next()->owner);
			game::Brush_AddToList2(symbiot);
			brushes.emplace_back(symbiot);

			game::Patch_Invert(seam);
			game::Patch_Lightmap_Texturing_dirty(seam);
			game::Patch_UpdateSelected(seam, 0);
		}

		// ---------------------------------
		// right seam

		seam = game::Patch_Duplicate(p);
		{
			for (auto w = p->width - 2; w > 0; w--)
			{
				// remove columns from left to right till there are only 2 left
				game::Patch_RemoveColumn(seam, true);
			}

			for (auto i = 0; i < p->height; i++)
			{
				// move points from the left column -> right backside column
				utils::vector::copy(backside->ctrl[backside->width - 1][i].xyz, seam->ctrl[0][i].xyz);
			}

			game::Patch_Rebuild(seam, 1);
			seam->type |= game::PATCH_SEAM;

			auto* symbiot = game::Brush_AddToList(seam->pSymbiot, game::g_selected_brushes_next()->owner);
			game::Brush_AddToList2(symbiot);
			brushes.emplace_back(symbiot);

			game::Patch_Invert(seam);
			game::Patch_Lightmap_Texturing_dirty(seam);
			game::Patch_UpdateSelected(seam, 0);
		}

		// ---------------------------------
		// front seam

		seam = game::Patch_Duplicate(p);
		{
			for (auto h = p->height - 2; h > 0; h--)
			{
				// remove rows from bottom to top till there are only 2 left
				game::Patch_RemoveRow(seam, true);
			}

			for (auto i = 0; i < p->width; i++)
			{
				// move points from the lower row -> upper backside row
				utils::vector::copy(backside->ctrl[i][p->height - 1].xyz, seam->ctrl[i][0].xyz);
			}

			game::Patch_Rebuild(seam, 1);
			seam->type |= game::PATCH_SEAM;

			auto* symbiot = game::Brush_AddToList(seam->pSymbiot, game::g_selected_brushes_next()->owner);
			game::Brush_AddToList2(symbiot);
			brushes.emplace_back(symbiot);

			game::Patch_Invert(seam);
			game::Patch_Lightmap_Texturing_dirty(seam);
			game::Patch_UpdateSelected(seam, 0);
		}

		// ---------------------------------
		// back seam

		seam = game::Patch_Duplicate(p);
		{
			for (auto h = p->height - 2; h > 0; h--)
			{
				// remove rows from top to bottom till there are only 2 left
				game::Patch_RemoveRow(seam, false);
			}

			for (auto i = 0; i < p->width; i++)
			{
				// move points from the upper row -> lower backside row
				utils::vector::copy(backside->ctrl[i][0].xyz, seam->ctrl[i][1].xyz);
			}

			game::Patch_Rebuild(seam, 1);
			seam->type |= game::PATCH_SEAM;

			auto* symbiot = game::Brush_AddToList(seam->pSymbiot, game::g_selected_brushes_next()->owner);
			game::Brush_AddToList2(symbiot);
			brushes.emplace_back(symbiot);

			game::Patch_Invert(seam);
			game::Patch_Lightmap_Texturing_dirty(seam);
			game::Patch_UpdateSelected(seam, 0);
		}

		// ---------------------------------
		// invert backside (last)

		game::Patch_Invert(backside); // invert patch after building seams
		game::Patch_Lightmap_Texturing_dirty(seam);
		game::Patch_UpdateSelected(backside, 0);

		for (const auto b : brushes)
		{
			game::Brush_Select((game::selbrush_def_t*)b, true, true, false);
		}
	}

	void __declspec(naked) thicken_terrain_patch_stub()
	{
		const static uint32_t retn_addr = 0x448757;
		__asm
		{
			pushad;
			mov     eax, [ebp + 8];
			push	eax; // thickness (inverted)
			push	ebx; // patch
			call	thicken_terrain_patch;
			add		esp, 8;
			popad;

			jmp		retn_addr;
		}
	}

	void on_thicken_dialog()
	{
		GET_GUI(ggui::thicken_patch_dialog)->toggle();
	}

	void on_cap_dialog()
	{
		GET_GUI(ggui::cap_patch_dialog)->toggle();
	}

	pmesh::pmesh()
	{
		// implement terrain patch thickenning
		utils::hook(0x448752, thicken_terrain_patch_stub, HOOK_JUMP).install()->quick();

		// replace stock thicken dialog with our own
		utils::hook::detour(0x42B0D0, on_thicken_dialog, HK_JUMP);

		// replace stock cap dialog with our own
		utils::hook::detour(0x42AD40, on_cap_dialog, HK_JUMP);
	}

	pmesh::~pmesh()
	{ }
}
