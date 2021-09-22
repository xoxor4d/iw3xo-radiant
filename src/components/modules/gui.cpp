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

#if USE_LAYERED_AS_BACKGROUND
	// *
	// initialize imgui context
	void imgui_init_layered()
	{
		// get the device from d3d9ex::_d3d9/Ex::CreateDevice
		IDirect3DDevice9* device = game::glob::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");

		if (!cmainframe::activewnd->m_pXYWnd)
		{
			return;
		}

		// create/set context
		ggui::state.clayeredwnd.context = ImGui::CreateContext();
		ImGui::SetCurrentContext(ggui::state.clayeredwnd.context);

		ImGuiIO& io = ImGui::GetIO();
		imgui_init_fonts();

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		io.ConfigWindowsResizeFromEdges = true;
		io.IniFilename = "iw3r_layered_imgui.ini";

		// implementation
		ImGui_ImplWin32_Init(layermatwnd_struct->m_hWnd);
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();

		// fully initialized
		ggui::state.clayeredwnd.context_initialized = true;
		ggui::state.clayeredwnd.dx_window = &game::dx->windows[ggui::LAYERED];
	}
#endif

	
	// *
	// initialize imgui context
	void imgui_init_czwnd()
	{
		// get the device from d3d9ex::_d3d9/Ex::CreateDevice
		IDirect3DDevice9* device = game::glob::d3d9_device;
		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");

		if (!cmainframe::activewnd->m_pZWnd)
		{
			return;
		}

		// create/set context
		ggui::state.czwnd.context = ImGui::CreateContext();
		ImGui::SetCurrentContext(ggui::state.czwnd.context);

		ImGuiIO& io = ImGui::GetIO();
		imgui_init_fonts();

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows
		io.ConfigWindowsResizeFromEdges = true;
		io.IniFilename = "iw3r_czwnd_imgui.ini";

		// implementation
		ImGui_ImplWin32_Init(cmainframe::activewnd->m_pZWnd->GetWindow());
		ImGui_ImplDX9_Init(device);

		// style
		ImGui::StyleColorsDevgui();

		// fully initialized
		ggui::state.czwnd.context_initialized = true;
		ggui::state.czwnd.dx_window = &game::dx->windows[ggui::CZWND];
	}

	
	// *
	// dockspace creation
	// TODO! - rewrite when docking v3 is avail.
	
	void czwnd_gui(ggui::imgui_context_cz& context)
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
		// render backgrounds
		
		if(dvars::gui_mainframe_background)
		{
			if (dvars::gui_mainframe_background->current.integer == 1)
			{
				if (game::g_PrefsDlg()->m_nView == 1)
				{
					SetWindowPos(cmainframe::activewnd->m_pXYWnd->m_hWnd, HWND_BOTTOM, 0, 0, zwnd->width, zwnd->height, SWP_NOZORDER);
					const auto cxy_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nWidth), static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nHeight));

					ImGui::SetNextWindowPos(ImVec2(0, 0));
					ImGui::SetNextWindowSize(cxy_size);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					ImGui::Begin("mainview_grid", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
					{
						if (ggui::rtt_gridwnd.scene_texture)
						{
							const auto IO = ImGui::GetIO();
							const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

							ImGui::Image(ggui::rtt_gridwnd.scene_texture, cxy_size);
							ggui::rtt_gridwnd.window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);

							ggui::rtt_gridwnd.cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
							ggui::rtt_gridwnd.cursor_pos_pt = CPoint((LONG)ggui::rtt_gridwnd.cursor_pos.x, (LONG)ggui::rtt_gridwnd.cursor_pos.y);
						}
					}
					ImGui::PopStyleVar(2);
					ImGui::End();
				}
			}
			
			else if (dvars::gui_mainframe_background->current.integer == 2)
			{
				if (game::g_PrefsDlg()->m_nView == 1)
				{
					SetWindowPos(cmainframe::activewnd->m_pCamWnd->m_hWnd, HWND_BOTTOM, 0, 0, zwnd->width, zwnd->height, SWP_NOZORDER);
					auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));

					ImGui::SetNextWindowPos(ImVec2(0, 0));
					ImGui::SetNextWindowSize(camera_size);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					ImGui::Begin("mainview_camera", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
					{
						if (ggui::rtt_camerawnd.scene_texture)
						{
							const auto IO = ImGui::GetIO();
							const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

							ImGui::Image(ggui::rtt_camerawnd.scene_texture, camera_size);
							ggui::rtt_camerawnd.window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);

							ggui::rtt_camerawnd.cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
							ggui::rtt_camerawnd.cursor_pos_pt = CPoint((LONG)ggui::rtt_camerawnd.cursor_pos.x, (LONG)ggui::rtt_camerawnd.cursor_pos.y);
						}
					}
					ImGui::PopStyleVar(2);
					ImGui::End();
				}
			}
		}

		
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


		// *
		// toolbar settings
		
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

	
	void copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest)
	{
		// get the backbuffer surface
		IDirect3DSurface9* surf_backbuffer = nullptr;
		//game::dx->windows[wnd].swapChain->lpVtbl->GetBackBuffer((IDirect3DSwapChain9*)game::dx->windows[wnd].swapChain, 0, D3DBACKBUFFER_TYPE_MONO, &surf_backbuffer);
		game::dx->windows[wnd].swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surf_backbuffer);
		
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
			D3DXCreateTexture(game::dx->device, game::dx->windows[wnd].width, game::dx->windows[wnd].height, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_R8G8B8, D3DPOOL_DEFAULT, &dest);
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
	

	// render to texture - grid window
	void rtt_grid_window()
	{
		int p_styles = 0;
		int p_colors = 0;
		
		const auto cxy_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nWidth), static_cast<float>(cmainframe::activewnd->m_pXYWnd->m_nHeight));
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); p_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;

		ImGui::Begin("cxy to texture", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		if (ggui::rtt_gridwnd.scene_texture)
		{
			const auto IO = ImGui::GetIO();
			ggui::rtt_gridwnd.scene_size_imgui = ImGui::GetWindowSize() - ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing());

			// hack to disable left mouse window movement
			ImGui::BeginChild("scene_child_cxy", ImVec2(cxy_size.x, cxy_size.y + ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_NoMove);
			{
				const auto screenpos = ImGui::GetCursorScreenPos();
				SetWindowPos(cmainframe::activewnd->m_pXYWnd->m_hWnd, HWND_BOTTOM, (int)screenpos.x, (int)screenpos.y, (int)ggui::rtt_gridwnd.scene_size_imgui.x, (int)ggui::rtt_gridwnd.scene_size_imgui.y, SWP_NOZORDER);
				
				const auto pre_image_cursor = ImGui::GetCursorPos();
				
				ImGui::Image(ggui::rtt_gridwnd.scene_texture, cxy_size);
				ggui::rtt_gridwnd.window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);

				// pop ItemSpacing
				ImGui::PopStyleVar(); p_styles--;
				
				ImGui::SetCursorPos(pre_image_cursor);
				const auto cursor_screen_pos = ImGui::GetCursorScreenPos();
				
				ggui::rtt_gridwnd.cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
				ggui::rtt_gridwnd.cursor_pos_pt = CPoint((LONG)ggui::rtt_gridwnd.cursor_pos.x, (LONG)ggui::rtt_gridwnd.cursor_pos.y);

				/*ImGui::Indent(4.0f);

				ImGui::Text("Cursor Screen Pos:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %.2f, %.2f >", cursor_screen_pos.x, cursor_screen_pos.y);

				ImGui::Text("Mouse Pos:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %.2f, %.2f >", IO.MousePos.x, IO.MousePos.y);

				ImGui::Text("RTT Cursor Pos Pt:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %d, %d >", (int)ggui::rtt_gridwnd.cursor_pos_pt.x, (int)ggui::rtt_gridwnd.cursor_pos_pt.y);

				
				ImGui::Text("CXY Pt Down:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %d, %d >", (int)cmainframe::activewnd->m_pXYWnd->m_ptDown.x, (int)cmainframe::activewnd->m_pXYWnd->m_ptDown.y);

				ImGui::Text("CXY Pt Cursor:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %d, %d >", (int)cmainframe::activewnd->m_pXYWnd->m_ptCursor.x, (int)cmainframe::activewnd->m_pXYWnd->m_ptCursor.y);*/
				
				ImGui::EndChild();
			}
		}
		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();
	}

	
	// render to texture - camera window
	void rtt_camera_window()
	{
		int p_styles = 0;
		int p_colors = 0;
		
		const auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)); p_styles++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
		ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;

		ImGui::Begin("camera to texture", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		if (ggui::rtt_camerawnd.scene_texture)
		{
			const auto IO = ImGui::GetIO();
			ggui::rtt_camerawnd.scene_size_imgui = ImGui::GetWindowSize() - ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing());
			
			//ggui::rtt_camerawnd.scene_size_imgui = ImGui::GetWindowSize();
			//SetWindowPos(cmainframe::activewnd->m_pCamWnd->m_hWnd, HWND_BOTTOM, 0, 0, (int)ggui::rtt_camerawnd.scene_size_imgui.x, (int)ggui::rtt_camerawnd.scene_size_imgui.y, SWP_NOZORDER);

			// hack to disable left mouse window movement
			ImGui::BeginChild("scene_child", ImVec2(camera_size.x, camera_size.y + ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_NoMove);
			{
				const auto screenpos = ImGui::GetCursorScreenPos();
				SetWindowPos(cmainframe::activewnd->m_pCamWnd->GetWindow(), HWND_BOTTOM, (int)screenpos.x, (int)screenpos.y, (int)ggui::rtt_camerawnd.scene_size_imgui.x, (int)ggui::rtt_camerawnd.scene_size_imgui.y, SWP_NOZORDER);

				const auto pre_image_cursor = ImGui::GetCursorPos();

				ImGui::Image(ggui::rtt_camerawnd.scene_texture, camera_size);
				ggui::rtt_camerawnd.window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);

				// pop ItemSpacing
				ImGui::PopStyleVar(); p_styles--;

				ImGui::SetCursorPos(pre_image_cursor);
				const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

				ggui::rtt_camerawnd.cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
				ggui::rtt_camerawnd.cursor_pos_pt = CPoint((LONG)ggui::rtt_camerawnd.cursor_pos.x, (LONG)ggui::rtt_camerawnd.cursor_pos.y);

				/*ImGui::Indent(4.0f);

				ImGui::Text("Cursor Screen Pos:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %.2f, %.2f >", cursor_screen_pos.x, cursor_screen_pos.y);

				ImGui::Text("Mouse Pos:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %.2f, %.2f >", IO.MousePos.x, IO.MousePos.y);

				ImGui::Text("RTT Cursor Pos Pt:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %d, %d >", (int)ggui::rtt_camerawnd.cursor_pos_pt.x, (int)ggui::rtt_camerawnd.cursor_pos_pt.y);


				ImGui::Text("CXY Pt Button:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %d, %d >", (int)cmainframe::activewnd->m_pCamWnd->m_ptButton.x, (int)cmainframe::activewnd->m_pCamWnd->m_ptButton.y);

				ImGui::Text("CXY Pt Cursor:"); ImGui::SameLine(); ImGui::SetCursorPosX(pre_image_cursor.x + 140.0f);
				ImGui::Text("< %d, %d >", (int)cmainframe::activewnd->m_pCamWnd->m_ptCursor.x, (int)cmainframe::activewnd->m_pCamWnd->m_ptCursor.y);*/

				ImGui::EndChild();
			}
		}
		ImGui::PopStyleColor(p_colors);
		ImGui::PopStyleVar(p_styles);
		ImGui::End();
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
			// copy scene to texture
			copy_scene_to_texture(ggui::CCAMERAWND, ggui::rtt_camerawnd.scene_texture);
		}

		
		// *
		// | -------------------- XY Window ------------------------
		// *
		
		if (game::dx->targetWindowIndex == ggui::CXYWND) //if (game::glob::gui_present.cxywnd)
		{
			// copy scene to texture
			copy_scene_to_texture(ggui::CXYWND, ggui::rtt_gridwnd.scene_texture);
		}


		// *
		// | ------------- Layered Materials Window -----------------
		// *
		
#if USE_LAYERED_AS_BACKGROUND
		if (game::dx->targetWindowIndex == ggui::LAYERED)
		{
			if (!ggui::state.clayeredwnd.context_initialized)
			{
				imgui_init_layered();

				// allow 1 frame to pass
				return;
			}

			IMGUI_BEGIN_LAYERED;

			// global style vars for current context
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameBorderSize = 0.0f;
			style.Colors[ImGuiCol_WindowBg] = ImGui::ToImVec4(dvars::gui_window_bg_color->current.vector);

			// begin context frame
			gui::begin_frame();

			// docking, default layout ... 
			czwnd_gui(ggui::state.clayeredwnd);

			// color menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.clayeredwnd.m_colors,
				ggui::colors::menu(ggui::state.clayeredwnd.m_colors), nullptr);

			// toolbar edit menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.clayeredwnd.m_toolbar_edit,
				ggui::toolbar::menu_toolbar_edit(ggui::state.clayeredwnd.m_toolbar_edit), ggui::toolbar::save_settings_ini());

			// command bind menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.clayeredwnd.m_cmdbinds,
				ggui::hotkeys::menu(ggui::state.clayeredwnd.m_cmdbinds), ggui::hotkeys::on_close());

			// command bind helper menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.clayeredwnd.m_cmdbinds_helper,
				ggui::hotkeys::helper_menu(ggui::state.clayeredwnd.m_cmdbinds_helper), nullptr);

			// demo menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.clayeredwnd.m_demo,
				ImGui::ShowDemoWindow(&ggui::state.clayeredwnd.m_demo.menustate), nullptr);

			// end the current context frame
			goto END_FRAME;
		}
#endif


		// *
		// | --------------------- Z Window ------------------------
		// *
		
		if (game::dx->targetWindowIndex == ggui::CZWND)
		{
			if (!ggui::state.czwnd.context_initialized)
			{
				imgui_init_czwnd();

				// allow 1 frame to pass
				return;
			}

			IMGUI_BEGIN_CZWND;

			// global style vars for current context
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameBorderSize = 0.0f;
			style.Colors[ImGuiCol_WindowBg] = ImGui::ToImVec4(dvars::gui_window_bg_color->current.vector);

			// begin context frame
			gui::begin_frame();

			// docking, default layout ... 
			czwnd_gui(ggui::state.czwnd);

			// background
			if (dvars::gui_mainframe_background && dvars::gui_mainframe_background->current.integer != 1) {
				rtt_grid_window();
			}

			if (dvars::gui_mainframe_background && dvars::gui_mainframe_background->current.integer != 2) {
				rtt_camera_window();
			}
			
			// color menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.czwnd.m_colors,
				ggui::colors::menu(ggui::state.czwnd.m_colors), nullptr);

			// toolbar edit menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.czwnd.m_toolbar_edit,
				ggui::toolbar::menu_toolbar_edit(ggui::state.czwnd.m_toolbar_edit), ggui::toolbar::save_settings_ini());

			// command bind menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.czwnd.m_cmdbinds,
				ggui::hotkeys::menu(ggui::state.czwnd.m_cmdbinds), ggui::hotkeys::on_close());

			// command bind helper menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.czwnd.m_cmdbinds_helper,
				ggui::hotkeys::helper_menu(ggui::state.czwnd.m_cmdbinds_helper), nullptr);

			// demo menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.czwnd.m_demo,
				ImGui::ShowDemoWindow(&ggui::state.czwnd.m_demo.menustate), nullptr);

			
#ifdef DEBUG
			ImGui::SetNextWindowSizeConstraints(ImVec2(200, 400), ImVec2(FLT_MAX, FLT_MAX));
			ImGui::SetNextWindowPos(ImVec2(6, 76), ImGuiCond_Appearing);
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

					ImGui::debug_table_entry_vec3("Origin", ccam->camera.origin);
					ImGui::debug_table_entry_vec3("Angles", ccam->camera.angles);
					ImGui::debug_table_entry_vec3("Forward", ccam->camera.forward);
					ImGui::debug_table_entry_vec3("Right", ccam->camera.right);
					ImGui::debug_table_entry_int("Viewport Width", ccam->camera.width);
					ImGui::debug_table_entry_int("Viewport Height", ccam->camera.height);

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

					ImGui::debug_table_entry_int2("PT Cursor", reinterpret_cast<int*>(&ccam->m_ptCursor));
					ImGui::debug_table_entry_int2("PT Last Cursor", reinterpret_cast<int*>(&ccam->m_ptLastCursor));
					ImGui::debug_table_entry_int2("PT Button", reinterpret_cast<int*>(&ccam->m_ptButton));

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
#endif
			
			
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
		if (ggui::state.czwnd.context_initialized)
		{
			IMGUI_BEGIN_CZWND;
			// Shutdown calls d3dDevice->Release D:
			// ImGui_ImplDX9_Shutdown();

			ImGui::DestroyPlatformWindows();
			ImGui_ImplDX9_InvalidateDeviceObjects();

			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
		
#if USE_LAYERED_AS_BACKGROUND
		if (ggui::state.clayeredwnd.context_initialized)
		{
			IMGUI_BEGIN_LAYERED;
			// Shutdown calls d3dDevice->Release D:
			// ImGui_ImplDX9_Shutdown();

			ImGui::DestroyPlatformWindows();
			ImGui_ImplDX9_InvalidateDeviceObjects();

			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
#endif

		memset(&ggui::state, 0, sizeof(ggui::imgui_state_t));
	}

	// *
	// toggle a imgui menu by command (or key (scheduler))
	void gui::toggle(ggui::imgui_context_menu& menu, [[maybe_unused]] int keycatcher, bool onCommand = false)
	{
		if(!ggui::cz_context_ready())
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

		dvars::gui_mainframe_background = dvars::register_int(
			/* name		*/ "gui_mainframe_background",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Window to use as the main background. 0 = None, 1 = Grid, 2 = Camera");
	}

	
	// *
	gui::gui()
	{
		// hotkey hooks
		ggui::hotkeys::init();

		command::register_command("demo"s, [](std::vector<std::string> args)
		{
			gui::toggle(ggui::state.czwnd.m_demo, 0, true);
		});
	}

	gui::~gui()
	{ }
}