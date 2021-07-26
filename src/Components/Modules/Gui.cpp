#include "STDInclude.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define mainframe_thiscall(return_val, addr)	\
	Utils::Hook::Call<return_val(__fastcall)(CMainFrame*)>(addr)(CMainFrame::ActiveWindow)


//Utils::Hook::Call<LRESULT(__fastcall)(CMainFrame*)>(0x428BC0)(CMainFrame::ActiveWindow);


#define IMGUI_REGISTERMENU(menu, function)			\
    if (menu.menustate) {							\
		function;									\
		menu.was_open = true;						\
	}												\
	else if(menu.was_open) {						\
		Gui::save_settings();						\
		menu.was_open = false;						\
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

// execute a single command
#define CMDEXEC(command) Game::Cmd_ExecuteSingleCommand(0, 0, command)

namespace Components
{
	/*bool imgui_camera_init = false;
	bool imgui_cxy_init = false;
	
	ImGuiContext* _context_camera;
	ImGuiContext* _context_cxy;*/
	
	// *
	// initialize imgui
	void Gui::imgui_init()
	{
		// get the device from D3D9Ex::_D3D9/Ex::CreateDevice
		IDirect3DDevice9* device = Game::Globals::d3d9_device;

		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");
		ASSERT_MSG(CCamWnd::ActiveWindow, "CCamWnd::ActiveWindow == nullptr");

		Game::Globals::_context_camera = ImGui::CreateContext();
		ImGui::SetCurrentContext(Game::Globals::_context_camera);
		
        ImGuiIO& io = ImGui::GetIO();

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// init regular font (see Fonts.cpp)
        io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

		auto cam = CCamWnd::ActiveWindow;

		ImGui_ImplWin32_Init(cam->GetWindow());
		//ImGui_ImplWin32_Init(CMainFrame::ActiveWindow->GetWindow()); //cam->GetWindow() // CMainFrame::ActiveWindow->m_hWnd
		ImGui_ImplDX9_Init(device);

		// Style
		ImGui::StyleColorsDevgui();
		GGUI_READY = true;
		Game::Globals::imgui_camera_init = true;
	}

	void imgui_init_cxy()
	{
		// get the device from D3D9Ex::_D3D9/Ex::CreateDevice
		IDirect3DDevice9* device = Game::Globals::d3d9_device;

		ASSERT_MSG(device, "IDirect3DDevice9 == nullptr");

		if(!CMainFrame::ActiveWindow->m_pXYWnd)
		{
			return;
		}

		Game::Globals::_context_cxy = ImGui::CreateContext();
		ImGui::SetCurrentContext(Game::Globals::_context_cxy);

		ImGuiIO& io = ImGui::GetIO();

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// init regular font (see Fonts.cpp)
		io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

		auto cam = CCamWnd::ActiveWindow;

		//ImGui_ImplWin32_Init(cam->GetWindow());
		ImGui_ImplWin32_Init(CMainFrame::ActiveWindow->m_pXYWnd->GetWindow()); //cam->GetWindow() // CMainFrame::ActiveWindow->m_hWnd
		ImGui_ImplDX9_Init(device);

		// Style
		ImGui::StyleColorsDevgui();
		GGUI_READY = true;
		Game::Globals::imgui_cxy_init = true;
	}

	/*
	 * auto vtable = reinterpret_cast<CStatusBar_vtbl*>(CMainFrame::ActiveWindow->m_wndStatusBar.__vftable);
	 * reinterpret_cast<CStatusBar_vtbl*>(CMainFrame::ActiveWindow->m_wndStatusBar.__vftable)->SetStatusText(&CMainFrame::ActiveWindow->m_wndStatusBar, 0x75);
	 *
	 * auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(CMainFrame::ActiveWindow->m_wndSplit.__vftable);
	 * vtable->RecalcLayout(&CMainFrame::ActiveWindow->m_wndSplit);
	 * 
	 */

	void testgui(Game::gui_menus_t& menu)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiIO& io = ImGui::GetIO();
		
		// *
		// create dockspace
		
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImVec2 vp_size = ImVec2(CMainFrame::ActiveWindow->m_pCamWnd->camera.width, CMainFrame::ActiveWindow->m_pCamWnd->camera.height);

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size); //ImGui::SetNextWindowSize(vp_size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowBgAlpha(0.0f);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking; // ImGuiWindowFlags_MenuBar
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("dockspace_camera", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspace_id = ImGui::GetID("dockspace_camera");
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar; ImGuiDockNodeFlags_HiddenTabBar;
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		ImGui::End();

		// ----------------------------

		//style.Colors[ImGuiCol_WindowBg].w = 0.8f;
		style.WindowMinSize.x = 400.0f;
		style.WindowMinSize.y = 40.0f;


		
		// *
		// styles

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 4.0f));

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(20, 20, 20, 90));
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(100, 100, 100, 70));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));

		//ImGui::SetNextWindowPos(viewport->Pos);
		
		ImGui::Begin("test_gui", nullptr, 
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar 
			//| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar
		);

		// *
		// sizes
		const auto parentwnd_pos = ImGui::GetMainViewport()->Pos;
		menu.position[0] = parentwnd_pos.x; //0.0f;
		menu.position[1] = parentwnd_pos.y; //0.0f;
		menu.size[0] = static_cast<float>(CMainFrame::ActiveWindow->m_pCamWnd->camera.width); //ImGui::GetWindowWidth();
		menu.size[1] = 40.0f;

		ImGui::SetWindowPos(ImVec2(menu.position[0], menu.position[1]), ImGuiCond_FirstUseEver);
		ImGui::SetWindowSize(ImVec2(menu.size[0], menu.size[1]), ImGuiCond_FirstUseEver);
		
		// *
		// gui elements

		// lock x
		if (const auto image = Game::Image_RegisterHandle("lock_x"); image)
		{
			ImVec2 uv0 = (Game::g_nScaleHow == 2 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 6) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (Game::g_nScaleHow == 2 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 6) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, ImVec2(32.0f, 32.0f), uv0, uv1, 0)) 
			{
				// lock x
				mainframe_thiscall(LRESULT, 0x428BC0);
			} TT("lock grid along the x-axis");
		}

		// lock y
		if (const auto image = Game::Image_RegisterHandle("lock_y"); image)
		{
			ImVec2 uv0 = (Game::g_nScaleHow == 1 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 5) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (Game::g_nScaleHow == 1 || Game::g_nScaleHow == 4 || Game::g_nScaleHow == 5) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, ImVec2(32.0f, 32.0f), uv0, uv1, 0))
			{
				// lock x
				mainframe_thiscall(LRESULT, 0x428B60);
			} TT("lock grid along the y-axis");
		}

		// lock z
		if (const auto image = Game::Image_RegisterHandle("lock_z"); image)
		{
			ImVec2 uv0 = (Game::g_nScaleHow > 0 && Game::g_nScaleHow <= 3) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (Game::g_nScaleHow > 0 && Game::g_nScaleHow <= 3) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, ImVec2(32.0f, 32.0f), uv0, uv1, 0))
			{
				// lock x
				mainframe_thiscall(LRESULT, 0x428B90);
			} TT("lock grid along the z-axis");
		}

		
		ImGui::SameLine();
		if (ImGui::Button("Lock X")) { mainframe_thiscall(LRESULT, 0x428BC0); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Y")) { mainframe_thiscall(LRESULT, 0x428B60); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Z")) { mainframe_thiscall(LRESULT, 0x428B90); }

		
		ImGui::SameLine();
		if(ImGui::Button("Switch Console Splitter"))
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

		ImGui::SameLine();
		if (ImGui::Button("Set Statustext"))
		{
			const auto vtable = reinterpret_cast<CStatusBar_vtbl*>(CMainFrame::ActiveWindow->m_wndStatusBar.__vftable);
			vtable->SetStatusText(&CMainFrame::ActiveWindow->m_wndStatusBar, 0x75);
		}

		ImGui::SameLine();
		if(ImGui::Button("show cam"))
		{
			//ShowWindow(Game::Globals::test_hwnd, SW_SHOW);
		}

	END_GUI:
		ImGui::PopStyleColor(5);
		ImGui::PopStyleVar(1);
		ImGui::End();
	}


	/*
	 *
	 *
	 * ImGui::BeginDockableDragDropSource 
	 * edit is_drag_docking to allow docking from everywhere
	 * 
	 */
	

	// *
	// main rendering loop (D3D9Ex::D3D9Device::EndScene())
	void Gui::render_loop()
	{
		// *
		// menus

		Gui::any_open_menus();

		//Game::dx->targetWindowIndex
		
		// radiant draws multiple windows using d3d => multiple EndScene calls
		// * hook 'R_EndFrame' within 'Wnd::OnPaint' and enable the gui_present.Wnd bool
		// * check if we should draw imgui for the current scene
		if(Game::dx->targetWindowIndex == 0)// || Game::Globals::gui_present.CCamWnd)
		{
			//if (!GET_GGUI.imgui_initialized) {
			if(!Game::Globals::imgui_camera_init) {
				Gui::imgui_init();
			}

			ImGui::SetCurrentContext(Game::Globals::_context_camera);
			
			Gui::begin_frame();
			
			testgui(GET_GGUI.menus[Game::GUI_MENUS::DEVGUI]);
			GET_GGUI.menus[Game::GUI_MENUS::DEVGUI].menustate = true;

			//IMGUI_REGISTERMENU(GET_GGUI.menus[Game::GUI_MENUS::DEMO], ImGui::ShowDemoWindow(&GET_GGUI.menus[Game::GUI_MENUS::DEMO].menustate));
			
			//ImGui::ShowDemoWindow(nullptr);
			//GET_GGUI.menus[Game::GUI_MENUS::DEMO].menustate = true;
			
			// reset so we don't draw on the next scene
			//Game::Globals::gui_present.CCamWnd = false;
			goto END_FRAME;
		}

		if (Game::dx->targetWindowIndex == 1) //if (Game::Globals::gui_present.CXYWnd)
		{
			//if (!GET_GGUI.imgui_initialized) {
			if (!Game::Globals::imgui_cxy_init) {
				imgui_init_cxy();
			}

			ImGui::SetCurrentContext(Game::Globals::_context_cxy);
			
			Gui::begin_frame();

			testgui(GET_GGUI.menus[Game::GUI_MENUS::DEVGUI]);
			GET_GGUI.menus[Game::GUI_MENUS::DEVGUI].menustate = true;

			IMGUI_REGISTERMENU(GET_GGUI.menus[Game::GUI_MENUS::DEMO], ImGui::ShowDemoWindow(&GET_GGUI.menus[Game::GUI_MENUS::DEMO].menustate));

			//ImGui::ShowDemoWindow(nullptr);
			//GET_GGUI.menus[Game::GUI_MENUS::DEMO].menustate = true;
			
			// reset so we don't draw on the next scene
			//Game::Globals::gui_present.CXYWnd = false;
			goto END_FRAME;
		}

		return;

		// ------------

		//IMGUI_REGISTERMENU(gui.menus[Game::GUI_MENUS::DEMO], ImGui::ShowDemoWindow(&gui.menus[Game::GUI_MENUS::DEMO].menustate));
		//testgui(gui.menus[Game::GUI_MENUS::DEVGUI]);
		//IMGUI_REGISTERMENU(gui.menus[Game::GUI_MENUS::CHANGELOG], _UI::create_changelog(gui.menus[Game::GUI_MENUS::CHANGELOG]));

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
	void Gui::reset()
	{
		if (GGUI_READY)
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();

			memset(&Game::Globals::gui, 0, sizeof(Game::gui_t));
			GGUI_READY_DVARS = true;
		}
	}

	// *
	// toggle a imgui menu by command (or key (scheduler))
	void Gui::toggle(Game::gui_menus_t &menu, [[maybe_unused]] int keycatcher, bool onCommand = false)
	{
		if (!GGUI_READY)
		{
			return;
		}

		// toggle menu by key or command
		if (menu.hk_is_clicked || onCommand)
		{
			ImGuiIO& io = ImGui::GetIO();
			std::fill_n(io.KeysDown, 512, 0);

			menu.menustate = !menu.menustate;

			// on close
			if (!menu.menustate)
			{
				// check if there is still some open menu
				if (GGUI_ANY_MENUS_OPEN)
				{
					// do nothing with the mouse
					return;
				}
			}
		}
	}

	// *
	// set menu layout (origin / size / anker)
	void Gui::set_menu_layout(Game::gui_menus_t& menu, const float x, const float y, const float width, const float height, const int horzAlign = 0, const int vertAlign = 0)
	{
		menu.position[0] = x;
		menu.position[1] = y;

		menu.size[0] = width;
		menu.size[1] = height;

		menu.horzAlign = horzAlign;
		menu.vertAlign = vertAlign;
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
	// called on init from Commands::ForceDvarsOnInit()
	void Gui::load_settings()
	{
		// pretty useless but might be needed later
		GGUI_READY_DVARS = true;
	}

	// *
	// save dvars to disk (only modified dvars are saved)
	void Gui::save_settings()
	{
	}

	// *
	// check if there are any open menus
	bool Gui::any_open_menus()
	{
		auto& gui = GET_GGUI;
		for (int m = 0; m < GGUI_MENU_COUNT; m++)
		{
			if (gui.menus[m].menustate)
			{
				if (gui.menus[m].mouse_ignores_menustate)
				{
					gui.any_menus_open = false;
					return false;
				}
				else
				{
					gui.any_menus_open = true;
					return true;
				}
			}
		}

		// negative flag
		if (gui.any_menus_open)
		{
			gui.any_menus_open = false;
		}

		
		return false;
	}

	// *
	// not using a macro so one can see the structure while debugging
	Game::gui_menus_t& Gui::GetMenu(Game::GUI_MENUS id)
	{
		return Game::Globals::gui.menus[id];
	}

	void Gui::register_dvars()
	{
		Dvars::imgui_devgui_pos_x = Dvars::Register_AddonFloat(
			/* name		*/ "imgui_devgui_pos_x",
			/* value	*/ 20.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::saved,
			/* desc		*/ "X position of the devgui");

		Dvars::imgui_devgui_pos_y = Dvars::Register_AddonFloat(
			/* name		*/ "imgui_devgui_pos_y",
			/* value	*/ 20.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::saved,
			/* desc		*/ "Y position of the devgui");


		Dvars::imgui_devgui_size_x = Dvars::Register_AddonFloat(
			/* name		*/ "imgui_devgui_size_x",
			/* value	*/ 550.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::saved,
			/* desc		*/ "width of the devgui");

		Dvars::imgui_devgui_size_y = Dvars::Register_AddonFloat(
			/* name		*/ "imgui_devgui_size_y",
			/* value	*/ 680.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 4096.0f,
			/* flags	*/ Game::dvar_flags::saved,
			/* desc		*/ "width of the devgui");
	}

	typedef HRESULT(__stdcall* SwapChainPresent_t)(Game::IDirect3DSwapChain9* , const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags);
	SwapChainPresent_t __oSwapChainPresent;

	HRESULT __stdcall Present(Game::IDirect3DSwapChain9* pThis, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags)
	{
		int x = 1;

		Gui::render_loop();
		
		return __oSwapChainPresent(pThis, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	}

	void _imgui_init()
	{
		if (!GET_GGUI.imgui_initialized) {
			Gui::imgui_init();
		}
	}
	
	void __declspec(naked) imgui_init_stub()
	{
		const static uint32_t og_func = 0x456930;
		const static uint32_t retn_pt = 0x48BCF7;
		__asm
		{
			pushad;
			call	_imgui_init;
			popad;
			
			call	og_func;
			jmp		retn_pt;
		}
	}
	
	// *
	// 
	Gui::Gui()
	{
		//Utils::Hook(0x48BCF2, imgui_init_stub, HOOK_CALL).install()->quick();

		// *
		// Commands
		Command::RegisterCommand("gui"s, [](std::vector < std::string > args)
		{
				Gui::toggle(GET_GGUI.menus[Game::GUI_MENUS::DEVGUI], 0, true);
		});
		
		Command::RegisterCommand("gui_demo"s, [](std::vector < std::string > args)
		{
			Gui::toggle(GET_GGUI.menus[Game::GUI_MENUS::DEMO], 0, true);
		});

		Command::RegisterCommand("dx"s, [](std::vector<std::string> args) ->void
		{
			auto dx = Game::dx;
			const uintptr_t _present_ptr = (uintptr_t&)dx->windows[0].swapChain->lpVtbl->Present;
			__oSwapChainPresent = reinterpret_cast<SwapChainPresent_t>(Utils::Hook::Detour(_present_ptr, Present, HK_JUMP));
			
			int x = 1;
		});
	}

	Gui::~Gui()
	{ }
}