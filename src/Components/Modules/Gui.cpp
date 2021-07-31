#include "std_include.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define mainframe_thiscall(return_val, addr)	\
	utils::hook::call<return_val(__fastcall)(cmainframe*)>(addr)(cmainframe::activewnd)


#define mainframe_cdeclcall(return_val, addr)	\
	utils::hook::call<return_val(__cdecl)(cmainframe*)>(addr)(cmainframe::activewnd)


#define cdeclcall(return_val, addr)	\
	utils::hook::call<return_val(__cdecl)()>(addr)()


#define IMGUI_REGISTER_TOGGLEABLE_MENU(menu, function) \
    if(menu.menustate) {    \
        function;           \
    }

// -------------------------------------------------------------------

// show tooltip after x seconds
#define TTDELAY 0.5f 

// tooltip with delay
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) ImGui::SetTooltip(tooltip)

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing) 


// left label menu widget, eg. "dragfloat"
#define IMGUI_MENU_WIDGET_SINGLE(label, func)                                                       \
    ImGui::Text(label); ImGui::SameLine();                                                          \
    const ImGuiMenuColumns* offsets = &ImGui::GetCurrentWindow()->DC.MenuColumns;                   \
	ImGui::SetCursorPosX(static_cast<float>(offsets->OffsetShortcut + 5));                          \
	ImGui::PushItemWidth(static_cast<float>(offsets->Widths[2] + offsets->Widths[3] + 5));          \
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));   \
	func; ImGui::PopItemWidth(); ImGui::PopStyleVar();


namespace components
{

	// *
	// initialize the imgui camerawnd context
	void imgui_init_ccamerawnd()
	{
		// get the device from d3d9ex::_d3d9/Ex::CreateDevice
		IDirect3DDevice9* device = game::glob::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");
		
		//ASSERT_MSG(cmainframe::activewnd->m_pCamWnd, "cmainframe::activewnd->m_pCamWnd == nullptr");
		if (!cmainframe::activewnd->m_pCamWnd)
		{
			return;
		}
		
		ggui::state.ccamerawnd.context = ImGui::CreateContext();
		ASSERT_MSG(ggui::state.ccamerawnd.context, "ccamerawnd.context == nullptr");
		
		// set context
		ImGui::SetCurrentContext(ggui::state.ccamerawnd.context);

        ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		//io.MouseDrawCursor = true;

		// font (see Fonts.cpp)
        io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

		// implementation
		ImGui_ImplWin32_Init(cmainframe::activewnd->m_pCamWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();

		// fully initialized
		ggui::state.ccamerawnd.context_initialized = true;
		ggui::state.ccamerawnd.dx_window = &game::dx->windows[ggui::CCAMERAWND];
	}

	
	// *
	// initialize the imgui xywnd context
	void imgui_init_cxywnd()
	{
		// get the device from d3d9ex::_d3d9/Ex::CreateDevice
		IDirect3DDevice9* device = game::glob::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");

		//ASSERT_MSG(cmainframe::activewnd->m_pXYWnd, "cmainframe::activewnd->m_pXYWnd == nullptr");
		if(!cmainframe::activewnd->m_pXYWnd)
		{
			return;
		}

		ggui::state.cxywnd.context = ImGui::CreateContext();
		ASSERT_MSG(ggui::state.cxywnd.context, "cxywnd.context == nullptr");

		// set context
		ImGui::SetCurrentContext(ggui::state.cxywnd.context);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.MouseDrawCursor = true;
		
		// font (see Fonts.cpp)
		io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

		// implementation
		ImGui_ImplWin32_Init(cmainframe::activewnd->m_pXYWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();
		
		// fully initialized
		ggui::state.cxywnd.context_initialized = true;
		ggui::state.cxywnd.dx_window = &game::dx->windows[ggui::CXYWND];
	}

	
	/*
	 * auto vtable = reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable);
	 * reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable)->SetStatusText(&cmainframe::activewnd->m_wndStatusBar, 0x75);
	 *
	 * auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);
	 * vtable->RecalcLayout(&cmainframe::activewnd->m_wndSplit);
	 * 
	 */

	void cxywnd_gui(ggui::imgui_context& context)
	{
		ImGuiIO& io = ImGui::GetIO();

		int _stylevars = 0;
		int _stylecolors = 0;
		
		// styles that need to be set before the dockspace is created
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(800, 36));	_stylevars++;


		// *
		// create main dockspace

		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, (ImVec4)ImColor(1, 1, 1, 200));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("dockspace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		
		// *
		// menu bar within dockspace

		if (ImGui::BeginMenuBar()) 
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Map")) {
					mainframe_cdeclcall(void, 0x423AA0); //cmainframe::OnFileNew
				}

				if (ImGui::MenuItem("Open", "CTRL-O")) {
					mainframe_cdeclcall(void, 0x423AE0); //cmainframe::OnFileOpen
				}

				if (ImGui::MenuItem("Save", "CTRL-S")) {
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
					if (ImGui::MenuItem("New Project")){
						mainframe_cdeclcall(void, 0x426E80); //cmainframe::OnFileNewproject
					}

					if (ImGui::MenuItem("Set Startup Project")){
						mainframe_cdeclcall(void, 0x427010); //cmainframe::OnSetStartupProject
					}

					if (ImGui::MenuItem("Project Settings")) {
						mainframe_cdeclcall(void, 0x428DE0); //cmainframe::OnFileProjectsettings
					}
					
					ImGui::EndMenu(); // Project Settings
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Exit"))
				{
					PostMessage(cmainframe::activewnd->GetWindow(), WM_CLOSE, 0, 0L);
				}

				ImGui::EndMenu(); // File
			}

			// ----------------------------
			
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL-Z", nullptr, game::g_lastundo() && game::g_lastundo()->done)) {
					cdeclcall(void, 0x428730); //cmainframe::OnEditUndo / OnUpdateEditUndo
				}
				
				if (ImGui::MenuItem("Redo", "CTRL-Y", nullptr, game::g_lastredo())) {
					cdeclcall(void, 0x428740); //cmainframe::OnEditRedo / OnUpdateEditRedo
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Copy", "CTRL-C")) {
					mainframe_thiscall(void, 0x4286B0); //cmainframe::OnEditCopybrush
				}

				if (ImGui::MenuItem("Paste", "CTRL-V")) {
					mainframe_thiscall(void, 0x4286D0); //cmainframe::OnEditPastebrush
				}
				
				if (ImGui::MenuItem("Delete", "Backspace")) {
					cdeclcall(void, 0x425690); //cmainframe::OnSelectionDelete
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Map Info", "M")) {
					cdeclcall(void, 0x426C60); //cmainframe::OnEditMapinfo
				}

				if (ImGui::MenuItem("Entity Info")) {
					cdeclcall(void, 0x426D40); //cmainframe::OnEditEntityinfo
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Enter Prefab")) {
					cdeclcall(void, 0x42BF70); //cmainframe::OnPrefabEnter
				}

				if (ImGui::MenuItem("Leave Prefab")) {
					cdeclcall(void, 0x42BF80); //cmainframe::OnPrefabLeave
				}

				SEPERATORV(0.0f);
				
				if (ImGui::MenuItem("Preferences", "P")) {
					mainframe_thiscall(void, 0x426950); //cmainframe::OnPrefs
				}

				ImGui::EndMenu(); // Edit
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::BeginMenu("Toggle"))
				{
					if (ImGui::MenuItem("Camera View", "SHIFT-CTRL-C", nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426A40); // cmainframe::OnTogglecamera
					}

					if (ImGui::MenuItem("Console View", 0, nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426A90); // cmainframe::OnToggleconsole
					}

					if (ImGui::MenuItem("Z View", "SHIFT-CTRL-Z", nullptr, cmainframe::is_combined_view())) {
						mainframe_thiscall(void, 0x426B30); // cmainframe::OnTogglez
					}

					if (ImGui::MenuItem("Entity View", "N")) {
						mainframe_thiscall(void, 0x423F00); // cmainframe::OnViewEntity
					}

					if (ImGui::MenuItem("Layers", "L")) {
						cdeclcall(void, 0x42BD10); // cmainframe::OnLayersDlg
					}

					if (ImGui::MenuItem("Filter Settings", "F")) {
						mainframe_thiscall(void, 0x42B7A0); // cmainframe::OnFilterDlg
					}

					ImGui::EndMenu(); // Toggle
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Center", "END")) {
					mainframe_thiscall(void, 0x423C50); // cmainframe::OnViewCenter
				}

				if (ImGui::MenuItem("Up Floor", "PAGE-UP")) {
					mainframe_thiscall(void, 0x424700); // cmainframe::OnViewUpfloor
				} TT("move up one floor, if there is one");

				if (ImGui::MenuItem("Down Floor", "PAGE-DOWN")) {
					mainframe_thiscall(void, 0x423ED0); // cmainframe::OnViewDownfloor
				} TT("move down one floor, if there is one");

				SEPERATORV(0.0f);
				
				if (ImGui::MenuItem("Next (XY, YZ, XZ)", "CTRL-TAB")) {
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

					if (ImGui::MenuItem("Show Names", 0, (game::g_qeglobals->d_xyShowFlags & 8) == 0)) {
						mainframe_thiscall(void, 0x42BA40); // cmainframe::OnSelectNames
					}

					if (ImGui::MenuItem("Show Angles", 0, (game::g_qeglobals->d_xyShowFlags & 2) == 0)) {
						mainframe_thiscall(void, 0x42BAA0); // cmainframe::OnSelectAngles
					}

					if (ImGui::MenuItem("Show Grid Blocks", 0, (game::g_qeglobals->d_xyShowFlags & 16) == 0)) {
						mainframe_thiscall(void, 0x42BB00); // cmainframe::OnSelectBlocks
					}

					if (ImGui::MenuItem("Show Connections", 0, (game::g_qeglobals->d_xyShowFlags & 4) == 0)) {
						mainframe_thiscall(void, 0x42BBC0); // cmainframe::OnSelectConnections
					}
					
					if (ImGui::MenuItem("Show Coordinates", 0, (game::g_qeglobals->d_xyShowFlags & 32) == 0)) {
						mainframe_thiscall(void, 0x42BB60); // cmainframe::OnSelectCoordinates
					}
					
					ImGui::EndMenu(); // XY Window
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Hide Selected", "H")) {
					cdeclcall(void, 0x42B6A0); // cmainframe::OnHideSelected
				}

				if (ImGui::MenuItem("Hide Unselected", "ALT-H")) {
					cdeclcall(void, 0x42B6C0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Last Hidden", "SHIFT-CTRL-H")) {
					cdeclcall(void, 0x42B6E0); // cmainframe::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Hidden", "SHIFT-H")) {
					cdeclcall(void, 0x42B6D0); // cmainframe::OnShowHidden
				}
				
				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Draw Entities As"))
				{
					if (ImGui::MenuItem("Bounding Box", 0, game::g_PrefsDlg()->m_nEntityShowState == game::ENTITY_BOXED)) {
						mainframe_thiscall(void, 0x42B320); // cmainframe::OnViewEntitiesasBoundingbox
					}

					if (ImGui::MenuItem("Wireframe", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B410); // cmainframe::OnViewEntitiesasWireframe
					}

					if (ImGui::MenuItem("Selected Wireframe", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SELECTED_ONLY | game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B380); // cmainframe::OnViewEntitiesasSelectedwireframe
					}

					if (ImGui::MenuItem("Selected Skinned", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SELECTED_ONLY | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B350); // cmainframe::OnViewEntitiesasSelectedskinned
					}

					if (ImGui::MenuItem("Skinned", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3B0); // cmainframe::OnViewEntitiesasSkinned
					}

					if (ImGui::MenuItem("Skinned And Boxed", 0, game::g_PrefsDlg()->m_nEntityShowState == (game::ENTITY_SKINNED | game::ENTITY_BOXED | game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3E0); // cmainframe::OnViewEntitiesasSkinnedandboxed
					}
					
					ImGui::EndMenu(); // Draw Entities As
				}

				if (ImGui::BeginMenu("Light Preview"))
				{
					if (ImGui::MenuItem("Enable Light Preview", "F8", game::g_PrefsDlg()->enable_light_preview)) {
						mainframe_thiscall(void, 0x4240C0); // cmainframe::OnEnableLightPreview
					}

					if (ImGui::MenuItem("Enable Sun Preview", "CTRL-F8", game::g_PrefsDlg()->preview_sun_aswell)) {
						mainframe_thiscall(void, 0x424060); // cmainframe::OnPreviewSun
					}

					SEPERATORV(0.0f);
					
					if (ImGui::MenuItem("Start Previewing Selected", "SHIFT-F8")) {
						mainframe_thiscall(void, 0x424120); // cmainframe::OnStartPreviewSelected
					}

					if (ImGui::MenuItem("Stop Previewing Selected", "SHIFT-ALT-F8")) {
						mainframe_thiscall(void, 0x424170); // cmainframe::OnStopPreviewSelected
					}

					if (ImGui::MenuItem("Clear Preview List", "ALT-F8")) {
						mainframe_thiscall(void, 0x4241C0); // cmainframe::OnClearPreviewList
					}

					SEPERATORV(0.0f);
					
					if (ImGui::MenuItem("Preview At Max Intensity", "ALT-CTRL-F8", !game::g_qeglobals->preview_at_max_intensity)) {
						cdeclcall(void, 0x425670); // cmainframe::OnPreviewAtMaxIntensity
					}

					if (ImGui::MenuItem("Show Regions For Selected", "SHIFT-CTRL-F8")) {
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

				IMGUI_MENU_WIDGET_SINGLE("Cubic Scale", ImGui::DragInt("", &game::g_PrefsDlg()->m_nCubicScale, 1, 1, 220));

				if (ImGui::MenuItem("Cubic Clipping", "CTRL-\\", game::g_PrefsDlg()->m_bCubicClipping)) {
					mainframe_thiscall(void, 0x428F90); // cmainframe::OnViewCubicclipping
				}

				ImGui::EndMenu(); // View
			}

			ImGui::EndMenuBar();
		}

		
		// *
		// create default docking layout
		
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("cxywnd_dockspace_layout");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);

			static auto first_time = true;
			if (first_time)
			{
				first_time = false;

				ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
				ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

				// create toolbar dockspace
				auto dock_toolbar = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.0f, nullptr, &dockspace_id);
				ImGui::DockBuilderSetNodeSize(dock_toolbar, ImVec2(viewport->Size.x, 36));

				// split the resulting node (dockspace_id) -> dockspace_id will be everything to the right of the split
				//auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);

				// we now dock our windows into the docking node we made above
				ImGui::DockBuilderDockWindow("toolbar_xywnd", dock_toolbar);
				//ImGui::DockBuilderDockWindow("test_gui1", dock_id_left);
				//ImGui::DockBuilderDockWindow("test_gui2", dockspace_id);
				
				ImGui::DockBuilderFinish(dockspace_id);
			}
		}
		
		ImGui::PopStyleColor(1);
		ImGui::End();
		
		// ----------------------------

		/*ImGui::Begin("test_gui1", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Hello, left!");
		ImGui::End();

		ImGui::Begin("test_gui2", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Hello, right!");
		ImGui::End();*/

		// ----------------------------


		// *
		// create toolbar window
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 4.0f));		_stylevars++;

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));			_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(1, 1, 1, 120));		_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(100, 100, 100, 70));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));		_stylecolors++;

		ImGui::Begin("toolbar_xywnd", nullptr, 
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar
		);

		
		// *
		// sizes (not really needed because the toolbar is pre-docked)

		auto toolbar_menu = context.menus[0];
		
		toolbar_menu.position[0] = viewport->Pos.x;
		toolbar_menu.position[1] = viewport->Pos.y;
		toolbar_menu.size[0] = viewport->Size.x; //static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width); //ImGui::GetWindowWidth();
		toolbar_menu.size[1] = 40.0f;

		ImGui::SetWindowPos(ImVec2(toolbar_menu.position[0], toolbar_menu.position[1]), ImGuiCond_FirstUseEver);
		ImGui::SetWindowSize(ImVec2(toolbar_menu.size[0], toolbar_menu.size[1]), ImGuiCond_FirstUseEver);

		
		// *
		// gui elements

		const ImVec2 toolbar_imagebutton_size = ImVec2(28.0f, 28.0f);

		
		// lock x
		if (const auto image = game::Image_RegisterHandle("lock_x"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// cmainframe::OnScalelockX
				mainframe_thiscall(LRESULT, 0x428BC0);
			} TT("lock grid along the x-axis");
		}

		
		// lock y
		if (const auto image = game::Image_RegisterHandle("lock_y"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// cmainframe::OnScalelockY
				mainframe_thiscall(LRESULT, 0x428B60);
			} TT("lock grid along the y-axis");
		}

		
		// lock z
		if (const auto image = game::Image_RegisterHandle("lock_z"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// cmainframe::OnScalelockZ
				mainframe_thiscall(LRESULT, 0x428B90);
			} TT("lock grid along the z-axis");
		}

		
		// TODO! - remove me
		ImGui::SameLine();
		if (ImGui::Button("Lock X")) { mainframe_thiscall(LRESULT, 0x428BC0); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Y")) { mainframe_thiscall(LRESULT, 0x428B60); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Z")) { mainframe_thiscall(LRESULT, 0x428B90); }


		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if(ImGui::Button("Switch Console <-> Splitter"))
		{
			const auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);
			
			const auto pTop = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 0, 0);
			const auto pBottom = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 1, 0);

			if(!pTop || !pBottom)
			{
				goto END_GUI;
			}

			const auto _top = pTop->m_hWnd;
			const auto idTop = GetWindowLongA(_top, GWL_ID);

			const auto _bottom = pBottom->m_hWnd;
			const auto idBottom = GetWindowLongA(_bottom, GWL_ID);

			SetWindowLongA(_top, GWL_ID, idBottom);
			SetWindowLongA(_bottom, GWL_ID, idTop);

			vtable->RecalcLayout(&cmainframe::activewnd->m_wndSplit);
		}

		// TODO! - remove me
		ImGui::SameLine();
		if (ImGui::Button("Set Statustext"))
		{
			const auto vtable = reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable);
			vtable->SetStatusText(&cmainframe::activewnd->m_wndStatusBar, 0x75);
		}

		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if(ImGui::Button("Toggle Toolbar"))
		{
			typedef void(__thiscall* CFrameWnd_ShowControlBar_t)(CFrameWnd*, CControlBar*, BOOL bShow, BOOL bDelay);
			CFrameWnd_ShowControlBar_t CFrameWnd_ShowControlBar = reinterpret_cast<CFrameWnd_ShowControlBar_t>(0x59E9DD);

			auto vtable = reinterpret_cast<CToolBar_vtbl*>(cmainframe::activewnd->m_wndToolBar.__vftable);
			CFrameWnd_ShowControlBar(cmainframe::activewnd, &cmainframe::activewnd->m_wndToolBar, vtable->IsVisible(&cmainframe::activewnd->m_wndToolBar) ? 0 : 1, 1);
		}

		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if (ImGui::Button("Toggle Menubar"))
		{
			if(!ggui::mainframe_menubar_visible)
			{
				const auto menubar = LoadMenu(cmainframe::activewnd->m_pModuleState->m_hCurrentInstanceHandle, MAKEINTRESOURCE(0xD6)); // 0xD6 = IDR_MENU_QUAKE3
				SetMenu(cmainframe::activewnd->GetWindow(), menubar);
				ggui::mainframe_menubar_visible = true;
			}
			else
			{
				SetMenu(cmainframe::activewnd->GetWindow(), 0);
				ggui::mainframe_menubar_visible = false;
			}
		}

	END_GUI:
		ImGui::PopStyleColor(_stylecolors);
		ImGui::PopStyleVar(_stylevars);
		ImGui::End();
	}

	
	// *
	// main rendering loop (d3d9ex::d3d9device::EndScene())
	void gui::render_loop()
	{
		// *
		// menus

		//gui::any_open_menus();

		/* - radiant draws multiple windows using d3d => multiple endscene / present calls
		 * - each window should have its own imgui context
		 * - use dx->targetWindowIndex to distinguish between windows
		 */
		
		if (game::dx->targetWindowIndex == ggui::CCAMERAWND)
		{
			if(!ggui::state.ccamerawnd.context_initialized)
			{
				imgui_init_ccamerawnd();

				// allow 1 frame to pass
				return;
			}

			// set context
			IMGUI_BEGIN_CCAMERAWND;

			// begin context frame
			gui::begin_frame();

			// TODO! always show demo window (for now)
			ImGui::ShowDemoWindow(nullptr);
			
			// end the current context frame
			goto END_FRAME;
		}

		// ------------------------------------------------------
		
		if (game::dx->targetWindowIndex == ggui::CXYWND) //if (game::glob::gui_present.cxywnd)
		{
			if (!ggui::state.cxywnd.context_initialized)
			{
				imgui_init_cxywnd();

				// allow 1 frame to pass
				return;
			}

			IMGUI_BEGIN_CXYWND;

			// global style vars for current context
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameBorderSize = 0.0f;

			// begin context frame
			gui::begin_frame();

			// create all dockable windows in cxywnd_gui
			cxywnd_gui(ggui::state.cxywnd);

			// toggleable demo menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.menus[ggui::e_menu::XY_DEMO],
					ImGui::ShowDemoWindow(&ggui::state.cxywnd.menus[ggui::e_menu::XY_DEMO].menustate));

			// end the current context frame
			goto END_FRAME;
		}

		return;

		// ------------

	END_FRAME:
		gui::end_frame();

		ImGuiIO& io = ImGui::GetIO();

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	// *
	// shutdown imgui (d3d9ex::d3d9device::Release)
	void gui::shutdown()
	{
		if(ggui::state.ccamerawnd.context_initialized)
		{
			IMGUI_BEGIN_CCAMERAWND;
			// Shutdown calls d3dDevice->Release D:
			// ImGui_ImplDX9_Shutdown();
			
			ImGui::DestroyPlatformWindows();
			ImGui_ImplDX9_InvalidateDeviceObjects();

			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();

			ggui::state.ccamerawnd.context_initialized = false;
		}

		if (ggui::state.cxywnd.context_initialized)
		{
			IMGUI_BEGIN_CXYWND;
			// Shutdown calls d3dDevice->Release D:
			// ImGui_ImplDX9_Shutdown();

			ImGui::DestroyPlatformWindows();
			ImGui_ImplDX9_InvalidateDeviceObjects();
			
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();

			ggui::state.cxywnd.context_initialized = false;
		}

		memset(&ggui::state, 0, sizeof(ggui::imgui_state_t));
	}

	
	bool gui::all_contexts_ready()
	{
		if(ggui::state.ccamerawnd.context_initialized 
			&& ggui::state.cxywnd.context_initialized)
		{
			return true;
		}

		return false;
	}

	// *
	// toggle a imgui menu by command (or key (scheduler))
	void gui::toggle(ggui::imgui_context_menu& menu, [[maybe_unused]] int keycatcher, bool onCommand = false)
	{
		if(!gui::all_contexts_ready())
		{
			return;
		}

		// TODO! fix me
		// toggle menu by key or command
		if (onCommand)
		{
			menu.menustate = !menu.menustate;

			// on close
			if (!menu.menustate)
			{
			}
		}
	}


	// *
	void gui::begin_frame()
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	
	// *
	void gui::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	
	// *
	void gui::register_dvars()
	{
		//dvars::imgui_devgui_pos_x = dvars::register_float(
		//	/* name		*/ "imgui_devgui_pos_x",
		//	/* value	*/ 20.0f,
		//	/* minVal	*/ 0.0f,
		//	/* maxVal	*/ 4096.0f,
		//	/* flags	*/ game::dvar_flags::saved,
		//	/* desc		*/ "X position of the devgui");
	}

	
	// *
	gui::gui()
	{
		command::register_command("demo"s, [](std::vector<std::string> args)
		{
			gui::toggle(ggui::state.cxywnd.menus[ggui::e_menu::XY_DEMO], 0, true);
		});
	}

	gui::~gui()
	{ }
}