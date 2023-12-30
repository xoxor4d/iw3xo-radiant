#include "std_include.hpp"

namespace ggui
{
	// right hand side toolbar
	void camera_dialog::toolbar()
	{
		bool hide_cam_toolbar = false;

		if (dvars::gui_toolbox_integrate_cam_toolbar && dvars::gui_toolbox_integrate_cam_toolbar->current.enabled)
		{
			const auto toolbox = GET_GUI(ggui::toolbox_dialog);

			if (toolbox->is_active() && !toolbox->is_inactive_tab())
			{
				hide_cam_toolbar = true;
			}
		}

		const auto CAM_DEBUG_TEXT_Y_OFFS = hide_cam_toolbar ? 140.0f : 180.0f;
		const auto prefs = game::g_PrefsDlg();

		ImVec2 toolbar_button_open_size = ImVec2(22.0f, 22.0f);
		ImVec2 toolbar_button_size = ImVec2(32.0f, 32.0f);
		ImVec4 toolbar_button_background = ImVec4(0.1f, 0.1f, 0.1f, 0.55f);
		ImVec4 toolbar_button_background_hovered = ImVec4(0.05f, 0.05f, 0.05f, 0.65f);
		ImVec4 toolbar_button_background_active = ImVec4(0.1f, 0.1f, 0.1f, 0.55f);

		// right side alignment
		static float toolbar_line_width = toolbar_button_size.x + 8.0f; // used as first frame estimate
		const float  collapse_button_offset = (toolbar_button_size.x - toolbar_button_open_size.x) * 0.5f;
		const float  offs = m_toolbar_state ? collapse_button_offset : 0.0f;
		//ImGui::SameLine(ImGui::GetWindowWidth() - (toolbar_line_width + 8.0f - offs));

		const auto upper_right = ImVec2(
			this->rtt_get_position().x + this->rtt_get_size().x - (toolbar_line_width + 8.0f - offs),
			this->rtt_get_position().y);

		ImGui::SetCursorScreenPos(upper_right);

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

		{
			const auto phys = components::physx_impl::get();
			const bool is_prefab_simulation_active = (phys->m_phys_sim_run && phys->m_phys_active_actor_count);

			if (components::effects::effect_is_playing() || components::effects::effect_is_paused() || is_prefab_simulation_active)
			{
				const auto cursor_pos = ImGui::GetCursorPos();
				ImGui::SetCursorPosX(cursor_pos.x - CAM_DEBUG_TEXT_Y_OFFS);

				ImGui::BeginGroup();
				{
					const float offset = 16.0f;
					float y_offset = offset;

					ImGui::SetCursorPosY(cursor_pos.y + y_offset); y_offset += offset;
					ImGui::Text("FX Drawsurf Count: %d", components::renderer::effect_drawsurf_count_);


					if (phys->m_effect_is_using_physics || is_prefab_simulation_active)
					{
						ImGui::SetCursorPosY(cursor_pos.y + y_offset); y_offset += offset;
						ImGui::Text("PhysX %d ms/frame", phys->m_phys_msec_step);

						ImGui::SetCursorPosY(cursor_pos.y + y_offset); y_offset += offset;
						ImGui::Text("PhysX bodies %d", is_prefab_simulation_active ? phys->m_phys_active_actor_count : phys->m_fx_active_actor_count);

						ImGui::SetCursorPosY(cursor_pos.y + y_offset); y_offset += offset;
						ImGui::Text("PhysX brushes %d", phys->m_static_brush_count);

						ImGui::SetCursorPosY(cursor_pos.y + y_offset); y_offset += offset;
						ImGui::Text("PhysX terrain %d", phys->m_static_terrain_count);
					}

					ImGui::EndGroup();
				}

				ImGui::SetCursorPos(cursor_pos);
			}
		}


		if (hide_cam_toolbar)
		{
			ImGui::PopStyleColor(_stylecolors);
			ImGui::PopStyleVar(_stylevars);
			return;
		}

		const auto tb = GET_GUI(ggui::toolbar_dialog);

		// group all so we can get the actual toolbar width for the next frame
		ImGui::BeginGroup();
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + collapse_button_offset);

			static bool hov_open_toolbar;
			if (tb->image_togglebutton("arrow_down"
				, hov_open_toolbar
				, m_toolbar_state
				, "Collapse/Expand camera toolbar"
				, &toolbar_button_background
				, &toolbar_button_background_hovered
				, &toolbar_button_background_active
				, &toolbar_button_open_size))
			{
				m_toolbar_state = m_toolbar_state ? false : true;
			} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

			if (m_toolbar_state)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
				{
					static bool hov_guizmo_enable;
					if (tb->image_togglebutton("guizmo_enable"
						, hov_guizmo_enable
						, dvars::guizmo_enable->current.enabled
						, "Enable guizmo"
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						dvars::set_bool(dvars::guizmo_enable, !dvars::guizmo_enable->current.enabled);
					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

					if (dvars::guizmo_enable->current.enabled)
					{
						static bool hov_guizmo_local_world;

						if (tb->image_togglebutton("guizmo_world_local"
							, hov_guizmo_local_world
							, ggui::camera_guizmo::g_guizmo_local
							, "Guizmo: Toggle between local and world mode\nConsiders angle of last selection when used to manipulate multiple objects."
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							ggui::camera_guizmo::g_guizmo_local = !ggui::camera_guizmo::g_guizmo_local;
						} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());
					}

					//if (dvars::guizmo_enable->current.enabled)
					//{
					//	static bool hov_guizmo_grid_snapping;
					//	if (tb->image_togglebutton("guizmo_grid_snapping"
					//		, hov_guizmo_grid_snapping
					//		, dvars::guizmo_snapping->current.enabled
					//		, "Guizmo: Enable grid-snapping"
					//		, &toolbar_button_background
					//		, &toolbar_button_background_hovered
					//		, &toolbar_button_background_active
					//		, &toolbar_button_size))
					//	{
					//		dvars::set_bool(dvars::guizmo_snapping, !dvars::guizmo_snapping->current.enabled);
					//	} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());


					//	/*static bool hov_guizmo_brush_mode;
					//	if (tb->image_togglebutton("guizmo_brush_mode"
					//		, hov_guizmo_brush_mode
					//		, dvars::guizmo_brush_mode->current.enabled
					//		, "Guizmo: Enable brush mode"
					//		, &toolbar_button_background
					//		, &toolbar_button_background_hovered
					//		, &toolbar_button_background_active
					//		, &toolbar_button_size))
					//	{
					//		dvars::set_bool(dvars::guizmo_brush_mode, !dvars::guizmo_brush_mode->current.enabled);
					//	} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());*/
					//}

					ImGui::PopStyleVar();
				}

				//SPACING(0.0f, 0.0f);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
				{
					static bool hov_cubicclip;
					if (tb->image_togglebutton("cubic_clip"
						, hov_cubicclip
						, prefs->m_bCubicClipping
						, std::string("Cubic Clipping " + ggui::hotkey_dialog::get_hotkey_for_command("ToggleCubicClip")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						mainframe_thiscall(LRESULT, 0x428F90); // CMainFrame::OnViewCubicclipping
					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

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

					static bool hov_gameview;
					if (tb->image_togglebutton("gameview"
						, hov_gameview
						, dvars::radiant_gameview->current.enabled
						, std::string("Gameview " + ggui::hotkey_dialog::get_hotkey_for_command("xo_gameview")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						components::gameview::p_this->set_state(!dvars::radiant_gameview->current.enabled);

					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

					ImGui::PopStyleVar();
				}

				if (components::d3dbsp::Com_IsBspLoaded())
				{
					SPACING(0.0f, 0.0f);

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
					{
						if(dvars::gui_camera_toolbar_merge_bsp_buttons && dvars::gui_camera_toolbar_merge_bsp_buttons->current.enabled)
						{
							static bool hov_toggle_bsp_radiant;
							const bool show_bsp_icon = dvars::r_draw_bsp->current.enabled;

							if (tb->image_togglebutton(show_bsp_icon ? "toggle_bsp" : "toggle_radiant_world"
								, hov_toggle_bsp_radiant
								, dvars::r_draw_bsp->current.enabled
								, std::string("Toggle between visibility of the loaded d3dbsp and radiant " + ggui::hotkey_dialog::get_hotkey_for_command("toggle_bsp_radiant") + "\nButton can be split into two via 'Preferences > Camera > Merge BSP/Radiant buttons'").c_str()
								, &toolbar_button_background
								, &toolbar_button_background_hovered
								, &toolbar_button_background_active
								, &toolbar_button_size))
							{
								components::d3dbsp::toggle_radiant_bsp_view(!show_bsp_icon);

							} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());
						}
						else
						{
							static bool hov_bsp_rendering;
							if (tb->image_togglebutton("toggle_bsp"
								, hov_bsp_rendering
								, dvars::r_draw_bsp->current.enabled
								, std::string("Toggle visibility of loaded d3dbsp " + ggui::hotkey_dialog::get_hotkey_for_command("toggle_bsp")).c_str()
								, &toolbar_button_background
								, &toolbar_button_background_hovered
								, &toolbar_button_background_active
								, &toolbar_button_size))
							{
								dvars::set_bool(dvars::r_draw_bsp, !dvars::r_draw_bsp->current.enabled);

							} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());


							static bool hov_toggle_world;

							const auto gameview = components::gameview::get();
							const bool tstate = gameview->get_all_geo_state() || gameview->get_all_ents_state() || gameview->get_all_triggers_state() || gameview->get_all_others_state();

							if (tb->image_togglebutton("toggle_radiant_world"
								, hov_toggle_world
								, !tstate
								, std::string("Toggle radiant rendering " + ggui::hotkey_dialog::get_hotkey_for_command("toggle_filter_all")).c_str()
								, &toolbar_button_background
								, &toolbar_button_background_hovered
								, &toolbar_button_background_active
								, &toolbar_button_size))
							{
								gameview->toggle_all_filters(!tstate);

							} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());
						}

						ImGui::PushID("settings3");
						static bool hov_bsp_settings;
						if (tb->image_togglebutton("fakesun_settings"
							, hov_bsp_settings
							, hov_bsp_settings
							, "Open BSP settings menu"
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							const auto cs = GET_GUI(ggui::camera_settings_dialog);
							cs->handle_toggle_request(camera_settings_dialog::tab_state_bsp);
							cs->focus_tab(camera_settings_dialog::tab_state_bsp);

						} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());
						ImGui::PopID();

						ImGui::PopStyleVar();
					}
				}
				
				SPACING(0.0f, 0.0f);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f));
				{
					static bool hov_fakesunpreview;
					if (tb->image_togglebutton("sunpreview"
						, hov_fakesunpreview
						, dvars::r_fakesun_preview->current.enabled
						, std::string("Fake sun preview " + ggui::hotkey_dialog::get_hotkey_for_command("fakesun_toggle") + "\nSupports specular and bump mapping.\n(Does not affect d3dbsp)").c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						components::command::execute("fakesun_toggle");

					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());


					//if (dvars::r_fakesun_preview->current.enabled)
					//{
						static bool hov_fakesun_fog;
						if (tb->image_togglebutton("fakesun_fog"
							, hov_fakesun_fog
							, dvars::r_fakesun_fog_enabled->current.enabled
							, std::string("Toggle Fog " + ggui::hotkey_dialog::get_hotkey_for_command("fakesun_fog_toggle") + "\n(needs 'Fake sun preview' to work on radiant brushes / models)\n(Does affect d3dbsp)").c_str()
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							components::command::execute("fakesun_fog_toggle");

						} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());
					//}


					static bool hov_filmtweaks_settings;
					const auto r_filmtweakenable = game::Dvar_FindVar("r_filmtweakenable");

					if (tb->image_togglebutton("filmtweaks"
						, hov_filmtweaks_settings
						, r_filmtweakenable->current.enabled
						, std::string("Toggle filmtweaks " + ggui::hotkey_dialog::get_hotkey_for_command("filmtweak_toggle")).c_str()
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						components::command::execute("filmtweak_toggle");

					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());


					static bool hov_fakesun_settings;
					if (tb->image_togglebutton("fakesun_settings"
						, hov_fakesun_settings
						, hov_fakesun_settings
						, "Open Fakesun / PostFX settings menu"
						, &toolbar_button_background
						, &toolbar_button_background_hovered
						, &toolbar_button_background_active
						, &toolbar_button_size))
					{
						const auto cs = GET_GUI(ggui::camera_settings_dialog);
						cs->handle_toggle_request(camera_settings_dialog::tab_state_fakesun);
						cs->focus_tab(camera_settings_dialog::tab_state_fakesun);
						
					} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

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
							//if (can_fx_play)
							{
								ImGui::BeginDisabled(!can_fx_play || components::effects::effect_is_repeating());
								{
									static bool hov_fx_play;
									if (tb->image_togglebutton("fx_play"
										, hov_fx_play
										, can_fx_play && !components::effects::effect_is_repeating()
										, std::string("Play Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_play")).c_str()
										, &toolbar_button_background
										, &toolbar_button_background_hovered
										, &toolbar_button_background_active
										, &toolbar_button_size))
									{
										components::effects::play();

									} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

									ImGui::EndDisabled();
								}

								ImGui::BeginDisabled(!can_fx_play);
								{
									static bool hov_fx_repeat;
									if (tb->image_togglebutton("fx_repeat"
										, hov_fx_repeat
										, components::effects::effect_is_repeating()
										, std::string("Re-trigger Effect every X seconds for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_repeat")).c_str()
										, &toolbar_button_background
										, &toolbar_button_background_hovered
										, &toolbar_button_background_active
										, &toolbar_button_size))
									{
										components::effects::repeat();

									} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());


									static bool hov_fx_pause;
									if (tb->image_togglebutton("fx_pause"
										, hov_fx_pause
										, can_fx_play
										, std::string("Pause Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_pause")).c_str()
										, &toolbar_button_background
										, &toolbar_button_background_hovered
										, &toolbar_button_background_active
										, &toolbar_button_size))
									{
										components::effects::pause();

									} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());


									static bool hov_fx_stop;
									if (tb->image_togglebutton("fx_stop"
										, hov_fx_stop
										, can_fx_play
										, std::string("Stop Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_stop")).c_str()
										, &toolbar_button_background
										, &toolbar_button_background_hovered
										, &toolbar_button_background_active
										, &toolbar_button_size))
									{
										components::effects::stop();

									} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

									static bool hov_fx_edit;
									if (tb->image_togglebutton("fx_edit"
										, hov_fx_edit
										, hov_fx_edit
										, "Edit Effect for last selected fx_origin"
										, &toolbar_button_background
										, &toolbar_button_background_hovered
										, &toolbar_button_background_active
										, &toolbar_button_size))
									{
										components::effects::edit();

									} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());

									ImGui::EndDisabled();
								}
							}

							ImGui::EndGroup();

							if (!can_fx_play)
							{
								TT("Select an fx_origin to enable controls.");
							}

							ImGui::PushID("settings2");
							static bool hov_fx_settings;
							if (tb->image_togglebutton("fakesun_settings"
								, hov_fx_settings
								, hov_fx_settings
								, "Open Effects / PhysX settings menu"
								, &toolbar_button_background
								, &toolbar_button_background_hovered
								, &toolbar_button_background_active
								, &toolbar_button_size))
							{
								const auto cs = GET_GUI(ggui::camera_settings_dialog);
								cs->handle_toggle_request(camera_settings_dialog::tab_state_effects);
								cs->focus_tab(camera_settings_dialog::tab_state_effects);

							} ggui::rtt_handle_windowfocus_overlaywidget(this->rtt_get_hovered_state());
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

	void camera_dialog::convert_selection_to_prefab_imgui_menu()
	{
		// logic :: ggui::file_dialog_frame
		if (ImGui::MenuItem("Convert Selection to Prefab"))
		{
			const auto egui = GET_GUI(ggui::entity_dialog);
			const std::string path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath") + "\\prefabs"s;

			const auto file = GET_GUI(ggui::file_dialog);
			file->set_default_path(path_str);
			file->set_file_handler(ggui::FILE_DIALOG_HANDLER::MISC_PREFAB_CREATE);
			file->set_file_op_type(file_dialog::FileDialogType::SaveFile);
			file->set_file_ext(".map");
			file->set_blocking();
			file->open();
		}
	}

	void rotate_stamped_prefab(const float* angles)
	{
		float rotate_axis[4][3];

		if (angles[2] != 0.0f)
		{
			const float angle = -angles[2];
			if (angle != 0.0f)
			{
				game::Select_GetMid(&rotate_axis[0][0]);
				game::Select_RotateAxis(0, angle, &rotate_axis[0][0]);
				game::Select_ApplyMatrix_SelectedBrushes(false, &rotate_axis[0][0], angle, false);
			}
		}

		if (angles[0] != 0.0f)
		{
			const float angle = -angles[0];
			if (angle != 0.0f)
			{
				game::Select_GetMid(&rotate_axis[0][0]);
				game::Select_RotateAxis(1, angle, &rotate_axis[0][0]);
				game::Select_ApplyMatrix_SelectedBrushes(false, &rotate_axis[0][0], angle, false);
			}
		}

		if (angles[1] != 0.0f)
		{
			const float angle = -angles[1];
			if (angle != 0.0f)
			{
				game::Select_GetMid(&rotate_axis[0][0]);
				game::Select_RotateAxis(2, angle, &rotate_axis[0][0]);
				game::Select_ApplyMatrix_SelectedBrushes(false, &rotate_axis[0][0], angle, false);
			}
		}
	}

#if 0 // T5
	void GetRotatedBounds(const float(*baseBounds)[3], const float* origin, const float(*axis)[3], float(*rotatedBounds)[3])
	{
		int axisIndex; // [esp+10h] [ebp-8h]
		int offset; // [esp+14h] [ebp-4h]
		int offseta; // [esp+14h] [ebp-4h]
		int offsetb; // [esp+14h] [ebp-4h]

		for (axisIndex = 0;
			axisIndex < 3;
			++axisIndex)
		{
			(*rotatedBounds)[axisIndex] = origin[axisIndex];
			(*rotatedBounds)[axisIndex + 3] = origin[axisIndex];
			offset = (*axis)[axisIndex] >= 0.0 ? 0 : 0xC;
			(*rotatedBounds)[axisIndex] = (float)(*(float*)((char*)*baseBounds + offset) * (float)(*axis)[axisIndex]) + (*rotatedBounds)[axisIndex];
			(*rotatedBounds)[axisIndex + 3] = (float)(*(float*)((char*)&(*baseBounds)[3] - offset) * (float)(*axis)[axisIndex]) + (*rotatedBounds)[axisIndex + 3];
			offseta = (*axis)[axisIndex + 3] >= 0.0 ? 0 : 0xC;
			(*rotatedBounds)[axisIndex] = (float)(*(float*)((char*)&(*baseBounds)[1] + offseta) * (float)(*axis)[axisIndex + 3]) + (*rotatedBounds)[axisIndex];
			(*rotatedBounds)[axisIndex + 3] = (float)(*(float*)((char*)&(*baseBounds)[4] - offseta) * (float)(*axis)[axisIndex + 3]) + (*rotatedBounds)[axisIndex + 3];
			offsetb = (*axis)[axisIndex + 6] >= 0.0 ? 0 : 0xC;
			(*rotatedBounds)[axisIndex] = (float)(*(float*)((char*)&(*baseBounds)[2] + offsetb) * (float)(*axis)[axisIndex + 6]) + (*rotatedBounds)[axisIndex];
			(*rotatedBounds)[axisIndex + 3] = (float)(*(float*)((char*)&(*baseBounds)[5] - offsetb) * (float)(*axis)[axisIndex + 6]) + (*rotatedBounds)[axisIndex + 3];
		}
	}
#endif

	bool camera_dialog::stamp_prefab_imgui_imgui_menu(game::selbrush_def_t* sb)
	{
		if (ImGui::MenuItem("Stamp Prefab"))
		{
			const auto egui = GET_GUI(ggui::entity_dialog);

			// disable snappping for all comming operations
			// Prefab_Leave does brush snapping we dont want
			const auto saved_snapping = game::g_PrefsDlg()->m_bNoClamp;
			game::g_PrefsDlg()->m_bNoClamp = true;


			// deselect everything
			// => only select prefab 
			if (sb)
			{
				game::Select_Deselect(false);
				game::Brush_Select(sb, true, false, false);
			}

			// calculate mid-point of prefab (center might not match the original non-rotated center, thus final position might be off)
			game::vec3_t prefab_midpoint = {};
			game::Select_GetMid(prefab_midpoint);


			// save prefab ptr
			game::selbrush_def_t* og_prefab = game::g_selected_brushes();

			if (!og_prefab || !og_prefab->def)
			{
				game::printf_to_console("[ERR] something went wrong while stamping prefab");
				return false;
			}

			// get prefab angles
			game::vec3_t prefab_angles = {};
			egui->get_vec3_for_key_from_entity(og_prefab->def->owner, prefab_angles, "angles");

			// get prefab origin
			//game::vec3_t prefab_origin = {};
			//egui->get_vec3_for_key_from_entity(og_prefab->def->owner, prefab_origin, "origin");


#if 0		// perfect rotations for Y-Axis rotated prefabs, fails for everything else
			// get bounds rotated

			float org_bounds[2][3];
			org_bounds[0][0] = 131072.0f;
			org_bounds[0][1] = 131072.0f;
			org_bounds[0][2] = 131072.0f;
			org_bounds[1][0] = -131072.0f;
			org_bounds[1][1] = -131072.0f;
			org_bounds[1][2] = -131072.0f;

			FOR_ALL_BRUSHES(prefab, sb->owner->prefab->active_brushlist, sb->owner->prefab->active_brushlist_next)
			{
				utils::vector::clamp_vec3(prefab->def->mins, prefab->def->maxs, org_bounds[0], org_bounds[1]);
			}

			game::vec3_t rotated_mid;
			utils::vector::add(org_bounds[0], org_bounds[1], rotated_mid);
			utils::vector::scale(rotated_mid, 0.5f, rotated_mid);

			float axis[3][3];
			game::AnglesToAxis(prefab_angles, &axis[0][0]);

			float rot_bounds[2][3];
			GetRotatedBounds(org_bounds, prefab_origin, axis, rot_bounds);

			game::vec3_t asd;
			utils::vector::add(rot_bounds[0], rot_bounds[1], asd);
			utils::vector::scale(asd, 0.5f, asd);
#endif


			// enter prefab and select everything -> copy
			// !! entering / leaving a prefab clears the entire undo stack !!
			game::Prefab_Enter();
			game::Select_Deselect(false);
			game::Select_Invert();

			game::Selection_Copy();

			// leave prefab -> undo, delete prefab
			game::Prefab_Leave();


			// deselect all, then select the original prefab
			game::Select_Deselect(false);
			game::Brush_Select(og_prefab, false, false, false);


			// create undo and delete the original prefab
			game::Undo_ClearRedo();
			game::Undo_GeneralStart("delete prefab");
			game::Undo_AddBrushList(game::currSelectedBrushes);

			game::Select_Delete();

			game::Undo_EndBrushList(game::currSelectedBrushes);
			game::Undo_End();

			// paste the copied brushes
			game::Selection_Paste();
			//game::clipboard_clear(cmainframe::activewnd->m_pXYWnd); // buggy


			// calculate mid-point of stamped brushes
			game::vec3_t stamped_midpoint = {};
			game::Select_GetMid(stamped_midpoint);


			game::vec3_t origin_offset = {};
			utils::vector::subtract(prefab_midpoint, stamped_midpoint, origin_offset);
			//utils::vector::subtract(asd, stamped_midpoint, origin_offset);


			// offset all stamped brushes
			FOR_ALL_SELECTED_BRUSHES(stamped)
			{
				game::Brush_Move(origin_offset, stamped->def, 0);
			}

			if (prefab_angles[0] == 0.0f && prefab_angles[1] == 0.0f && prefab_angles[2] == 0.0f)
			{
				game::g_PrefsDlg()->m_bNoClamp = saved_snapping;
			}
			else
			{
				// delay rotation (waw radiant does that by sending a window message to execute the rotation command)
				components::exec::on_gui_once([prefab_angles, saved_snapping]
				{
					rotate_stamped_prefab(prefab_angles);
					game::g_PrefsDlg()->m_bNoClamp = saved_snapping;

				}, 1ms);
			}

			return true;
		}

		return false;
	}

	// right click context menu
	void camera_dialog::context_menu()
	{
		ggui::context_menu_style_begin();

		static game::trace_t cam_trace[21] = {};
		static bool cam_context_menu_open = false;
		static bool cam_context_menu_pending_open = false;

		if (cmainframe::activewnd->m_pCamWnd->cam_was_not_dragged)
		{
			// alt detection is kinda bugged via imgui
			const bool is_alt_key_pressed = GetKeyState(VK_MENU) < 0;

			// extend / extrude selected brush to traced face (ala bo3's reflection probe bounds)
			if (is_alt_key_pressed && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && game::is_single_brush_selected())
			{
				cam_context_menu_open = false;
				cam_context_menu_pending_open = false;

				float dir[3];

				ccamwnd::calculate_ray_direction(
					this->rtt_get_cursor_pos_cpoint().x, 
					static_cast<int>(this->rtt_get_size().y) - this->rtt_get_cursor_pos_cpoint().y,
					dir);

				game::Test_Ray(cmainframe::activewnd->m_pCamWnd->camera.origin, dir, 0, cam_trace, 20);

				if (cam_trace[0].brush && cam_trace[0].brush->def)
				{
					game::face_t_new* hit_face = nullptr;

					for(auto i = 0; i < cam_trace[0].brush->def->facecount; i++)
					{
						if (utils::vector::compare(cam_trace[0].face_normal, cam_trace[0].brush->def->brush_faces[i].plane.normal))
						{
							hit_face = &cam_trace[0].brush->def->brush_faces[i];
							break;
						}
					}

					if (hit_face)
					{
						std::map<float, int, std::greater<>> face_dot_products;

						if(const auto	selbrush = game::g_selected_brushes(); 
										selbrush && selbrush->def)
						{
							for (auto i = 0; i < selbrush->def->facecount; i++)
							{
								// invert normal
								game::vec3_t inv_normal;
								utils::vector::scale(selbrush->def->brush_faces[i].plane.normal, -1.0f, inv_normal);

								float plane_plane_dot = utils::vector::dot(hit_face->plane.normal, inv_normal);
								face_dot_products.emplace(std::make_pair(plane_plane_dot, i));
							}

							const int best_index = face_dot_products.begin()->second;

							// backup points in case the brush gets degenerated
							game::vec3_t backup_points[3] = {};
							utils::vector::copy(selbrush->def->brush_faces[best_index].planepts0, backup_points[0]);
							utils::vector::copy(selbrush->def->brush_faces[best_index].planepts1, backup_points[1]);
							utils::vector::copy(selbrush->def->brush_faces[best_index].planepts2, backup_points[2]);

							// create undo
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("extrude to face");
							game::Undo_AddBrush(selbrush->def);

							// copy plane points
							utils::vector::copy(hit_face->planepts2, selbrush->def->brush_faces[best_index].planepts0);
							utils::vector::copy(hit_face->planepts1, selbrush->def->brush_faces[best_index].planepts1);
							utils::vector::copy(hit_face->planepts0, selbrush->def->brush_faces[best_index].planepts2);

							game::Brush_MakeFacePlanes(selbrush->def);
							game::BrushPt_t points[1024];

							// test if extending results in a valid brush or if it gets degenerated
							if (game::CM_ForEachBrushPlaneIntersection(selbrush->def, points) < 4)
							{
								game::printf_to_console("invalid face");

								// restore plane
								utils::vector::copy(backup_points[0], selbrush->def->brush_faces[best_index].planepts0);
								utils::vector::copy(backup_points[1], selbrush->def->brush_faces[best_index].planepts1);
								utils::vector::copy(backup_points[2], selbrush->def->brush_faces[best_index].planepts2);
							}
							else // valid brush -> build windings
							{
								game::Brush_BuildWindings(selbrush->def, false);
								++selbrush->def->version;
							}

							game::Undo_End();


							if (   selbrush->def->mins[0] == selbrush->def->maxs[0]
								|| selbrush->def->mins[1] == selbrush->def->maxs[1]
								|| selbrush->def->mins[2] == selbrush->def->maxs[2])
							{
								game::printf_to_console("^1 Can not extrude to own face. Aborting ...\n");

								// CMainFrame::OnEditUndo
								mainframe_thiscall(void, 0x428730);
							}
						}
					}
				}
			}
			else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || cam_context_menu_open || cam_context_menu_pending_open)
			{
				if (!cam_context_menu_open)
				{
					cam_context_menu_pending_open = true;

					float dir[3];
					ccamwnd::calculate_ray_direction(
						this->rtt_get_cursor_pos_cpoint().x,
						static_cast<int>(this->rtt_get_size().y) - this->rtt_get_cursor_pos_cpoint().y,
						dir);

					game::Test_Ray(cmainframe::activewnd->m_pCamWnd->camera.origin, dir, 0, cam_trace, 20);

					if (cam_trace[0].brush)
					{
						// sort traces by drawsurf order
						const auto trace_array_end = (DWORD)&cam_trace[21];
						const auto compare_func = reinterpret_cast<void*>(0x404BA0);

						utils::hook::call<void(__cdecl)(game::trace_t*, DWORD _trace_array_end, int _size_trace_array, void*)>(0x408CE0)
							(cam_trace, trace_array_end, 20, compare_func);
					}
				}

				// #
				// #

				//const auto export_selection_as_prefab_menu = [&]() -> void
				//{
				//	if (ImGui::MenuItem("Export Selection as Prefab"))
				//	{
				//		// logic :: ggui::file_dialog_frame
				//		if (dvars::gui_use_new_filedialog->current.enabled)
				//		{
				//			const auto egui = GET_GUI(ggui::entity_dialog);
				//			const std::string path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath") + "\\prefabs"s;

				//			const auto file = GET_GUI(ggui::file_dialog);
				//			file->set_default_path(path_str);
				//			file->set_file_handler(ggui::FILE_DIALOG_HANDLER::MAP_EXPORT);
				//			file->set_file_op_type(file_dialog::FileDialogType::SaveFile);
				//			file->set_file_ext(".map");
				//			file->open();
				//		}
				//		else
				//		{
				//			mainframe_thiscall(void, 0x4293A0); //cmainframe::OnFileExportmap
				//		}
				//	}
				//};

				// this is stupid but imgui creates a little empty square otherwise
				if (cam_trace[0].brush || game::g_prefab_stack_level /*|| game::is_any_brush_selected()*/)
				{
					if (!ImGui::IsKeyPressed(ImGuiKey_Escape) && ImGui::BeginPopupContextItem("context_menu##camera"))
					{
						cam_context_menu_open = true;
						cam_context_menu_pending_open = false;

						bool entering_or_leaving_prefab = false;

						if (cam_trace[0].brush)
						{
							bool any_selected = false;

							// list all brushes / objects by their material name or similar
							for (auto t = 0; t < 20 && cam_trace[t].brush; t++)
							{
								if (cam_trace[t].selected)
								{
									any_selected = true;
								}

								ImGui::PushID(t);
								{
									// get material name for brushes / patches
									auto vis_name = fx_system::Material_GetName(cam_trace[t].face->visArray->handle);

									// get prefab / model or classname for entities
									if (const auto  tb = cam_trace[t].brush;
													tb->def && tb->def->owner)
									{
										if (auto val = GET_GUI(ggui::entity_dialog)->get_value_for_key_from_epairs(tb->def->owner->epairs, "classname");
												 val && (val == "misc_prefab"s || val == "misc_model"s))
										{
											if (auto prefab_str = GET_GUI(ggui::entity_dialog)->get_value_for_key_from_epairs(tb->def->owner->epairs, "model");
													 prefab_str)
											{
												vis_name = prefab_str;
											}
										}
										else if (tb->def->owner->eclass && tb->def->owner->eclass->name != "worldspawn"s)
										{
											vis_name = tb->def->owner->eclass->name;
										}
									}

									bool b_selection = false;

									if (ImGui::IsKeyDown(ImGuiKey_ModCtrl))
									{
										b_selection = ImGui::MenuItemFlags(vis_name, cam_trace[t].selected, true, ImGuiSelectableFlags_DontClosePopups);
									}
									else
									{
										b_selection = ImGui::MenuItem(vis_name, 0, cam_trace[t].selected);
									}

									if (b_selection)
									{
										if (cam_trace[t].selected)
										{
											game::Brush_Deselect((game::brush_t*)cam_trace[t].brush);
											cam_trace[t].selected = false;
										}
										else
										{
											game::Brush_Select(cam_trace[t].brush, false, false, false);
											cam_trace[t].selected = true;
										}
									}

									ImGui::PopID();
								}
							}

							SEPERATORV(0.0f);

							if (ImGui::MenuItem("Select All"))
							{
								for (auto t = 0; t < 20 && cam_trace[t].brush; t++)
								{
									game::Brush_Select(cam_trace[t].brush, false, false, false);
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

							// prefab
							{
								bool prefab_sep = false;

								if (!game::multiple_edit_entities)
								{
									if (const auto	tb = cam_trace[0].brush;
													tb->def && tb->def->owner)
									{
										// if prefab is hovered
										if (const auto	val = GET_GUI(ggui::entity_dialog)->get_value_for_key_from_epairs(tb->def->owner->epairs, "classname");
														val && val == "misc_prefab"s)
										{
											SEPERATORV(0.0f);
											prefab_sep = true;

											if (ImGui::MenuItem("Enter Prefab"))
											{
												if (!cam_trace[0].selected)
												{
													game::Brush_Select(cam_trace[0].brush, false, false, false);
												}

												game::Prefab_Enter();
												entering_or_leaving_prefab = true;
											}

											if (game::g_prefab_stack_level)
											{
												if (ImGui::MenuItem("Leave Prefab"))
												{
													game::Prefab_Leave();
													entering_or_leaving_prefab = true;
												}
											}

											if (stamp_prefab_imgui_imgui_menu(cam_trace[0].brush))
											{
												entering_or_leaving_prefab = true;
											}
										}
										else if (game::g_prefab_stack_level)
										{
											SEPERATORV(0.0f);
											prefab_sep = true;

											if (ImGui::MenuItem("Leave Prefab"))
											{
												game::Prefab_Leave();
												entering_or_leaving_prefab = true;
											}
										}
									}
								}

								// within a prefab but no other prefab hovered
								else if (game::g_prefab_stack_level)
								{
									if (!prefab_sep)
									{
										SEPERATORV(0.0f);
										prefab_sep = true;
									}

									/*if (game::is_any_brush_selected())
									{
										export_selection_as_prefab_menu();
									}*/

									if (ImGui::MenuItem("Leave Prefab"))
									{
										game::Prefab_Leave();
										entering_or_leaving_prefab = true;
									}
								}

								//if (game::is_any_brush_selected())
								//{
								//	if (!prefab_sep) 
								//	{
								//		SEPERATORV(0.0f);
								//		prefab_sep = true;
								//	}
								//	
								//	//convert_selection_to_prefab_imgui_menu();
								//	//export_selection_as_prefab_menu();
								//}
							}
							
							// subdivision and texture operations

							if (any_selected)
							{
								//if (!game::multiple_edit_entities)
								{
									if (const auto	 selbrush = game::g_selected_brushes();
													(selbrush && selbrush->def && selbrush->patch))
									{
										if (selbrush->patch->def->type != game::PATCH_TERRAIN)
										{
											SEPERATORV(0.0f);

											if (ImGui::Selectable("Subdivision ++", false, ImGuiSelectableFlags_DontClosePopups))
											{
												// CMainFrame::OnOverBrightShiftUp
												cdeclcall(void, 0x428EB0);
											}

											if (ImGui::Selectable("Subdivision --", false, ImGuiSelectableFlags_DontClosePopups))
											{
												// CMainFrame::OnOverBrightShiftDown
												cdeclcall(void, 0x428EE0);
											}
										}

										SEPERATORV(0.0f);

										if (ImGui::MenuItem("Texture - Fit"))
										{
											//Brush_FitTexture
											utils::hook::call<void(__cdecl)(float _x, float _y, int _unk)>(0x4939E0)(1.0f, 1.0f, 0);
										}

										if (ImGui::MenuItem("Texture - Lmap"))
										{
											//Patch_Lightmap_Texturing
											utils::hook::call<void(__cdecl)()>(0x448110)();
										}
									}
								}
							}
						} // no brush hit
						else if (game::g_prefab_stack_level) // within prefab
						{
							if (ImGui::MenuItem("Leave Prefab"))
							{
								game::Prefab_Leave();
								entering_or_leaving_prefab = true;
							}
						}

						// NEVER try to access brush data after entering or leaving a prefab (within the same frame)
						if (!entering_or_leaving_prefab)
						{
							if (game::is_any_brush_selected())
							{
								ggui::context::xyzcam_general_selection();

								if (cam_trace[0].brush)
								{
									ggui::context::grouping_menu(cam_trace[0].brush);
								}
								else
								{
									ggui::context::grouping_menu(game::g_selected_brushes());
								}


								SEPERATORV(0.0f);
								convert_selection_to_prefab_imgui_menu();
							}
							else if (cam_trace[0].brush)
							{
								ggui::context::grouping_menu(cam_trace[0].brush);
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

		ggui::context_menu_style_end();
	}


	// drag-drop target
	void camera_dialog::drag_drop_target(bool& accepted_dragdrop)
	{
		// model selection drop target
		if (ImGui::BeginDragDropTarget())
		{
			if (ImGui::AcceptDragDropPayload("MODEL_SELECTOR_ITEM"))
			{
				const auto entity_gui = GET_GUI(ggui::entity_dialog);
				const auto m_selector = GET_GUI(ggui::modelselector_dialog);
				// reset manual left mouse capture
				ggui::dragdrop_reset_leftmouse_capture();

				ggui::entity_dialog::addprop_helper_s no_undo = {};
				bool trace_hit_void = false;

				if (m_selector->m_overwrite_selection)
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

					entity_gui->add_prop("model", m_selector->m_preview_model_name.c_str(), &no_undo);
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
					game::CreateEntityFromName("misc_model");
					g_block_radiant_modeldialog = false;

					entity_gui->add_prop("model", m_selector->m_preview_model_name.c_str(), &no_undo);
					// ^ model dialog -> OpenDialog // CEntityWnd_EntityWndProc

					//const auto camerawnd = ggui::get_rtt_camerawnd();

					float dir[3];
					ccamwnd::calculate_ray_direction(
						this->rtt_get_cursor_pos_cpoint().x, 
						static_cast<int>(this->rtt_get_size().y) - this->rtt_get_cursor_pos_cpoint().y,
						dir);

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
						trace_hit_void = true;

						float dist = 100.0f;
						if (const auto	model = m_selector->m_preview_model_ptr;
										model)
						{
							dist += model->radius;
						}

						utils::vector::ma(cmainframe::activewnd->m_pCamWnd->camera.origin, dist, dir, origin);
					}

					char origin_str_buf[64] = {};
					if (sprintf_s(origin_str_buf, "%.3f %.3f %.3f", origin[0], origin[1], origin[2]))
					{
						entity_gui->add_prop("origin", origin_str_buf, &no_undo);
					}

					game::Undo_End();
				}

				// only drop if trace hit something
				if(!trace_hit_void)
				{
					// CMainFrame::OnDropSelected
					cdeclcall(void, 0x425BE0);
				}

				accepted_dragdrop = true;
			}

			// -----------------------
			// -----------------------

			const auto spawn_entity = [this, &accepted_dragdrop](const char* entity_class, const char* kv) -> void
			{
				const auto payload = imgui::GetDragDropPayload();
				const std::string prefab_path = /*"prefabs/"s +*/ std::string(static_cast<const char*>(payload->Data), payload->DataSize);

				const auto entity_gui = GET_GUI(ggui::entity_dialog);

				// reset manual left mouse capture
				ggui::dragdrop_reset_leftmouse_capture();

				ggui::entity_dialog::addprop_helper_s no_undo = {};
				bool trace_hit_void = false;

				game::Select_Deselect(true);
				game::Undo_ClearRedo();
				game::Undo_GeneralStart("create entity");

				if ((DWORD*)game::g_selected_brushes_next() == game::currSelectedBrushes)
				{
					game::CreateEntityBrush(0, 0, cmainframe::activewnd->m_pXYWnd);
				}

				// do not open the original modeldialog for this use-case, see: create_entity_from_name_intercept()
				g_block_radiant_modeldialog = true;
				game::CreateEntityFromName(entity_class);
				g_block_radiant_modeldialog = false;

				entity_gui->add_prop(kv, prefab_path.c_str(), &no_undo);

				float dir[3];
				ccamwnd::calculate_ray_direction(
					this->rtt_get_cursor_pos_cpoint().x,
					static_cast<int>(this->rtt_get_size().y) - this->rtt_get_cursor_pos_cpoint().y,
					dir);

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
					trace_hit_void = true;

					const float dist = 100.0f;
					utils::vector::ma(cmainframe::activewnd->m_pCamWnd->camera.origin, dist, dir, origin);
				}

				char origin_str_buf[64] = {};
				if (sprintf_s(origin_str_buf, "%.3f %.3f %.3f", origin[0], origin[1], origin[2]))
				{
					entity_gui->add_prop("origin", origin_str_buf, &no_undo);
				}

				game::Undo_End();

				// only drop if trace hit something
				if (!trace_hit_void)
				{
					// CMainFrame::OnDropSelected
					cdeclcall(void, 0x425BE0);
				}

				accepted_dragdrop = true;
			};

			// #

			if (imgui::AcceptDragDropPayload("PREFAB_BROWSER_ITEM"))
			{
				spawn_entity("misc_prefab", "model");
			}

			// #

			if (imgui::AcceptDragDropPayload("EFFECT_BROWSER_ITEM"))
			{
				spawn_entity("fx_origin", "fx");
			}

			ImGui::EndDragDropTarget();
		}
	}


	// render to texture - camera window
	void camera_dialog::camera_gui()
	{
		int p_styles = 0;
		int p_colors = 0;

		const auto IO = ImGui::GetIO();

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

		ImGui::Begin("Camera Window##rtt", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		if(this->rtt_get_texture())
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
			this->rtt_set_size(ImGui::GetWindowSize() - ImVec2(0.0f, frame_height) - window_padding_both);

			if (this->rtt_is_focus_pending())
			{
				ImGui::SetNextWindowFocus();
				this->rtt_set_focus_state(false);
			}

			// hack to disable left mouse window movement
			ImGui::BeginChild("scene_child", ImVec2(camera_size.x, camera_size.y + frame_height) + window_padding_both, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				this->rtt_set_position(ImGui::GetCursorScreenPos());

				SetWindowPos(
					cmainframe::activewnd->m_pCamWnd->GetWindow(), 
					HWND_BOTTOM, 
					static_cast<int>(this->rtt_get_position().x),
					static_cast<int>(this->rtt_get_position().y),
					static_cast<int>(this->rtt_get_size().x), 
					static_cast<int>(this->rtt_get_size().y),
					SWP_NOZORDER);

				const auto pre_image_cursor = ImGui::GetCursorPos();

				ImGui::Image(this->rtt_get_texture(), camera_size);
				this->rtt_set_hovered_state(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup));
				this->m_rtt_focused = imgui::IsWindowFocused();

				static bool accepted_dragdrop = false;

				// hack to show mouse cursor when using ALT+TAB whilst moving the camera
				// needed because there is no mousebutton "up" event in that case ^
				// gridwindow is fine for whatever reason

				if (!components::physx_impl::get()->m_character_controller_enabled)
				{
					CURSORINFO ci = { sizeof(CURSORINFO) };
					if (GetCursorInfo(&ci))
					{
						if (ci.cbSize == 0)
						{
							if (cmainframe::activewnd->m_pCamWnd->m_nCambuttonstate != 14) // CTRL + SHIFT + RMB
							{
								if (cmainframe::activewnd->m_pCamWnd->m_nCambuttonstate != 10) // CTRL + RMB
								{
									if (cmainframe::activewnd->m_pCamWnd->m_nCambuttonstate != 2
										&& !cmainframe::activewnd->m_pCamWnd->cursor_visible)
									{
										int sw_cur;
										do
										{
											sw_cur = ShowCursor(1);
										} while (sw_cur < 0);

										cmainframe::activewnd->m_pCamWnd->cursor_visible = true;
									}
								}
							}
						}
					}
				}


				// --------------------

				if (!game::glob::is_loading_map)
				{
					// right click context menu
					camera_dialog::context_menu();
				}

				// drag-drop target (modelpreview)
				camera_dialog::drag_drop_target(accepted_dragdrop);

				// mesh painting
				components::mesh_painter::get()->paint_frame();


				// #


				// pop ItemSpacing
				ImGui::PopStyleVar(); p_styles--;

				ImGui::SetCursorPos(pre_image_cursor);

				const auto cursor_screen_pos = ImGui::GetCursorScreenPos();
				this->rtt_set_cursor_pos(ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y));

				// fix top left undock triangle
				ggui::redraw_undocking_triangle(wnd, this->rtt_get_hovered_state());

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

	bool camera_dialog::gui()
	{
		return false;
	}

	void camera_dialog::on_init()
	{
		if (dvars::gui_camera_toolbar_defaultopen && dvars::gui_camera_toolbar_defaultopen->current.enabled)
		{
			m_toolbar_state = true;
		}

		components::command::register_command_with_hotkey("guizmo_world_local"s, [](auto)
		{
			ggui::camera_guizmo::g_guizmo_local = !ggui::camera_guizmo::g_guizmo_local;
		});
	}

	void camera_dialog::on_open()
	{ }

	void camera_dialog::on_close()
	{ }

	REGISTER_GUI(camera_dialog);
}