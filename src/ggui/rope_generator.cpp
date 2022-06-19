#include "std_include.hpp"

namespace ggui
{
	void rope_generator_dialog::generate_rope(int thickness, int slack, int subdiv, bool delete_null)
	{
		const auto egui = GET_GUI(ggui::entity_dialog);

		int null_count = 0;
		game::vec3_t pts[2];

		FOR_ALL_SELECTED_BRUSHES(sb)
		{
			if (sb->def)
			{
				const char* class_str = egui->get_value_for_key_from_epairs(sb->def->owner->epairs, "classname");
				if (class_str != "info_null"s)
				{
					null_count = 0;
					break;
				}

				for (int j = 0; j < 3; j++)
				{
					pts[null_count][j] = sb->def->mins[j] + abs((sb->def->maxs[j] - sb->def->mins[j]) * 0.5f);
				}

				null_count++;
			}
		}

		if (null_count != 2)
		{
			game::printf_to_console("Must have 2 info_null's selected!");
			return;
		}

		// ------------------------------

		if (delete_null)
		{
			// CMainFrame::OnSelectionDelete()
			cdeclcall(void, 0x425690);
		}

		// ------------------------------
		// create undo for deselection and deselect everything

		game::Undo_ClearRedo();
		game::Undo_GeneralStart("delesect null");

		FOR_ALL_SELECTED_BRUSHES(sb)
		{
			if (game::g_lastundo())
			{
				game::Undo_AddEntity_W(sb->owner->firstActive);
			}
			else
			{
				game::printf_to_console("Undo_AddEntity: no last undo.");
			}
		}

		// deselect everything
		game::Select_Deselect(1);
		game::Undo_End();


		// ------------------------------
		// create an undo for the creation of the rope 

		game::Undo_ClearRedo();
		game::Undo_GeneralStart("create brush");
		game::Undo_AddBrushList(game::currSelectedBrushes);


		// ------------------------------
		// generate a new brush

		game::vec3_t mins, maxs;
		const float half_width = static_cast<float>(thickness) * 0.5f;
		utils::vector::set_vec3(mins, -half_width);
		utils::vector::set_vec3(maxs, half_width);


		// Brush_Alloc
		const auto new_b = utils::hook::call<game::brush_t_with_custom_def* (__cdecl)(void*, void*)>(0x4751E0)(game::g_qeglobals->random_texture_stuff, 0);
		game::Brush_Create(maxs, mins, new_b, 0);

		if (!new_b)
		{
			return;
		}

		game::Brush_BuildWindings(new_b, 1);
		++new_b->version;

		game::Entity_LinkBrush(new_b, game::g_world_entity()->firstActive);
		const auto b_linked = game::Brush_AddToList(new_b, game::g_world_entity());
		if (b_linked->onext || b_linked->oprev)
		{
			__debugbreak();
		}

		game::Brush_AddToList2(b_linked);

		/*game::Undo_EndBrushList(game::currSelectedBrushes);
		game::Undo_End();*/


		// ------------------------------
		// cylinder

		// create patch cylinder (creates an undo)
		// cdeclcall(void, 0x42A3B0); // CMainFrame::OnCurvePatchtube

		// Patch_BrushToMesh
		utils::hook::call<void(__cdecl)(bool _bCone, bool _bBevel, bool _bEndcap, bool _bSquare)>(0x43ACC0)(false, false, false, false);


		// rotate cylinder Y (always spawned with opening facing Z)
		game::selection_rotate_axis(1, 90);

		// get cylinder 
		const auto cyl = game::g_selected_brushes();

		// get angles between the two null's and rotate accordingly
		game::vec3_t angles = {};
		{
			game::vec3_t diff = {};
			utils::vector::subtract(pts[1], pts[0], diff);
			utils::vector::vectoangles(diff, angles);
		}

		if (angles[0] != 0.0f)
		{
			game::selection_rotate_axis(1, static_cast<int>(-angles[0]));
		}

		if (angles[1] != 0.0f)
		{
			game::selection_rotate_axis(2, static_cast<int>(-angles[1]));
		}

		if (angles[2] != 0.0f)
		{
			game::selection_rotate_axis(0, static_cast<int>(angles[2]));
		}

		// change selection mode
		game::g_qeglobals->d_select_mode = game::select_t::sel_curvepoint;


		// move individual rows
		{
			game::vec3_t center_r0, center_mid, center_r2;

			{
				// select all 9 vertices of the first row
				Patch_SelectRow(0, cyl->def->patch, 0);

				// calc center of all selected points
				ggui::camera_guizmo::get_selection_center_movepoints(center_r0);

				// get dist between second null and center of first row
				game::vec3_t diff_r0 = {};
				utils::vector::subtract(pts[1], center_r0, diff_r0);

				// move points accordingly
				for (auto i = 0; i < game::g_qeglobals->d_num_move_points; i++)
				{
					utils::vector::add(game::g_qeglobals->d_move_points[i]->xyz, diff_r0, game::g_qeglobals->d_move_points[i]->xyz);
				}
			}

			{
				// select all 9 vertices of the third row
				Patch_SelectRow(2, cyl->def->patch, 0);

				// calc center of all selected points
				ggui::camera_guizmo::get_selection_center_movepoints(center_r2);

				// get dist between first null and center of last row
				game::vec3_t diff_r2 = {};
				utils::vector::subtract(pts[0], center_r2, diff_r2);

				// move points accordingly
				for (auto i = 0; i < game::g_qeglobals->d_num_move_points; i++)
				{
					utils::vector::add(game::g_qeglobals->d_move_points[i]->xyz, diff_r2, game::g_qeglobals->d_move_points[i]->xyz);
				}
			}

			{
				// calc center between the 2 nulls
				game::vec3_t center_of_two_nulls;

				utils::vector::add(pts[0], pts[1], center_of_two_nulls);
				utils::vector::scale(center_of_two_nulls, 0.5f, center_of_two_nulls);

				// select all 9 vertices of the middle row
				Patch_SelectRow(1, cyl->def->patch, 0);

				// calc center of all selected points
				ggui::camera_guizmo::get_selection_center_movepoints(center_mid);

				// get dist between center of null's and center of middle row
				game::vec3_t diff_mid = {};
				utils::vector::subtract(center_of_two_nulls, center_mid, diff_mid);

				// add slack (10% of dist between nulls)
				diff_mid[2] -= fabs(utils::vector::distance(pts[1], pts[0])) * static_cast<float>(slack) * 0.01f;

				// move points accordingly
				for (auto i = 0; i < game::g_qeglobals->d_num_move_points; i++)
				{
					utils::vector::add(game::g_qeglobals->d_move_points[i]->xyz, diff_mid, game::g_qeglobals->d_move_points[i]->xyz);
				}
			}
		}

		// set material to caulk no texture is selected
		if (cyl->def->patch->texture.radMtl && cyl->def->patch->texture.radMtl->name == "$default"s)
		{
			SetMaterial("caulk", &cyl->def->patch->texture);
			++cyl->def->patch->version;
		}

		// Patch_Lightmap_Texturing
		// utils::hook::call<void(__cdecl)()>(0x448110)(); // creates an undo
		game::Patch_Lightmap_Texturing_dirty(cyl->def->patch);

		game::Patch_UpdateSelected(cyl->def->patch, 1);

		// change selection mode
		game::g_qeglobals->d_select_mode = game::select_t::sel_brush;


		// ------------------------------
		// default curve patch subdivision is 2 
		if(subdiv != 2)
		{
			// bring patch down to lowest subdivision
			cdeclcall(void, 0x428EE0); // CMainFrame::OnOverBrightShiftDown

			for(auto i = 0; i < subdiv - 1 && i < 3; i++)
			{
				cdeclcall(void, 0x428EB0); // CMainFrame::OnOverBrightShiftUp
			}
		}


		game::Undo_EndBrushList(game::currSelectedBrushes);
		game::Undo_End();
	}

	bool rope_generator_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 270.0f);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		ImGui::SetNextWindowSize(ImVec2(400.0f, 270.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Rope Generator##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);

			const char* description_str = "Create and select two info_null entities and hit generate";
			ImGui::SetCursorForCenteredText(description_str); ImGui::TextUnformatted(description_str);

			SPACING(0.0f, 4.0f);
			ImGui::Separator();
			SPACING(0.0f, 4.0f);

			ImGui::DragInt("Rope thickness", &this->m_thickness, 0.1f, 2, 256);
			ImGui::DragInt("Rope slack in %", &this->m_slack, 0.1f, 0, 100);
			ImGui::DragInt("Rope subdivision", &this->m_subdiv, 0.1f, 1, 4);

			SPACING(0.0f, 12.0f);

			static float generate_group_width = 120.0f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionMax().x - generate_group_width) * 0.5f);
			ImGui::BeginGroup();
			{
				// enable / disable generate button

				const auto egui = GET_GUI(ggui::entity_dialog);
				int null_count = 0;

				FOR_ALL_SELECTED_BRUSHES(sb)
				{
					if (sb->def)
					{
						const char* class_str = egui->get_value_for_key_from_epairs(sb->def->owner->epairs, "classname");
						if (class_str != "info_null"s)
						{
							null_count = 0;
							break;
						}

						null_count++;
					}
				}

				ImGui::BeginDisabled(null_count != 2);
				{
					if (ImGui::Button("Generate"))
					{
						generate_rope(this->m_thickness, this->m_slack, this->m_subdiv, this->m_delete_null);
					}

					ImGui::EndDisabled();
				}
				

				ImGui::SameLine();
				ImGui::Checkbox("Delete null-entities", &this->m_delete_null);

				ImGui::EndGroup();
			}
			generate_group_width = ImGui::GetItemRectSize().x;
			

			SPACING(0.0f, 2.0f);
			
			ImGui::End();
		}
		else
		{
			return false;
		}

		return true;
	}

	void rope_generator_dialog::on_open()
	{ }

	void rope_generator_dialog::on_close()
	{ }

	REGISTER_GUI(rope_generator_dialog);
}
