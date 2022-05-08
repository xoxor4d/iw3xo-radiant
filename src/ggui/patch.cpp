#include "std_include.hpp"

namespace ggui
{
	void curve_patch_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(245.0f, 205.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_Appearing);

		if (ImGui::Begin("Curve Patch##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);

			const int curvepatch_step = 2;
			static int curvepatch_width = 3;
			static int curvepatch_height = 3;
			static bool delete_selection = true;

			if (ImGui::InputScalar("Width", ImGuiDataType_U32, &curvepatch_width, &curvepatch_step, nullptr, "%d"))
			{
				curvepatch_width =
					curvepatch_width < 3 ? 3 :
					curvepatch_width > 15 ? 15 : curvepatch_width;
			}

			if (ImGui::InputScalar("Height", ImGuiDataType_U32, &curvepatch_height, &curvepatch_step, nullptr, "%d"))
			{
				curvepatch_height =
					curvepatch_height < 3 ? 3 :
					curvepatch_height > 15 ? 15 : curvepatch_height;
			}

			SPACING(0.0f, 2.0f);

			const float button_height = ImGui::GetFrameHeightWithSpacing();
			const float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX()) * 0.5f - 8.0f;

			const bool enable_creation = game::is_single_brush_selected();

			ImGui::Checkbox("Delete Original Selection", &delete_selection);

			SPACING(0.0f, 2.0f);

			ImGui::BeginGroup();
			{
				ImGui::BeginDisabled(!enable_creation);
				{
					if (ImGui::Button("Ok", ImVec2(button_width, button_height)))
					{
						game::Undo_ClearRedo();
						game::Undo_GeneralStart("make simple patch mesh");
						game::Undo_AddBrushList_Selected();

						const auto p = game::Patch_GenericMesh(curvepatch_width, curvepatch_height, cmainframe::activewnd->m_pActiveXY->m_nViewType, delete_selection, false);

						if(!delete_selection)
						{
							game::Select_Deselect(true);
							game::Brush_Select(p, true, true, false);
						}

						game::Patch_Lightmap_Texturing_dirty(p->def->patch);

						game::Undo_EndBrushList_Selected();
						game::Undo_End();

						this->close();
					}

					ImGui::EndDisabled();
				}

				ImGui::EndGroup();

				if (!enable_creation)
				{
					TT("Select a single brush to enable patch creation");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(button_width, button_height)))
			{
				this->close();
			}
			
			ImGui::End();
		}
	}

	void curve_patch_dialog::on_open()
	{ }

	void curve_patch_dialog::on_close()
	{ }

	REGISTER_GUI(curve_patch_dialog);


	// -----------------------------------------------------------------------------


	void terrain_patch_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(245.0f, 170.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_Appearing);

		if (ImGui::Begin("Terrain Patch##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);

			const int terrainpatch_step = 1;
			static int terrainpatch_width = 2;
			static int terrainpatch_height = 2;

			if (ImGui::InputScalar("Width", ImGuiDataType_U32, &terrainpatch_width, &terrainpatch_step, nullptr, "%d"))
			{
				terrainpatch_width =
					terrainpatch_width < 2 ? 2 :
					terrainpatch_width > 16 ? 16 : terrainpatch_width;
			}

			if (ImGui::InputScalar("Height", ImGuiDataType_U32, &terrainpatch_height, &terrainpatch_step, nullptr, "%d"))
			{
				terrainpatch_height =
					terrainpatch_height < 2 ? 2 :
					terrainpatch_height > 16 ? 16 : terrainpatch_height;
			}

			SPACING(0.0f, 2.0f);

			const float button_height = ImGui::GetFrameHeightWithSpacing();
			const float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX()) * 0.5f - 8.0f;

			const bool enable_creation = game::is_single_brush_selected();

			SPACING(0.0f, 2.0f);

			ImGui::BeginGroup();
			{
				ImGui::BeginDisabled(!enable_creation);
				{
					if (ImGui::Button("Ok", ImVec2(button_width, button_height)))
					{
						game::Undo_ClearRedo();
						game::Undo_GeneralStart("make simple terrain mesh");
						game::Undo_AddBrushList_Selected();

						const auto p = game::Create_Terrain(terrainpatch_width, terrainpatch_height, cmainframe::activewnd->m_pActiveXY->m_nViewType);
						game::Patch_Lightmap_Texturing_dirty(p->def->patch);

						game::Undo_EndBrushList_Selected();
						game::Undo_End();

						this->close();
					}

					ImGui::EndDisabled();
				}

				ImGui::EndGroup();

				if (!enable_creation)
				{
					TT("Select a single brush to enable patch creation");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(button_width, button_height)))
			{
				this->close();
			}

			ImGui::End();
		}
	}

	void terrain_patch_dialog::on_open()
	{ }

	void terrain_patch_dialog::on_close()
	{ }

	REGISTER_GUI(terrain_patch_dialog);
}
