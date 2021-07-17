#include "STDInclude.hpp"
#include "Utils/vector.hpp"

std::map<std::string, std::function<void(std::vector<std::string>)>> commands;

void RegisterCommand(const std::string &name, std::function<void(std::vector<std::string>)> cb)
{
	commands[name] = cb;
}

void ExecuteCommand(std::vector<std::string> args)
{
	if (commands.find(args[0]) != commands.end()) 
	{
		commands[args[0]](args);
	}
	else 
	{
		Game::ConsoleError(Utils::VA("Unknown command \"%s\"", args[0].data()));
	}
}

void CommandThread()
{
	while (true)
	{
		std::string input;
		std::getline(std::cin, input);

		std::vector<std::string> args;

		if (input.find(' ') != std::string::npos) 
		{
			args = Utils::split(input, ' ');
		}
		else 
		{
			args.push_back(input);
		}

		ExecuteCommand(args);
	}
}

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

BOOL Init()
{
	// Hook / Grab CameraWnd object (only when using floating windows) :: (CMainFrame::UpdateWindows sets CMainFrame::ActiveWindow otherwise)
	Utils::Hook(0x42270C, CCam_ctor_stub, HOOK_JUMP).install()->quick();

	// Hook MainFrameWnd continuous thread
	Utils::Hook(0x421A90, (PBYTE)&CMainFrame::hk_RoutineProcessing, HOOK_JUMP).install()->quick();

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
	CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(CommandThread), nullptr, 0, nullptr);

	return TRUE;
}

namespace Components
{
	QuickPatch::QuickPatch()
	{
		Init();

		// NOP startup console-spam
		Utils::Hook::Nop(0x4818DF, 5); // ScanFile
		Utils::Hook::Nop(0x48B8BE, 5); // ScanWeapon

		// disable black world on selecting a brush with sun preview enabled -> no longer able to clone brushes ...
		//Utils::Hook::Set<BYTE>(0x484904, 0xEB);

		// disable black world on selecting a brush with sun preview enabled -> still disables active sun preview .. no black world tho
		Utils::Hook::Nop(0x406A11, 5);

		// --------
		// Commands 

		// print dvar values to console
		RegisterCommand("getdvar"s, [](std::vector < std::string > args)
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
				dvarType += "|-> %s\n";

				std::string dvarDescription;

				if (!getDvarDummy->description)
				{
					dvarDescription = "no description";
				}
				else
				{
					dvarDescription = getDvarDummy->description;
				}

				printf(Utils::VA(dvarType.c_str(), getDvarDummy->name, Game::Dvar_DisplayableValue(getDvarDummy), dvarDescription.c_str()));
			}

			else
			{
				Game::ConsoleError(Utils::VA("unkown dvar: \"%s\"", args[1].data()));
			}
		});

		// set dvar values via console
		RegisterCommand("setdvar"s, [](std::vector < std::string > args)
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
