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

	bool toolbox_treenode_begin(int& style_colors, int& style_vars)
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

		ImGui::GetWindowDrawList()->AddRectFilled(min_coords, max_coords, ImGui::ColorConvertFloat4ToU32(ImVec4(0.25f, 0.25f, 0.25f, 1.0f)));

		if (!ImGui::TreeNodeEx("Brush Contents", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PopStyleColor(style_colors);
			ImGui::PopStyleVar(style_vars);
			return false;
		}

		ImGui::PopStyleVar(2); style_vars = style_vars - 2; // ImGuiStyleVar_ItemSpacing
		SPACING(0.0f, 0.0f);
		return true;
	}

	void toolbox_treenode_end(int style_colors, int style_vars)
	{
		ImGui::TreePop();
		ImGui::PopStyleColor(style_colors);
		ImGui::PopStyleVar(style_vars);
	}

	// ---

	void toolbox_dialog::child_brush()
	{
		const float vert_sep_space = 6.0f;

		setup_child();
		{
			ImVec4 toolbar_button_background = ImVec4(0.145f, 0.145f, 0.145f, 1.0f);
			ImVec4 toolbar_button_background_active = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
			ImVec4 toolbar_button_background_hovered = ImVec4(0.225f, 0.225f, 0.225f, 1.0f);
			ImVec2 toolbar_button_size = ImVec2(32.0f, 32.0f);

			const auto tb = GET_GUI(ggui::toolbar_dialog);

			ImGui::title_with_seperator_helpmark("Flip Selection", false, 0, 1.0f, 2.0f,
				"Flips selection along axis");

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

			SPACING(0.0f, vert_sep_space);

			ImGui::title_with_seperator_helpmark("Rotate Selection", false, 0, 1.0f, 2.0f,
				"Rotate selection around axis");

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

			if(ImGui::Button("Hollow Brush"))
			{
				cdeclcall(void, 0x425570); // CMainFrame::OnSelectionMakehollow
			}
			ImGui::SameLine();
			if (ImGui::Button("Auto Caulk"))
			{
				cdeclcall(void, 0x425600); // CMainFrame::OnSelectionAutoCaulk
			}

			SPACING(0.0f, vert_sep_space);

			int style_colors = 0;
			int style_vars = 0;

			if(toolbox_treenode_begin(style_colors, style_vars))
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

				if (ImGui::Button("Structural", ImVec2(120.0f, 0.0f)))
				{
					cdeclcall(void, 0x426200); // CMainFrame::OnSelectionMakeStructural
				}

				ImGui::SameLine();
				if (ImGui::Button("Weapon Clip", ImVec2(120.0f, 0.0f)))
				{
					cdeclcall(void, 0x426240); // CMainFrame::OnSelectionMakeWeaponclip
				}

				toolbox_treenode_end(style_colors, style_vars);
			}

			SPACING(0.0f, vert_sep_space);

			ImGui::title_with_seperator("Brush Primitives", false, 0, 1.0f, 2.0f);

			const int step_size = 1;
			static int sides_cone = 8;
			static int sides_cylinder = 8;

			ImGui::BeginDisabled(!game::is_single_brush_selected());
			{
				ImGui::SetNextItemWidth(160.0f);
				if (ImGui::InputScalar("##sides_cone", ImGuiDataType_U32, &sides_cone, &step_size, nullptr, "%d"))
				{
					sides_cone = sides_cone < 3 ? 3 : sides_cone;
				}
				ImGui::SameLine();
				if (ImGui::Button("Make Cone"))
				{
					game::Undo_ClearRedo();
					game::Undo_GeneralStart("make cone");
					game::Undo_AddBrushList_Selected();

					game::Brush_MakeSidedCone(sides_cone);

					game::Undo_EndBrushList_Selected();
					game::Undo_End();
				}

				ImGui::SetNextItemWidth(160.0f);
				if (ImGui::InputScalar("##sides_cylinder", ImGuiDataType_U32, &sides_cylinder, &step_size, nullptr, "%d"))
				{
					sides_cylinder = sides_cylinder < 3 ? 3 : sides_cylinder;
				}
				ImGui::SameLine();
				if (ImGui::Button("Make Cylinder"))
				{
					game::Undo_ClearRedo();
					game::Undo_GeneralStart("make cylinder");
					game::Undo_AddBrushList_Selected();

					game::Brush_MakeSided_Axis(sides_cone, true);

					game::Undo_EndBrushList_Selected();
					game::Undo_End();
				}

				ImGui::EndDisabled();
			}
		}
	}

	void toolbox_dialog::child_patch()
	{
		setup_child();
		{
			const auto prefs = game::g_PrefsDlg();

			ImGui::title_with_seperator("Load / Save", false);

			ImGui::Checkbox("Loose changes dialog on exit", &prefs->loose_changes);
			ImGui::Checkbox("Enable snapshots", &prefs->m_bSnapShots);

			// -----------------
			ImGui::title_with_seperator("System Settings");
			ImGui::DragInt("Max fps grid window", &dvars::radiant_maxfps_grid->current.integer, 0.1f, dvars::radiant_maxfps_grid->domain.integer.min, dvars::radiant_maxfps_grid->domain.integer.max);
			ImGui::DragInt("Max fps camera window", &dvars::radiant_maxfps_camera->current.integer, 0.1f, dvars::radiant_maxfps_camera->domain.integer.min, dvars::radiant_maxfps_camera->domain.integer.max);
			ImGui::DragInt("Max fps texture window", &dvars::radiant_maxfps_textures->current.integer, 0.1f, dvars::radiant_maxfps_textures->domain.integer.min, dvars::radiant_maxfps_textures->domain.integer.max);
			ImGui::DragInt("Max fps modelselector window", &dvars::radiant_maxfps_modelselector->current.integer, 0.1f, dvars::radiant_maxfps_modelselector->domain.integer.min, dvars::radiant_maxfps_modelselector->domain.integer.max);
			ImGui::DragInt("Max fps gui", &dvars::radiant_maxfps_mainframe->current.integer, 0.1f, dvars::radiant_maxfps_mainframe->domain.integer.min, dvars::radiant_maxfps_mainframe->domain.integer.max);
		}
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

		/*if (ImGui::BeginListBox("##pref_cat", ImVec2(180.0f, ImGui::GetContentRegionAvail().y)))
		{
			int i = 0;
			for (const auto& child : _toolbox_childs)
			{
				const bool is_selected = (m_child_current == i);
				if (ImGui::Selectable(child.first.c_str(), is_selected))
				{
					m_child_current = i;
					m_update_scroll = true;
				}

				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}

				i++;
			}

			ImGui::EndListBox();
		}*/

		ImGui::SetCursorPosX(pre_button_cursor.x + toolbar_button_size.x - 1.0f); // -1 to hide right button border
		ImGui::SetCursorPosY(pre_button_cursor.y - indent_offset);
		//ImGui::SetCursorPosY(0.0f);

		// needs to be this way or ImGui throws an assert when minimizing radiant while having the toolbox window open
		if (!ImGui::BeginChild("##pref_child", ImVec2(0, 0), false))
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

		// end "##pref_child"
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

