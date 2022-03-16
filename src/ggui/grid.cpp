#include "std_include.hpp"

namespace ggui::grid
{
	// gui::render_loop()
	// render to texture - imgui grid window

	void gui()
	{
		int p_styles = 0;
		int p_colors = 0;

		const auto IO = ImGui::GetIO();
		const auto cxy_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nWidth), static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nHeight));
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

		float window_padding = 0.0f;

		if (dvars::gui_rtt_padding_enabled && dvars::gui_rtt_padding_enabled->current.enabled) 
		{
			  window_padding = dvars::gui_rtt_padding_size ? static_cast<float>( dvars::gui_rtt_padding_size->current.integer ) : 6.0f;
		}

		const ImVec2 window_padding_both = ImVec2(window_padding * 2.0f, window_padding * 2.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(window_padding, window_padding)); p_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;

		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ToImVec4(dvars::gui_rtt_padding_color->current.vector)); p_colors++;

		auto gridwnd = ggui::get_rtt_gridwnd();
		if (gridwnd->should_set_focus)
		{
			ImGui::SetNextWindowFocus();
			gridwnd->should_set_focus = false;
		}

		ImGui::Begin("Grid Window##rtt", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

		if (gridwnd->scene_texture)
		{
			bool tabbar_visible = true;
			const auto wnd = ImGui::GetCurrentWindow();

			if (ImGui::IsWindowDocked() && wnd)
			{
				if (wnd->DockNode && wnd->DockNode->IsHiddenTabBar())
				{
					tabbar_visible = false;
				}
			}

			const float frame_height = tabbar_visible ? ImGui::GetFrameHeightWithSpacing() : 0.0f;
			gridwnd->scene_size_imgui = ImGui::GetWindowSize() - ImVec2(0.0f, frame_height) - window_padding_both;

			// hack to disable left mouse window movement
			ImGui::BeginChild("scene_child_cxy", ImVec2(cxy_size.x, cxy_size.y + frame_height) + window_padding_both, false, ImGuiWindowFlags_NoMove);
			{
				const auto screenpos = ImGui::GetCursorScreenPos();
				SetWindowPos(cmainframe::activewnd->m_pXYWnd->m_hWnd, HWND_BOTTOM, (int)screenpos.x, (int)screenpos.y, (int)gridwnd->scene_size_imgui.x, (int)gridwnd->scene_size_imgui.y, SWP_NOZORDER);

				const auto pre_image_cursor = ImGui::GetCursorPos();

				ImGui::Image(gridwnd->scene_texture, cxy_size);
				gridwnd->window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);


				// model selection drop target
				if (ImGui::BeginDragDropTarget())
				{
					if (ImGui::AcceptDragDropPayload("MODEL_SELECTOR_ITEM"))
					{
						// reset manual left mouse capture
						ggui::dragdrop_reset_leftmouse_capture();

						const auto m_selector = ggui::get_rtt_modelselector();
						ggui::entity::addprop_helper_s no_undo = {};

						if (m_selector->overwrite_selection)
						{
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("change entity model");

							if (components::remote_net::selection_is_brush(game::g_selected_brushes()->def))
							{
								// nothing but worldspawn selected, lets spawn a new entity
								goto SPAWN_AWAY;
							}

							if ((DWORD*)game::g_selected_brushes_next() == game::currSelectedBrushes)
							{
								// nothing but worldspawn selected, lets spawn a new entity
								goto SPAWN_AWAY;
							}

							ggui::entity::AddProp("model", m_selector->preview_model_name.c_str(), &no_undo);
							game::Undo_End();
						}
						else
						{
							game::Select_Deselect(1);
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("create entity");

							if ((DWORD*)game::g_selected_brushes_next() == game::currSelectedBrushes)
							{
							SPAWN_AWAY:
								game::CreateEntityBrush(static_cast<int>(gridwnd->scene_size_imgui.y) - gridwnd->cursor_pos_pt.y, gridwnd->cursor_pos_pt.x, cmainframe::activewnd->m_pXYWnd);
							}

							// do not open the original modeldialog for this use-case, see: create_entity_from_name_intercept()
							g_block_radiant_modeldialog = true;

							//CreateEntityFromName(classname);
							utils::hook::call<void(__cdecl)(const char*)>(0x465CC0)("misc_model");

							g_block_radiant_modeldialog = false;

							ggui::entity::AddProp("model", ggui::get_rtt_modelselector()->preview_model_name.c_str(), &no_undo);
							// ^ model dialog -> OpenDialog // CEntityWnd_EntityWndProc

							game::Undo_End();
						}
					}
				}

				// pop ItemSpacing
				ImGui::PopStyleVar(); p_styles--;

				ImGui::SetCursorPos(pre_image_cursor);
				const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

				gridwnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
				gridwnd->cursor_pos_pt = CPoint((LONG)gridwnd->cursor_pos.x, (LONG)gridwnd->cursor_pos.y);

				ggui::FixDockingTabbarTriangle(wnd, gridwnd);

				ImGui::EndChild();
			}
		}

		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();
	}
}