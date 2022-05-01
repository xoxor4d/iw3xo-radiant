#include "std_include.hpp"
#include <WinHttpClient.h>

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

	std::ranges::transform(html.begin(), html.end(), std::back_inserter(game::glob::gh_update_releases_json), [](wchar_t c)
	{
		return (char)c;
	});

	if (!game::glob::gh_update_releases_json.empty())
	{
		rapidjson::Document doc;
		doc.Parse(game::glob::gh_update_releases_json.c_str());

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

			if (doc[0].HasMember("assets"))
			{
				if (doc[0]["assets"][0].HasMember("browser_download_url"))
				{
					game::glob::gh_update_link = doc[0]["assets"][0]["browser_download_url"].GetString();
				}

				if (doc[0]["assets"][0].HasMember("name"))
				{
					game::glob::gh_update_zip_name = doc[0]["assets"][0]["name"].GetString();
				}
			}
		}

		if (!game::glob::gh_update_tag.empty())
		{
			const auto tag_ver = utils::try_stof(game::glob::gh_update_tag, true);
			if (tag_ver > (float)REVISION)
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

		if(components::discord::g_enable_discord_rpc)
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

	void SelectRow(int row /*eax*/, game::patchMesh_t* p /*edi*/, int multi)
	{
		const static uint32_t func_addr = 0x43C710;
		__asm
		{
			push	multi;
			mov		eax, row;
			mov		edi, p;
			call	func_addr;
			add		esp, 4;
		}
	}

	// update / rebuild the patch (also visually)
	void Patch_UpdateSelected_r(game::patchMesh_t* p /*esi*/, int always_true)
	{
		const static uint32_t func_addr = 0x438D80;
		__asm
		{
			push	always_true;
			mov		esi, p;
			call	func_addr;
			add		esp, 4;
		}
	}

	void Patch_CalcBounds(game::patchMesh_t* p, game::vec3_t& vMin, game::vec3_t& vMax)
	{
		vMin[0] = vMin[1] = vMin[2] = 99999.0f;
		vMax[0] = vMax[1] = vMax[2] = -99999.0f;
		
		//p->bDirty = true;
		for (int w = 0; w < p->width; w++)
		{
			for (int h = 0; h < p->height; h++)
			{
				for (int j = 0; j < 3; j++)
				{
					const float f = p->ctrl[w][h].xyz[j];
					if (f < vMin[j])
					{
						vMin[j] = f;
					}
						
					if (f > vMax[j])
					{
						vMax[j] = f;
					}
				}
			}
		}
	}

	void selection_rotate_axis(int axis, int deg)
	{
		class rot_helper
		{
		public:
			char pad[16];
			int deg;
			int unused;

			rot_helper(int deg)
			{
				this->deg = deg;
				this->unused = 0;
			}
		}; STATIC_ASSERT_OFFSET(rot_helper, deg, 0x10);

		rot_helper helper(deg);

		switch(axis)
		{
		case 0:
			utils::hook::call<void(__stdcall)(rot_helper*, int*)>(0x450EF0)(&helper, &helper.unused);
			break;

		case 1:
			utils::hook::call<void(__stdcall)(rot_helper*, int*)>(0x450F80)(&helper, &helper.unused);
			break;

		case 2:
			utils::hook::call<void(__stdcall)(rot_helper*, int*)>(0x451010)(&helper, &helper.unused);
			break;
		}
	}

	void generate_rope()
	{
		const auto egui = GET_GUI(ggui::entity_dialog);

		game::vec3_t pts[2];
		int null_count = 0;

		FOR_ALL_SELECTED_BRUSHES(sb)
		{
			if(sb->def && null_count < 2)
			{
				const char* class_str = egui->get_value_for_key_from_epairs(sb->def->owner->epairs, "classname");
				if(class_str != "info_null"s)
				{
					null_count = 0;
					break;
				}

				for (int j = 0; j < 3; j++)
				{
					pts[null_count][j] = sb->def->mins[j] + abs((sb->def->maxs[j] - sb->def->mins[j]) * 0.5f);
				}

				null_count++;
			}
		}

		if(null_count != 2)
		{
			game::printf_to_console("Must have 2 info_null's selected!");
			return;
		}

		// deselect everything
		game::Select_Deselect(1);


		// -------------------
		// generate a new brush

		game::vec3_t mins = { -8.0f, -8.0f, -8.0f };
		game::vec3_t maxs = { 8.0f, 8.0f, 8.0f };

		// Brush_Alloc
		const auto new_b = utils::hook::call<game::brush_t_with_custom_def* (__cdecl)(void*, void*)>(0x4751E0)(game::g_qeglobals->random_texture_stuff, 0);
		game::Brush_Create(maxs, mins, new_b, 0);

		if (!new_b)
		{
			return;
		}

		game::Brush_BuildWindings(new_b, 1);
		++new_b->version;

		game::Entity_LinkBrush(new_b, game::g_world_entity()->firstActive);
		const auto b_linked = game::Brush_AddToList(new_b, game::g_world_entity());
		if (b_linked->onext || b_linked->oprev)
		{
			__debugbreak();
		}

		game::Brush_AddToList2(b_linked);


		// -------------------
		// create cylinder

		// create patch cylinder
		cdeclcall(void, 0x42A3B0); // CMainFrame::OnCurvePatchtube

		// rotate cylinder Y (always spawned with opening facing Z)
		cdeclcall(void, 0x425190); // CMainFrame::OnBrushRotatey

		// get cylinder 
		const auto cyl = game::g_selected_brushes();


		// get angles between the two null's and rotate accordingly
		game::vec3_t angles = {};
		{
			game::vec3_t diff = {};
			utils::vector::subtract(pts[1], pts[0], diff);
			utils::vector::vectoangles(diff, angles);
		}
		
		if (angles[0] != 0.0f)
		{
			selection_rotate_axis(1, static_cast<int>(-angles[0]));
		}

		if (angles[1] != 0.0f)
		{
			selection_rotate_axis(2, static_cast<int>(-angles[1]));
		}

		if (angles[2] != 0.0f)
		{
			selection_rotate_axis(0, static_cast<int>(angles[2]));			
		}


		// change selection mode
		game::g_qeglobals->d_select_mode = game::select_t::sel_curvepoint; 


		// move individual rows
		{
			game::vec3_t center_r0, center_mid, center_r2;

			{
				// select all 9 vertices of the first row
				SelectRow(0, cyl->def->patch, 0);

				// calc center of all selected points
				ggui::camera_guizmo::get_selection_center_movepoints(center_r0);

				// get dist between second null and center of first row
				game::vec3_t diff_r0 = {};
				utils::vector::subtract(pts[1], center_r0, diff_r0);

				// move points accordingly
				for (auto i = 0; i < game::g_qeglobals->d_num_move_points; i++)
				{
					utils::vector::add(game::g_qeglobals->d_move_points[i]->xyz, diff_r0, game::g_qeglobals->d_move_points[i]->xyz);
				}
			}

			{
				// select all 9 vertices of the third row
				SelectRow(2, cyl->def->patch, 0);

				// calc center of all selected points
				ggui::camera_guizmo::get_selection_center_movepoints(center_r2);

				// get dist between first null and center of last row
				game::vec3_t diff_r2 = {};
				utils::vector::subtract(pts[0], center_r2, diff_r2);

				// move points accordingly
				for (auto i = 0; i < game::g_qeglobals->d_num_move_points; i++)
				{
					utils::vector::add(game::g_qeglobals->d_move_points[i]->xyz, diff_r2, game::g_qeglobals->d_move_points[i]->xyz);
				}
			}

			{
				// calc center between the 2 nulls
				game::vec3_t center_of_two_nulls;
				
				utils::vector::add(pts[0], pts[1], center_of_two_nulls);
				utils::vector::scale(center_of_two_nulls, 0.5f, center_of_two_nulls);

				// select all 9 vertices of the middle row
				SelectRow(1, cyl->def->patch, 0);

				// calc center of all selected points
				ggui::camera_guizmo::get_selection_center_movepoints(center_mid);

				// get dist between center of null's and center of middle row
				game::vec3_t diff_mid = {};
				utils::vector::subtract(center_of_two_nulls, center_mid, diff_mid);

				// add slack (10% of dist between nulls)
				diff_mid[2] -= fabs(utils::vector::distance(pts[1], pts[0])) * 0.1f;

				// move points accordingly
				for (auto i = 0; i < game::g_qeglobals->d_num_move_points; i++)
				{
					utils::vector::add(game::g_qeglobals->d_move_points[i]->xyz, diff_mid, game::g_qeglobals->d_move_points[i]->xyz);
				}
			}
		}

		Patch_UpdateSelected_r(cyl->def->patch, 1);
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

		// add iw3xradiant search path (imgui images)
		utils::hook(0x4A29A7, fs_scan_base_directory_stub, HOOK_JUMP).install()->quick();

		// do not load "_glow" fonts (qerfont_glow)
		utils::hook::nop(0x552806, 5);

		// nop com_math.cpp "det" line:1775 assert (MatrixInverse44)
		utils::hook::nop(0x4A6BC9, 5);

		// set max undos -> done via preference window
		//utils::hook::set<int32_t>(0x739F6C, 512);

		// set max undo memory
		utils::hook::set<int32_t>(0x739F70, 0x01000000); // default 2mb, now 16mb

		// read parsed entities counter for debug printing
		utils::hook::nop(0x4865FB, 8);
		utils::hook(0x4865FB, map_parse_error_stub_01, HOOK_JUMP).install()->quick();

		// map parsing debug prints (parsed entities and brushes, helpful for ParseEntity errors)
		utils::hook(0x483EDB, parseentity_stub, HOOK_JUMP).install()->quick();


		// * ---------------------------

		command::register_command("rope"s, [](auto)
		{
			generate_rope();
		});

		command::register_command("rope_select"s, [](auto)
		{
			const auto cyl = game::g_selected_brushes();
			SelectRow(0, cyl->def->patch, 0);
		});

		command::register_command("rope_rot"s, [](auto)
		{
			class rotate_around_axis_helper
			{
			public:
				char pad[16];
				int deg;
				int unused;

				rotate_around_axis_helper(int deg)
				{
					this->deg = deg;
					this->unused = 0;
				}
			}; STATIC_ASSERT_OFFSET(rotate_around_axis_helper, deg, 0x10);


			rotate_around_axis_helper helper(90);
			utils::hook::call<void(__stdcall)(rotate_around_axis_helper*, int*)>(0x450EF0)(&helper, &helper.unused);
				
		});

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
