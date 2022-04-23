#include "std_include.hpp"

// show tooltip after x seconds
#define TTDELAY 0.5f 

// tooltip with delay
#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) ImGui::SetTooltip(tooltip)

// spacing dummy
#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 

// seperator with spacing
#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing) 

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
		ggui::m_ggui_context = ImGui::CreateContext();
		ImGui::SetCurrentContext(ggui::m_ggui_context);

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
		ggui::m_ggui_initialized = true;
	}

	
	// *
	// dockspace creation
	void handle_dockspace()
	{
		int _stylevars = 0;
		int _stylecolors = 0;

		ImGuiIO& io = ImGui::GetIO();
		const auto tb = GET_GUI(ggui::toolbar_dialog);

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
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ToImVec4(dvars::gui_menubar_bg_color->current.vector));			_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));							_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));					_stylecolors++;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); _stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); _stylevars++;

		// use padding to make room for the top / left floating toolbar
		const auto window_padding = ImVec2(0.0f, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, window_padding); _stylevars++;
		ImGui::Begin("dockspace", nullptr, window_flags);

		ImGui::PopStyleVar(_stylevars); _stylevars = 0;
		ImGui::PopStyleColor(); _stylecolors--;


		// *
		// render backgrounds (makes no sense currently)

#if 0
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
						
						if (auto gridwnd = ggui::get_rtt_gridwnd();
								 gridwnd->scene_texture)
						{
							const auto IO = ImGui::GetIO();
							const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

							ImGui::Image(gridwnd->scene_texture, cxy_size);
							gridwnd->window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);

							gridwnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
							gridwnd->cursor_pos_pt = CPoint((LONG)gridwnd->cursor_pos.x, (LONG)gridwnd->cursor_pos.y);
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
					const auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));

					ImGui::SetNextWindowPos(ImVec2(0, 0));
					ImGui::SetNextWindowSize(camera_size);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					ImGui::Begin("mainview_camera", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);
					{
						if (const auto	camerawnd = GET_GUI(ggui::camera_dialog);
										camerawnd->rtt_get_texture())
						{
							const auto IO = ImGui::GetIO();
							const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

							ImGui::Image(camerawnd->rtt_get_texture(), camera_size);
							camerawnd->rtt_set_hovered_state(ImGui::IsItemHovered(ImGuiHoveredFlags_None));
							camerawnd->rtt_set_cursor_pos(ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y));
						}
					}
					ImGui::PopStyleVar(2);
					ImGui::End();
				}
			}
		}
#endif

		
		// *
		// menubar

		auto saved_menubar_offset = ImVec2(0.0f, 0.0f);
		const auto current_window = ImGui::GetCurrentWindow();

		// dirty hack to remove horizontal window padding from the menubar (left -> first element)
		if (tb->m_toolbar_axis == ImGuiAxis_Y)
		{
			saved_menubar_offset = current_window->DC.MenuBarOffset;
			current_window->DC.MenuBarOffset.x = 10.0f;
		}

		const auto menubar = GET_GUI(ggui::menubar_dialog);
		menubar->menubar();
		menubar->set_height(ImGui::GetCurrentWindow()->MenuBarHeight());


		// *
		// toolbar settings
		
		// restore window padding
		if (tb->m_toolbar_axis == ImGuiAxis_Y)
		{
			current_window->DC.MenuBarOffset = saved_menubar_offset;
		}

		// set toolbar axis once using saved settings
		if(!ggui::m_dockspace_initiated)
		{
			const ImGuiID toolbar_id = ImHashStr("toolbar##xywnd");
			if (const auto  settings = ImGui::FindWindowSettings(toolbar_id);
							settings)
			{
				tb->m_toolbar_axis		= settings->Size.y > settings->Size.x ? ImGuiAxis_Y : ImGuiAxis_X;
				tb->m_toolbar_dock_left = settings->DockId;
				tb->m_toolbar_dock_top	= settings->DockId;
			}
		}


		// *
		// create default docking layout

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			
			ImGuiID main_dock;

			const ImGuiID dockspace_id = ImGui::GetID("cxywnd_dockspace_layout");
			const auto dockspace_size = ImVec2(0.0f, 0.0f);
			const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar;

			ImGui::DockSpace(dockspace_id, dockspace_size, dockspace_flags);

			// *
			// main dockspace
			// - init once on startup
			// - uses "saved" user layout from the ini or creates a default layout
			// - reset to the default layout at any time, respecting toolbar position
			
			if (!ggui::m_dockspace_initiated || ggui::m_dockspace_reset)
			{
				// get the root node (DockSpace)
				const auto root = ImGui::DockBuilderGetNode(dockspace_id);

				// check if there is a basic tree
				if (  !ggui::m_dockspace_reset 
					&& root
					&& root->ChildNodes[0])
				{
					// the way we create the initial layout results in the following ID's
					tb->m_toolbar_dock_top = ImGui::FindNodeByID(0x00000001);
					tb->m_toolbar_dock_left = ImGui::FindNodeByID(0x00000003);
					ggui::m_dockspace_outer_left_node = ImGui::FindNodeByID(0x00000005);
				}
				else 
				{
					// clear existing layout
					ImGui::DockBuilderRemoveNode(dockspace_id); 

					// empty dockspace node
					main_dock = ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); 
					ImGui::DockBuilderSetNodeSize(main_dock, viewport->Size);
					
					tb->m_toolbar_dock_top = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Up, 0.01f, nullptr, &main_dock);
					ImGui::DockBuilderSetNodeSize(tb->m_toolbar_dock_top, ImVec2{ viewport->Size.x, 36 });

					tb->m_toolbar_dock_left = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Left, 0.01f, nullptr, &main_dock);
					ImGui::DockBuilderSetNodeSize(tb->m_toolbar_dock_left, ImVec2(36, viewport->Size.y));
					
					ggui::m_dockspace_outer_left_node = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Left, 0.5f, nullptr, &main_dock);
					const auto dockspace_right_top_node = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Up, 0.6f, nullptr, &main_dock);

					if (tb->m_toolbar_axis == ImGuiAxis_X)
					{
						ImGui::DockBuilderDockWindow("toolbar##window", tb->m_toolbar_dock_top);
					}
					else
					{
						ImGui::DockBuilderDockWindow("toolbar##window", tb->m_toolbar_dock_left);
					}

					ImGui::DockBuilderDockWindow("Grid Window##rtt", ggui::m_dockspace_outer_left_node);
					ImGui::DockBuilderDockWindow("Camera Window##rtt", dockspace_right_top_node);

					ImGui::DockBuilderDockWindow("Textures##rtt", main_dock);
					ImGui::DockBuilderDockWindow("Console##window", main_dock);
					
					ImGui::DockBuilderFinish(dockspace_id);

					// ^ open texture window on initial startup
					if(const auto tex = GET_GUI(ggui::texture_dialog);
								 !tex->is_initiated())
					{
						tex->open();
						tex->set_initiated();
					}
					
					// ^ open console on initial startup
					if(const auto con = GET_GUI(ggui::console_dialog); 
								 !con->is_initiated())
					{
						con->open();
						con->set_initiated();
					}
				}

				ggui::m_dockspace_initiated = true;
				ggui::m_dockspace_reset = false;
			}
		}

		ImGui::End();

		
		// *
		// toolbar

		GET_GUI(ggui::toolbar_dialog)->toolbar();

		
		// *
		// clean-up
		
		ImGui::PopStyleColor(_stylecolors);
		ImGui::PopStyleVar(_stylevars);
	}


	// *
	// main rendering loop (d3d9ex::d3d9device::EndScene())
	void gui::render_loop()
	{
		/* - radiant draws multiple windows using d3d
		 *   => multiple endscene / present calls
		 * - use dx->targetWindowIndex to distinguish between windows
		 */

		 // *
		 // | -------------------- Camera Window ------------------------
		 // *

		if (game::dx->targetWindowIndex == ggui::CCAMERAWND)
		{
			// copy scene to texture
			// done in renderer::camera_postfx() if post effects are active

			if (!renderer::postfx::is_any_active())
			{
				renderer::copy_scene_to_texture(ggui::CCAMERAWND, GET_GUI(ggui::camera_dialog)->rtt_get_texture());
			}
		}

		
		// *
		// | -------------------- Grid Window ------------------------
		// *
		
		if (game::dx->targetWindowIndex == ggui::CXYWND)
		{
			renderer::copy_scene_to_texture(ggui::CXYWND, GET_GUI(ggui::grid_dialog)->rtt_get_texture());
		}

		
		// *
		// | -------------------- Texture Window ------------------------
		// *

		if(game::dx->targetWindowIndex == ggui::CTEXWND)
		{
			renderer::copy_scene_to_texture(ggui::CTEXWND, GET_GUI(ggui::texture_dialog)->rtt_get_texture());
		}


		// *
		// | ------------- Layered Materials Window (Model Preview) -----------------
		// *
		
		if (game::dx->targetWindowIndex == ggui::LAYERED)
		{
			// so empty
		}

		
		// *
		// | --------------------- Z Window (Dear ImGui Canvas) ------------------------
		// *
		
		if (game::dx->targetWindowIndex == ggui::CZWND)
		{
			if (!ggui::m_ggui_initialized)
			{
				imgui_init_czwnd();

				// allow 1 frame to pass
				return;
			}

			IMGUI_BEGIN_CZWND;

			// global style vars for current context
			ImGuiStyle& style = ImGui::GetStyle();

			style.Colors[ImGuiCol_WindowBg] = ImGui::ToImVec4(dvars::gui_window_bg_color->current.vector);
			style.Colors[ImGuiCol_ChildBg] = ImGui::ToImVec4(dvars::gui_window_child_bg_color->current.vector);
			style.Colors[ImGuiCol_Border] = ImGui::ToImVec4(dvars::gui_border_color->current.vector);
			style.Colors[ImGuiCol_DockingPreview] = ImGui::ToImVec4(dvars::gui_dockingpreview_color->current.vector);

			style.FrameBorderSize = 0.0f;
			style.WindowMenuButtonPosition = 0;

			// begin context frame
			gui::begin_frame();

			// load/save window states
			gui::saved_windowstates();

			// used by postfx logic
			game::glob::frames_per_second = ImGui::GetIO().Framerate;

			// docking, default layout ... 
			handle_dockspace();

			// -
			// separate windows for grid/camera if not used as background
#if 0
			if (dvars::gui_mainframe_background && dvars::gui_mainframe_background->current.integer != 1) 
			{
				ggui::grid::gui();
			}

			if (dvars::gui_mainframe_background && dvars::gui_mainframe_background->current.integer != 2) 
			{
				GET_GUI(ggui::camera_dialog)->camera_gui();
			}

#else
			// non-closable windows 
			GET_GUI(ggui::grid_dialog)->grid_gui();
			GET_GUI(ggui::camera_dialog)->camera_gui();
#endif

			if(ggui::m_demo_menu_state)
			{
				// demo menu
				ImGui::ShowDemoWindow(&ggui::m_demo_menu_state);
			}

			// draw/handle gui classes
			for (const auto& module : ggui::loader::get_modules())
			{
				if(module)
				{
					module->frame();
				}
			}

			// handle file dialogs
			ggui::file_dialog_frame();

			// hide external console if it is visible
			if(const auto con = GetConsoleWindow(); 
				IsWindowVisible(con))
			{
				ShowWindow(con, SW_HIDE);
			}

			// debug

			//game::GfxRenderTarget* targets = reinterpret_cast<game::GfxRenderTarget*>(0x174F4A8);
			//game::GfxRenderTarget* depth = &targets[game::R_RENDERTARGET_FLOAT_Z];

			/*ImGui::Begin("Debug", nullptr);
			ImGui::Image(postSun->image->texture.data, ImVec2(game::dx->windows[ggui::e_gfxwindow::CCAMERAWND].width, game::dx->windows[ggui::e_gfxwindow::CCAMERAWND].height));
			ImGui::End();*/

			//if(depth && depth->image && depth->image->texture.data)
			//{
			//	ImGui::Begin("Depthbuffer", nullptr);
			//	ImGui::Image(depth->image->texture.data, ImVec2(ggui::get_rtt_camerawnd()->scene_size_imgui.x, ggui::get_rtt_camerawnd()->scene_size_imgui.y));
			//	ImGui::End();
			//}

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
		if (ggui::m_ggui_initialized)
		{
			IMGUI_BEGIN_CZWND;

			// ImGui_ImplDX9_Shutdown();

			ImGui::DestroyPlatformWindows();
			ImGui_ImplDX9_InvalidateDeviceObjects();

			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
	}

#define HANDLE_SAVED_STATE(_GUI_CLASS, _DVAR, _STARTUP)										\
		if(!(_STARTUP))																		\
		{																					\
			if ((_DVAR))																	\
				GET_GUI(_GUI_CLASS)->toggle(true, (_DVAR)->current.enabled);				\
		}																					\
		else																				\
		{																					\
			if ((_DVAR) && GET_GUI(_GUI_CLASS)->is_active() != (_DVAR)->current.enabled)	\
				dvars::set_bool((_DVAR), GET_GUI(_GUI_CLASS)->is_active());					\
		}																					\

	// *
	// handles opened/closed states of windows via dvars
	// register dvars @ gui::register_dvars()
	void gui::saved_windowstates()
	{
		// handles init and update
		HANDLE_SAVED_STATE(ggui::console_dialog, dvars::gui_saved_state_console, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE(ggui::entity_dialog, dvars::gui_saved_state_entity, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE(ggui::filter_dialog, dvars::gui_saved_state_filter, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE(ggui::modelselector_dialog, dvars::gui_saved_state_modelselector, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE(ggui::surface_dialog, dvars::gui_saved_state_surfinspector, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE(ggui::texture_dialog, dvars::gui_saved_state_textures, ggui::m_init_saved_states);

		ggui::m_init_saved_states = true;
	}

	// *
	void gui::begin_frame()
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	
	// *
	void gui::end_frame()
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	void on_map_load()
	{
		if (const auto con = GetConsoleWindow();
			!IsWindowVisible(con))
		{
			ShowWindow(con, SW_SHOW);
		}
	}

	__declspec(naked) void on_map_load_stub()
	{
		const static uint32_t func_addr = 0x485E50;
		const static uint32_t retn_addr = 0x4866BD;
		__asm
		{
			pushad;
			call	on_map_load;
			popad;

			call	func_addr; // og
			jmp		retn_addr;
		}
	}

	// *
	// register_addon_dvars()
	void gui::register_dvars()
	{
		dvars::gui_menubar_bg_color = dvars::register_vec4(
			/* name		*/ "gui_menubar_bg_color",
			/* x		*/ 0.11f,
			/* y		*/ 0.11f,
			/* z		*/ 0.11f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui menubar background color");

		dvars::gui_window_bg_color = dvars::register_vec4(
			/* name		*/ "gui_window_bg_color",
			/* x		*/ 0.172f,
			/* y		*/ 0.172f,
			/* z		*/ 0.172f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "backgroundcolor of gui windows (undocked)");
		
		dvars::gui_window_child_bg_color = dvars::register_vec4(
			/* name		*/ "gui_window_child_bg_color",
			/* x		*/ 0.17f,
			/* y		*/ 0.17f,
			/* z		*/ 0.17f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "backgroundcolor of gui child windows");

		dvars::gui_border_color = dvars::register_vec4(
			/* name		*/ "gui_border_color",
			/* x		*/ 1.0f,
			/* y		*/ 0.2f,
			/* z		*/ 0.2f,
			/* w		*/ 0.55f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "color of undocked gui windowborders");

		dvars::gui_dockingpreview_color = dvars::register_vec4(
			/* name		*/ "gui_dockingpreview_color",
			/* x		*/ 0.49f,
			/* y		*/ 0.2f,
			/* z		*/ 0.2f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "backgroundcolor of docking preview");
		
		// *
		// *
		
		dvars::gui_toolbar_bg_color = dvars::register_vec4(
			/* name		*/ "gui_toolbar_bg_color",
			/* x		*/ 0.11f,
			/* y		*/ 0.11f,
			/* z		*/ 0.11f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "gui toolbar background color");

		
		dvars::gui_toolbar_button_color = dvars::register_vec4(
			/* name		*/ "gui_toolbar_button_color",
			/* x		*/ 0.11f,
			/* y		*/ 0.11f,
			/* z		*/ 0.11f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "backgroundcolor of toolbar button");
		
		dvars::gui_toolbar_button_hovered_color = dvars::register_vec4(
			/* name		*/ "gui_toolbar_button_hovered_color",
			/* x		*/ 0.189f,
			/* y		*/ 0.189f,
			/* z		*/ 0.189f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "color of hovered toolbar button");

		dvars::gui_toolbar_button_active_color = dvars::register_vec4(
			/* name		*/ "gui_toolbar_button_active_color",
			/* x		*/ 1.0f,
			/* y		*/ 0.2f,
			/* z		*/ 0.148f,
			/* w		*/ 0.31f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "color of active toolbar button");

		// *
		// *
		
		dvars::gui_rtt_padding_enabled = dvars::register_bool(
			/* name		*/ "gui_rtt_padding_enabled",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "draw a border around render to texture windows (grid/camera)");

		dvars::gui_rtt_padding_size = dvars::register_int(
			/* name		*/ "gui_rtt_padding_size",
			/* default	*/ 4,
			/* mins		*/ 1,
			/* maxs		*/ 20,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bordersize of render to texture windows (grid/camera)");

		dvars::gui_rtt_padding_color = dvars::register_vec4(
			/* name		*/ "gui_rtt_padding_color",
			/* x		*/ 0.169f,
			/* y		*/ 0.169f,
			/* z		*/ 0.169f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bordercolor of render to texture windows (grid/camera)");

		// *
		// *

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

		dvars::gui_menubar_show_mouseorigin = dvars::register_bool(
			/* name		*/ "gui_menubar_show_mouseorigin",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "draw the mouse cursor origin within the menubar (old statusbar text)");

		dvars::gui_draw_fps = dvars::register_bool(
			/* name		*/ "gui_draw_fps",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "draw gui fps within the camera window");


		// *
		// gui::saved_windowstates()
		
		dvars::gui_saved_state_console = dvars::register_bool(
			/* name		*/ "gui_saved_state_console",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved opened/closed state of console window");

		dvars::gui_saved_state_filter = dvars::register_bool(
			/* name		*/ "gui_saved_state_filter",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved opened/closed state of filter window");

		dvars::gui_saved_state_entity = dvars::register_bool(
			/* name		*/ "gui_saved_state_entity",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved opened/closed state of entity window");

		dvars::gui_saved_state_textures = dvars::register_bool(
			/* name		*/ "gui_saved_state_textures",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved opened/closed state of texture window");

		dvars::gui_saved_state_modelselector = dvars::register_bool(
			/* name		*/ "gui_saved_state_modelselector",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved opened/closed state of modelselector window");

		dvars::gui_saved_state_surfinspector = dvars::register_bool(
			/* name		*/ "gui_saved_state_surfinspector",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved opened/closed state of surface inspector window");
	}

	
	// *
	gui::gui()
	{
		// show external console on map load
		utils::hook(0x4866B8, on_map_load_stub, HOOK_JUMP).install()->quick();

		GET_GUI(ggui::entity_dialog)->hooks();
		GET_GUI(ggui::filter_dialog)->hooks();
		GET_GUI(ggui::hotkey_dialog)->hooks();
		GET_GUI(ggui::preferences_dialog)->hooks();
		GET_GUI(ggui::surface_dialog)->hooks();
		GET_GUI(ggui::vertex_edit_dialog)->hooks();
		GET_GUI(ggui::modelselector_dialog)->init();

		command::register_command("demo"s, [](std::vector<std::string> args)
		{
			ggui::m_demo_menu_state = !ggui::m_demo_menu_state;
		});
	}

	gui::~gui()
	{ }
}