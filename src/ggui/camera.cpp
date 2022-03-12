#include "std_include.hpp"

namespace ggui::camera
{
	constexpr float CAM_DEBUG_TEXT_Y_OFFS = 180.0f;

	bool	g_camera_toolbar_state = false;

	// right hand side toolbar
	void toolbar()
	{
		const auto camwnd = ggui::get_rtt_camerawnd();
		const auto prefs = game::g_PrefsDlg();

		ImVec2 toolbar_button_open_size = ImVec2(22.0f, 22.0f);
		ImVec2 toolbar_button_size = ImVec2(32.0f, 32.0f);
		ImVec4 toolbar_button_background = ImVec4(0.1f, 0.1f, 0.1f, 0.55f);
		ImVec4 toolbar_button_background_hovered = ImVec4(0.05f, 0.05f, 0.05f, 0.65f);
		ImVec4 toolbar_button_background_active = ImVec4(0.1f, 0.1f, 0.1f, 0.55f);

		// right side alignment
		static float toolbar_line_width = toolbar_button_size.x + 8.0f; // used as first frame estimate
		const float  collapse_button_offset = (toolbar_button_size.x - toolbar_button_open_size.x) * 0.5f;
		const float  offs = g_camera_toolbar_state ? collapse_button_offset : 0.0f;
		ImGui::SameLine(ImGui::GetWindowWidth() - (toolbar_line_width + 8.0f - offs));

		// offset toolbar vertically
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);

		int _stylevars = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.0f, 4.0f));		_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));		_stylevars++;

		int _stylecolors = 0;
		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));				_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));				_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(70, 70, 70, 0));		_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 0));	_stylecolors++;

		if (dvars::gui_draw_fps && dvars::gui_draw_fps->current.enabled)
		{
			const auto cursor_pos = ImGui::GetCursorPos();
			ImGui::SetCursorPosX(cursor_pos.x - CAM_DEBUG_TEXT_Y_OFFS);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::SetCursorPos(cursor_pos);
		}

		if (components::effects::effect_is_playing() || components::effects::effect_is_paused())
		{
			const auto cursor_pos = ImGui::GetCursorPos();
			ImGui::SetCursorPosX(cursor_pos.x - CAM_DEBUG_TEXT_Y_OFFS);
			ImGui::SetCursorPosY(cursor_pos.y + 16.0f);
			ImGui::Text("Fx Drawsurf Count: %d", components::renderer::effect_drawsurf_count_);
			ImGui::SetCursorPos(cursor_pos);
		}

		// group all so we can get the actual toolbar width for the next frame
		ImGui::BeginGroup();
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + collapse_button_offset);

			static bool hov_open_toolbar;
			if (ggui::toolbar::image_togglebutton("arrow_down"
				, hov_open_toolbar
				, g_camera_toolbar_state
				, "Collapse/Expand camera toolbar"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &toolbar_button_background_active
				, &toolbar_button_open_size))
			{
				g_camera_toolbar_state = g_camera_toolbar_state ? false : true;
			} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

			if (g_camera_toolbar_state)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
				{
					static bool hov_guizmo_enable;
					if (ggui::toolbar::image_togglebutton("guizmo_enable"
						, hov_guizmo_enable
						, dvars::guizmo_enable->current.enabled
						, "Enable guizmo"
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						dvars::set_bool(dvars::guizmo_enable, !dvars::guizmo_enable->current.enabled);
					} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

					if (dvars::guizmo_enable->current.enabled)
					{
						static bool hov_guizmo_grid_snapping;
						if (ggui::toolbar::image_togglebutton("guizmo_grid_snapping"
							, hov_guizmo_grid_snapping
							, dvars::guizmo_snapping->current.enabled
							, "Guizmo: Enable grid-snapping"
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							dvars::set_bool(dvars::guizmo_snapping, !dvars::guizmo_snapping->current.enabled);
						} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);


						static bool hov_guizmo_brush_mode;
						if (ggui::toolbar::image_togglebutton("guizmo_brush_mode"
							, hov_guizmo_brush_mode
							, dvars::guizmo_brush_mode->current.enabled
							, "Guizmo: Enable brush mode"
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							dvars::set_bool(dvars::guizmo_brush_mode, !dvars::guizmo_brush_mode->current.enabled);
						} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);
					}

					ImGui::PopStyleVar();
				}

				SPACING(0.0f, 0.0f);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
				{
					static bool hov_cubicclip;
					if (ggui::toolbar::image_togglebutton("cubic_clip"
						, hov_cubicclip
						, prefs->m_bCubicClipping
						, std::string("Cubic Clipping " + ggui::hotkeys::get_hotkey_for_command("ToggleCubicClip")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						mainframe_thiscall(LRESULT, 0x428F90); // CMainFrame::OnViewCubicclipping
					} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

#if 0
					static bool hov_camera_movement;
					ImGui::BeginGroup();
					{
						ImVec2 prebutton_cursor = ImGui::GetCursorScreenPos();

						if (ggui::toolbar::image_togglebutton("camera_movement"
							, hov_camera_movement
							, prefs->camera_mode
							, "Toggle Camera Movement Mode"
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							mainframe_thiscall(LRESULT, 0x429EB0); // CMainFrame::OnToggleCameraMovementMode
						} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

						if (prefs->camera_mode)
						{
							prebutton_cursor.x += (toolbar_button_size.x * 0.45f);
							prebutton_cursor.y += (toolbar_button_size.y * 0.4f);

							ImGui::PushFontFromIndex(ggui::REGULAR_12PX);
							ImGui::SetCursorScreenPos(prebutton_cursor);
							ImGui::Text("%d/2", prefs->camera_mode);
							ImGui::PopFont();
						}
					}
					ImGui::EndGroup();
#endif

					static bool hov_gameview;
					if (ggui::toolbar::image_togglebutton("gameview"
						, hov_gameview
						, dvars::radiant_gameview->current.enabled
						, std::string("Gameview " + ggui::hotkeys::get_hotkey_for_command("xo_gameview")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						components::gameview::p_this->set_state(!dvars::radiant_gameview->current.enabled);

					} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

					ImGui::PopStyleVar();
				}

				SPACING(0.0f, 0.0f);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
				{
					static bool hov_fakesunpreview;
					if (ggui::toolbar::image_togglebutton("sunpreview"
						, hov_fakesunpreview
						, dvars::r_fakesun_preview->current.enabled
						, std::string("Fake sun preview " + ggui::hotkeys::get_hotkey_for_command("fakesun_toggle") + "\nSupports specular and bump mapping.").c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						components::command::execute("fakesun_toggle");

					} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);


					if (dvars::r_fakesun_preview->current.enabled)
					{
						static bool hov_fakesun_fog;
						if (ggui::toolbar::image_togglebutton("fakesun_fog"
							, hov_fakesun_fog
							, dvars::r_fakesun_fog_enabled->current.enabled
							, std::string("Toggle Fog " + ggui::hotkeys::get_hotkey_for_command("fakesun_fog_toggle")).c_str()
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							components::command::execute("fakesun_fog_toggle");

						} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);
					}


					static bool hov_filmtweaks_settings;
					const auto r_filmtweakenable = game::Dvar_FindVar("r_filmtweakenable");

					if (ggui::toolbar::image_togglebutton("filmtweaks"
						, hov_filmtweaks_settings
						, r_filmtweakenable->current.enabled
						, std::string("Toggle filmtweaks " + ggui::hotkeys::get_hotkey_for_command("filmtweak_toggle")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						components::command::execute("filmtweak_toggle");

					} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);


					static bool hov_fakesun_settings;
					if (ggui::toolbar::image_togglebutton("fakesun_settings"
						, hov_fakesun_settings
						, hov_fakesun_settings
						, "Open Fakesun / PostFX settings menu"
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						if (ggui::camera_settings::get_tabstate_fakesun() && ggui::camera_settings::is_tabstate_fakesun_active())
						{
							// close entire window if tab is in-front
							components::gui::toggle(ggui::state.czwnd.m_camera_settings, 0, true);
						}
						else if (!ggui::state.czwnd.m_camera_settings.menustate)
						{
							// open window with focused fakesun tab
							ggui::camera_settings::set_tabstate_fakesun(true);
							components::gui::toggle(ggui::state.czwnd.m_camera_settings, 0, true);
						}
						else
						{
							// window is open but tab not focused
							ggui::camera_settings::set_tabstate_fakesun(true);
							ggui::camera_settings::focus_fakesun();
						}

					} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

					ImGui::PopStyleVar();
				}

				//if(components::effects::effect_can_play())
				{
					SPACING(0.0f, 0.0f);

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
					{
						const bool can_fx_play = components::effects::effect_can_play();
						ImGui::BeginGroup();
						{
							ImGui::BeginDisabled(!can_fx_play || components::effects::effect_is_repeating());
							{
								static bool hov_fx_play;
								if (ggui::toolbar::image_togglebutton("fx_play"
									, hov_fx_play
									, can_fx_play && !components::effects::effect_is_repeating()
									, std::string("Play Effect for last selected fx_origin " + ggui::hotkeys::get_hotkey_for_command("fx_play")).c_str()
									, &toolbar_button_background
									, &toolbar_button_background_hovered
									, &toolbar_button_background_active
									, &toolbar_button_size))
								{
									components::effects::play();

								} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

								ImGui::EndDisabled();
							}

							ImGui::BeginDisabled(!can_fx_play);
							{
								static bool hov_fx_repeat;
								if (ggui::toolbar::image_togglebutton("fx_repeat"
									, hov_fx_repeat
									, components::effects::effect_is_repeating()
									, std::string("Re-trigger Effect every X seconds for last selected fx_origin " + ggui::hotkeys::get_hotkey_for_command("fx_repeat")).c_str()
									, &toolbar_button_background
									, &toolbar_button_background_hovered
									, &toolbar_button_background_active
									, &toolbar_button_size))
								{
									components::effects::repeat();

								} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);


								static bool hov_fx_pause;
								if (ggui::toolbar::image_togglebutton("fx_pause"
									, hov_fx_pause
									, can_fx_play
									, std::string("Stop Effect for last selected fx_origin " + ggui::hotkeys::get_hotkey_for_command("fx_pause")).c_str()
									, &toolbar_button_background
									, &toolbar_button_background_hovered
									, &toolbar_button_background_active
									, &toolbar_button_size))
								{
									components::effects::pause();

								} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);


								static bool hov_fx_stop;
								if (ggui::toolbar::image_togglebutton("fx_stop"
									, hov_fx_stop
									, can_fx_play
									, std::string("Stop Effect for last selected fx_origin " + ggui::hotkeys::get_hotkey_for_command("fx_stop")).c_str()
									, &toolbar_button_background
									, &toolbar_button_background_hovered
									, &toolbar_button_background_active
									, &toolbar_button_size))
								{
									components::effects::stop();

								} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

								static bool hov_fx_edit;
								if (ggui::toolbar::image_togglebutton("fx_edit"
									, hov_fx_edit
									, hov_fx_edit
									, "Edit Effect for last selected fx_origin"
									, &toolbar_button_background
									, &toolbar_button_background_hovered
									, &toolbar_button_background_active
									, &toolbar_button_size))
								{
									components::effects::edit();

								} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

								ImGui::EndDisabled();
							}

							ImGui::EndGroup();

							if (!can_fx_play)
							{
								TT("Select an fx_origin to enable controls.");
							}

							ImGui::PushID("settings2");
							static bool hov_fx_settings;
							if (ggui::toolbar::image_togglebutton("fakesun_settings"
								, hov_fx_settings
								, hov_fx_settings
								, "Open FX settings menu"
								, &toolbar_button_background
								, &toolbar_button_background_hovered
								, &toolbar_button_background_active
								, &toolbar_button_size))
							{
								if (ggui::camera_settings::get_tabstate_effects() && ggui::camera_settings::is_tabstate_effects_active())
								{
									// close entire window if tab is in-front
									components::gui::toggle(ggui::state.czwnd.m_camera_settings, 0, true);
								}
								else if (!ggui::state.czwnd.m_camera_settings.menustate)
								{
									// open window with focused effects tab
									ggui::camera_settings::set_tabstate_effects(true);
									components::gui::toggle(ggui::state.czwnd.m_camera_settings, 0, true);
								}
								else
								{
									// window is open but tab not focused
									ggui::camera_settings::set_tabstate_effects(true);
									ggui::camera_settings::focus_effects();
								}

							} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);
							ImGui::PopID();
						}

						ImGui::PopStyleVar();
					}
				}
			}

			ImGui::EndGroup();
		}

		ImGui::PopStyleVar(_stylevars);
		ImGui::PopStyleColor(_stylecolors);

		// save width for the next frame
		toolbar_line_width = ImGui::GetItemRectSize().x;
	}


	// right click context menu
	void context_menu()
	{
		const auto camerawnd = ggui::get_rtt_camerawnd();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));

		static game::trace_t cam_trace[20] = {};
		static bool cam_context_menu_open = false;
		static bool cam_context_menu_pending_open = false;

		if (cmainframe::activewnd->m_pCamWnd->cam_was_not_dragged)
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || cam_context_menu_open || cam_context_menu_pending_open)
			{
				if (!cam_context_menu_open)
				{
					cam_context_menu_pending_open = true;

					float dir[3];
					ccamwnd::calculate_ray_direction(camerawnd->cursor_pos_pt.x, static_cast<int>(camerawnd->scene_size_imgui.y) - camerawnd->cursor_pos_pt.y, dir);

					// trace
					utils::hook::call<void(__cdecl)(float* _start, float* _dir, int _contents, game::trace_t* _trace, int _num_traces)>(0x48D7C0)
						(cmainframe::activewnd->m_pCamWnd->camera.origin, dir, 0, cam_trace, 20);

					if (cam_trace[0].brush)
					{
						// sort traces by drawsurf order
						auto trace_array_end = (DWORD)&cam_trace[21];
						const static uint32_t sort_traces_func = 0x408CA0;
						__asm
						{
							pushad;
							mov		esi, trace_array_end;
							mov		edi, offset cam_trace;
							call	sort_traces_func;
							popad;
						}
					}
				}

				if (cam_trace[0].brush)
				{
					if (!ImGui::IsKeyPressed(ImGuiKey_Escape) && ImGui::BeginPopupContextItem("context_menu##camera"))
					{
						cam_context_menu_open = true;
						cam_context_menu_pending_open = false;

						bool any_selected = false;

						for (auto t = 0; t < 20 && cam_trace[t].brush; t++)
						{
							if (cam_trace[t].selected)
							{
								any_selected = true;
							}

							ImGui::PushID(t);
							{
								auto material_name = fx_system::Material_GetName(cam_trace[t].face->visArray->handle);

								if (ImGui::MenuItem(material_name, 0, cam_trace[t].selected))
								{
									if (cam_trace[t].selected)
									{
										game::Brush_Deselect((game::brush_t*)cam_trace[t].brush);
										cam_trace[t].selected = false;
									}
									else
									{
										game::Brush_Select((game::brush_t*)cam_trace[t].brush, false, false, false);
										cam_trace[t].selected = true;
									}
								}

								ImGui::PopID();
							}
						}

						ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
						SEPERATORV(0.0f);
						ImGui::PopStyleColor();

						if (ImGui::MenuItem("Select All"))
						{
							for (auto t = 0; t < 20 && cam_trace[t].brush; t++)
							{
								game::Brush_Select((game::brush_t*)cam_trace[t].brush, false, false, false);
								cam_trace[t].selected = true;
							}
						}

						if (any_selected)
						{
							if (ImGui::MenuItem("Deselect All"))
							{
								for (auto t = 0; t < 20 && cam_trace[t].brush; t++)
								{
									game::Brush_Deselect((game::brush_t*)cam_trace[t].brush);
									cam_trace[t].selected = false;
								}
							}
						}

						ImGui::EndPopup();
					}
					else
					{
						if (cam_context_menu_open)
						{
							cam_context_menu_pending_open = false;
						}

						cam_context_menu_open = false;
					}
				}
				else
				{
					cam_context_menu_open = false;
					cam_context_menu_pending_open = false;
				}
			}
		}

		ImGui::PopStyleVar(2);
	}


	// drag-drop target
	void drag_drop_target(bool& accepted_dragdrop)
	{
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
						game::CreateEntityBrush(0, 0, cmainframe::activewnd->m_pXYWnd);
					}

					// do not open the original modeldialog for this use-case, see: create_entity_from_name_intercept()
					g_block_radiant_modeldialog = true;

					//CreateEntityFromName(classname);
					utils::hook::call<void(__cdecl)(const char*)>(0x465CC0)("misc_model");

					g_block_radiant_modeldialog = false;

					ggui::entity::AddProp("model", m_selector->preview_model_name.c_str(), &no_undo);
					// ^ model dialog -> OpenDialog // CEntityWnd_EntityWndProc

					const auto camerawnd = ggui::get_rtt_camerawnd();

					float dir[3];
					ccamwnd::calculate_ray_direction(camerawnd->cursor_pos_pt.x, static_cast<int>(camerawnd->scene_size_imgui.y) - camerawnd->cursor_pos_pt.y, dir);

					game::trace_t trace = {};
					game::Trace_AllDirectionsIfFailed(cmainframe::activewnd->m_pCamWnd->camera.origin, &trace, dir, 0x1200);

					float origin[3];

					// if trace hit something other then the void
					if (trace.brush)
					{
						utils::vector::ma(cmainframe::activewnd->m_pCamWnd->camera.origin, trace.dist, dir, origin);
					}
					// if trace hit nothing, spawn model infront of the camera
					else
					{
						float dist = 100.0f;
						if (auto model = m_selector->preview_model_ptr;
							model)
						{
							dist += model->radius;
						}

						utils::vector::ma(cmainframe::activewnd->m_pCamWnd->camera.origin, dist, dir, origin);
					}

					char origin_str_buf[64] = {};
					if (sprintf_s(origin_str_buf, "%.3f %.3f %.3f", origin[0], origin[1], origin[2]))
					{
						ggui::entity::AddProp("origin", origin_str_buf, &no_undo);
					}

					game::Undo_End();
				}

				accepted_dragdrop = true;
			}

			ImGui::EndDragDropTarget();
		}
	}


	// render to texture - camera window
	void gui()
	{
		int p_styles = 0;
		int p_colors = 0;

		const auto IO = ImGui::GetIO();
		const auto camerawnd = ggui::get_rtt_camerawnd();
		const auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

		float window_padding = 0.0f;
		if (dvars::gui_rtt_padding_enabled && dvars::gui_rtt_padding_enabled->current.enabled)
		{
			window_padding = dvars::gui_rtt_padding_size ? static_cast<float>(dvars::gui_rtt_padding_size->current.integer) : 6.0f;
		}

		const ImVec2 window_padding_both = ImVec2(window_padding * 2.0f, window_padding * 2.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(window_padding, window_padding)); p_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;
		//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TabUnfocusedActive)); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ToImVec4(dvars::gui_rtt_padding_color->current.vector)); p_colors++;

		if (!camerawnd->one_time_init)
		{
			if (dvars::gui_camera_toolbar_defaultopen && dvars::gui_camera_toolbar_defaultopen->current.enabled)
			{
				g_camera_toolbar_state = true;
			}

			camerawnd->one_time_init = true;
		}

		if (camerawnd->should_set_focus)
		{
			ImGui::SetNextWindowFocus();
			camerawnd->should_set_focus = false;
		}

		ImGui::Begin("Camera Window##rtt", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		if (camerawnd->scene_texture)
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
			camerawnd->scene_size_imgui = ImGui::GetWindowSize() - ImVec2(0.0f, frame_height) - window_padding_both;

			// hack to disable left mouse window movement
			ImGui::BeginChild("scene_child", ImVec2(camera_size.x, camera_size.y + frame_height) + window_padding_both, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				camerawnd->scene_pos_imgui = ImGui::GetCursorScreenPos();
				SetWindowPos(cmainframe::activewnd->m_pCamWnd->GetWindow(), HWND_BOTTOM, (int)camerawnd->scene_pos_imgui.x, (int)camerawnd->scene_pos_imgui.y, (int)camerawnd->scene_size_imgui.x, (int)camerawnd->scene_size_imgui.y, SWP_NOZORDER);

				const auto pre_image_cursor = ImGui::GetCursorPos();

				ImGui::Image(camerawnd->scene_texture, camera_size);
				camerawnd->window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				static bool accepted_dragdrop = false;

				// right click context menu
				context_menu();

				// drag-drop target (modelpreview)
				drag_drop_target(accepted_dragdrop);

				// pop ItemSpacing
				ImGui::PopStyleVar(); p_styles--;

				ImGui::SetCursorPos(pre_image_cursor);
				const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

				camerawnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
				camerawnd->cursor_pos_pt = CPoint((LONG)camerawnd->cursor_pos.x, (LONG)camerawnd->cursor_pos.y);

				// fix top left undock triangle
				ggui::FixDockingTabbarTriangle(wnd, camerawnd);

				// 3d guizmo
				ggui::camera_guizmo::guizmo(camera_size, accepted_dragdrop);

				// right hand side toolbar
				toolbar(); 

				ImGui::EndChild();
			}
		}

		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();
	}
}