#include "std_include.hpp"

namespace ggui
{
	void menubar_dialog::set_grid_size(const xywnd::E_GRID_SIZES size)
	{
		game::g_qeglobals->d_gridsize = size;
		if (game::g_PrefsDlg()->m_bSnapTToGrid)
		{
			game::g_qeglobals->d_savedinfo.d_gridsize = xywnd::GRID_SIZES[size];
		}

		mainframe_thiscall(void, 0x428A00); // CMainFrame::SetGridStatus
		game::g_nUpdateBits |= W_Z | W_XY;
	}

	void menubar_dialog::set_render_method(const game::RENDER_METHOD_E meth)
	{
		switch (meth)
		{
		case game::RM_WIREFRAME: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80DE; break;
		case game::RM_FULLBRIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80DF; break;
		case game::RM_NORMALFAKELIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E0; break;
		case game::RM_VIEWFAKELIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E1; break;
		case game::RM_CASETEXTURES: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E2; break;
		}

		if (cmainframe::activewnd->m_pCamWnd->camera.draw_mode != meth)
		{
			cmainframe::activewnd->m_pCamWnd->camera.draw_mode = meth;
			game::g_nUpdateBits |= W_CAMERA;
		}
	}

	void menubar_dialog::set_texture_resolution(int picmip)
	{
		game::g_qeglobals->d_savedinfo.d_picmip = picmip;

		// CMainFrame::PicMip
		mainframe_stdcall(void, 0x420860); // sets the stock menu ..

		// R_UpdateMipMap
		cdeclcall(void, 0x5139A0);

		// R_ReloadImages
		game::R_ReloadImages();

		game::g_nUpdateBits = -1;
	}

	void menubar_dialog::menubar()
	{
		const auto prefs = game::g_PrefsDlg();

		// *
		// menu bar within dockspace

		ggui::context_menu_style_begin();

		if (ImGui::BeginMenuBar())
		{
			ImGui::BeginGroup();

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Map")) {
					cdeclcall(void, 0x423AA0); //cmainframe::OnFileNew
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Open ..", ggui::hotkey_dialog::get_hotkey_for_command("FileOpen").c_str())) 
				{
					// logic :: ggui::file_dialog_frame
					if(dvars::gui_use_new_filedialog->current.enabled)
					{
						const auto egui = GET_GUI(ggui::entity_dialog);
						const std::string path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");

						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(ggui::FILE_DIALOG_HANDLER::MAP_LOAD);
						file->set_file_op_type(file_dialog::FileDialogType::OpenFile);
						file->set_file_ext(".map");
						file->open();
					}
					else
					{
						cdeclcall(void, 0x423AE0); //cmainframe::OnFileOpen
					}
				}

				if (ImGui::BeginMenu("Open Recent", game::g_qeglobals->d_lpMruMenu->wNbItemFill))
				{
					// itemfill = amount of strings inside "lpMRU"
					for (std::uint16_t i = 0; i < game::g_qeglobals->d_lpMruMenu->wNbItemFill; i++)
					{
						// "lpMRU" strings are 128chars wide
						const char* mru_item_str = game::g_qeglobals->d_lpMruMenu->lpMRU + (game::g_qeglobals->d_lpMruMenu->wMaxSizeLruItem * i);

						if (mru_item_str && ImGui::MenuItem(mru_item_str))
						{
							typedef  BOOL(__thiscall* CMainFrame_OnMru_t)(cmainframe* pThis, std::uint16_t nID);
							CMainFrame_OnMru_t CMainFrame_OnMru = reinterpret_cast<CMainFrame_OnMru_t>(0x423FE0);

							//mru id's start at 8000 (wIdMru) + 1 for the first item as DoMru() subtracts 1
							CMainFrame_OnMru(cmainframe::activewnd, i + 1 + game::g_qeglobals->d_lpMruMenu->wIdMru);
						}
					}

					ImGui::EndMenu(); // Open Recent
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Save", ggui::hotkey_dialog::get_hotkey_for_command("FileSave").c_str())) {
					cdeclcall(void, 0x423B80); //cmainframe::OnFileSave
				}

				if (ImGui::MenuItem("Save As ..")) 
				{
					// logic :: ggui::file_dialog_frame
					if (dvars::gui_use_new_filedialog->current.enabled)
					{
						const auto egui = GET_GUI(ggui::entity_dialog);
						const std::string path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");

						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(ggui::FILE_DIALOG_HANDLER::MAP_SAVE);
						file->set_file_op_type(file_dialog::FileDialogType::SaveFile);
						file->set_file_ext(".map");
						file->open();
					}
					else
					{
						cdeclcall(void, 0x423BC0); //cmainframe::OnFileSaveas
					}
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Generate File"))
				{
					if (ImGui::MenuItem("Save Selected"))
					{
						// logic :: ggui::file_dialog_frame
						if (dvars::gui_use_new_filedialog->current.enabled)
						{
							const auto egui = GET_GUI(ggui::entity_dialog);
							const std::string path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");

							const auto file = GET_GUI(ggui::file_dialog);
							file->set_default_path(path_str);
							file->set_file_handler(ggui::FILE_DIALOG_HANDLER::MAP_EXPORT);
							file->set_file_op_type(file_dialog::FileDialogType::SaveFile);
							file->set_file_ext(".map");
							file->open();
						}
						else
						{
							mainframe_thiscall(void, 0x4293A0); //cmainframe::OnFileExportmap
						}
					}

					// is this even remotely useful?
					if (ImGui::MenuItem("Save Region", "", nullptr, game::g_region_active)) {
						mainframe_cdeclcall(void, 0x429020); //cmainframe::OnFileSaveregion
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Createfx"))
					{
						components::effects::generate_createfx();

					} TT("Generate createfx files for current map.\n (bin\\IW3xRadiant\\createfx)");
					
					if (ImGui::MenuItem("Pointfile")) {
						mainframe_cdeclcall(void, 0x423B20); //cmainframe::OnPointfileOpen
					}

					if (ImGui::MenuItem("Errorfile")) {
						mainframe_cdeclcall(void, 0x423B40); //cmainframe::OnErrorFile
					}

					ImGui::EndMenu(); // Misc
				}

				if (ImGui::MenuItem("Project Settings")) {
					mainframe_cdeclcall(void, 0x428DE0); //cmainframe::OnFileProjectsettings
				}

				//if (ImGui::BeginMenu("Project Settings"))
				//{
				//	if (ImGui::MenuItem("New Project")) {
				//		mainframe_cdeclcall(void, 0x426E80); //cmainframe::OnFileNewproject
				//	}

				//	if (ImGui::MenuItem("Set Startup Project")) {
				//		mainframe_cdeclcall(void, 0x427010); //cmainframe::OnSetStartupProject
				//	}

				//	if (ImGui::MenuItem("Project Settings")) {
				//		mainframe_cdeclcall(void, 0x428DE0); //cmainframe::OnFileProjectsettings
				//	}

				//	ImGui::EndMenu(); // Project Settings
				//}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Exit", ggui::hotkey_dialog::get_hotkey_for_command("Quit").c_str()))
				{
					PostMessage(cmainframe::activewnd->GetWindow(), WM_CLOSE, 0, 0L);
				}

				ImGui::EndMenu(); // File
			}

			// ----------------------------

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", ggui::hotkey_dialog::get_hotkey_for_command("Undo").c_str(), nullptr, game::g_lastundo() && game::g_lastundo()->done)) {
					cdeclcall(void, 0x428730); //cmainframe::OnEditUndo / OnUpdateEditUndo
				}

				if (ImGui::MenuItem("Redo", ggui::hotkey_dialog::get_hotkey_for_command("Redo").c_str(), nullptr, game::g_lastredo())) {
					cdeclcall(void, 0x428740); //cmainframe::OnEditRedo / OnUpdateEditRedo
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Copy", ggui::hotkey_dialog::get_hotkey_for_command("Copy").c_str())) {
					mainframe_thiscall(void, 0x4286B0); //cmainframe::OnEditCopybrush
				}

				if (ImGui::MenuItem("Paste", ggui::hotkey_dialog::get_hotkey_for_command("Paste").c_str())) {
					mainframe_thiscall(void, 0x4286D0); //cmainframe::OnEditPastebrush
				}

				if (ImGui::MenuItem("Delete", ggui::hotkey_dialog::get_hotkey_for_command("DeleteSelection").c_str())) {
					cdeclcall(void, 0x425690); //cmainframe::OnSelectionDelete
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Map Info", ggui::hotkey_dialog::get_hotkey_for_command("MapInfo").c_str())) {
					cdeclcall(void, 0x426C60); //cmainframe::OnEditMapinfo
				}

				if (ImGui::MenuItem("Entity Info")) {
					//cdeclcall(void, 0x426D40); //cmainframe::OnEditEntityinfo
					GET_GUI(ggui::entity_info)->toggle();
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Enter Prefab", ggui::hotkey_dialog::get_hotkey_for_command("EnterPrefab").c_str())) {
					cdeclcall(void, 0x42BF70); //cmainframe::OnPrefabEnter
				}

				if (ImGui::MenuItem("Leave Prefab", ggui::hotkey_dialog::get_hotkey_for_command("LeavePrefab").c_str())) {
					cdeclcall(void, 0x42BF80); //cmainframe::OnPrefabLeave
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Edit Colors ..")) {
					GET_GUI(ggui::gui_colors_dialog)->toggle();
				}

				if (ImGui::MenuItem("Edit Toolbar ..")) {
					GET_GUI(ggui::toolbar_edit_dialog)->toggle();
				}

				if (ImGui::MenuItem("Edit Hotkeys ..")) {
					GET_GUI(hotkey_dialog)->toggle();
				}

				if (ImGui::MenuItem("Preferences", ggui::hotkey_dialog::get_hotkey_for_command("Preferences").c_str())) {
					GET_GUI(ggui::preferences_dialog)->toggle();
				}

				ImGui::EndMenu(); // Edit
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::BeginMenu("Toggle"))
				{
					if (ImGui::MenuItem("Game View", ggui::hotkey_dialog::get_hotkey_for_command("xo_gameview").c_str(), dvars::radiant_gameview->current.enabled)) {
						components::gameview::p_this->set_state(!dvars::radiant_gameview->current.enabled);
					}

					if (ImGui::MenuItem("Console", ggui::hotkey_dialog::get_hotkey_for_command("ViewConsole").c_str())) {
						GET_GUI(console_dialog)->toggle();
					}

					if (ImGui::MenuItem("Filter", ggui::hotkey_dialog::get_hotkey_for_command("ViewFilters").c_str())) {
						GET_GUI(filter_dialog)->toggle();
					}

					if (ImGui::MenuItem("Entity", ggui::hotkey_dialog::get_hotkey_for_command("ViewEntityInfo").c_str())) {
						GET_GUI(ggui::entity_dialog)->toggle();
					}

					if (ImGui::MenuItem("Mesh Painter")) {
						GET_GUI(ggui::mesh_painter_dialog)->toggle();
					}

					if (ImGui::MenuItem("Effects Browser")) {
						GET_GUI(ggui::effects_browser)->toggle();
					}

					if (ImGui::MenuItem("Surface Inspector", ggui::hotkey_dialog::get_hotkey_for_command("SurfaceInspector").c_str()))
					{
						if (dvars::gui_use_new_surfinspector && dvars::gui_use_new_surfinspector->current.enabled)
						{
							if (dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.integer)
							{
								GET_GUI(ggui::entity_dialog)->toggle();
							}
							else
							{
								GET_GUI(ggui::surface_dialog)->toggle();
							}
						}
						else
						{
							// SurfaceInspector::DoSurface
							cdeclcall(void, 0x4585D0);
						}
					}

					if (ImGui::MenuItem("Layers", ggui::hotkey_dialog::get_hotkey_for_command("ToggleLayers").c_str())) {
						GET_GUI(ggui::layer_dialog)->toggle();
					}

					if (ImGui::MenuItem("Textures", ggui::hotkey_dialog::get_hotkey_for_command("ViewTextures").c_str())) {
						GET_GUI(ggui::texture_dialog)->toggle();
					}

					if (ImGui::MenuItem("Model Browser", ggui::hotkey_dialog::get_hotkey_for_command("xo_modelselector").c_str())) {
						GET_GUI(ggui::modelselector_dialog)->toggle();
					}

					if (ImGui::MenuItem("Prefab Browser", hotkey_dialog::get_hotkey_for_command("prefab_browser").c_str())) {
						GET_GUI(ggui::prefab_preview_dialog)->toggle();
					}

					if (ImGui::MenuItem("Toolbox", ggui::hotkey_dialog::get_hotkey_for_command("toggle_toolbox").c_str())) {
						GET_GUI(ggui::toolbox_dialog)->toggle();
					}

					if (ImGui::MenuItem("ImGui Demo")) {
						ggui::m_demo_menu_state = !ggui::m_demo_menu_state;
					}

					SEPERATORV(0.0f);

					if (ImGui::BeginMenu("Original Windows"))
					{
#ifdef DEBUG
						if (ImGui::MenuItem("Menubar", 0, ggui::mainframe_menubar_enabled))
						{
							if (!ggui::mainframe_menubar_enabled)
							{
								components::command::execute("menubar_show");
							}
							else
							{
								components::command::execute("menubar_hide");
							}

							game::CPrefsDlg_SavePrefs();
						}

						if (ImGui::MenuItem("Toolbar", 0, dvars::mainframe_show_toolbar->current.enabled))
						{
							typedef void(__thiscall* CFrameWnd_ShowControlBar_t)(CFrameWnd*, CControlBar*, BOOL bShow, BOOL bDelay);
							const auto CFrameWnd_ShowControlBar = reinterpret_cast<CFrameWnd_ShowControlBar_t>(0x59E9DD);

							if (!dvars::mainframe_show_toolbar->current.enabled)
							{
								CFrameWnd_ShowControlBar(cmainframe::activewnd, &cmainframe::activewnd->m_wndToolBar, 1, 1);
								dvars::set_bool(dvars::mainframe_show_toolbar, true);
							}
							else
							{
								CFrameWnd_ShowControlBar(cmainframe::activewnd, &cmainframe::activewnd->m_wndToolBar, 0, 1);
								dvars::set_bool(dvars::mainframe_show_toolbar, false);
							}
						}

						if (ImGui::MenuItem("Camera View (Original)", ggui::hotkey_dialog::get_hotkey_for_command("ToggleCamera").c_str(), nullptr, cmainframe::is_combined_view())) {
							mainframe_thiscall(void, 0x426A40); // cmainframe::OnTogglecamera
						}

						if (ImGui::MenuItem("XY View (Original)", ggui::hotkey_dialog::get_hotkey_for_command("ToggleView").c_str(), nullptr, cmainframe::is_combined_view())) {
							mainframe_thiscall(void, 0x426AE0); // cmainframe::OnToggleview
						}
#endif

						if (ImGui::MenuItem("Layers (Original)"))
						{
							const auto hwnd = game::layer_dlg->GetWindow();
							ShowWindow(game::layer_dlg->GetWindow(), IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
						}

						if (ImGui::MenuItem("Surface Inspector (Original)"))
						{

							// SurfaceInspector::DoSurface
							cdeclcall(void, 0x4585D0);
						}

						if (ImGui::MenuItem("Vert Edit Dialog (Original)"))
						{
							CWnd* vEdit = reinterpret_cast<CWnd*>(0x25D65B0);

							if (IsWindowVisible(vEdit->GetWindow()))
							{
								utils::hook::call<void(__fastcall)(CWnd*, int, int)>(0x58EA4F)(vEdit, 0, SW_HIDE);
							}
							else
							{
								utils::hook::call<void(__fastcall)(CWnd*, int, int)>(0x58EA4F)(vEdit, 0, SW_SHOW);
							}
						}

						ImGui::EndMenu(); // Original Windows
					}

					ImGui::EndMenu(); // Toggle
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Center", ggui::hotkey_dialog::get_hotkey_for_command("CenterView").c_str())) {
					mainframe_thiscall(void, 0x423C50); // cmainframe::OnViewCenter
				}

				if (ImGui::MenuItem("Center 2D On Camera", ggui::hotkey_dialog::get_hotkey_for_command("Center2DOnCamera").c_str())) {
					cdeclcall(void, 0x42A2D0); // cmainframe::OnCenter2DOnCamera
				}

				if (ImGui::MenuItem("Center Camera On Selection", ggui::hotkey_dialog::get_hotkey_for_command("center_camera_on_selection").c_str())) {
					components::command::execute("center_camera_on_selection");
				}

				if (ImGui::MenuItem("Up Floor", ggui::hotkey_dialog::get_hotkey_for_command("UpFloor").c_str())) {
					mainframe_thiscall(void, 0x424700); // cmainframe::OnViewUpfloor
				} TT("move up one floor, if there is one");

				if (ImGui::MenuItem("Down Floor", ggui::hotkey_dialog::get_hotkey_for_command("DownFloor").c_str())) {
					mainframe_thiscall(void, 0x423ED0); // cmainframe::OnViewDownfloor
				} TT("move down one floor, if there is one");

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Next (XY, YZ, XZ)", ggui::hotkey_dialog::get_hotkey_for_command("NextView").c_str())) {
					mainframe_thiscall(void, 0x426DB0); // cmainframe::OnViewNextview
				}

				if (ImGui::BeginMenu("Grid Window"))
				{
					if (ImGui::MenuItem("XY")) {
						mainframe_thiscall(void, 0x424710); // cmainframe::OnViewXy
					}

					if (ImGui::MenuItem("YZ")) {
						mainframe_thiscall(void, 0x423FB0); // cmainframe::OnViewYz
					}

					if (ImGui::MenuItem("XZ")) {
						mainframe_thiscall(void, 0x424A80); // cmainframe::OnViewXz
					}

					ImGui::EndMenu(); // Grid Window
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Hide Selected", ggui::hotkey_dialog::get_hotkey_for_command("HideSelected").c_str())) {
					cdeclcall(void, 0x42B6A0); // cmainframe::OnHideSelected
				}

				if (ImGui::MenuItem("Hide Unselected", ggui::hotkey_dialog::get_hotkey_for_command("HideUnSelected").c_str())) {
					cdeclcall(void, 0x42B6C0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Last Hidden", ggui::hotkey_dialog::get_hotkey_for_command("ShowLastHidden").c_str())) {
					cdeclcall(void, 0x42B6E0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Hidden", ggui::hotkey_dialog::get_hotkey_for_command("ShowHidden").c_str())) {
					cdeclcall(void, 0x42B6D0); // cmainframe::OnShowHidden
				}

				ImGui::EndMenu(); // View
			}

			if (ImGui::BeginMenu("Renderer"))
			{
				if (ImGui::BeginMenu("General"))
				{
					if (ImGui::MenuItem("Outline Selected", ggui::hotkey_dialog::get_hotkey_for_command("ToggleOutlineDraw").c_str(), !game::g_qeglobals->dontDrawSelectedOutlines)) {
						game::g_qeglobals->dontDrawSelectedOutlines ^= 1;
					}

					if (ImGui::MenuItem("Tint Selected", ggui::hotkey_dialog::get_hotkey_for_command("ToggleTintDraw").c_str(), !game::g_qeglobals->dontDrawSelectedTint)) {
						game::g_qeglobals->dontDrawSelectedTint ^= 1;
					}

					if (ImGui::MenuItem("Show Connections", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 4) == 0)) {
						mainframe_thiscall(void, 0x42BBC0); // cmainframe::OnSelectConnections
					} TT("Draw connection lines between entities, eg. target <-> targetname");

					if (ImGui::MenuItem("Show Angles", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 2) == 0)) {
						mainframe_thiscall(void, 0x42BAA0); // cmainframe::OnSelectAngles
					} TT("Draw entity angles");

					if (ImGui::MenuItem("Draw Model Origins", 0, dvars::r_draw_model_origin->current.enabled)) {
						dvars::set_bool(dvars::r_draw_model_origin, !dvars::r_draw_model_origin->current.enabled);
					} TT("Draw small boxes around entity origins (size can be set in preferences/general \"Model origin size\"");

					ImGui::EndMenu(); // General
				}

				if (ImGui::BeginMenu("Grid Window"))
				{
					if (ImGui::MenuItem("Show Names", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 8) == 0)) {
						mainframe_thiscall(void, 0x42BA40); // cmainframe::OnSelectNames
					} TT("Draw entity classnames");

					if (ImGui::MenuItem("Show Grid Blocks", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 16) == 0)) {
						mainframe_thiscall(void, 0x42BB00); // cmainframe::OnSelectBlocks
					}

					if (ImGui::MenuItem("Draw Edge and Block Coordinates", 0, dvars::grid_draw_edge_coordinates->current.enabled)) {
						dvars::set_bool(dvars::grid_draw_edge_coordinates, !dvars::grid_draw_edge_coordinates->current.enabled);
					}

					if (ImGui::MenuItem("Draw Crosshair", ggui::hotkey_dialog::get_hotkey_for_command("ToggleCrosshairs").c_str(), game::g_bCrossHairs)) {
						game::g_bCrossHairs ^= 1;
					}

					ImGui::EndMenu(); // Grid Window
				}

				if (ImGui::BeginMenu("Render Method / Features"))
				{
					if (const auto	draw_water = game::Dvar_FindVar("r_drawWater");
						draw_water)
					{
						if (ImGui::MenuItem("Draw Water", 0, draw_water->current.enabled)) {
							dvars::set_bool(draw_water, !draw_water->current.enabled);
						} TT("Toggle case-textured <> real water rendering");
					}

					if (const auto	filmtweaks = game::Dvar_FindVar("r_filmtweakenable");
						filmtweaks)
					{
						if (ImGui::MenuItem("Filmtweaks", 0, filmtweaks->current.enabled)) {
							dvars::set_bool(filmtweaks, !filmtweaks->current.enabled);
						} TT("Toggle filmtweak usage");
					}

					if (ImGui::MenuItem("Filmtweak Settings"))
					{
						const auto cs = GET_GUI(ggui::camera_settings_dialog);
						cs->handle_toggle_request(camera_settings_dialog::tab_state_fakesun);
						cs->focus_tab(camera_settings_dialog::tab_state_fakesun);
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Fake Sun Preview", ggui::hotkey_dialog::get_hotkey_for_command("fakesun_toggle").c_str(), dvars::r_fakesun_preview->current.enabled)) {
						components::command::execute("fakesun_toggle");
					} TT("with specular and bumpmap support, settings @preferences->developer");

					if (ImGui::MenuItem("Fake Sun Fog", 0, dvars::r_fakesun_fog_enabled->current.enabled)) {
						dvars::set_bool(dvars::r_fakesun_fog_enabled, !dvars::r_fakesun_fog_enabled->current.enabled);
					} TT("settings @preferences->developer");

					if (ImGui::MenuItem("Fake Sun Settings"))
					{
						const auto cs = GET_GUI(ggui::camera_settings_dialog);
						cs->handle_toggle_request(camera_settings_dialog::tab_state_fakesun);
						cs->focus_tab(camera_settings_dialog::tab_state_fakesun);
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Wireframe", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == game::RM_WIREFRAME)) {
						set_render_method(game::RM_WIREFRAME);
					}

					if (ImGui::MenuItem("Fullbright", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == game::RM_FULLBRIGHT)) {
						set_render_method(game::RM_FULLBRIGHT);
					}

					if (ImGui::MenuItem("Normal-Based Fake Lighting", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == game::RM_NORMALFAKELIGHT)) {
						set_render_method(game::RM_NORMALFAKELIGHT);
					}

					if (ImGui::MenuItem("View-Based Fake Lighting", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == game::RM_VIEWFAKELIGHT)) {
						set_render_method(game::RM_VIEWFAKELIGHT);
					}

					if (ImGui::MenuItem("Case Textures", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == game::RM_CASETEXTURES)) {
						set_render_method(game::RM_CASETEXTURES);
					}

					ImGui::EndMenu(); // Render Method
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Texture Filter"))
				{
					if (ImGui::MenuItem("Unchanged", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 0)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 0);
					}

					if (ImGui::MenuItem("Force Trilinear", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 1)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 1);
					}

					if (ImGui::MenuItem("Force Bilinear", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 2)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 2);
					}

					if (ImGui::MenuItem("Force Mipmaps Off", 0, game::Dvar_FindVar("r_texFilterMipMode")->current.integer == 3)) {
						game::Dvar_SetInt(game::Dvar_FindVar("r_texFilterMipMode"), 3);
					}

					ImGui::EndMenu(); // Texture Filter
				}

				if (ImGui::BeginMenu("Texture Resolution"))
				{
					if (ImGui::MenuItem("Maximum", 0, game::g_qeglobals->d_savedinfo.d_picmip == 0)) {
						set_texture_resolution(0);
					}

					if (ImGui::MenuItem("High", 0, game::g_qeglobals->d_savedinfo.d_picmip == 1)) {
						set_texture_resolution(1);
					}

					if (ImGui::MenuItem("Normal", 0, game::g_qeglobals->d_savedinfo.d_picmip == 2)) {
						set_texture_resolution(2);
					}

					if (ImGui::MenuItem("Low", 0, game::g_qeglobals->d_savedinfo.d_picmip == 3)) {
						set_texture_resolution(3);
					}

					ImGui::EndMenu(); // Texture Resolution
				}

				if (ImGui::BeginMenu("Specular Texture Resolution"))
				{
					const auto r_picmip_spec = game::Dvar_FindVar("r_picmip_spec");

					if (ImGui::MenuItem("Maximum", 0, r_picmip_spec->current.integer == 0)) {
						game::Dvar_SetInt(r_picmip_spec, 0);
					}

					if (ImGui::MenuItem("High", 0, r_picmip_spec->current.integer == 1)) {
						game::Dvar_SetInt(r_picmip_spec, 1);
					}

					if (ImGui::MenuItem("Normal", 0, r_picmip_spec->current.integer == 2)) {
						game::Dvar_SetInt(r_picmip_spec, 2);
					}

					if (ImGui::MenuItem("Low", 0, r_picmip_spec->current.integer == 3)) {
						game::Dvar_SetInt(r_picmip_spec, 3);
					}

					ImGui::EndMenu(); // Texture Resolution
				}

				if (ImGui::BeginMenu("Bump Texture Resolution"))
				{
					const auto r_picmip_bump = game::Dvar_FindVar("r_picmip_bump");

					if (ImGui::MenuItem("Maximum", 0, r_picmip_bump->current.integer == 0)) {
						game::Dvar_SetInt(r_picmip_bump, 0);
					}

					if (ImGui::MenuItem("High", 0, r_picmip_bump->current.integer == 1)) {
						game::Dvar_SetInt(r_picmip_bump, 1);
					}

					if (ImGui::MenuItem("Normal", 0, r_picmip_bump->current.integer == 2)) {
						game::Dvar_SetInt(r_picmip_bump, 2);
					}

					if (ImGui::MenuItem("Low", 0, r_picmip_bump->current.integer == 3)) {
						game::Dvar_SetInt(r_picmip_bump, 3);
					}

					ImGui::EndMenu(); // Texture Resolution
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Reload All Images", ggui::hotkey_dialog::get_hotkey_for_command("RefreshTextures").c_str())) {
					cdeclcall(void, 0x428B50); // CMainFrame::OnTextureRefresh
				} TT("This only reloads the iwi's\nReload individual materials from the context menu (texture browser)");

				if (ImGui::MenuItem("Load New Materials")) {
					utils::hook::call<void(__cdecl)()>(0x45AE40)();
				} TT("This only loads materials that were not present at the start of radiant\nReload individual materials from the context menu (texture browser)");

				if (ImGui::MenuItem("Reload XModels")) 
				{
					memset(game::com_fileDataHashTable, 0, sizeof(uintptr_t) * 1024);
				}

#if 0			// material reload tests
				if (ImGui::MenuItem("Sort World Surfaces"))
				{
					game::R_SortWorldSurfaces();

					// ugly hacks - ref for later

					///*memset(game::rg->Material_materialHashTable, 0, sizeof(game::rg->Material_materialHashTable));
					//memset(game::rgp->sortedMaterials, 0, sizeof(game::rgp->sortedMaterials));
					//game::rgp->materialCount = 0;*/

					//cdeclcall(void, 0x51C020); // Material_Sort

					//unsigned int index = 0;
					//bool exists = false;

					//// Material_GetHashIndex
					//utils::hook::call<int(__cdecl)(const char* _name, unsigned int* _index_out, bool* _exists)>(0x510E10)("wc/case1024", &index, &exists);

					//// Material_Load
					//const auto t_mat = utils::hook::call<game::Material* (__cdecl)(const char* _name, int _type)>(0x51B690)("case1024", 9); //game::Material_RegisterHandle("wc/case1024", 9);

					////if(exists && t_mat)
					////{
					////	utils::hook::call<void(__cdecl)(void*)>(0x4AC2A0)(game::rg->Material_materialHashTable[index]);
					////	game::rg->Material_materialHashTable[index] = nullptr;
					////	game::Material_Add(index, t_mat);
					////}

					//game::rgp->sortedMaterials[game::rgp->materialCount] = t_mat;
					//game::rgp->materialCount++;
					//game::rgp->needSortMaterials = 1;

					////cdeclcall(void, 0x51C020); // Material_Sort

					//components::exec::on_gui_once([]() 
					//{
					//	bool found = false;
					//	for(auto i = 0; i < game::rgp->materialCount; i++)
					//	{
					//		if(!found && game::rgp->sortedMaterials[i]->info.name == "case1024"s)
					//		{
					//			game::rgp->sortedMaterials[i] = nullptr;
					//			//game::rgp->materialCount--;
					//			game::rgp->needSortMaterials = true;

					//			found = true;
					//		}
					//		else if(found)
					//		{
					//			game::rgp->sortedMaterials[i - 1] = game::rgp->sortedMaterials[i];
					//		}
					//	}

					//	if(found)
					//	{
					//		game::rgp->sortedMaterials[game::rgp->materialCount - 1] = nullptr;
					//		game::rgp->materialCount--;
					//	}

					//	game::rgp->needSortMaterials = 1;

					//	cdeclcall(void, 0x52E9F0); // R_SortWorldSurfaces

					//	
					//});

					//game::Material_RegisterHandle("css_portal", 3);
				}
#endif

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Show Patches As"))
				{
					// dvars::r_draw_patch_backface_wireframe
					if (ImGui::MenuItem("Draw Backface Wireframe", 0, dvars::r_draw_patch_backface_wireframe->current.enabled))
					{
						game::Dvar_SetBool(dvars::r_draw_patch_backface_wireframe, !dvars::r_draw_patch_backface_wireframe->current.enabled);
					}

					if (ImGui::MenuItem("Textured", 0, !prefs->g_nPatchAsWireframe))
					{
						prefs->g_nPatchAsWireframe = 0;
						game::CPrefsDlg_SavePrefs();
					}

					if (ImGui::MenuItem("Untextured", 0, prefs->g_nPatchAsWireframe == 1))
					{
						prefs->g_nPatchAsWireframe = 1;
						game::CPrefsDlg_SavePrefs();
					}

					if (ImGui::MenuItem("Textured + Wireframe", 0, prefs->g_nPatchAsWireframe == 2))
					{
						prefs->g_nPatchAsWireframe = 2;
						game::CPrefsDlg_SavePrefs();
					}

					ImGui::EndMenu(); // Draw Patches As
				}

				if (ImGui::BeginMenu("Show Entities As"))
				{
					if (ImGui::MenuItem("Bounding Box", 0, prefs->m_nEntityShowState == game::ENTITY_BOXED)) {
						mainframe_thiscall(void, 0x42B320); // cmainframe::OnViewEntitiesasBoundingbox
					}

					if (ImGui::MenuItem("Wireframe", 0, prefs->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B410); // cmainframe::OnViewEntitiesasWireframe
					}

					if (ImGui::MenuItem("Selected Wireframe", 0, prefs->m_nEntityShowState == (game::ENTITY_SELECTED_ONLY | game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B380); // cmainframe::OnViewEntitiesasSelectedwireframe
					}

					if (ImGui::MenuItem("Selected Skinned", 0, prefs->m_nEntityShowState == (game::ENTITY_SELECTED_ONLY | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B350); // cmainframe::OnViewEntitiesasSelectedskinned
					}

					if (ImGui::MenuItem("Skinned", 0, prefs->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3B0); // cmainframe::OnViewEntitiesasSkinned
					}

					if (ImGui::MenuItem("Skinned And Boxed", 0, prefs->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_BOXED | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3E0); // cmainframe::OnViewEntitiesasSkinnedandboxed
					}

					ImGui::EndMenu(); // Show Entities As
				}

				if (ImGui::BeginMenu("Light / Sun Preview"))
				{
					if (ImGui::MenuItem("Enable Light Preview", ggui::hotkey_dialog::get_hotkey_for_command("LightPreviewToggle").c_str(), prefs->enable_light_preview)) {
						mainframe_thiscall(void, 0x4240C0); // cmainframe::OnEnableLightPreview
					}

					if (ImGui::MenuItem("Enable Sun Preview", ggui::hotkey_dialog::get_hotkey_for_command("LightPreviewSun").c_str(), prefs->preview_sun_aswell)) {
						mainframe_thiscall(void, 0x424060); // cmainframe::OnPreviewSun
					}

					if (ImGui::MenuItem("Enable Sun Shadows", 0, dvars::r_sunpreview_shadow_enable->current.enabled)) {
						game::Dvar_SetBool(dvars::r_sunpreview_shadow_enable, !dvars::r_sunpreview_shadow_enable->current.enabled);
					}

					IMGUI_MENU_WIDGET_SINGLE("Sun Shadow Distance", ImGui::DragFloat("##sunshadow_dist_scalar", &dvars::r_sunpreview_shadow_dist->current.value, 25.0f, 0, FLT_MAX, "%.0f"));

					SEPERATORV(0.0f);

					// debug_sundir
					if (ImGui::MenuItem("Debug Sundirection", 0, game::glob::debug_sundir)) {
						game::glob::debug_sundir = !game::glob::debug_sundir;
					}

					IMGUI_MENU_WIDGET_SINGLE("Sundirection Length", ImGui::DragFloat("##sundir_debug_length", &game::glob::debug_sundir_length, 15.0f, 100.0f, FLT_MAX, "%.0f"));
					IMGUI_MENU_WIDGET_SINGLE("Sundirection Position X", ImGui::DragFloat("##sundir_debug_posx", &game::glob::debug_sundir_startpos[0], 15.0f, -FLT_MAX, FLT_MAX, "%.0f"));
					IMGUI_MENU_WIDGET_SINGLE("Sundirection Position Y", ImGui::DragFloat("##sundir_debug_posy", &game::glob::debug_sundir_startpos[1], 15.0f, -FLT_MAX, FLT_MAX, "%.0f"));
					IMGUI_MENU_WIDGET_SINGLE("Sundirection Position Z", ImGui::DragFloat("##sundir_debug_posz", &game::glob::debug_sundir_startpos[2], 15.0f, -FLT_MAX, FLT_MAX, "%.0f"));

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Start Previewing Selected", ggui::hotkey_dialog::get_hotkey_for_command("LightPreviewStart").c_str())) {
						mainframe_thiscall(void, 0x424120); // cmainframe::OnStartPreviewSelected
					}

					if (ImGui::MenuItem("Stop Previewing Selected", ggui::hotkey_dialog::get_hotkey_for_command("LightPreviewStop").c_str())) {
						mainframe_thiscall(void, 0x424170); // cmainframe::OnStopPreviewSelected
					}

					if (ImGui::MenuItem("Clear Preview List", ggui::hotkey_dialog::get_hotkey_for_command("LightPreviewClear").c_str())) {
						mainframe_thiscall(void, 0x4241C0); // cmainframe::OnClearPreviewList
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Preview At Max Intensity", ggui::hotkey_dialog::get_hotkey_for_command("MaxLightIntensity").c_str(), !game::g_qeglobals->preview_at_max_intensity)) {
						cdeclcall(void, 0x425670); // cmainframe::OnPreviewAtMaxIntensity
					}

					if (ImGui::MenuItem("Show Regions For Selected", ggui::hotkey_dialog::get_hotkey_for_command("LightPreviewRegions").c_str())) {
						mainframe_thiscall(void, 0x4241E0); // cmainframe::OnShowRegionsForSelected
					}

					ImGui::EndMenu(); // Light Preview
				}

				SEPERATORV(0.0f);

				//if (ImGui::MenuItem("Cubic Clip Zoom In", "CTRL-[")) {
				//	mainframe_thiscall(void, 0x428F10); // cmainframe::OnViewCubein
				//}

				//if (ImGui::MenuItem("Cubic Clip Zoom Out", "CTRL-]")) {
				//	mainframe_thiscall(void, 0x428F50); // cmainframe::OnViewCubeout
				//}

				IMGUI_MENU_WIDGET_SINGLE("Cubic Scale", ImGui::DragInt("##cubic_scalar", &prefs->m_nCubicScale, 1, 1, 220));

				if (ImGui::MenuItem("Cubic Clipping", ggui::hotkey_dialog::get_hotkey_for_command("ToggleCubicClip").c_str(), prefs->m_bCubicClipping)) {
					mainframe_thiscall(void, 0x428F90); // cmainframe::OnViewCubicclipping
				}

				ImGui::EndMenu(); // Renderer
			}

			if (ImGui::BeginMenu("Effects"))
			{
				if (ImGui::MenuItem("Effect Settings .."))
				{
					const auto cs = GET_GUI(ggui::camera_settings_dialog);
					cs->handle_toggle_request(camera_settings_dialog::tab_state_effects);
					cs->focus_tab(camera_settings_dialog::tab_state_effects);
				}

				if (ImGui::MenuItem("Effects Browser")) 
				{
					GET_GUI(ggui::effects_browser)->toggle();
				}

				if (ImGui::MenuItem("Edit Current Effect", 0, nullptr, components::effects::effect_can_play()))
				{
					components::effects::edit();
				}

				if (ImGui::MenuItem("Reload Current Effect", 0, nullptr, components::effects::effect_can_play()))
				{
					if (components::effects_editor::is_editor_active())
					{
						GET_GUI(ggui::effects_editor_dialog)->m_pending_reload = true;
					}
					else
					{
						components::command::execute("fx_reload");
					}
				}

				SEPERATORV(0.0f);

				const auto r_showTris = game::Dvar_FindVar("r_showTris");
				if (ImGui::MenuItem("Draw Debug Tris", 0, r_showTris->current.enabled)) {
					dvars::set_int(r_showTris, r_showTris->current.integer ? 0 : 1);
				}

				SEPERATORV(0.0f);

				IMGUI_MENU_WIDGET_SINGLE("Timescale", ImGui::DragFloat("##timescale", &fx_system::ed_timescale, 0.005f, 0.001f, 50.0f));
				IMGUI_MENU_WIDGET_SINGLE("Repeat Delay", ImGui::DragFloat("##repeatdelay", &fx_system::ed_looppause, 0.01f, 0.05f, FLT_MAX, "%.2f"));
				ImGui::EndMenu(); // Effects
			}

			if (ImGui::BeginMenu("d3dbsp"))
			{
				if (ImGui::MenuItem("Load d3dbsp .."))
				{
					const auto egui = GET_GUI(ggui::entity_dialog);
					const std::string path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath") + "\\raw\\maps\\mp\\"s;

					const auto file = GET_GUI(ggui::file_dialog);
					file->set_default_path(path_str);
					file->set_file_handler(ggui::FILE_DIALOG_HANDLER::D3DBSP_LOAD);
					file->set_file_op_type(file_dialog::FileDialogType::OpenFile);
					file->set_file_ext(".d3dbsp");
					file->open();
				}

				if (ImGui::MenuItem("Reload d3dbsp"))
				{
					components::d3dbsp::reload_bsp();

				} TT("Reload the currently loaded bsp.\nTries to automatically load a bsp based of the .map name if no bsp is loaded.");

				SEPERATORV(0.0f);

				const bool can_reload_bsp = components::d3dbsp::Com_IsBspLoaded() && !components::d3dbsp::loaded_bsp_path.empty();
				ImGui::BeginDisabled(!can_reload_bsp);
				{
					const auto gameview = components::gameview::p_this;
					const bool tstate = gameview->get_all_geo_state() || gameview->get_all_ents_state() || gameview->get_all_triggers_state() || gameview->get_all_others_state();

					if (ImGui::MenuItem("Draw d3dbsp", ggui::hotkey_dialog::get_hotkey_for_command("toggle_bsp").c_str(), dvars::r_draw_bsp->current.enabled))
					{
						dvars::set_bool(dvars::r_draw_bsp, !dvars::r_draw_bsp->current.enabled);
					}

					if (ImGui::MenuItem("Toggle d3dbsp/radiant", ggui::hotkey_dialog::get_hotkey_for_command("toggle_bsp_radiant").c_str()))
					{
						components::command::execute("toggle_bsp_radiant");
					}
					
					if (ImGui::MenuItem("Draw Radiant World", ggui::hotkey_dialog::get_hotkey_for_command("toggle_filter_all").c_str(), !tstate))
					{
						components::command::execute("toggle_filter_all");
					}

					ImGui::EndDisabled();
				}

				SEPERATORV(0.0f);

				const bool can_compile = !game::g_prefab_stack_level;
				imgui::BeginDisabled(!can_compile);
				{
					if (ImGui::MenuItem("Compile current map", ggui::hotkey_dialog::get_hotkey_for_command("bsp_compile").c_str()))
					{
						components::d3dbsp::compile_current_map();

					} TT("Compile currently loaded .map with settings specified within 'Compile Settings'.\nAutomatically reloads the bsp when finished.");

					imgui::EndDisabled();
				}

				if (ImGui::MenuItem("Compile Settings .."))
				{
					const auto cs = GET_GUI(ggui::camera_settings_dialog);
					cs->handle_toggle_request(camera_settings_dialog::tab_state_bsp);
					cs->focus_tab(camera_settings_dialog::tab_state_bsp);
				}

				ImGui::EndMenu(); // BSP
			}

			if (ImGui::BeginMenu("Selection"))
			{
				if (ImGui::MenuItem("Drag Edges", ggui::hotkey_dialog::get_hotkey_for_command("DragEdges").c_str())) {
					cdeclcall(void, 0x4257D0); // CMainFrame::OnSelectionDragedges
				}

				if (ImGui::MenuItem("Drag Vertices", ggui::hotkey_dialog::get_hotkey_for_command("DragVertices").c_str())) {
					cdeclcall(void, 0x425840); // CMainFrame::OnSelectionDragVertices
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Select"))
				{
					if (ImGui::MenuItem("Select Targetname")) {
						cdeclcall(void, 0x426390); // CMainFrame::OnSelectionTargetname
					}

					if (ImGui::MenuItem("Select Classname")) {
						cdeclcall(void, 0x4263A0); // CMainFrame::OnSelectionClassname
					}

					if (ImGui::MenuItem("Select By Key/Value", ggui::hotkey_dialog::get_hotkey_for_command("SelectionKeyValue").c_str())) {
						cdeclcall(void, 0x4263B0); // CMainFrame::OnSelectionKeyValue
					}

					if (ImGui::MenuItem("Select Connected", ggui::hotkey_dialog::get_hotkey_for_command("SelectConnectedEntities").c_str())) {
						cdeclcall(void, 0x425550); // CMainFrame::OnSelectConneted
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Select Complete Tall")) {
						cdeclcall(void, 0x426340); // CMainFrame::OnSelectionSelectcompletetall
					}

					if (ImGui::MenuItem("Select Partial Tall")) {
						cdeclcall(void, 0x426360); // CMainFrame::OnSelectionSelectpartialtall
					}

					if (ImGui::MenuItem("Select Touching")) {
						cdeclcall(void, 0x426370); // CMainFrame::OnSelectionSelecttouching
					}

					if (ImGui::MenuItem("Select Inside")) {
						cdeclcall(void, 0x426350); // CMainFrame::OnSelectionSelectinside
					}

					ImGui::EndMenu(); // Select
				}

				if (ImGui::MenuItem("Deselect", ggui::hotkey_dialog::get_hotkey_for_command("UnSelectSelection").c_str())) {
					mainframe_thiscall(void, 0x425740); // CMainFrame::OnSelectionDeselect
				}

				if (ImGui::MenuItem("Clone", ggui::hotkey_dialog::get_hotkey_for_command("CloneSelection").c_str())) {
					cdeclcall(void, 0x425480); // CMainFrame::OnSelectionClone
				}

				if (ImGui::MenuItem("Invert", ggui::hotkey_dialog::get_hotkey_for_command("InvertSelection").c_str())) {
					cdeclcall(void, 0x42B6F0); // CMainFrame::OnSelectionInvert
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Flip"))
				{
					if (ImGui::MenuItem("Flip X")) {
						cdeclcall(void, 0x4250A0); // CMainFrame::OnBrushFlipx
					}

					if (ImGui::MenuItem("Flip Y")) {
						cdeclcall(void, 0x4250C0); // CMainFrame::OnBrushFlipy
					}

					if (ImGui::MenuItem("Flip Z")) {
						cdeclcall(void, 0x4250E0); // CMainFrame::OnBrushFlipz
					}

					ImGui::EndMenu(); // Flip
				}

				if (ImGui::BeginMenu("Rotate"))
				{
					if (ImGui::MenuItem("Rotate X")) {
						cdeclcall(void, 0x425100); // CMainFrame::OnBrushRotatex
					}

					if (ImGui::MenuItem("Rotate Y")) {
						cdeclcall(void, 0x425190); // CMainFrame::OnBrushRotatey
					}

					if (ImGui::MenuItem("Rotate Z", ggui::hotkey_dialog::get_hotkey_for_command("RotateZ").c_str())) {
						cdeclcall(void, 0x425220); // CMainFrame::OnBrushRotatez
					}

					if (ImGui::MenuItem("Free Rotation", ggui::hotkey_dialog::get_hotkey_for_command("MouseRotate").c_str(), game::g_bRotateMode)) {
						mainframe_thiscall(void, 0x428570); // CMainFrame::OnSelectMouserotate
					}

					if (ImGui::MenuItem("Arbitrary Rotation")) {
						cdeclcall(void, 0x425300); // CMainFrame::OnSelectionArbitraryrotation
					}

					ImGui::EndMenu(); // Rotate
				}

				if (ImGui::BeginMenu("Scale"))
				{
					if (ImGui::MenuItem("Lock X", 0, (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6))) {
						mainframe_thiscall(LRESULT, 0x428BC0); // CMainFrame::OnScalelockX
					}

					if (ImGui::MenuItem("Lock Y", 0, (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5))) {
						mainframe_thiscall(LRESULT, 0x428B60); // CMainFrame::OnScalelockY
					}

					if (ImGui::MenuItem("Lock Z", 0, (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3))) {
						mainframe_thiscall(LRESULT, 0x428B90); // CMainFrame::OnScalelockZ
					}

					if (ImGui::MenuItem("Free Scaling", 0, game::g_bScaleMode)) {
						mainframe_thiscall(LRESULT, 0x428D20); // CMainFrame::OnSelectMousescale
					}

					if (ImGui::MenuItem("Arbitrary Scale")) {
						cdeclcall(void, 0x4283D0); // CMainFrame::OnSelectScale
					}

					ImGui::EndMenu(); // Scale
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Guizmo - World-Local", ggui::hotkey_dialog::get_hotkey_for_command("guizmo_world_local").c_str(), ggui::camera_guizmo::g_guizmo_local))
				{
					ggui::camera_guizmo::g_guizmo_local = !ggui::camera_guizmo::g_guizmo_local;
				}

				if (ImGui::MenuItem("Maya Export")) {
					cdeclcall(void, 0x4263C0); // CMainFrame::OnExportToMaya
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("CSG"))
				{
					if (ImGui::MenuItem("Hollow")) {
						cdeclcall(void, 0x425570); // CMainFrame::OnSelectionMakehollow
					}

					if (ImGui::MenuItem("Merge", ggui::hotkey_dialog::get_hotkey_for_command("CSGMerge").c_str())) {
						cdeclcall(void, 0x4255D0); // CMainFrame::OnSelectionCsgmerge
					}

					if (ImGui::MenuItem("Auto Caulk", ggui::hotkey_dialog::get_hotkey_for_command("AutoCaulk").c_str())) {
						cdeclcall(void, 0x425600); // CMainFrame::OnSelectionAutoCaulk
					}

					ImGui::EndMenu(); // CSG
				}

				if (ImGui::BeginMenu("Clipper"))
				{
					if (ImGui::MenuItem("Toggle Clipper", ggui::hotkey_dialog::get_hotkey_for_command("ToggleClipper").c_str())) {
						mainframe_thiscall(void, 0x426510); // CMainFrame::OnViewClipper
					}

					if (ImGui::MenuItem("Clip Selected", ggui::hotkey_dialog::get_hotkey_for_command("ClipSelected").c_str())) {
						mainframe_thiscall(void, 0x427170); // CMainFrame::OnClipSelected
					}

					if (ImGui::MenuItem("Split Selected", ggui::hotkey_dialog::get_hotkey_for_command("SplitSelected").c_str())) {
						mainframe_thiscall(void, 0x4271D0); // CMainFrame::OnSplitSelected
					}

					if (ImGui::MenuItem("Flip Clip Orientation", ggui::hotkey_dialog::get_hotkey_for_command("FlipClip").c_str())) {
						mainframe_thiscall(void, 0x427140); // CMainFrame::OnFlipClip
					}

					ImGui::EndMenu(); // Clipper
				}

				SEPERATORV(0.0f);

				{
					bool prefab_selected = false;

					auto sb = game::g_selected_brushes();
					if (sb->def && sb->def->owner)
					{
						prefab_selected = GET_GUI(ggui::entity_dialog)->get_value_for_key_from_epairs(sb->def->owner->epairs, "classname") == "misc_prefab"s;
					}

					ImGui::BeginDisabled(!prefab_selected);
					{
						if (ImGui::MenuItem("Enter Prefab"))
						{
							game::Prefab_Enter();
						}

						ImGui::EndDisabled();
					}

					ImGui::BeginDisabled(!game::g_prefab_stack_level);
					{
						if (ImGui::MenuItem("Leave Prefab"))
						{
							if (game::g_prefab_stack_level)
							{
								game::Prefab_Leave();
							}
						}

						ImGui::EndDisabled();
					}

					ImGui::BeginDisabled(!prefab_selected);
					{
						camera_dialog::stamp_prefab_imgui_imgui_menu();

						ImGui::EndDisabled();
					}

					// update sb because the selection could be invalid (if it was stamped) 
					sb = game::g_selected_brushes();

					ImGui::BeginDisabled(!(sb->def&& sb->def->owner));
					{
						camera_dialog::convert_selection_to_prefab_imgui_menu();
						ImGui::EndDisabled();
					}
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Connect Entities", ggui::hotkey_dialog::get_hotkey_for_command("ConnectSelection").c_str())) {
					cdeclcall(void, 0x425510); // CMainFrame::OnSelectionConnect
				}

				if (ImGui::MenuItem("Link Entities", ggui::hotkey_dialog::get_hotkey_for_command("LinkSelected").c_str())) {
					cdeclcall(void, 0x425500); // CMainFrame::OnLinkSelected
				}

				if (ImGui::MenuItem("Ungroup Entity")) {
					cdeclcall(void, 0x426380); // CMainFrame::OnSelectionUngroupentity
				}

				if (ImGui::MenuItem("Cycle Group Selection", ggui::hotkey_dialog::get_hotkey_for_command("Patch TAB").c_str())) {
					cdeclcall(void, 0x42A9E0); // CMainFrame::OnPatchTab
				} TT("Cycle between all brushes/patches of a selected group/brushmodel");

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Make Detail", ggui::hotkey_dialog::get_hotkey_for_command("MakeDetail").c_str())) {
					cdeclcall(void, 0x4261C0); // CMainFrame::OnSelectionMakeDetail
				}

				if (ImGui::MenuItem("Make Structural", ggui::hotkey_dialog::get_hotkey_for_command("MakeStructural").c_str())) {
					cdeclcall(void, 0x426200); // CMainFrame::OnSelectionMakeStructural
				}

				if (ImGui::MenuItem("Make Weapon Clip", ggui::hotkey_dialog::get_hotkey_for_command("MakeWeaponClip").c_str())) {
					cdeclcall(void, 0x426240); // CMainFrame::OnSelectionMakeWeaponclip
				}

				if (ImGui::MenuItem("Make Non-Colliding", ggui::hotkey_dialog::get_hotkey_for_command("MakeNonColliding").c_str())) {
					cdeclcall(void, 0x426280); // CMainFrame::OnSelectionMakeNonColliding
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Make Split Coplanar Geo")) {
					cdeclcall(void, 0x4262C0); // CMainFrame::OnSelectionMakeSplitCoplanar
				}

				if (ImGui::MenuItem("Make Don't Split Coplanar Geo")) {
					cdeclcall(void, 0x426300); // CMainFrame::OnSelectionMakeDontSplitCoplanar
				}

				ImGui::EndMenu(); // Selection
			}

			if (ImGui::BeginMenu("Grid"))
			{
				if (ImGui::MenuItem("Grid 0.5", ggui::hotkey_dialog::get_hotkey_for_command("SetGridPointFive").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_05)) {
					set_grid_size(xywnd::GRID_05);
				}

				if (ImGui::MenuItem("Grid 1", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid1").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_1)) {
					set_grid_size(xywnd::GRID_1);
				}

				if (ImGui::MenuItem("Grid 2", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid2").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_2)) {
					set_grid_size(xywnd::GRID_2);
				}

				if (ImGui::MenuItem("Grid 4", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid4").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_4)) {
					set_grid_size(xywnd::GRID_4);
				}

				if (ImGui::MenuItem("Grid 8", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid8").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_8)) {
					set_grid_size(xywnd::GRID_8);
				}

				if (ImGui::MenuItem("Grid 16", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid16").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_16)) {
					set_grid_size(xywnd::GRID_16);
				}

				if (ImGui::MenuItem("Grid 32", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid32").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_32)) {
					set_grid_size(xywnd::GRID_32);
				}

				if (ImGui::MenuItem("Grid 64", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid64").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_64)) {
					set_grid_size(xywnd::GRID_64);
				}

				if (ImGui::MenuItem("Grid 128", 0, game::g_qeglobals->d_gridsize == xywnd::GRID_128)) {
					set_grid_size(xywnd::GRID_128);
				}

				if (ImGui::MenuItem("Grid 256", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid256").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_256)) {
					set_grid_size(xywnd::GRID_256);
				}

				if (ImGui::MenuItem("Grid 512", ggui::hotkey_dialog::get_hotkey_for_command("SetGrid512").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_512)) {
					set_grid_size(xywnd::GRID_512);
				}

				if (ImGui::MenuItem("Snap To Grid", ggui::hotkey_dialog::get_hotkey_for_command("ToggleSnapToGrid").c_str(), !prefs->m_bNoClamp)) {
					mainframe_thiscall(void, 0x428380); // CMainFrame::OnSnaptogrid
				}

				if (imgui::MenuItem("Snap To Grid - Guizmo", "", dvars::guizmo_snapping->current.enabled)) {
					dvars::set_bool(dvars::guizmo_snapping, !dvars::guizmo_snapping->current.enabled);
				}

				SEPERATORV(0.0f);

				if (imgui::MenuItem("New-Patch-Dragging", ggui::hotkey_dialog::get_hotkey_for_command("new_patch_drag").c_str(), dvars::grid_new_patch_drag->current.enabled)) {
					dvars::set_bool(dvars::grid_new_patch_drag, !dvars::grid_new_patch_drag->current.enabled);
				} TT(dvars::grid_new_patch_drag->description);

				ImGui::EndMenu(); // Grid
			}

			if (ImGui::BeginMenu("Textures"))
			{
				//if (ImGui::MenuItem("Show All", ggui::hotkey_dialog::get_hotkey_for_command("ShowAllTextures").c_str())) {
				//	cdeclcall(void, 0x42B440); // CMainFrame::OnTexturesShowall
				//}

				//if (ImGui::MenuItem("Show In Use", ggui::hotkey_dialog::get_hotkey_for_command("ShowTexturesInUse").c_str())) {
				//	mainframe_thiscall(void, 0x424B20); // CMainFrame::OnTexturesShowinuse
				//}

				if (ImGui::MenuItem("Surface Inspector", ggui::hotkey_dialog::get_hotkey_for_command("SurfaceInspector").c_str())) {
					cdeclcall(void, 0x424B60); // CMainFrame::OnTexturesInspector
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Find / Replace")) {
					cdeclcall(void, 0x428B40); // CMainFrame::OnTextureReplaceall
				}

				//if (ImGui::BeginMenu("Texture Window Scale"))
				//{
				//	if (ImGui::MenuItem("200%", 0, prefs->m_nTextureWindowScale == 200)) {
				//		mainframe_thiscall(void, 0x42B020); // CMainFrame::OnTexturesTexturewindowscale200
				//	}

				//	if (ImGui::MenuItem("100%", 0, prefs->m_nTextureWindowScale == 100)) {
				//		mainframe_thiscall(void, 0x42B000); // CMainFrame::OnTexturesTexturewindowscale100
				//	}

				//	if (ImGui::MenuItem("50%", 0, prefs->m_nTextureWindowScale == 50)) {
				//		mainframe_thiscall(void, 0x42B060); // CMainFrame::OnTexturesTexturewindowscale50
				//	}

				//	if (ImGui::MenuItem("25%", 0, prefs->m_nTextureWindowScale == 25)) {
				//		mainframe_thiscall(void, 0x42B040); // CMainFrame::OnTexturesTexturewindowscale25
				//	}

				//	if (ImGui::MenuItem("10%", 0, prefs->m_nTextureWindowScale == 10)) {
				//		mainframe_thiscall(void, 0x42AFE0); // CMainFrame::OnTexturesTexturewindowscale10
				//	}

				//	ImGui::EndMenu(); // Texture Window Scale
				//}

				if (ImGui::BeginMenu("Texture Lock"))
				{
					if (ImGui::MenuItem("Moves", ggui::hotkey_dialog::get_hotkey_for_command("ToggleTexMoveLock").c_str(), prefs->m_bTextureLock == 1)) {
						mainframe_thiscall(void, 0x426B80); // CMainFrame::OnToggleLockMoves
					}

					if (ImGui::MenuItem("Rotations", ggui::hotkey_dialog::get_hotkey_for_command("ToggleTexRotateLock").c_str(), prefs->m_bRotateLock == 1)) {
						mainframe_thiscall(void, 0x429230); // CMainFrame::OnToggleLockRotations
					}

					if (ImGui::MenuItem("Lightmaps", ggui::hotkey_dialog::get_hotkey_for_command("ToggleLightmapLock").c_str(), prefs->m_bLightmapLock == 1)) {
						mainframe_thiscall(void, 0x426BF0); // CMainFrame::OnToggleLockLightmap
					}

					ImGui::EndMenu(); // Texture Lock
				}

				SEPERATORV(0.0f);

				//if (ImGui::BeginMenu("Layered Materials"))
				//{
				//	if (ImGui::MenuItem("Toogle Tool Window", ggui::hotkey_dialog::get_hotkey_for_command("ToggleLayeredMaterialWnd").c_str()))
				//	{
				//		cdeclcall(void, 0x42BFE0); // CMainFrame::OnToggleLayeredMaterials
				//	}

				//	if (ImGui::MenuItem("Save", ggui::hotkey_dialog::get_hotkey_for_command("SaveLayeredMaterials").c_str())) {
				//		cdeclcall(void, 0x42C020); // CMainFrame::OnSaveLayeredMaterials
				//	}

				//	ImGui::EndMenu(); // Layered Materials
				//}

				if (ImGui::MenuItem("Reload All Images", ggui::hotkey_dialog::get_hotkey_for_command("RefreshTextures").c_str())) {
					cdeclcall(void, 0x428B50); // CMainFrame::OnTextureRefresh
				} TT("This only reloads the iwi's\nReload individual materials from the context menu (texture browser)");

				if (ImGui::MenuItem("Load New Materials")) {
					utils::hook::call<void(__cdecl)()>(0x45AE40)();
				} TT("This only loads materials that were not present at the start of radiant\nReload individual materials from the context menu (texture browser)");

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Edit Layer"))
				{
					if (ImGui::MenuItem("Cycle", ggui::hotkey_dialog::get_hotkey_for_command("TexLayerCycle").c_str())) {
						cdeclcall(void, 0x424010); // CMainFrame::OnEditLayerCycle
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Material", ggui::hotkey_dialog::get_hotkey_for_command("TexLayerMaterial").c_str(), game::g_qeglobals->current_edit_layer == 0)) {
						cdeclcall(void, 0x424030); // CMainFrame::OnEditLayerMaterial
					}

					if (ImGui::MenuItem("Lightmap", ggui::hotkey_dialog::get_hotkey_for_command("TexLayerLightmap").c_str(), game::g_qeglobals->current_edit_layer == 1)) {
						cdeclcall(void, 0x424040); // CMainFrame::OnEditLayerLightmap
					}

					if (ImGui::MenuItem("Smoothing", 0, game::g_qeglobals->current_edit_layer == 2)) {
						cdeclcall(void, 0x424050); // CMainFrame::OnEditLayerSmoothing
					}

					ImGui::EndMenu(); // Edit Layer
				}

#if 0
				if (ImGui::BeginMenu("Usage Filter"))
				{
					for (std::uint8_t i = 0; i < game::texWndGlob_usageCount; i++)
					{
						const char* name = game::filter_usage_array[i].name;
						if (name)
						{
							if (ImGui::MenuItem(name, 0, game::texWndGlob_usageFilter == i))
							{
								game::texWndGlob_usageFilter = i;
								game::g_nUpdateBits |= W_TEXTURE;
								g_texwnd->nPos[0].nPos_current = 0; // scroll to top
							}
						}
						else if (game::filter_usage_array[i].index == -1)
						{
							SEPERATORV(0.0f);
						}
					}

					ImGui::EndMenu(); // Usage Filter
				}

				if (ImGui::BeginMenu("Locale Filter"))
				{
					for (std::uint8_t i = 0; i < game::texWndGlob_localeCount; i++)
					{
						const char* name = game::filter_locale_array[i].name;
						if (name)
						{
							if (ImGui::MenuItem(name, 0, game::texWndGlob_localeFilter == i))
							{
								game::texWndGlob_localeFilter = i;
								game::g_nUpdateBits |= W_TEXTURE;
								g_texwnd->nPos[0].nPos_current = 0; // scroll to top
							}
						}
						else if (game::filter_locale_array[i].index == -1)
						{
							SEPERATORV(0.0f);
						}
					}

					ImGui::EndMenu(); // Locale Filter
				}

				if (ImGui::BeginMenu("Surface Type Filter"))
				{
					for (std::uint8_t i = 0; i < 29; i++) // hardcoded value
					{
						const char* name = game::filter_surfacetype_array[i].name;
						if (name)
						{
							if (ImGui::MenuItem(name, 0, game::texWndGlob_surfaceTypeFilter == i))
							{
								game::texWndGlob_surfaceTypeFilter = i;
								game::g_nUpdateBits |= W_TEXTURE;
								g_texwnd->nPos[0].nPos_current = 0; // scroll to top
							}
						}
					}

					ImGui::EndMenu(); // Surface Type Filter
				}
#endif

				ImGui::EndMenu(); // Textures
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Rope Generator")) {
					GET_GUI(ggui::rope_generator_dialog)->toggle();
				}

				if (ImGui::MenuItem("Noise Generator")) {
					GET_GUI(ggui::vertex_edit_dialog)->toggle();
				}

				if (ImGui::MenuItem("Mesh Painter")) {
					GET_GUI(ggui::mesh_painter_dialog)->toggle();
				}

				SEPERATORV(0);

				if (ImGui::MenuItem("PhysX Movement", ggui::hotkey_dialog::get_hotkey_for_command("physx_movement").c_str())) {
					components::physx_impl::spawn_character();
				}

				ImGui::EndMenu(); // Tools
			}

			if (ImGui::BeginMenu("Misc"))
			{
				if (ImGui::MenuItem("Find Brush")) {
					cdeclcall(void, 0x424B80); // CMainFrame::OnMiscFindbrush
				}

				if (ImGui::MenuItem("Go To Position")) {
					cdeclcall(void, 0x424BA0); // CMainFrame::OnMiscGoToPosition
				}

				if (ImGui::MenuItem("-> Leak Spot / Error", ggui::hotkey_dialog::get_hotkey_for_command("NextLeakSpot").c_str())) {
					cdeclcall(void, 0x424BC0); // CMainFrame::OnMiscNextleakspot
				}

				if (ImGui::MenuItem("<- Leak Spot / Error", ggui::hotkey_dialog::get_hotkey_for_command("PrevLeakSpot").c_str())) {
					cdeclcall(void, 0x424BE0); // CMainFrame::OnMiscPreviousleakspot
				}

				//if (ImGui::MenuItem("Print XY View")) {
				//	cdeclcall(void, 0x424C00); // CMainFrame::OnMiscPrintxy
				//}

				if (ImGui::MenuItem("Entity Color Dialog", ggui::hotkey_dialog::get_hotkey_for_command("EntityColor").c_str())) {
					cdeclcall(void, 0x424C10); // CMainFrame::OnMiscSelectentitycolor
				}

				if (ImGui::MenuItem("Script Group Dialog")) {
					cdeclcall(void, 0x424E20); // CMainFrame::OnScriptGroup
				}

				if (ImGui::MenuItem("Delete Exportables")) {
					cdeclcall(void, 0x424E30); // CMainFrame::OnDeleteExportables
				}

				if (ImGui::MenuItem("Vehicle Group Dialog", ggui::hotkey_dialog::get_hotkey_for_command("VehicleGroup").c_str())) {
					cdeclcall(void, 0x42BD50); // CMainFrame::OnMiscVehicleGroup
				}

				if (ImGui::MenuItem("Dyn Entity Dialog", ggui::hotkey_dialog::get_hotkey_for_command("DynEntities").c_str())) {
					cdeclcall(void, 0x42BD90); // CMainFrame::OnMiscDynEntities
				}

				if (ImGui::MenuItem("Models Replace Dialog")) {
					cdeclcall(void, 0x42BF00); // CMainFrame::OnReplaceModels
				}

				// no clue what that does
				if (ImGui::MenuItem("Cycle Preview Models", ggui::hotkey_dialog::get_hotkey_for_command("TogglePreviewModels").c_str())) {
					mainframe_thiscall(void, 0x42BDD0); // CMainFrame::OnMiscCyclePreviewModels
				}

				ImGui::EndMenu(); // Misc
			}

			if (ImGui::BeginMenu("Region"))
			{
				if (ImGui::MenuItem("Region Off")) {
					cdeclcall(void, 0x4252B0); // CMainFrame::OnRegionOff
				} TT("turn off region filter");

				if (ImGui::MenuItem("Set XY")) {
					cdeclcall(void, 0x4252F0); // CMainFrame::OnRegionSetxy
				}

				if (ImGui::MenuItem("Set Tall Brush")) {
					cdeclcall(void, 0x4252E0); // CMainFrame::OnRegionSettallbrush
				} TT("hide everything thats not within the selection brush\nbrush selection stretches across the Z axis");

				if (ImGui::MenuItem("Set Brush")) {
					cdeclcall(void, 0x4252C0); // CMainFrame::OnRegionSetbrush
				} TT("hide everything thats not within the selection brush");

				if (ImGui::MenuItem("Set Selected Brush")) {
					cdeclcall(void, 0x4252D0); // CMainFrame::OnRegionSetselection
				}

				ImGui::EndMenu(); // Region
			}

			if (ImGui::BeginMenu("Brush"))
			{
				if (ImGui::MenuItem("Arbitrary Sided Cylinder")) {
					cdeclcall(void, 0x424EE0); // CMainFrame::OnBrushArbitrarysided
				}

				if (ImGui::MenuItem("Cone")) {
					cdeclcall(void, 0x429170); // CMainFrame::OnBrushMakecone
				}

				if (ImGui::MenuItem("Sphere")) {
					cdeclcall(void, 0x42B630); // CMainFrame::OnBrushPrimitivesSphere
				}

				if (ImGui::MenuItem("Box")) {
					cdeclcall(void, 0x429200); // CMainFrame::OnMakePhysBox
				}

				if (ImGui::MenuItem("Cylinder")) {
					cdeclcall(void, 0x4291D0); // CMainFrame::OnMakePhysCylinder
				}

				ImGui::EndMenu(); // Brush
			}

			if (ImGui::BeginMenu("Patch"))
			{
				//if (ImGui::MenuItem("Simple Curve Patch")) {
				//	cdeclcall(void, 0x429A20); // CMainFrame::OnCurveSimplepatchmesh
				//}

				if (ImGui::MenuItem("Simple Curve Patch")) {
					GET_GUI(ggui::curve_patch_dialog)->toggle();
				}

				//if (ImGui::MenuItem("Simple Terrain Patch")) {
				//	cdeclcall(void, 0x429DA0); // CMainFrame::OnTerrainSimplepatchmesh
				//}

				if (ImGui::MenuItem("Simple Terrain Patch")) {
					GET_GUI(ggui::terrain_patch_dialog)->toggle();
				}

				if (ImGui::MenuItem("Curve To Terrain")) {
					cdeclcall(void, 0x429B30); // CMainFrame::OnCurveToTerrain
				}

				if (ImGui::MenuItem("Faces To Terrain")) {
					cdeclcall(void, 0x429BE0); // CMainFrame::OnFaceToTerrain
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Primitives"))
				{
					if (ImGui::MenuItem("Bevel")) {
						cdeclcall(void, 0x42A450); // CMainFrame::OnCurvePatchbevel
					}

					if (ImGui::MenuItem("Square Bevel")) {
						cdeclcall(void, 0x42B5E0); // CMainFrame::OnCuveSquareBevel
					}

					if (ImGui::MenuItem("End Cap")) {
						cdeclcall(void, 0x42A4A0); // CMainFrame::OnCurvePatchendcap
					}

					if (ImGui::MenuItem("Square End Cap")) {
						cdeclcall(void, 0x42B590); // CMainFrame::OnCurveSquareEndCap
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Cylinder")) {
						cdeclcall(void, 0x42A3B0); // CMainFrame::OnCurvePatchtube
					}

					if (ImGui::MenuItem("Dense Cylinder")) {
						cdeclcall(void, 0x42AB90); // CMainFrame::OnCurvePatchdensetube
					}

					if (ImGui::MenuItem("Very Dense Cylinder")) {
						cdeclcall(void, 0x42AC40); // CMainFrame::OnCurvePatchverydensetube
					}

					if (ImGui::MenuItem("Square Cylinder")) {
						cdeclcall(void, 0x42AF00); // CMainFrame::OnCurvePatchsquare
					}

					if (ImGui::MenuItem("Cone")) {
						cdeclcall(void, 0x42A360); // CMainFrame::OnCurvePatchcone

						// fix cone not showing before moving or editing the patch in any way afterwards
						const auto b = game::g_selected_brushes();
						if (b && b->def && b->def->patch)
						{
							game::Patch_UpdateSelected(b->def->patch, 1);
						}
					}

					// not implemented
					//if (ImGui::MenuItem("Sphere")) {
					//	cdeclcall(void, 0x1); // CMainFrame::
					//}

					ImGui::EndMenu(); // Primitives
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Vert Edit Dialog", ggui::hotkey_dialog::get_hotkey_for_command("VertEdit").c_str())) {
					cdeclcall(void, 0x42BCD0); // CMainFrame::OnVertexEditDlg
				}

				if (ImGui::MenuItem("Advanced Edit Dialog", ggui::hotkey_dialog::get_hotkey_for_command("AdvancedCurveEdit").c_str())) {
					cdeclcall(void, 0x42BC90); // CMainFrame::OnAdvancedEditDlg
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Insert"))
				{
					if (ImGui::MenuItem("Insert 2 Columns")) {
						cdeclcall(void, 0x42A740); // CMainFrame::OnCurveInsertInsertcolumn
					}

					if (ImGui::MenuItem("Insert 2 Rows")) {
						cdeclcall(void, 0x42A790); // CMainFrame::OnCurveInsertInsertrow
					}

					if (ImGui::MenuItem("Add 2 Columns", ggui::hotkey_dialog::get_hotkey_for_command("IncPatchColumn").c_str())) {
						cdeclcall(void, 0x42A6A0); // CMainFrame::OnCurveInsertAddcolumn
					}

					if (ImGui::MenuItem("Add 2 Rows", ggui::hotkey_dialog::get_hotkey_for_command("IncPatchRow").c_str())) {
						cdeclcall(void, 0x42A6F0); // CMainFrame::OnCurveInsertAddrow
					}

					if (ImGui::MenuItem("Add Terrain Row / Column", ggui::hotkey_dialog::get_hotkey_for_command("AddTerrainRow").c_str())) {
						cdeclcall(void, 0x42B080); // CMainFrame::OnAddTerrainRowColumn
					}

					if (ImGui::MenuItem("Extrude Terrain Row", ggui::hotkey_dialog::get_hotkey_for_command("ExtrudeTerrainRow").c_str())) {
						cdeclcall(void, 0x42B0A0); // CMainFrame::ExtrudeTerrainRow
					}

					ImGui::EndMenu(); // Insert
				}

				if (ImGui::BeginMenu("Delete"))
				{
					if (ImGui::MenuItem("First 2 Columns", ggui::hotkey_dialog::get_hotkey_for_command("DecPatchColumn").c_str())) {
						cdeclcall(void, 0x42A810); // CMainFrame::OnCurveDeleteFirstcolumn
					}

					if (ImGui::MenuItem("First 2 Rows", ggui::hotkey_dialog::get_hotkey_for_command("DecPatchRow").c_str())) {
						cdeclcall(void, 0x42A860); // CMainFrame::OnCurveDeleteFirstrow
					}

					if (ImGui::MenuItem("Last 2 Columns")) {
						cdeclcall(void, 0x42A8B0); // CMainFrame::OnCurveDeleteLastcolumn
					}

					if (ImGui::MenuItem("Last 2 Rows")) {
						cdeclcall(void, 0x42A900); // CMainFrame::OnCurveDeleteLastrow
					}

					if (ImGui::MenuItem("Remove Terrain Row / Column", ggui::hotkey_dialog::get_hotkey_for_command("RemoveTerrainRow").c_str())) {
						cdeclcall(void, 0x42B0B0); // CMainFrame::OnRemoveTerrainRowColumn
					}

					ImGui::EndMenu(); // Delete
				}

				if (ImGui::BeginMenu("Matrix"))
				{
					if (ImGui::MenuItem("Transpose", ggui::hotkey_dialog::get_hotkey_for_command("PatchMatrixTranspose").c_str())) {
						cdeclcall(void, 0x42B1E0); // CMainFrame::OnCurveMatrixTranspose
					}

					if (ImGui::MenuItem("Re-disperse Vertices", ggui::hotkey_dialog::get_hotkey_for_command("RedisperseVertices").c_str())) {
						cdeclcall(void, 0x42A270); // CMainFrame::OnRedistPatchPoints
					}

					if (ImGui::MenuItem("Re-disperse Columns", ggui::hotkey_dialog::get_hotkey_for_command("RedisperseCols").c_str())) {
						cdeclcall(void, 0x42AD80); // CMainFrame::OnCurveRedisperseCols
					}

					if (ImGui::MenuItem("Re-disperse Rows", ggui::hotkey_dialog::get_hotkey_for_command("RedisperseRows").c_str())) {
						cdeclcall(void, 0x42AD90); // CMainFrame::OnCurveRedisperseRows
					}

					ImGui::EndMenu(); // Matrix
				}

				if (ImGui::BeginMenu("Weld"))
				{
					if (ImGui::MenuItem("Connect Selection / Weld", ggui::hotkey_dialog::get_hotkey_for_command("ConnectSelection").c_str())) {
						cdeclcall(void, 0x425510); // CMainFrame::OnSelectionConnect
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Weld to Model Vertices", 0, prefs->m_bVertSnapModel)) {
						mainframe_thiscall(void, 0x42A180); // CMainFrame::OnVertSnapModel
					} TT("Allows welding of patch verts to model vertices");

					if (ImGui::MenuItem("Weld to Brush Vertices", 0, prefs->m_bVertSnapBrush)) {
						mainframe_thiscall(void, 0x42A1D0); // CMainFrame::OnVertSnapBrush
					} TT("Allows welding of patch verts to brush vertices");

					if (ImGui::MenuItem("Weld to Prefab Vertices", 0, prefs->m_bVertSnapPrefab)) {
						mainframe_thiscall(void, 0x42A220); // CMainFrame::OnVertSnapPrefab
					} TT("Allows welding of patch verts to prefab vertices");

					ImGui::EndMenu(); // Weld
				}

				if (ImGui::MenuItem("Inc Subdivision", ggui::hotkey_dialog::get_hotkey_for_command("OverBrightShiftUp").c_str())) {
					cdeclcall(void, 0x428EB0); // CMainFrame::OnOverBrightShiftUp
				} TT("Curve Patches: increase vertex count (subdivide)");

				if (ImGui::MenuItem("Dec Subdivision", ggui::hotkey_dialog::get_hotkey_for_command("OverBrightShiftDown").c_str())) {
					cdeclcall(void, 0x428EE0); // CMainFrame::OnOverBrightShiftDown
				} TT("Curve Patches: decrease vertex count (decimate)");

				if (ImGui::MenuItem("Split", ggui::hotkey_dialog::get_hotkey_for_command("SplitPatch").c_str())) {
					cdeclcall(void, 0x42B0C0); // CMainFrame::OnSplitPatch
				}

				// TODO: add 'InvertCurveTextureX' and 'InvertCurveTextureY'
				if (ImGui::MenuItem("Invert", ggui::hotkey_dialog::get_hotkey_for_command("InvertCurve").c_str())) {
					cdeclcall(void, 0x42A7E0); // CMainFrame::OnCurveNegative
				}

				if (ImGui::MenuItem("Thicken", ggui::hotkey_dialog::get_hotkey_for_command("ThickenPatch").c_str())) {
					cdeclcall(void, 0x42B0D0); // CMainFrame::OnCurveThicken
				}

				if (ImGui::MenuItem("Cap", ggui::hotkey_dialog::get_hotkey_for_command("CapCurrentCurve").c_str())) {
					cdeclcall(void, 0x42AD40); // CMainFrame::OnCurveCap
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Turn Terrain Edges")) {
					cdeclcall(void, 0x4294E0); // CMainFrame::OnTurnTerrainEdges
				}

				if (ImGui::MenuItem("Lock Vertices", ggui::hotkey_dialog::get_hotkey_for_command("ToggleLockPatchVertices").c_str())) {
					mainframe_thiscall(void, 0x42B4F0); // CMainFrame::ToggleLockPatchVertMode
				}

				if (ImGui::MenuItem("Unlock Vertices", ggui::hotkey_dialog::get_hotkey_for_command("ToggleUnlockPatchVertices").c_str())) {
					mainframe_thiscall(void, 0x42B510); // CMainFrame::ToggleUnlockPatchVertMode
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Cycle Cap Texture", ggui::hotkey_dialog::get_hotkey_for_command("CycleCapTexturePatch").c_str())) {
					cdeclcall(void, 0x42B1A0); // CMainFrame::OnCurveCyclecap
				}

				if (ImGui::MenuItem("Cap Texture", ggui::hotkey_dialog::get_hotkey_for_command("ApplyPatchCap").c_str())) {
					cdeclcall(void, 0x42AE50); // CMainFrame::OnPatchCap
				}

				ImGui::EndMenu(); // Patch
			}

			imgui::TextUnformatted("|");

			if (ImGui::MenuItem("Help"))
			{
				GET_GUI(help_dialog)->toggle();
			}

			const char* about_str = game::glob::gh_update_avail ? "Update Available" : "About";

			if(game::glob::gh_update_avail)
			{
				ImGui::PushFontFromIndex(ggui::BOLD_18PX);
			}

			if (ImGui::MenuItem(about_str))
			{
				GET_GUI(about_dialog)->toggle();
			}

			if (game::glob::gh_update_avail)
			{
				ImGui::PopFont();
			}

			//if (ImGui::BeginMenu("Help"))
			//{
			//	//if (ImGui::MenuItem("Command List")) {
			//	//	cdeclcall(void, 0x426E00); // CMainFrame::OnHelpCommandlist
			//	//}

			//	if (ImGui::MenuItem("About / Version"))
			//	{
			//		GET_GUI(about_dialog)->toggle();
			//	}

			//	ImGui::EndMenu(); // Help
			//}

			ImGui::EndGroup(); // used to calculate total width below

			/*if (dvars::gui_menubar_show_mouseorigin && dvars::gui_menubar_show_mouseorigin->current.enabled)
			{
				const auto menubar_width = ImGui::GetItemRectSize().x + 24.0f;
				const auto gridpos_text_width = ImGui::CalcTextSize(cmainframe::activewnd->m_strStatus[1]).x;

				RECT _rect;
				GetClientRect(cmainframe::activewnd->GetWindow(), &_rect);
				const int mainframe_width = _rect.right - _rect.left;

				if (mainframe_width >= menubar_width + gridpos_text_width + 8.0f)
				{
					if (cmainframe::activewnd->m_strStatus[1])
					{
						ImGui::SameLine(ImGui::GetWindowWidth() - gridpos_text_width - 8.0f);
						ImGui::TextUnformatted(cmainframe::activewnd->m_strStatus[1]);
					}
				}
			}*/

			const auto process = components::process::get();
			const bool active_proc = process->is_active();

			const auto menubar_height = ImGui::GetItemRectSize().y;
			float proc_str_width = 0.0f;

			if (active_proc)
			{
				const auto tb = GET_GUI(ggui::toolbar_dialog);

				ImVec4 toolbar_button_background_active = ImGui::ToImVec4(dvars::gui_menubar_bg_color->current.vector);
				ImVec4 toolbar_button_background_hovered = toolbar_button_background_active + ImVec4(0.10f, 0.1f, 0.1f, 0.0f);
				ImVec2 toolbar_button_size = ImVec2(menubar_height, menubar_height);

				switch(process->get_indicator_type())
				{
					case components::process::INDICATOR_TYPE_PROGRESS:
						{
							const float progress_width = 200.0f;

							const char* proc_str = process->m_indicator_str.empty() ? "Spawning Process" : process->m_indicator_str.c_str();
							proc_str_width = ImGui::CalcTextSize(proc_str).x + progress_width;

							// not using a group to calculate the widget size because it flickers upon text change (frame delay)

							ImGui::SameLine(ImGui::GetWindowWidth() - proc_str_width - menubar_height);

							static bool hov_active_proc;
							if (tb->image_button_label(proc_str
								, "fx_stop"
								, true
								, hov_active_proc
								, "Kill active process."
								, &toolbar_button_background_hovered
								, &toolbar_button_background_active
								, &toolbar_button_size))
							{
								process->kill_process();
							}

							ImVec4 progress_color = ImVec4(0.49f, 0.2f, 0.2f, 1.0f);
							ImVec4 progress_bg_color = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

							ImGui::SameLine(ImGui::GetWindowWidth() - progress_width + 8.0f);
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
							ImGui::BufferingBar("##buffer_bar", process->m_indicator_progress, ImVec2(progress_width, 6.0f),
								ImGui::ColorConvertFloat4ToU32(progress_bg_color), 
								ImGui::ColorConvertFloat4ToU32(progress_color));

							break;
						}

					default:
					case components::process::INDICATOR_TYPE_SPINNER:
						{
							ImVec4 spinner_color = ImVec4(0.49f, 0.2f, 0.2f, 1.0f);

							const char* proc_str = process->m_indicator_str.empty() ? "Spawning Process" : process->m_indicator_str.c_str();
							proc_str_width = ImGui::CalcTextSize(proc_str).x + 32.0f;

							// not using a group to calculate the widget size because it flickers upon text change (frame delay)

							ImGui::SameLine(ImGui::GetWindowWidth() - proc_str_width - menubar_height);

							static bool hov_active_proc;
							if (tb->image_button_label(proc_str
								, "fx_stop"
								, true
								, hov_active_proc
								, "Kill active process."
								, &toolbar_button_background_hovered
								, &toolbar_button_background_active
								, &toolbar_button_size))
							{
								process->kill_process();
							}

							ImGui::SameLine(ImGui::GetWindowWidth() - 26.0f);
							ImGui::Spinner("##proc_spinner", 6.0, 2.0f, ImGui::ColorConvertFloat4ToU32(spinner_color));

							break;
						}
				}
			}

			if (!game::glob::is_loading_map)
			{
				const auto& tw = components::time_wasted::get();

				if (const auto& entry = tw->get_entry(tw->get_map_string());
					entry)
				{
					const char* timer_str = utils::va("Time wasted: %dh : %dm", entry->time / 60, entry->time % 60);
					const float str_width = imgui::CalcTextSize(timer_str).x;
					imgui::SameLine(imgui::GetWindowWidth() - proc_str_width - str_width - menubar_height - (proc_str_width != 0.0f ? 20.0f : 0.0f));

					imgui::SetCursorPosY(imgui::GetCursorPosY() - 3.0f);
					imgui::TextUnformatted(timer_str);
				}
			}

			imgui::EndMenuBar();
		}

		ggui::context_menu_style_end();
	}

	REGISTER_GUI(menubar_dialog);
}
