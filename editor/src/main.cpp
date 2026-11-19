#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "codol/packed_scripts.hpp"
#include "ui.hpp"
#include "assets.hpp"
#include "preview.hpp"
#include "ext.hpp"
#include "png_wic.hpp"

// CoDOL Radiant: the standalone host. A plain Win32 window with a Direct3D 9 device and a docking ImGui on it.
// No CoD4 radiant, no hooks: everything shown comes from the 1.1.1.13 install through codol_zone / codol_render /
// codol_view, and from the editor's own code.

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace editor
{
	namespace
	{
		HWND g_hwnd = nullptr;
		IDirect3D9* g_d3d = nullptr;
		IDirect3DDevice9* g_device = nullptr;
		D3DPRESENT_PARAMETERS g_pp = {};
		bool g_reset_pending = false;
		bool g_device_lost = false;
		bool g_exit = false;
		bool g_close_requested = false;
		bool g_minimized = false;
		std::string g_title;

		struct log_line
		{
			bool error;
			std::string text;
		};

		std::mutex g_log_mutex;
		std::vector<log_line> g_log;
		bool g_log_scroll = false;

		bool g_show_console = true;
		bool g_show_demo = false;
		bool g_show_about = false;
		bool g_show_settings = false;
		std::string g_console_input;
		std::vector<std::filesystem::path> g_dropped;		// WM_DROPFILES, handled in the next frame
		std::string g_shot_path;							// ed_shot: the next frame is saved there

		// ---------------------------------------------------------------------------------------------

		void reset_device()
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			ggui::codol_viewport_dialog::on_device_lost();

			const HRESULT hr = g_device->Reset(&g_pp);
			if (FAILED(hr))
			{
				g_device_lost = true;
				return;
			}

			g_device_lost = false;
			ggui::codol_viewport_dialog::on_device_reset();
			ImGui_ImplDX9_CreateDeviceObjects();
		}

		bool create_device()
		{
			g_d3d = Direct3DCreate9(D3D_SDK_VERSION);
			if (!g_d3d)
			{
				return false;
			}

			g_pp = {};
			g_pp.Windowed = TRUE;
			g_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			g_pp.BackBufferFormat = D3DFMT_X8R8G8B8;
			g_pp.EnableAutoDepthStencil = TRUE;
			g_pp.AutoDepthStencilFormat = D3DFMT_D24S8;
			g_pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			g_pp.hDeviceWindow = g_hwnd;

			// the game's shaders are vs_3_0 / ps_3_0: hardware vertex processing is required
			const DWORD flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED;

			if (FAILED(g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hwnd, flags, &g_pp, &g_device)))
			{
				return false;
			}

			game::glob::d3d9_device = g_device;
			return true;
		}

		LRESULT WINAPI wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam)
		{
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			{
				return 1;
			}

			switch (msg)
			{
			case WM_SIZE:
				g_minimized = wparam == SIZE_MINIMIZED;

				if (g_device && wparam != SIZE_MINIMIZED)
				{
					const UINT w = LOWORD(lparam), h = HIWORD(lparam);
					if (w > 0 && h > 0 && (w != g_pp.BackBufferWidth || h != g_pp.BackBufferHeight))
					{
						g_pp.BackBufferWidth = w;
						g_pp.BackBufferHeight = h;
						g_reset_pending = true;
					}
				}

				return 0;

			case WM_SYSCOMMAND:
				// no ALT application menu: ALT is a modifier in the views
				if ((wparam & 0xfff0) == SC_KEYMENU)
				{
					return 0;
				}

				break;

			case WM_DROPFILES:
			{
				// files dropped on the window: images make a material, exports a model ( assets.hpp )
				const auto drop = reinterpret_cast<HDROP>(wparam);
				const UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);

				for (UINT i = 0; i < count; i++)
				{
					wchar_t file[MAX_PATH * 4] = {};
					if (DragQueryFileW(drop, i, file, static_cast<UINT>(std::size(file))))
					{
						g_dropped.emplace_back(file);
					}
				}

				DragFinish(drop);
				return 0;
			}

			case WM_CLOSE:
				// asked in the next frame: a map with unsaved changes gets a question first
				g_close_requested = true;
				return 0;

			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;

			default:
				break;
			}

			return DefWindowProcW(hwnd, msg, wparam, lparam);
		}

		// ---------------------------------------------------------------------------------------------

		void init_fonts()
		{
			using namespace components;
			ImGuiIO& io = ImGui::GetIO();

			// the same set, in the same order, as the radiant plugin ( the shared code picks fonts by index )
			io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_bold_compressed_data, fonts::opensans_bold_compressed_size, 18.0f);
			io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_bold_compressed_data, fonts::opensans_bold_compressed_size, 17.0f);
			io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 12.0f);
			io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 14.0f);
			io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(fonts::opensans_regular_compressed_data, fonts::opensans_regular_compressed_size, 18.0f);

			// monospace font of the script workbench ( index 5 ), with the glyphs of the game's GBK scripts
			if (!GetEnvironmentVariableA("CODOL_NO_MONO_FONT", nullptr, 0))
			{
				char windir[MAX_PATH] = {};
				if (GetWindowsDirectoryA(windir, sizeof(windir)))
				{
					for (const char* name : { "\\Fonts\\consola.ttf", "\\Fonts\\cour.ttf", "\\Fonts\\lucon.ttf" })
					{
						const std::string path = std::string(windir) + name;
						if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES)
						{
							io.Fonts->AddFontFromFileTTF(path.c_str(), 15.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
							break;
						}
					}
				}
			}
		}

		void init_style()
		{
			ImGui::StyleColorsDark();

			auto& style = ImGui::GetStyle();
			style.WindowRounding = 2.0f;
			style.FrameRounding = 2.0f;
			style.GrabRounding = 2.0f;
			style.TabRounding = 2.0f;
			style.PopupRounding = 2.0f;
			style.ScrollbarRounding = 2.0f;
			style.WindowBorderSize = 1.0f;
			style.FramePadding = ImVec2(8.0f, 4.0f);
			style.ItemSpacing = ImVec2(8.0f, 6.0f);
			style.WindowMenuButtonPosition = ImGuiDir_None;

			auto* c = style.Colors;
			c[ImGuiCol_WindowBg] = ImVec4(0.115f, 0.118f, 0.125f, 1.0f);
			c[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			c[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.145f, 0.155f, 0.98f);
			c[ImGuiCol_Border] = ImVec4(0.24f, 0.25f, 0.27f, 1.0f);
			c[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.092f, 0.098f, 1.0f);
			c[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.185f, 0.20f, 1.0f);
			c[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.25f, 0.27f, 1.0f);
			c[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.30f, 0.33f, 1.0f);
			c[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.092f, 0.098f, 1.0f);
			c[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.135f, 0.145f, 1.0f);
			c[ImGuiCol_Tab] = ImVec4(0.13f, 0.135f, 0.145f, 1.0f);
			c[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.42f, 0.36f, 1.0f);
			c[ImGuiCol_TabActive] = ImVec4(0.22f, 0.33f, 0.28f, 1.0f);
			c[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.115f, 0.12f, 1.0f);
			c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.22f, 0.20f, 1.0f);
			c[ImGuiCol_Header] = ImVec4(0.22f, 0.33f, 0.28f, 1.0f);
			c[ImGuiCol_HeaderHovered] = ImVec4(0.27f, 0.40f, 0.34f, 1.0f);
			c[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.45f, 0.38f, 1.0f);
			c[ImGuiCol_Button] = ImVec4(0.20f, 0.21f, 0.23f, 1.0f);
			c[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.40f, 0.34f, 1.0f);
			c[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.45f, 0.38f, 1.0f);
			c[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.80f, 0.62f, 1.0f);
			c[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.66f, 0.52f, 1.0f);
			c[ImGuiCol_SliderGrabActive] = ImVec4(0.48f, 0.80f, 0.62f, 1.0f);
			c[ImGuiCol_Separator] = ImVec4(0.24f, 0.25f, 0.27f, 1.0f);
			c[ImGuiCol_DockingPreview] = ImVec4(0.40f, 0.66f, 0.52f, 0.6f);
			c[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.082f, 0.086f, 1.0f);
			c[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.50f, 0.42f, 0.6f);
		}

		// ---------------------------------------------------------------------------------------------

		void draw_console()
		{
			if (!g_show_console)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(900.0f, 240.0f), ImGuiCond_FirstUseEver);

			if (ImGui::Begin("Console##window", &g_show_console, ImGuiWindowFlags_NoCollapse))
			{
				const float footer = ImGui::GetFrameHeightWithSpacing();

				if (ImGui::BeginChild("##console_lines", ImVec2(0.0f, -footer), false, ImGuiWindowFlags_HorizontalScrollbar))
				{
					std::lock_guard lock(g_log_mutex);

					ImGuiListClipper clipper;
					clipper.Begin(static_cast<int>(g_log.size()));

					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						{
							const auto& line = g_log[static_cast<std::size_t>(i)];

							if (line.error)
							{
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.45f, 0.40f, 1.0f));
							}

							ImGui::TextUnformatted(line.text.c_str());

							if (line.error)
							{
								ImGui::PopStyleColor();
							}
						}
					}

					if (g_log_scroll)
					{
						ImGui::SetScrollHereY(1.0f);
						g_log_scroll = false;
					}
				}

				ImGui::EndChild();

				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::InputTextWithHint("##console_input", "command, eg. codol_viewport mp_rust", &g_console_input, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!g_console_input.empty())
					{
						log("> " + g_console_input);
						command::execute(g_console_input);
						g_console_input.clear();
					}

					ImGui::SetKeyboardFocusHere(-1);
				}
			}

			ImGui::End();
		}

		void draw_menu_contents();

		void draw_menu()
		{
			// actions_check: every menu once, off the screen, as if it were open
			if (menu::audit_requested())
			{
				menu::audit_begin();
				draw_menu_contents();
				menu::audit_end();
			}

			if (!ImGui::BeginMainMenuBar())
			{
				return;
			}

			draw_menu_contents();
			ImGui::EndMainMenuBar();
		}

		void draw_menu_contents()
		{
			auto& st = ui::get();

			if (editor::menu::begin("File"))
			{
				ui::draw_file_menu_items();
				ui::draw_file_menu_extra();
				ImGui::Separator();
				prefs::draw_recent_menu();
				editor::menu::item("Settings ..", nullptr, &g_show_settings);
				ImGui::Separator();

				if (editor::menu::item("Exit"))
				{
					ui::exit_editor();
				}

				editor::menu::end();
			}

			if (editor::menu::begin("Edit"))
			{
				ui::draw_edit_menu();
				ImGui::Separator();
				ext::draw_edit_menu_items();		// Edit Colors / Edit Toolbar / Edit Hotkeys / Preferences
				ui::draw_edit_menu_extra();
				editor::menu::end();
			}

			if (editor::menu::begin("Brush"))
			{
				ui::draw_brush_menu();
				editor::menu::end();
			}

			if (editor::menu::begin("Patch"))
			{
				ui::draw_patch_menu();
				editor::menu::end();
			}

			if (editor::menu::begin("View"))
			{
				const auto item = [](const char* label, ggui::ggui_module* module_)
				{
					bool open = module_->is_active();
					if (editor::menu::item(label, nullptr, &open))
					{
						module_->toggle(true, open);
					}
				};

				item("3D View", GET_GUI(ggui::codol_viewport_dialog));
				ui::draw_grid_view_menu_items();
				ImGui::Separator();
				editor::menu::item("Entity", nullptr, &st.show_entity);
				editor::menu::item("Selection", nullptr, &st.show_selection);
				editor::menu::item("Layers", nullptr, &st.show_layers);
				editor::menu::item("Models", nullptr, &st.show_models);
				editor::menu::item("Materials", editor::menu::key("window.materials"), &st.show_materials);
				editor::menu::item("Assets", nullptr, &st.show_assets);
				editor::menu::item("Preview", nullptr, &st.show_preview);
				editor::menu::item("Surface inspector", editor::menu::key("view.surface"), &st.show_surface);
				ImGui::Separator();
				item("Maps", GET_GUI(ggui::codol_map_ents_dialog));
				item("Script Workbench", GET_GUI(ggui::script_workbench_dialog));
				item("Game", GET_GUI(ggui::game_view_dialog));
				editor::menu::item("Console", nullptr, &g_show_console);
				ui::draw_view_menu_extra();
				editor::menu::end();
			}

			// Selection, Grid, Misc, Region
			ext::draw_menus();		// Textures, Renderer, Effects, Tools
			ui::draw_extra_menus();

			if (editor::menu::begin("Game"))
			{
				if (editor::menu::item("Reload scripts in the game"))
				{
					components::codol_game::reload_scripts("menu");
				}

				if (editor::menu::item("Game window"))
				{
					GET_GUI(ggui::game_view_dialog)->open();
				}

				editor::menu::end();
			}

			if (editor::menu::begin("Help"))
			{
				ext::draw_help_menu_items();
				ImGui::Separator();
				editor::menu::item("About", nullptr, &g_show_about);
				editor::menu::item("ImGui demo", nullptr, &g_show_demo);
				editor::menu::end();
			}
		}

		void draw_about()
		{
			if (!g_show_about)
			{
				return;
			}

			ImGui::SetNextWindowSize(ImVec2(520.0f, 0.0f), ImGuiCond_Appearing);

			if (ImGui::Begin("About##window", &g_show_about, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
			{
				ImGui::TextUnformatted("CoDOL Radiant");
				ImGui::Separator();
				ImGui::PushTextWrapPos(0.0f);
				ImGui::TextUnformatted(
					"A map and script editor for Call of Duty Online 1.1.1.13.\n\n"
					"Standalone: maps, models, materials and shaders are read from the game's own fastfiles and drawn with the "
					"game's own shaders. No CoD4 radiant, no mod tools, no converted assets.\n\n"
					"Started as a fork of iw3xo-radiant ( xoxor4d ).");
				ImGui::PopTextWrapPos();
			}

			ImGui::End();
		}

		void first_layout(const ImGuiID dockspace)
		{
			// windows that have never been placed are put into the main dock space
			static bool done = false;
			if (done)
			{
				return;
			}

			done = true;

			if (ImGui::DockBuilderGetNode(dockspace) && ImGui::DockBuilderGetNode(dockspace)->IsSplitNode())
			{
				return; // a saved layout exists
			}

			ImGui::DockBuilderRemoveNode(dockspace);
			ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace, ImGui::GetMainViewport()->WorkSize);

			// the classic arrangement: the 3D view next to a 2D view, lists on the left, what is selected on the right
			ImGuiID center = dockspace;
			const ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.2f, nullptr, &center);
			const ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.16f, nullptr, &center);
			ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.22f, nullptr, &center);
			const ImGuiID right_bottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.42f, nullptr, &right);
			const ImGuiID grid = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.45f, nullptr, &center);

			ImGui::DockBuilderDockWindow("Script Workbench##codol_window", center);
			ImGui::DockBuilderDockWindow("Game##codol_window", center);
			ImGui::DockBuilderDockWindow("CoDOL Viewport##window", center);
			ImGui::DockBuilderDockWindow("2D View##grid0", grid);
			ImGui::DockBuilderDockWindow("2D View 2##grid1", grid);
			ImGui::DockBuilderDockWindow("2D View 3##grid2", grid);
			ImGui::DockBuilderDockWindow("Models##window", left);
			ImGui::DockBuilderDockWindow("Materials##window", left);
			ImGui::DockBuilderDockWindow("Assets##window", left);
			ImGui::DockBuilderDockWindow("Layers##window", left);
			ImGui::DockBuilderDockWindow("Maps##window", left);
			ImGui::DockBuilderDockWindow("Entity##window", right);
			ImGui::DockBuilderDockWindow("Selection##window", right_bottom);
			ImGui::DockBuilderDockWindow("Surface##window", right_bottom);
			ImGui::DockBuilderDockWindow("Console##window", bottom);
			ImGui::DockBuilderFinish(dockspace);
		}

		void frame()
		{
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ui::apply_injected_input();
			ImGui::NewFrame();

			const auto frame_start = std::chrono::steady_clock::now();

			if (g_close_requested)
			{
				g_close_requested = false;
				ui::exit_editor();
			}

			ui::process_shortcuts();
			draw_menu();
			toolbar::draw();		// under the menu bar, before the dock space takes the rest

			const ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			first_layout(dockspace);

			components::codol_game::frame();
			codol::packed_scripts::ensure_defaults();		// a changed install folder

			// the renderer follows the document, before any view draws
			get_scene().sync();

			for (const auto& module_ : ggui::loader::get_modules())
			{
				module_->frame();
			}

			if (!g_dropped.empty())
			{
				const auto files = std::move(g_dropped);
				g_dropped.clear();
				assets::files_dropped(files);
			}

			assets::frame();
			preview::frame();

			ui::draw_grid_views();
			ui::draw_entity_window();
			ui::draw_layers_window();
			ui::draw_surface_window();
			ui::draw_create_entity_popup();
			ui::draw_modals();
			ui::draw_install_window();
			ui::draw_brush_patch_windows();
			ui::draw_extra_windows();

			// map name and the modified mark in the title bar
			if (std::string title = get_scene().title(); title != g_title)
			{
				g_title = std::move(title);
				const std::string text = g_title.empty() ? "CoDOL Radiant - Call of Duty Online 1.1.1.13" : g_title + " - CoDOL Radiant";
				SetWindowTextW(g_hwnd, std::filesystem::u8path(text).wstring().c_str());
			}

			draw_console();
			prefs::draw_window(&g_show_settings);
			draw_about();
			ext::frame();			// preferences, hotkeys, colours, toolbar, textures, renderer, effects, tools

			if (g_show_demo)
			{
				ImGui::ShowDemoWindow(&g_show_demo);
			}

			ImGui::RenderToasts();
			ImGui::EndFrame();

			g_device->SetRenderState(D3DRS_ZENABLE, FALSE);
			g_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			g_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
			g_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(20, 21, 22), 1.0f, 0);

			std::chrono::steady_clock::duration capture_time{};

			if (SUCCEEDED(g_device->BeginScene()))
			{
				ImGui::Render();
				ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
				// the frame dump of CODOL_AUTOSHOT ( reading back and encoding a frame ) is not the editor's work
				const auto capture_start = std::chrono::steady_clock::now();
				components::codol_game::dev_capture_frame();

				if (!g_shot_path.empty())
				{
					const bool saved = save_backbuffer_png(g_shot_path);
					log(saved ? "[CoDOL] frame saved: " + g_shot_path : "^1[CoDOL] could not save the frame to " + g_shot_path);
					g_shot_path.clear();
				}
				capture_time = std::chrono::steady_clock::now() - capture_start;

				g_device->EndScene();
			}

			// the editor's own work per frame ( Present waits for the display )
			{
				auto& st = ui::get();
				const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - frame_start - capture_time).count();
				st.frame_ms = st.frame_ms * 0.9 + ms * 0.1;
				st.frame_ms_max = std::max(st.frame_ms_max, ms);
			}

			if (g_device->Present(nullptr, nullptr, nullptr, nullptr) == D3DERR_DEVICELOST)
			{
				g_device_lost = true;
			}
		}
	}

	// -------------------------------------------------------------------------------------------------

	void log(const std::string& text)
	{
		std::lock_guard lock(g_log_mutex);

		// development aid, like CODOL_AUTOSHOT: CODOL_LOG = <file> gets every console line ( unattended tests read it )
		{
			static int state = 0; // 0: unchecked, 1: active, 2: off
			static std::string path;

			if (state == 0)
			{
				char env[MAX_PATH] = {};
				state = GetEnvironmentVariableA("CODOL_LOG", env, sizeof(env)) ? 1 : 2;
				path = env;
			}

			if (state == 1)
			{
				if (std::ofstream out(path, std::ios::app | std::ios::binary); out)
				{
					out << text;

					if (text.empty() || text.back() != '\n')
					{
						out << '\n';
					}
				}
			}
		}

		std::size_t start = 0;
		while (start <= text.size())
		{
			auto end = text.find('\n', start);
			if (end == std::string::npos)
			{
				end = text.size();
			}

			if (end > start)
			{
				std::string line = text.substr(start, end - start);
				const bool error = line.rfind("^1", 0) == 0;

				if (error)
				{
					line.erase(0, 2);
				}

				g_log.push_back({ error, std::move(line) });
			}

			start = end + 1;
		}

		if (g_log.size() > 5000)
		{
			g_log.erase(g_log.begin(), g_log.begin() + 1000);
		}

		g_log_scroll = true;
	}

	bool save_backbuffer_png(const std::string& path)
	{
		if (!g_device)
		{
			return false;
		}

		IDirect3DSurface9* target = nullptr;
		if (FAILED(g_device->GetRenderTarget(0, &target)) || !target)
		{
			return false;
		}

		D3DSURFACE_DESC desc = {};
		target->GetDesc(&desc);

		bool ok = false;
		IDirect3DSurface9* copy = nullptr;

		if (SUCCEEDED(g_device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &copy, nullptr)) && copy)
		{
			D3DLOCKED_RECT rect = {};

			if (SUCCEEDED(g_device->GetRenderTargetData(target, copy)) && SUCCEEDED(copy->LockRect(&rect, nullptr, D3DLOCK_READONLY)))
			{
				tools::image img;
				img.width = static_cast<int>(desc.Width);
				img.height = static_cast<int>(desc.Height);
				img.rgba.resize(static_cast<std::size_t>(img.width) * static_cast<std::size_t>(img.height) * 4);

				for (int y = 0; y < img.height; y++)
				{
					const auto* src = static_cast<const std::uint8_t*>(rect.pBits) + static_cast<std::size_t>(y) * static_cast<std::size_t>(rect.Pitch);
					auto* dst = img.rgba.data() + static_cast<std::size_t>(y) * static_cast<std::size_t>(img.width) * 4;

					for (int x = 0; x < img.width; x++)
					{
						dst[x * 4 + 0] = src[x * 4 + 2];
						dst[x * 4 + 1] = src[x * 4 + 1];
						dst[x * 4 + 2] = src[x * 4 + 0];
						dst[x * 4 + 3] = 255;
					}
				}

				copy->UnlockRect();

				std::string error;
				ok = tools::save_png(std::filesystem::u8path(path), img, error);
			}

			copy->Release();
		}

		target->Release();
		return ok;
	}

	HWND main_window()
	{
		return g_hwnd;
	}

	void request_exit()
	{
		g_exit = true;
	}
}

int WINAPI wWinMain(const HINSTANCE instance, HINSTANCE, PWSTR, int)
{
	using namespace editor;

	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = wnd_proc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
	wc.lpszClassName = L"CoDOLRadiant";
	RegisterClassExW(&wc);

	g_hwnd = CreateWindowExW(0, wc.lpszClassName, L"CoDOL Radiant - Call of Duty Online 1.1.1.13", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900, nullptr, nullptr, instance, nullptr);

	if (!g_hwnd)
	{
		return 1;
	}

	if (!create_device())
	{
		MessageBoxW(g_hwnd, L"Could not create a Direct3D 9 device with hardware vertex processing.", L"CoDOL Radiant", MB_ICONERROR);
		return 1;
	}

	DragAcceptFiles(g_hwnd, TRUE);
	ShowWindow(g_hwnd, GetEnvironmentVariableA("CODOL_WINDOWED", nullptr, 0) ? SW_SHOWNORMAL : SW_SHOWMAXIMIZED);
	UpdateWindow(g_hwnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	static const std::string ini_path = utils::fs::get_basepath() + "\\codol_editor_layout.ini";

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.IniFilename = ini_path.c_str();

	init_fonts();
	init_style();

	ImGui_ImplWin32_Init(g_hwnd);
	ImGui_ImplDX9_Init(g_device);

	// settings, then the game bridge ( rcon, reload, console commands )
	components::codol_game::register_dvars();
	ext::init();			// settings, actions and commands of ext.hpp, before the settings are read
	dvars::load_config();

	{
		components::codol_game game_bridge;

		log("CoDOL Radiant - standalone editor for Call of Duty Online 1.1.1.13");

		if (components::codol_game::install_path().empty())
		{
			log("^1The CoDOL install folder is not set: File > Settings ( codol_install_path ).");
			g_show_settings = true;
		}

		// the script folder ( <game>\main\sh1x_rawfiles ) and the reference copy of the packed scripts
		codol::packed_scripts::ensure_defaults();

		GET_GUI(ggui::codol_viewport_dialog)->open();
		GET_GUI(ggui::codol_map_ents_dialog)->open();

		// development aid, like CODOL_AUTOSHOT but at a moment a command list picks: ed_shot <file.png>
		command::register_command("ed_shot", [](const std::vector<std::string>& args)
		{
			if (args.size() > 1) { g_shot_path = args[1]; }
			else { log("usage: ed_shot <file.png>   the frame as it is drawn next"); }
		});

		while (!g_exit)
		{
			MSG msg;
			while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);

				if (msg.message == WM_QUIT)
				{
					g_exit = true;
				}
			}

			if (g_exit)
			{
				break;
			}

			if (g_minimized)
			{
				Sleep(50);
				continue;
			}

			if (g_device_lost)
			{
				const HRESULT hr = g_device->TestCooperativeLevel();
				if (hr == D3DERR_DEVICELOST)
				{
					Sleep(50);
					continue;
				}

				if (hr == D3DERR_DEVICENOTRESET)
				{
					g_reset_pending = true;
				}
			}

			if (g_reset_pending)
			{
				g_reset_pending = false;
				reset_device();

				if (g_device_lost)
				{
					Sleep(50);
					continue;
				}
			}

			frame();
		}

		// the game window goes back to the desktop before this window dies ( ~codol_game )
		components::codol_game::detach_game_window();
	}

	dvars::save_config();

	// windows first: they hold device objects
	ggui::loader::destroy_modules();

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	game::glob::d3d9_device = nullptr;

	if (g_device)
	{
		g_device->Release();
	}

	if (g_d3d)
	{
		g_d3d->Release();
	}

	DestroyWindow(g_hwnd);
	UnregisterClassW(wc.lpszClassName, instance);
	CoUninitialize();
	return 0;
}
