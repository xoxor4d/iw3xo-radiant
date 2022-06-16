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
		const float child_indent = 12.0f;
		const auto child_size = ImGui::GetContentRegionAvail();

		const auto min = ImGui::GetCursorScreenPos();
		const auto max = ImVec2(min.x + child_size.x, min.y + child_size.y);
		ImGui::GetWindowDrawList()->AddRect(min, max, ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)), 0.0f, ImDrawFlags_RoundCornersBottom);

		SPACING(0.0f, 6.0f);
		ImGui::Indent(child_indent);
	}

	bool toolbox_treenode_begin(const char* name, bool default_open, int& style_colors, int& style_vars)
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

		if (!ImGui::TreeNodeEx(name, default_open ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None))
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
	void toolbox_treenode_end(int style_colors, int style_vars, float end_spacing = 8.0f)
	{
		ImGui::TreePop();
		ImGui::PopStyleColor(style_colors);
		ImGui::PopStyleVar(style_vars);

		SPACING(0.0f, end_spacing);
	}

	void center_horz_begin(const float group_width, float indent = 4.0f)
	{
		if (group_width != 0.0f)
		{
			// floorf to ensure no half pixel offsets (image 1px borders)
			ImGui::SetCursorPosX(floorf((ImGui::GetWindowWidth() - ImGui::GetCursorPos().x) * 0.5f - (group_width * 0.5f) + indent));
		}

		ImGui::BeginGroup();
	}

	void center_horz_end(float& group_width)
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

			
			if (toolbox_treenode_begin("Manipulation", true, style_colors, style_vars))
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

				SPACING(0.0f, 0.0f);

				static float manipulation_l2_width = 100.0f;
				center_horz_begin(manipulation_l2_width);
				{
					if (ImGui::Button("Hollow Brush"))
					{
						cdeclcall(void, 0x425570); // CMainFrame::OnSelectionMakehollow
					}
					ImGui::SameLine();
					if (ImGui::Button("Auto Caulk"))
					{
						cdeclcall(void, 0x425600); // CMainFrame::OnSelectionAutoCaulk
					}

					center_horz_end(manipulation_l2_width);
				}

				toolbox_treenode_end(style_colors, style_vars);
			}

			if(toolbox_treenode_begin("Brush Contents", true, style_colors, style_vars))
			{
				static float brush_contents_l1_width = 100.0f;
				center_horz_begin(brush_contents_l1_width);
				{
					if (ImGui::Button("Detail", ImVec2(120.0f, 0.0f)))
					{
						cdeclcall(void, 0x4261C0); // CMainFrame::OnSelectionMakeDetail
					}

					ImGui::SameLine();
					if (ImGui::Button("Non-Colliding", ImVec2(120.0f, 0.0f)))
					{
						cdeclcall(void, 0x426280); // CMainFrame::OnSelectionMakeNonColliding
					}
				}
				center_horz_end(brush_contents_l1_width);

				SPACING(0.0f, 0.0f);

				static float brush_contents_l2_width = 100.0f;
				center_horz_begin(brush_contents_l2_width);
				{
					if (ImGui::Button("Structural", ImVec2(120.0f, 0.0f)))
					{
						cdeclcall(void, 0x426200); // CMainFrame::OnSelectionMakeStructural
					}

					ImGui::SameLine();
					if (ImGui::Button("Weapon Clip", ImVec2(120.0f, 0.0f)))
					{
						cdeclcall(void, 0x426240); // CMainFrame::OnSelectionMakeWeaponclip
					}
				}
				center_horz_end(brush_contents_l2_width);

				toolbox_treenode_end(style_colors, style_vars);
			}

			if (toolbox_treenode_begin("Brush Primitives", true, style_colors, style_vars))
			{
				const int step_size = 1;
				static int sides_cone = 8;
				static int sides_cylinder = 8;

				ImGui::BeginDisabled(!game::is_single_brush_selected());
				{
					static float cone_l1_width = 100.0f;
					center_horz_begin(cone_l1_width);
					{
						ImGui::SetNextItemWidth(130.0f);
						if (ImGui::InputScalar("##sides_cone", ImGuiDataType_U32, &sides_cone, &step_size, nullptr, "%d"))
						{
							sides_cone = sides_cone < 3 ? 3 : sides_cone;
						}
						ImGui::SameLine();
						if (ImGui::Button("Make Cone", ImVec2(110.0f, 0.0f)))
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
						ImGui::SetNextItemWidth(130.0f);
						if (ImGui::InputScalar("##sides_cylinder", ImGuiDataType_U32, &sides_cylinder, &step_size, nullptr, "%d"))
						{
							sides_cylinder = sides_cylinder < 3 ? 3 : sides_cylinder;
						}
						ImGui::SameLine();
						if (ImGui::Button("Make Cylinder", ImVec2(110.0f, 0.0f)))
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
				
				toolbox_treenode_end(style_colors, style_vars);
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

			if (toolbox_treenode_begin("Manipulation", true, style_colors, style_vars))
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

				toolbox_treenode_end(style_colors, style_vars);
			}

			if (toolbox_treenode_begin("Terrain", true, style_colors, style_vars))
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
						}

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
					}

					ImGui::EndDisabled(); // game::is_single_brush_selected()
				}

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
				}

				toolbox_treenode_end(style_colors, style_vars);
			}

			if (toolbox_treenode_begin("Curve", true, style_colors, style_vars))
			{
				const int step_size = 1;
				static int curvepatch_width = 2;
				static int curvepatch_height = 2;
				static float total_widget_width = 100.0f;

				ImGui::BeginDisabled(!game::is_single_brush_selected());
				{
					static float width_height_settings_width = 100.0f;

					const int curve_step = 2;
					const float curve_button_width = 76.0f;
					const float curve_button_spacing = 14.0f;
					const float cursor_y = ImGui::GetCursorPosY();

					// floorf to ensure no half pixel offsets (image 1px borders)
					ImGui::SetCursorPosX(floorf((ImGui::GetWindowWidth() - ImGui::GetCursorPos().x) * 0.5f - (width_height_settings_width * 0.5f) + 4.0f - (curve_button_width * 0.5f) - (curve_button_spacing * 0.5f)));

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
						}

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
						total_widget_width = ImGui::GetItemRectSize().x;
					}

					ImGui::EndDisabled(); // game::is_single_brush_selected()
				}

				static float faces_to_terrain_width = 100.0f;
				center_horz_begin(faces_to_terrain_width);
				{
					const auto curr = game::g_selected_brushes();
					//if(curr && curr->def && curr->def->patch && curr->def->patch->flags == game::PATCH_TYPE::PATCH_GENERIC)

					ImGui::BeginDisabled(!(curr && curr->def && curr->def->patch && (curr->def->patch->type == game::PATCH_TYPE::PATCH_GENERIC || curr->def->patch->type == game::PATCH_TYPE::PATCH_SEAM)));
					{
						if (ImGui::Button("Curve to terrain", ImVec2(total_widget_width, 0.0f)))
						{
							cdeclcall(void, 0x429B30); // CMainFrame::OnCurveToTerrain
						}

						ImGui::EndDisabled();
					}
					center_horz_end(faces_to_terrain_width);
				}

				toolbox_treenode_end(style_colors, style_vars);
			}
		}
		ImGui::PopID();
	}

	void toolbox_dialog::gui()
	{
		if (!this->is_initiated())
		{
			register_child(CAT_BRUSH, std::bind(&toolbox_dialog::child_brush, this));
			register_child(CAT_PATCH, std::bind(&toolbox_dialog::child_patch, this));

			this->set_initiated();
		}

		const auto indent_offset = 8.0f;

		const auto MIN_WINDOW_SIZE = ImVec2(800.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(800.0f, 600.0f);
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
			return;
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

		static bool hov_brush;
		if (tb->image_togglebutton("toggle_draw_surfs_portal"
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

		static bool hov_patch;
		if(tb->image_togglebutton("cycle_patch_edge_direction"
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

		ImGui::SetCursorPosX(pre_button_cursor.x + toolbar_button_size.x - 1.0f); // -1 to hide right button border
		ImGui::SetCursorPosY(pre_button_cursor.y - indent_offset);

		// needs to be this way or ImGui throws an assert when minimizing radiant while having the toolbox window open
		if (!ImGui::BeginChild("##toolbox_child", ImVec2(0, 0), false))
		{
			ImGui::EndChild();
			ImGui::PopStyleColor(stylecolors);
			ImGui::PopStyleVar(stylevars);
			ImGui::End();
			return;
		}

		// draw selected childs 
		for (const auto& child : _toolbox_childs)
		{
			if (static_cast<int>(child.second.index) == m_child_current)
			{
				child.second.callback();
			}
		}

		// end "##toolbox_child"
		ImGui::EndChild();

		ImGui::PopStyleColor(stylecolors);
		ImGui::PopStyleVar(stylevars);

		// end "##toolbox_window"
		ImGui::End();
	}

	void toolbox_dialog::init()
	{
		components::command::register_command_with_hotkey("toggle_toolbox"s, [this](auto)
		{
			GET_GUI(ggui::toolbox_dialog)->toggle();
		});
	}


	REGISTER_GUI(toolbox_dialog);
}

