#include "std_include.hpp"
#include <iomanip>

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define mainframe_thiscall(return_val, addr)	\
	utils::hook::call<return_val(__fastcall)(cmainframe*)>(addr)(cmainframe::activewnd)

#define mainframe_cdeclcall(return_val, addr)	\
	utils::hook::call<return_val(__cdecl)(cmainframe*)>(addr)(cmainframe::activewnd)

#define mainframe_stdcall(return_val, addr)	\
	utils::hook::call<return_val(__stdcall)(cmainframe*)>(addr)(cmainframe::activewnd)

#define cdeclcall(return_val, addr)	\
	utils::hook::call<return_val(__cdecl)()>(addr)()


#define IMGUI_REGISTER_TOGGLEABLE_MENU(menu, function, function_on_close) \
    if(menu.menustate) {		\
        function;				\
        menu.was_open = true;	\
    }							\
    else if(menu.was_open) {	\
		function_on_close;		\
		menu.was_open = false;	\
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


	/* ---- ref -----
	 * auto vtable = reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable);
	 * reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable)->SetStatusText(&cmainframe::activewnd->m_wndStatusBar, 0x75);
	 *
	 * auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);
	 * vtable->RecalcLayout(&cmainframe::activewnd->m_wndSplit);
	 */


namespace components
{
	void imgui_init_fonts()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_bold_compressed_data, fonts::opensans_bold_compressed_size, 18.0f);
		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 12.0f);
		io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);
	}
	
	// *
	// initialize the imgui camerawnd context
	void imgui_init_ccamerawnd()
	{
		// get the device from d3d9ex::_d3d9/Ex::CreateDevice
		IDirect3DDevice9* device = game::glob::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");
		
		if (!cmainframe::activewnd->m_pCamWnd)
		{
			return;
		}

		// create/set context
		ggui::state.ccamerawnd.context = ImGui::CreateContext();
		ImGui::SetCurrentContext(ggui::state.ccamerawnd.context);
		
        ImGuiIO& io = ImGui::GetIO();
		imgui_init_fonts();
		
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		io.ConfigWindowsResizeFromEdges = true;
		io.IniFilename = "iw3r_ccam_imgui.ini";
		
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

		if(!cmainframe::activewnd->m_pXYWnd)
		{
			return;
		}

		// create/set context
		ggui::state.cxywnd.context = ImGui::CreateContext();
		ImGui::SetCurrentContext(ggui::state.cxywnd.context);

		ImGuiIO& io = ImGui::GetIO();
		imgui_init_fonts();
		
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		io.ConfigWindowsResizeFromEdges = true;
		io.IniFilename = "iw3r_cxy_imgui.ini";
		
		// implementation
		ImGui_ImplWin32_Init(cmainframe::activewnd->m_pXYWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();

		// fully initialized
		ggui::state.cxywnd.context_initialized = true;
		ggui::state.cxywnd.dx_window = &game::dx->windows[ggui::CXYWND];
	}


	// *
	// dockspace creation
	// TODO! - rewrite when docking v3 is avail.
	
	void cxywnd_gui(ggui::imgui_context_cxy& context)
	{
		int _stylevars = 0;
		int _stylecolors = 0;

		ImGuiIO& io = ImGui::GetIO();
		
		bool floating_toolbar = dvars::gui_floating_toolbar && dvars::gui_floating_toolbar->current.enabled;
		bool floating_toolbar_resizes_dockspace = dvars::gui_resize_dockspace && dvars::gui_resize_dockspace->current.enabled;

		
		// *
		// create main dockspace

		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGuiWindowFlags
		window_flags  = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		window_flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::ToImVec4(dvars::gui_menubar_bg_color->current.vector));		_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ToImVec4(dvars::gui_dockedwindow_bg_color->current.vector));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));										_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));									_stylecolors++;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		// use padding to make room for the top / left floating toolbar
		auto window_padding = ImVec2(0.0f, 0.0f);
		if(floating_toolbar && floating_toolbar_resizes_dockspace)
		{
			window_padding = ImVec2(5.0f, 50.0f);
			
			if(ggui::toolbar_axis == ImGuiAxis_Y)
			{
				window_padding = ImVec2(50.0f, 5.0f);
			}
		}
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, window_padding);
		ImGui::Begin("dockspace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		
		// *
		// menubar

		auto saved_menubar_offset = ImVec2(0.0f, 0.0f);
		const auto current_window = ImGui::GetCurrentWindow();

		// dirty hack to remove horizontal window padding from the menubar (left -> first element)
		if (ggui::toolbar_axis == ImGuiAxis_Y)
		{
			saved_menubar_offset = current_window->DC.MenuBarOffset;
			current_window->DC.MenuBarOffset.x = 10.0f;
		}

		ggui::menubar::menu(context);
		ggui::menubar_height = ImGui::GetCurrentWindow()->MenuBarHeight();

		// restore window padding
		if (ggui::toolbar_axis == ImGuiAxis_Y)
		{
			current_window->DC.MenuBarOffset = saved_menubar_offset;
		}
		
		// re-check toolbar dvars in case they got modified from within the menubar
		floating_toolbar = dvars::gui_floating_toolbar && dvars::gui_floating_toolbar->current.enabled;
		floating_toolbar_resizes_dockspace = dvars::gui_resize_dockspace && dvars::gui_resize_dockspace->current.enabled;

		// set toolbar axis once using saved settings
		if(!context.m_toolbar.one_time_init)
		{
			const ImGuiID toolbar_id = ImHashStr("toolbar##xywnd");
			if (const auto	settings = ImGui::FindWindowSettings(toolbar_id);
				settings)
			{
				ggui::toolbar_axis =
					settings->Size.y > settings->Size.x ? ImGuiAxis_Y : ImGuiAxis_X;

				ggui::toolbar_dock_left = settings->DockId;
				ggui::toolbar_dock_top = settings->DockId;
			}
		}

		
		// *
		// create default docking layout

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			auto dockspace_size = ImVec2(0.0f, 0.0f);
			if (floating_toolbar && floating_toolbar_resizes_dockspace)
			{
				dockspace_size = ImVec2(0.0f, viewport->Size.y - 85.0f);

				if (ggui::toolbar_axis == ImGuiAxis_Y)
				{
					//auto region_left = ImGui::GetContentRegionAvailWidth();
					dockspace_size = ImVec2(viewport->Size.x - (2.0f * ggui::toolbar_size.x), 0.0f);
				}
			}

			ImGuiID main_dock;
			ImGuiID dockspace_id = ImGui::GetID("cxywnd_dockspace_layout");
			ImGui::DockSpace(dockspace_id, dockspace_size, ImGuiDockNodeFlags_PassthruCentralNode);


			// *
			// main dockspace
			// - init once on startup
			// - uses "saved" user layout from the ini or creates a default layout
			// - reset to the default layout at any time, respecting toolbar position
			
			if (!context.m_toolbar.one_time_init || ggui::reset_dockspace)
			{
				// get the root node (DockSpace)
				const auto root = ImGui::DockBuilderGetNode(dockspace_id);

				// check if there is a basic tree
				if (  !ggui::reset_dockspace 
					&& root
					&& root->ChildNodes[0])
					//&& root->ChildNodes[1] 
					//&& root->ChildNodes[1]->ChildNodes[0] 
					//&& root->ChildNodes[1]->ChildNodes[1]) 
				{
					// the way we create the initial layout results in the following ID's
					ggui::toolbar_dock_top = ImGui::FindNodeByID(0x00000001);
					ggui::toolbar_dock_left = ImGui::FindNodeByID(0x00000003);
				}
				else 
				{
					// clear existing layout
					ImGui::DockBuilderRemoveNode(dockspace_id); 

					// empty dockspace node
					main_dock = ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); 
					ImGui::DockBuilderSetNodeSize(main_dock, viewport->Size);
					
					ggui::toolbar_dock_top = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Up, 0.01f, nullptr, &main_dock);
					ImGui::DockBuilderSetNodeSize(ggui::toolbar_dock_top, ImVec2{ viewport->Size.x, 36 });

					ggui::toolbar_dock_left = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Left, 0.01f, nullptr, &main_dock);
					ImGui::DockBuilderSetNodeSize(ggui::toolbar_dock_left, ImVec2(36, viewport->Size.y));
					
					ggui::dockspace_sidebar_id = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Left, 0.35f, nullptr, &main_dock);

					if (!floating_toolbar)
					{
						if (ggui::toolbar_axis == ImGuiAxis_X)
						{
							ImGui::DockBuilderDockWindow("toolbar##xywnd", ggui::toolbar_dock_top);
						}
						else
						{
							ImGui::DockBuilderDockWindow("toolbar##xywnd", ggui::toolbar_dock_left);
						}
					}

					ImGui::DockBuilderDockWindow("Colors##xywnd", ggui::dockspace_sidebar_id);
					ImGui::DockBuilderDockWindow("Hotkeys##xywnd", ggui::dockspace_sidebar_id);
					ImGui::DockBuilderDockWindow("Hotkeys Helper##xywnd", ggui::dockspace_sidebar_id);
					ImGui::DockBuilderDockWindow("Toolbar Editor##xywnd", ggui::dockspace_sidebar_id);

					ImGui::DockBuilderFinish(dockspace_id);
				}

				context.m_toolbar.one_time_init = true;
				ggui::reset_dockspace = false;
			}
			
		}

		ImGui::End();

		
		// *
		// toolbar
		
		ggui::toolbar::menu_new(context.m_toolbar, context.m_toolbar_edit);
		//ggui::toolbar::menu_old(context.m_toolbar);


		// *
		// clean-up
		
		ImGui::PopStyleColor(_stylecolors);
		ImGui::PopStyleVar(_stylevars);
	}

	void copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest, [[maybe_unused]] ImVec2 dest_size = ImVec2(0, 0))
	{
		//IDirect3DTexture9* dest = *dest_d;
		
		// get the backbuffer surface
		IDirect3DSurface9* surf_backbuffer = nullptr;
		game::dx->windows[wnd].swapChain->lpVtbl->GetBackBuffer((IDirect3DSwapChain9*)game::dx->windows[wnd].swapChain, 0, D3DBACKBUFFER_TYPE_MONO, &surf_backbuffer);

		// write surface to file (test)
		//D3DXSaveSurfaceToFileA("surface_to_file.png", D3DXIFF_PNG, surf_backbuffer, NULL, NULL);

		// check if window size was changed -> release and recreate the texture surface
		if (dest)
		{
			D3DSURFACE_DESC desc;
			dest->GetLevelDesc(0, &desc);

			if (desc.Width != static_cast<unsigned int>(game::dx->windows[wnd].width) || desc.Height != static_cast<unsigned int>(game::dx->windows[wnd].height))
			{
				dest->Release();
				dest = nullptr;
			}
		}

		// create or re-create ^ the texture surface
		if (!dest)
		{
			/*if(dest_size.x != 0.0f && dest_size.y != 0.0f)
			{
				D3DXCreateTexture(game::dx->device, dest_size.x, dest_size.y, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &dest);
			}
			else
			{*/ // D3DFMT_A8R8G8B8
				D3DXCreateTexture(game::dx->device, game::dx->windows[wnd].width, game::dx->windows[wnd].height, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_R8G8B8, D3DPOOL_DEFAULT, &dest);
			//}
		}

		// "bind" texture to surface
		IDirect3DSurface9* surf_texture;
		dest->GetSurfaceLevel(0, &surf_texture);

		// "copy" backbuffer to our texture surface
		game::dx->device->StretchRect(surf_backbuffer, NULL, surf_texture, NULL, D3DTEXF_NONE);

		// release the backbuffer surface or we'll leak memory
		surf_backbuffer->Release();

		// write texture to file (test)
		//D3DXSaveTextureToFileA("texture_to_file.png", D3DXIFF_PNG, dest, NULL);
	}

	void debug_table_entry_vec3(const char* label, const float* vec3)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);
		
		for (auto i = 0; i < 3; i++) 
		{
			ImGui::TableNextColumn();
			ImGui::Text("%.2f", vec3[i]);
		}
	}

	void debug_table_entry_float(const char* label, const float* val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);
		
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", val);
	}

	void debug_table_entry_int(const char* label, const int val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);
		
		ImGui::TableNextColumn();
		ImGui::Text("%d", val);
	}

	void debug_table_entry_int2(const char* label, const int* val)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(label);
		
		for (auto i = 0; i < 2; i++) 
		{
			ImGui::TableNextColumn();
			ImGui::Text("%d", val[i]);
		}
	}

	ImVec2 pt_last_cursor = ImVec2(0,  0);
	ImVec2 pt_button = ImVec2(0, 0);
	int imgui_cam_mousestate = 0;
	bool imgui_cam_mouse1_was_down = false;
	bool imgui_cam_mouse2_was_down = false;
	
	void cam_mousemoved(ccamwnd* wnd, int buttons, int x, int y)
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
	// main rendering loop (d3d9ex::d3d9device::EndScene())
	void gui::render_loop()
	{
		/* - radiant draws multiple windows using d3d => multiple endscene / present calls
		 * - each window should have its own imgui context
		 * - use dx->targetWindowIndex to distinguish between windows
		 */

		 // *
		 // | -------------------- Camera Window ------------------------
		 // *
		
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

			// copy scene to texture
			copy_scene_to_texture(ggui::CCAMERAWND, game::glob::scene_texture_ccam, game::glob::scene_texture_ccam_dest_size);

			ImGui::SetNextWindowSizeConstraints(ImVec2(200, 400), ImVec2(FLT_MAX, FLT_MAX));
			ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
			ImGui::Begin("Camera Debug", nullptr, ImGuiWindowFlags_NoCollapse);

			const auto ccam = cmainframe::activewnd->m_pCamWnd;

			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
			if (ImGui::TreeNodeEx("Camera Struct", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
				if (ImGui::BeginTable("camera_struct_table", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn(" Camera Struct", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 120.0f);
					ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 38.0f);
					ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 38.0f);
					ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthFixed, 38.0f);
					ImGui::TableHeadersRow();

					debug_table_entry_vec3("Origin", ccam->camera.origin);
					debug_table_entry_vec3("Angles", ccam->camera.angles);
					debug_table_entry_vec3("Forward", ccam->camera.forward);
					debug_table_entry_vec3("Right", ccam->camera.right);
					debug_table_entry_int("Viewport Width", ccam->camera.width);
					debug_table_entry_int("Viewport Height", ccam->camera.height);

					ImGui::EndTable();
				}
				ImGui::PopStyleVar();
				ImGui::TreePop();
			}
			
			if (ImGui::TreeNodeEx("CPoints", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
				if (ImGui::BeginTable("cpoints_table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn(" CPoints", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 120.0f);
					ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 38.0f);
					ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 38.0f);
					ImGui::TableHeadersRow();

					debug_table_entry_int2("PT Cursor", reinterpret_cast<int*>(&ccam->m_ptCursor));
					debug_table_entry_int2("PT Last Cursor", reinterpret_cast<int*>(&ccam->m_ptLastCursor));
					debug_table_entry_int2("PT Button", reinterpret_cast<int*>(&ccam->m_ptButton));
					
					ImGui::EndTable();
				}
				ImGui::PopStyleVar();
				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Misc", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Button State: \t < %d >", ccam->m_nCambuttonstate);
				ImGui::TreePop();
			}
			
			ImGui::PopStyleVar();
			ImGui::End();

			// TODO! always show demo window (for now)
			//ImGui::ShowDemoWindow(nullptr);

			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.ccamerawnd.m_demo,
				ImGui::ShowDemoWindow(&ggui::state.ccamerawnd.m_demo.menustate), nullptr);
			
			// end the current context frame
			goto END_FRAME;
		}

		
		// *
		// | -------------------- XY Window ------------------------
		// *
		
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
			style.Colors[ImGuiCol_WindowBg] = ImGui::ToImVec4(dvars::gui_window_bg_color->current.vector);

			// begin context frame
			gui::begin_frame();

			// docking, default layout ... 
			cxywnd_gui(ggui::state.cxywnd);

			auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));

			//game::glob::camera_in_xy_active = true;
			//ImGui::SetNextWindowSize(camera_size + ImVec2(8, 8));
			
			ImGui::Begin("camera to texture", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize/*| ImGuiWindowFlags_NoResize*/);
			if (game::glob::scene_texture_ccam)
			{
				const auto IO = ImGui::GetIO();
				const auto ccam = cmainframe::activewnd->m_pCamWnd;
				bool is_camera_preview_hovered = false;
				
				// hack to disable left mouse window movement
				ImGui::BeginChild("scene_child", camera_size, false, ImGuiWindowFlags_NoMove);
				{
					const auto pre_image_cursor = ImGui::GetCursorPos();
					//game::glob::scene_texture_ccam_dest_size = ImGui::GetContentRegionAvail();
					//ImGui::Image(game::glob::scene_texture_ccam, game::glob::scene_texture_ccam_dest_size);
					ImGui::Image(game::glob::scene_texture_ccam, camera_size);

					if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
					{
						is_camera_preview_hovered = true;
					}

					ImGui::SetCursorPos(pre_image_cursor);
					const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

					ImGui::Indent(4.0f);

					ImGui::Text("Cursor Screen Pos:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
					ImGui::Text("< %.2f, %.2f >", cursor_screen_pos.x, cursor_screen_pos.y);

					ImGui::Text("Mouse Pos:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
					ImGui::Text("< %.2f, %.2f >", IO.MousePos.x, IO.MousePos.y);

					auto internal_last_cursor = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
					CPoint internal_last_cursor_point = CPoint(internal_last_cursor.x, internal_last_cursor.y);
					ImGui::Text("PT Last Cursor:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
					ImGui::Text("< %.2f, %.2f >", internal_last_cursor.x, internal_last_cursor.y);

					imgui_cam_mousestate |= (IO.KeyShift ? 4 : 0);
					imgui_cam_mousestate |= (IO.KeyCtrl  ? 8 : 0);

					// left mouse down
					if (is_camera_preview_hovered && IO.MouseClicked[0])
					{
						typedef  void(__thiscall* CamWnd__DropModelsToPlane_t)(ccamwnd*, int x, int y, int buttons);
						const auto CamWnd__DropModelsToPlane = reinterpret_cast<CamWnd__DropModelsToPlane_t>(0x403D30);

						ccam->m_ptLastCursor = internal_last_cursor_point;
						imgui_cam_mousestate |= 1;

						CamWnd__DropModelsToPlane(ccam, internal_last_cursor_point.x, ccam->camera.height - internal_last_cursor_point.y - 1, imgui_cam_mousestate);
					}
					
					// right mouse down
					if (is_camera_preview_hovered && IO.MouseClicked[1])
					{
						typedef  void(__thiscall* CamWnd__DropModelsToPlane_t)(ccamwnd*, int x, int y, int buttons);
						const auto CamWnd__DropModelsToPlane = reinterpret_cast<CamWnd__DropModelsToPlane_t>(0x403D30);

						imgui_cam_mousestate |= 2;
						
						CamWnd__DropModelsToPlane(ccam, internal_last_cursor_point.x, ccam->camera.height - internal_last_cursor_point.y - 1, imgui_cam_mousestate);
					}
					
					//if (is_camera_preview_hovered && ImGui::IsAnyMouseDown())
					//{
					//	pt_button = ImVec2(internal_last_cursor.x, (internal_last_cursor.y - ccam->camera.height) * -1);

					//	imgui_cam_mousestate |= (IO.MouseDown[0] ? 1 : 0);
					//	imgui_cam_mousestate |= (IO.MouseDown[1] ? 2 : 0);

					//	// TODO! - no mouse 3/4/5 input
					//	imgui_cam_mousestate |= (IO.MouseDown[2] ? 16 : 0);
					//	imgui_cam_mousestate |= (IO.MouseDown[3] ? 32 : 0);
					//	imgui_cam_mousestate |= (IO.MouseDown[4] ? 64 : 0);

					//	

					//	//typedef  void(__thiscall* CCamWnd__OnMouseMove_t)(ccamwnd*, int buttons, CPoint pos);
					//	//const auto CCamWnd__OnMouseMove = reinterpret_cast<CCamWnd__OnMouseMove_t>(0x403100);

					//	//CCamWnd__OnMouseMove(ccam, imgui_cam_mousestate, new_pos);
					//	CPoint point = CPoint(internal_last_cursor.x, internal_last_cursor.y);
					//	
					//	if(pt_last_cursor.x != internal_last_cursor.x || pt_last_cursor.y != internal_last_cursor.y)
					//	{
					//		cam_mousemoved(ccam, imgui_cam_mousestate, point.x, ccam->camera.height - point.y - 1);

					//		pt_last_cursor.x = internal_last_cursor.x;
					//		pt_last_cursor.y = internal_last_cursor.y;
					//		ccam->m_ptLastCursor.x = internal_last_cursor.x;
					//		ccam->m_ptLastCursor.y = internal_last_cursor.y;
					//	}
					//}

					if(is_camera_preview_hovered)
					{
						if(IO.MouseDown[0] && !imgui_cam_mouse1_was_down)
						{
							imgui_cam_mouse1_was_down = true;
						}

						if (IO.MouseDown[1] && !imgui_cam_mouse2_was_down)
						{
							imgui_cam_mouse2_was_down = true;
						}
					}

					if (!IO.MouseDown[0] && imgui_cam_mouse1_was_down)
					{
						// on mouse up
						ccam->m_nCambuttonstate = 0;
						ccam->prob_some_cursor = 0;
						ccam->x47 = 0;

						int result;
						
						do
						{
							result = ShowCursor(1);
						} while (result < 0);
						
						imgui_cam_mouse1_was_down = false;
					}

					if (!IO.MouseDown[1] && imgui_cam_mouse2_was_down)
					{
						// on mouse up
						ccam->m_nCambuttonstate = 0;
						ccam->prob_some_cursor = 0;
						ccam->x47 = 0;

						int result;

						do
						{
							result = ShowCursor(1);
						} while (result < 0);
						
						imgui_cam_mouse2_was_down = false;
					}
    
					ImGui::Text("PT Button:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
					ImGui::Text("< %.2f, %.2f >", pt_button.x, pt_button.y);

					ImGui::Text("Mouse State:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
					ImGui::Text("< %d >", imgui_cam_mousestate);

					/*ccam->m_nCambuttonstate = imgui_cam_mousestate;
					ccam->m_ptButton.x = pt_button.x;
					ccam->m_ptButton.y = pt_button.y;
					ccam->m_ptLastCursor.x = pt_last_cursor.x;
					ccam->m_ptLastCursor.y = pt_last_cursor.y;*/
					
					imgui_cam_mousestate = 0;
					
					ImGui::EndChild();
				}
			}
			ImGui::End();
			
			// color menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_colors, 
				ggui::colors::menu(ggui::state.cxywnd.m_colors), nullptr);

			// toolbar edit menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_toolbar_edit,
				ggui::toolbar::menu_toolbar_edit(ggui::state.cxywnd.m_toolbar_edit), ggui::toolbar::save_settings_ini());
			
			// command bind menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_cmdbinds, 
				ggui::hotkeys::menu(ggui::state.cxywnd.m_cmdbinds), ggui::hotkeys::on_close());

			// command bind helper menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_cmdbinds_helper,
				ggui::hotkeys::helper_menu(ggui::state.cxywnd.m_cmdbinds_helper), nullptr);
			
			// demo menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_demo,
					ImGui::ShowDemoWindow(&ggui::state.cxywnd.m_demo.menustate), nullptr);

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
		dvars::gui_menubar_bg_color = dvars::register_vec4(
			/* name		*/ "gui_menubar_bg_color",
			/* x		*/ 0.30f,
			/* y		*/ 0.01f,
			/* z		*/ 0.02f,
			/* w		*/ 0.6f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui menubar background color");
		
		dvars::gui_dockedwindow_bg_color = dvars::register_vec4(
			/* name		*/ "gui_dockedwindow_bg_color",
			/* x		*/ 0.050f,
			/* y		*/ 0.050f,
			/* z		*/ 0.050f,
			/* w		*/ 0.65f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui toolbar background color");

		dvars::gui_window_bg_color = dvars::register_vec4(
			/* name		*/ "gui_window_bg_color",
			/* x		*/ 0.050f,
			/* y		*/ 0.050f,
			/* z		*/ 0.050f,
			/* w		*/ 0.65f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui background color");

		dvars::gui_floating_toolbar = dvars::register_bool(
			/* name		*/ "gui_floating_toolbar",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "enable floating toolbar (saved between restarts)");

		dvars::gui_resize_dockspace = dvars::register_bool(
			/* name		*/ "gui_resize_dockspace",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "resize dockspace if floating toolbar is enabled (for top-floating toolbar)");
	}

	
	// *
	gui::gui()
	{
		// hotkey hooks
		ggui::hotkeys::init();

		
		command::register_command("xydemo"s, [](std::vector<std::string> args)
		{
			gui::toggle(ggui::state.cxywnd.m_demo, 0, true);
		});

		command::register_command("camdemo"s, [](std::vector<std::string> args)
		{
			gui::toggle(ggui::state.ccamerawnd.m_demo, 0, true);
		});
	}

	gui::~gui()
	{ }
}