#include "std_include.hpp"

namespace ggui::menubar
{
	/*const float GRID_SIZES[11] =
	{ 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f, 256.0f, 512.0f };

	enum E_GRID_SIZES : int
	{
		GRID_05,
		GRID_1,
		GRID_2,
		GRID_4,
		GRID_8,
		GRID_16,
		GRID_32,
		GRID_64,
		GRID_128,
		GRID_256,
		GRID_512,
	};*/

	
	void set_grid_size(const xywnd::E_GRID_SIZES size)
	{
		game::g_qeglobals->d_gridsize = size;
		if (game::g_PrefsDlg()->m_bSnapTToGrid)
		{
			game::g_qeglobals->d_savedinfo.d_gridsize_float = xywnd::GRID_SIZES[size];
		}

		mainframe_thiscall(void, 0x428A00); // CMainFrame::SetGridStatus
		game::g_nUpdateBits |= W_Z | W_XY;
	}


	enum RENDER_METHOD_E : int
	{
		RM_WIREFRAME,
		RM_FULLBRIGHT,
		RM_NORMALFAKELIGHT,
		RM_VIEWFAKELIGHT,
		RM_CASETEXTURES,
	};

	
	void set_render_method(const RENDER_METHOD_E meth)
	{
		switch (meth)
		{
		case RM_WIREFRAME: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80DE; break;
		case RM_FULLBRIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80DF; break;
		case RM_NORMALFAKELIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E0; break;
		case RM_VIEWFAKELIGHT: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E1; break;
		case RM_CASETEXTURES: game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E2; break;
		}

		if (cmainframe::activewnd->m_pCamWnd->camera.draw_mode != meth)
		{
			cmainframe::activewnd->m_pCamWnd->camera.draw_mode = meth;
			game::g_nUpdateBits |= W_CAMERA;
		}
	}

	
	void set_texture_resolution(int picmip)
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

	
	void menu(ggui::imgui_context_cz& context)
	{
		const auto prefs = game::g_PrefsDlg();
		
		// *
		// menu bar within dockspace
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 6));

		if (ImGui::BeginMenuBar())
		{
			ImGui::BeginGroup();
			
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Map")) {
					mainframe_cdeclcall(void, 0x423AA0); //cmainframe::OnFileNew
				}

				if (ImGui::MenuItem("Open", hotkeys::get_hotkey_for_command("FileOpen").c_str())) {
					mainframe_cdeclcall(void, 0x423AE0); //cmainframe::OnFileOpen
				}

				if (ImGui::MenuItem("Save", hotkeys::get_hotkey_for_command("FileSave").c_str())) {
					mainframe_cdeclcall(void, 0x423B80); //cmainframe::OnFileSave
				}

				if (ImGui::MenuItem("Save As")) {
					mainframe_cdeclcall(void, 0x423BC0); //cmainframe::OnFileSaveas
				}

				if (ImGui::MenuItem("Save Selected")) {
					mainframe_thiscall(void, 0x4293A0); //cmainframe::OnFileExportmap
				}

				SEPERATORV(0.0f);

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

				if (ImGui::BeginMenu("Misc"))
				{
					if (ImGui::MenuItem("Save Region", "", nullptr, game::g_region_active)) {
						mainframe_cdeclcall(void, 0x429020); //cmainframe::OnFileSaveregion
					}

					if (ImGui::MenuItem("Pointfile")) {
						mainframe_cdeclcall(void, 0x423B20); //cmainframe::OnPointfileOpen
					}

					if (ImGui::MenuItem("Errorfile")) {
						mainframe_cdeclcall(void, 0x423B40); //cmainframe::OnErrorFile
					}

					ImGui::EndMenu(); // Misc
				}

				if (ImGui::BeginMenu("Project Settings"))
				{
					if (ImGui::MenuItem("New Project")) {
						mainframe_cdeclcall(void, 0x426E80); //cmainframe::OnFileNewproject
					}

					if (ImGui::MenuItem("Set Startup Project")) {
						mainframe_cdeclcall(void, 0x427010); //cmainframe::OnSetStartupProject
					}

					if (ImGui::MenuItem("Project Settings")) {
						mainframe_cdeclcall(void, 0x428DE0); //cmainframe::OnFileProjectsettings
					}

					ImGui::EndMenu(); // Project Settings
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Exit", hotkeys::get_hotkey_for_command("Quit").c_str()))
				{
					PostMessage(cmainframe::activewnd->GetWindow(), WM_CLOSE, 0, 0L);
				}

				ImGui::EndMenu(); // File
			}

			// ----------------------------

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", hotkeys::get_hotkey_for_command("Undo").c_str(), nullptr, game::g_lastundo() && game::g_lastundo()->done)) {
					cdeclcall(void, 0x428730); //cmainframe::OnEditUndo / OnUpdateEditUndo
				}

				if (ImGui::MenuItem("Redo", hotkeys::get_hotkey_for_command("Redo").c_str(), nullptr, game::g_lastredo())) {
					cdeclcall(void, 0x428740); //cmainframe::OnEditRedo / OnUpdateEditRedo
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Copy", hotkeys::get_hotkey_for_command("Copy").c_str())) {
					mainframe_thiscall(void, 0x4286B0); //cmainframe::OnEditCopybrush
				}

				if (ImGui::MenuItem("Paste", hotkeys::get_hotkey_for_command("Paste").c_str())) {
					mainframe_thiscall(void, 0x4286D0); //cmainframe::OnEditPastebrush
				}

				if (ImGui::MenuItem("Delete", hotkeys::get_hotkey_for_command("DeleteSelection").c_str())) {
					cdeclcall(void, 0x425690); //cmainframe::OnSelectionDelete
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Map Info", hotkeys::get_hotkey_for_command("MapInfo").c_str())) {
					cdeclcall(void, 0x426C60); //cmainframe::OnEditMapinfo
				}

				if (ImGui::MenuItem("Entity Info")) {
					cdeclcall(void, 0x426D40); //cmainframe::OnEditEntityinfo
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Enter Prefab", hotkeys::get_hotkey_for_command("EnterPrefab").c_str())) {
					cdeclcall(void, 0x42BF70); //cmainframe::OnPrefabEnter
				}

				if (ImGui::MenuItem("Leave Prefab", hotkeys::get_hotkey_for_command("LeavePrefab").c_str())) {
					cdeclcall(void, 0x42BF80); //cmainframe::OnPrefabLeave
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Gui Settings"))
				{
					if(dvars::gui_floating_toolbar && dvars::gui_resize_dockspace)
					{
						if (ImGui::MenuItem("Floating Toolbar", 0, dvars::gui_floating_toolbar->current.enabled)) 
						{
							dvars::set_bool(dvars::gui_floating_toolbar, dvars::gui_floating_toolbar->current.enabled ? false : true);
							ggui::toolbar_reset = true;
							
						} TT(dvars::gui_floating_toolbar->description);
						
						if (ImGui::MenuItem("^ Resize Dockspace", 0, dvars::gui_resize_dockspace->current.enabled))
						{
							dvars::set_bool(dvars::gui_resize_dockspace, dvars::gui_resize_dockspace->current.enabled ? false : true);
						} TT(dvars::gui_resize_dockspace->description);

						if(ImGui::MenuItem("Reset Dockspace"))
						{
							ggui::reset_dockspace = true; // used to trigger dockspace rebuilding (cxywnd_gui)
						}
					}
					
					ImGui::EndMenu(); // Gui Settings
				}

				if (ImGui::MenuItem("Edit Colors ...")) {
					components::gui::toggle(context.m_colors, 0, true);
				}
				
				if (ImGui::MenuItem("Edit Toolbar ...")) {
					components::gui::toggle(context.m_toolbar_edit, 0, true);
				}

				if (ImGui::MenuItem("Edit Hotkeys ...")) {
					components::gui::toggle(context.m_cmdbinds, 0, true);
				}

				if (ImGui::MenuItem("Console")) {
					components::gui::toggle(context.m_console, 0, true);
				}

				if (ImGui::MenuItem("Filter")) {
					components::gui::toggle(context.m_filter, 0, true);
				}

				if (ImGui::MenuItem("Entity")) {
					components::gui::toggle(context.m_entity, 0, true);
				}

				if (ImGui::MenuItem("Demo")) {
					components::gui::toggle(context.m_demo, 0, true);
				}

				if (ImGui::MenuItem("Preferences ImGui")) {
					components::gui::toggle(context.m_preferences, 0, true);
				}

				if (ImGui::MenuItem("Model Selector")) {
					const auto m_selector = ggui::get_rtt_modelselector();
					m_selector->menustate = !m_selector->menustate;
				}
				
				if (ImGui::MenuItem("Preferences ...", hotkeys::get_hotkey_for_command("Preferences").c_str())) {
					mainframe_thiscall(void, 0x426950); //cmainframe::OnPrefs
				}

				ImGui::EndMenu(); // Edit
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::BeginMenu("Toggle"))
				{
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

					if (ImGui::MenuItem("Camera View (Original)", hotkeys::get_hotkey_for_command("ToggleCamera").c_str(), nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426A40); // cmainframe::OnTogglecamera
					}

					if (ImGui::MenuItem("XY View (Original)", hotkeys::get_hotkey_for_command("ToggleView").c_str(), nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426AE0); // cmainframe::OnToggleview
					}

					if (ImGui::MenuItem("Textures (Original)", 0, false, cmainframe::is_combined_view())) {
						ShowWindow(cmainframe::activewnd->m_pTexWnd->GetWindow(), SW_HIDE);
					}

					if (ImGui::MenuItem("Console View", hotkeys::get_hotkey_for_command("ViewConsole").c_str(), nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426A90); // cmainframe::OnToggleconsole
					}

					// this would be stupid
					//if (ImGui::MenuItem("Z View", hotkeys::get_hotkey_for_command("ToggleZ").c_str(), nullptr, cmainframe::is_combined_view())) {
					//	mainframe_thiscall(void, 0x426B30); // cmainframe::OnTogglez
					//}

					if (ImGui::MenuItem("XY Crosshair", hotkeys::get_hotkey_for_command("ToggleCrosshairs").c_str(), game::g_bCrossHairs)) {
						game::g_bCrossHairs ^= 1;
					}

					if (ImGui::MenuItem("Selected Outlines", hotkeys::get_hotkey_for_command("ToggleOutlineDraw").c_str(), !game::g_qeglobals->dontDrawSelectedOutlines)) {
						game::g_qeglobals->dontDrawSelectedOutlines ^= 1;
					}

					if (ImGui::MenuItem("Selected Tint", hotkeys::get_hotkey_for_command("ToggleTintDraw").c_str(), !game::g_qeglobals->dontDrawSelectedTint)) {
						game::g_qeglobals->dontDrawSelectedTint ^= 1;
					}

					if (ImGui::MenuItem("Entity View", hotkeys::get_hotkey_for_command("ViewEntityInfo").c_str())) {
						mainframe_thiscall(void, 0x423F00); // cmainframe::OnViewEntity
					}

					if (ImGui::MenuItem("Layers", hotkeys::get_hotkey_for_command("ToggleLayers").c_str())) {
						cdeclcall(void, 0x42BD10); // cmainframe::OnLayersDlg
					}

					if (ImGui::MenuItem("Filter Settings", hotkeys::get_hotkey_for_command("ViewFilters").c_str())) {
						mainframe_thiscall(void, 0x42B7A0); // cmainframe::OnFilterDlg
					}

					if (ImGui::MenuItem("Textures", hotkeys::get_hotkey_for_command("ViewTextures").c_str())) {
						components::gui::toggle(ggui::get_rtt_texturewnd(), 0, true);
					}

					ImGui::EndMenu(); // Toggle
				}

				if (ImGui::BeginMenu("Background"))
				{
					if (ImGui::MenuItem("None", 0, dvars::gui_mainframe_background->current.integer == 0)) {
						dvars::set_int(dvars::gui_mainframe_background, 0);
					}

					if (ImGui::MenuItem("Grid", 0, dvars::gui_mainframe_background->current.integer == 1)) {
						dvars::set_int(dvars::gui_mainframe_background, 1);
					}

					if (ImGui::MenuItem("Camera", 0, dvars::gui_mainframe_background->current.integer == 2)) {
						dvars::set_int(dvars::gui_mainframe_background, 2);
					}
					
					ImGui::EndMenu(); // Background
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Center", hotkeys::get_hotkey_for_command("CenterView").c_str())) {
					mainframe_thiscall(void, 0x423C50); // cmainframe::OnViewCenter
				}

				if (ImGui::MenuItem("Center 2D On Camera", hotkeys::get_hotkey_for_command("Center2DOnCamera").c_str())) {
					cdeclcall(void, 0x42A2D0); // cmainframe::OnCenter2DOnCamera
				}

				if (ImGui::MenuItem("Up Floor", hotkeys::get_hotkey_for_command("UpFloor").c_str())) {
					mainframe_thiscall(void, 0x424700); // cmainframe::OnViewUpfloor
				} TT("move up one floor, if there is one");

				if (ImGui::MenuItem("Down Floor", hotkeys::get_hotkey_for_command("DownFloor").c_str())) {
					mainframe_thiscall(void, 0x423ED0); // cmainframe::OnViewDownfloor
				} TT("move down one floor, if there is one");

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Next (XY, YZ, XZ)", hotkeys::get_hotkey_for_command("NextView").c_str())) {
					mainframe_thiscall(void, 0x426DB0); // cmainframe::OnViewNextview
				}

				if (ImGui::BeginMenu("XY Window"))
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

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Show Names", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 8) == 0)) {
						mainframe_thiscall(void, 0x42BA40); // cmainframe::OnSelectNames
					}

					if (ImGui::MenuItem("Show Angles", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 2) == 0)) {
						mainframe_thiscall(void, 0x42BAA0); // cmainframe::OnSelectAngles
					}

					if (ImGui::MenuItem("Show Grid Blocks", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 16) == 0)) {
						mainframe_thiscall(void, 0x42BB00); // cmainframe::OnSelectBlocks
					}

					if (ImGui::MenuItem("Show Connections", 0, (game::g_qeglobals->d_savedinfo.d_xyShowFlags & 4) == 0)) {
						mainframe_thiscall(void, 0x42BBC0); // cmainframe::OnSelectConnections
					}

					if (ImGui::MenuItem("Draw Model Origins", 0, dvars::r_draw_model_origin->current.enabled)) {
						dvars::set_bool(dvars::r_draw_model_origin, !dvars::r_draw_model_origin->current.enabled);
					}

					if (ImGui::MenuItem("Game View", 0, dvars::radiant_gameview->current.enabled)) 
					{
						components::gameview::p_this->set_state(!dvars::radiant_gameview->current.enabled);
					}

					ImGui::EndMenu(); // XY Window
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Hide Selected", hotkeys::get_hotkey_for_command("HideSelected").c_str())) {
					cdeclcall(void, 0x42B6A0); // cmainframe::OnHideSelected
				}

				if (ImGui::MenuItem("Hide Unselected", hotkeys::get_hotkey_for_command("HideUnSelected").c_str())) {
					cdeclcall(void, 0x42B6C0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Last Hidden", hotkeys::get_hotkey_for_command("ShowLastHidden").c_str())) {
					cdeclcall(void, 0x42B6E0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Hidden", hotkeys::get_hotkey_for_command("ShowHidden").c_str())) {
					cdeclcall(void, 0x42B6D0); // cmainframe::OnShowHidden
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Show Patches As"))
				{
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

				if (ImGui::BeginMenu("Light Preview"))
				{
					if (ImGui::MenuItem("Enable Light Preview", hotkeys::get_hotkey_for_command("LightPreviewToggle").c_str(), prefs->enable_light_preview)) {
						mainframe_thiscall(void, 0x4240C0); // cmainframe::OnEnableLightPreview
					}

					if (ImGui::MenuItem("Enable Sun Preview", hotkeys::get_hotkey_for_command("LightPreviewSun").c_str(), prefs->preview_sun_aswell)) {
						mainframe_thiscall(void, 0x424060); // cmainframe::OnPreviewSun
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Start Previewing Selected", hotkeys::get_hotkey_for_command("LightPreviewStart").c_str())) {
						mainframe_thiscall(void, 0x424120); // cmainframe::OnStartPreviewSelected
					}

					if (ImGui::MenuItem("Stop Previewing Selected", hotkeys::get_hotkey_for_command("LightPreviewStop").c_str())) {
						mainframe_thiscall(void, 0x424170); // cmainframe::OnStopPreviewSelected
					}

					if (ImGui::MenuItem("Clear Preview List", hotkeys::get_hotkey_for_command("LightPreviewClear").c_str())) {
						mainframe_thiscall(void, 0x4241C0); // cmainframe::OnClearPreviewList
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Preview At Max Intensity", hotkeys::get_hotkey_for_command("MaxLightIntensity").c_str(), !game::g_qeglobals->preview_at_max_intensity)) {
						cdeclcall(void, 0x425670); // cmainframe::OnPreviewAtMaxIntensity
					}

					if (ImGui::MenuItem("Show Regions For Selected", hotkeys::get_hotkey_for_command("LightPreviewRegions").c_str())) {
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

				IMGUI_MENU_WIDGET_SINGLE("Cubic Scale", ImGui::DragInt("", &prefs->m_nCubicScale, 1, 1, 220));

				if (ImGui::MenuItem("Cubic Clipping", hotkeys::get_hotkey_for_command("ToggleCubicClip").c_str(), prefs->m_bCubicClipping)) {
					mainframe_thiscall(void, 0x428F90); // cmainframe::OnViewCubicclipping
				}

				ImGui::EndMenu(); // View
			}

			if (ImGui::BeginMenu("Selection"))
			{
				if (ImGui::MenuItem("Drag Edges", hotkeys::get_hotkey_for_command("DragEdges").c_str())) {
					cdeclcall(void, 0x4257D0); // CMainFrame::OnSelectionDragedges
				}

				if (ImGui::MenuItem("Drag Vertices", hotkeys::get_hotkey_for_command("DragVertices").c_str())) {
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

					if (ImGui::MenuItem("Select By Key/Value", hotkeys::get_hotkey_for_command("SelectionKeyValue").c_str())) {
						cdeclcall(void, 0x4263B0); // CMainFrame::OnSelectionKeyValue
					}

					if (ImGui::MenuItem("Select Connected", hotkeys::get_hotkey_for_command("SelectConnectedEntities").c_str())) {
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

				if (ImGui::MenuItem("Deselect", hotkeys::get_hotkey_for_command("UnSelectSelection").c_str())) {
					mainframe_thiscall(void, 0x425740); // CMainFrame::OnSelectionDeselect
				}

				if (ImGui::MenuItem("Clone", hotkeys::get_hotkey_for_command("CloneSelection").c_str())) {
					cdeclcall(void, 0x425480); // CMainFrame::OnSelectionClone
				}
				
				if (ImGui::MenuItem("Invert", hotkeys::get_hotkey_for_command("InvertSelection").c_str())) {
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

					if (ImGui::MenuItem("Rotate Z", hotkeys::get_hotkey_for_command("RotateZ").c_str())) {
						cdeclcall(void, 0x425220); // CMainFrame::OnBrushRotatez
					}

					if (ImGui::MenuItem("Free Rotation", hotkeys::get_hotkey_for_command("MouseRotate").c_str(), game::g_bRotateMode)) {
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


				if (ImGui::MenuItem("Maya Export")) {
					cdeclcall(void, 0x4263C0); // CMainFrame::OnExportToMaya
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("CSG"))
				{
					if (ImGui::MenuItem("Hollow")) {
						cdeclcall(void, 0x425570); // CMainFrame::OnSelectionMakehollow
					}

					if (ImGui::MenuItem("Merge", hotkeys::get_hotkey_for_command("CSGMerge").c_str())) {
						cdeclcall(void, 0x4255D0); // CMainFrame::OnSelectionCsgmerge
					}

					if (ImGui::MenuItem("Auto Caulk", hotkeys::get_hotkey_for_command("AutoCaulk").c_str())) {
						cdeclcall(void, 0x425600); // CMainFrame::OnSelectionAutoCaulk
					}

					ImGui::EndMenu(); // CSG
				}

				if (ImGui::BeginMenu("Clipper"))
				{
					if (ImGui::MenuItem("Toggle Clipper", hotkeys::get_hotkey_for_command("ToggleClipper").c_str())) {
						mainframe_thiscall(void, 0x426510); // CMainFrame::OnViewClipper
					}

					if (ImGui::MenuItem("Clip Selected", hotkeys::get_hotkey_for_command("ClipSelected").c_str())) {
						mainframe_thiscall(void, 0x427170); // CMainFrame::OnClipSelected
					}

					if (ImGui::MenuItem("Split Selected", hotkeys::get_hotkey_for_command("SplitSelected").c_str())) {
						mainframe_thiscall(void, 0x4271D0); // CMainFrame::OnSplitSelected
					}

					if (ImGui::MenuItem("Flip Clip Orientation", hotkeys::get_hotkey_for_command("FlipClip").c_str())) {
						mainframe_thiscall(void, 0x427140); // CMainFrame::OnFlipClip
					}

					ImGui::EndMenu(); // Clipper
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Connect Entities", hotkeys::get_hotkey_for_command("ConnectSelection").c_str())) {
					cdeclcall(void, 0x425510); // CMainFrame::OnSelectionConnect
				}

				if (ImGui::MenuItem("Link Entities", hotkeys::get_hotkey_for_command("LinkSelected").c_str())) {
					cdeclcall(void, 0x425500); // CMainFrame::OnLinkSelected
				}

				if (ImGui::MenuItem("Ungroup Entity")) {
					cdeclcall(void, 0x426380); // CMainFrame::OnSelectionUngroupentity
				}

				if (ImGui::MenuItem("Cycle Group Selection", hotkeys::get_hotkey_for_command("Patch TAB").c_str())) {
					cdeclcall(void, 0x42A9E0); // CMainFrame::OnPatchTab
				} TT("Cycle between all brushes/patches of a selected group/brushmodel");

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Make Detail", hotkeys::get_hotkey_for_command("MakeDetail").c_str())) {
					cdeclcall(void, 0x4261C0); // CMainFrame::OnSelectionMakeDetail
				}

				if (ImGui::MenuItem("Make Structural", hotkeys::get_hotkey_for_command("MakeStructural").c_str())) {
					cdeclcall(void, 0x426200); // CMainFrame::OnSelectionMakeStructural
				}

				if (ImGui::MenuItem("Make Weapon Clip", hotkeys::get_hotkey_for_command("MakeWeaponClip").c_str())) {
					cdeclcall(void, 0x426240); // CMainFrame::OnSelectionMakeWeaponclip
				}

				if (ImGui::MenuItem("Make Non-Colliding", hotkeys::get_hotkey_for_command("MakeNonColliding").c_str())) {
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
				if (ImGui::MenuItem("Grid 0.5", hotkeys::get_hotkey_for_command("SetGridPointFive").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_05)) {
					set_grid_size(xywnd::GRID_05);
				}

				if (ImGui::MenuItem("Grid 1", hotkeys::get_hotkey_for_command("SetGrid1").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_1)) {
					set_grid_size(xywnd::GRID_1);
				}

				if (ImGui::MenuItem("Grid 2", hotkeys::get_hotkey_for_command("SetGrid2").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_2)) {
					set_grid_size(xywnd::GRID_2);
				}

				if (ImGui::MenuItem("Grid 4", hotkeys::get_hotkey_for_command("SetGrid4").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_4)) {
					set_grid_size(xywnd::GRID_4);
				}

				if (ImGui::MenuItem("Grid 8", hotkeys::get_hotkey_for_command("SetGrid8").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_8)) {
					set_grid_size(xywnd::GRID_8);
				}

				if (ImGui::MenuItem("Grid 16", hotkeys::get_hotkey_for_command("SetGrid16").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_16)) {
					set_grid_size(xywnd::GRID_16);
				}

				if (ImGui::MenuItem("Grid 32", hotkeys::get_hotkey_for_command("SetGrid32").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_32)) {
					set_grid_size(xywnd::GRID_32);
				}

				if (ImGui::MenuItem("Grid 64", hotkeys::get_hotkey_for_command("SetGrid64").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_64)) {
					set_grid_size(xywnd::GRID_64);
				}

				if (ImGui::MenuItem("Grid 128", 0, game::g_qeglobals->d_gridsize == xywnd::GRID_128)) {
					set_grid_size(xywnd::GRID_128);
				}

				if (ImGui::MenuItem("Grid 256", hotkeys::get_hotkey_for_command("SetGrid256").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_256)) {
					set_grid_size(xywnd::GRID_256);
				}

				if (ImGui::MenuItem("Grid 512", hotkeys::get_hotkey_for_command("SetGrid512").c_str(), game::g_qeglobals->d_gridsize == xywnd::GRID_512)) {
					set_grid_size(xywnd::GRID_512);
				}

				if (ImGui::MenuItem("Snap To Grid", hotkeys::get_hotkey_for_command("ToggleSnapToGrid").c_str(), !prefs->m_bNoClamp)) {
					mainframe_thiscall(void, 0x428380); // CMainFrame::OnSnaptogrid
				}

				ImGui::EndMenu(); // Grid
			}


			if (ImGui::BeginMenu("Textures"))
			{
				if (ImGui::MenuItem("Show All", hotkeys::get_hotkey_for_command("ShowAllTextures").c_str())) {
					cdeclcall(void, 0x42B440); // CMainFrame::OnTexturesShowall
				}

				if (ImGui::MenuItem("Show In Use", hotkeys::get_hotkey_for_command("ShowTexturesInUse").c_str())) {
					mainframe_thiscall(void, 0x424B20); // CMainFrame::OnTexturesShowinuse
				}

				if (ImGui::MenuItem("Surface Inspector", hotkeys::get_hotkey_for_command("SurfaceInspector").c_str())) {
					cdeclcall(void, 0x424B60); // CMainFrame::OnTexturesInspector
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Find / Replace")) {
					cdeclcall(void, 0x428B40); // CMainFrame::OnTextureReplaceall
				}

				if (ImGui::BeginMenu("Render Method"))
				{
					if (ImGui::MenuItem("Wireframe", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_WIREFRAME)) {
						set_render_method(RM_WIREFRAME);
					}

					if (ImGui::MenuItem("Fullbright", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_FULLBRIGHT)) {
						set_render_method(RM_FULLBRIGHT);
					}

					if (ImGui::MenuItem("Normal-Based Fake Lighting", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_NORMALFAKELIGHT)) {
						set_render_method(RM_NORMALFAKELIGHT);
					}

					if (ImGui::MenuItem("View-Based Fake Lighting", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_VIEWFAKELIGHT)) {
						set_render_method(RM_VIEWFAKELIGHT);
					}

					if (ImGui::MenuItem("Case Textures", 0, cmainframe::activewnd->m_pCamWnd->camera.draw_mode == RM_CASETEXTURES)) {
						set_render_method(RM_CASETEXTURES);
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Alpha Rendering", 0, prefs->camera_masked)) {
						mainframe_thiscall(void, 0x429F10); // CMainFrame::OnToggleTextureAlphaRendering
					}

					ImGui::EndMenu(); // Render Method
				}

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

				if (ImGui::BeginMenu("Texture Window Scale"))
				{
					if (ImGui::MenuItem("200%", 0, prefs->m_nTextureWindowScale == 200)) {
						mainframe_thiscall(void, 0x42B020); // CMainFrame::OnTexturesTexturewindowscale200
					}

					if (ImGui::MenuItem("100%", 0, prefs->m_nTextureWindowScale == 100)) {
						mainframe_thiscall(void, 0x42B000); // CMainFrame::OnTexturesTexturewindowscale100
					}

					if (ImGui::MenuItem("50%", 0, prefs->m_nTextureWindowScale == 50)) {
						mainframe_thiscall(void, 0x42B060); // CMainFrame::OnTexturesTexturewindowscale50
					}

					if (ImGui::MenuItem("25%", 0, prefs->m_nTextureWindowScale == 25)) {
						mainframe_thiscall(void, 0x42B040); // CMainFrame::OnTexturesTexturewindowscale25
					}

					if (ImGui::MenuItem("10%", 0, prefs->m_nTextureWindowScale == 10)) {
						mainframe_thiscall(void, 0x42AFE0); // CMainFrame::OnTexturesTexturewindowscale10
					}

					ImGui::EndMenu(); // Texture Window Scale
				}

				if (ImGui::BeginMenu("Texture Lock"))
				{
					if (ImGui::MenuItem("Moves", hotkeys::get_hotkey_for_command("ToggleTexMoveLock").c_str(), prefs->m_bTextureLock == 1)) {
						mainframe_thiscall(void, 0x426B80); // CMainFrame::OnToggleLockMoves
					}

					if (ImGui::MenuItem("Rotations", hotkeys::get_hotkey_for_command("ToggleTexRotateLock").c_str(), prefs->m_bRotateLock == 1)) {
						mainframe_thiscall(void, 0x429230); // CMainFrame::OnToggleLockRotations
					}

					if (ImGui::MenuItem("Lightmaps", hotkeys::get_hotkey_for_command("ToggleLightmapLock").c_str(), prefs->m_bLightmapLock == 1)) {
						mainframe_thiscall(void, 0x426BF0); // CMainFrame::OnToggleLockLightmap
					}

					ImGui::EndMenu(); // Texture Lock
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Layered Materials"))
				{
					if (ImGui::MenuItem("Toogle Tool Window", hotkeys::get_hotkey_for_command("ToggleLayeredMaterialWnd").c_str()))
					{
						cdeclcall(void, 0x42BFE0); // CMainFrame::OnToggleLayeredMaterials
					}

					if (ImGui::MenuItem("Save", hotkeys::get_hotkey_for_command("SaveLayeredMaterials").c_str())) {
						cdeclcall(void, 0x42C020); // CMainFrame::OnSaveLayeredMaterials
					}

					ImGui::EndMenu(); // Layered Materials
				}

				if (ImGui::BeginMenu("Edit Layer"))
				{
					if (ImGui::MenuItem("Cycle", hotkeys::get_hotkey_for_command("TexLayerCycle").c_str())) {
						cdeclcall(void, 0x424010); // CMainFrame::OnEditLayerCycle
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Material", hotkeys::get_hotkey_for_command("TexLayerMaterial").c_str(), game::g_qeglobals->current_edit_layer == 0)) {
						cdeclcall(void, 0x424030); // CMainFrame::OnEditLayerMaterial
					}

					if (ImGui::MenuItem("Lightmap", hotkeys::get_hotkey_for_command("TexLayerLightmap").c_str(), game::g_qeglobals->current_edit_layer == 1)) {
						cdeclcall(void, 0x424040); // CMainFrame::OnEditLayerLightmap
					}

					if (ImGui::MenuItem("Smoothing", 0, game::g_qeglobals->current_edit_layer == 2)) {
						cdeclcall(void, 0x424050); // CMainFrame::OnEditLayerSmoothing
					}

					ImGui::EndMenu(); // Edit Layer
				}

				if (ImGui::MenuItem("Refresh Textures", hotkeys::get_hotkey_for_command("RefreshTextures").c_str())) {
					cdeclcall(void, 0x428B50); // CMainFrame::OnTextureRefresh
				}

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
								g_texwnd->nPos = 0; // scroll to top
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
								g_texwnd->nPos = 0; // scroll to top
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
								g_texwnd->nPos = 0; // scroll to top
							}
						}
					}

					ImGui::EndMenu(); // Surface Type Filter
				}

				ImGui::EndMenu(); // Textures
			}


			if (ImGui::BeginMenu("Misc"))
			{
				if (ImGui::MenuItem("Find Brush")) {
					cdeclcall(void, 0x424B80); // CMainFrame::OnMiscFindbrush
				}

				if (ImGui::MenuItem("Got To Position")) {
					cdeclcall(void, 0x424BA0); // CMainFrame::OnMiscGoToPosition
				}

				if (ImGui::MenuItem("-> Leak Spot / Error", hotkeys::get_hotkey_for_command("NextLeakSpot").c_str())) {
					cdeclcall(void, 0x424BC0); // CMainFrame::OnMiscNextleakspot
				}

				if (ImGui::MenuItem("<- Leak Spot / Error", hotkeys::get_hotkey_for_command("PrevLeakSpot").c_str())) {
					cdeclcall(void, 0x424BE0); // CMainFrame::OnMiscPreviousleakspot
				}

				if (ImGui::MenuItem("Print XY View")) {
					cdeclcall(void, 0x424C00); // CMainFrame::OnMiscPrintxy
				}

				if (ImGui::MenuItem("Entity Color Dialog", hotkeys::get_hotkey_for_command("EntityColor").c_str())) {
					cdeclcall(void, 0x424C10); // CMainFrame::OnMiscSelectentitycolor
				}

				if (ImGui::MenuItem("Script Group Dialog")) {
					cdeclcall(void, 0x424E20); // CMainFrame::OnScriptGroup
				}

				if (ImGui::MenuItem("Delete Exportables")) {
					cdeclcall(void, 0x424E30); // CMainFrame::OnDeleteExportables
				}

				if (ImGui::MenuItem("Vehicle Group Dialog", hotkeys::get_hotkey_for_command("VehicleGroup").c_str())) {
					cdeclcall(void, 0x42BD50); // CMainFrame::OnMiscVehicleGroup
				}

				if (ImGui::MenuItem("Dyn Entity Dialog", hotkeys::get_hotkey_for_command("DynEntities").c_str())) {
					cdeclcall(void, 0x42BD90); // CMainFrame::OnMiscDynEntities
				}

				if (ImGui::MenuItem("Models Replace Dialog")) {
					cdeclcall(void, 0x42BF00); // CMainFrame::OnReplaceModels
				}

				// no clue what that does
				if (ImGui::MenuItem("Cycle Preview Models", hotkeys::get_hotkey_for_command("TogglePreviewModels").c_str())) {
					mainframe_thiscall(void, 0x42BDD0); // CMainFrame::OnMiscCyclePreviewModels
				}

				ImGui::EndMenu(); // Misc
			}


			if (ImGui::BeginMenu("Region"))
			{
				if (ImGui::MenuItem("Region Off")) {
					cdeclcall(void, 0x4252B0); // CMainFrame::OnRegionOff
				}

				if (ImGui::MenuItem("Set XY")) {
					cdeclcall(void, 0x4252F0); // CMainFrame::OnRegionSetxy
				}

				if (ImGui::MenuItem("Set Tall Brush")) {
					cdeclcall(void, 0x4252E0); // CMainFrame::OnRegionSettallbrush
				}

				if (ImGui::MenuItem("Set Brush")) {
					cdeclcall(void, 0x4252C0); // CMainFrame::OnRegionSetbrush
				}

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
				if (ImGui::MenuItem("Simple Curve Patch")) {
					cdeclcall(void, 0x429A20); // CMainFrame::OnCurveSimplepatchmesh
				}

				if (ImGui::MenuItem("Simple Terrain Patch")) {
					cdeclcall(void, 0x429DA0); // CMainFrame::OnTerrainSimplepatchmesh
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
					}

					// not implemented
					//if (ImGui::MenuItem("Sphere")) {
					//	cdeclcall(void, 0x1); // CMainFrame::
					//}

					ImGui::EndMenu(); // Primitives
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Advanced Edit Dialog", hotkeys::get_hotkey_for_command("AdvancedCurveEdit").c_str())) {
					cdeclcall(void, 0x42BC90); // CMainFrame::OnAdvancedEditDlg
				}

				if (ImGui::BeginMenu("Insert"))
				{
					if (ImGui::MenuItem("Insert 2 Columns")) {
						cdeclcall(void, 0x42A740); // CMainFrame::OnCurveInsertInsertcolumn
					}

					if (ImGui::MenuItem("Insert 2 Rows")) {
						cdeclcall(void, 0x42A790); // CMainFrame::OnCurveInsertInsertrow
					}

					if (ImGui::MenuItem("Add 2 Columns", hotkeys::get_hotkey_for_command("IncPatchColumn").c_str())) {
						cdeclcall(void, 0x42A6A0); // CMainFrame::OnCurveInsertAddcolumn
					}

					if (ImGui::MenuItem("Add 2 Rows", hotkeys::get_hotkey_for_command("IncPatchRow").c_str())) {
						cdeclcall(void, 0x42A6F0); // CMainFrame::OnCurveInsertAddrow
					}

					if (ImGui::MenuItem("Add Terrain Row / Column", hotkeys::get_hotkey_for_command("AddTerrainRow").c_str())) {
						cdeclcall(void, 0x42B080); // CMainFrame::OnAddTerrainRowColumn
					}

					if (ImGui::MenuItem("Extrude Terrain Row", hotkeys::get_hotkey_for_command("ExtrudeTerrainRow").c_str())) {
						cdeclcall(void, 0x42B0A0); // CMainFrame::ExtrudeTerrainRow
					}

					ImGui::EndMenu(); // Insert
				}

				if (ImGui::BeginMenu("Delete"))
				{
					if (ImGui::MenuItem("First 2 Columns", hotkeys::get_hotkey_for_command("DecPatchColumn").c_str())) {
						cdeclcall(void, 0x42A810); // CMainFrame::OnCurveDeleteFirstcolumn
					}

					if (ImGui::MenuItem("First 2 Rows", hotkeys::get_hotkey_for_command("DecPatchRow").c_str())) {
						cdeclcall(void, 0x42A860); // CMainFrame::OnCurveDeleteFirstrow
					}

					if (ImGui::MenuItem("Last 2 Columns")) {
						cdeclcall(void, 0x42A8B0); // CMainFrame::OnCurveDeleteLastcolumn
					}

					if (ImGui::MenuItem("Last 2 Rows")) {
						cdeclcall(void, 0x42A900); // CMainFrame::OnCurveDeleteLastrow
					}

					if (ImGui::MenuItem("Remove Terrain Row / Column", hotkeys::get_hotkey_for_command("RemoveTerrainRow").c_str())) {
						cdeclcall(void, 0x42B0B0); // CMainFrame::OnRemoveTerrainRowColumn
					}

					ImGui::EndMenu(); // Delete
				}

				if (ImGui::BeginMenu("Matrix"))
				{
					if (ImGui::MenuItem("Transpose", hotkeys::get_hotkey_for_command("PatchMatrixTranspose").c_str())) {
						cdeclcall(void, 0x42B1E0); // CMainFrame::OnCurveMatrixTranspose
					}

					if (ImGui::MenuItem("Re-disperse Vertices", hotkeys::get_hotkey_for_command("RedisperseVertices").c_str())) {
						cdeclcall(void, 0x42A270); // CMainFrame::OnRedistPatchPoints
					}

					if (ImGui::MenuItem("Re-disperse Columns", hotkeys::get_hotkey_for_command("RedisperseCols").c_str())) {
						cdeclcall(void, 0x42AD80); // CMainFrame::OnCurveRedisperseCols
					}

					if (ImGui::MenuItem("Re-disperse Rows", hotkeys::get_hotkey_for_command("RedisperseRows").c_str())) {
						cdeclcall(void, 0x42AD90); // CMainFrame::OnCurveRedisperseRows
					}

					ImGui::EndMenu(); // Matrix
				}

				if (ImGui::BeginMenu("Weld"))
				{
					if (ImGui::MenuItem("Connect Selection / Weld", hotkeys::get_hotkey_for_command("ConnectSelection").c_str())) {
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

				if (ImGui::MenuItem("Split", hotkeys::get_hotkey_for_command("SplitPatch").c_str())) {
					cdeclcall(void, 0x42B0C0); // CMainFrame::OnSplitPatch
				}

				// TODO: add 'InvertCurveTextureX' and 'InvertCurveTextureY'
				if (ImGui::MenuItem("Invert", hotkeys::get_hotkey_for_command("InvertCurve").c_str())) {
					cdeclcall(void, 0x42A7E0); // CMainFrame::OnCurveNegative
				}

				if (ImGui::MenuItem("Thicken", hotkeys::get_hotkey_for_command("ThickenPatch").c_str())) {
					cdeclcall(void, 0x42B0D0); // CMainFrame::OnCurveThicken
				}

				if (ImGui::MenuItem("Cap", hotkeys::get_hotkey_for_command("CapCurrentCurve").c_str())) {
					cdeclcall(void, 0x42AD40); // CMainFrame::OnCurveCap
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Turn Terrain Edges")) {
					cdeclcall(void, 0x4294E0); // CMainFrame::OnTurnTerrainEdges
				}

				if (ImGui::MenuItem("Lock Vertices", hotkeys::get_hotkey_for_command("ToggleLockPatchVertices").c_str())) {
					mainframe_thiscall(void, 0x42B4F0); // CMainFrame::ToggleLockPatchVertMode
				}

				if (ImGui::MenuItem("Unlock Vertices", hotkeys::get_hotkey_for_command("ToggleUnlockPatchVertices").c_str())) {
					mainframe_thiscall(void, 0x42B510); // CMainFrame::ToggleUnlockPatchVertMode
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Cycle Cap Texture", hotkeys::get_hotkey_for_command("CycleCapTexturePatch").c_str())) {
					cdeclcall(void, 0x42B1A0); // CMainFrame::OnCurveCyclecap
				}

				if (ImGui::MenuItem("Vert Edit Dialog", hotkeys::get_hotkey_for_command("VertEdit").c_str())) {
					cdeclcall(void, 0x42BCD0); // CMainFrame::OnVertexEditDlg
				}

				if (ImGui::MenuItem("Cap Texture", hotkeys::get_hotkey_for_command("ApplyPatchCap").c_str())) {
					cdeclcall(void, 0x42AE50); // CMainFrame::OnPatchCap
				}

				ImGui::EndMenu(); // Patch
			}


			if (ImGui::BeginMenu("Help"))
			{
				// TODO! implement help

				if (ImGui::MenuItem("Command List")) {
					cdeclcall(void, 0x426E00); // CMainFrame::OnHelpCommandlist
				}

				// TODO! implement about
				ImGui::EndMenu(); // Help
			}
			ImGui::EndGroup();

			if(dvars::gui_menubar_show_mouseorigin && dvars::gui_menubar_show_mouseorigin->current.enabled)
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
			}

			ImGui::PopStyleVar(2); // ImGuiStyleVar_WindowPadding | ImGuiStyleVar_ItemSpacing
			ImGui::EndMenuBar();
		}
	}
}