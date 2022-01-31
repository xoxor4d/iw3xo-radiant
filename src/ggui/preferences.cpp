#include "std_include.hpp"

namespace ggui::preferences
{
	int		dev_num_01 = 0;
	float	dev_vec_01[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float	dev_color_01[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	// --------------------------------------
	
	const std::string CAT_GUI = "Gui";
	const std::string CAT_GENERAL = "General";
	const std::string CAT_GRID = "Grid";
	const std::string CAT_CAMERA = "Camera";
	const std::string CAT_TEXTURES = "Textures";
	const std::string CAT_LIVELINK = "Live Link";
	const std::string CAT_DEVELOPER = "Developer";

	bool _pref_update_scroll = false;
	
	int _pref_child_current = -1;
	unsigned int _pref_child_count = 0;
	
	const float _pref_child_bg_col[4] = { 0.1f, 0.1f, 0.1f, 0.2f };
	const float _pref_child_bg_highlight_col[4] = { 0.7f, 0.7f, 0.7f, 0.1f };
	
	struct pref_child_s
	{
		unsigned int index;
		std::function<void()> callback;
	};
	
	nlohmann::fifo_map<std::string, pref_child_s> _pref_childs;

	void register_child(const std::string& _child_name, std::function<void()> _callback)
	{
		_pref_childs[_child_name] = pref_child_s
		(
			_pref_child_count,
			_callback
		);

		_pref_child_count++;
	}

	float pref_child_lambda(const std::string& child_name, const float child_height, const float* bg_color, const float* border_color, const std::function<void()>& cb)
	{
		auto background_color = bg_color;

		// do not offset the first child
		if(_pref_childs[child_name].index)
		{
			SPACING(0.0f, 24.0f);
		}

		if ((int)_pref_childs[child_name].index == _pref_child_current)
		{
			background_color = _pref_child_bg_highlight_col;
		}

		const std::string child_str = "[ "s + child_name + " ]"s;
		const float child_indent = 8.0f;
		const float child_width = ImGui::GetContentRegionAvailWidth() - child_indent;

		const auto min = ImGui::GetCursorScreenPos();
		const auto max = ImVec2(min.x + child_width, min.y + child_height);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(background_color)), 10.0f, ImDrawFlags_RoundCornersBottom);
		ImGui::GetWindowDrawList()->AddRect(min, max, ImGui::ColorConvertFloat4ToU32(ImGui::ToImVec4(border_color)), 10.0f, ImDrawFlags_RoundCornersBottom);

		ImGui::BeginGroup();
		ImGui::title_with_background(child_str.c_str(), min, child_width, 38.0f, dvars::gui_window_bg_color->current.vector, dvars::gui_border_color->current.vector, false, child_indent);

		if (_pref_update_scroll)
		{
			if ((int)_pref_childs[child_name].index == _pref_child_current)
			{
				ImGui::SetScrollHereY(0.0f);
				_pref_update_scroll = false;
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

	void child_gui()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_GUI, height, _pref_child_bg_col, dvars::gui_border_color->current.vector, []
		{
			ImGui::title_with_seperator("General", false);

			ImGui::Checkbox("Show mousecursor origin within the menubar", &dvars::gui_menubar_show_mouseorigin->current.enabled);

			if (ImGui::Checkbox("Floating Toolbar", &dvars::gui_floating_toolbar->current.enabled)) {
				ggui::toolbar_reset = true;
			}

			// -----------------
			ImGui::title_with_seperator("Docking");

			if (ImGui::Button("Reset Dockspace")) {
				ggui::reset_dockspace = true;
			}

			ImGui::SameLine();
			
			ImGui::BeginDisabled(!dvars::gui_floating_toolbar->current.enabled);
			{
				ImGui::Checkbox("Resize Dockspace for floating toolbar", &dvars::gui_resize_dockspace->current.enabled);
			}
			ImGui::EndDisabled();

			const char* background_names[] = { "None", "Grid", "Camera" };
			const char* background_str = (dvars::gui_mainframe_background->current.integer >= 0 && dvars::gui_mainframe_background->current.integer <= 2) ? background_names[dvars::gui_mainframe_background->current.integer] : "Unknown";
			ImGui::SliderInt("Dockspace Background", &dvars::gui_mainframe_background->current.integer, 0, 2, background_str);


			// -----------------
			ImGui::title_with_seperator("Property Editor");
			ImGui::Checkbox("Default Open - Classlist", &dvars::gui_props_classlist_defaultopen->current.enabled); TT(dvars::gui_props_classlist_defaultopen->description);
			ImGui::Checkbox("Default Open - Spawnflags", &dvars::gui_props_spawnflags_defaultopen->current.enabled); TT(dvars::gui_props_spawnflags_defaultopen->description);
			ImGui::Checkbox("Default Open - Comments", &dvars::gui_props_comments_defaultopen->current.enabled); TT(dvars::gui_props_comments_defaultopen->description);
			ImGui::Checkbox("Incorporate Surface Inspector into property window", &dvars::gui_props_surfinspector->current.enabled); TT(dvars::gui_props_surfinspector->description);
		});
	}

	void child_general()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_GENERAL, height, _pref_child_bg_col, dvars::gui_border_color->current.vector, []
		{
			auto prefs = game::g_PrefsDlg();

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

	void child_grid()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_GRID, height, _pref_child_bg_col, dvars::gui_border_color->current.vector, []
		{
			auto prefs = game::g_PrefsDlg();

			ImGui::title_with_seperator("Mouse", false);
			ImGui::Checkbox("Zoom to cursor", &dvars::grid_zoom_to_cursor->current.enabled);
			ImGui::Checkbox("Enable right-click context menu", &prefs->m_bRightClick); TT("Org: Right click to drop entities (really wrong)");
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
	
	void child_camera()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_CAMERA, height, _pref_child_bg_col, dvars::gui_border_color->current.vector, []
		{
			auto prefs = game::g_PrefsDlg();

			ImGui::title_with_seperator("General", false);
			ImGui::SliderInt("Camera mode", &prefs->camera_mode, 0, 2);
			ImGui::DragFloat("Field of view", &prefs->camera_fov, 0.1f, 1.0f, 180.0f, "%.1f");


			// -----------------
			ImGui::title_with_seperator("Mouse");
			ImGui::SliderInt("Camera speed", &prefs->m_nMoveSpeed, 10, 5000);
			ImGui::SliderInt("Camera angle speed", &prefs->m_nAngleSpeed, 1, 1000);
			ImGui::Checkbox("Enable right-click context menu", &prefs->m_bRightClick); TT("Org: Right click to drop entities (really wrong)");


			// -----------------
			ImGui::title_with_seperator("Visuals / Rendering");
			ImGui::Checkbox("Cull sky when using cubic clipping", &prefs->b_mCullSky);
			ImGui::Checkbox("Draw shadowcaster materials on xmodels", &dvars::r_draw_model_shadowcaster->current.enabled); TT("Render black shadowcaster materials on xmodels.\nMostly seen/used on trees");

			// -----------------
			ImGui::title_with_seperator("Toolbar");
			ImGui::Checkbox("Draw FPS within the camera window", &dvars::gui_draw_fps->current.enabled);
			ImGui::Checkbox("Default Open - Toolbar", &dvars::gui_camera_toolbar_defaultopen->current.enabled);

		});
	}

	void child_textures()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_TEXTURES, height, _pref_child_bg_col, dvars::gui_border_color->current.vector, []
		{
			ImGui::title_with_seperator("General", false);
			ImGui::Checkbox("Draw scrollbar", &dvars::gui_texwnd_draw_scrollbar->current.enabled);
			ImGui::Checkbox("Show scroll position in percent", &dvars::gui_texwnd_draw_scrollpercent->current.enabled);

			ImGui::title_with_seperator("Surface Inspector");
			ImGui::Checkbox("Use new experimental surfaceinspector", &dvars::gui_use_new_surfinspector->current.enabled); TT(dvars::gui_use_new_surfinspector->description);
			ImGui::Checkbox("Incorporate Surface Inspector into property window##2", &dvars::gui_props_surfinspector->current.enabled); TT(dvars::gui_props_surfinspector->description);

			ImGui::title_with_seperator("Quality");

			// actual picmip vars the renderer is using (used to check if quality was changed)
			auto& r_picmip_renderer_val = *reinterpret_cast<int*>(0x14E6CF8);
			auto& r_picmip_spec_renderer_val = *reinterpret_cast<int*>(0x14E6D00);
			auto& r_picmip_bump_renderer_val = *reinterpret_cast<int*>(0x14E6CFC);
			
			// 3 = Low, 0 = Maximum

			const auto r_picmip = game::Dvar_FindVar("r_picmip");
			const auto r_picmip_spec = game::Dvar_FindVar("r_picmip_spec");
			const auto r_picmip_bump = game::Dvar_FindVar("r_picmip_bump");

			const char* quality_names[4] = { "Low", "Normal", "High", "Maximum" };
			const char* quality_col  = (r_picmip->current.integer >= 0 && r_picmip->current.integer <= 3) ? quality_names[3 - r_picmip->current.integer] : "Unknown";
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
			if(ImGui::Button("Apply / Reload Textures", ImVec2(ImGui::CalcItemWidth(), 0.0f))) //ImVec2(sliderint_size.x - 145.0f, 0.0f)))
			{
				// R_UpdateMipMap
				cdeclcall(void, 0x5139A0);
				game::R_ReloadImages();
			}
			ImGui::EndDisabled();
		});
	}

	void child_livelink()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_LIVELINK, height, _pref_child_bg_col, dvars::gui_border_color->current.vector, []
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

	
	void child_developer()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_DEVELOPER, height, _pref_child_bg_col, dvars::gui_border_color->current.vector, []
		{
			ImGui::Text("Fx Drawsurf Count: %d", components::effect_drawsurf_count);

			ImGui::BeginDisabled(!fx_system::ed_is_editor_effect_valid);
			{
				if (ImGui::Button("Fx Play"))
				{
					components::command::execute("fx_play");
				}

				ImGui::SameLine();

				if (ImGui::Button("Fx Repeat"))
				{
					if (components::effects::effect_is_repeating())
					{
						fx_system::ed_is_repeating = false;
						return;
					}

					if (components::effects::effect_is_playing())
					{
						fx_system::ed_is_repeating = true;
						return;
					}

					components::command::execute("fx_play");

					if (components::effects::effect_is_playing() && !components::effects::effect_is_repeating())
					{
						fx_system::ed_is_repeating = true;
					}
					else
					{
						fx_system::ed_is_repeating = false;
					}
				}

				ImGui::SameLine();

				if (ImGui::Button("Fx Stop"))
				{
					components::command::execute("fx_stop");
				}
			}
			ImGui::EndDisabled();

			ImGui::SameLine();

			if (ImGui::Button("Fx Load Def"))
			{
				components::command::execute("fx_load");
			}

			ImGui::SameLine();

			if (ImGui::Button("Toggle Show Tris"))
			{
				auto tris = game::Dvar_FindVar("r_showTris");
				if(tris)
				{
					dvars::set_int(tris, tris->current.integer ? 0 : 1);
				}
			}

			ImGui::DragFloat("Fx Timescale", &fx_system::ed_timescale, 0.005f, 0.001f, 50.0f);

			
			/*ImGui::DragInt("Int 01", &dev_num_01, 0.1f);
			ImGui::DragFloat3("Vec4 01", dev_vec_01, 0.1f);
			ImGui::ColorEdit4("Color 01", dev_color_01, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);*/

		});
	}
	
	void menu(ggui::imgui_context_menu& menu)
	{
		if(!menu.one_time_init)
		{
			register_child(CAT_GENERAL, child_general);
			register_child(CAT_GUI, child_gui);
			register_child(CAT_GRID, child_grid);
			register_child(CAT_CAMERA, child_camera);
			register_child(CAT_TEXTURES, child_textures);
			register_child(CAT_LIVELINK, child_livelink);

//#ifdef DEBUG
			register_child(CAT_DEVELOPER, child_developer);
//#endif
			
			menu.one_time_init = true;
		}
		
		const auto MIN_WINDOW_SIZE = ImVec2(800.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(800.0f, 600.0f);
        ggui::set_next_window_initial_pos_and_constraints(MIN_WINDOW_SIZE, INITIAL_WINDOW_SIZE);

		int stylevars = 0, stylecolors = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f)); stylevars++;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.08f, 0.31f)); stylecolors++;

		if (!ImGui::Begin("Preferences##window", &menu.menustate, ImGuiWindowFlags_NoCollapse))
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
				const bool is_selected = (_pref_child_current == i);
				if (ImGui::Selectable(child.first.c_str(), is_selected))
				{
					_pref_child_current = i;
					_pref_update_scroll = true;
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
	void on_prefsdialog_command()
	{
		components::gui::toggle(ggui::state.czwnd.m_preferences, 0, true);
	}

	void register_dvars()
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
		
	}
	
	void hooks()
	{
		// detour CMainFrame::OnPrefs (hotkey to open the original dialog)
		utils::hook::detour(0x426950, on_prefsdialog_command, HK_JUMP);
	}
}
