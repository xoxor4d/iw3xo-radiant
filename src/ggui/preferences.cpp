#include "std_include.hpp"

namespace ggui::preferences
{
	const std::string CAT_GUI = "Gui";
	const std::string CAT_GENERAL = "General";
	const std::string CAT_GRID = "Grid";
	const std::string CAT_CAMERA = "Camera";
	const std::string CAT_LIVELINK = "Live Link";
	
	std::vector<std::string> _pref_categories;
	static size_t _pref_current = 0;

	void init_categories()
	{
		// order
		_pref_categories.emplace_back(CAT_GENERAL);
		_pref_categories.emplace_back(CAT_GUI);
		_pref_categories.emplace_back(CAT_GRID);
		_pref_categories.emplace_back(CAT_CAMERA);
		_pref_categories.emplace_back(CAT_LIVELINK);
	}

	void child_gui()
	{
		if (_pref_categories[_pref_current] == CAT_GUI)
		{
			ImGui::BeginChild("##pref_child", ImVec2(0, 0), false);
			{
				ImGui::Checkbox("Show mousecursor origin within the menubar", &dvars::gui_menubar_show_mouseorigin->current.enabled);

				if(ImGui::Checkbox("Floating Toolbar", &dvars::gui_floating_toolbar->current.enabled)) {
					ggui::toolbar_reset = true;
				}

				ImGui::BeginDisabled(!dvars::gui_floating_toolbar->current.enabled);
				{
					ImGui::Checkbox("Resize Dockspace for floating toolbar", &dvars::gui_resize_dockspace->current.enabled);
				}
				ImGui::EndDisabled();

				if(ImGui::Button("Reset Dockspace")) {
					ggui::reset_dockspace = true;
				}
			}
			ImGui::EndChild();
		}
	}

	void child_general()
	{
		if (_pref_categories[_pref_current] == CAT_GENERAL)
		{
			ImGui::BeginChild("##pref_child", ImVec2(0, 0), false);
			{
				auto prefs = game::g_PrefsDlg();
				
				ImGui::Checkbox("Load last project on open", &prefs->m_bLoadLast);
				ImGui::Checkbox("Load last map on open", &prefs->m_bLoadLastMap);

				ImGui::Checkbox("Enable snapshots", &prefs->m_bSnapShots);
				ImGui::Checkbox("Enable autosave", &prefs->m_bAutoSave);
				ImGui::BeginDisabled(!prefs->m_bAutoSave);
				{
					ImGui::SliderInt("Autosave intervall", &prefs->m_nAutoSave, 1, 60);
				}
				ImGui::EndDisabled();

				ImGui::Checkbox("Remove tiny brushes on map load", &prefs->m_bCleanTinyBrushes); TT("Not a saved setting!");
				ImGui::BeginDisabled(!prefs->m_bCleanTinyBrushes);
				{
					ImGui::DragFloat("Tiny brush threshold", &prefs->m_fTinySize, 0.005f, 0, 16.0f, "%.2f");
					TT("Brush will be removed if ANY side is below the threshold. Not a saved setting!");
				}
				ImGui::EndDisabled();
				
				ImGui::Checkbox("Loose changes dialog on exit", &prefs->loose_changes);

				ImGui::Checkbox("Linking entities keeps selection", &prefs->linking_keeps_selection);

				if(ImGui::SliderInt("Undo levels", &prefs->m_nUndoLevels, 16, 512))
				{
					game::g_undoMaxSize = prefs->m_nUndoLevels;
				}

				ImGui::DragFloat("Model origin size", &prefs->model_origin_size, 0.1f, 0, 100, "%.1f");
				ImGui::DragFloat("Prefab origin size", &prefs->prefab_origin_size, 0.1f, 0, 100, "%.1f");

				ImGui::DragInt("Model planting scale base", &prefs->scale_base, 0.1f, 1, 1000);
				ImGui::DragInt("Model planting scale range", &prefs->scale_range, 0.1f, 1, 1000);

				ImGui::DragInt("Tolerant weld distance", &prefs->tolerant_weld, 0.1f, 1, 128);
				ImGui::DragInt("Splay distance", &prefs->splay, 0.1f, 1, 128); TT("Distance used to push 2 or more selected vertices apart when using splay (default ALT+W)");

				ImGui::DragInt("Vehicle arrow time", &prefs->vehicle_arrow_time, 0.1f, 0, 10000);
				ImGui::DragInt("Vehicle arrow size", &prefs->vehicle_arrow_size, 0.1f, 0, 1000);
				
				//ImGui::Checkbox("Entities use \"_color\" value", &prefs->m_bColoredEnts); // they always do
				
				// disables setting d_gridsize_float when using menubar->grid 1 to 512 but has no effect on actual texture snap
				//ImGui::Checkbox("Snap textures to grid", &prefs->m_bSnapTToGrid);
				//ImGui::Text("Texture Snap to Grid Size: %.2f", game::g_qeglobals->d_savedinfo.d_gridsize_float);

				// ini
				/*std::string aaa = std::to_string(dvars::radiant_livePort->current.integer);
				if (ImGui::InputText("aaa", &aaa, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					game::printf_to_console(aaa.c_str());
				}*/
			}
			ImGui::EndChild();
		}
	}

	void child_grid()
	{
		if (_pref_categories[_pref_current] == CAT_GRID)
		{
			ImGui::BeginChild("##pref_child", ImVec2(0, 0), false);
			{
				auto prefs = game::g_PrefsDlg();

				ImGui::Checkbox("Zoom to cursor", &dvars::grid_zoom_to_cursor->current.enabled);
				ImGui::Checkbox("Alt always drags when multiple brushes are selected", &prefs->m_bALTEdge); TT("Holding ALT and having selected multiple brushes wont extrude, but instead move brushes when clicked outside");
				ImGui::Checkbox("Enable right-click context menu", &prefs->m_bRightClick); TT("Org: Right click to drop entities (really wrong)");

				ImGui::Checkbox("Disable grid snapping", &prefs->m_bNoClamp); TT("Org: Don't clamp plane points");
				
				ImGui::Checkbox("Show sizing info", &prefs->m_bSizePaint);
				ImGui::Checkbox("Thick selection lines", &prefs->thick_selection_lines);
				ImGui::Checkbox("Fast view dragging", &prefs->fast_2d_view_dragging); TT("Hides static models when moving the grid");

				ImGui::Checkbox("Texture brushes", &prefs->texture_brush_2d);
				ImGui::Checkbox("Texture meshes", &prefs->texture_mesh_2d);
			}
			ImGui::EndChild();
		}
	}
	
	void child_camera()
	{
		if (_pref_categories[_pref_current] == CAT_CAMERA)
		{
			ImGui::BeginChild("##pref_child", ImVec2(0, 0), false);
			{
				auto prefs = game::g_PrefsDlg();

				ImGui::SliderInt("Camera mode", &prefs->camera_mode, 0, 2);
				ImGui::SliderInt("Camera speed", &prefs->m_nMoveSpeed, 10, 5000);
				ImGui::SliderInt("Camera angle speed", &prefs->m_nAngleSpeed, 1, 1000);
				ImGui::DragFloat("Field of view", &prefs->camera_fov, 0.1f, 1.0f, 180.0f, "%.1f");
				ImGui::Checkbox("Cull sky when using cubic clipping", &prefs->b_mCullSky);
				ImGui::Checkbox("Enable right-click context menu", &prefs->m_bRightClick); TT("Org: Right click to drop entities (really wrong)");
			}
			ImGui::EndChild();
		}
	}

	

	void child_livelink()
	{
		if (_pref_categories[_pref_current] == CAT_LIVELINK)
		{
			ImGui::BeginChild("##pref_child", ImVec2(0, 0), false);
			{
				ImGui::Checkbox("Enable live-link", &dvars::radiant_live->current.enabled);
				ImGui::Checkbox("Enable debug prints", &dvars::radiant_liveDebug->current.enabled);
				ImGui::DragInt("Port", &dvars::radiant_livePort->current.integer, 0.5f, 1, 99999);
			}
			ImGui::EndChild();
		}
	}
	
	void menu(ggui::imgui_context_menu& menu)
	{
		if(!menu.one_time_init)
		{
			init_categories();
			menu.one_time_init = true;
		}
		
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(600.0f, 800.0f);
        ggui::set_next_window_initial_pos_and_constraints(MIN_WINDOW_SIZE, INITIAL_WINDOW_SIZE);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		ImGui::Begin("Preferences##window", &menu.menustate, ImGuiWindowFlags_NoCollapse);

		if (ImGui::BeginListBox("##pref_cat", ImVec2(180.0f, ImGui::GetContentRegionAvail().y)))
		{
			for (size_t i = 0; i < _pref_categories.size(); i++)
			{
				const bool is_selected = (_pref_current == i);
				if (ImGui::Selectable(_pref_categories[i].c_str(), is_selected)) {
					_pref_current = i;
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}

		ImGui::SameLine();

		child_gui();
		child_general();
		child_grid();
		child_camera();
		child_livelink();

		ImGui::PopStyleVar();
		ImGui::End();
	}
}
