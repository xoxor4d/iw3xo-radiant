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