#include "std_include.hpp"

// Create a message pump thread so that we can update certain windows at a constant framerate
DWORD WINAPI realtimewnd_msg_pump(LPVOID)
{
	const int maxfps = 1000 / 250;

	int sys_timeBase = 0;
	int com_frameTime = 0;
	int com_lastFrameTime = 0;

	int quater_update = 0;
	
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

			game::glob::frametime_ms = com_frameTime - com_lastFrameTime;
			com_lastFrameTime = com_frameTime;

			if(quater_update >= 4)
			{
				quater_update = 0;
			}

			quater_update++;


			// update camera window ~ 250fps
			if  (const auto hwnd = cmainframe::activewnd->m_pCamWnd->GetWindow();
				hwnd != nullptr)
			{
				if(game::glob::frametime_ms <= 100)
				{
					game::glob::ccamwindow_realtime = true;
					SendMessageA(hwnd, WM_PAINT, 0, 0);
				}
				else
				{
					game::glob::ccamwindow_realtime = false;
				}
			}

			
			// update grid window ~ 250fps
			if (const auto hwnd = cmainframe::activewnd->m_pXYWnd->GetWindow();
				hwnd != nullptr)
			{
				SendMessageA(hwnd, WM_PAINT, 0, 0);
			}

			// update z window ~ 250fps
			if (const auto hwnd = cmainframe::activewnd->m_pZWnd->GetWindow();
				hwnd != nullptr)
			{
				SendMessageA(hwnd, WM_PAINT, 0, 0);
			}

			//if(quater_update == 3) // update texture window ~ 60fps
			{
				if (const auto hwnd = cmainframe::activewnd->m_pTexWnd->GetWindow();
					hwnd != nullptr)
				{
					SendMessageA(hwnd, WM_PAINT, 0, 0);
				}
			}

			if (const auto hwnd = layermatwnd_struct->m_content_hwnd;
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
	// Create a message pump thread to paint certain windows at a constant and continuous framerate
	CreateThread(nullptr, 0, realtimewnd_msg_pump, nullptr, 0, nullptr);

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
	/*__declspec(naked) void ccam_init_stub()
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
	}*/

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
		ggui::filter::hooks();
		ggui::entity::hooks();
		ggui::modelselector::init();
		ggui::preferences::hooks();

		// add iw3xradiant search path (imgui images)
		utils::hook(0x4A2452, fs_scan_base_directory_stub, HOOK_JUMP).install()->quick();

		// do not load "_glow" fonts (qerfont_glow)
		utils::hook::nop(0x552806, 5);

		// hook / Grab CameraWnd object (only when using floating windows) :: (cmainframe::update_windows sets cmainframe::activewnd otherwise)
		//utils::hook(0x42270C, ccam_init_stub, HOOK_JUMP).install()->quick();

		// disable black world on selecting a brush with sun preview enabled -> still disables active sun preview .. no black world tho
		utils::hook::nop(0x406A11, 5);

		// nop com_math.cpp "det" line:1775 assert (MatrixInverse44)
		utils::hook::nop(0x4A6BC9, 5);

		// set max undos
		//utils::hook::set<int32_t>(0x739F6C, 512);

		// set max undo memory
		utils::hook::set<int32_t>(0x739F70, 0x01000000); // default 2mb, now 16mb
		
		// disable black world on selecting a brush with sun preview enabled -> no longer able to clone brushes ...
		//utils::hook::set<BYTE>(0x484904, 0xEB);
	}

	main_module::~main_module()
	{ }
}
