#include "std_include.hpp"

namespace ggui
{
	void toolbox_dialog::register_child(const std::string& _child_name, const std::function<void()>& _callback)
	{
		_toolbox_childs[_child_name] = toolbox_child_s
		(
			m_child_count,
			_callback
		);

		m_child_count++;
	}

	void setup_child()
	{
		const auto window = ImGui::GetCurrentWindow();

		const float child_indent = 12.0f;
		const auto child_size = ImGui::GetContentRegionAvail();
		const float window_height = window->ContentSize.y > window->SizeFull.y ? window->ContentSize.y : window->SizeFull.y;

		const auto min = ImGui::GetCursorScreenPos();
		const auto max = ImVec2(min.x + child_size.x, min.y + window_height);
		ImGui::GetWindowDrawList()->AddRect(min, max, ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)), 0.0f, ImDrawFlags_RoundCornersBottom);

		SPACING(0.0f, 6.0f);
		ImGui::Indent(child_indent);
	}

	bool toolbox_dialog::treenode_begin(const char* name, bool default_open, int& style_colors, int& style_vars)
	{
		// set/reset
		style_colors = 0; style_vars = 0;

		const auto min_coords = ImGui::GetCursorScreenPos() - ImVec2(0.0f, 3.0f);
		const auto max_coords = ImVec2(min_coords.x + ImGui::GetWindowContentRegionWidth() - 24.0f, min_coords.y + 25.0f); // indent_offset

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f); style_vars++;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 5.0f)); style_vars++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 8.0f)); style_vars++;

		const auto treenode_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, treenode_color); style_colors++;
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, treenode_color); style_colors++;

		const bool hovered = ImGui::IsMouseHoveringRect(min_coords, max_coords, true);
		ImGui::GetWindowDrawList()->AddRectFilled(min_coords, max_coords, 
			hovered ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.28f, 0.28f, 0.28f, 1.0f))
					: ImGui::ColorConvertFloat4ToU32(ImVec4(0.25f, 0.25f, 0.25f, 1.0f)));

		//const ImRect total_bb(min_coords, max_coords);
		//ImGui::ItemHoverable(total_bb, ImGui::GetID(name));

		if (!ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_SpanFullWidth | (default_open ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None)))
		{
			ImGui::PopStyleColor(style_colors);
			ImGui::PopStyleVar(style_vars);
			return false;
		}

		ImGui::PopStyleVar(2); style_vars = style_vars - 2; // ImGuiStyleVar_ItemSpacing
		SPACING(0.0f, 0.0f);

		return true;
	}

	// only call if begin returned true
	void toolbox_dialog::treenode_end(int style_colors, int style_vars, float end_spacing)
	{
		ImGui::TreePop();
		ImGui::PopStyleColor(style_colors);
		ImGui::PopStyleVar(style_vars);

		SPACING(0.0f, end_spacing);
	}

	void toolbox_dialog::center_horz_begin(const float group_width, float indent)
	{
		if (group_width != 0.0f)
		{
			// floorf to ensure no half pixel offsets (image 1px borders)
			ImGui::SetCursorPosX(floorf((ImGui::GetWindowWidth() - ImGui::GetCursorPos().x) * 0.5f - (group_width * 0.5f) + indent));
		}

		ImGui::BeginGroup();
	}

	void toolbox_dialog::center_horz_end(float& group_width)
	{
		ImGui::EndGroup();

		// only update group width if change is larger then 1px because floorf in 'center_horz_begin' causes a wiggle
		const float new_width = ImGui::GetItemRectSize().x;
		group_width = abs(group_width - new_width) != 1.0f ? new_width : group_width;
	}

	// ---

	void toolbox_dialog::child_brush()
	{
		int style_colors = 0;
		int style_vars = 0;

		ImGui::PushID("child_brush");
		setup_child();
		{
			const float max_widget_width = 251.0f;

			const ImVec4 toolbar_button_background_active = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
			const ImVec4 toolbar_button_background_hovered = ImVec4(0.225f, 0.225f, 0.225f, 1.0f);
			const ImVec2 toolbar_button_size = ImVec2(32.0f, 32.0f);

			const auto tb = GET_GUI(ggui::toolbar_dialog);

			/*ImGui::title_with_seperator_helpmark("Flip Selection", false, 0, 1.0f, 2.0f,
				"Flips selection along axis");*/

			/*if (toolbox_treenode_begin("Name", true, style_colors, style_vars))
			{
				static float _l1_width = 100.0f;
				center_horz_begin(_l1_width);
				{
					
				}
				center_horz_end(_l1_width);

				toolbox_treenode_end(style_colors, style_vars);
			}*/

			
			if (treenode_begin("Manipulation", true, style_colors, style_vars))
			{
				static float manipulation_l1_width = 100.0f;
				center_horz_begin(manipulation_l1_width);
				{
					static bool hov_rot_x;
					if (tb->image_button_label("##rotate_x"
						, "rotate_x"
						, false
						, hov_rot_x
						, "Rotate around x-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x425100); // CMainFrame::OnBrushRotatex
					}

					ImGui::SameLine();
					static bool hov_flip_x;
					if (tb->image_button_label("##flip_x"
						, "flip_x"
						, false
						, hov_flip_x
						, "Flip along x-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x4250A0); // CMainFrame::OnBrushFlipx
					}

					ImGui::SameLine();
					static bool hov_rot_y;
					if (tb->image_button_label("##rotate_y"
						, "rotate_y"
						, false
						, hov_rot_y
						, "Rotate around y-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x425190); // CMainFrame::OnBrushRotatey
					}

					ImGui::SameLine();
					static bool hov_flip_y;
					if (tb->image_button_label("##flip_y"
						, "flip_y"
						, false
						, hov_flip_y
						, "Flip along y-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x4250C0); // CMainFrame::OnBrushFlipy
					}

					ImGui::SameLine();
					static bool hov_rot_z;
					if (tb->image_button_label("##rotate_z"
						, "rotate_z"
						, false
						, hov_rot_z
						, "Rotate around z-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x425220); // CMainFrame::OnBrushRotatez
					}

					ImGui::SameLine();
					static bool hov_flip_z;
					if (tb->image_button_label("##flip_z"
						, "flip_z"
						, false
						, hov_flip_z
						, "Flip along z-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x4250E0); // CMainFrame::OnBrushFlipz
					}

					center_horz_end(manipulation_l1_width);
				}

				imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				imgui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));

				static float manipulation_l2_width = 100.0f;
				center_horz_begin(manipulation_l2_width);
				{
					if (ImGui::Button("Hollow Brush", ImVec2(max_widget_width * 0.5f - 6.0f, 32.0f)))
					{
						cdeclcall(void, 0x425570); // CMainFrame::OnSelectionMakehollow
					} TT("Hollows the selected brush. Uses current grid size as wall size");

					ImGui::SameLine(0.0f, 4.0f);
					if (ImGui::Button("Auto Caulk", ImVec2(max_widget_width * 0.5f - 6.0f, 32.0f)))
					{
						cdeclcall(void, 0x425600); // CMainFrame::OnSelectionAutoCaulk
					} TT(std::string("Automatically caulk invisible faces\n" + ggui::hotkey_dialog::get_hotkey_for_command("AutoCaulk", true)).c_str());

					center_horz_end(manipulation_l2_width);
				}


				static float manipulation_l3_width = 100.0f;
				center_horz_begin(manipulation_l3_width);
				{
					if (ImGui::Button("Toggle New Patch Dragging", ImVec2(max_widget_width - 8.0f, 32.0f)))
					{
						dvars::set_bool(dvars::grid_new_patch_drag, !dvars::grid_new_patch_drag->current.enabled);
					}

					const std::string dvar_desc = dvars::grid_new_patch_drag->description + " "s + ggui::hotkey_dialog::get_hotkey_for_command("new_patch_drag", true);
					TT(dvar_desc.c_str());

					if (dvars::grid_new_patch_drag->current.enabled)
					{
						imgui::SameLine();
						imgui::SetCursorPos(ImVec2(imgui::GetCursorPos().x - 34.0f, imgui::GetCursorPos().y + 7.0f));
						imgui::RenderCheckMark(imgui::GetWindowDrawList(), imgui::GetCursorScreenPos(), imgui::GetColorU32(ImGuiCol_Text), 16.0f);
					}

					center_horz_end(manipulation_l3_width);
				}

				imgui::PopStyleColor();
				imgui::PopStyleVar();

				treenode_end(style_colors, style_vars);
			}

			if (treenode_begin("Brush Contents", true, style_colors, style_vars))
			{
				imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				imgui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));

				static float brush_contents_l1_width = 100.0f;
				center_horz_begin(brush_contents_l1_width);
				{
					if (ImGui::Button("Detail", ImVec2(max_widget_width * 0.5f - 2.0f, 32.0f)))
					{
						cdeclcall(void, 0x4261C0); // CMainFrame::OnSelectionMakeDetail
					}

					ImGui::SameLine(0.0f, 4.0f);
					if (ImGui::Button("Non-Colliding", ImVec2(max_widget_width * 0.5f - 2.0f, 32.0f)))
					{
						cdeclcall(void, 0x426280); // CMainFrame::OnSelectionMakeNonColliding
					}
				}
				center_horz_end(brush_contents_l1_width);

				static float brush_contents_l2_width = 100.0f;
				center_horz_begin(brush_contents_l2_width);
				{
					if (ImGui::Button("Structural", ImVec2(max_widget_width * 0.5f - 2.0f, 32.0f)))
					{
						cdeclcall(void, 0x426200); // CMainFrame::OnSelectionMakeStructural
					}

					ImGui::SameLine(0.0f, 4.0f);
					if (ImGui::Button("Weapon Clip", ImVec2(max_widget_width * 0.5f - 2.0f, 32.0f)))
					{
						cdeclcall(void, 0x426240); // CMainFrame::OnSelectionMakeWeaponclip
					}
				}
				center_horz_end(brush_contents_l2_width);

				imgui::PopStyleColor();
				imgui::PopStyleVar();

				treenode_end(style_colors, style_vars);
			}

			if (treenode_begin("Brush Primitives", true, style_colors, style_vars))
			{
				const int step_size = 1;
				static int sides_cone = 8;
				static int sides_cylinder = 8;

				ImGui::BeginDisabled(!game::is_single_brush_selected());
				{
					static float cone_l1_width = 100.0f;
					center_horz_begin(cone_l1_width);
					{
						ImGui::SetNextItemWidth(max_widget_width * 0.5f - 2.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(4.0f, 4.0f));
						if (ImGui::InputScalar("##sides_cone", ImGuiDataType_U32, &sides_cone, &step_size, nullptr, "%d"))
						{
							sides_cone = sides_cone < 3 ? 3 : sides_cone;
						}
						ImGui::PopStyleVar();

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("Make Cone", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("make cone");
							game::Undo_AddBrushList_Selected();

							game::Brush_MakeSidedCone(sides_cone);

							game::Undo_EndBrushList_Selected();
							game::Undo_End();
						}
					}
					center_horz_end(cone_l1_width);

					static float cone_l2_width = 100.0f;
					center_horz_begin(cone_l2_width);
					{
						ImGui::SetNextItemWidth(max_widget_width * 0.5f - 2.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(4.0f, 4.0f));
						if (ImGui::InputScalar("##sides_cylinder", ImGuiDataType_U32, &sides_cylinder, &step_size, nullptr, "%d"))
						{
							sides_cylinder = sides_cylinder < 3 ? 3 : sides_cylinder;
						}
						ImGui::PopStyleVar();

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("Make Cylinder", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("make cylinder");
							game::Undo_AddBrushList_Selected();

							game::Brush_MakeSided_Axis(sides_cone, true);

							game::Undo_EndBrushList_Selected();
							game::Undo_End();
						}

						center_horz_end(cone_l2_width);
					}

					ImGui::EndDisabled();
				}
				
				treenode_end(style_colors, style_vars);
			}
		}
		ImGui::PopID();
	}

	void toolbox_dialog::child_patch()
	{
		int style_colors = 0;
		int style_vars = 0;

		ImGui::PushID("child_patch");
		setup_child();
		{
			const ImVec4 toolbar_button_background_active = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
			const ImVec4 toolbar_button_background_hovered = ImVec4(0.225f, 0.225f, 0.225f, 1.0f);
			const ImVec2 toolbar_button_size = ImVec2(32.0f, 32.0f);

			const auto tb = GET_GUI(ggui::toolbar_dialog);

			static float max_widget_width = 240.0f; // assumed first value - depends on total width of curve patch creation widget
			static float widget_start_offset_screen = 0.0f;

			const auto selbrush = game::g_selected_brushes();
			const auto is_patch = selbrush && selbrush->def && selbrush->def->patch;
			const auto is_curve_patch = selbrush && selbrush->def && selbrush->def->patch && (selbrush->def->patch->type == game::PATCH_TYPE::PATCH_GENERIC || selbrush->def->patch->type == game::PATCH_TYPE::PATCH_SEAM);
			const auto atleast_two_verts_selected = game::g_qeglobals->d_num_move_points >= 2;

			if (treenode_begin("Manipulation", true, style_colors, style_vars))
			{
				static float manipulation_l1_width = 100.0f;
				center_horz_begin(manipulation_l1_width);
				{
					static bool hov_rot_x;
					if (tb->image_button_label("##rotate_x"
						, "rotate_x"
						, false
						, hov_rot_x
						, "Rotate around x-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x425100); // CMainFrame::OnBrushRotatex
					}

					ImGui::SameLine();
					static bool hov_flip_x;
					if (tb->image_button_label("##flip_x"
						, "flip_x"
						, false
						, hov_flip_x
						, "Flip along x-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x4250A0); // CMainFrame::OnBrushFlipx
					}

					ImGui::SameLine();
					static bool hov_rot_y;
					if (tb->image_button_label("##rotate_y"
						, "rotate_y"
						, false
						, hov_rot_y
						, "Rotate around y-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x425190); // CMainFrame::OnBrushRotatey
					}

					ImGui::SameLine();
					static bool hov_flip_y;
					if (tb->image_button_label("##flip_y"
						, "flip_y"
						, false
						, hov_flip_y
						, "Flip along y-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x4250C0); // CMainFrame::OnBrushFlipy
					}

					ImGui::SameLine();
					static bool hov_rot_z;
					if (tb->image_button_label("##rotate_z"
						, "rotate_z"
						, false
						, hov_rot_z
						, "Rotate around z-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x425220); // CMainFrame::OnBrushRotatez
					}

					ImGui::SameLine();
					static bool hov_flip_z;
					if (tb->image_button_label("##flip_z"
						, "flip_z"
						, false
						, hov_flip_z
						, "Flip along z-axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x4250E0); // CMainFrame::OnBrushFlipz
					}

					center_horz_end(manipulation_l1_width);
				}

				static float manipulation_l2_width = 100.0f;
				center_horz_begin(manipulation_l2_width);
				{
					const auto prefs = game::g_PrefsDlg();

					static bool hov_cycle_edges;
					if (tb->image_button_label("##cycle_patch_edge_direction"
						, "cycle_patch_edge_direction"
						, game::g_qeglobals->d_select_mode == 9
						, hov_cycle_edges
						, "Toggle terrain-quad edge cycle mode"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						mainframe_thiscall(void, 0x42B530); // CMainFrame::OnCycleTerrainEdge
					}

					ImGui::SameLine();
					static bool hov_tolerant_weld;
					if (tb->image_button_label("##tolerant_weld"
						, "tolerant_weld"
						, prefs->m_bTolerantWeld
						, hov_tolerant_weld
						, "Toggle tolerant weld / Draw tolerant weld lines"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						mainframe_thiscall(void, 0x42A130); // CMainFrame::OnTolerantWeld
					}

					ImGui::SameLine();
					static bool hov_redisp_patch_pts;
					if (tb->image_button_label("##redisperse_patch_points"
						, "redisperse_patch_points"
						, false
						, hov_redisp_patch_pts
						, std::string("Redisperse Patch Points " + ggui::hotkey_dialog::get_hotkey_for_command("RedisperseVertices", true)).c_str()
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x42A270); // CMainFrame::OnRedistPatchPoints
					}

					imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
					imgui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));

					ImGui::SameLine();
					if (ImGui::Button("Adv. Edit Dialog", ImVec2(116.0f, 32.0f)))
					{
						cdeclcall(void, 0x42BC90); // CMainFrame::OnAdvancedEditDlg
					} TT(std::string("Toggle the advanced vertex edit dialog\n" + ggui::hotkey_dialog::get_hotkey_for_command("AdvancedCurveEdit", true)).c_str());

					imgui::PopStyleColor();
					imgui::PopStyleVar();

					center_horz_end(manipulation_l2_width);



					imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
					imgui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));

					static float manipulation_l3_width = 100.0f;
					center_horz_begin(manipulation_l3_width);
					{
						if (ImGui::Button("Toggle New Patch Dragging", ImVec2(max_widget_width - 8.0f, 32.0f)))
						{
							dvars::set_bool(dvars::grid_new_patch_drag, !dvars::grid_new_patch_drag->current.enabled);
						}

						const std::string dvar_desc = dvars::grid_new_patch_drag->description + " "s + ggui::hotkey_dialog::get_hotkey_for_command("new_patch_drag", true);
						TT(dvar_desc.c_str());


						if (dvars::grid_new_patch_drag->current.enabled)
						{
							imgui::SameLine();
							imgui::SetCursorPos(ImVec2(imgui::GetCursorPos().x - 34.0f, imgui::GetCursorPos().y + 7.0f));
							imgui::RenderCheckMark(imgui::GetWindowDrawList(), imgui::GetCursorScreenPos(), imgui::GetColorU32(ImGuiCol_Text), 16.0f);
						}

						center_horz_end(manipulation_l3_width);
					}

					imgui::PopStyleColor();
					imgui::PopStyleVar();

				}

				treenode_end(style_colors, style_vars);
			} // manipulation node

			if (treenode_begin("Terrain", true, style_colors, style_vars))
			{
				const int step_size = 1;
				static int terrainpatch_width = 2;
				static int terrainpatch_height = 2;
				static float total_widget_width = 100.0f;

				ImGui::BeginDisabled(!game::is_single_brush_selected());
				{
					static float width_height_settings_width = 100.0f;

					const float terrain_button_width = 76.0f;
					const float terrain_button_spacing = 14.0f;
					const float cursor_y = ImGui::GetCursorPosY();

					// floorf to ensure no half pixel offsets (image 1px borders)
					ImGui::SetCursorPosX(floorf((ImGui::GetWindowWidth() - ImGui::GetCursorPos().x) * 0.5f - (width_height_settings_width * 0.5f) + 4.0f - (terrain_button_width * 0.5f) - (terrain_button_spacing * 0.5f)));

					ImGui::BeginGroup();
					{
						ImGui::BeginGroup();
						{
							ImGui::SetNextItemWidth(110.0f);
							if (ImGui::InputScalar("Width##terrain", ImGuiDataType_U32, &terrainpatch_width, &step_size, nullptr, "%d"))
							{
								terrainpatch_width =
									terrainpatch_width < 2 ? 2 :
									terrainpatch_width > 16 ? 16 : terrainpatch_width;
							}

							ImGui::SetNextItemWidth(110.0f);
							if (ImGui::InputScalar("Height##terrain", ImGuiDataType_U32, &terrainpatch_height, &step_size, nullptr, "%d"))
							{
								terrainpatch_height =
									terrainpatch_height < 2 ? 2 :
									terrainpatch_height > 16 ? 16 : terrainpatch_height;
							}

							ImGui::EndGroup(); // terrain width / height

							// only update group width if change is larger then 1px because floorf in 'center_horz_begin' causes a wiggle
							const float new_width = ImGui::GetItemRectSize().x;
							width_height_settings_width = abs(width_height_settings_width - new_width) != 1.0f ? new_width : width_height_settings_width;
						} // group 2

						ImGui::SameLine(0, terrain_button_spacing);
						ImGui::SetCursorPosY(cursor_y);

						if (ImGui::Button("Create##terrain", ImVec2(terrain_button_width, 60.0f)))
						{
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("make simple terrain mesh");
							game::Undo_AddBrushList_Selected();

							const auto p = game::Create_Terrain(terrainpatch_width, terrainpatch_height, cmainframe::activewnd->m_pActiveXY->m_nViewType);
							game::Patch_Lightmap_Texturing_dirty(p->def->patch);

							game::Undo_EndBrushList_Selected();
							game::Undo_End();
						}

						ImGui::EndGroup(); // total widget
						total_widget_width = ImGui::GetItemRectSize().x;
					} // group 1

					ImGui::EndDisabled(); 
				} // game::is_single_brush_selected()

				static float faces_to_terrain_width = 100.0f;
				center_horz_begin(faces_to_terrain_width);
				{
					ImGui::BeginDisabled(game::g_selected_faces_count == 0);
					{
						if (ImGui::Button("Faces to terrain", ImVec2(total_widget_width, 0.0f)))
						{
							cdeclcall(void, 0x429BE0); // CMainFrame::OnFaceToTerrain
						}

						ImGui::EndDisabled();
					}
					center_horz_end(faces_to_terrain_width);
				} // faces to terrain

				treenode_end(style_colors, style_vars);
			} // terrain node

			if (treenode_begin("Curve", true, style_colors, style_vars))
			{
				static int curvepatch_width = 3;
				static int curvepatch_height = 3;
				//static float total_widget_start_offset = 0.0f;
				

				ImGui::BeginDisabled(!game::is_single_brush_selected());
				{
					static float width_height_settings_width = 100.0f;

					const int curve_step = 2;
					const float curve_button_width = 76.0f;
					const float curve_button_spacing = 14.0f;
					const float cursor_y = ImGui::GetCursorPosY();

					// floorf to ensure no half pixel offsets (image 1px borders)
					ImGui::SetCursorPosX(floorf((ImGui::GetWindowWidth() - ImGui::GetCursorPos().x) * 0.5f - (width_height_settings_width * 0.5f) + 4.0f - (curve_button_width * 0.5f) - (curve_button_spacing * 0.5f)));

					//total_widget_start_offset = ImGui::GetCursorPosX();
					widget_start_offset_screen = ImGui::GetCursorScreenPos().x;

					ImGui::BeginGroup();
					{
						ImGui::BeginGroup();
						{
							ImGui::SetNextItemWidth(110.0f);
							if (ImGui::InputScalar("Width##curve", ImGuiDataType_U32, &curvepatch_width, &curve_step, nullptr, "%d"))
							{
								// check or make uneven
								curvepatch_width =
									curvepatch_width % 2 != 0 ? curvepatch_width : curvepatch_width - 1;

								// clamp
								curvepatch_width =
									curvepatch_width < 3 ? 3 :
									curvepatch_width > 15 ? 15 : curvepatch_width;
							}

							ImGui::SetNextItemWidth(110.0f);
							if (ImGui::InputScalar("Height##curve", ImGuiDataType_U32, &curvepatch_height, &curve_step, nullptr, "%d"))
							{
								// check or make uneven
								curvepatch_height = 
									curvepatch_height % 2 != 0 ? curvepatch_height : curvepatch_height - 1;

								// clamp
								curvepatch_height =
									curvepatch_height < 3 ? 3 :
									curvepatch_height > 15 ? 15 : curvepatch_height;
							}

							ImGui::EndGroup(); // terrain width / height

							// only update group width if change is larger then 1px because floorf in 'center_horz_begin' causes a wiggle
							const float new_width = ImGui::GetItemRectSize().x;
							width_height_settings_width = abs(width_height_settings_width - new_width) != 1.0f ? new_width : width_height_settings_width;
						} // group 2

						ImGui::SameLine(0, curve_button_spacing);
						ImGui::SetCursorPosY(cursor_y);

						if (ImGui::Button("Create##curve", ImVec2(curve_button_width, 60.0f)))
						{
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("make simple patch mesh");
							game::Undo_AddBrushList_Selected();

							const auto p = game::Patch_GenericMesh(curvepatch_width, curvepatch_height, cmainframe::activewnd->m_pActiveXY->m_nViewType, true, false);

							game::Patch_Lightmap_Texturing_dirty(p->def->patch);

							game::Undo_EndBrushList_Selected();
							game::Undo_End();
						}

						ImGui::EndGroup(); // total widget
						max_widget_width = ImGui::GetItemRectSize().x;
					} // group 1

					ImGui::EndDisabled(); // game::is_single_brush_selected()
				} // !is_single_brush_selected

				static float faces_to_terrain_width = 100.0f;
				center_horz_begin(faces_to_terrain_width);
				{
					ImGui::BeginDisabled(!is_curve_patch);
					{
						if (ImGui::Button("Curve to terrain", ImVec2(max_widget_width, 0.0f)))
						{
							cdeclcall(void, 0x429B30); // CMainFrame::OnCurveToTerrain
						}

						ImGui::EndDisabled();
					}
					center_horz_end(faces_to_terrain_width);
				} // curve to terrain

				static float subdevision_width = 100.0f;
				center_horz_begin(subdevision_width);
				{
					ImGui::BeginDisabled(!is_curve_patch);
					{
						if (ImGui::Button("-- Subdivision", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x428EE0); // CMainFrame::OnOverBrightShiftDown
						} TT(std::string("Curve Patches: decrease vertex count (decimate)\n" + ggui::hotkey_dialog::get_hotkey_for_command("OverBrightShiftDown", true)).c_str());

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("++ Subdivision", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x428EB0); // CMainFrame::OnOverBrightShiftUp
						} TT(std::string("Curve Patches: increase vertex count (subdivide)\n" + ggui::hotkey_dialog::get_hotkey_for_command("OverBrightShiftUp", true)).c_str());

						ImGui::EndDisabled();
					}
					center_horz_end(subdevision_width);
				} // subdivision

				SPACING(0.0f, 0.0f);
				ImGui::SetCursorScreenPos(ImVec2(widget_start_offset_screen, ImGui::GetCursorScreenPos().y));
				ImGui::title_inside_seperator("Cap Patch", false, max_widget_width, 4.0f, 1.0f); 

				const auto min_coords = ImVec2(widget_start_offset_screen, ImGui::GetCursorScreenPos().y) - ImVec2(0.0f, 0.0f);
				const auto max_coords = ImVec2(min_coords.x + max_widget_width, min_coords.y + 68.0f);

				const auto cap_button_size = ImVec2(48.0f, 48.0f);
				const auto& style = ImGui::GetStyle();

				ImGui::GetWindowDrawList()->AddRectFilled(min_coords, max_coords, 
					is_curve_patch ? ImGui::ColorConvertFloat4ToU32(ImVec4(0.173f, 0.173f, 0.173f, 1.0f))
								   : ImGui::ColorConvertFloat4ToU32(ImVec4(0.175f, 0.175f, 0.175f, 1.0f)), style.FrameRounding);

				SPACING(0.0f, 4.0f);

				static float cap_width = 100.0f;
				center_horz_begin(cap_width);
				{
					ImGui::BeginDisabled(!is_curve_patch);
					{
						static bool hov_bevel;
						if (tb->image_button_label("##patch_cap_bevel"
							, "patch_cap_bevel"
							, false
							, hov_bevel
							, "Create inside bevel"
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &cap_button_size))
						{
							components::pmesh::cap_current(0);
						}

						ImGui::SameLine();

						static bool hov_bevel_inverted;
						if (tb->image_button_label("##patch_cap_bevel_inv"
							, "patch_cap_bevel"
							, true
							, hov_bevel_inverted
							, "Create outside bevel"
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &cap_button_size))
						{
							components::pmesh::cap_current(2);
						}

						ImGui::SameLine(0.0f, 16.0f);

						static bool hov_endcap;
						if (tb->image_button_label(""
							, "patch_cap_endcap"
							, false
							, hov_endcap
							, "Create inside endcap"
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &cap_button_size))
						{
							components::pmesh::cap_current(1);
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
							, &cap_button_size))
						{
							components::pmesh::cap_current(3);
						}

						ImGui::EndDisabled();

						SPACING(0.0f, 4.0f);
					}
					center_horz_end(cap_width);
				} // bevel

				treenode_end(style_colors, style_vars);
			} // curve node

			if (treenode_begin("General / Vertices", true, style_colors, style_vars))
			{
				static float _l0_width = 100.0f;
				center_horz_begin(_l0_width);
				{
					ImGui::BeginDisabled(!is_patch);
					{
						static int thickness_in_units = 8;
						ImGui::SetNextItemWidth(max_widget_width * 0.5f - 2.0f - ImGui::CalcTextSize("Units").x - 20.0f);
						if (ImGui::DragInt("Units##thickness", &thickness_in_units, 0.01f, 1, INT16_MAX))
						{
							thickness_in_units =
								thickness_in_units < 1 ? 1 : thickness_in_units;
						}

						ImGui::SameLine(0, 14.0f);
						if (ImGui::Button("Thicken", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("thicken patch");
							game::Undo_AddBrushList_Selected();

							game::Patch_Thicken(thickness_in_units, true);

							game::Undo_EndBrushList_Selected();
							game::Undo_End();
						}

						ImGui::EndDisabled();
					}

					center_horz_end(_l0_width);
				}

				// #

				SPACING(0.0f, 0.0f);
				ImGui::SetCursorScreenPos(ImVec2(widget_start_offset_screen, ImGui::GetCursorScreenPos().y));
				ImGui::title_inside_seperator("Between Two Vertices", false, max_widget_width, 4.0f, 1.0f);

				static float _l1_width = 100.0f;
				center_horz_begin(_l1_width);
				{
					ImGui::BeginDisabled(!atleast_two_verts_selected);
					{
						if (ImGui::Button("-- Row / Column", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42B0B0); // CMainFrame::OnRemoveTerrainRowColumn
						} TT(std::string("Delete row / column along selected vertices\n" + ggui::hotkey_dialog::get_hotkey_for_command("RemoveTerrainRow", true)).c_str());

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("++ Row / Column", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42B080); // CMainFrame::OnAddTerrainRowColumn
						} TT(std::string("Adds a new row / column inbetween selected vertices\n" + ggui::hotkey_dialog::get_hotkey_for_command("AddTerrainRow", true)).c_str());

						// #

						if (ImGui::Button("Split", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42B0C0); // CMainFrame::OnSplitPatch
						} TT(std::string("Split patch along selected vertices\n" + ggui::hotkey_dialog::get_hotkey_for_command("SplitPatch", true)).c_str());

						ImGui::EndDisabled(); // atleast_two_verts_selected

						ImGui::BeginDisabled(!(atleast_two_verts_selected || is_patch));
						{
							ImGui::SameLine(0.0f, 4.0f);
							if (ImGui::Button("Weld", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
							{
								cdeclcall(void, 0x425510); // CMainFrame::OnSelectionConnect
							} TT(std::string("Weld two vertices or patches if not in vertex edit mode\n" + ggui::hotkey_dialog::get_hotkey_for_command("ConnectSelection", true)).c_str());

							ImGui::EndDisabled();
						}

						ImGui::BeginDisabled(!atleast_two_verts_selected);

						if (ImGui::Button("Extrude", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42B0A0); // CMainFrame::ExtrudeTerrainRow
						} TT(std::string("Extrude row / column at selected vertices\n" + ggui::hotkey_dialog::get_hotkey_for_command("ExtrudeTerrainRow", true)).c_str());

						
						ImGui::EndDisabled();
					} // 2 selected verts
					center_horz_end(_l1_width);
				}

				// #

				SPACING(0.0f, 0.0f);
				ImGui::SetCursorScreenPos(ImVec2(widget_start_offset_screen, ImGui::GetCursorScreenPos().y));
				ImGui::title_inside_seperator("Vertex Color", false, max_widget_width, 4.0f, 1.0f);
				//SPACING(0.0f, 0.0f);

				static float _l2_width = 100.0f;
				center_horz_begin(_l2_width);
				{
					static float vertex_edit_color[4] = {};
					const bool enable_vert_color_edit = game::g_qeglobals->d_num_move_points > 0;

					ImGui::BeginDisabled(!enable_vert_color_edit);
					{
						ImGui::SetNextItemWidth(max_widget_width);
						if (ImGui::ColorPicker4("##vertex_color", vertex_edit_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoSidePreview))
						{
							for (auto pt = 0; pt < game::g_qeglobals->d_num_move_points; pt++)
							{
								const auto vert = game::g_qeglobals->d_move_points[pt];
								vert->vert_color.r = utils::pack_float(vertex_edit_color[0]);
								vert->vert_color.g = utils::pack_float(vertex_edit_color[1]);
								vert->vert_color.b = utils::pack_float(vertex_edit_color[2]);
								vert->vert_color.a = utils::pack_float(vertex_edit_color[3]);
							}

							FOR_ALL_SELECTED_BRUSHES(sb)
							{
								if (sb->patch && sb->patch->def)
								{
									game::Patch_UpdateSelected(sb->patch->def, true);
								}
							}
						}

						ImGui::EndDisabled();
					}
					center_horz_end(_l2_width);
				}


				treenode_end(style_colors, style_vars);
			} // general node

			if (treenode_begin("Primitives", false, style_colors, style_vars))
			{
				static float primitives_width = 100.0f;
				center_horz_begin(primitives_width);
				{
					ImGui::BeginDisabled(!game::is_single_brush_selected());
					{
						if (ImGui::Button("Bevel", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42A450); // CMainFrame::OnCurvePatchbevel
						}

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("Square Bevel", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42B5E0); // CMainFrame::OnCuveSquareBevel
						}

						// #

						if (ImGui::Button("End Cap", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42A4A0); // CMainFrame::OnCurvePatchendcap
						}

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("Square End Cap", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42B590); // CMainFrame::OnCurveSquareEndCap
						}

						// #

						if (ImGui::Button("Cylinder", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42A3B0); // CMainFrame::OnCurvePatchtube
						}

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("Square Cylinder", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42AF00); // CMainFrame::OnCurvePatchsquare
						}

						// #

						if (ImGui::Button("Dense Cylinder", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42AB90); // CMainFrame::OnCurvePatchdensetube
						}

						ImGui::SameLine(0.0f, 4.0f);
						if (ImGui::Button("Very Dense Cyl.", ImVec2(max_widget_width * 0.5f - 2.0f, 0.0f)))
						{
							cdeclcall(void, 0x42AC40); // CMainFrame::OnCurvePatchverydensetube
						}

						// #

						if (ImGui::Button("Cone", ImVec2(max_widget_width, 0.0f)))
						{
							cdeclcall(void, 0x42A360); // CMainFrame::OnCurvePatchcone

							// fix cone not showing before moving or editing the patch in any way afterwards
							const auto b = game::g_selected_brushes();
							if(b && b->def && b->def->patch)
							{
								game::Patch_UpdateSelected(b->def->patch, 1);
							}
							
						}

						ImGui::EndDisabled();
					} // 2 selected verts
					center_horz_end(primitives_width);
				}

				treenode_end(style_colors, style_vars);
			} // primitives node

			SPACING(0.0f, 6.0f);
		}
		ImGui::PopID();
	}

	void toolbox_dialog::child_surface_inspector()
	{
		int style_colors = 0;
		int style_vars = 0;

		ImGui::PushID("surface_inspector");
		setup_child();
		{
			const ImVec4 toolbar_button_background_active = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
			const ImVec4 toolbar_button_background_hovered = ImVec4(0.225f, 0.225f, 0.225f, 1.0f);
			const ImVec2 toolbar_button_size = ImVec2(32.0f, 32.0f);

			const auto tb = GET_GUI(ggui::toolbar_dialog);
			static float max_widget_width = 251.0f; // assumed first value - depends on total width of curve patch creation widget
	
			if (treenode_begin("Manipulation", true, style_colors, style_vars))
			{
				static float manipulation_l1_width = 100.0f;
				center_horz_begin(manipulation_l1_width);
				{
					static bool hov_texflipx;
					if (tb->image_button_label("##texflip_x"
						, "texflip_x"
						, false
						, hov_texflipx
						, "Flip Texture along X-Axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x42BF40); // CMainFrame::OnTextureFlipX
					}

					ImGui::SameLine();
					static bool hov_texflipy;
					if (tb->image_button_label("##texflip_y"
						, "texflip_y"
						, false
						, hov_texflipy
						, "Flip Texture along Y-Axis"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x42BF50); // CMainFrame::OnTextureFlipY
					}

					ImGui::SameLine();
					static bool hov_texflip90;
					if (tb->image_button_label("##texflip_90"
						, "texflip_90"
						, false
						, hov_texflip90
						, "Rotate Texture 90 Degrees"
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x42BF60); // CMainFrame::OnTextureFlip90
					}

					ImGui::SameLine();
					static bool hov_cycle_layer;
					if (tb->image_button_label("##cycle_layer"
						, "cycle_layer"
						, false
						, hov_cycle_layer
						, std::string("Cycle Texture Layer " + ggui::hotkey_dialog::get_hotkey_for_command("TexLayerCycle")).c_str()
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						cdeclcall(void, 0x424010); // CMainFrame::OnEditLayerCycle
					}
					
					center_horz_end(manipulation_l1_width);
				}

				treenode_end(style_colors, style_vars);
			} // manipulation node

			GET_GUI(ggui::surface_dialog)->inspector_controls(true, max_widget_width);
		}
		ImGui::PopID();
	}

	void toolbox_dialog::child_entity_properties()
	{
		int style_colors = 0;
		int style_vars = 0;

		imgui::PushID("entity_properties");
		setup_child();
		{
			const auto ent = GET_GUI(ggui::entity_dialog);
			
			if (treenode_begin("Properties", true, style_colors, style_vars))
			{
				imgui::SetNextItemWidth(-1);
				ent->draw_entprops(ImGui::CalcItemWidth() - 8.0f, 8.0f);

				treenode_end(style_colors, style_vars);
			} // manipulation node

			if (treenode_begin("Spawnflags", false, style_colors, style_vars))
			{
				ent->draw_checkboxes();

				treenode_end(style_colors, style_vars);
			} // manipulation node

			if (treenode_begin("Comments", false, style_colors, style_vars))
			{
				imgui::SetNextItemWidth(-1);
				ent->draw_comments(8.0f);

				treenode_end(style_colors, style_vars);
			} // manipulation node

			SPACING(0.0f, 4.0f);
		}
		imgui::PopID();
	}

	void toolbox_dialog::child_filter()
	{
		int style_colors = 0;
		int style_vars = 0;

		imgui::PushID("filter");
		setup_child();
		{
			const auto gui = GET_GUI(ggui::filter_dialog);

			if (treenode_begin("Geometry Filters", true, style_colors, style_vars))
			{
				gui->geometry_filters(ImGui::GetContentRegionAvail().x - 12.0f);
				treenode_end(style_colors, style_vars);
			} // manipulation node

			if (treenode_begin("Entity Filters", true, style_colors, style_vars))
			{
				gui->entity_filters(ImGui::GetContentRegionAvail().x - 12.0f);
				treenode_end(style_colors, style_vars);
			} // manipulation node

			if (treenode_begin("Trigger Filters", true, style_colors, style_vars))
			{
				gui->trigger_filters(ImGui::GetContentRegionAvail().x - 12.0f);
				treenode_end(style_colors, style_vars);
			} // manipulation node

			if (treenode_begin("Other Filters", true, style_colors, style_vars))
			{
				gui->other_filters(ImGui::GetContentRegionAvail().x - 12.0f);
				treenode_end(style_colors, style_vars);
			} // manipulation node

			SPACING(0.0f, 4.0f);
		}
		imgui::PopID();
	}

	void toolbox_dialog::child_layers()
	{
		imgui::PushID("layers");
		imgui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.125f));

		{
			const auto window = imgui::GetCurrentWindow();

			const auto child_size = imgui::GetContentRegionAvail();
			const float window_height = window->ContentSize.y > window->SizeFull.y ? window->ContentSize.y : window->SizeFull.y;

			const auto min = imgui::GetCursorScreenPos();
			const auto max = ImVec2(min.x + child_size.x, min.y + window_height);
			imgui::GetWindowDrawList()->AddRect(min, max, imgui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)), 0.0f, ImDrawFlags_RoundCornersBottom);
			imgui::Indent(4.0f);
		}

		const auto gui = GET_GUI(ggui::layer_dialog);
		gui->do_table();
		gui->do_context_menu();

		imgui::PopStyleColor();
		imgui::PopID();
	}

	bool toolbox_dialog::gui()
	{
		const auto indent_offset = 8.0f;

		const auto MIN_WINDOW_SIZE = ImVec2(334.0f, 400.0f); 
		const auto INITIAL_WINDOW_SIZE = ImVec2(334.0f, 600.0f);
		ggui::set_next_window_initial_pos_and_constraints(MIN_WINDOW_SIZE, INITIAL_WINDOW_SIZE);

		int stylevars = 0, stylecolors = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); stylevars++;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.08f, 0.31f)); stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.225f, 0.225f, 0.225f, 1.0f)); stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.225f, 0.225f, 0.225f, 1.0f)); stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.225f, 0.225f, 0.225f, 1.0f)); stylecolors++;

		if (!ImGui::Begin("Toolbox##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PopStyleColor(stylecolors);
			ImGui::PopStyleVar(stylevars);
			ImGui::End();
			return false;
		}

		ImGui::Indent(indent_offset - 2.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + indent_offset);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f)); stylevars++; // tooltipps

		// #
		// tab buttons

		ImVec4 toolbar_button_background = ImVec4(0.145f, 0.145f, 0.145f, 1.0f);
		ImVec4 toolbar_button_background_active = ImVec4(0.225f, 0.225f, 0.225f, 1.0f);
		ImVec4 toolbar_button_background_hovered = ImVec4(0.225f, 0.225f, 0.225f, 1.0f);

		ImVec2 toolbar_button_size = ImVec2(32.0f, 32.0f);
		const auto tb = GET_GUI(ggui::toolbar_dialog);
		const auto pre_button_cursor = ImGui::GetCursorPos();

		ImGuiContext& g = *GImGui;
		const auto actual_button_size = ImVec2(28.0f + g.Style.FramePadding.x, 28.0f + g.Style.FramePadding.y);

		ImVec4 button_border_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		ImVec2 mins, maxs;

		// #

		mins = ImGui::GetCursorScreenPos();

		static bool hov_brush;
		if (tb->image_togglebutton("toolbox_brush" // toggle_draw_surfs_portal
			, hov_brush
			, m_child_current == static_cast<int>(_toolbox_childs[CAT_BRUSH].index)
			, "Brushes"
			, &toolbar_button_background
			, &toolbar_button_background_hovered
			, &toolbar_button_background_active
			, &toolbar_button_size))
		{
			m_child_current = static_cast<int>(_toolbox_childs[CAT_BRUSH].index);
			m_update_scroll = true;
		}

		maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
		ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

		// #

		mins = ImGui::GetCursorScreenPos();

		static bool hov_patch;
		if (tb->image_togglebutton("toolbox_patch"
			, hov_patch
			, m_child_current == static_cast<int>(_toolbox_childs[CAT_PATCH].index)
			, "Patches"
			, &toolbar_button_background
			, &toolbar_button_background_hovered
			, &toolbar_button_background_active
			, &toolbar_button_size))
		{
			m_child_current = static_cast<int>(_toolbox_childs[CAT_PATCH].index);
			m_update_scroll = true;
		}

		maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
		ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

		// #

		mins = ImGui::GetCursorScreenPos();

		if (dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.integer == 2)
		{
			static bool hov_surf_inspector;
			if (tb->image_togglebutton("surface_inspector"
				, hov_surf_inspector
				, m_child_current == static_cast<int>(_toolbox_childs[CAT_SURF_INSP].index)
				, "Surface Inspector"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &toolbar_button_background_active
				, &toolbar_button_size))
			{
				m_child_current = static_cast<int>(_toolbox_childs[CAT_SURF_INSP].index);
				m_update_scroll = true;
			}
		}

		maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
		ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

		// #

		mins = ImGui::GetCursorScreenPos();

		if (dvars::gui_props_toolbox && dvars::gui_props_toolbox->current.enabled)
		{
			static bool hov_ent_props;
			if (tb->image_togglebutton("entity_properties"
				, hov_ent_props
				, m_child_current == static_cast<int>(_toolbox_childs[CAT_ENTITY_PROPS].index)
				, "Entity Properties"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &toolbar_button_background_active
				, &toolbar_button_size))
			{
				m_child_current = static_cast<int>(_toolbox_childs[CAT_ENTITY_PROPS].index);
				m_update_scroll = true;
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
		}

		// #

		mins = ImGui::GetCursorScreenPos();

		if (dvars::gui_toolbox_integrate_filter && dvars::gui_toolbox_integrate_filter->current.enabled)
		{
			static bool hov_filter;
			if (tb->image_togglebutton("filter"
				, hov_filter
				, m_child_current == static_cast<int>(_toolbox_childs[CAT_FILTER].index)
				, "Filter"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &toolbar_button_background_active
				, &toolbar_button_size))
			{
				m_child_current = static_cast<int>(_toolbox_childs[CAT_FILTER].index);
				m_update_scroll = true;
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
		}

		// #

		mins = ImGui::GetCursorScreenPos();

		if (dvars::gui_toolbox_integrate_layers && dvars::gui_toolbox_integrate_layers->current.enabled)
		{
			static bool hov_layers;
			if (tb->image_togglebutton("layers"
				, hov_layers
				, m_child_current == static_cast<int>(_toolbox_childs[CAT_LAYERS].index)
				, "Layers"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &toolbar_button_background_active
				, &toolbar_button_size))
			{
				m_child_current = static_cast<int>(_toolbox_childs[CAT_LAYERS].index);
				m_update_scroll = true;
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
		}


		// ------------------------------------------------------

		// #

		if (dvars::gui_toolbox_integrate_cam_toolbar && dvars::gui_toolbox_integrate_cam_toolbar->current.enabled)
		{
			auto cam_toolbar_active_bg = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);

			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));

			SPACING(0.0f, 16.0f);

			const auto prefs = game::g_PrefsDlg();

			mins = ImGui::GetCursorScreenPos();

			static bool hov_guizmo_enable;
			if (tb->image_togglebutton("guizmo_enable"
				, hov_guizmo_enable
				, dvars::guizmo_enable->current.enabled
				, "Enable guizmo"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &cam_toolbar_active_bg
				, &toolbar_button_size))
			{
				dvars::set_bool(dvars::guizmo_enable, !dvars::guizmo_enable->current.enabled);
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			if (dvars::guizmo_enable->current.enabled)
			{
				mins = ImGui::GetCursorScreenPos();

				static bool hov_guizmo_local_world;
				if (tb->image_togglebutton("guizmo_world_local"
					, hov_guizmo_local_world
					, ggui::camera_guizmo::g_guizmo_local
					, std::string("Guizmo: Toggle between local and world mode\nConsiders angle of last selection when used to manipulate multiple objects\n" + ggui::hotkey_dialog::get_hotkey_for_command("guizmo_world_local")).c_str()
					, &toolbar_button_background
					, &toolbar_button_background_hovered
					, &toolbar_button_background_active
					, &toolbar_button_size))
				{
					ggui::camera_guizmo::g_guizmo_local = !ggui::camera_guizmo::g_guizmo_local;
				}

				maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
				ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
			}

			/*if (dvars::guizmo_enable->current.enabled)
			{
				mins = ImGui::GetCursorScreenPos();

				static bool hov_guizmo_grid_snapping;
				if (tb->image_togglebutton("guizmo_grid_snapping"
					, hov_guizmo_grid_snapping
					, dvars::guizmo_snapping->current.enabled
					, "Guizmo: Enable grid-snapping"
					, &toolbar_button_background
					, &toolbar_button_background_hovered
					, &cam_toolbar_active_bg
					, &toolbar_button_size))
				{
					dvars::set_bool(dvars::guizmo_snapping, !dvars::guizmo_snapping->current.enabled);
				}

				maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
				ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
			}*/

			// #

			mins = ImGui::GetCursorScreenPos();

			static bool hov_cubicclip;
			if (tb->image_togglebutton("cubic_clip"
				, hov_cubicclip
				, prefs->m_bCubicClipping
				, std::string("Cubic Clipping " + ggui::hotkey_dialog::get_hotkey_for_command("ToggleCubicClip")).c_str()
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &cam_toolbar_active_bg
				, &toolbar_button_size))
			{
				mainframe_thiscall(LRESULT, 0x428F90); // CMainFrame::OnViewCubicclipping
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			mins = ImGui::GetCursorScreenPos();

			const auto px = components::physx_impl::get();
			static bool hov_movement;
			if (tb->image_togglebutton("physx_movement"
				, hov_movement
				, px->m_character_controller_enabled
				, std::string("Toggle PhysX movement " + ggui::hotkey_dialog::get_hotkey_for_command("physx_movement") + "\nDouble Jump to fly").c_str()
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &toolbar_button_background_active
				, &toolbar_button_size))
			{
				components::physx_impl::spawn_character();
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			mins = ImGui::GetCursorScreenPos();

			static bool hov_gameview;
			if (tb->image_togglebutton("gameview"
				, hov_gameview
				, dvars::radiant_gameview->current.enabled
				, std::string("Gameview " + ggui::hotkey_dialog::get_hotkey_for_command("xo_gameview")).c_str()
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &cam_toolbar_active_bg
				, &toolbar_button_size))
			{
				components::gameview::p_this->set_state(!dvars::radiant_gameview->current.enabled);
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			if (components::d3dbsp::Com_IsBspLoaded())
			{
				SPACING(0.0f, 0.0f);

				if (dvars::gui_camera_toolbar_merge_bsp_buttons && dvars::gui_camera_toolbar_merge_bsp_buttons->current.enabled)
				{
					mins = ImGui::GetCursorScreenPos();

					static bool hov_toggle_bsp_radiant;
					const bool show_bsp_icon = dvars::r_draw_bsp->current.enabled;

					if (tb->image_togglebutton(show_bsp_icon ? "toggle_bsp" : "toggle_radiant_world"
						, hov_toggle_bsp_radiant
						, dvars::r_draw_bsp->current.enabled
						, std::string("Toggle between visibility of the loaded d3dbsp and radiant " + ggui::hotkey_dialog::get_hotkey_for_command("toggle_bsp_radiant") + "\nButton can be split into two via 'Preferences > Camera > Merge BSP/Radiant buttons'").c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &cam_toolbar_active_bg
						, &toolbar_button_size))
					{
						components::command::execute("toggle_bsp_radiant");

					}

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
				}
				else
				{
					mins = ImGui::GetCursorScreenPos();

					static bool hov_bsp_rendering;
					if (tb->image_togglebutton("toggle_bsp"
						, hov_bsp_rendering
						, dvars::r_draw_bsp->current.enabled
						, std::string("Toggle visibility of loaded d3dbsp " + ggui::hotkey_dialog::get_hotkey_for_command("toggle_bsp")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &cam_toolbar_active_bg
						, &toolbar_button_size))
					{
						dvars::set_bool(dvars::r_draw_bsp, !dvars::r_draw_bsp->current.enabled);
					}

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

					// #

					mins = ImGui::GetCursorScreenPos();

					static bool hov_toggle_world;
					const auto gameview = components::gameview::p_this;
					const bool tstate = gameview->get_all_geo_state() || gameview->get_all_ents_state() || gameview->get_all_triggers_state() || gameview->get_all_others_state();

					if (tb->image_togglebutton("toggle_radiant_world"
						, hov_toggle_world
						, !tstate
						, std::string("Toggle radiant rendering " + ggui::hotkey_dialog::get_hotkey_for_command("toggle_filter_all")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &cam_toolbar_active_bg
						, &toolbar_button_size))
					{
						components::command::execute("toggle_filter_all");
					}

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
				}

				// #

				mins = ImGui::GetCursorScreenPos();

				ImGui::PushID("settings3");
				static bool hov_bsp_settings;
				if (tb->image_togglebutton("fakesun_settings"
					, hov_bsp_settings
					, hov_bsp_settings
					, "Open BSP settings menu"
					, &toolbar_button_background
					, &toolbar_button_background_hovered
					, &cam_toolbar_active_bg
					, &toolbar_button_size))
				{
					const auto cs = GET_GUI(ggui::camera_settings_dialog);
					cs->handle_toggle_request(camera_settings_dialog::tab_state_bsp);
					cs->focus_tab(camera_settings_dialog::tab_state_bsp);
				}
				ImGui::PopID();

				maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
				ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
			} // Com_IsBspLoaded

			SPACING(0.0f, 8.0f);

			// #

			mins = ImGui::GetCursorScreenPos();

			static bool hov_fakesunpreview;
			if (tb->image_togglebutton("sunpreview"
				, hov_fakesunpreview
				, dvars::r_fakesun_preview->current.enabled
				, std::string("Fake sun preview " + ggui::hotkey_dialog::get_hotkey_for_command("fakesun_toggle") + "\nSupports specular and bump mapping.\n(Does not affect d3dbsp)").c_str()
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &cam_toolbar_active_bg
				, &toolbar_button_size))
			{
				components::command::execute("fakesun_toggle");
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			mins = ImGui::GetCursorScreenPos();

			static bool hov_fakesun_fog;
			if (tb->image_togglebutton("fakesun_fog"
				, hov_fakesun_fog
				, dvars::r_fakesun_fog_enabled->current.enabled
				, std::string("Toggle Fog " + ggui::hotkey_dialog::get_hotkey_for_command("fakesun_fog_toggle") + "\n(needs 'Fake sun preview' to work on radiant brushes / models)\n(Does affect d3dbsp)").c_str()
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &cam_toolbar_active_bg
				, &toolbar_button_size))
			{
				components::command::execute("fakesun_fog_toggle");
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			mins = ImGui::GetCursorScreenPos();

			static bool hov_filmtweaks_settings;
			const auto r_filmtweakenable = game::Dvar_FindVar("r_filmtweakenable");

			if (tb->image_togglebutton("filmtweaks"
				, hov_filmtweaks_settings
				, r_filmtweakenable->current.enabled
				, std::string("Toggle filmtweaks " + ggui::hotkey_dialog::get_hotkey_for_command("filmtweak_toggle")).c_str()
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &cam_toolbar_active_bg
				, &toolbar_button_size))
			{
				components::command::execute("filmtweak_toggle");
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			mins = ImGui::GetCursorScreenPos();

			static bool hov_fakesun_settings;
			if (tb->image_togglebutton("fakesun_settings"
				, hov_fakesun_settings
				, hov_fakesun_settings
				, "Open Fakesun / PostFX settings menu"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &cam_toolbar_active_bg
				, &toolbar_button_size))
			{
				const auto cs = GET_GUI(ggui::camera_settings_dialog);
				cs->handle_toggle_request(camera_settings_dialog::tab_state_fakesun);
				cs->focus_tab(camera_settings_dialog::tab_state_fakesun);
			}

			maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
			ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

			// #

			SPACING(0.0f, 8.0f);

			const bool can_fx_play = components::effects::effect_can_play();
			ImGui::BeginGroup();
			{
				ImGui::BeginDisabled(!can_fx_play || components::effects::effect_is_repeating());
				{
					mins = ImGui::GetCursorScreenPos();

					static bool hov_fx_play;
					if (tb->image_togglebutton("fx_play"
						, hov_fx_play
						, can_fx_play && !components::effects::effect_is_repeating()
						, std::string("Play Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_play")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background
						, &toolbar_button_size))
					{
						components::effects::play();
					} 

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

					ImGui::EndDisabled();
				}

				ImGui::BeginDisabled(!can_fx_play);
				{
					mins = ImGui::GetCursorScreenPos();

					static bool hov_fx_repeat;
					if (tb->image_togglebutton("fx_repeat"
						, hov_fx_repeat
						, components::effects::effect_is_repeating()
						, std::string("Re-trigger Effect every X seconds for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_repeat")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &cam_toolbar_active_bg
						, &toolbar_button_size))
					{
						components::effects::repeat();
					}

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

					// #

					mins = ImGui::GetCursorScreenPos();

					static bool hov_fx_pause;
					if (tb->image_togglebutton("fx_pause"
						, hov_fx_pause
						, components::effects::effect_is_paused()
						, std::string("Pause Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_pause")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &cam_toolbar_active_bg
						, &toolbar_button_size))
					{
						components::effects::pause();
					}

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

					// #

					mins = ImGui::GetCursorScreenPos();

					static bool hov_fx_stop;
					if (tb->image_togglebutton("fx_stop"
						, hov_fx_stop
						, can_fx_play
						, std::string("Stop Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_stop")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background
						, &toolbar_button_size))
					{
						components::effects::stop();
					}

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

					// #

					mins = ImGui::GetCursorScreenPos();

					static bool hov_fx_edit;
					if (tb->image_togglebutton("fx_edit"
						, hov_fx_edit
						, GET_GUI(ggui::effects_editor_dialog)->is_active()
						, "Edit Effect for last selected fx_origin"
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &cam_toolbar_active_bg
						, &toolbar_button_size))
					{
						components::effects::edit();
					}

					maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
					ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));

					ImGui::EndDisabled();
				}

				ImGui::EndGroup();

				if (!can_fx_play)
				{
					TT("Select an fx_origin to enable controls.");
				}

				// #

				mins = ImGui::GetCursorScreenPos();

				ImGui::PushID("settings2");
				static bool hov_fx_settings;
				if (tb->image_togglebutton("fakesun_settings"
					, hov_fx_settings
					, hov_fx_settings
					, "Open FX settings menu"
					, &toolbar_button_background
					, &toolbar_button_background_hovered
					, &toolbar_button_background_active
					, &toolbar_button_size))
				{
					const auto cs = GET_GUI(ggui::camera_settings_dialog);
					cs->handle_toggle_request(camera_settings_dialog::tab_state_effects);
					cs->focus_tab(camera_settings_dialog::tab_state_effects);
				}
				ImGui::PopID();

				maxs = ImVec2(mins.x + actual_button_size.x, mins.y + actual_button_size.y);
				ImGui::GetWindowDrawList()->AddRect(mins, maxs, ImGui::ColorConvertFloat4ToU32(button_border_color));
			}
		} //ImGui::PopStyleVar();

		// #

		ImGui::SetCursorPosX(pre_button_cursor.x + toolbar_button_size.x - 1.0f); // -1 to hide right button border
		ImGui::SetCursorPosY(pre_button_cursor.y - indent_offset);

		// needs to be this way or ImGui throws an assert when minimizing radiant while having the toolbox window open
		if (!ImGui::BeginChild("##toolbox_child", ImVec2(0, 0), false))
		{
			ImGui::EndChild();
			ImGui::PopStyleColor(stylecolors);
			ImGui::PopStyleVar(stylevars);
			ImGui::End();
			return false;
		}

		// change childs using TAB
		if (!(ImGui::GetIO().KeyMods == ImGuiKeyModFlags_Ctrl) && ImGui::IsWindowHovered() && !ImGui::GetIO().WantTextInput && ImGui::IsKeyReleased(ImGuiKey_Tab))
		{
			m_child_current++;

			if (m_child_current >= static_cast<int>(m_child_count))
			{
				m_child_current = 0;
			}
		}

		// draw selected child
		for (const auto& child : _toolbox_childs)
		{
			if (static_cast<int>(child.second.index) == m_child_current)
			{
				// switch to other child when user no longer has surface inspector / entity properties incorporated
				if (   (dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.integer != 2 && child.first == CAT_SURF_INSP) 
					|| (dvars::gui_props_toolbox && !dvars::gui_props_toolbox->current.enabled && child.first == CAT_ENTITY_PROPS)
					|| (dvars::gui_toolbox_integrate_filter && !dvars::gui_toolbox_integrate_filter->current.enabled && child.first == CAT_FILTER)
					|| (dvars::gui_toolbox_integrate_layers && !dvars::gui_toolbox_integrate_layers->current.enabled && child.first == CAT_LAYERS))
				{
					focus_child(toolbox_dialog::TB_CHILD::BRUSH);
					break;
				}

				if (m_update_scroll)
				{
					ImGui::SetScrollHereY();
					m_update_scroll = false;
				}

				if(child.second.callback)
				child.second.callback();
			}
		}

		if (dvars::gui_saved_state_toolbox_child && dvars::gui_saved_state_toolbox_child->current.integer != m_child_current)
		{
			dvars::set_int(dvars::gui_saved_state_toolbox_child, m_child_current);
		}

		// end "##toolbox_child"
		ImGui::EndChild();

		ImGui::PopStyleColor(stylecolors);
		ImGui::PopStyleVar(stylevars);

		// end "##toolbox_window"
		ImGui::End();

		return true;
	}

	void toolbox_dialog::on_init()
	{
		register_child(CAT_BRUSH, std::bind(&toolbox_dialog::child_brush, this));
		register_child(CAT_PATCH, std::bind(&toolbox_dialog::child_patch, this));
		register_child(CAT_SURF_INSP, std::bind(&toolbox_dialog::child_surface_inspector, this));
		register_child(CAT_ENTITY_PROPS, std::bind(&toolbox_dialog::child_entity_properties, this));
		register_child(CAT_FILTER, std::bind(&toolbox_dialog::child_filter, this));
		register_child(CAT_LAYERS, std::bind(&toolbox_dialog::child_layers, this));

		if (dvars::gui_saved_state_toolbox_child)
		{
			m_child_current = dvars::gui_saved_state_toolbox_child->current.integer;
		}

		// #

		components::command::register_command_with_hotkey("toggle_toolbox"s, [this](auto)
		{
			const auto gui = GET_GUI(ggui::toolbox_dialog);

			if (gui->is_inactive_tab() && gui->is_active())
			{
				gui->set_bring_to_front(true);
				return;
			}

			gui->toggle();
		});
	}


	void toolbox_dialog::register_dvars()
	{
		dvars::gui_props_toolbox = dvars::register_bool(
			/* name		*/ "gui_props_toolbox",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "integrate entity window into toolbox");

		dvars::gui_toolbox_integrate_filter = dvars::register_bool(
			/* name		*/ "gui_toolbox_integrate_filter",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "integrate filter window into toolbox");

		dvars::gui_toolbox_integrate_layers = dvars::register_bool(
			/* name		*/ "gui_toolbox_integrate_layers",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "integrate layer window into toolbox");
	}


	REGISTER_GUI(toolbox_dialog);
}

