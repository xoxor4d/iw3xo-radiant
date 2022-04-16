#include "std_include.hpp"

namespace ggui::grid
{
	struct eclass_context_ent
	{
		std::string token;
		std::string ent_str;
	};

	struct eclass_context_group
	{
		std::string group_name;
		std::string path_including_group;
		std::vector<eclass_context_group> childs;
		std::vector<eclass_context_ent> ents;
	};

	std::vector<eclass_context_group> eclass_context_groups; // grouped entities
	std::vector<eclass_context_ent> eclass_context_ents; // single ungrouped entities

	eclass_context_group* eclass_ctr_curr;
	eclass_context_group* eclass_ctr_prev;

	// ouch
	void build_eclass_context_new()
	{
		const auto entity_gui = GET_GUI(ggui::entity_dialog);

		for (size_t i = 0; i < entity_gui->m_classlist.size(); ++i)
		{
			std::string class_str = entity_gui->m_classlist[i]->name;
			std::vector<std::string> split = utils::split(class_str, '_');

			std::string class_str_next;

			if(i + 1 < entity_gui->m_classlist.size())
			{
				class_str_next = entity_gui->m_classlist[i + 1]->name;
			}
			else
			{
				class_str_next = class_str;
			}

			std::vector<std::string> split_next = utils::split(class_str_next, '_');

			if (!split.empty() && !split_next.empty())
			{
				// cap layer depth at 2 for anything other then actors
				const size_t group_depth = split[0] == "actor" ? 4 : 2;

				for (size_t t = 0; t < split.size(); t++)
				{
					// compare current and next entity token
					if(t < group_depth && t + 1 < split.size() && t < split_next.size() && split[t] == split_next[t])
					{
						// build complete path to current group
						std::string path_str;
						for (size_t cc = 0; cc < t + 1; cc++)
						{
							path_str += (cc ? "_"s : ""s) + split[cc]; // no initial _
						}

						eclass_context_group gr = { split[t], path_str };

						// check if group exists in current layer
						bool exists = false;

						const size_t m_size = eclass_ctr_curr ? eclass_ctr_curr->childs.size() : eclass_context_groups.size();
						for (auto g = 0u; g < m_size; g++)
						{
							std::string* group_str = eclass_ctr_curr ? &eclass_ctr_curr->childs[g].group_name : &eclass_context_groups[g].group_name;
							if (split[t] == *group_str)
							{
								eclass_ctr_curr = eclass_ctr_curr ? &eclass_ctr_curr->childs[g] : &eclass_context_groups[g];
								exists = true;
								break;
							}
						}

						if(!exists)
						{
							eclass_ctr_curr = eclass_ctr_curr ? 
								  &eclass_ctr_curr->childs.emplace_back(gr)
								: &eclass_context_groups.emplace_back(gr);
						}
					}
					else
					{
						// ungrouped entity in subgroup

						// second token and up
						if (eclass_ctr_curr)
						{
							// check if current entity is part of the last group
							bool exists = false;

							for (size_t cc = 0; cc < eclass_ctr_curr->childs.size(); cc++)
							{
								if (split[t] == eclass_ctr_curr->childs[cc].group_name)
								{
									// build child string
									std::string child_str = split[t];
									for (size_t nn = t + 1; nn < split.size(); nn++)
									{
										child_str += "_"s + split[nn];
									}

									eclass_ctr_curr->childs[cc].ents.push_back({ child_str, class_str});
									exists = true;
									break;
								}
							}

							// not part of last group
							if(!exists)
							{
								// build child string
								std::string child_str = split[t];
								for (size_t cc = t + 1; cc < split.size(); cc++)
								{
									child_str += "_"s + split[cc];
								}

								// combine item and group when eg: [weapon_ak47] = item and group -> "context->weapon->ak47->ak47 .. ak47_mp etc"
								if(split[0] == "weapon" && t + 1 == split.size() && split[t] == split_next[t])
								{
									// complete path to group
									std::string path_str;
									for (size_t cc = 0; cc < t + 1; cc++)
									{
										path_str += (cc ? "_"s : ""s) + split[cc]; // no initial _
									}

									eclass_context_group gr = { split[t], path_str };
									gr.ents.push_back({ child_str, class_str });

									eclass_ctr_curr = &eclass_ctr_curr->childs.emplace_back(gr);
								}
								else
								{
									eclass_ctr_curr->ents.push_back({ child_str, class_str });
								}
							}

							// do not compare left over tokens
							break;
						}
						else // first token
						{
							// build child string
							std::string child_str = split[t];
							for (size_t cc = t + 1; cc < split.size(); cc++)
							{
								child_str += "_"s + split[cc];
							}

							bool exists = false;
							if(eclass_ctr_prev)
							{
								// is current ent part of the last group?
								if(child_str.starts_with(eclass_ctr_prev->path_including_group))
								{
									utils::replace(child_str, eclass_ctr_prev->path_including_group + "_"s, "");

									eclass_ctr_prev->ents.push_back({ child_str, class_str });
									exists = true;
									break;
								}
							}

							if (!exists)
							{
								eclass_context_ents.push_back({ child_str, class_str });
							}

							// do not compare left over tokens
							break;
						}
					}
				}

				eclass_ctr_prev = eclass_ctr_curr;
				eclass_ctr_curr = nullptr;
			}
		}

		// fix edge cases
		// currently only fixes weapon_g3 not being within the g3 group
		// because classlist contains "weapon_g3" - "weapon_g36c" - ... g36c - "weapon_g3_acog_mp"
		for (auto& group : eclass_context_groups)
		{
			//if (group.group_name == "weapon")
			{
				// check all seperate ents
				for (size_t w = 0u; w < group.ents.size(); w++)
				{
					// check current ent against all groups
					for (auto& weap_group : group.childs)
					{
						if (weap_group.group_name == group.ents[w].token)
						{
							weap_group.ents.insert(weap_group.ents.begin(), group.ents[w]);
							group.ents.erase(group.ents.begin() + w);
						}
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

		static std::string grid_context_last_spawned_entity;

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
						if (const auto  val = GET_GUI(ggui::entity_dialog)->get_value_for_key_from_epairs(sb->def->owner->epairs, "classname");
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
					if (eclass_context_groups.empty())
					{
						/*auto gui = GET_GUI(ggui::entity_dialog);
						for (size_t i = 0; i < gui->m_classlist.size(); ++i)
						{
							game::printf_to_console(gui->m_classlist[i]->name);
						}*/

						build_eclass_context_new();
					}

					auto handle_menu_item = [&](const eclass_context_group& group) -> void
					{
						for (const auto& str : group.ents)
						{
							const char* c_str = str.ent_str.c_str();
							if(ImGui::MenuItem(c_str))
							{
								game::CreateEntityFromClassname(cxywnd, c_str, cxywnd->m_ptDrag.x, cxywnd->m_ptDrag.y);
								grid_context_last_spawned_entity = str.ent_str;
							}
						}
					};

					// ouch
					if (!eclass_context_groups.empty())
					{
						for (const auto& ca : eclass_context_groups)
						{
							if (ImGui::BeginMenu(ca.group_name.c_str()))
							{
								for (const auto& child : ca.childs)
								{
									if (ImGui::BeginMenu(child.group_name.c_str()))
									{
										for (const auto& aa : child.childs)
										{
											if (ImGui::BeginMenu(aa.group_name.c_str()))
											{
												for (const auto& ab : aa.childs)
												{
													if (ImGui::BeginMenu(ab.group_name.c_str()))
													{
														for (const auto& ac : ab.childs)
														{
															if (ImGui::BeginMenu(ac.group_name.c_str())) // not needed since group depth is capped at 4
															{
																/*for (const auto& str : ac.ents)
																{
																	ImGui::MenuItem(str.ent_str.c_str());
																}*/
																handle_menu_item(ac);
																ImGui::EndMenu();
															}
														}

														/*for (const auto& str : ab.ents)
														{
															ImGui::MenuItem(str.ent_str.c_str());
														}*/
														handle_menu_item(ab);
														ImGui::EndMenu();
													}
												}

												/*for (const auto& str : aa.ents)
												{
													ImGui::MenuItem(str.ent_str.c_str());
												}*/
												handle_menu_item(aa);
												ImGui::EndMenu();
											}
										}

										/*for (const auto& str : child.ents)
										{
											ImGui::MenuItem(str.ent_str.c_str());
										}*/
										handle_menu_item(child);
										ImGui::EndMenu();
									}
								}

								/*for (const auto& str : ca.ents)
								{
									ImGui::MenuItem(str.ent_str.c_str());
								}*/
								handle_menu_item(ca);
								ImGui::EndMenu();
							}
						}
					}

					if(!eclass_context_ents.empty())
					{
						for (const auto& str : eclass_context_ents)
						{
							if(str.ent_str == "worldspawn")
							{
								continue;
							}

							//ImGui::MenuItem(str.ent_str.c_str());

							const char* c_str = str.ent_str.c_str();
							if (ImGui::MenuItem(c_str))
							{
								game::CreateEntityFromClassname(cxywnd, c_str, cxywnd->m_ptDrag.x, cxywnd->m_ptDrag.y);
								grid_context_last_spawned_entity = str.ent_str;
							}
						}
					}

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
				const auto entity_gui = GET_GUI(ggui::entity_dialog);

				// reset manual left mouse capture
				ggui::dragdrop_reset_leftmouse_capture();

				const auto m_selector = ggui::get_rtt_modelselector();
				ggui::entity_dialog::addprop_helper_s no_undo = {};

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

					entity_gui->add_prop("model", m_selector->preview_model_name.c_str(), &no_undo);
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

					entity_gui->add_prop("model", ggui::get_rtt_modelselector()->preview_model_name.c_str(), &no_undo);
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

				ggui::redraw_undocking_triangle(wnd, gridwnd);

				ImGui::EndChild();
			}
		}

		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();
	}
}