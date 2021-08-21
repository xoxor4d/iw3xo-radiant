#include "std_include.hpp"

// tests
//#define SPLITTER_TEST
//#define CREATE_SECOND_CAMERA
//#define HIDE_MAINFRAME_MENUBAR // fully working
//#define HIDE_MAINFRAME_TOOLBAR // fully working

// Create a message pump thread so that we can update certain windows at a constant framerate
DWORD WINAPI realtimewnd_msg_pump(LPVOID)
{
	const int maxfps = 1000 / 250;

	int sys_timeBase = 0;
	int com_frameTime = 0;
	int com_lastFrameTime = 0;
	int frametime_msec = 0;
	
	while(true)
	{
		if(game::glob::d3d9_device)
		{
			while (true)
			{
				if(!sys_timeBase)
				{
					sys_timeBase = timeGetTime();
				}
				
				com_frameTime = timeGetTime() - sys_timeBase;
				int last = com_lastFrameTime;
				
				if ((com_frameTime - com_lastFrameTime) < 0)
				{
					last = com_frameTime;
					com_lastFrameTime = com_frameTime;
				}

				if ((com_frameTime - last) >= maxfps)
				{
					break;
				}
				
				Sleep(0u);
			}

			frametime_msec = com_frameTime - com_lastFrameTime;
			com_lastFrameTime = com_frameTime;

			// update cam window ~ 250fps
			if  (const auto hwnd = cmainframe::activewnd->m_pCamWnd->GetWindow();
				hwnd != nullptr)
			{
				if(frametime_msec <= 100)
				{
					game::glob::ccamwindow_realtime = true;
					SendMessageA(hwnd, WM_PAINT, 0, 0);
				}
				else
				{
					game::glob::ccamwindow_realtime = false;
				}
			}

			// update xy window ~ 250fps
			if (const auto hwnd = cmainframe::activewnd->m_pXYWnd->GetWindow();
				hwnd != nullptr)
			{
				SendMessageA(hwnd, WM_PAINT, 0, 0);
			}
		}
	}

	return TRUE;
}

BOOL init_threads()
{
	// Create a message pump thread to have the camera update at a constant framerate
	CreateThread(nullptr, 0, realtimewnd_msg_pump, nullptr, 0, nullptr);

	// Create LiveRadiant thread (connecting to the server)
	CreateThread(nullptr, 0, remote_net_search_server_thread, nullptr, 0, nullptr);

	// Create LiveRadiant thread (receiving commands from the server)
	CreateThread(nullptr, 0, remote_net_receive_packet_thread, nullptr, 0, nullptr);


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

	// command Thread
	//CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(command_thread), nullptr, 0, nullptr);
	CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(components::command::command_thread), nullptr, 0, nullptr);
	return TRUE;
}


namespace components
{
	__declspec(naked) void ccam_init_stub()
	{
		const static uint32_t CCam_activewnd_Func = 0x402C40;
		const static uint32_t retnPt = 0x422711;
		__asm
		{
			Call	CCam_activewnd_Func
			mov		ccamwnd::activewnd, eax
			mov		game::glob::radiant_floatingWindows, 1
			jmp		retnPt
		}
	}

#ifdef SPLITTER_TEST
	void setrowinfo()
	{
		typedef void (__thiscall* CSplitterWnd__SetRowInfo_t)(CSplitterWnd*, int row, int, int);
		/* ------------------------- */ const auto test = reinterpret_cast<CSplitterWnd__SetRowInfo_t>(0x5A544B);

		test(&cmainframe::activewnd->m_wndSplit, 2, 50, 50);
	}

	__declspec(naked) void setrowinfo_stub()
	{
		const static uint32_t retn_pt = 0x422FC9;
		__asm
		{
			pushad;
			call	setrowinfo;
			popad;

			mov     esi, [edx + 1C8h]; // og
			jmp		retn_pt;
		}
	}

	void createstatic(void* create_context)
	{
		//(main_frame->m_wndSplit.CreateStatic)(&main_frame->m_wndSplit, 1, 0, &ceditWnd, 25, 100, a3);
		auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);

		tagSIZE size = tagSIZE(25, 100);
		void* texclass = reinterpret_cast<void*>(0x6D50D8); // cam
		
		vtable->CreateView(&cmainframe::activewnd->m_wndSplit, 2, 0, (CRuntimeClass*)texclass, size, (CCreateContext*)create_context);

		
		typedef CWnd* (__thiscall* CSplitterWnd__GetPane_t)(CSplitterWnd*, int row, int col);
		/* ------------------------- */ const auto test = reinterpret_cast<CSplitterWnd__GetPane_t>(0x5A5409);

		ccamwnd* window = reinterpret_cast<ccamwnd*>(test(&cmainframe::activewnd->m_wndSplit, 2, 0));
		cmainframe::activewnd->m_pCamWnd = window;
	}

	__declspec(naked) void createstatic_stub()
	{
		const static uint32_t retn_pt = 0x422D84;
		__asm
		{
			pushad;
			push	ecx; // context
			call	createstatic;
			add		esp, 4h;
			popad;

			mov     edx, [edi + 6E0h]; // og
			jmp		retn_pt;
		}
	}
#endif


	void add_iw3xradiant_searchpath()
	{
		if(const auto fs_basepath = game::Dvar_FindVar("fs_basepath"); 
			fs_basepath)
		{
			game::FS_ScanForDir("bin/IW3xRadiant", fs_basepath->current.string, false);
		}
		
	}
	
	__declspec(naked) void fs_scan_base_directory_stub()
	{
		const static uint32_t FS_RegisterDvars_Func = 0x4A2310;
		const static uint32_t retn_pt = 0x4A2457;
		__asm
		{
			call	FS_RegisterDvars_Func;
			
			pushad;
			call	add_iw3xradiant_searchpath;
			popad;

			jmp		retn_pt;
		}
	}

#ifdef CREATE_SECOND_CAMERA
	void __declspec(naked) set_rinitforwindow_stub()
	{
		const static uint32_t retn_pt = 0x4166CB;
		__asm
		{
			mov     eax, game::glob::test_hwnd;
			jmp		retn_pt;
		}
	}

	ccamwnd* cam__init(ccamwnd* cam)
	{
		const static uint32_t CCamWnd_CamInit_Func = 0x402C40;
		__asm
		{
			mov		esi, cam;
			call	CCamWnd_CamInit_Func;
		}
	}

	void testtest()
	{
		typedef void* (__cdecl* operator_new_t)(size_t size);
		operator_new_t operator_new = reinterpret_cast<operator_new_t>(0x583A58);

		auto _new = operator_new(828u);
		auto cam = cam__init(reinterpret_cast<ccamwnd*>(_new));

		tagRECT rect = { 5, 25, 100, 100 };
		
		auto vtable = reinterpret_cast<CWnd_vtbl*>(cam->__vftable);

		//cmainframe::activewnd->m_pCamWnd = cam;

		vtable->Create(
			cam
			, "QCamera"
			, 0
			, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZEBOX
			, &rect
			,cmainframe::activewnd
			, 1234u
			, 0);

		//cam->m_hWnd = cmainframe::activewnd->m_pCamWnd->m_hWnd;
		game::glob::test_hwnd = cam->m_hWnd;

		ShowWindow(cam->m_hWnd, SW_SHOW);

		int x = 1;
	}
	
	__declspec(naked) void test_stub()
	{
		const static uint32_t CreateQEChildren_Func = 0x421820;
		const static uint32_t retn_pt = 0x422989;
		__asm
		{
			pushad;
			call	testtest;
			popad;

			call	CreateQEChildren_Func; // og
			jmp		retn_pt;
		}
	}
#endif

#ifdef HIDE_MAINFRAME_TOOLBAR
	__declspec(naked) void show_toolbar_stub()
	{
		const static uint32_t retn_pt = 0x4211CB;
		__asm
		{
			mov		eax, 0;
			jmp		retn_pt;
		}
	}
#endif

	void load_raw_materials_progressbar(int index, int material_total_count)
	{
		const int    idx = index + 1;
		const double percentage = ((double)idx / (double)material_total_count);
		
		const int val  = static_cast<int>(percentage * 100);
		const int lpad = static_cast<int>(percentage * 60);
		const int rpad = 60 - lpad;

		printf("\rLoading raw materials: %3d%% [%.*s%*s] %d/%d", val, lpad, "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||", rpad, "", idx, material_total_count);

		if (val == 100)
		{
			printf("\n");
		}
			
		fflush(stdout);
	}

	__declspec(naked) void load_raw_materials_progressbar_stub()
	{
		const static uint32_t retn_pt = 0x45AF65;
		__asm
		{
			pushad;
			mov		eax, [ebp - 54h];
			push	eax;
			push	ecx;
			call	load_raw_materials_progressbar;
			add		esp, 8;
			popad;
			
			add     ecx, 1; // og
			cmp     ecx, [ebp - 54h]; // ebp - 54 = total amount of materials

			jmp		retn_pt;
		}
	}


	

	//void testtest()
	//{
	//	// R_Set2D
	//	utils::function<void(game::GfxCmdBufSourceState*)>(0x53CF50)(game::gfx_cmd_buf_source_state);
	//	
	//	if(test_font && test_color)
	//	{
	//		utils::function<void(const char* text, int text_len, game::Font_s* font, float x, float y, float xscale, float yscale, float rotation, float* color, int style, int curor_pos, char cursor_letter)>
	//			(0x4FB980)("Test", 0x7FFFFFFF, test_font, 0.0f, 0.0f, 2.0f, 2.0f, 0.0f, test_color, 0, 0, 0);
	//	}
	//}

	//__declspec(naked) void set2d_test_stub()
	//{
	//	const static uint32_t R_DrawLine_Func = 0x4FD1A0;
	//	const static uint32_t retn_pt = 0x46569C;
	//	__asm
	//	{
	//		pushad;
	//		call	testtest;
	//		popad;
	//		
	//		call	R_DrawLine_Func;
	//		jmp		retn_pt;
	//	}
	//}

	int printf_stub_without_vastart(_In_z_ _Printf_format_string_ char const* const _Format, va_list _ArgList)
	{
		int _Result;
		_Result = _vfprintf_l(stdout, _Format, NULL, _ArgList);
		__crt_va_end(_ArgList);
		return _Result;
	}

	quick_patch::quick_patch()
	{
		// TODO! :: move this or rename quick_patch to main or something
		init_threads();

		radiantapp::main();
		cmainframe::main();
		ccamwnd::main();
		cxywnd::main();

		// redirect console prints
		utils::hook::nop(0x420A54, 10); utils::hook::nop(0x40A9E0, 10);
		utils::hook::set(0x25D5A54, printf_stub_without_vastart);
		// ^
		utils::hook::detour(0x499E90, printf, HK_JUMP);

		
//#define CONSOLE_TEST
#ifdef CONSOLE_TEST
		// disable bottom console (results in small scrollbar at xy top left?)
		utils::hook::set<BYTE>(0x422CD4 + 1, 0x1);

		// disable SetRowInfo for console pane
		utils::hook::nop(0x422F8A, 39);
		// disable SetRowInfo for console pane (LoadRegistryInfo)
		utils::hook::nop(0x423185, 23);
		// disable m_wndSplit.RecalcLayout
		utils::hook::nop(0x42327D, 20);
		// disable setting of g_pEdit
		utils::hook::set<BYTE>(0x422D71, 0xEB);
#endif

		
		// add iw3xradiant search path (imgui images)
		utils::hook(0x4A2452, fs_scan_base_directory_stub, HOOK_JUMP).install()->quick();

		// disable top-most mode for inspector/entity window
		utils::hook::nop(0x496CB6, 13); // clear instructions
		utils::hook::set<BYTE>(0x496CB6, 0xB9); // mov ecx,00000000 (0xB9 00 00 00 00)
		utils::hook::set<DWORD>(0x496CB6 + 1, 0x0); // mov ecx,00000000 (0xB9 00 00 00 00)

		// load raw materials progressbar
		utils::hook::nop(0x45AF5F, 6);
		utils::hook(0x45AF5F, load_raw_materials_progressbar_stub, HOOK_JUMP).install()->quick();

		// silence "Could not connect to source control"
		utils::hook::nop(0x420B59, 5);

		
#ifdef HIDE_MAINFRAME_TOOLBAR
		utils::hook(0x4211C6, show_toolbar_stub, HOOK_JUMP).install()->quick();
#endif

		
#ifdef HIDE_MAINFRAME_MENUBAR
		// -----------------------------------------------------------------------
		// disable mainframe menubar
		
		// create mainframe without menu
		utils::hook::set<BYTE>(0x4507CA + 1, 0x0);

		// cmainframe::OnCreate :: nop CMenu::DestroyMenu call (nullptr exception)
		utils::hook::nop(0x42104A, 5);

		// cmainframe::OnCreate :: make LoadMenuA load a null menu
		utils::hook::set<BYTE>(0x421057 + 1, 0x0);

		// -----------------------------------------------------------------------
#endif

		
#ifdef CREATE_SECOND_CAMERA
		// Create static
		utils::hook(0x4166C6, set_rinitforwindow_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x422984, test_stub, HOOK_JUMP).install()->quick();
#endif
		
		
#ifdef SPLITTER_TEST
		// -----------------------------------------------------------------------------------------------------------------------------
		// this creates a third main splitter below the console and assigns the camwindow (input etc is working but drawing doesnt work)

		// 3 panes for main splitter
		utils::hook::set<BYTE>(0x422CD4 + 1, 0x3);

		// Create static
		utils::hook::nop(0x422D7E, 6);
		utils::hook(0x422D7E, createstatic_stub, HOOK_JUMP).install()->quick();
		
		// set rowinfo
		utils::hook::nop(0x422FC3, 6);
		utils::hook(0x422FC3, setrowinfo_stub, HOOK_JUMP).install()->quick();
		
		// -----------------------------------------------------------------------------------------------------------------------------
#endif


		// hook / Grab CameraWnd object (only when using floating windows) :: (cmainframe::update_windows sets cmainframe::activewnd otherwise)
		utils::hook(0x42270C, ccam_init_stub, HOOK_JUMP).install()->quick();
		
		// disable black world on selecting a brush with sun preview enabled -> no longer able to clone brushes ...
		//utils::hook::set<BYTE>(0x484904, 0xEB);

		// disable black world on selecting a brush with sun preview enabled -> still disables active sun preview .. no black world tho
		utils::hook::nop(0x406A11, 5);

		// NOP startup console-spam
		utils::hook::nop(0x4818DF, 5); // ScanFile
		utils::hook::nop(0x48B8BE, 5); // ScanWeapon

		
		// remove the statusbar (not the console!)
		//utils::hook::nop(0x41F8E0, 5);
		//utils::hook::nop(0x420B04, 63);

		

		// disable text in console
		//utils::hook::nop(0x422D76, 5);

		// no console + textures tab on entity window if split view
		//utils::hook::set<BYTE>(0x4966CA, 0xEB);


		
		//utils::hook::set<BYTE>(0x422D42 + 1, 0x0);
		//utils::hook::set<BYTE>(0x422D58 + 1, 0x0);
		
		// d_hwndEdit == console
		//utils::hook::nop(0x422D71, 10);

		//utils::hook(0x465697, set2d_test_stub, HOOK_JUMP).install()->quick();


		// --------
		// Commands 

		// print dvar values to console
		command::register_command("getdvar"s, [](std::vector < std::string > args)
		{
			// Check if enough arguments have been passed to the command
			if (args.size() == 1)
			{
				game::console_error("usage: getdvar <dvarName>");
				return;
			}

			std::string	  dvarType;
			game::dvar_s* dvar_dummy = game::Dvar_FindVar(args[1].c_str());

			if (dvar_dummy)
			{
				switch (dvar_dummy->type)
				{
				case game::dvar_type::boolean:
					dvarType = "[BOOL] %s->current.enabled = %s\n";
					break;

				case game::dvar_type::value:
					dvarType = "[FLOAT] %s->current.value = %s\n";
					break;

				case game::dvar_type::vec2:
					dvarType = "[VEC2] %s->current.vector = %s\n";
					break;

				case game::dvar_type::vec3:
					dvarType = "[VEC3] %s->current.vector = %s\n";
					break;

				case game::dvar_type::vec4:
					dvarType = "[VEC4] %s->current.vector = %s\n";
					break;

				case game::dvar_type::integer:
					dvarType = "[INT] %s->current.integer = %s\n";
					break;

				case game::dvar_type::enumeration:
					dvarType = "[ENUM] %s->current.integer = " + std::to_string(dvar_dummy->current.integer) + " :: %s\n";
					break;

				case game::dvar_type::string:
					dvarType = "[STRING] %s->current.string = %s\n";
					break;

				case game::dvar_type::color:
					dvarType = "[COLOR] %s->current.vector = %s\n";
					break;

				case game::dvar_type::rgb:
					dvarType = "[RGB] %s->current.vector = %s\n";
					break;

				default:
					dvarType = "[UNKOWN] %s = %s\n";
					break;
				}

				// dvar description
				//dvarType += "|-> %s\n";

				std::string dvar_description;

				if (!dvar_dummy->description)
				{
					dvar_description = "no description";
				}
				else
				{
					dvar_description = dvar_dummy->description;
				}

				printf(utils::va(dvarType.c_str(), dvar_dummy->name, game::Dvar_DisplayableValue(dvar_dummy)));
				printf("|-> %s\n", dvar_description.c_str());
			}

			else
			{
				game::console_error(utils::va("unkown dvar: \"%s\"", args[1].data()));
			}
		});

		// set dvar values via console
		command::register_command("setdvar"s, [](std::vector < std::string > args)
		{
			// Check if enough arguments have been passed to the command
			if (args.size() <= 2)
			{
				game::console_error("usage: setdvar <dvarName> <value/s>");
				return;
			}

			game::dvar_s* dvar_dummy = game::Dvar_FindVar(args[1].c_str());

			// if dvar exists
			if (dvar_dummy)
			{

				std::string dvar_value;

				// do not append a " " if we only have 1 dvarString arg
				if (args.size() == 3)
				{
					dvar_value = args[2];
				}
				else
				{
					// combine all dvar value args
					for (auto argCount = 2; argCount < (int)args.size(); argCount++)
					{
						dvar_value += args[argCount] + " ";
					}
				}

				game::Dvar_SetFromStringFromSource(dvar_value.c_str(), dvar_dummy, 1);
			}
			else
			{
				game::console_error(utils::va("unkown dvar: \"%s\"", args[1].data()));
			}
		});
	}

	quick_patch::~quick_patch()
	{ }
}
