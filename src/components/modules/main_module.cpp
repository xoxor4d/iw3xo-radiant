#include "std_include.hpp"

DWORD WINAPI paint_msg_loop(LPVOID)
{
	int base_time = 0;
	int current_frame = 0;

	double timer_grid = 0;
	double timer_camera = 0;
	double timer_textures = 0;
	double timer_modelselector = 0;
	double timer_gui = 0;
	
	while (true)
	{
		if (game::glob::d3d9_device)
		{
			const float maxfps_grid = 1000.0f / (float)dvars::radiant_maxfps_grid->current.integer;
			const float maxfps_camera = 1000.0f / (float)dvars::radiant_maxfps_camera->current.integer;
			const float maxfps_textures = 1000.0f / (float)dvars::radiant_maxfps_textures->current.integer;
			const float maxfps_modelselector = 1000.0f / (float)dvars::radiant_maxfps_modelselector->current.integer;
			const float maxfps_mainframe = 1000.0f / (float)dvars::radiant_maxfps_mainframe->current.integer;

			float maxfps_gui_f; // force gui to use the lowest frametime / highest framerate of the above or its own setting
			maxfps_gui_f = fminf(maxfps_grid, maxfps_camera);
			maxfps_gui_f = fminf(maxfps_gui_f, maxfps_textures);
			maxfps_gui_f = fminf(maxfps_gui_f, maxfps_modelselector);

			{ // cap / limit gui framerate to the highest framerate of the above
				const int val = static_cast<int>(1000.0f / maxfps_gui_f);
				dvars::radiant_maxfps_mainframe->domain.integer.min = val;

				if (maxfps_gui_f <= maxfps_mainframe)
				{
					if(val >= dvars::radiant_maxfps_mainframe->domain.integer.min && val <= dvars::radiant_maxfps_mainframe->domain.integer.max)
					{
						dvars::set_int(dvars::radiant_maxfps_mainframe, val);
					}
				}
				else
				{
					maxfps_gui_f = maxfps_mainframe;
				}
			}
			
			if (!base_time) {
				base_time = timeGetTime();
			}

			current_frame = static_cast<int>(timeGetTime()) - base_time;

			if (current_frame - static_cast<int>(timer_gui) < 0) {
				current_frame = static_cast<int>(timer_gui);
			}

			if (current_frame > timer_grid)
			{
				timer_grid = current_frame;
				if (const auto hwnd = cmainframe::activewnd->m_pXYWnd->GetWindow();
					hwnd != nullptr)
				{
					SendMessageA(hwnd, WM_PAINT, 0, 0);
				}
				timer_grid += maxfps_grid;
			}

			if (current_frame > timer_camera)
			{
				timer_camera = current_frame;

				if (const auto hwnd = cmainframe::activewnd->m_pCamWnd->GetWindow();
					hwnd != nullptr)
				{
					SendMessageA(hwnd, WM_PAINT, 0, 0);

					/*if (components::effects::effect_can_play())
					{
						if (components::effects::effect_is_playing())
						{
							components::effects::editor_trigger_effect(fx_system::ed_playback_tick);
						}
						else
						{
							components::effects::editor_on_effect_play_repeat();
						}
					}*/
				}
				timer_camera += maxfps_camera;
			}

			if (current_frame > timer_textures)
			{
				timer_textures = current_frame;

				if (const auto hwnd = cmainframe::activewnd->m_pTexWnd->GetWindow();
					hwnd != nullptr)
				{
					SendMessageA(hwnd, WM_PAINT, 0, 0);
				}
				timer_textures += maxfps_textures;
			}

			if (current_frame > timer_modelselector)
			{
				timer_modelselector = current_frame;

				if (const auto hwnd = layermatwnd_struct->m_content_hwnd;
					hwnd != nullptr)
				{
					SendMessageA(hwnd, WM_PAINT, 0, 0);
				}
				timer_modelselector += maxfps_modelselector;
			}
			
			if (current_frame > timer_gui)
			{
				timer_gui = current_frame;

				if (const auto hwnd = cmainframe::activewnd->m_pZWnd->GetWindow();
					hwnd != nullptr)
				{
					SendMessageA(hwnd, WM_PAINT, 0, 0);
				}

				timer_gui += maxfps_gui_f;
			}
		}
	}
	
	return TRUE;
}

BOOL init_threads()
{
	//CreateThread(nullptr, 0, gui_paint_msg, nullptr, 0, nullptr);
	CreateThread(nullptr, 0, paint_msg_loop, nullptr, 0, nullptr);

	// Create LiveRadiant thread (connecting to the server)
	CreateThread(nullptr, 0, remote_net_search_server_thread, nullptr, 0, nullptr);

	// Create LiveRadiant thread (receiving commands from the server)
	CreateThread(nullptr, 0, remote_net_receive_packet_thread, nullptr, 0, nullptr);

	game::glob::command_thread_running = false;
	if (CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(components::command::command_thread), nullptr, 0, nullptr))
	{
		game::glob::command_thread_running = true;
	}

	// -----------
	// I/O Console

	// Disable STDOUT buffering
	setvbuf(stdout, nullptr, _IONBF, 0);

	// Create an external console for Radiant
	if (AllocConsole())
	{
		FILE *file = nullptr;
		freopen_s(&file, "CONIN$", "r", stdin);
		freopen_s(&file, "CONOUT$", "w", stdout);
		freopen_s(&file, "CONOUT$", "w", stderr);

		SetConsoleTitleA("IW3R Console");
	}
	
	return TRUE;
}


namespace components
{
	void add_iw3xradiant_searchpath()
	{
		if(const auto fs_basepath = game::Dvar_FindVar("fs_basepath"); 
			fs_basepath)
		{
			game::FS_ScanForDir("bin/IW3xRadiant", fs_basepath->current.string, false);
		}
		
	}
	
	void __declspec(naked) fs_scan_base_directory_stub()
	{
		const static uint32_t Com_Printf_Func = 0x40B5D0;
		const static uint32_t retn_pt = 0x4A29AC;
		__asm
		{
			call	Com_Printf_Func;
			
			pushad;
			call	add_iw3xradiant_searchpath;
			popad;

			jmp		retn_pt;
		}
	}

	main_module::main_module()
	{
		init_threads();

		// init internal console class
		static ggui::console console;
		ggui::console::hooks();
		
		radiantapp::hooks();
		cmainframe::hooks();
		czwnd::hooks();
		cxywnd::hooks();
		ccamwnd::hooks();
		clayermatwnd::hooks();
		ctexwnd::hooks();

		// ggui hooks ~ gui::gui()


		// add iw3xradiant search path (imgui images)
		utils::hook(0x4A29A7, fs_scan_base_directory_stub, HOOK_JUMP).install()->quick();

		// do not load "_glow" fonts (qerfont_glow)
		utils::hook::nop(0x552806, 5);

		// hook / Grab CameraWnd object (only when using floating windows) :: (cmainframe::update_windows sets cmainframe::activewnd otherwise)
		//utils::hook(0x42270C, ccam_init_stub, HOOK_JUMP).install()->quick();

#if 0 // not needed because func was rewritten (renderer)
		// disable black world on selecting a brush with sun preview enabled -> still disables active sun preview .. no black world tho
		utils::hook::nop(0x406A11, 5);

		// sunpreview: disable shadow projection for unselected brushes and entities
		utils::hook::nop(0x406A70, 5);
		// sunpreview: disable shadow projection for selected brushes and entities
		utils::hook::nop(0x406A86, 5);
		// sunpreview: disable shadow polyoffset
		utils::hook::nop(0x406A8E, 5);

		// sunpreview: keep sunpreview active even if a brush is selected ... what the fuck -> can no longer copy-paste brushes
		//utils::hook::nop(0x484947, 5);

		// disable black world on selecting a brush with sun preview enabled -> no longer able to clone brushes ...
		//utils::hook::set<BYTE>(0x484904, 0xEB);
#endif

		// nop com_math.cpp "det" line:1775 assert (MatrixInverse44)
		utils::hook::nop(0x4A6BC9, 5);

		// set max undos -> done via preference window
		//utils::hook::set<int32_t>(0x739F6C, 512);

		// set max undo memory
		utils::hook::set<int32_t>(0x739F70, 0x01000000); // default 2mb, now 16mb

		// * ---------------------------

		// creates a brush that encupsules all selected brushes/patches and uses texture info of the first selected brush
		// then deletes the original selection
		command::register_command_with_hotkey("brush_from_selected"s, [this](auto)
		{
			game::vec3_t bounds_maxs = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
			game::vec3_t bounds_mins = { FLT_MAX, FLT_MAX, FLT_MAX };

			for (auto	sb = game::g_selected_brushes_next();
				(DWORD*)sb != game::currSelectedBrushes;
				sb = sb->next)
			{
				if(remote_net::selection_is_brush(sb->def))
				{
					if (sb->def->maxs[0] > bounds_maxs[0])
					{
						bounds_maxs[0] = sb->def->maxs[0];
					}

					if (sb->def->maxs[1] > bounds_maxs[1])
					{
						bounds_maxs[1] = sb->def->maxs[1];
					}

					if (sb->def->maxs[2] > bounds_maxs[2])
					{
						bounds_maxs[2] = sb->def->maxs[2];
					}


					if (sb->def->mins[0] < bounds_mins[0])
					{
						bounds_mins[0] = sb->def->mins[0];
					}

					if (sb->def->mins[1] < bounds_mins[1])
					{
						bounds_mins[1] = sb->def->mins[1];
					}

					if (sb->def->mins[2] < bounds_mins[2])
					{
						bounds_mins[2] = sb->def->mins[2];
					}
				}
			}

			if (bounds_maxs[0] == -FLT_MAX || bounds_maxs[1] == -FLT_MAX || bounds_maxs[2] == -FLT_MAX)
			{
				return;
			}

			if (bounds_mins[0] == FLT_MAX || bounds_mins[1] == FLT_MAX || bounds_mins[2] == FLT_MAX)
			{
				return;
			}

			// should not happen
			if((DWORD*)game::g_selected_brushes_next() == game::currSelectedBrushes)
			{
				//// Brush_Alloc
				//auto new_b = utils::hook::call<game::brush_t_with_custom_def* (__cdecl)(void*, void*)>(0x4751E0)(game::qeglobals_t().random_texture_stuff, nullptr);
				//game::Brush_Create(bounds_maxs, bounds_mins, new_b, 0);

				//if (!new_b)
				//{
				//	return;
				//}

				//game::Brush_BuildWindings(new_b, 1);
				//++new_b->version;

				//game::Entity_LinkBrush(new_b, game::g_world_entity()->firstActive);
				//auto b_linked = game::Brush_AddToList(new_b, game::g_world_entity());
				//if (b_linked->onext || b_linked->oprev)
				//{
				//	__debugbreak();
				//}

				//game::Brush_AddToList2(b_linked);
			}

			// should always replace the first selected brush
			else
			{
				// needs Undo_AddBrushList and Undo_EndBrushList

				game::Undo_ClearRedo();
				game::Undo_GeneralStart("create brush from selection");

				auto curr_sb = game::g_selected_brushes_next();
				auto b = game::g_selected_brushes_next()->def;
				game::Brush_Create(bounds_maxs, bounds_mins, b, 0);
				game::Brush_BuildWindings(b, 1);
				++b->version;

				// remove all initial selection brushes
				if (auto sb = curr_sb->next; 
						 sb)
				{
					game::selbrush_def_t* next = nullptr;

					do
					{
						if (sb == curr_sb)
						{
							break;
						}

						if (!remote_net::selection_is_brush(sb->def))
						{
							break;
						}

						next = sb->next;

						// Brush_Free
						utils::hook::call<void(__cdecl)(game::selbrush_def_t*)>(0x475BA0)(sb);

						sb = next;

					} while (next);
				}

				game::Undo_End();
			}
		});
	}

	main_module::~main_module()
	{ }
}
