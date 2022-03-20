#include "std_include.hpp"

namespace ggui::grid
{
	struct eclass_context_helper
	{
		bool is_single;
		std::string group_name;
		std::vector<std::string> childs;
	};

	std::vector<eclass_context_helper> eclass_context_menus;

	void build_eclass_context()
	{
		int current_dir_idx = 0;

		for (size_t i = 0; i < entity::classlist.size(); ++i)
		{
			std::string class_str = entity::classlist[i]->name;
			std::vector<std::string> split = utils::split(class_str, '_');

			// first element
			if(!i)
			{
				eclass_context_helper t = { false, split[0] };
				eclass_context_menus.push_back(t);
				eclass_context_menus[0].childs.push_back(class_str);

				continue;
			}

			std::string class_str_prev = entity::classlist[i - 1]->name;
			std::vector<std::string> split_prev = utils::split(class_str_prev, '_');

			if(split[0] == split_prev[0])
			{
				eclass_context_menus[current_dir_idx].childs.push_back(class_str);
			}
			else
			{
				// check for single element group
				if(eclass_context_menus[current_dir_idx].childs.size() <= 1)
				{
					eclass_context_menus[current_dir_idx].is_single = true;
				}

				current_dir_idx++;
				eclass_context_helper t = { false, split[0] };
				eclass_context_menus.push_back(t);
				eclass_context_menus[current_dir_idx].childs.push_back(class_str);

				// check if current element is the last element
				if(i + 1 >= entity::classlist.size())
				{
					// check for single element group
					if (eclass_context_menus[current_dir_idx].childs.size() <= 1)
					{
						eclass_context_menus[current_dir_idx].is_single = true;
					}
				}
			}
		}
	}

	// right click context menu
	void context_menu()
	{
		const auto cxywnd = cmainframe::activewnd->m_pXYWnd;

		static bool grid_context_open = false;
		static bool grid_context_pending_open = false;

		static std::string grid_context_last_spawned_entity = "";

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			if(cxywnd->m_ptDrag.x == cxywnd->m_ptDown.x && cxywnd->m_ptDrag.y == cxywnd->m_ptDown.y)
			{
				grid_context_pending_open = true;
			}
		}

		if(grid_context_open || grid_context_pending_open)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));

			if (!ImGui::IsKeyPressed(ImGuiKey_Escape) && ImGui::BeginPopupContextItem("context_menu##grid"))
			{
				grid_context_open = true;

				ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

				if (game::is_any_brush_selected())
				{
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

						if(ImGui::MenuItem("Add Selection To Active Layer"))
						{
							// CXYWnd::OnSelectionAddToActiveLayer
							utils::hook::call<void(__fastcall)(void*)>(0x466930)(cmainframe::activewnd->m_pXYWnd);
						}

						ImGui::EndMenu();
					}

					SEPERATORV(0.0f);

					const auto sb = game::g_selected_brushes();

					if (sb && sb->def && sb->def->owner)
					{
						if (const auto  val = entity::ValueForKey(sb->def->owner->epairs, "classname");
										val && val == "misc_prefab"s)
						{
							if (ImGui::MenuItem("Enter Prefab"))
							{
								// CMainFrame::OnPrefabEnter
								cdeclcall(void, 0x42BF70);
							}

							if (ImGui::MenuItem("Leave Prefab"))
							{
								// CMainFrame::OnPrefabLeave
								cdeclcall(void, 0x42BF80);
							}

							SEPERATORV(0.0f);
						}
					}
				}
				else if (game::g_prefab_stack_level) // nothing selected and within prefab
				{
					if (ImGui::MenuItem("Leave Prefab"))
					{
						cdeclcall(void, 0x42BF80);
					}

					SEPERATORV(0.0f);
				}

				if (ImGui::BeginMenu("Create Entity"))
				{
					// create entity list
					if (eclass_context_menus.empty())
					{
						build_eclass_context();
					}

					if(!eclass_context_menus.empty())
					{
						for(const auto& ca : eclass_context_menus)
						{
							if(ca.group_name == "worldspawn")
							{
								continue;
							}

							if(ca.is_single)
							{
								const char* eclass_str = ca.childs[0].c_str();
								if (ImGui::MenuItem(eclass_str))
								{
									game::CreateEntityFromClassname(cxywnd, eclass_str, cxywnd->m_ptDrag.x, cxywnd->m_ptDrag.y);
									grid_context_last_spawned_entity = eclass_str;
								}
							}
							else
							{
								if (ImGui::BeginMenu(ca.group_name.c_str()))
								{
									for(const auto& child : ca.childs)
									{
										const char* eclass_str = child.c_str();
										if (ImGui::MenuItem(eclass_str))
										{
											game::CreateEntityFromClassname(cxywnd, eclass_str, cxywnd->m_ptDrag.x, cxywnd->m_ptDrag.y);
											grid_context_last_spawned_entity = eclass_str;
										}
									}

									ImGui::EndMenu();
								}
							}
						}
					}

					// we can not iterate over g_eclass because it also holds classes not part of the project
					/*for(const auto eclass : entity::classlist)
					{
						if (ImGui::MenuItem(eclass->name))
						{
							game::CreateEntityFromClassname(cxywnd, eclass->name, cxywnd->m_ptDrag.x, cxywnd->m_ptDrag.y);
						}
					}*/

					ImGui::EndMenu();
				}

				if(!grid_context_last_spawned_entity.empty())
				{
					const char* last_ent_str = grid_context_last_spawned_entity.c_str();
					if (ImGui::MenuItem(last_ent_str))
					{
						game::CreateEntityFromClassname(cxywnd, last_ent_str, cxywnd->m_ptDrag.x, cxywnd->m_ptDrag.y);
					}
				}
				

				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}
			else
			{
				if (grid_context_open)
				{
					grid_context_pending_open = false;
				}

				grid_context_open = false;
			}

			ImGui::PopStyleVar(2);
		}
		
	}

	// drag-drop target
	void drag_drop_target()
	{
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
						const auto gridwnd = ggui::get_rtt_gridwnd();
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
	}

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

		const auto gridwnd = ggui::get_rtt_gridwnd();
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

				// right click context menu
				if(dvars::gui_use_new_context_menu->current.enabled)
				{
					context_menu();
				}

				// model selection drop target
				drag_drop_target();

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