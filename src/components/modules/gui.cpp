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
	 */

	
	void cxywnd_gui(ggui::imgui_context_cxy& context)
	{
		ImGuiIO& io = ImGui::GetIO();

		int _stylevars = 0; int _stylecolors = 0;
		
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

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::ToImVec4(dvars::gui_menubar_bg_color->current.vector));		_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ToImVec4(dvars::gui_dockedwindow_bg_color->current.vector));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));							_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));					_stylecolors++;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("dockspace", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		
		// *
		// menubar
		
		ggui::menubar::menu(context);

		
		// *
		// create default docking layout

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("cxywnd_dockspace_layout");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode); // | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode);

			// init dockspace once
			if (!context.m_toolbar.one_time_init)
			{
				context.m_toolbar.one_time_init = true;

				// clear any previous layout
				ImGui::DockBuilderRemoveNode(dockspace_id);
				ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

				// create toolbar dockspace
				const auto dock_toolbar = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.01f, nullptr, &dockspace_id);
				ImGui::DockBuilderSetNodeSize(dock_toolbar, ImVec2(viewport->Size.x, 16)); // 36

				// ^ undockable toolbar without tabbar
				ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_toolbar);
				node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoDockingSplitMe | ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoResizeY;
				
				// split the resulting node (dockspace_id) -> dockspace_id will be everything to the right of the split
				const float split_ratio_left = viewport->Size.x < 1000.0f ? 0.5f : (1.0f / viewport->Size.x) * 500.0f;
				const auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, split_ratio_left, nullptr, &dockspace_id);
				
				// we now dock our windows into the docking node we made above
				ImGui::DockBuilderDockWindow("toolbar_xywnd", dock_toolbar);
				ImGui::DockBuilderDockWindow("Colors##xywnd", dock_id_left);
				ImGui::DockBuilderDockWindow("Hotkeys##xywnd", dock_id_left);
				
				ImGui::DockBuilderFinish(dockspace_id);
			}
		}

		ImGui::End();

		
		// *
		// toolbar
		
		ggui::toolbar::menu(context.m_toolbar);


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

			// create all dockable windows in cxywnd_gui
			cxywnd_gui(ggui::state.cxywnd);

			//color_menu(ggui::state.cxywnd.m_colors); // always open
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_colors, 
				ggui::colors::menu(ggui::state.cxywnd.m_colors), nullptr);

			// toggleable command bind menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_cmdbinds, 
				ggui::hotkeys::menu(ggui::state.cxywnd.m_cmdbinds), ggui::hotkeys::on_close());

			// toggleable command bind helper menu
			IMGUI_REGISTER_TOGGLEABLE_MENU(ggui::state.cxywnd.m_cmdbinds_helper,
				ggui::hotkeys::helper_menu(ggui::state.cxywnd.m_cmdbinds_helper), nullptr);
			
			// toggleable demo menu
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