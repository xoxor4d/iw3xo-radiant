#include "std_include.hpp"
#include <WinHttpClient.h>
#include <codecvt>

bool get_html(const std::string& url, std::wstring& header, std::wstring& hmtl)
{
	const auto wurl = std::wstring(url.begin(), url.end());
	bool ret = false;

	try
	{
		WinHttpClient client(wurl);
		std::string url_protocol = url.substr(0, 5);

		std::ranges::transform(url_protocol.begin(), url_protocol.end(), url_protocol.begin(), (int (*)(int))std::toupper);

		if (url_protocol == "HTTPS")
		{
			client.SetRequireValidSslCertificates(false);
		}

		client.SetUserAgent(L"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:19.0) Gecko/20100101 Firefox/19.0");

		if (client.SendHttpRequest())
		{
			header = client.GetResponseHeader();
			hmtl = client.GetResponseContent();
			ret = true;
		}
	}
	catch (...)
	{
		header = L"Error";
		hmtl = L"";
	}

	return ret;
}

DWORD WINAPI update_check(LPVOID)
{
	const std::string url = "https://api.github.com/repos/xoxor4d/iw3xo-radiant/releases";

	std::wstring header, html;
	get_html(url, header, html);

	/*std::ranges::transform(html.begin(), html.end(), std::back_inserter(game::glob::gh_update_releases_json), [](wchar_t c)
	{
		return (char)c;
	});*/

	// chatgpt is awesome
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	game::glob::gh_update_releases_json = converter.to_bytes(html);

	if (!game::glob::gh_update_releases_json.empty())
	{
		rapidjson::Document doc;

		if (doc.Parse(game::glob::gh_update_releases_json.c_str()).HasParseError())
		{
//#if DEBUG
//			auto err = doc.GetParseError();
//			__debugbreak();
//#endif
			return TRUE;
		}

		if (!doc.Empty())
		{
			// latest release should be at index 0
			if (doc[0].HasMember("tag_name"))
			{
				game::glob::gh_update_tag = doc[0]["tag_name"].GetString();
			}

			if (doc[0].HasMember("name"))
			{
				game::glob::gh_update_title = doc[0]["name"].GetString();
			}

			if (doc[0].HasMember("published_at"))
			{
				game::glob::gh_update_date = doc[0]["published_at"].GetString();

				const auto pos = game::glob::gh_update_date.find_first_of('T');
				if (pos)
				{
					game::glob::gh_update_date = game::glob::gh_update_date.substr(0, pos);
				}
			}

			if (doc[0].HasMember("assets") && !doc[0]["assets"].Empty())
			{
				rapidjson::Value& call_command = doc[0]["assets"][0];
				const rapidjson::Value::ConstMemberIterator download_url_itr = call_command.FindMember("browser_download_url");
				const rapidjson::Value::ConstMemberIterator release_name_itr = call_command.FindMember("name");

				if (download_url_itr != call_command.MemberEnd())
				{
					game::glob::gh_update_link = download_url_itr->value.GetString();
				}

				if (release_name_itr != call_command.MemberEnd())
				{
					game::glob::gh_update_zip_name = release_name_itr->value.GetString();
				}
			}
		}

		if (!game::glob::gh_update_tag.empty())
		{
			const auto tag_ver = utils::try_stof(game::glob::gh_update_tag, true);
			if (tag_ver > utils::try_stof(GIT_TAG, true))
			{
				if (!game::glob::gh_update_title.empty() && !game::glob::gh_update_link.empty() && !game::glob::gh_update_zip_name.empty())
				{
					game::glob::gh_update_avail = true;
				}
			}
		}
	}

	return TRUE;
}

// #
// #

DWORD WINAPI discord_rpc(LPVOID)
{
	while (true)
	{
		Sleep(5000);

		if (components::discord::g_enable_discord_rpc)
		{
			components::discord::init();
			components::discord::update_discord();
		}
		else
		{
			components::discord::shutdown();
		}
	}
}

// #
// #

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
		if (game::glob::in_shutdown)
		{
			break;
		}

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
				}

				if (const auto hwnd = components::renderer::get_window(components::renderer::CFXWND)->hwnd;
					hwnd != nullptr)
				{
					SendMessageA(hwnd, WM_PAINT, 0, 0);
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

// #
// #

BOOL init_threads()
{
	// continuous gui painting
	CreateThread(nullptr, 0, paint_msg_loop, nullptr, 0, nullptr);

	// live-link thread (search game server)
	CreateThread(nullptr, 0, remote_net_search_server_thread, nullptr, 0, nullptr);

	// live-link thread (receiving commands from the server)
	CreateThread(nullptr, 0, remote_net_receive_packet_thread, nullptr, 0, nullptr);

	// discord rich presence
	CreateThread(nullptr, 0, discord_rpc, nullptr, 0, nullptr);

	// check for iw3xradiant updates
	CreateThread(nullptr, 0, update_check, nullptr, 0, nullptr);

	game::glob::command_thread_running = false;
	if (CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(components::command::command_thread), nullptr, 0, nullptr))
	{
		game::glob::command_thread_running = true;
	}

	// disable stdout buffering
	setvbuf(stdout, nullptr, _IONBF, 0);

	// create an external console
	if (AllocConsole())
	{
		FILE *file = nullptr;
		freopen_s(&file, "CONIN$", "r", stdin);
		freopen_s(&file, "CONOUT$", "w", stdout);
		freopen_s(&file, "CONOUT$", "w", stderr);

		SetConsoleTitleA("IW3xRadiant Console");
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


	// *
	// map parsing debug prints (parsed entities and brushes, helpful for ParseEntity errors)

	int map_parse_entity_num_counter = 0;
	void __declspec(naked) map_parse_error_stub_01()
	{
		const static uint32_t retn_addr = 0x486603;
		__asm
		{
			mov		edx, dword ptr[ebp - 8];
			mov		map_parse_entity_num_counter, edx;

			add     dword ptr[ebp - 8], 1; // ebp - 8 = entitiynum counter
			cmp     dword ptr[esi + 4], 0;
			jmp		retn_addr;
		}
	}

	void parseentity_debug_print()
	{
		if(dvars::parse_debug && dvars::parse_debug->current.enabled)
		{
			game::printf_to_console("parsing entity: %d -- parsed brushes: %d\n", map_parse_entity_num_counter, game::g_qeglobals->d_parsed_brushes - 1);
		}
	}

	void __declspec(naked) parseentity_stub()
	{
		const static uint32_t retn_addr = 0x483EE0;
		__asm
		{
			pushad;
			call	parseentity_debug_print;
			popad;

			mov     ecx, 2; // og
			jmp		retn_addr;
		}
	}

	void undo_general_start_print(const char* op)
	{
		game::printf_to_console("[UNDO] '%s' added to the undo stack. New stack size: %d\n", op, game::g_undoId);
	}

	void __declspec(naked) undo_general_start_stub()
	{
		const static uint32_t retn_addr = 0x45E3F9;
		__asm
		{
			push    ebx; // og
			push    esi; // og
			push    edi; // og
			mov     edi, eax; // og

			pushad;
			push	eax;
			call	undo_general_start_print;
			add		esp, 4;
			popad;

			jmp		retn_addr;
		}
	}

	void undo_undo_print(const char* op)
	{
		game::printf_to_console("[UNDO] '%s' undone. New stack size: %d\n", op, game::g_undoSize - 1); // g_undoSize will be decremented right after the hook
	}

	void __declspec(naked) undo_undo_stub()
	{
		const static uint32_t retn_addr = 0x45F1CB;
		__asm
		{
			pushad;
			push	edx; // operation
			call	undo_undo_print;
			add		esp, 4;
			popad;

			jmp		retn_addr;
		}
	}

	// ----------------------------------


	main_module::main_module()
	{
		init_threads();

		GET_GUI(ggui::console_dialog)->hooks();
		radiantapp::hooks();
		cmainframe::hooks();
		czwnd::hooks();
		cxywnd::hooks();
		ccamwnd::hooks();
		clayermatwnd::hooks();
		ctexwnd::hooks();

		// *
		// *

		// add iw3xradiant search path (imgui images)
		utils::hook(0x4A29A7, fs_scan_base_directory_stub, HOOK_JUMP).install()->quick();

		// set max undo memory
		utils::hook::set<int32_t>(0x739F70, 0x01000000); // default 2mb, now 16mb

		// increase s_hunkTotal x3 (200 to 600 mb)
		utils::hook::set<int32_t>(0x4AC582 + 1, 0x25800000);
		utils::hook::set<int32_t>(0x4AC588 + 6, 0x25800000);

		// read parsed entities counter for debug printing
		utils::hook::nop(0x4865FB, 8);
		utils::hook(0x4865FB, map_parse_error_stub_01, HOOK_JUMP).install()->quick();

		// map parsing debug prints (parsed entities and brushes, helpful for ParseEntity errors)
		utils::hook(0x483EDB, parseentity_stub, HOOK_JUMP).install()->quick();

		// print undo creation and stack size
		utils::hook(0x45E3F4, undo_general_start_stub, HOOK_JUMP).install()->quick();

		// print undo undo and stack size
		utils::hook(0x45F1C6, undo_undo_stub, HOOK_JUMP).install()->quick();


		// * ---------------------------

		/*command::register_command("rope"s, [](const std::vector<std::string>& args)
		{
			if (args.size() != 4)
			{
				game::printf_to_console("Usage: rope <int: thickness>  <int: slack in percent (0-100)>  <bool: delete nulls>");
				return;
			}

			generate_rope(utils::try_stoi(args[1], false), utils::try_stoi(args[2], false), utils::try_stoi(args[3], false));
		});*/

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
