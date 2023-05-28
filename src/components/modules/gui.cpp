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
		ImGui::MergeIconsWithLatestFont(12.0f, false);

		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_bold_compressed_data, fonts::opensans_bold_compressed_size, 17.0f);
		ImGui::MergeIconsWithLatestFont(12.0f, false);

		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 12.0f);
		io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 14.0f);

		ImFontConfig font_cfg;
		font_cfg.FontDataOwnedByAtlas = false;

		io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f, &font_cfg);

		// font awesome
		ImGui::MergeIconsWithLatestFont(18.0f, false);
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
		io.IniFilename = "IW3xRadiant\\imgui.ini";

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
					
					ggui::m_dockspace_outer_left_node = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Left, 0.45f, nullptr, &main_dock);
					const auto dockspace_toolbox_node = ImGui::DockBuilderSplitNode(main_dock, ImGuiDir_Right, 0.325f, nullptr, &main_dock);

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

					ImGui::DockBuilderDockWindow("Filters##window", dockspace_toolbox_node);
					ImGui::DockBuilderDockWindow("Toolbox##window", dockspace_toolbox_node);

					ImGui::DockBuilderDockWindow("Textures##rtt", main_dock);
					ImGui::DockBuilderDockWindow("Model Browser##rtt", main_dock);
					ImGui::DockBuilderDockWindow("Prefab Browser##window", main_dock);
					ImGui::DockBuilderDockWindow("Console##window", main_dock); 
					
					ImGui::DockBuilderFinish(dockspace_id);

					GET_GUI(ggui::texture_dialog)->open();
					GET_GUI(ggui::texture_dialog)->set_bring_to_front(true);

					GET_GUI(ggui::console_dialog)->open();
					GET_GUI(ggui::modelselector_dialog)->open();
					GET_GUI(ggui::prefab_preview_dialog)->open();
					GET_GUI(ggui::filter_dialog)->open();

					GET_GUI(ggui::toolbox_dialog)->open();
					GET_GUI(ggui::toolbox_dialog)->set_bring_to_front(true);
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
		if (dvars::r_reflectionprobe_generate->current.enabled)
		{
			return;
		}

		exec::on_gui_execute();

		/* - radiant draws multiple windows using d3d
		 *   => multiple endscene / present calls
		 * - use dx->targetWindowIndex to distinguish between windows
		 */

		 // *
		 // | -------------------- Camera Window ------------------------
		 // *

		if (game::dx->targetWindowIndex == renderer::CCAMERAWND)
		{
			// copy scene to texture
			// done in renderer::camera_postfx() if post effects are active

			if (!renderer::postfx::is_any_active())
			{
				renderer::copy_scene_to_texture(renderer::CCAMERAWND, GET_GUI(ggui::camera_dialog)->rtt_get_texture());
			}
		}

		
		// *
		// | -------------------- Grid Window ------------------------
		// *
		
		if (game::dx->targetWindowIndex == renderer::CXYWND)
		{
			renderer::copy_scene_to_texture(renderer::CXYWND, GET_GUI(ggui::grid_dialog)->rtt_get_texture());
		}

		
		// *
		// | -------------------- Texture Window ------------------------
		// *

		if (game::dx->targetWindowIndex == renderer::CTEXWND)
		{
			renderer::copy_scene_to_texture(renderer::CTEXWND, GET_GUI(ggui::texture_dialog)->rtt_get_texture());
		}


		// *
		// | ------------- Layered Materials Window (Model Preview) -----------------
		// *
		
		if (game::dx->targetWindowIndex == renderer::LAYERED)
		{
			// so empty
		}

		// *
		// | ------------- Fx Window (Effects Browser) -----------------
		// *

		if (game::dx->targetWindowIndex == renderer::CFXWND)
		{
			renderer::copy_scene_to_texture(renderer::CFXWND, GET_GUI(ggui::effects_browser)->rtt_get_texture());
		}

		
		// *
		// | --------------------- Z Window (Dear ImGui Canvas) ------------------------
		// *
		
		if (game::dx->targetWindowIndex == renderer::CZWND)
		{
			if (!ggui::m_ggui_initialized)
			{
				imgui_init_czwnd();

				// optional init function for each module
				for (const auto& module : ggui::loader::get_modules())
				{
					if (module && !module->is_gui_initiated())
					{
						module->on_init();
						module->set_gui_initiated();
					}
				}

				// load command map after gui init because gui's might add hotkeys
				ggui::hotkey_dialog::load_commandmap();

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

			// non-closable windows 
			GET_GUI(ggui::grid_dialog)->grid_gui();
			GET_GUI(ggui::camera_dialog)->camera_gui();

			if (ggui::m_demo_menu_state)
			{
				// demo menu
				ImGui::ShowDemoWindow(&ggui::m_demo_menu_state);
			}

			// draw/handle gui classes
			for (const auto& module : ggui::loader::get_modules())
			{
				if (module)
				{
					module->frame();
				}
			}

			// handle file dialogs
			ggui::file_dialog_frame();

			// hide external console
			utils::show_external_console(false);
			game::glob::is_loading_map = false;

#if 0
			if (game::s_world->reflectionProbes && game::s_world->reflectionProbes->reflectionImage
				&& game::s_world->reflectionProbes->reflectionImage->texture.data)
			{
				ImGui::Begin("Reflection Probes Debug", nullptr);

				ImGui::Image(game::s_world->reflectionProbes[0].reflectionImage->texture.data, ImVec2(300, 300));
				ImGui::SameLine();
				ImGui::Image(game::s_world->reflectionProbes[1].reflectionImage->texture.data, ImVec2(300, 300));


				if (reflectionprobes::imgui_cube_surfaces[0])
					ImGui::Image(reflectionprobes::imgui_cube_surfaces[0], ImVec2(300, 300));


				ImGui::SameLine();
				if (reflectionprobes::imgui_cube_surfaces[1])
					ImGui::Image(reflectionprobes::imgui_cube_surfaces[1], ImVec2(300, 300));


				ImGui::SameLine();
				if (reflectionprobes::imgui_cube_surfaces[2])
					ImGui::Image(reflectionprobes::imgui_cube_surfaces[2], ImVec2(300, 300));


				if (reflectionprobes::imgui_cube_surfaces[3])
					ImGui::Image(reflectionprobes::imgui_cube_surfaces[3], ImVec2(300, 300));

				ImGui::SameLine();
				if (reflectionprobes::imgui_cube_surfaces[4])
					ImGui::Image(reflectionprobes::imgui_cube_surfaces[4], ImVec2(300, 300));

				ImGui::SameLine();
				if (reflectionprobes::imgui_cube_surfaces[5])
					ImGui::Image(reflectionprobes::imgui_cube_surfaces[5], ImVec2(300, 300));

				ImGui::End();
			}
#endif

			// debug
			//game::GfxRenderTarget* targets = reinterpret_cast<game::GfxRenderTarget*>(0x174F4A8);
			//game::GfxRenderTarget* depth = &targets[game::R_RENDERTARGET_FLOAT_Z];

			/*ImGui::Begin("Debug", nullptr);
			ImGui::Image(postSun->image->texture.data, ImVec2(game::dx->windows[ggui::e_gfxwindow::CCAMERAWND].width, game::dx->windows[ggui::e_gfxwindow::CCAMERAWND].height));
			ImGui::End();*/

			/*if(depth && depth->image && depth->image->texture.data)
			{
				ImGui::Begin("Depthbuffer", nullptr);
				ImGui::Image(depth->image->texture.data, ImVec2(300, 300));
				ImGui::End();
			}*/

			/*ImGui::Begin("Test");
			{
				if (ImGui::Button("Snapshot Toast"))
				{
					ImGuiToast toast(ImGuiToastType_Info, 2500);
					toast.set_title("Snapshot created");
					toast.set_content(R"(D:\COD4Modtools\map_source\snapshots\mp_bsptest.map.5)");

					ImGui::InsertNotification(toast);
				}
			}
			ImGui::End();*/
			
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));

			ImGui::RenderNotifications();

			ImGui::PopStyleVar(2); 
			ImGui::PopStyleColor(1);

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

		ggui::m_ggui_second_frame = true;
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

#define HANDLE_SAVED_STATE_INT(_GUI_CLASS, _DVAR, _STARTUP)									\
		if(!(_STARTUP) && (_DVAR))															\
		{																					\
			const auto dialog = GET_GUI(_GUI_CLASS);										\
			dialog->toggle(true, (_DVAR)->current.integer);									\
			dialog->set_bring_to_front((_DVAR)->current.integer == 2);						\
		}																					\
		else if((_DVAR))																	\
		{																					\
			const auto dialog = GET_GUI(_GUI_CLASS);										\
			const bool open_and_front = dialog->is_active() && !dialog->is_inactive_tab();	\
			dvars::set_int((_DVAR), open_and_front ? 2 : dialog->is_active() ? 1 : 0);		\
		}

	// *
	// handles opened/closed states of windows via dvars
	// register dvars @ gui::register_dvars()
	void gui::saved_windowstates()
	{
		// handles init and update
		HANDLE_SAVED_STATE_INT(ggui::console_dialog, dvars::gui_saved_state_console, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::filter_dialog, dvars::gui_saved_state_filter, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::entity_dialog, dvars::gui_saved_state_entity, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::effects_browser, dvars::gui_saved_state_effects_browser, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::texture_dialog, dvars::gui_saved_state_textures, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::modelselector_dialog, dvars::gui_saved_state_modelselector, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::prefab_preview_dialog, dvars::gui_saved_state_prefab_browser, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::surface_dialog, dvars::gui_saved_state_surfinspector, ggui::m_init_saved_states);
		HANDLE_SAVED_STATE_INT(ggui::toolbox_dialog, dvars::gui_saved_state_toolbox, ggui::m_init_saved_states);

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
		game::glob::is_loading_map = true;
		GET_GUI(ggui::entity_dialog)->m_update_worldspawn = true;
		utils::show_external_console(true);

		components::effects::stop();
		components::physx_impl::get()->clear_dynamic_prefabs();
		components::physx_impl::get()->clear_static_collision();
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
	// *


	void map_autosave_toast(const char* file, int is_snapshot)
	{
		ImGuiToast toast(ImGuiToastType_Info, 2000);
		toast.set_title(is_snapshot ? "Snapshot created" : "Autosave created");
		toast.set_content(file);

		ImGui::InsertNotification(toast);
	}

	__declspec(naked) void map_snapshot_toast_stub()
	{
		const static uint32_t retn_addr = 0x48B391; // Map_SaveFile
		__asm
		{
			pushad;
			push	1;
			push	ebx;
			call	map_autosave_toast;
			add		esp, 8;
			popad;

			// org
			push    1; 
			push    0;
			mov     ecx, ebx;
			jmp		retn_addr;
		}
	}

	__declspec(naked) void map_autosave_toast_stub()
	{
		const char* map_str;

		const static uint32_t func_addr = 0x486C00; // Map_SaveFile
		const static uint32_t retn_addr = 0x48B5DE;
		__asm
		{
			mov		map_str, ecx;
			call	func_addr;
			add		esp, 8;

			pushad;
			push	0;
			push	map_str;
			call	map_autosave_toast;
			add		esp, 8;
			popad;

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
			/* x		*/ 0.173f,
			/* y		*/ 0.173f,
			/* z		*/ 0.173f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "backgroundcolor of gui windows (undocked)");
		
		dvars::gui_window_child_bg_color = dvars::register_vec4(
			/* name		*/ "gui_window_child_bg_color",
			/* x		*/ 0.173f,
			/* y		*/ 0.173f,
			/* z		*/ 0.173f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "backgroundcolor of gui child windows");

		dvars::gui_border_color = dvars::register_vec4(
			/* name		*/ "gui_border_color",
			/* x		*/ 0.0f,
			/* y		*/ 0.0f,
			/* z		*/ 0.0f,
			/* w		*/ 1.0f,
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

		dvars::gui_use_new_filedialog = dvars::register_bool(
			/* name		*/ "gui_use_new_filedialog",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "use new filedialogs, turn off to use windows file dialogs");

		dvars::gui_grid_context_show_select = dvars::register_bool(
			/* name		*/ "gui_grid_context_show_select",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "show select menu in grid context menu (when a brush is selected)");

		// *
		// gui::saved_windowstates()

		dvars::gui_saved_state_console = dvars::register_int(
			/* name		*/ "gui_saved_state_console",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_filter = dvars::register_int(
			/* name		*/ "gui_saved_state_filter",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_entity = dvars::register_int(
			/* name		*/ "gui_saved_state_entity",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_effects_browser = dvars::register_int(
			/* name		*/ "gui_saved_state_effects_browser",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_textures = dvars::register_int(
			/* name		*/ "gui_saved_state_textures",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_modelselector = dvars::register_int(
			/* name		*/ "gui_saved_state_modelselector",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_prefab_browser = dvars::register_int(
			/* name		*/ "gui_saved_state_prefab_browser",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_surfinspector = dvars::register_int(
			/* name		*/ "gui_saved_state_surfinspector",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_toolbox = dvars::register_int(
			/* name		*/ "gui_saved_state_toolbox",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");

		dvars::gui_saved_state_toolbox_child = dvars::register_int(
			/* name		*/ "gui_saved_state_toolbox_child",
			/* default	*/ 0,
			/* mins		*/ 0,
			/* maxs		*/ 5,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "saved closed/opened/active state of window");
	}

	
	// *
	gui::gui()
	{
		// show external console on map load
		utils::hook(0x4866B8, on_map_load_stub, HOOK_JUMP).install()->quick();

		// add snapshot toast
		utils::hook::nop(0x48B38B, 6); utils::hook(0x48B38B, map_snapshot_toast_stub, HOOK_JUMP).install()->quick();

		// add autosave toast
		utils::hook(0x48B5D6, map_autosave_toast_stub, HOOK_JUMP).install()->quick();

		ggui::entity_dialog::hooks();
		ggui::filter_dialog::hooks();
		ggui::grid_dialog::hooks();
		ggui::hotkey_dialog::hooks();
		ggui::layer_dialog::hooks();
		ggui::preferences_dialog::hooks();
		ggui::surface_dialog::hooks();
		ggui::vertex_edit_dialog::hooks();

		command::register_command("demo"s, [](std::vector<std::string> args)
		{
			ggui::m_demo_menu_state = !ggui::m_demo_menu_state;
		});
	}

	gui::~gui()
	{ }
}