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

			if (ImGui::MenuItem("Ungroup Entity"))
			{
				// CMainFrame::OnSelectionUngroupentity
				cdeclcall(void, 0x426380);
			} TT("eg: dismember a script_brushmodel to individual/normal brushes");

			if (ImGui::MenuItem("Add Selection To Active Layer"))
			{
				// CXYWnd::OnSelectionAddToActiveLayer
				utils::hook::call<void(__fastcall)(void*)>(0x466930)(cmainframe::activewnd->m_pXYWnd);
			}

			ImGui::EndMenu();
		}
	}
}
