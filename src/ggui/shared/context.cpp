#include "std_include.hpp"


namespace ggui::context
{
	void xyzcam_general_selection()
	{
		if (dvars::gui_grid_context_show_select->current.enabled)
		{
			if (ImGui::BeginMenu("Select"))
			{
				if (ImGui::MenuItem("Select Complete Tall")) {
					cdeclcall(void, 0x426340); // CMainFrame::OnSelectionSelectcompletetall
				}

				if (ImGui::MenuItem("Select Partial Tall")) {
					cdeclcall(void, 0x426360); // CMainFrame::OnSelectionSelectpartialtall
				}

				if (ImGui::MenuItem("Select Touching")) {
					cdeclcall(void, 0x426370); // CMainFrame::OnSelectionSelecttouching
				}

				if (ImGui::MenuItem("Select Inside")) {
					cdeclcall(void, 0x426350); // CMainFrame::OnSelectionSelectinside
				}

				ImGui::EndMenu(); // Select
			}

			SEPERATORV(0.0f);
		}

		if (ImGui::MenuItem("Make Structural"))
		{
			// CMainFrame::OnSelectionMakeStructural
			cdeclcall(void, 0x426200);
		}

		if (ImGui::MenuItem("Make Detail"))
		{
			// CMainFrame::OnSelectionMakeDetail
			cdeclcall(void, 0x4261C0);
		}

		if (ImGui::MenuItem("Make Non-Colliding"))
		{
			// CMainFrame::OnSelectionMakeNonColliding
			cdeclcall(void, 0x426280);
		}

		if (ImGui::BeginMenu("Advanced"))
		{
			if (ImGui::MenuItem("Make Weapon Clip"))
			{
				// CMainFrame::OnSelectionMakeWeaponclip
				cdeclcall(void, 0x426240);
			}

			if (ImGui::MenuItem("Make Split Coplanar Geo"))
			{
				// CMainFrame::OnSelectionMakeSplitCoplanar
				cdeclcall(void, 0x4262C0);
			}

			if (ImGui::MenuItem("Make Don't Split Coplanar Geo"))
			{
				// CMainFrame::OnSelectionMakeDontSplitCoplanar
				cdeclcall(void, 0x426300);
			}

			SEPERATORV(0.0f);

			if (ImGui::MenuItem("Add Selection To Active Layer"))
			{
				// CXYWnd::OnSelectionAddToActiveLayer
				utils::hook::call<void(__fastcall)(void*)>(0x466930)(cmainframe::activewnd->m_pXYWnd);
			}

			ImGui::EndMenu();
		}
	}

	void grouping_menu(game::selbrush_def_t* sb)
	{
		if (sb && sb->def && sb->def->owner)
		{
			if (imgui::BeginMenu("Group"))
			{
				const auto ents = GET_GUI(ggui::entity_dialog);

				bool selection_is_group = false;
				if (const auto val = ents->get_value_for_key_from_epairs(sb->def->owner->epairs, "classname");
					val == "func_group"s || val == "func_cullgroup"s || val == "script_brushmodel"s)
				{
					selection_is_group = true;
				}

				imgui::BeginDisabled(!selection_is_group);
				{
					if (imgui::MenuItem("Select Brush Group"))
					{
						game::Brush_Select(sb, true, false, false);
					}

					imgui::EndDisabled();
				} TT("Select every member of the group (brushmodel, func_group)");

				imgui::BeginDisabled(!game::is_any_brush_selected());
				{
					if (imgui::MenuItem("Group Selected Brushes"))
					{
						game::CreateEntityFromClassname(cmainframe::activewnd->m_pXYWnd, "func_group", 0, 0);
					} TT("Creates a func_group");

					if (imgui::MenuItem("Ungroup"))
					{
						// CMainFrame::OnSelectionUngroupentity
						cdeclcall(void, 0x426380);
					} TT("dismember a group (brushmodel, func_group)");

					imgui::EndDisabled();
				}

				imgui::EndMenu();
			}
		}
	}
}
