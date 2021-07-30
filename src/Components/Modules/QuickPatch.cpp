#include "STDInclude.hpp"

// tests
//#define SPLITTER_TEST
//#define CREATE_SECOND_CAMERA
//#define HIDE_MAINFRAME_MENUBAR // fully working
//#define HIDE_MAINFRAME_TOOLBAR // fully working

DWORD WINAPI realtimewnd_msg_pump(LPVOID)
{
	const int maxfps = 1000 / 250;

	int sys_timeBase = 0;
	int com_frameTime = 0;
	int com_lastFrameTime = 0;
	
	while(true)
	{
		if(Game::Globals::d3d9_device)
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
			
			com_lastFrameTime = com_frameTime;

			// update cam window ~ 250fps
			if  (const auto hwnd = CMainFrame::ActiveWindow->m_pCamWnd->GetWindow();
				hwnd != nullptr)
			{
				SendMessageA(hwnd, WM_PAINT, 0, 0);
			}

			// update xy window ~ 250fps
			if (const auto hwnd = CMainFrame::ActiveWindow->m_pXYWnd->GetWindow();
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
	CreateThread(nullptr, 0, RemoteNet_SearchServerThread, nullptr, 0, nullptr);

	// Create LiveRadiant thread (receiving commands from the server)
	CreateThread(nullptr, 0, RemoteNet_ReceivePacket_Thread, nullptr, 0, nullptr);


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

	// Command Thread
	//CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(CommandThread), nullptr, 0, nullptr);
	CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Components::Command::CommandThread), nullptr, 0, nullptr);
	return TRUE;
}


namespace Components
{
	__declspec(naked) void CCam_ctor_stub()
	{
		const static uint32_t CCam_ActiveWindow_Func = 0x402C40;
		const static uint32_t retnPt = 0x422711;
		__asm
		{
			Call	CCam_ActiveWindow_Func
			mov		CCamWnd::ActiveWindow, eax
			mov		Game::Globals::radiant_floatingWindows, 1
			jmp		retnPt
		}
	}

#ifdef SPLITTER_TEST
	void setrowinfo()
	{
		typedef void (__thiscall* CSplitterWnd__SetRowInfo_t)(CSplitterWnd*, int row, int, int);
		/* ------------------------- */ const auto test = reinterpret_cast<CSplitterWnd__SetRowInfo_t>(0x5A544B);

		test(&CMainFrame::ActiveWindow->m_wndSplit, 2, 50, 50);
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
		auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(CMainFrame::ActiveWindow->m_wndSplit.__vftable);

		tagSIZE size = tagSIZE(25, 100);
		void* texclass = reinterpret_cast<void*>(0x6D50D8); // cam
		
		vtable->CreateView(&CMainFrame::ActiveWindow->m_wndSplit, 2, 0, (CRuntimeClass*)texclass, size, (CCreateContext*)create_context);

		
		typedef CWnd* (__thiscall* CSplitterWnd__GetPane_t)(CSplitterWnd*, int row, int col);
		/* ------------------------- */ const auto test = reinterpret_cast<CSplitterWnd__GetPane_t>(0x5A5409);

		CCamWnd* window = reinterpret_cast<CCamWnd*>(test(&CMainFrame::ActiveWindow->m_wndSplit, 2, 0));
		CMainFrame::ActiveWindow->m_pCamWnd = window;
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
		if(const auto fs_basepath = Game::Dvar_FindVar("fs_basepath"); 
			fs_basepath)
		{
			Game::FS_ScanForDir("bin/IW3xRadiant", fs_basepath->current.string, false);
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
			mov     eax, Game::Globals::test_hwnd;
			jmp		retn_pt;
		}
	}

	CCamWnd* cam__init(CCamWnd* cam)
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
		auto cam = cam__init(reinterpret_cast<CCamWnd*>(_new));

		tagRECT rect = { 5, 25, 100, 100 };
		
		auto vtable = reinterpret_cast<CWnd_vtbl*>(cam->__vftable);

		//CMainFrame::ActiveWindow->m_pCamWnd = cam;

		vtable->Create(
			cam
			, "QCamera"
			, 0
			, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZEBOX
			, &rect
			,CMainFrame::ActiveWindow
			, 1234u
			, 0);

		//cam->m_hWnd = CMainFrame::ActiveWindow->m_pCamWnd->m_hWnd;
		Game::Globals::test_hwnd = cam->m_hWnd;

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
	
	QuickPatch::QuickPatch()
	{
		init_threads();
		
		CMainFrame::main();
		CCamWnd::main();
		CXYWnd::main();

		Utils::Hook(0x4A2452, fs_scan_base_directory_stub, HOOK_JUMP).install()->quick();

		
#ifdef HIDE_MAINFRAME_TOOLBAR
		Utils::Hook(0x4211C6, show_toolbar_stub, HOOK_JUMP).install()->quick();
#endif

		
#ifdef HIDE_MAINFRAME_MENUBAR
		// -----------------------------------------------------------------------
		// disable mainframe menubar
		
		// create mainframe without menu
		Utils::Hook::Set<BYTE>(0x4507CA + 1, 0x0);

		// CMainFrame::OnCreate :: nop CMenu::DestroyMenu call (nullptr exception)
		Utils::Hook::Nop(0x42104A, 5);

		// CMainFrame::OnCreate :: make LoadMenuA load a null menu
		Utils::Hook::Set<BYTE>(0x421057 + 1, 0x0);

		// -----------------------------------------------------------------------
#endif

		
#ifdef CREATE_SECOND_CAMERA
		// Create static
		Utils::Hook(0x4166C6, set_rinitforwindow_stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x422984, test_stub, HOOK_JUMP).install()->quick();
#endif
		
		
#ifdef SPLITTER_TEST
		// -----------------------------------------------------------------------------------------------------------------------------
		// this creates a third main splitter below the console and assigns the camwindow (input etc is working but drawing doesnt work)

		// 3 panes for main splitter
		Utils::Hook::Set<BYTE>(0x422CD4 + 1, 0x3);

		// Create static
		Utils::Hook::Nop(0x422D7E, 6);
		Utils::Hook(0x422D7E, createstatic_stub, HOOK_JUMP).install()->quick();
		
		// set rowinfo
		Utils::Hook::Nop(0x422FC3, 6);
		Utils::Hook(0x422FC3, setrowinfo_stub, HOOK_JUMP).install()->quick();
		
		// -----------------------------------------------------------------------------------------------------------------------------
#endif


		// Hook / Grab CameraWnd object (only when using floating windows) :: (CMainFrame::UpdateWindows sets CMainFrame::ActiveWindow otherwise)
		Utils::Hook(0x42270C, CCam_ctor_stub, HOOK_JUMP).install()->quick();
		
		// disable black world on selecting a brush with sun preview enabled -> no longer able to clone brushes ...
		//Utils::Hook::Set<BYTE>(0x484904, 0xEB);

		// disable black world on selecting a brush with sun preview enabled -> still disables active sun preview .. no black world tho
		Utils::Hook::Nop(0x406A11, 5);

		// NOP startup console-spam
		//Utils::Hook::Nop(0x4818DF, 5); // ScanFile
		//Utils::Hook::Nop(0x48B8BE, 5); // ScanWeapon
		
		// remove the statusbar (not the console!)
		//Utils::Hook::Nop(0x41F8E0, 5);
		//Utils::Hook::Nop(0x420B04, 63);

		// disable bottom console
		//Utils::Hook::Set<BYTE>(0x422CD4 + 1, 0x1);

		// disable text in console
		//Utils::Hook::Nop(0x422D76, 5);

		// no console + textures tab on entity window if split view
		//Utils::Hook::Set<BYTE>(0x4966CA, 0xEB);


		
		//Utils::Hook::Set<BYTE>(0x422D42 + 1, 0x0);
		//Utils::Hook::Set<BYTE>(0x422D58 + 1, 0x0);
		
		// d_hwndEdit == console
		//Utils::Hook::Nop(0x422D71, 10);


		// --------
		// Commands 

		// print dvar values to console
		Command::RegisterCommand("getdvar"s, [](std::vector < std::string > args)
		{
			// Check if enough arguments have been passed to the command
			if (args.size() == 1)
			{
				Game::ConsoleError("usage: getdvar <dvarName>");
				return;
			}

			std::string dvarType;
			Game::dvar_s* getDvarDummy = Game::Dvar_FindVar(args[1].c_str());

			//Dvars::radiant_livePort
			if (getDvarDummy)
			{
				switch (getDvarDummy->type)
				{
				case Game::dvar_type::boolean:
					dvarType = "[BOOL] %s->current.enabled = %s\n";
					break;

				case Game::dvar_type::value:
					dvarType = "[FLOAT] %s->current.value = %s\n";
					break;

				case Game::dvar_type::vec2:
					dvarType = "[VEC2] %s->current.vector = %s\n";
					break;

				case Game::dvar_type::vec3:
					dvarType = "[VEC3] %s->current.vector = %s\n";
					break;

				case Game::dvar_type::vec4:
					dvarType = "[VEC4] %s->current.vector = %s\n";
					break;

				case Game::dvar_type::integer:
					dvarType = "[INT] %s->current.integer = %s\n";
					break;

				case Game::dvar_type::enumeration:
					dvarType = "[ENUM] %s->current.integer = " + std::to_string(getDvarDummy->current.integer) + " :: %s\n";
					break;

				case Game::dvar_type::string:
					dvarType = "[STRING] %s->current.string = %s\n";
					break;

				case Game::dvar_type::color:
					dvarType = "[COLOR] %s->current.vector = %s\n";
					break;

				case Game::dvar_type::rgb:
					dvarType = "[RGB] %s->current.vector = %s\n";
					break;

				default:
					dvarType = "[UNKOWN] %s = %s\n";
					break;
				}

				// dvar description
				//dvarType += "|-> %s\n";

				std::string dvarDescription;

				if (!getDvarDummy->description)
				{
					dvarDescription = "no description";
				}
				else
				{
					dvarDescription = getDvarDummy->description;
				}

				printf(Utils::VA(dvarType.c_str(), getDvarDummy->name, Game::Dvar_DisplayableValue(getDvarDummy)));
				printf("|-> %s\n", dvarDescription.c_str());
			}

			else
			{
				Game::ConsoleError(Utils::VA("unkown dvar: \"%s\"", args[1].data()));
			}
		});

		// set dvar values via console
		Command::RegisterCommand("setdvar"s, [](std::vector < std::string > args)
		{
			// Check if enough arguments have been passed to the command
			if (args.size() <= 2)
			{
				Game::ConsoleError("usage: setdvar <dvarName> <value/s>");
				return;
			}

			Game::dvar_s* getDvarDummy = Game::Dvar_FindVar(args[1].c_str());

			// if dvar exists
			if (getDvarDummy)
			{

				std::string dvarValue;

				// do not append a " " if we only have 1 dvarString arg
				if (args.size() == 3)
				{
					dvarValue = args[2];
				}

				else
				{
					// combine all dvar value args
					for (auto argCount = 2; argCount < (int)args.size(); argCount++)
					{
						dvarValue += args[argCount] + " ";
					}
				}

				Game::Dvar_SetFromStringFromSource(dvarValue.c_str(), getDvarDummy, 1);
			}
			else
			{
				Game::ConsoleError(Utils::VA("unkown dvar: \"%s\"", args[1].data()));
			}
		});
	}

	QuickPatch::~QuickPatch()
	{ }
}
