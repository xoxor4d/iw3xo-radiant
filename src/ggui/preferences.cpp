#include "std_include.hpp"

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


			// -----------------
			ImGui::title_with_seperator("Property Editor & Toolbox");
			ImGui::Checkbox("Default Open - Classlist", &dvars::gui_props_classlist_defaultopen->current.enabled); TT(dvars::gui_props_classlist_defaultopen->description);
			ImGui::Checkbox("Default Open - Spawnflags", &dvars::gui_props_spawnflags_defaultopen->current.enabled); TT(dvars::gui_props_spawnflags_defaultopen->description);
			ImGui::Checkbox("Default Open - Comments", &dvars::gui_props_comments_defaultopen->current.enabled); TT(dvars::gui_props_comments_defaultopen->description);

			ImGui::Checkbox("Integrate camera toolbar into toolbox", &dvars::gui_toolbox_integrate_cam_toolbar->current.enabled); TT(dvars::gui_toolbox_integrate_cam_toolbar->description);
			ImGui::Checkbox("Integrate entity-properties into toolbox", &dvars::gui_props_toolbox->current.enabled); TT(dvars::gui_props_toolbox->description);
			ImGui::Checkbox("Integrate filter window into toolbox", &dvars::gui_toolbox_integrate_filter->current.enabled); TT(dvars::gui_toolbox_integrate_filter->description);

			const char* incorp_surf_inspector_strings[4] = { "None", "Entity Properties", "Toolbox" };
			if (ImGui::SliderInt("Integrate surface-inspector into", &dvars::gui_props_surfinspector->current.integer, 0, 2, incorp_surf_inspector_strings[dvars::gui_props_surfinspector->current.integer]))
			{
				dvars::gui_props_surfinspector->modified = true;
			} TT("Integration into the entity window only works if the entity window is not integrated into the toolbox");
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

			SPACING(0.0f, 6.0f);

			ImGui::Checkbox("Enable 'lose changes dialog' on exit", &prefs->loose_changes);
			ImGui::Checkbox("Enable snapshots", &prefs->m_bSnapShots);

			ImGui::Checkbox("Enable autosave", &prefs->m_bAutoSave);
			ImGui::BeginDisabled(!prefs->m_bAutoSave);
			{
				ImGui::SliderInt("Autosave interval", &prefs->m_nAutoSave, 1, 60);
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
				prefs->m_nUndoLevels = ImClamp(prefs->m_nUndoLevels, 16, 512);
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

			ImGui::title_with_seperator("Gui", false);
			if (ImGui::Checkbox("Use new grid context menu", &dvars::gui_use_new_context_menu->current.enabled))
			{
				game::g_PrefsDlg()->m_bRightClick = !dvars::gui_use_new_context_menu->current.enabled;
			}

			ImGui::Checkbox("Show 'Select' menu in grid context menu", &dvars::gui_grid_context_show_select->current.enabled);


			// -----------------
			ImGui::title_with_seperator("Mouse", false);
			ImGui::Checkbox("Zoom to cursor", &dvars::grid_zoom_to_cursor->current.enabled);
			ImGui::Checkbox("Disable grid snapping", &prefs->m_bNoClamp); TT("Org: Don't clamp plane points");


			// -----------------
			ImGui::title_with_seperator("Dragging");
			ImGui::Checkbox("Alt always drags when multiple brushes are selected", &prefs->m_bALTEdge); TT("Holding ALT and having selected multiple brushes wont extrude, but instead move brushes when clicked outside");
			ImGui::Checkbox("Fast view dragging", &prefs->fast_2d_view_dragging); TT("Hides static models when moving the grid");


			// -----------------
			ImGui::title_with_seperator("Visuals");
			ImGui::Checkbox("Show mousecursor origin", &dvars::gui_menubar_show_mouseorigin->current.enabled);
			ImGui::Checkbox("Show sizing info", &prefs->m_bSizePaint);
			ImGui::Checkbox("Draw edge and grid block coordinates", &dvars::grid_draw_edge_coordinates->current.enabled);
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

			if (ImGui::DragFloat("Field of view", &prefs->camera_fov, 0.1f, 10.0f, 160.0f, "%.1f"))
			{
				prefs->camera_fov = ImClamp(prefs->camera_fov, 10.0f, 160.0f);
			}


			// -----------------
			ImGui::title_with_seperator("Mouse");
			ImGui::SliderInt("Camera speed", &prefs->m_nMoveSpeed, 10, 5000);
			ImGui::SliderInt("Camera angle speed", &prefs->m_nAngleSpeed, 1, 1000);
			if (imgui::SliderFloat("Camera angle speed (physx movement)", &dvars::physx_camera_sensitivity->current.value, 10.0f, 1000.0f, "%.0f"))
			{
				dvars::physx_camera_sensitivity->current.value = ImClamp(dvars::physx_camera_sensitivity->current.value, 10.0f, 1000.0f);
			}


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
			ImGui::Checkbox("Merge BSP/Radiant buttons", &dvars::gui_camera_toolbar_merge_bsp_buttons->current.enabled); TT(dvars::gui_camera_toolbar_merge_bsp_buttons->description);

		});
	}

	void preferences_dialog::child_effects_browser()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_EFFECTS_BROWSER, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			game::dvar_s* dvar = nullptr;
			imgui::title_with_seperator("Grid", false);
			
			dvar = dvars::fx_browser_grid_sections;
			if (imgui::DragInt("Grid Sections", &dvar->current.integer, 0.1f, dvar->domain.integer.min, dvar->domain.integer.max))
			{
				dvar->current.integer = ImClamp(dvar->current.integer, dvar->domain.integer.min, dvar->domain.integer.max);
			}

			dvar = dvars::fx_browser_grid_scale;
			if (imgui::DragInt("Grid Scale", &dvar->current.integer, 0.1f, dvar->domain.integer.min, dvar->domain.integer.max))
			{
				dvar->current.integer = ImClamp(dvar->current.integer, dvar->domain.integer.min, dvar->domain.integer.max);
			}

			imgui::ColorEdit4("Grid Color", dvars::fx_browser_grid_color->current.vector, ImGuiColorEditFlags_Float);

			dvar = dvars::fx_browser_grid_line_width;
			if (imgui::DragInt("Line Width", &dvar->current.integer, 0.1f, dvar->domain.integer.min, dvar->domain.integer.max))
			{
				dvar->current.integer = ImClamp(dvar->current.integer, dvar->domain.integer.min, dvar->domain.integer.max);
			}

			dvar = dvars::fx_browser_grid_font_scale;
			if (imgui::DragFloat("Font Scale", &dvar->current.value, 0.01f, dvar->domain.value.min, dvar->domain.value.max))
			{
				dvar->current.value = ImClamp(dvar->current.value, dvar->domain.value.min, dvar->domain.value.max);
			}

			imgui::ColorEdit4("Font Color", dvars::fx_browser_grid_font_color->current.vector, ImGuiColorEditFlags_Float);

			if (imgui::Button("Regenerate Grid"))
			{
				cfxwnd::get()->m_grid_generated = false;
			}

			imgui::title_with_seperator("Rendering");

			dvar = dvars::fx_browser_use_camera_for_distortion;
			imgui::Checkbox("Use Camera Image for Distortion", &dvar->current.enabled);
			TT(dvar->description);
		});
	}


	void preferences_dialog::child_textures()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_TEXTURES, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			/*ImGui::title_with_seperator("General", false);
			ImGui::Checkbox("Draw scrollbar", &dvars::gui_texwnd_draw_scrollbar->current.enabled);
			ImGui::Checkbox("Show scroll position in percent", &dvars::gui_texwnd_draw_scrollpercent->current.enabled);*/

			ImGui::title_with_seperator("Quality", false);

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

	void preferences_dialog::child_renderer_bsp()
	{
		static float height = 0.0f;
		height = pref_child_lambda(CAT_RENDERER, height, m_child_bg_col, dvars::gui_border_color->current.vector, [this]
		{
			const float second_column = ImGui::GetWindowContentRegionWidth() * 0.5f;

			ImGui::title_with_seperator("General", false);

			ImGui::DvarBool_External("Draw water", "r_drawWater");
			ImGui::SameLine(second_column);
			ImGui::DvarBool_External("Enable normalmapping", "r_normalMap");
			
			ImGui::DvarBool_External("Display model triangle count", "r_showTriCounts");
			ImGui::SameLine(second_column);
			ImGui::DvarBool_External("Display model surface count", "r_showSurfCounts");

			
			ImGui::DvarBool_External("Display model vertex count", "r_showVertCounts");
			ImGui::SameLine(second_column);
			ImGui::DvarBool_External("Display model names", "r_showSModelNames");

			SPACING(0.0f, 6.0f);

			ImGui::DvarInt_External("Display aabb Trees", "r_showaabbtrees");
			ImGui::DvarInt_External("Display portals", "r_showportals");


			// -----------------
			ImGui::title_with_seperator("BSP Only");

			ImGui::DvarBool_External("Load and draw BSP entities", "bsp_load_entities");
			ImGui::Checkbox("Export reflections as tga files", &dvars::r_reflectionprobe_export_tga->current.enabled); TT(dvars::r_reflectionprobe_export_tga->description);
			ImGui::Checkbox("Automatically compile reflections when building bsp", &dvars::bsp_gen_reflections_on_compile->current.enabled); TT(dvars::bsp_gen_reflections_on_compile->description);

			SPACING(0.0f, 4.0f);

			ImGui::DvarBool_External("Draw detail", "r_detail");
			ImGui::SameLine(second_column);
			ImGui::DvarBool_External("Draw decals", "r_drawDecals");

			ImGui::DvarBool_External("Draw world", "r_drawWorld");
			ImGui::SameLine(second_column);
			ImGui::DvarBool_External("Draw static models", "r_drawsmodels");

			ImGui::DvarBool_External("Draw xmodels", "r_drawXModels");
			ImGui::SameLine(second_column);
			ImGui::DvarBool_External("Draw entities", "r_drawEntities");

			ImGui::DvarBool_External("Enable fullbright", "r_fullbright");
			ImGui::SameLine(second_column);
			ImGui::DvarBool_External("Enable specular", "r_specular");

			SPACING(0.0f, 6.0f);

			ImGui::DvarBool_External("Enable shadows", "sm_enable");
			ImGui::DvarFloat_External("Shadow samplesize", "sm_sunSampleSizeNear");

			SPACING(0.0f, 6.0f);

			ImGui::DvarEnum_External("Debugshader", "r_debugshader");
			ImGui::DvarEnum_External("Colormap", "r_colorMap");
			ImGui::DvarEnum_External("Specularmap", "r_specularmap");
			ImGui::DvarEnum_External("Lightmap", "r_lightmap");
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

			ImGui::DragFloat("xywnd scale", &cmainframe::activewnd->m_pXYWnd->m_fScale, 0.01f, 0.01f, 100.0f);

			SPACING(0.0f, 4.0f);

			ImGui::Text("camera - cambuttonstate: %d", cmainframe::activewnd->m_pCamWnd->m_nCambuttonstate);
			ImGui::Text("camera - cursor_visible: %d", cmainframe::activewnd->m_pCamWnd->cursor_visible);
			ImGui::Text("camera - window hovered: %d", GET_GUI(ggui::camera_dialog)->rtt_is_hovered());

			SPACING(0.0f, 4.0f);

			game::qtexture_s* texwndglob_textures = reinterpret_cast<game::qtexture_s*>(*(DWORD*)0x25E79A8);

			const char* mat_names[4] = { "ace_wall_stone1_plain", "ch_factory_floorgrate", "ch_grass_01", "me_wallpaper2" };

			if (ImGui::Button("Load tex favs"))
			{
				ctexwnd::load_favourites();
			}

			if(ImGui::Button("Texture filter test"))
			{
				// hide all
				game::qtexture_s* tex = texwndglob_textures;
				for (; tex; tex = tex->prev)
				{
					tex->is_in_use = false;
				}

				tex = texwndglob_textures;
				for (; tex; tex = tex->prev)
				{
					for(auto m = 0; m < IM_ARRAYSIZE(mat_names); m++)
					{
						if (utils::string_equals(tex->name, mat_names[m]))
						{
							tex->is_in_use = true;
						}
					}
				}
			}

			SPACING(0.0f, 4.0f);

			if(ImGui::Button("Toast Success"))
			{
				ImGui::InsertNotification(
					{ ImGuiToastType_Success, 2000, "Hello World! This is a success! %s", ICON_FA_APPLE_ALT});
			}

			ImGui::SameLine();
			if (ImGui::Button("Toast Info"))
			{
				ImGuiToast toast(ImGuiToastType_Info, 2000);
				toast.set_title("Autosave");

				ImGui::InsertNotification(toast);
			}

			ImGui::SameLine();
			if (ImGui::Button("Print Error"))
			{
				game::printf_to_console("[ERR] This is an error message!");
			}

			static bool frequent_toast_test = false;
			imgui::Checkbox("Frequent Toast Test", &frequent_toast_test);

			if (frequent_toast_test)
			{
				game::printf_to_console("[ERR] Test Toast");
			}

			//if (imgui::Button("Mesh Test"))
			//{
			//	struct GfxVertexBuffer
			//	{
			//		IDirect3DVertexBuffer9* vb;
			//	};

			//	if (game::is_single_brush_selected())
			//	{
			//		auto x = game::g_selected_brushes();
			//		game::faceVis_s* vis = x->faces;

			//		GfxVertexBuffer vb = {};
			//		WORD first_index = 0;

			//		//Editor_GetVertexBufferAndIndex
			//		utils::hook::call<void(__cdecl)(int low16_firstIndex__high16_vbOffset, GfxVertexBuffer* vb, WORD* firstIndex)>(0x51CCE0)(vis->visArray->vertHandle, &vb, &first_index);

			//		int yy = 0;
			//	}
			//}

			// -----------------
			ImGui::title_with_seperator("Movement Vars");

			const auto cct = components::physx_impl::get()->m_cct_camera;
			const auto cam = cmainframe::activewnd->m_pCamWnd->camera;

			ImGui::DragFloat("Gravity", &cct->m_gravity, 0.1f, -2000.0f, 2000.0f, "%.1f");
			ImGui::DragFloat("Friction", &cct->m_friction, 0.01f, 0.0f, 100.0f, "%.1f");
			ImGui::DragFloat("JumpSpeed", &cct->m_jump_velocity, 0.01f, 0.0f, 1000.0f, "%.1f");

			ImGui::Separator();
			imgui::Spacing();

			imgui::PushFontFromIndex(BOLD_18PX);
			ImGui::Text("Velocity: %.2f, %.2f, %.2f", cct->m_player_velocity[0], cct->m_player_velocity[1], cct->m_player_velocity[2]);

			const PxVec3 origin (toVec3(cct->get_controller()->get_foot_position()));
			ImGui::Text("Origin: %.2f, %.2f, %.5f", origin.x, origin.y, origin.z);
			imgui::Text("Cam Forward: %.2f, %.2f, %.2f", cam.forward[0], cam.forward[1], cam.forward[2]);
			ImGui::Text("Speed: %.2f", cct->m_player_speed);
			imgui::Text("Msec: %.0f", components::renderer::get()->m_cam_msec * 1000.0f);

			imgui::Spacing();

			imgui::Text("walking: %s", cct->m_walking ? "true" : "false");
			imgui::Text("on_ground: %s", cct->m_on_ground ? "true" : "false");
			imgui::Text("is_jumping: %s", cct->m_is_jumping ? "true" : "false");
			imgui::Text("bounce: %s", cct->m_bounce ? "true" : "false");
			imgui::Text("ground_type: %s", cct->m_ground_type == 0 ? "NONE" : cct->m_ground_type == 1 ? "GROUND" : "SLOPE");
			imgui::Text("ground normal: %.2f, %.2f, %.2f", cct->m_groundtrace.normal.x, cct->m_groundtrace.normal.y, cct->m_groundtrace.normal.z);

			imgui::PopFont();


			// -----------------
			ImGui::title_with_seperator("Free vars");

			ImGui::DragInt("Integer01", &dev_num_01, 0.1f);
			ImGui::DragInt("Integer02", &dev_num_02, 0.1f);
			ImGui::Checkbox("Bool01", &dev_bool_01);

			ImGui::DragFloat3("Vec4 01", dev_vec_01, 25.0f);
			ImGui::ColorEdit4("Color 01", dev_color_01, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit4("Color 02", dev_color_02, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit4("Color 03", dev_color_03, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		});
	}

	bool preferences_dialog::gui()
	{
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
			return false;
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
			return false;
		}

		SPACING(0.0f, 0.0f);

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

		return true;
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

		dvars::gui_camera_toolbar_merge_bsp_buttons = dvars::register_bool(
			/* name		*/ "gui_camera_toolbar_merge_bsp_buttons",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Enable: merge 'hide/show bsp/radiant' buttons to toggle between bsp and radiant with a single click.\nDisable: split buttons");
	}

	void preferences_dialog::on_init()
	{
		register_child(CAT_GENERAL, std::bind(&preferences_dialog::child_general, this));
		register_child(CAT_GUI, std::bind(&preferences_dialog::child_gui, this));
		register_child(CAT_GRID, std::bind(&preferences_dialog::child_grid, this));
		register_child(CAT_CAMERA, std::bind(&preferences_dialog::child_camera, this));
		register_child(CAT_EFFECTS_BROWSER, std::bind(&preferences_dialog::child_effects_browser, this));
		register_child(CAT_TEXTURES, std::bind(&preferences_dialog::child_textures, this));
		register_child(CAT_RENDERER, std::bind(&preferences_dialog::child_renderer_bsp, this));
		register_child(CAT_LIVELINK, std::bind(&preferences_dialog::child_livelink, this));
		register_child(CAT_DEVELOPER, std::bind(&preferences_dialog::child_developer, this));

	}


	REGISTER_GUI(preferences_dialog);
}

