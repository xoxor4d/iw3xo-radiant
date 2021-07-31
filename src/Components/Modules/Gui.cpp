#include "STDInclude.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define mainframe_thiscall(return_val, addr)	\
	Utils::Hook::Call<return_val(__fastcall)(CMainFrame*)>(addr)(CMainFrame::ActiveWindow)

#define mainframe_cdeclcall(return_val, addr)	\
	Utils::Hook::Call<return_val(__cdecl)(CMainFrame*)>(addr)(CMainFrame::ActiveWindow)

#define cdeclcall(return_val, addr)	\
	Utils::Hook::Call<return_val(__cdecl)()>(addr)()

//#define IMGUI_REGISTERMENU(menu, function)			\
//    if (menu.menustate) {							\
//		function;									\
//		menu.was_open = true;						\
//	}												\
//	else if(menu.was_open) {						\
//		menu.was_open = false;						\
//	}

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

#define IMGUI_MENU_WIDGET_SINGLE(label, func)                                                       \
    ImGui::Text(label); ImGui::SameLine();                                                          \
    const ImGuiMenuColumns* offsets = &ImGui::GetCurrentWindow()->DC.MenuColumns;                   \
	ImGui::SetCursorPosX(offsets->OffsetShortcut + 5);                                              \
	ImGui::PushItemWidth(offsets->Widths[2] + offsets->Widths[3] + 5);                              \
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));   \
	func; ImGui::PopItemWidth(); ImGui::PopStyleVar();

namespace Components
{

	// *
	// initialize the imgui camerawnd context
	void imgui_init_ccamerawnd()
	{
		// get the device from D3D9Ex::_D3D9/Ex::CreateDevice
		IDirect3DDevice9* device = Game::Globals::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");
		
		//ASSERT_MSG(CMainFrame::ActiveWindow->m_pCamWnd, "CMainFrame::ActiveWindow->m_pCamWnd == nullptr");
		if (!CMainFrame::ActiveWindow->m_pCamWnd)
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
		ImGui_ImplWin32_Init(CMainFrame::ActiveWindow->m_pCamWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();

		// fully initialized
		ggui::state.ccamerawnd.context_initialized = true;
		ggui::state.ccamerawnd.dx_window = &Game::dx->windows[ggui::CCAMERAWND];
	}

	
	// *
	// initialize the imgui xywnd context
	void imgui_init_cxywnd()
	{
		// get the device from D3D9Ex::_D3D9/Ex::CreateDevice
		IDirect3DDevice9* device = Game::Globals::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");

		//ASSERT_MSG(CMainFrame::ActiveWindow->m_pXYWnd, "CMainFrame::ActiveWindow->m_pXYWnd == nullptr");
		if(!CMainFrame::ActiveWindow->m_pXYWnd)
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
		ImGui_ImplWin32_Init(CMainFrame::ActiveWindow->m_pXYWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();
		
		// fully initialized
		ggui::state.cxywnd.context_initialized = true;
		ggui::state.cxywnd.dx_window = &Game::dx->windows[ggui::CXYWND];
	}

	
	/*
	 * auto vtable = reinterpret_cast<CStatusBar_vtbl*>(CMainFrame::ActiveWindow->m_wndStatusBar.__vftable);
	 * reinterpret_cast<CStatusBar_vtbl*>(CMainFrame::ActiveWindow->m_wndStatusBar.__vftable)->SetStatusText(&CMainFrame::ActiveWindow->m_wndStatusBar, 0x75);
	 *
	 * auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(CMainFrame::ActiveWindow->m_wndSplit.__vftable);
	 * vtable->RecalcLayout(&CMainFrame::ActiveWindow->m_wndSplit);
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
					mainframe_cdeclcall(void, 0x423AA0); //CMainFrame::OnFileNew
				}

				if (ImGui::MenuItem("Open", "CTRL-O")) {
					mainframe_cdeclcall(void, 0x423AE0); //CMainFrame::OnFileOpen
				}

				if (ImGui::MenuItem("Save", "CTRL-S")) {
					mainframe_cdeclcall(void, 0x423B80); //CMainFrame::OnFileSave
				}

				if (ImGui::MenuItem("Save As")) {
					mainframe_cdeclcall(void, 0x423BC0); //CMainFrame::OnFileSaveas
				}

				if (ImGui::MenuItem("Save Selected")) {
					mainframe_thiscall(void, 0x4293A0); //CMainFrame::OnFileExportmap
				}

				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Open Recent", Game::g_qeglobals->d_lpMruMenu->wNbItemFill))
				{
					// itemfill = amount of strings inside "lpMRU"
					for (std::uint16_t i = 0; i < Game::g_qeglobals->d_lpMruMenu->wNbItemFill; i++)
					{
						// "lpMRU" strings are 128chars wide
						const char* mru_item_str = Game::g_qeglobals->d_lpMruMenu->lpMRU + (Game::g_qeglobals->d_lpMruMenu->wMaxSizeLruItem * i);

						if (mru_item_str && ImGui::MenuItem(mru_item_str))
						{
							typedef  BOOL(__thiscall* CMainFrame_OnMru_t)(CMainFrame* pThis, std::uint16_t nID);
							CMainFrame_OnMru_t CMainFrame_OnMru = reinterpret_cast<CMainFrame_OnMru_t>(0x423FE0);

							//mru id's start at 8000 (wIdMru) + 1 for the first item as DoMru() subtracts 1
							CMainFrame_OnMru(CMainFrame::ActiveWindow, i + 1 + Game::g_qeglobals->d_lpMruMenu->wIdMru);
						}
					}

					ImGui::EndMenu(); // Open Recent
				}
				
				if (ImGui::BeginMenu("Misc")) 
				{
					if (ImGui::MenuItem("Save Region", "", nullptr, Game::g_region_active)) {
						mainframe_cdeclcall(void, 0x429020); //CMainFrame::OnFileSaveregion
					}

					if (ImGui::MenuItem("Pointfile")) {
						mainframe_cdeclcall(void, 0x423B20); //CMainFrame::OnPointfileOpen
					}

					if (ImGui::MenuItem("Errorfile")) {
						mainframe_cdeclcall(void, 0x423B40); //CMainFrame::OnErrorFile
					}
					
					ImGui::EndMenu(); // Misc
				}
				
				if (ImGui::BeginMenu("Project Settings"))
				{
					if (ImGui::MenuItem("New Project")){
						mainframe_cdeclcall(void, 0x426E80); //CMainFrame::OnFileNewproject
					}

					if (ImGui::MenuItem("Set Startup Project")){
						mainframe_cdeclcall(void, 0x427010); //CMainFrame::OnSetStartupProject
					}

					if (ImGui::MenuItem("Project Settings")) {
						mainframe_cdeclcall(void, 0x428DE0); //CMainFrame::OnFileProjectsettings
					}
					
					ImGui::EndMenu(); // Project Settings
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Exit"))
				{
					PostMessage(CMainFrame::ActiveWindow->GetWindow(), WM_CLOSE, 0, 0L);
				}

				ImGui::EndMenu(); // File
			}

			// ----------------------------
			
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL-Z", nullptr, Game::g_lastundo() && Game::g_lastundo()->done)) {
					cdeclcall(void, 0x428730); //CMainFrame::OnEditUndo / OnUpdateEditUndo
				}
				
				if (ImGui::MenuItem("Redo", "CTRL-Y", nullptr, Game::g_lastredo())) {
					cdeclcall(void, 0x428740); //CMainFrame::OnEditRedo / OnUpdateEditRedo
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Copy", "CTRL-C")) {
					mainframe_thiscall(void, 0x4286B0); //CMainFrame::OnEditCopybrush
				}

				if (ImGui::MenuItem("Paste", "CTRL-V")) {
					mainframe_thiscall(void, 0x4286D0); //CMainFrame::OnEditPastebrush
				}
				
				if (ImGui::MenuItem("Delete", "Backspace")) {
					cdeclcall(void, 0x425690); //CMainFrame::OnSelectionDelete
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Map Info", "M")) {
					cdeclcall(void, 0x426C60); //CMainFrame::OnEditMapinfo
				}

				if (ImGui::MenuItem("Entity Info")) {
					cdeclcall(void, 0x426D40); //CMainFrame::OnEditEntityinfo
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Enter Prefab")) {
					cdeclcall(void, 0x42BF70); //CMainFrame::OnPrefabEnter
				}

				if (ImGui::MenuItem("Leave Prefab")) {
					cdeclcall(void, 0x42BF80); //CMainFrame::OnPrefabLeave
				}

				SEPERATORV(0.0f);
				
				if (ImGui::MenuItem("Preferences", "P")) {
					mainframe_thiscall(void, 0x426950); //CMainFrame::OnPrefs
				}

				ImGui::EndMenu(); // Edit
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::BeginMenu("Toggle"))
				{
					if (ImGui::MenuItem("Camera View", "SHIFT-CTRL-C", nullptr, Game::mainframe_is_combined_view())) {
						mainframe_thiscall(void, 0x426A40); // CMainFrame::OnTogglecamera
					}

					if (ImGui::MenuItem("Console View", 0, nullptr, Game::mainframe_is_combined_view())) {
						mainframe_thiscall(void, 0x426A90); // CMainFrame::OnToggleconsole
					}

					if (ImGui::MenuItem("Z View", "SHIFT-CTRL-Z", nullptr, Game::mainframe_is_combined_view())) {
						mainframe_thiscall(void, 0x426B30); // CMainFrame::OnTogglez
					}

					if (ImGui::MenuItem("Entity View", "N")) {
						mainframe_thiscall(void, 0x423F00); // CMainFrame::OnViewEntity
					}

					if (ImGui::MenuItem("Layers", "L")) {
						cdeclcall(void, 0x42BD10); // CMainFrame::OnLayersDlg
					}

					if (ImGui::MenuItem("Filter Settings", "F")) {
						mainframe_thiscall(void, 0x42B7A0); // CMainFrame::OnFilterDlg
					}

					ImGui::EndMenu(); // Toggle
				}
				
				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Center", "END")) {
					mainframe_thiscall(void, 0x423C50); // CMainFrame::OnViewCenter
				}

				if (ImGui::MenuItem("Up Floor", "PAGE-UP")) {
					mainframe_thiscall(void, 0x424700); // CMainFrame::OnViewUpfloor
				} TT("move up one floor, if there is one");

				if (ImGui::MenuItem("Down Floor", "PAGE-DOWN")) {
					mainframe_thiscall(void, 0x423ED0); // CMainFrame::OnViewDownfloor
				} TT("move down one floor, if there is one");

				SEPERATORV(0.0f);
				
				if (ImGui::MenuItem("Next (XY, YZ, XZ)", "CTRL-TAB")) {
					mainframe_thiscall(void, 0x426DB0); // CMainFrame::OnViewNextview
				}

				if (ImGui::BeginMenu("XY Window"))
				{
					if (ImGui::MenuItem("XY")) {
						mainframe_thiscall(void, 0x424710); // CMainFrame::OnViewXy
					}

					if (ImGui::MenuItem("YZ")) {
						mainframe_thiscall(void, 0x423FB0); // CMainFrame::OnViewYz
					}

					if (ImGui::MenuItem("XZ")) {
						mainframe_thiscall(void, 0x424A80); // CMainFrame::OnViewXz
					}

					SEPERATORV(0.0f);

					if (ImGui::MenuItem("Show Names", 0, (Game::g_qeglobals->d_xyShowFlags & 8) == 0)) {
						mainframe_thiscall(void, 0x42BA40); // CMainFrame::OnSelectNames
					}

					if (ImGui::MenuItem("Show Angles", 0, (Game::g_qeglobals->d_xyShowFlags & 2) == 0)) {
						mainframe_thiscall(void, 0x42BAA0); // CMainFrame::OnSelectAngles
					}

					if (ImGui::MenuItem("Show Grid Blocks", 0, (Game::g_qeglobals->d_xyShowFlags & 16) == 0)) {
						mainframe_thiscall(void, 0x42BB00); // CMainFrame::OnSelectBlocks
					}

					if (ImGui::MenuItem("Show Connections", 0, (Game::g_qeglobals->d_xyShowFlags & 4) == 0)) {
						mainframe_thiscall(void, 0x42BBC0); // CMainFrame::OnSelectConnections
					}
					
					if (ImGui::MenuItem("Show Coordinates", 0, (Game::g_qeglobals->d_xyShowFlags & 32) == 0)) {
						mainframe_thiscall(void, 0x42BB60); // CMainFrame::OnSelectCoordinates
					}
					
					ImGui::EndMenu(); // XY Window
				}

				SEPERATORV(0.0f);

				if (ImGui::MenuItem("Hide Selected", "H")) {
					cdeclcall(void, 0x42B6A0); // CMainFrame::OnHideSelected
				}

				if (ImGui::MenuItem("Hide Unselected", "ALT-H")) {
					cdeclcall(void, 0x42B6C0); // CMainFrame::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Last Hidden", "SHIFT-CTRL-H")) {
					cdeclcall(void, 0x42B6E0); // CMainFrame::OnHideUnselected
				}

				if (ImGui::MenuItem("Show Hidden", "SHIFT-H")) {
					cdeclcall(void, 0x42B6D0); // CMainFrame::OnShowHidden
				}
				
				SEPERATORV(0.0f);

				if (ImGui::BeginMenu("Draw Entities As"))
				{
					if (ImGui::MenuItem("Bounding Box", 0, Game::g_PrefsDlg()->m_nEntityShowState == Game::ENTITY_BOXED)) {
						mainframe_thiscall(void, 0x42B320); // CMainFrame::OnViewEntitiesasBoundingbox
					}

					if (ImGui::MenuItem("Wireframe", 0, Game::g_PrefsDlg()->m_nEntityShowState == (Game::ENTITY_SKINNED | Game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B410); // CMainFrame::OnViewEntitiesasWireframe
					}

					if (ImGui::MenuItem("Selected Wireframe", 0, Game::g_PrefsDlg()->m_nEntityShowState == (Game::ENTITY_SELECTED_ONLY | Game::ENTITY_WIREFRAME))) {
						mainframe_thiscall(void, 0x42B380); // CMainFrame::OnViewEntitiesasSelectedwireframe
					}

					if (ImGui::MenuItem("Selected Skinned", 0, Game::g_PrefsDlg()->m_nEntityShowState == (Game::ENTITY_SELECTED_ONLY | Game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B350); // CMainFrame::OnViewEntitiesasSelectedskinned
					}

					if (ImGui::MenuItem("Skinned", 0, Game::g_PrefsDlg()->m_nEntityShowState == (Game::ENTITY_SKINNED | Game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3B0); // CMainFrame::OnViewEntitiesasSkinned
					}

					if (ImGui::MenuItem("Skinned And Boxed", 0, Game::g_PrefsDlg()->m_nEntityShowState == (Game::ENTITY_SKINNED | Game::ENTITY_BOXED | Game::ENTITY_SKIN_MODEL))) {
						mainframe_thiscall(void, 0x42B3E0); // CMainFrame::OnViewEntitiesasSkinnedandboxed
					}
					
					ImGui::EndMenu(); // Draw Entities As
				}

				if (ImGui::BeginMenu("Light Preview"))
				{
					if (ImGui::MenuItem("Enable Light Preview", "F8", Game::g_PrefsDlg()->enable_light_preview)) {
						mainframe_thiscall(void, 0x4240C0); // CMainFrame::OnEnableLightPreview
					}

					if (ImGui::MenuItem("Enable Sun Preview", "CTRL-F8", Game::g_PrefsDlg()->preview_sun_aswell)) {
						mainframe_thiscall(void, 0x424060); // CMainFrame::OnPreviewSun
					}

					SEPERATORV(0.0f);
					
					if (ImGui::MenuItem("Start Previewing Selected", "SHIFT-F8")) {
						mainframe_thiscall(void, 0x424120); // CMainFrame::OnStartPreviewSelected
					}

					if (ImGui::MenuItem("Stop Previewing Selected", "SHIFT-ALT-F8")) {
						mainframe_thiscall(void, 0x424170); // CMainFrame::OnStopPreviewSelected
					}

					if (ImGui::MenuItem("Clear Preview List", "ALT-F8")) {
						mainframe_thiscall(void, 0x4241C0); // CMainFrame::OnClearPreviewList
					}

					SEPERATORV(0.0f);
					
					if (ImGui::MenuItem("Preview At Max Intensity", "ALT-CTRL-F8", !Game::g_qeglobals->preview_at_max_intensity)) {
						cdeclcall(void, 0x425670); // CMainFrame::OnPreviewAtMaxIntensity
					}

					if (ImGui::MenuItem("Show Regions For Selected", "SHIFT-CTRL-F8")) {
						mainframe_thiscall(void, 0x4241E0); // CMainFrame::OnShowRegionsForSelected
					}

					ImGui::EndMenu(); // Light Preview
				}

				SEPERATORV(0.0f);

				//if (ImGui::MenuItem("Cubic Clip Zoom In", "CTRL-[")) {
				//	mainframe_thiscall(void, 0x428F10); // CMainFrame::OnViewCubein
				//}

				//if (ImGui::MenuItem("Cubic Clip Zoom Out", "CTRL-]")) {
				//	mainframe_thiscall(void, 0x428F50); // CMainFrame::OnViewCubeout
				//}

				IMGUI_MENU_WIDGET_SINGLE("Cubic Scale", ImGui::DragInt("", &Game::g_PrefsDlg()->m_nCubicScale, 1, 1, 220));

				if (ImGui::MenuItem("Cubic Clipping", "CTRL-\\", Game::g_PrefsDlg()->m_bCubicClipping)) {
					mainframe_thiscall(void, 0x428F90); // CMainFrame::OnViewCubicclipping
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
		toolbar_menu.size[0] = viewport->Size.x; //static_cast<float>(CMainFrame::ActiveWindow->m_pCamWnd->camera.width); //ImGui::GetWindowWidth();
		toolbar_menu.size[1] = 40.0f;

		ImGui::SetWindowPos(ImVec2(toolbar_menu.position[0], toolbar_menu.position[1]), ImGuiCond_FirstUseEver);
		ImGui::SetWindowSize(ImVec2(toolbar_menu.size[0], toolbar_menu.size[1]), ImGuiCond_FirstUseEver);

		
		// *
		// gui elements

		const ImVec2 toolbar_imagebutton_size = ImVec2(28.0f, 28.0f);

		
		// lock x
		if (const auto image = Game::Image_RegisterHandle("lock_x"); image)
		{
			ImVec2 uv0 = (Game::g_nScaleHow == 2 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 6) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (Game::g_nScaleHow == 2 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 6) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// CMainFrame::OnScalelockX
				mainframe_thiscall(LRESULT, 0x428BC0);
			} TT("lock grid along the x-axis");
		}

		
		// lock y
		if (const auto image = Game::Image_RegisterHandle("lock_y"); image)
		{
			ImVec2 uv0 = (Game::g_nScaleHow == 1 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 5) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (Game::g_nScaleHow == 1 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 5) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// CMainFrame::OnScalelockY
				mainframe_thiscall(LRESULT, 0x428B60);
			} TT("lock grid along the y-axis");
		}

		
		// lock z
		if (const auto image = Game::Image_RegisterHandle("lock_z"); image)
		{
			ImVec2 uv0 = (Game::g_nScaleHow > 0 && Game::g_nScaleHow <= 3) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (Game::g_nScaleHow > 0 && Game::g_nScaleHow <= 3) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				// CMainFrame::OnScalelockZ
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
			const auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(CMainFrame::ActiveWindow->m_wndSplit.__vftable);
			
			const auto pTop = afx::CSplitterWnd__GetPane(&CMainFrame::ActiveWindow->m_wndSplit, 0, 0);
			const auto pBottom = afx::CSplitterWnd__GetPane(&CMainFrame::ActiveWindow->m_wndSplit, 1, 0);

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

			vtable->RecalcLayout(&CMainFrame::ActiveWindow->m_wndSplit);
		}

		// TODO! - remove me
		ImGui::SameLine();
		if (ImGui::Button("Set Statustext"))
		{
			const auto vtable = reinterpret_cast<CStatusBar_vtbl*>(CMainFrame::ActiveWindow->m_wndStatusBar.__vftable);
			vtable->SetStatusText(&CMainFrame::ActiveWindow->m_wndStatusBar, 0x75);
		}

		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if(ImGui::Button("Toggle Toolbar"))
		{
			typedef void(__thiscall* CFrameWnd_ShowControlBar_t)(CFrameWnd*, CControlBar*, BOOL bShow, BOOL bDelay);
			CFrameWnd_ShowControlBar_t CFrameWnd_ShowControlBar = reinterpret_cast<CFrameWnd_ShowControlBar_t>(0x59E9DD);

			auto vtable = reinterpret_cast<CToolBar_vtbl*>(CMainFrame::ActiveWindow->m_wndToolBar.__vftable);
			CFrameWnd_ShowControlBar(CMainFrame::ActiveWindow, &CMainFrame::ActiveWindow->m_wndToolBar, vtable->IsVisible(&CMainFrame::ActiveWindow->m_wndToolBar) ? 0 : 1, 1);
		}

		// TODO! - do not show on the main toolbar
		ImGui::SameLine();
		if (ImGui::Button("Toggle Menubar"))
		{
			if(!ggui::mainframe_menubar_visible)
			{
				const auto menubar = LoadMenu(CMainFrame::ActiveWindow->m_pModuleState->m_hCurrentInstanceHandle, MAKEINTRESOURCE(0xD6)); // 0xD6 = IDR_MENU_QUAKE3
				SetMenu(CMainFrame::ActiveWindow->GetWindow(), menubar);
				ggui::mainframe_menubar_visible = true;
			}
			else
			{
				SetMenu(CMainFrame::ActiveWindow->GetWindow(), 0);
				ggui::mainframe_menubar_visible = false;
			}
		}

	END_GUI:
		ImGui::PopStyleColor(_stylecolors);
		ImGui::PopStyleVar(_stylevars);
		ImGui::End();
	}

	
	// *
	// main rendering loop (D3D9Ex::D3D9Device::EndScene())
	void Gui::render_loop()
	{
		// *
		// menus

		//Gui::any_open_menus();

		/* - radiant draws multiple windows using d3d => multiple endscene / present calls
		 * - each window should have its own imgui context
		 * - use dx->targetWindowIndex to distinguish between windows
		 */
		
		if (Game::dx->targetWindowIndex == ggui::CCAMERAWND)
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
			Gui::begin_frame();

			// TODO! always show demo window (for now)
			ImGui::ShowDemoWindow(nullptr);
			
			// end the current context frame
			goto END_FRAME;
		}

		// ------------------------------------------------------
		
		if (Game::dx->targetWindowIndex == ggui::CXYWND) //if (Game::Globals::gui_present.CXYWnd)
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
			Gui::begin_frame();

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
		Gui::end_frame();

		ImGuiIO& io = ImGui::GetIO();

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	// *
	// shutdown imgui when game window resets (Window::CreateMainWindow())
	void Gui::shutdown()
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

	
	bool Gui::all_contexts_ready()
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
	void Gui::toggle(ggui::imgui_context_menu& menu, [[maybe_unused]] int keycatcher, bool onCommand = false)
	{
		if(!Gui::all_contexts_ready())
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
	void Gui::begin_frame()
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	
	// *
	void Gui::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	
	// *
	//bool Gui::any_open_menus()
	//{
	//	auto& gui = GET_GGUI;
	//	for (int m = 0; m < GGUI_MENU_COUNT; m++)
	//	{
	//		if (gui.menus[m].menustate)
	//		{
	//			if (gui.menus[m].mouse_ignores_menustate)
	//			{
	//				gui.any_menus_open = false;
	//				return false;
	//			}
	//			else
	//			{
	//				gui.any_menus_open = true;
	//				return true;
	//			}
	//		}
	//	}

	//	// negative flag
	//	if (gui.any_menus_open)
	//	{
	//		gui.any_menus_open = false;
	//	}

	//	
	//	return false;
	//}

	
	// *
	void Gui::register_dvars()
	{
		//Dvars::imgui_devgui_pos_x = Dvars::Register_AddonFloat(
		//	/* name		*/ "imgui_devgui_pos_x",
		//	/* value	*/ 20.0f,
		//	/* minVal	*/ 0.0f,
		//	/* maxVal	*/ 4096.0f,
		//	/* flags	*/ Game::dvar_flags::saved,
		//	/* desc		*/ "X position of the devgui");
	}

	
	// *
	Gui::Gui()
	{
		Command::RegisterCommand("demo"s, [](std::vector<std::string> args)
		{
			Gui::toggle(ggui::state.cxywnd.menus[ggui::e_menu::XY_DEMO], 0, true);
		});
	}

	Gui::~Gui()
	{ }
}