#include "std_include.hpp"

namespace ggui::preferences
{
	

}

namespace ggui
{
	void preferences_dialog::register_child(const std::string& _child_name, const std::function<void()>& _callback)
	{
		_pref_childs[_child_name] = pref_child_s
		(
			m_child_count,
			_callback
		);

		m_child_count++;
	}

	float preferences_dialog::pref_child_lambda(const std::string& child_name, const float child_height, const float* bg_color, const float* border_color, const std::function<void()>& cb)
	{
		auto background_color = bg_color;

		// do not offset the first child
		if (_pref_childs[child_name].index)
		{
			SPACING(0.0f, 24.0f);
		}

		if ((int)_pref_childs[child_name].index == m_child_current)
		{
			background_color = m_child_bg_highlight_col;
		}

		const std::string child_str = "[ "s + child_name + " ]"s;
		const float child_indent = 8.0f;
		const float child_width = ImGui::GetContentRegionAvail().x - child_indent;

		const auto min = ImGui::GetCursorScreenPos();
		const auto max = ImVec2(min.x + child_width, min.y + child_height);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(background_color)), 10.0f, ImDrawFlags_RoundCornersBottom);
		ImGui::GetWindowDrawList()->AddRect(min, max, ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(border_color)), 10.0f, ImDrawFlags_RoundCornersBottom);

		ImGui::BeginGroup();
		ImGui::title_with_background(child_str.c_str(), min, child_width, 38.0f, dvars::gui_window_bg_color->current.vector, dvars::gui_border_color->current.vector, false, child_indent);

		if (m_update_scroll)
		{
			if ((int)_pref_childs[child_name].index == m_child_current)
			{
				ImGui::SetScrollHereY(0.0f);
				m_update_scroll = false;
			}
		}

		ImGui::Indent(child_indent);

		ImGui::PushClipRect(min, max, true);
		cb();
		ImGui::PopClipRect();

		SPACING(0.0f, 12.0f);
		ImGui::EndGroup();
		return ImGui::GetItemRectSize().y;
	}

	// ---

	void preferences_dialog::child_gui()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_GUI, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			ImGui::title_with_seperator("General", false);

			ImGui::Checkbox("Show mousecursor origin within the menubar", &dvars::gui_menubar_show_mouseorigin->current.enabled);

			if (ImGui::Button("Edit Colors ..")) {
				GET_GUI(ggui::gui_colors_dialog)->toggle();
			}

			ImGui::SameLine();
			if (ImGui::Button("Edit Toolbar ..")) {
				GET_GUI(ggui::toolbar_edit_dialog)->toggle();
			}

			// -----------------
			ImGui::title_with_seperator("Docking");

			if (ImGui::Button("Reset Dockspace")) {
				ggui::m_dockspace_reset = true;
			}

			//const char* background_names[] = { "None", "Grid", "Camera" };
			//const char* background_str = (dvars::gui_mainframe_background->current.integer >= 0 && dvars::gui_mainframe_background->current.integer <= 2) ? background_names[dvars::gui_mainframe_background->current.integer] : "Unknown";
			//ImGui::SliderInt("Dockspace Background", &dvars::gui_mainframe_background->current.integer, 0, 2, background_str);

			// -----------------
			ImGui::title_with_seperator("New / Ported Features");
			ImGui::Checkbox("Use new experimental surface-inspector", &dvars::gui_use_new_surfinspector->current.enabled); TT(dvars::gui_use_new_surfinspector->description);
			ImGui::Checkbox("Use new vertex edit dialog", &dvars::gui_use_new_vertedit_dialog->current.enabled); TT(dvars::gui_use_new_vertedit_dialog->description);
			ImGui::Checkbox("Use new file dialogs", &dvars::gui_use_new_filedialog->current.enabled); TT(dvars::gui_use_new_filedialog->description);

			if (ImGui::Checkbox("Use new grid context menu", &dvars::gui_use_new_context_menu->current.enabled))
			{
				game::g_PrefsDlg()->m_bRightClick = !dvars::gui_use_new_context_menu->current.enabled;
			}

			// -----------------
			ImGui::title_with_seperator("Property Editor");
			ImGui::Checkbox("Default Open - Classlist", &dvars::gui_props_classlist_defaultopen->current.enabled); TT(dvars::gui_props_classlist_defaultopen->description);
			ImGui::Checkbox("Default Open - Spawnflags", &dvars::gui_props_spawnflags_defaultopen->current.enabled); TT(dvars::gui_props_spawnflags_defaultopen->description);
			ImGui::Checkbox("Default Open - Comments", &dvars::gui_props_comments_defaultopen->current.enabled); TT(dvars::gui_props_comments_defaultopen->description);
			ImGui::Checkbox("Incorporate surface-inspector into property window", &dvars::gui_props_surfinspector->current.enabled); TT(dvars::gui_props_surfinspector->description);
		});
	}

	void preferences_dialog::child_general()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_GENERAL, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			const auto prefs = game::g_PrefsDlg();

			ImGui::title_with_seperator("Load / Save", false);

			ImGui::Checkbox("Load last project on open", &prefs->m_bLoadLast);
			ImGui::Checkbox("Load last map on open", &prefs->m_bLoadLastMap);

			ImGui::Checkbox("Remove tiny brushes on map load", &prefs->m_bCleanTinyBrushes); TT("Not a saved setting!");
			ImGui::BeginDisabled(!prefs->m_bCleanTinyBrushes);
			{
				ImGui::DragFloat("Tiny brush threshold", &prefs->m_fTinySize, 0.005f, 0, 16.0f, "%.2f");
				TT("Brush will be removed if ANY side is below the threshold. Not a saved setting!");
			}
			ImGui::EndDisabled();

			ImGui::Checkbox("Loose changes dialog on exit", &prefs->loose_changes);
			ImGui::Checkbox("Enable snapshots", &prefs->m_bSnapShots);

			ImGui::Checkbox("Enable autosave", &prefs->m_bAutoSave);
			ImGui::BeginDisabled(!prefs->m_bAutoSave);
			{
				ImGui::SliderInt("Autosave intervall", &prefs->m_nAutoSave, 1, 60);
			}
			ImGui::EndDisabled();


			// -----------------
			ImGui::title_with_seperator("System Settings");
			ImGui::DragInt("Max fps grid window", &dvars::radiant_maxfps_grid->current.integer, 0.1f, dvars::radiant_maxfps_grid->domain.integer.min, dvars::radiant_maxfps_grid->domain.integer.max);
			ImGui::DragInt("Max fps camera window", &dvars::radiant_maxfps_camera->current.integer, 0.1f, dvars::radiant_maxfps_camera->domain.integer.min, dvars::radiant_maxfps_camera->domain.integer.max);
			ImGui::DragInt("Max fps texture window", &dvars::radiant_maxfps_textures->current.integer, 0.1f, dvars::radiant_maxfps_textures->domain.integer.min, dvars::radiant_maxfps_textures->domain.integer.max);
			ImGui::DragInt("Max fps modelselector window", &dvars::radiant_maxfps_modelselector->current.integer, 0.1f, dvars::radiant_maxfps_modelselector->domain.integer.min, dvars::radiant_maxfps_modelselector->domain.integer.max);
			ImGui::DragInt("Max fps gui", &dvars::radiant_maxfps_mainframe->current.integer, 0.1f, dvars::radiant_maxfps_mainframe->domain.integer.min, dvars::radiant_maxfps_mainframe->domain.integer.max);

			ImGui::Checkbox("Discord Rich-Presence", &components::discord::g_enable_discord_rpc);

			// -----------------
			ImGui::title_with_seperator("Unordered Settings");
			ImGui::Checkbox("Linking entities keeps selection", &prefs->linking_keeps_selection);

			if (ImGui::SliderInt("Undo levels", &prefs->m_nUndoLevels, 16, 512))
			{
				game::g_undoMaxSize = prefs->m_nUndoLevels;
			}


			// -----------------
			ImGui::title_with_seperator("Views / Scales / Sizes");

			ImGui::DragFloat("Model origin size", &prefs->model_origin_size, 0.1f, 0, 100, "%.1f");
			ImGui::DragFloat("Prefab origin size", &prefs->prefab_origin_size, 0.1f, 0, 100, "%.1f");
			ImGui::DragInt("Model planting scale base", &prefs->scale_base, 0.1f, 1, 1000);
			ImGui::DragInt("Model planting scale range", &prefs->scale_range, 0.1f, 1, 1000);
			ImGui::DragInt("Tolerant weld distance", &prefs->tolerant_weld, 0.1f, 1, 128);
			ImGui::DragInt("Splay distance", &prefs->splay, 0.1f, 1, 128); TT("Distance used to push 2 or more selected vertices apart when using splay (default ALT+W)");
			ImGui::DragInt("Vehicle arrow time", &prefs->vehicle_arrow_time, 0.1f, 0, 10000);
			ImGui::DragInt("Vehicle arrow size", &prefs->vehicle_arrow_size, 0.1f, 0, 1000);
		});
	}

	void preferences_dialog::child_grid()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_GRID, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			const auto prefs = game::g_PrefsDlg();

			ImGui::title_with_seperator("Mouse", false);
			ImGui::Checkbox("Zoom to cursor", &dvars::grid_zoom_to_cursor->current.enabled);
			ImGui::Checkbox("Disable grid snapping", &prefs->m_bNoClamp); TT("Org: Don't clamp plane points");


			// -----------------
			ImGui::title_with_seperator("Dragging");
			ImGui::Checkbox("Alt always drags when multiple brushes are selected", &prefs->m_bALTEdge); TT("Holding ALT and having selected multiple brushes wont extrude, but instead move brushes when clicked outside");
			ImGui::Checkbox("Fast view dragging", &prefs->fast_2d_view_dragging); TT("Hides static models when moving the grid");


			// -----------------
			ImGui::title_with_seperator("Visuals");
			ImGui::Checkbox("Show sizing info", &prefs->m_bSizePaint);
			ImGui::Checkbox("Thick selection lines", &prefs->thick_selection_lines);
			ImGui::Checkbox("Texture brushes", &prefs->texture_brush_2d);
			ImGui::Checkbox("Texture meshes", &prefs->texture_mesh_2d);
		});
	}

	void preferences_dialog::child_camera()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_CAMERA, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			const auto prefs = game::g_PrefsDlg();

			ImGui::title_with_seperator("General", false);
			ImGui::SliderInt("Camera mode", &prefs->camera_mode, 0, 2);
			ImGui::DragFloat("Field of view", &prefs->camera_fov, 0.1f, 1.0f, 180.0f, "%.1f");


			// -----------------
			ImGui::title_with_seperator("Mouse");
			ImGui::SliderInt("Camera speed", &prefs->m_nMoveSpeed, 10, 5000);
			ImGui::SliderInt("Camera angle speed", &prefs->m_nAngleSpeed, 1, 1000);


			// -----------------
			ImGui::title_with_seperator("Visuals / Rendering");
			ImGui::Checkbox("Cull sky when using cubic clipping", &prefs->b_mCullSky);
			ImGui::Checkbox("Draw shadowcaster materials on xmodels", &dvars::r_draw_model_shadowcaster->current.enabled); TT("Render black shadowcaster materials on xmodels.\nMostly seen/used on trees");

			SPACING(0.0f, 4.0f);

			ImGui::Checkbox("Sunpreview enable shadows", &dvars::r_sunpreview_shadow_enable->current.enabled);
			ImGui::DragFloat("Shadow draw-distance", &dvars::r_sunpreview_shadow_dist->current.value, 25.0f, 0, FLT_MAX, "%.0f");

			// -----------------
			ImGui::title_with_seperator("Toolbar");
			ImGui::Checkbox("Draw FPS within the camera window", &dvars::gui_draw_fps->current.enabled);
			ImGui::Checkbox("Default Open - Toolbar", &dvars::gui_camera_toolbar_defaultopen->current.enabled);

		});
	}

	void preferences_dialog::child_textures()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_TEXTURES, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			ImGui::title_with_seperator("General", false);
			ImGui::Checkbox("Draw scrollbar", &dvars::gui_texwnd_draw_scrollbar->current.enabled);
			ImGui::Checkbox("Show scroll position in percent", &dvars::gui_texwnd_draw_scrollpercent->current.enabled);

			ImGui::title_with_seperator("Quality");

			// actual picmip vars the renderer is using (used to check if quality was changed)
			const auto& r_picmip_renderer_val = *reinterpret_cast<int*>(0x14E6CF8);
			const auto& r_picmip_spec_renderer_val = *reinterpret_cast<int*>(0x14E6D00);
			const auto& r_picmip_bump_renderer_val = *reinterpret_cast<int*>(0x14E6CFC);

			// 3 = Low, 0 = Maximum

			const auto r_picmip = game::Dvar_FindVar("r_picmip");
			const auto r_picmip_spec = game::Dvar_FindVar("r_picmip_spec");
			const auto r_picmip_bump = game::Dvar_FindVar("r_picmip_bump");

			const char* quality_names[4] = { "Low", "Normal", "High", "Maximum" };
			const char* quality_col = (r_picmip->current.integer >= 0 && r_picmip->current.integer <= 3) ? quality_names[3 - r_picmip->current.integer] : "Unknown";
			const char* quality_spec = (r_picmip_spec->current.integer >= 0 && r_picmip_spec->current.integer <= 3) ? quality_names[3 - r_picmip_spec->current.integer] : "Unknown";
			const char* quality_bump = (r_picmip_bump->current.integer >= 0 && r_picmip_bump->current.integer <= 3) ? quality_names[3 - r_picmip_bump->current.integer] : "Unknown";

			int r_picmip_proxy = 3 - r_picmip->current.integer;
			int r_picmip_spec_proxy = 3 - r_picmip_spec->current.integer;
			int r_picmip_bump_proxy = 3 - r_picmip_bump->current.integer;

			ImGui::SliderInt("Color Texture Quality", &r_picmip_proxy, 0, 3, quality_col);
			r_picmip->current.integer = 3 - r_picmip_proxy;
			game::g_qeglobals->d_savedinfo.d_picmip = r_picmip->current.integer;

			ImGui::SliderInt("Specular Texture Quality", &r_picmip_spec_proxy, 0, 3, quality_spec);
			r_picmip_spec->current.integer = 3 - r_picmip_spec_proxy;

			ImGui::SliderInt("Bump Texture Quality", &r_picmip_bump_proxy, 0, 3, quality_bump);
			r_picmip_bump->current.integer = 3 - r_picmip_bump_proxy;

			const auto sliderint_size = ImGui::GetItemRectSize();
			const bool needs_reload = r_picmip_renderer_val != r_picmip->current.integer || r_picmip_spec_renderer_val != r_picmip_spec->current.integer || r_picmip_bump_renderer_val != r_picmip_bump->current.integer;

			ImGui::BeginDisabled(!needs_reload);
			if (ImGui::Button("Apply / Reload Textures", ImVec2(ImGui::CalcItemWidth(), 0.0f))) //ImVec2(sliderint_size.x - 145.0f, 0.0f)))
			{
				// R_UpdateMipMap
				cdeclcall(void, 0x5139A0);
				game::R_ReloadImages();
			}
			ImGui::EndDisabled();
		});
	}

	void preferences_dialog::child_livelink()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_LIVELINK, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			ImGui::title_with_seperator("General", false);
			ImGui::Checkbox("Enable live-link", &dvars::radiant_live->current.enabled);
			ImGui::Checkbox("Sync Worldspawn", &dvars::radiant_liveWorldspawn->current.enabled); TT(dvars::radiant_liveWorldspawn->description);

			ImGui::InputScalar("Port", ImGuiDataType_S32, &dvars::radiant_livePort->current.integer);
			//ImGui::DragInt("Port", &dvars::radiant_livePort->current.integer, 0.5f, 1, 99999);


			// -----------------
			ImGui::title_with_seperator("Debug / Experimental Features");
			ImGui::Checkbox("Enable debug prints", &dvars::radiant_liveDebug->current.enabled);
		});
	}

	void preferences_dialog::child_developer()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_DEVELOPER, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			ImGui::title_with_seperator("Functional stuff", false);
			ImGui::Checkbox("Enable map-parsing debug prints", &dvars::parse_debug->current.enabled);

			// -----------------
			ImGui::title_with_seperator("Random stuff");
			if (ImGui::Button("Toggle template gui"))
			{
				GET_GUI(template_dialog)->toggle();
			}

			SPACING(0.0f, 4.0f);

			ImGui::Text("camera - cambuttonstate: %d", cmainframe::activewnd->m_pCamWnd->m_nCambuttonstate);
			ImGui::Text("camera - cursor_visible: %d", cmainframe::activewnd->m_pCamWnd->cursor_visible);
			ImGui::Text("camera - window hovered: %d", GET_GUI(ggui::camera_dialog)->rtt_is_hovered());

			SPACING(0.0f, 4.0f);

			ImGui::DragInt("Int 01", &dev_num_01, 0.1f);
			ImGui::DragFloat3("Vec4 01", dev_vec_01, 25.0f);
			ImGui::ColorEdit4("Color 01", dev_color_01, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		});
	}

	void preferences_dialog::gui()
	{
		if (!this->is_initiated())
		{
			register_child(CAT_GENERAL, std::bind(&preferences_dialog::child_general, this));
			register_child(CAT_GUI, std::bind(&preferences_dialog::child_gui, this));
			register_child(CAT_GRID, std::bind(&preferences_dialog::child_grid, this));
			register_child(CAT_CAMERA, std::bind(&preferences_dialog::child_camera, this));
			register_child(CAT_TEXTURES, std::bind(&preferences_dialog::child_textures, this));
			register_child(CAT_LIVELINK, std::bind(&preferences_dialog::child_livelink, this));
			register_child(CAT_DEVELOPER, std::bind(&preferences_dialog::child_developer, this));

			this->set_initiated();
		}

		const auto MIN_WINDOW_SIZE = ImVec2(800.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(800.0f, 600.0f);
		ggui::set_next_window_initial_pos_and_constraints(MIN_WINDOW_SIZE, INITIAL_WINDOW_SIZE);

		int stylevars = 0, stylecolors = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f)); stylevars++;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.08f, 0.31f)); stylecolors++;

		if (!ImGui::Begin("Preferences##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PopStyleColor(stylecolors);
			ImGui::PopStyleVar(stylevars);
			ImGui::End();
			return;
		}

		if (ImGui::BeginListBox("##pref_cat", ImVec2(180.0f, ImGui::GetContentRegionAvail().y)))
		{
			int i = 0;
			for (const auto& child : _pref_childs)
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
		}

		ImGui::SameLine();

		// needs to be this way or ImGui throws an assert when minimizing radiant while having the preference window open
		if (!ImGui::BeginChild("##pref_child", ImVec2(0, 0), false))
		{
			ImGui::EndChild();
			ImGui::PopStyleColor(stylecolors);
			ImGui::PopStyleVar(stylevars);
			ImGui::End();
			return;
		}

		// draw all "childs" 
		for (const auto& child : _pref_childs)
		{
			child.second.callback();
		}

		// end "##pref_child"
		ImGui::EndChild();

		ImGui::PopStyleColor(stylecolors);
		ImGui::PopStyleVar(stylevars);

		// end "Preferences##window"
		ImGui::End();
	}

	// CMainFrame::OnPrefs
	void preferences_dialog::on_prefsdialog_command()
	{
		GET_GUI(ggui::preferences_dialog)->toggle();
	}

	void preferences_dialog::hooks()
	{
		// detour CMainFrame::OnPrefs (hotkey to open the original dialog)
		utils::hook::detour(0x426950, preferences_dialog::on_prefsdialog_command, HK_JUMP);
	}

	void preferences_dialog::register_dvars()
	{
		dvars::radiant_maxfps_grid = dvars::register_int(
			/* name		*/ "radiant_maxfps_grid",
			/* val		*/ 250,
			/* minVal	*/ 30,
			/* maxVal	*/ 250,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "redraw the grid window x frames/second");

		dvars::radiant_maxfps_camera = dvars::register_int(
			/* name		*/ "radiant_maxfps_camera",
			/* val		*/ 250,
			/* minVal	*/ 30,
			/* maxVal	*/ 250,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "redraw the camera window x frames/second");

		dvars::radiant_maxfps_textures = dvars::register_int(
			/* name		*/ "radiant_maxfps_textures",
			/* val		*/ 60,
			/* minVal	*/ 30,
			/* maxVal	*/ 250,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "redraw the texture window x frames/second");

		dvars::radiant_maxfps_modelselector = dvars::register_int(
			/* name		*/ "radiant_maxfps_modelselector",
			/* val		*/ 250,
			/* minVal	*/ 30,
			/* maxVal	*/ 250,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "redraw the modelselector window x frames/second");

		dvars::radiant_maxfps_mainframe = dvars::register_int(
			/* name		*/ "radiant_maxfps_mainframe",
			/* val		*/ 250,
			/* minVal	*/ 30,
			/* maxVal	*/ 250,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "redraw the main window x frames/second (main gui)");

		dvars::gui_use_new_context_menu = dvars::register_bool(
			/* name		*/ "gui_use_new_context_menu",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Use new grid context menu");
	}

	REGISTER_GUI(preferences_dialog);
}

