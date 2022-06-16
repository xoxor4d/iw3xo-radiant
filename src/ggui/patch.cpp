#include "std_include.hpp"

namespace ggui
{
	void curve_patch_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(245.0f, 205.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_Once);

		if (ImGui::Begin("Curve Patch##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);

			const int curvepatch_step = 2;
			static int curvepatch_width = 3;
			static int curvepatch_height = 3;
			static bool delete_selection = true;

			if (ImGui::InputScalar("Width", ImGuiDataType_U32, &curvepatch_width, &curvepatch_step, nullptr, "%d"))
			{
				// check or make uneven
				curvepatch_width =
					curvepatch_width % 2 != 0 ? curvepatch_width : curvepatch_width - 1;

				// clamp
				curvepatch_width =
					curvepatch_width < 3 ? 3 :
					curvepatch_width > 15 ? 15 : curvepatch_width;
			}

			if (ImGui::InputScalar("Height", ImGuiDataType_U32, &curvepatch_height, &curvepatch_step, nullptr, "%d"))
			{
				// check or make uneven
				curvepatch_height =
					curvepatch_height % 2 != 0 ? curvepatch_height : curvepatch_height - 1;

				// clamp
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

	// #
	// -----------------------------------------------------------------------------

	void terrain_patch_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(245.0f, 170.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_Once);

		if (ImGui::Begin("Terrain Patch##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings))
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

	// #
	// -----------------------------------------------------------------------------

	void thicken_patch_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(245.0f, 170.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_Once);

		if (ImGui::Begin("Thicken Patch##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);

			static bool thicken_with_seams = true;
			static int thickness_in_units = 8;

			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::DragInt("Thickness in units", &thickness_in_units, 0.01f, 1, INT16_MAX))
			{
				thickness_in_units =
					thickness_in_units < 1 ? 1 : thickness_in_units;
			}
			
			ImGui::Checkbox("Create Seams", &thicken_with_seams); TT("Create outer seams if checked");

			SPACING(0.0f, 2.0f);

			const float button_height = ImGui::GetFrameHeightWithSpacing();
			const float button_width = (ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX()) * 0.5f - 8.0f;

			SPACING(0.0f, 2.0f);

			ImGui::BeginGroup();
			{
				if (ImGui::Button("Ok", ImVec2(button_width, button_height)))
				{
					game::Undo_ClearRedo();
					game::Undo_GeneralStart("thicken patch");
					game::Undo_AddBrushList_Selected();

					game::Patch_Thicken(thickness_in_units, thicken_with_seams);

					game::Undo_EndBrushList_Selected();
					game::Undo_End();

					this->close();
				}

				ImGui::EndGroup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(button_width, button_height)))
			{
				this->close();
			}

			ImGui::End();
		}
	}

	void thicken_patch_dialog::on_open()
	{ }

	void thicken_patch_dialog::on_close()
	{ }

	REGISTER_GUI(thicken_patch_dialog);

	// #
	// -----------------------------------------------------------------------------

	void cap_patch_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(245.0f, 320.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_Once);

		if (ImGui::Begin("Cap Patch##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);

			ImVec4 toolbar_button_background_active = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
			ImVec4 toolbar_button_background_hovered = toolbar_button_background_active + ImVec4(0.10f, 0.1f, 0.1f, 0.0f);
			ImVec2 toolbar_button_size = ImVec2(64.0f , 64.0f);

			const auto tb = GET_GUI(ggui::toolbar_dialog);

			ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::SetCursorForCenteredText("Bevel");
			ImGui::TextUnformatted("Bevel");
			ImGui::PopFont();

			SPACING(0.0f, 2.0f);

			static float bevel_group_width = 128.0f;
			const float center_offset = floorf((ImGui::GetWindowWidth() - bevel_group_width) * 0.5f - 8.0f);

			ImGui::SetCursorPosX(center_offset);
			ImGui::BeginGroup();
			{
				static bool hov_bevel;
				if (tb->image_button_label(""
					, "patch_cap_bevel"
					, false
					, hov_bevel
					, "Create inside bevel"
					, &toolbar_button_background_hovered
					, &toolbar_button_background_active
					, &toolbar_button_size))
				{
					components::pmesh::cap_current(0);
					this->close();
				}

				ImGui::SameLine();

				static bool hov_bevel_inverted;
				if (tb->image_button_label("##bevelinverted"
					, "patch_cap_bevel"
					, true
					, hov_bevel_inverted
					, "Create outside bevel"
					, &toolbar_button_background_hovered
					, &toolbar_button_background_active
					, &toolbar_button_size))
				{
					components::pmesh::cap_current(2);
					this->close();
				}

				ImGui::EndGroup();
				bevel_group_width = ImGui::GetItemRectSize().x;
			}

			SPACING(0.0f, 6.0f);

			ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::SetCursorForCenteredText("Endcap");
			ImGui::TextUnformatted("Endcap");
			ImGui::PopFont();

			SPACING(0.0f, 2.0f);

			ImGui::SetCursorPosX(center_offset);
			ImGui::BeginGroup();
			{
				static bool hov_endcap;
				if (tb->image_button_label(""
					, "patch_cap_endcap"
					, false
					, hov_endcap
					, "Create inside endcap"
					, &toolbar_button_background_hovered
					, &toolbar_button_background_active
					, &toolbar_button_size))
				{
					components::pmesh::cap_current(1);
					this->close();
				}

				ImGui::SameLine();

				static bool hov_endcap_inverted;
				if (tb->image_button_label("##endcapinverted"
					, "patch_cap_endcap"
					, true
					, hov_endcap_inverted
					, "Create outside endcap"
					, &toolbar_button_background_hovered
					, &toolbar_button_background_active
					, &toolbar_button_size))
				{
					components::pmesh::cap_current(3);
					this->close();
				}

				ImGui::EndGroup();
				bevel_group_width = ImGui::GetItemRectSize().x;
			}

			SPACING(0.0f, 10.0f);
		
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() * 0.5f) - 50.0f);
			if (ImGui::Button("Cancel", ImVec2(100.0f, ImGui::GetFrameHeightWithSpacing())))
			{
				this->close();
			}

			ImGui::End();
		}
	}

	void cap_patch_dialog::on_open()
	{ }

	void cap_patch_dialog::on_close()
	{ }

	REGISTER_GUI(cap_patch_dialog);
}
