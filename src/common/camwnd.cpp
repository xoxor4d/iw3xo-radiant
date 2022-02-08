#include "std_include.hpp"

bool cam_test = false;

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void mtx4x4_mul(game::GfxMatrix* mtx_out, game::GfxMatrix* a, game::GfxMatrix* b)
{
	mtx_out->m[0][0] = a->m[0][0] * b->m[0][0] + a->m[0][1] * b->m[1][0] + b->m[2][0] * a->m[0][2] + b->m[3][0] * a->m[0][3];
	mtx_out->m[0][1] = b->m[1][1] * a->m[0][1] + a->m[0][0] * b->m[0][1] + b->m[2][1] * a->m[0][2] + a->m[0][3] * b->m[3][1];
	mtx_out->m[0][2] = b->m[1][2] * a->m[0][1] + b->m[0][2] * a->m[0][0] + a->m[0][2] * b->m[2][2] + a->m[0][3] * b->m[3][2];
	mtx_out->m[0][3] = b->m[1][3] * a->m[0][1] + a->m[0][0] * b->m[0][3] + b->m[2][3] * a->m[0][2] + a->m[0][3] * b->m[3][3];
	mtx_out->m[1][0] = a->m[1][1] * b->m[1][0] + a->m[1][0] * b->m[0][0] + b->m[2][0] * a->m[1][2] + b->m[3][0] * a->m[1][3];
	mtx_out->m[1][1] = a->m[1][0] * b->m[0][1] + b->m[1][1] * a->m[1][1] + b->m[2][1] * a->m[1][2] + a->m[1][3] * b->m[3][1];
	mtx_out->m[1][2] = a->m[1][1] * b->m[1][2] + a->m[1][0] * b->m[0][2] + a->m[1][2] * b->m[2][2] + b->m[3][2] * a->m[1][3];
	mtx_out->m[1][3] = a->m[1][0] * b->m[0][3] + b->m[1][3] * a->m[1][1] + b->m[2][3] * a->m[1][2] + a->m[1][3] * b->m[3][3];
	mtx_out->m[2][0] = a->m[2][1] * b->m[1][0] + a->m[2][0] * b->m[0][0] + b->m[2][0] * a->m[2][2] + b->m[3][0] * a->m[2][3];
	mtx_out->m[2][1] = a->m[2][0] * b->m[0][1] + b->m[1][1] * a->m[2][1] + b->m[2][1] * a->m[2][2] + a->m[2][3] * b->m[3][1];
	mtx_out->m[2][2] = a->m[2][1] * b->m[1][2] + a->m[2][0] * b->m[0][2] + a->m[2][2] * b->m[2][2] + b->m[3][2] * a->m[2][3];
	mtx_out->m[2][3] = a->m[2][0] * b->m[0][3] + b->m[1][3] * a->m[2][1] + b->m[2][3] * a->m[2][2] + a->m[2][3] * b->m[3][3];
	mtx_out->m[3][0] = b->m[0][0] * a->m[3][0] + a->m[3][1] * b->m[1][0] + a->m[3][2] * b->m[2][0] + b->m[3][0] * a->m[3][3];
	mtx_out->m[3][1] = b->m[1][1] * a->m[3][1] + a->m[3][0] * b->m[0][1] + a->m[3][2] * b->m[2][1] + a->m[3][3] * b->m[3][1];
	mtx_out->m[3][2] = b->m[0][2] * a->m[3][0] + b->m[1][2] * a->m[3][1] + a->m[3][2] * b->m[2][2] + b->m[3][2] * a->m[3][3];
	mtx_out->m[3][3] = b->m[1][3] * a->m[3][1] + a->m[3][0] * b->m[0][3] + a->m[3][2] * b->m[2][3] + a->m[3][3] * b->m[3][3];
}

void get_matrices_for_guizmo(game::GfxMatrix* view, game::GfxMatrix* projection)
{
	float axis[9];
	axis[0] = cmainframe::activewnd->m_pCamWnd->camera.vpn[0];
	axis[1] = cmainframe::activewnd->m_pCamWnd->camera.vpn[1];
	axis[2] = cmainframe::activewnd->m_pCamWnd->camera.vpn[2];
	axis[3] = -cmainframe::activewnd->m_pCamWnd->camera.vright[0];
	axis[4] = -cmainframe::activewnd->m_pCamWnd->camera.vright[1];
	axis[5] = -cmainframe::activewnd->m_pCamWnd->camera.vright[2];
	axis[6] = cmainframe::activewnd->m_pCamWnd->camera.vup[0];
	axis[7] = cmainframe::activewnd->m_pCamWnd->camera.vup[1];
	axis[8] = cmainframe::activewnd->m_pCamWnd->camera.vup[2];

	//MatrixForViewer
	utils::hook::call<void(__cdecl)(float(*mtx)[4], const float* origin, const float*)>(0x4A7A70)(view->m, cmainframe::activewnd->m_pCamWnd->camera.origin, axis);

	const float tanHalfFovY = tan(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f;
	const float tanHalfFovX = tanHalfFovY * ((float)cmainframe::activewnd->m_pCamWnd->camera.width / (float)cmainframe::activewnd->m_pCamWnd->camera.height);

	// R_SetupProjection
	utils::hook::call<void(__cdecl)(game::GfxMatrix*, float halfx, float halfy, float znear)>(0x4A78E0)(projection, tanHalfFovX, tanHalfFovY, 0.0099999998f);

	// ----------

	// LH to RH conversion (not sure what I've done here .. but it works :e)

	game::GfxMatrix inv_view_mtx = {
		-1.f, 0.f, 0.f, 0.f,
		0.f, -1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	mtx4x4_mul(view, &inv_view_mtx, view);
	mtx4x4_mul(view, view, &inv_view_mtx);

	view->m[0][0] *= -1;
	view->m[0][1] *= -1;
	view->m[0][2] *= -1;
	view->m[1][0] *= -1;
	view->m[1][1] *= -1;
	view->m[1][2] *= -1;

	// ----------

	game::GfxMatrix inv_proj_mtx = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, -1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	mtx4x4_mul(projection, &inv_proj_mtx, projection);
	mtx4x4_mul(projection, projection, &inv_proj_mtx);
}

void get_selection_center_movepoints(float* center_point)
{
	const auto num_points = game::g_qeglobals->d_num_move_points;

	if(num_points)
	{
		// get bounds of selection
		game::vec3_t mins, maxs;
		utils::vector::set_vec3(mins, 131072.0f);
		utils::vector::set_vec3(maxs, -131072.0f);

		// get bounds from windings
		for (auto p = 0; p < num_points; p++)
		{
			for(auto v = 0; v < 3; v++)
			{
				game::g_qeglobals->d_move_points[p][v];

				// mins :: find the closest point on each axis
				if (mins[v] > game::g_qeglobals->d_move_points[p][v])
					mins[v] = game::g_qeglobals->d_move_points[p][v];

				// maxs :: find the furthest point on each axis
				if (maxs[v] < game::g_qeglobals->d_move_points[p][v])
					maxs[v] = game::g_qeglobals->d_move_points[p][v];
			}
		}

		utils::vector::add(mins, maxs, center_point);
		utils::vector::scale(center_point, 0.5f, center_point);
	}
}

void get_selection_center(float* center_point)
{
	// get bounds of selection
	game::vec3_t mins, maxs;
	utils::vector::set_vec3(mins,  131072.0f);
	utils::vector::set_vec3(maxs, -131072.0f);

	for (auto   sb = game::g_selected_brushes_next();
		(DWORD*)sb != game::currSelectedBrushes; // sb->next really points to &selected_brushes(currSelectedBrushes) eventually
				sb = sb->next)
	{
		if(sb->def)
		{
			utils::vector::clamp_vec3(sb->def->mins, sb->def->maxs, mins, maxs);
		}
	}

	utils::vector::add(mins, maxs, center_point);
	utils::vector::scale(center_point, 0.5f, center_point);
}

bool	_rttcam_toolbar_state = false;

// create dvars for these
//bool	guizmo_enable = true;
//bool	guizmo_enable_snapping = true;
//bool	guizmo_enable_brush_mode = true;

constexpr float CAM_DEBUG_TEXT_Y_OFFS = 180.0f;

void rtt_camera_window_toolbar()
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
	const float  offs = _rttcam_toolbar_state ? collapse_button_offset : 0.0f;
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

	if(dvars::gui_draw_fps && dvars::gui_draw_fps->current.enabled)
	{
		const auto cursor_pos = ImGui::GetCursorPos();
		ImGui::SetCursorPosX(cursor_pos.x - CAM_DEBUG_TEXT_Y_OFFS);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::SetCursorPos(cursor_pos);
	}

	if(components::effects::effect_is_playing() || components::effects::effect_is_paused())
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
			, _rttcam_toolbar_state
			, "Collapse/Expand camera toolbar"
			, &toolbar_button_background
			, &toolbar_button_background_hovered
			, &toolbar_button_background_active
			, &toolbar_button_open_size))
		{
			_rttcam_toolbar_state = _rttcam_toolbar_state ? false : true;
		} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

		if(_rttcam_toolbar_state)
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

				if(dvars::guizmo_enable->current.enabled)
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


				if(dvars::r_fakesun_preview->current.enabled)
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
					if(ggui::camera_settings::get_tabstate_fakesun() && ggui::camera_settings::is_tabstate_fakesun_active())
					{
						// close entire window if tab is in-front
						components::gui::toggle(ggui::state.czwnd.m_camera_settings, 0, true);
					}
					else if(!ggui::state.czwnd.m_camera_settings.menustate)
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
					ImGui::BeginDisabled(!can_fx_play);
					ImGui::BeginGroup();
					{
						static bool hov_fx_play;
						if (ggui::toolbar::image_togglebutton("fx_play"
							, hov_fx_play
							, can_fx_play
							, std::string("Play Effect for last selected fx_origin " + ggui::hotkeys::get_hotkey_for_command("fx_play")).c_str()
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							components::command::execute("fx_play");

						} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);


						static bool hov_fx_repeat;
						if (ggui::toolbar::image_togglebutton("fx_repeat"
							, hov_fx_repeat
							, can_fx_play
							, std::string("Re-trigger Effect every X seconds for last selected fx_origin " + ggui::hotkeys::get_hotkey_for_command("fx_repeat")).c_str()
							, &toolbar_button_background
							, &toolbar_button_background_hovered
							, &toolbar_button_background_active
							, &toolbar_button_size))
						{
							components::command::execute("fx_repeat");

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
							components::command::execute("fx_pause");

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
							components::command::execute("fx_stop");

						} ggui::rtt_handle_windowfocus_overlaywidget(camwnd);

						ImGui::EndDisabled();
						ImGui::EndGroup();

						if(!can_fx_play)
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

void Select_ApplyMatrix(float* rotate_axis /*eax*/, void* brush, int snap, float degree, int unk /*bool*/)
{
	const static uint32_t func_addr = 0x47CDE0;
	__asm
	{
		pushad;
		push	unk;
		push	0; // ecx?
		
		fld		degree;
		fstp    dword ptr[esp];
		
		push	snap;
		push	brush;
		
		mov		eax, rotate_axis; // lea
		
		call	func_addr;
		add		esp, 16;
		popad;
	}
}

void Select_RotateAxis(int axis /*eax*/, float degree, float* rotate_axis)
{
	const static uint32_t func_addr = 0x48FF40;
	__asm
	{
		pushad;
		push	rotate_axis;
		push	0; // ecx?

		mov		eax, axis;

		fld		degree;
		fstp    dword ptr[esp];
		
		call	func_addr;
		add		esp, 8;
		popad;
	}
}

void CameraCalcRayDir(int x, int y, float* dir)
{
	const auto cam = cmainframe::activewnd->m_pCamWnd;
	
	const float tan_half_y = tan(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f);
	const float tan_half_x = (tan_half_y * 0.75f + tan_half_y * 0.75f) / (float)cam->camera.height;

	const float xa = tan_half_x * (float)(y - cam->camera.height / 2);
	const float xb = tan_half_x * (float)(x - cam->camera.width / 2);

	dir[0] = cam->camera.vup[0] * xa + cam->camera.vright[0] * xb + cam->camera.vpn[0];
	dir[1] = cam->camera.vup[1] * xa + cam->camera.vright[1] * xb + cam->camera.vpn[1];
	dir[2] = cam->camera.vup[2] * xa + cam->camera.vright[2] * xb + cam->camera.vpn[2];

	utils::vector::normalize(dir);
}

// render to texture - camera window
void ccamwnd::rtt_camera_window()
{
	int p_styles = 0;
	int p_colors = 0;

	const auto IO = ImGui::GetIO();
	const auto camerawnd = ggui::get_rtt_camerawnd();
	const auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));
	ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

	float	window_padding = 0.0f;
	if(dvars::gui_rtt_padding_enabled && dvars::gui_rtt_padding_enabled->current.enabled) 
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
			_rttcam_toolbar_state = true;
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

		if(ImGui::IsWindowDocked() && wnd)
		{
			if(wnd->DockNode && wnd->DockNode->IsHiddenTabBar())
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


			// model selection drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (ImGui::AcceptDragDropPayload("MODEL_SELECTOR_ITEM"))
				{
					// reset manual left mouse capture
					ggui::dragdrop_reset_leftmouse_capture();
					
					const auto m_selector = ggui::get_rtt_modelselector();
					ggui::entity::addprop_helper_s no_undo = {};

					if(m_selector->overwrite_selection)
					{
						game::Undo_ClearRedo();
						game::Undo_GeneralStart("change entity model");

						if(components::remote_net::selection_is_brush(game::g_selected_brushes()->def))
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

						float dir[3];
						CameraCalcRayDir(camerawnd->cursor_pos_pt.x, static_cast<int>(camerawnd->scene_size_imgui.y) - camerawnd->cursor_pos_pt.y, dir);

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
				}

				ImGui::EndDragDropTarget();
			}


			
			// pop ItemSpacing
			ImGui::PopStyleVar(); p_styles--;

			ImGui::SetCursorPos(pre_image_cursor);
			const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

			camerawnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
			camerawnd->cursor_pos_pt = CPoint((LONG)camerawnd->cursor_pos.x, (LONG)camerawnd->cursor_pos.y);

			ggui::FixDockingTabbarTriangle(wnd, camerawnd);

			/*ImGui::Text("Moise is: %s", ImGui::IsMouseDown(ImGuiMouseButton_Left) ? "DOWN" : "UP");
			ImGui::SetCursorPos(pre_image_cursor);*/

			// *
			// ImGuizmo

			if(dvars::guizmo_enable->current.enabled)
			{
				// do not activate guizmo till initial left click is released
				// fixes unwanted transforms on multi selection via shift (if mouse is over guizmo)
				static bool guizmo_needs_activation = true;
				static bool guizmo_capture_active = false;

				if (const auto b = game::g_selected_brushes()->def; b)
				{
					if(guizmo_needs_activation)
					{
						camerawnd->capture_left_mousebutton = true;
						if(!guizmo_capture_active)
						{
							if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
							{
								guizmo_capture_active = true;
							}
						}
						else
						{
							guizmo_needs_activation = ImGui::IsMouseDown(ImGuiMouseButton_Left);

							/*if(guizmo_needs_activation)
							{
								game::printf_to_console("Mouse DOWN");
							}
							else
							{
								game::printf_to_console("Mouse UP");
							}*/
						}
					}
				}
				else
				{
					/*if(!guizmo_needs_activation)
					{
						game::printf_to_console("RESET");
					}*/

					// always track
					camerawnd->capture_left_mousebutton = true;
					guizmo_needs_activation = true;
					guizmo_capture_active = false;
				}

				if(guizmo_needs_activation)
				{
					goto SKIP_GUIZMO;
				}

				game::GfxMatrix view = {};
				game::GfxMatrix projection = {};

				// setup the view and projection matrices
				get_matrices_for_guizmo(&view, &projection);

				// imguizmo settings
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(camerawnd->scene_pos_imgui.x, camerawnd->scene_pos_imgui.y, camera_size.x, camera_size.y);

				const auto guizmo_mode = game::g_bRotateMode ? ImGuizmo::OPERATION::ROTATE : ImGuizmo::OPERATION::TRANSLATE;

				float mtx_scale[3] = { 1.0f, 1.0f, 1.0f };
				float angles[3] = { 0.0f, 0.0f, 0.0f };
				float snap[3] = { 0.0f, 0.0f, 0.0f };

				if (dvars::guizmo_snapping->current.enabled)
				{
					// default case
					float snap_size = xywnd::GRID_SIZES[game::g_qeglobals->d_gridsize];

					// snap to 1 2 5 10 15 30 60 90 when rotation mode is enabled 
					if (guizmo_mode == ImGuizmo::OPERATION::ROTATE)
					{
						switch (game::g_qeglobals->d_gridsize)
						{
						case 3: snap_size = 5.0f; break;
						case 4: snap_size = 10.0f; break;
						case 5: snap_size = 15.0f; break;
						case 6: snap_size = 30.0f; break;
						case 7: snap_size = 60.0f; break;
						case 9: snap_size = 90.0f; break;
						}
					}

					utils::vector::set_vec3(snap, snap_size);
				}

				if (dvars::guizmo_brush_mode->current.enabled)
				{
					if (const auto b = game::g_selected_brushes()->def; b)
					{
						// pass mouse input to ImGui
						if (ImGuizmo::IsOver())
						{
							camerawnd->capture_left_mousebutton = true;
						}

						// guizmo position
						game::vec3_t selection_center = {};

//#define VERTEX_GUIZMO
#ifdef VERTEX_GUIZMO
						const auto num_move_points = game::g_qeglobals->d_num_move_points;
						const auto selection_mode = game::g_qeglobals->d_select_mode;
						const bool in_vertex_mode = num_move_points && (selection_mode == game::sel_curvepoint || selection_mode == game::sel_area);
#else
						const bool in_vertex_mode = false;
#endif
						
						if (guizmo_mode == ImGuizmo::OPERATION::ROTATE)
						{
							// use radiants rotate origin
							utils::vector::copy(game::g_vRotateOrigin, selection_center);
						}
						else
						{
							if(in_vertex_mode)
							{
								get_selection_center_movepoints(selection_center);
							}
							else
							{
								// calculate center of brush/es
								get_selection_center(selection_center);
							}
						}

						float tmp_matrix[16];
						float delta_matrix[16];

						// build guizmo matrix from components
						ImGuizmo::RecomposeMatrixFromComponents(selection_center, angles, mtx_scale, tmp_matrix);
//#define BOUNDS_TEST					
#ifdef BOUNDS_TEST
						// *
						// bounds 
						game::vec3_t local_mins;
						game::vec3_t local_maxs;

						utils::vector::subtract(b->mins, selection_center, local_mins);
						utils::vector::subtract(b->maxs, selection_center, local_maxs);

						float bounds[] =
						{
							local_mins[0], local_mins[1], local_mins[2],
							local_maxs[0], local_maxs[1], local_maxs[2]
						};

						// draw guizmo / manipulate
						if (ImGuizmo::Manipulate(&view.m[0][0], &projection.m[0][0], guizmo_mode, ImGuizmo::MODE::WORLD, tmp_matrix, delta_matrix, snap, bounds))
#else
						if (ImGuizmo::Manipulate(&view.m[0][0], &projection.m[0][0], guizmo_mode, ImGuizmo::MODE::WORLD, tmp_matrix, delta_matrix, snap))
#endif
						{
							if (ImGuizmo::IsOver())
							{
								float delta_origin[3], delta_angles[3];

								// seperate manupulated matrix into components
								ImGuizmo::DecomposeMatrixToComponents(delta_matrix, delta_origin, delta_angles, mtx_scale);

								if (guizmo_mode == ImGuizmo::OPERATION::ROTATE)
								{
#if 0								// original way of getting the rotation matrix (here to stay to check against the one below)
									float rotate_axis_org[4][4] = {};
									rotate_axis_org[0][0] = game::g_vRotateOrigin[0];
									rotate_axis_org[0][1] = game::g_vRotateOrigin[1];
									rotate_axis_org[0][2] = game::g_vRotateOrigin[2];
									Select_RotateAxis(cmainframe::activewnd->m_pXYWnd->m_nViewType, delta_angles[cmainframe::activewnd->m_pXYWnd->m_nViewType], &rotate_axis_org[0][0]);
#endif
									float rotate_axis[4][4];

									// inverse angles
									utils::vector::inverse(delta_angles);

									// build a rotation matrix
									ImGuizmo::RecomposeMatrixFromComponents(game::g_vRotateOrigin, delta_angles, mtx_scale, &rotate_axis[0][0]);

									float rotate_axis_for_radiant[4][4] = {};

									// bring it into the format radiant expects
									rotate_axis_for_radiant[0][0] = rotate_axis[3][0];
									rotate_axis_for_radiant[0][1] = rotate_axis[3][1];
									rotate_axis_for_radiant[0][2] = rotate_axis[3][2];
									rotate_axis_for_radiant[0][3] = rotate_axis[0][0];

									rotate_axis_for_radiant[1][0] = rotate_axis[1][0];
									rotate_axis_for_radiant[1][1] = rotate_axis[2][0];
									rotate_axis_for_radiant[1][2] = rotate_axis[0][1];
									rotate_axis_for_radiant[1][3] = rotate_axis[1][1];

									rotate_axis_for_radiant[2][0] = rotate_axis[2][1];
									rotate_axis_for_radiant[2][1] = rotate_axis[0][2];
									rotate_axis_for_radiant[2][2] = rotate_axis[1][2];
									rotate_axis_for_radiant[2][3] = rotate_axis[2][2];

									// apply rotation to all selected brushes
									for (auto   sb = game::g_selected_brushes_next();
										(DWORD*)sb != game::currSelectedBrushes; // sb->next really points to &selected_brushes(currSelectedBrushes) eventually
												sb = sb->next)
									{
										if (const auto brush = sb->def; brush)
										{
											// degree is handled like a boolean, 0.0 will not rotate fixed size brushes / entities
											Select_ApplyMatrix(&rotate_axis_for_radiant[0][0], sb, false, 1.0f, false);
											components::remote_net::cmd_send_brush_select_deselect(true);
										}
									}
								}

								else if (guizmo_mode == ImGuizmo::OPERATION::TRANSLATE)
								{
									game::vec3_t end_vec = {};
									int end_vec_valid = true;
									
									// move all selected brushes using the delta
									for (auto	sb = game::g_selected_brushes_next();
										(DWORD*)sb != game::currSelectedBrushes; // sb->next really points to &selected_brushes(currSelectedBrushes) eventually
												sb = sb->next)
									{
										if (const auto brushes = sb->def; brushes)
										{
											if(in_vertex_mode)
											{
												end_vec_valid &= game::Brush_MoveVertex(delta_origin, brushes, game::g_qeglobals->d_move_points[0], end_vec);
											}
											else
											{
												game::Brush_Move(delta_origin, brushes, true);
												components::remote_net::cmd_send_brush_select_deselect(true);
											}
										}
									}
#ifdef VERTEX_GUIZMO
									if(end_vec_valid)
									{
										utils::vector::copy(end_vec, game::g_qeglobals->d_move_points[0]);
									}
#endif
								}
							}
						}
					}
				}

#if 0 // bounds

				game::vec3_t local_mins;
				game::vec3_t local_maxs;

				utils::vector::subtract(b->currSelection->mins, v_mid, local_mins);
				utils::vector::subtract(b->currSelection->maxs, v_mid, local_maxs);

				float bounds[] =
				{
					local_mins[0], local_mins[1], local_mins[2],
					local_maxs[0], local_maxs[1], local_maxs[2]
				};

				float tmp_matrix[16];
				ImGuizmo::RecomposeMatrixFromComponents(v_mid, angles, mtx_scale, tmp_matrix);

				if (ImGuizmo::Manipulate(&view.m[0][0], &projection.m[0][0], ImGuizmo::OPERATION::BOUNDS, ImGuizmo::MODE::WORLD, tmp_matrix, nullptr, nullptr, bounds))
				{

			}
#endif

				// ----------------------
				// entities (static models / lights / spawns etc)
				// actually handled pretty good via "Select_ApplyMatrix" above .. will still keep it for now

				else if (const auto edit_entity = game::g_edit_entity();
					edit_entity && edit_entity->epairs)
				{
					if (_stricmp(edit_entity->eclass->name, "worldspawn"))
					{
						// pass mouse input to imgui if guizmo is hovered (TODO: only pass left click)
						if (ImGuizmo::IsOver())
						{
							//camerawnd->window_hovered = false;
							camerawnd->capture_left_mousebutton = true;
						}

						for (auto epair = edit_entity->epairs; epair; epair = epair->next)
						{
							std::string key = utils::str_to_lower(epair->key);
							if (key == "angles")
							{
								// switch axis
								if (sscanf(epair->value, "%f %f %f", &angles[1], &angles[2], &angles[0]) == 3)
								{
								}

								break;
							}
						}

						float tmp_matrix[16];
						ImGuizmo::RecomposeMatrixFromComponents(edit_entity->origin, angles, mtx_scale, tmp_matrix);

						if (ImGuizmo::Manipulate(&view.m[0][0], &projection.m[0][0], guizmo_mode, ImGuizmo::MODE::WORLD, tmp_matrix, nullptr, snap))
						{
							if (ImGuizmo::IsOver())
							{
								float t_origin[3], t_angles[3];
								ImGuizmo::DecomposeMatrixToComponents(tmp_matrix, t_origin, t_angles, mtx_scale);

								char org_str[64] = {};
								ggui::entity::addprop_helper_s helper = {};

								helper.add_undo = false;

								if (sprintf_s(org_str, "%.5f %.5f %.5f", t_origin[0], t_origin[1], t_origin[2])) {
									helper.is_origin = true;
									ggui::entity::AddProp("origin", org_str, &helper);
								}

								// switch axis
								if (sprintf_s(org_str, "%.5f %.5f %.5f", t_angles[1], t_angles[2], t_angles[0])) {
									helper.is_angle = true;
									ggui::entity::AddProp("angles", org_str, &helper);
								}
							}

						}
					}
				}
			}

	SKIP_GUIZMO:
			rtt_camera_window_toolbar();
			
			ImGui::EndChild();
		}
	}
	ImGui::PopStyleColor(p_colors);
	ImGui::PopStyleVar(p_styles);
	ImGui::End();
}

// CTRL + SHIFT + RMButton
void cam_rotate()
{
	auto cam = cmainframe::activewnd->m_pCamWnd;
	auto prefs = game::g_PrefsDlg();
	
	CPoint point;
	GetCursorPos(&point);

	if (point.x != cam->m_ptCursor.x || point.y != cam->m_ptCursor.y)
	{
		// use anglespeed
		cam->camera.angles[1] = cam->camera.angles[1] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
		cam->camera.angles[0] = cam->camera.angles[0] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.y - cam->m_ptCursor.y);

		// org
		//cam->camera.angles[1] = cam->camera.angles[1] - prefs->m_nMoveSpeed / 500.0 * (point.x - cam->m_ptCursor.x);
		//cam->camera.angles[0] = cam->camera.angles[0] - prefs->m_nMoveSpeed / 500.0 * (point.y - cam->m_ptCursor.y);

		SetCursorPos(cam->m_ptCursor.x, cam->m_ptCursor.y);
		cam->x48 = 0;

		ShowCursor(0);
	}
}

void cam_positiondrag()
{
	auto cam = cmainframe::activewnd->m_pCamWnd;
	auto prefs = game::g_PrefsDlg();

	CPoint point;
	GetCursorPos(&point);
	
	if (point.x != cam->m_ptCursor.x || point.y != cam->m_ptCursor.y)
	{
		// use anglespeed
		cam->camera.angles[1] = cam->camera.angles[1] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
		
		// org
		//cam->camera.angles[1] = cam->camera.angles[1] - prefs->m_nMoveSpeed / 500.0 * (point.x - cam->m_ptCursor.x);

		const float delta = static_cast<float>(prefs->m_nMoveSpeed) / -250.0f * static_cast<float>(point.y - cam->m_ptCursor.y);
		utils::vector::normalize(cam->camera.vpn);
		
		cam->camera.origin[0] = cam->camera.vpn[0] * delta + cam->camera.origin[0];
		cam->camera.origin[1] = cam->camera.vpn[1] * delta + cam->camera.origin[1];
		//cam->camera.origin[2] = cam->camera.origin[2];

		SetCursorPos(cam->m_ptCursor.x, cam->m_ptCursor.y);
		cam->x48 = 0;
		
		ShowCursor(0);
	}
}

void ccamwnd::mouse_control(float dtime)
{
	const static uint32_t Cam_MouseControl_Func = 0x403950;
	__asm
	{
		push	esi;
		mov		esi, ecx; // esi = this
		push	dtime;
		call	Cam_MouseControl_Func; // __userpurge :: automatically fixes the stack
		pop		esi;
	}

	return;
}

void ccamwnd::mouse_up(ccamwnd* cam, int flags)
{
	cam->m_nCambuttonstate = 0;
	game::Drag_MouseUp(flags);
	cam->prob_some_cursor = 0;
	cam->x47 = 0;
	cam->x48 = 1; // prob. int16

	int sw_cur;
	do {
		sw_cur = ShowCursor(1);
	} while (sw_cur < 0);
}

void ccamwnd::mouse_moved(ccamwnd* wnd, int buttons, int x, int y)
{
	const static uint32_t CCamWnd__Cam_MouseMoved_Func = 0x404FC0;
	__asm
	{
		pushad;

		mov		eax, buttons;
		push	y;
		push	x;
		mov		ecx, wnd;
		call	CCamWnd__Cam_MouseMoved_Func; // cleans the stack

		popad;
	}
}

// *
// | -------------------- MSG typedefs ------------------------
// *


typedef void(__thiscall* on_ccamwnd_msg)(ccamwnd*, UINT, CPoint);

	// mouse scrolling handled in cmainframe
    on_ccamwnd_msg __on_lbutton_down;
    on_ccamwnd_msg __on_lbutton_up;
    on_ccamwnd_msg __on_rbutton_down;
    on_ccamwnd_msg __on_rbutton_up;
	on_ccamwnd_msg __on_mouse_move;


typedef void(__stdcall* on_ccamwnd_key)(UINT nChar, UINT nRepCnt, UINT nFlags);

	on_ccamwnd_key __on_keydown_cam;
	on_ccamwnd_key __on_keyup_cam;


// *
// | ----------------- Left Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_lbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_lbutton_down(pThis, nFlags, point); // not in use?
}

void __fastcall ccamwnd::on_lbutton_up([[maybe_unused]] ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	//__on_lbutton_up(pThis, nFlags, point);

	czwnd::on_lbutton_up(cmainframe::activewnd->m_pZWnd, nullptr, nFlags, point); // redirect
}


// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_rbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_rbutton_down(pThis, nFlags, point); // not in use?
}

void __fastcall ccamwnd::on_rbutton_up([[maybe_unused]] ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	//__on_rbutton_up(pThis, nFlags, point);

	czwnd::on_rbutton_up(cmainframe::activewnd->m_pZWnd, nullptr, nFlags, point); // redirect
}


// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall ccamwnd::on_mouse_move(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_mouse_move(pThis, nFlags, point);
}


// *
// | ----------------- Keys Up/Down ---------------------
// *

void __stdcall ccamwnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyDown
	__on_keydown_cam(nChar, nRepCnt, nFlags);
}

void __stdcall ccamwnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyUp
	__on_keyup_cam(nChar, nRepCnt, nFlags);
}


// *
// | ----------------- Windowproc ---------------------
// *

BOOL WINAPI ccamwnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// handle text input when the imgui camera window is active (selected) 
	// but the user is within a textbox in some other imgui window
	if (Msg == WM_CHAR || Msg == WM_KEYDOWN || Msg == WM_KEYUP)
	{
		if (ggui::cz_context_ready())
		{
			// set cz context (in-case we use multiple imgui context's)
			IMGUI_BEGIN_CZWND;

			if (!ggui::get_rtt_camerawnd()->window_hovered && ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
	}

#if 0
	if(Msg == WM_SIZE)
	{
		// disable postfx when resizing
		components::renderer::postfx::set_disable_duration(0.45f);
		components::renderer::postfx::disable();
	}
#endif
	
	// => og CamWndProc
    return utils::hook::call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x402D90)(hWnd, Msg, wParam, lParam);
}


void __declspec(naked) camwnd_set_child_window_style()
{
	const static uint32_t retn_pt = 0x402EF7;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD; //WS_VISIBLE;
		jmp		retn_pt;
	}
}

void __declspec(naked) camwnd_set_detatched_child_window_style()
{
	const static uint32_t retn_pt = 0x402ED1;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_POPUP;
		jmp		retn_pt;
	}
}

bool should_move_selection()
{
	if(dvars::guizmo_enable->current.enabled)
	{
		if(ImGuizmo::IsOver() || ImGuizmo::IsUsing())
		{
			return false;
		}
		
		// disable brush drsagging within the camera window
		/*if (ggui::get_rtt_camerawnd()->window_hovered && game::g_qeglobals->d_select_mode == game::select_t::sel_brush)
		{
			return false;
		}*/
	}

    return true;
}

void __declspec(naked) move_selection_stub()
{
    const static uint32_t move_selection_func = 0x47F0C0;
    const static uint32_t retn_pt = 0x48023D;
    __asm
    {
		pushad; // new
        call    should_move_selection;
        test    al, al;
		popad;	// new
        
        je      SKIP_MOVE;
        call    move_selection_func;

    SKIP_MOVE:
        jmp		retn_pt;
    }
}

// *
// *

void ccamwnd::register_dvars()
{
	dvars::guizmo_enable = dvars::register_bool(
		/* name		*/ "guizmo_enable",
		/* default	*/ true,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Enable guizmo");

	dvars::guizmo_snapping = dvars::register_bool(
		/* name		*/ "guizmo_snapping",
		/* default	*/ true,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Guizmo: Enable grid-snapping");

	dvars::guizmo_brush_mode = dvars::register_bool(
		/* name		*/ "guizmo_brush_mode",
		/* default	*/ true,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Guizmo: Enable brush mode");

	dvars::gui_camera_toolbar_defaultopen = dvars::register_bool(
		/* name		*/ "gui_camera_toolbar_defaultopen",
		/* default	*/ false,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "Open the camera toolbar by default");
}

void ccamwnd::hooks()
{
	// hook ccamwnd message handler
    utils::hook::set(0x402E86 + 4, ccamwnd::windowproc);

#if 1
	// ccamwnd::precreatewindow -> change window style for child windows (split view, not detatched)
	utils::hook::nop(0x402EEE, 2);
	utils::hook::nop(0x402EF0, 7);
		 utils::hook(0x402EF0, camwnd_set_child_window_style, HOOK_JUMP).install()->quick();

	// ccamwnd::precreatewindow -> change window style for detatched windows (split view, detatched)
	utils::hook::nop(0x402ECA, 7);
		 utils::hook(0x402ECA, camwnd_set_detatched_child_window_style, HOOK_JUMP).install()->quick();
#endif

	utils::hook(0x405346, cam_rotate, HOOK_CALL).install()->quick();
	utils::hook(0x405321, cam_positiondrag, HOOK_CALL).install()->quick();

	// disable entity dragging with mouse in camerawnd
	//utils::hook::set<BYTE>(0x40539D, 0xEB);
	utils::hook(0x480238, move_selection_stub, HOOK_JUMP).install()->quick();
	
	//utils::hook::nop(0x404AD1, 23);
	
	__on_lbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403160, ccamwnd::on_lbutton_down, HK_JUMP));
    __on_lbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4031D0, ccamwnd::on_lbutton_up, HK_JUMP));

    __on_rbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4032B0, ccamwnd::on_rbutton_down, HK_JUMP));
    __on_rbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403310, ccamwnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403100, ccamwnd::on_mouse_move, HK_JUMP));

	__on_keydown_cam	= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x402F60, ccamwnd::on_keydown, HK_JUMP));
	__on_keyup_cam		= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x408B70, ccamwnd::on_keyup, HK_JUMP));
}