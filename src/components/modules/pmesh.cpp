#include "std_include.hpp"

namespace components
{
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
			game::Brush_Select((game::brush_t*)b, true, true, false);
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

	pmesh::pmesh()
	{
		// implement terrain patch thickenning
		utils::hook(0x448752, thicken_terrain_patch_stub, HOOK_JUMP).install()->quick();
	}

	pmesh::~pmesh()
	{ }
}
