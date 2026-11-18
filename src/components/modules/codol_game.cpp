#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include <psapi.h>

#ifdef CODOL_STANDALONE
#include "host.hpp"
#endif

namespace components
{
	namespace
	{
		struct pending_result
		{
			std::function<void()> func;
		};

		std::mutex g_result_mutex;
		std::vector<pending_result> g_results;		// filled by worker threads, drained on the gui thread

		std::mutex g_status_mutex;
		std::string g_status;

		codol_game::embed_state g_embed;

		void post_to_gui(std::function<void()> func)
		{
			std::lock_guard lock(g_result_mutex);
			g_results.push_back({ std::move(func) });
		}

		std::string dvar_string(const game::dvar_s* dvar)
		{
			return (dvar && dvar->current.string) ? std::string(dvar->current.string) : std::string();
		}

		bool dir_exists(const std::string& path)
		{
			std::error_code ec;
			return !path.empty() && std::filesystem::is_directory(std::filesystem::u8path(path), ec);
		}

		std::string trim_copy(std::string s)
		{
			const auto b = s.find_first_not_of(" \t\r\n");
			if (b == std::string::npos)
			{
				return {};
			}

			const auto e = s.find_last_not_of(" \t\r\n");
			return s.substr(b, e - b + 1);
		}

		// the server keeps its rcon password in a state file, the same places the in-game console looks at
		std::string find_rcon_password()
		{
			if (auto pw = trim_copy(dvar_string(dvars::codol_rcon_password)); !pw.empty())
			{
				return pw;
			}

			std::vector<std::string> candidates;

			if (auto file = dvar_string(dvars::codol_rcon_password_file); !file.empty())
			{
				candidates.push_back(file);
			}

			if (const auto install = codol_game::install_path(); !install.empty())
			{
				candidates.push_back(install + "\\main\\k33_rcon.txt");
				candidates.push_back(install + "\\p2p\\state\\native_server_rcon.txt");
				candidates.push_back(install + "\\CODOL\\1.1.1.13_server\\alpha0022\\native_server_rcon.txt");
			}

			char env[MAX_PATH] = {};
			if (GetEnvironmentVariableA("CODOL_STATE_DIR", env, sizeof(env)))
			{
				candidates.push_back(std::string(env) + "\\native_server_rcon.txt");
			}

			for (const auto& path : candidates)
			{
				std::string text;
				if (codol::script::read_text_file(path, text))
				{
					if (auto pw = trim_copy(text); !pw.empty())
					{
						return pw;
					}
				}
			}

			return {};
		}

		// blocking: one rcon round trip on the calling thread
		bool rcon_blocking(const std::string& command, std::string& response, std::string& error)
		{
			const auto password = find_rcon_password();
			if (password.empty())
			{
				error = "no rcon password: set codol_rcon_password or codol_rcon_password_file";
				return false;
			}

			const SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (sock == INVALID_SOCKET)
			{
				error = "could not create a socket";
				return false;
			}

			sockaddr_in addr = {};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(static_cast<u_short>(dvars::codol_rcon_port ? dvars::codol_rcon_port->current.integer : 17733));
			addr.sin_addr.s_addr = inet_addr("127.0.0.1");

			const std::string packet = "\xff\xff\xff\xffrcon \"" + password + "\" " + command;
			sendto(sock, packet.data(), static_cast<int>(packet.size()), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

			// the answer can be split over several "print" packets, collect until the server goes quiet
			DWORD timeout = 1200;
			bool answered = false;
			response.clear();

			for (int i = 0; i < 64; i++)
			{
				setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

				char buf[8192];
				const int len = recv(sock, buf, sizeof(buf), 0);
				if (len <= 0)
				{
					break;
				}

				answered = true;
				timeout = 250;

				std::string_view part(buf, static_cast<std::size_t>(len));
				if (part.starts_with("\xff\xff\xff\xff"))
				{
					part.remove_prefix(4);
				}

				if (part.starts_with("print\n"))
				{
					part.remove_prefix(6);
				}

				response.append(part);
			}

			closesocket(sock);

			if (!answered)
			{
				error = "no answer from 127.0.0.1:" + std::to_string(ntohs(addr.sin_port)) + " ( is the local server running? )";
			}

			return answered;
		}

		// run a command line, capture stdout + stderr, return the exit code ( -1: could not start )
		int run_command(const std::string& command_line, const std::string& working_dir, std::string& output)
		{
			SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
			HANDLE read_pipe = nullptr, write_pipe = nullptr;

			if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0))
			{
				return -1;
			}

			SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);

			STARTUPINFOA si = {};
			si.cb = sizeof(si);
			si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			si.hStdOutput = write_pipe;
			si.hStdError = write_pipe;
			si.wShowWindow = SW_HIDE;

			PROCESS_INFORMATION pi = {};
			std::string mutable_cmd = "cmd.exe /c \"" + command_line + "\"";

			const BOOL ok = CreateProcessA(nullptr, mutable_cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
				working_dir.empty() ? nullptr : working_dir.c_str(), &si, &pi);

			CloseHandle(write_pipe);

			if (!ok)
			{
				CloseHandle(read_pipe);
				return -1;
			}

			char buf[4096];
			DWORD read = 0;
			while (ReadFile(read_pipe, buf, sizeof(buf), &read, nullptr) && read)
			{
				output.append(buf, read);
			}

			WaitForSingleObject(pi.hProcess, INFINITE);

			DWORD exit_code = 0;
			GetExitCodeProcess(pi.hProcess, &exit_code);

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(read_pipe);

			return static_cast<int>(exit_code);
		}

		// "mapname" is: "mp_killhouse_sh^7" default: ...
		std::string parse_dvar_answer(const std::string& response)
		{
			const auto is = response.find("is:");
			if (is == std::string::npos)
			{
				return {};
			}

			const auto q1 = response.find('"', is);
			const auto q2 = (q1 == std::string::npos) ? q1 : response.find('"', q1 + 1);
			if (q2 == std::string::npos)
			{
				return {};
			}

			auto value = response.substr(q1 + 1, q2 - q1 - 1);
			if (const auto caret = value.find('^'); caret != std::string::npos)
			{
				value.erase(caret);
			}

			return value;
		}

		std::string process_name_of_window(HWND hwnd)
		{
			DWORD pid = 0;
			GetWindowThreadProcessId(hwnd, &pid);
			if (!pid || pid == GetCurrentProcessId())
			{
				return {};
			}

			std::string name;
			if (const HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid))
			{
				char path[MAX_PATH] = {};
				DWORD size = sizeof(path);
				if (QueryFullProcessImageNameA(proc, 0, path, &size))
				{
					name = path;
					if (const auto slash = name.find_last_of("\\/"); slash != std::string::npos)
					{
						name.erase(0, slash + 1);
					}
				}

				CloseHandle(proc);
			}

			return name;
		}

		struct enum_ctx
		{
			std::vector<std::string> names;
			HWND best = nullptr;
			long best_area = 0;
		};

		BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM lparam)
		{
			auto* ctx = reinterpret_cast<enum_ctx*>(lparam);

			if (!IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER))
			{
				return TRUE;
			}

			const auto name = codol::script::to_lower(process_name_of_window(hwnd));
			if (name.empty() || std::find(ctx->names.begin(), ctx->names.end(), name) == ctx->names.end())
			{
				return TRUE;
			}

			RECT rc = {};
			GetWindowRect(hwnd, &rc);
			const long area = (rc.right - rc.left) * (rc.bottom - rc.top);

			if (area > ctx->best_area)
			{
				ctx->best_area = area;
				ctx->best = hwnd;
			}

			return TRUE;
		}
	}

	void codol_game::post_to_gui(std::function<void()> func)
	{
		components::post_to_gui(std::move(func));
	}

	int codol_game::run_command(const std::string& command_line, const std::string& working_dir, std::string& output)
	{
		return components::run_command(command_line, working_dir, output);
	}

	// -------------------------------------------------------------------------------------
	// paths

	std::string codol_game::install_path()
	{
		auto path = dvar_string(dvars::codol_install_path);
		while (!path.empty() && (path.back() == '\\' || path.back() == '/'))
		{
			path.pop_back();
		}

		return path;
	}

	std::vector<std::string> codol_game::script_roots()
	{
		std::vector<std::string> roots;

		const auto add = [&](std::string path)
		{
			while (!path.empty() && (path.back() == '\\' || path.back() == '/'))
			{
				path.pop_back();
			}

			if (dir_exists(path) && std::find(roots.begin(), roots.end(), path) == roots.end())
			{
				roots.push_back(path);
			}
		};

		add(dvar_string(dvars::codol_script_root));
		add(dvar_string(dvars::codol_script_ref_root));

		if (roots.empty())
		{
			// same variable the server launcher honours
			char env[MAX_PATH] = {};
			if (GetEnvironmentVariableA("CODOL_SCRIPTS", env, sizeof(env)))
			{
				add(env);
			}

			// fall back to the raw folder of the radiant project
			if (roots.empty())
			{
				add(utils::fs::get_basepath() + "\\raw");
			}
		}

		return roots;
	}

	// -------------------------------------------------------------------------------------
	// status

	std::string codol_game::status()
	{
		std::lock_guard lock(g_status_mutex);
		return g_status;
	}

	void codol_game::set_status(const std::string& text)
	{
		std::lock_guard lock(g_status_mutex);
		g_status = text;
	}

	// -------------------------------------------------------------------------------------
	// rcon

	void codol_game::rcon(const std::string& command, const rcon_callback& callback)
	{
		std::thread([command, callback]
		{
			std::string response, error;
			const bool ok = rcon_blocking(command, response, error);

			post_to_gui([=]
			{
				if (!ok)
				{
					game::printf_to_console("^1[CoDOL] rcon '%s' failed: %s\n", command.c_str(), error.c_str());
					set_status("rcon failed: " + error);
				}
				else if (dvars::codol_rcon_log && dvars::codol_rcon_log->current.enabled && !response.empty())
				{
					game::printf_to_console("[CoDOL] rcon '%s':\n%s\n", command.c_str(), response.c_str());
				}

				if (callback)
				{
					callback(ok, ok ? response : error);
				}
			});

		}).detach();
	}

	void codol_game::load_map(const std::string& map_name)
	{
		set_status("loading map " + map_name + " ...");
		rcon("map " + map_name, [map_name](const bool ok, const std::string&)
		{
			if (ok)
			{
				set_status("server is loading " + map_name);
				ImGui::Toast(ImGuiToastType_Success, "CoDOL", ("Server is loading " + map_name).c_str());
			}
		});
	}

	void codol_game::reload_scripts(const std::string& reason)
	{
		const int mode = dvars::codol_reload_mode ? dvars::codol_reload_mode->current.integer : RELOAD_MAP;
		const auto precheck = trim_copy(dvar_string(dvars::codol_reload_precheck_cmd));
		const auto precheck_cwd = dvar_string(dvars::codol_reload_precheck_cwd);

		set_status("reloading ( " + reason + " ) ...");

		std::thread([mode, precheck, precheck_cwd, reason]
		{
			// 1. optional pre-check ( e.g. a script resolver ): a broken script makes the map load fail
			//    and takes the server down with it, so do not reload if the check fails
			if (!precheck.empty())
			{
				std::string output;
				const int code = run_command(precheck, precheck_cwd, output);

				if (code != 0)
				{
					post_to_gui([=]
					{
						game::printf_to_console("^1[CoDOL] pre-check failed ( exit code %d ), not reloading:\n%s\n", code, output.c_str());
						set_status("pre-check failed, not reloaded - see console");
						ImGui::Toast(ImGuiToastType_Error, "CoDOL", "Script pre-check failed, the game was not reloaded.");
					});

					return;
				}
			}

			// 2. build the command
			std::string command, response, error;

			switch (mode)
			{
			case RELOAD_FAST_RESTART:
				command = "fast_restart";
				break;

			case RELOAD_MAP_RESTART:
				command = "map_restart";
				break;

			default:
			{
				if (!rcon_blocking("mapname", response, error))
				{
					post_to_gui([=]
					{
						game::printf_to_console("^1[CoDOL] reload failed: %s\n", error.c_str());
						set_status("reload failed: " + error);
						ImGui::Toast(ImGuiToastType_Error, "CoDOL", ("Reload failed: " + error).c_str());
					});

					return;
				}

				const auto map = parse_dvar_answer(response);
				if (map.empty())
				{
					post_to_gui([=]
					{
						game::printf_to_console("^1[CoDOL] reload failed: the server has no map loaded ( answer: %s )\n", response.c_str());
						set_status("reload failed: no map loaded on the server");
					});

					return;
				}

				command = "map " + map;
				break;
			}
			}

			// 3. send it
			response.clear();
			const bool ok = rcon_blocking(command, response, error);

			post_to_gui([=]
			{
				if (ok)
				{
					game::printf_to_console("[CoDOL] reloaded ( %s ): %s\n", reason.c_str(), command.c_str());
					set_status("sent '" + command + "' ( " + reason + " )");
					ImGui::Toast(ImGuiToastType_Success, "CoDOL", ("Game reloading: " + command).c_str());
				}
				else
				{
					game::printf_to_console("^1[CoDOL] reload failed: %s\n", error.c_str());
					set_status("reload failed: " + error);
					ImGui::Toast(ImGuiToastType_Error, "CoDOL", ("Reload failed: " + error).c_str());
				}
			});

		}).detach();
	}

	// -------------------------------------------------------------------------------------
	// game window embedding

	HWND codol_game::find_game_window()
	{
		enum_ctx ctx;

		for (const auto& name : utils::split(dvar_string(dvars::codol_client_exe), ';'))
		{
			if (auto n = trim_copy(codol::script::to_lower(name)); !n.empty())
			{
				ctx.names.push_back(n);
			}
		}

		if (ctx.names.empty())
		{
			return nullptr;
		}

		EnumWindows(enum_windows_proc, reinterpret_cast<LPARAM>(&ctx));
		return ctx.best;
	}

	bool codol_game::is_embedded()
	{
		if (g_embed.game && !IsWindow(g_embed.game))
		{
			// the game was closed while embedded
			if (g_embed.parent && IsWindow(g_embed.parent))
			{
				SetWindowLongPtrW(g_embed.parent, GWL_STYLE, g_embed.old_parent_style);
			}

			g_embed = {};
		}

		return g_embed.game != nullptr;
	}

	bool codol_game::attach_game_window(HWND parent)
	{
		if (is_embedded())
		{
			return true;
		}

		const HWND game_hwnd = find_game_window();
		if (!game_hwnd || !parent)
		{
			return false;
		}

		g_embed = {};
		g_embed.game = game_hwnd;
		g_embed.parent = parent;
		g_embed.old_style = GetWindowLongPtrW(game_hwnd, GWL_STYLE);
		g_embed.old_exstyle = GetWindowLongPtrW(game_hwnd, GWL_EXSTYLE);
		g_embed.old_parent_style = GetWindowLongPtrW(parent, GWL_STYLE);
		GetWindowRect(game_hwnd, &g_embed.old_rect);

		// radiant must not paint over the child
		SetWindowLongPtrW(parent, GWL_STYLE, g_embed.old_parent_style | WS_CLIPCHILDREN);

		LONG_PTR style = g_embed.old_style;
		style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
		style |= WS_CHILD;

		SetWindowLongPtrW(game_hwnd, GWL_STYLE, style);
		SetWindowLongPtrW(game_hwnd, GWL_EXSTYLE, g_embed.old_exstyle & ~(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME));
		SetParent(game_hwnd, parent);

		g_embed.visible = true;
		game::printf_to_console("[CoDOL] embedded game window ( %s )\n", process_name_of_window(game_hwnd).c_str());
		return true;
	}

	void codol_game::detach_game_window()
	{
		if (!g_embed.game)
		{
			return;
		}

		if (IsWindow(g_embed.game))
		{
			SetParent(g_embed.game, nullptr);
			SetWindowLongPtrW(g_embed.game, GWL_STYLE, g_embed.old_style);
			SetWindowLongPtrW(g_embed.game, GWL_EXSTYLE, g_embed.old_exstyle);

			SetWindowPos(g_embed.game, HWND_TOP, g_embed.old_rect.left, g_embed.old_rect.top,
				g_embed.old_rect.right - g_embed.old_rect.left, g_embed.old_rect.bottom - g_embed.old_rect.top,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOACTIVATE);
		}

		if (g_embed.parent && IsWindow(g_embed.parent))
		{
			SetWindowLongPtrW(g_embed.parent, GWL_STYLE, g_embed.old_parent_style);
		}

		g_embed = {};
	}

	void codol_game::place_game_window(const int x, const int y, const int w, const int h, const bool visible)
	{
		if (!is_embedded())
		{
			return;
		}

		if (!visible || w <= 0 || h <= 0)
		{
			if (g_embed.visible)
			{
				ShowWindowAsync(g_embed.game, SW_HIDE);
				g_embed.visible = false;
			}

			return;
		}

		const RECT rc = { x, y, x + w, y + h };
		const bool moved = memcmp(&rc, &g_embed.last_rect, sizeof(RECT)) != 0;

		if (moved || !g_embed.visible)
		{
			// async flags: never block radiant on the message queue of the game
			SetWindowPos(g_embed.game, HWND_TOP, x, y, w, h,
				SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS | SWP_FRAMECHANGED | (g_embed.visible ? 0 : SWP_SHOWWINDOW));

			g_embed.last_rect = rc;
			g_embed.visible = true;
		}
	}

	bool codol_game::launch_game()
	{
		const auto command = trim_copy(dvar_string(dvars::codol_launch_cmd));
		if (command.empty())
		{
			game::printf_to_console("^1[CoDOL] codol_launch_cmd is not set\n");
			return false;
		}

		std::string cwd = install_path();

		// a path to a file: run it from its own folder ( the launch scripts expect that )
		std::error_code ec;
		if (std::filesystem::is_regular_file(std::filesystem::u8path(command), ec))
		{
			if (const auto slash = command.find_last_of("\\/"); slash != std::string::npos)
			{
				cwd = command.substr(0, slash);
			}
		}

		STARTUPINFOA si = {};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi = {};
		std::string mutable_cmd = "cmd.exe /c \"" + command + "\"";

		if (!CreateProcessA(nullptr, mutable_cmd.data(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr,
			cwd.empty() ? nullptr : cwd.c_str(), &si, &pi))
		{
			game::printf_to_console("^1[CoDOL] could not start '%s' ( error %lu )\n", command.c_str(), GetLastError());
			return false;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		game::printf_to_console("[CoDOL] started: %s\n", command.c_str());
		set_status("started the game client");
		return true;
	}

	// -------------------------------------------------------------------------------------

	void codol_game::dev_capture_frame()
	{
		static int state = 0; // 0: unchecked, 1: active, 2: off
		static std::string path;
		static double next = 0.0;

		if (state == 0)
		{
			char env[MAX_PATH] = {};
			state = GetEnvironmentVariableA("CODOL_AUTOSHOT", env, sizeof(env)) ? 1 : 2;
			path = env;
		}

		if (state != 1 || ImGui::GetTime() < next || !game::glob::d3d9_device)
		{
			return;
		}

		next = ImGui::GetTime() + 2.0;

#ifdef CODOL_STANDALONE
		// write to a temp name first so a reader never sees a half written file
		const std::string tmp = path + ".tmp";
		if (editor::save_backbuffer_png(tmp))
		{
			MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
		}
#else
		IDirect3DSurface9* target = nullptr;
		if (SUCCEEDED(game::glob::d3d9_device->GetRenderTarget(0, &target)) && target)
		{
			// write to a temp name first so a reader never sees a half written file
			const std::string tmp = path + ".tmp";
			if (SUCCEEDED(D3DXSaveSurfaceToFileA(tmp.c_str(), D3DXIFF_PNG, target, nullptr, nullptr)))
			{
				MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
			}

			target->Release();
		}
#endif
	}

	void codol_game::frame()
	{
		// CODOL_STARTUP_CMDS: run console commands once the gui has settled.
		// "wait <seconds>" pauses the list ( imports run in the background )
		static bool startup_read = false;
		static std::vector<std::string> startup_cmds;
		static std::size_t startup_next = 0;
		static double startup_resume = 0.0;

		if (!startup_read && ImGui::GetFrameCount() > 60)
		{
			startup_read = true;

			// a list can be long ( a whole test run ): the limit of an environment variable
			static char env[32767] = {};
			if (GetEnvironmentVariableA("CODOL_STARTUP_CMDS", env, sizeof(env)))
			{
				for (const auto& cmd : utils::split(std::string(env), ';'))
				{
					if (auto c = trim_copy(cmd); !c.empty())
					{
						startup_cmds.push_back(c);
					}
				}
			}
		}

		while (startup_next < startup_cmds.size() && ImGui::GetTime() >= startup_resume)
		{
			const auto& c = startup_cmds[startup_next++];

			// words split on spaces, "quoted words" kept together ( ed_key angles "60 200 0" )
			std::vector<std::string> args;
			{
				std::string word;
				bool quoted = false, have = false;

				for (const char ch : c)
				{
					if (ch == '"')
					{
						quoted = !quoted;
						have = true;
					}
					else if (ch == ' ' && !quoted)
					{
						if (have || !word.empty())
						{
							args.push_back(word);
						}

						word.clear();
						have = false;
					}
					else
					{
						word.push_back(ch);
					}
				}

				if (have || !word.empty())
				{
					args.push_back(word);
				}
			}

			if (args.empty())
			{
				continue;
			}

			if (args[0] == "wait")
			{
				startup_resume = ImGui::GetTime() + (args.size() > 1 ? utils::try_stof(args[1], true) : 1.0f);
				break;
			}

			game::printf_to_console("[CoDOL] startup command: %s\n", c.c_str());
			command::execute_command(args);
		}

		std::vector<pending_result> results;
		{
			std::lock_guard lock(g_result_mutex);
			results.swap(g_results);
		}

		for (const auto& result : results)
		{
			result.func();
		}
	}

	void codol_game::register_dvars()
	{
		dvars::codol_install_path = dvars::register_string(
			/* name		*/ "codol_install_path",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "CoDOL 1.1.1.13 install folder ( contains main\\ and client_shipRetail\\ )");

		dvars::codol_script_root = dvars::register_string(
			/* name		*/ "codol_script_root",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "script tree the workbench edits ( the folder the server loads loose scripts from, eg. codol_rawfiles )");

		dvars::codol_script_ref_root = dvars::register_string(
			/* name		*/ "codol_script_ref_root",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "optional second script tree ( eg. rawfiles exported from the zones ). Files of the first root shadow it, animtrees are merged");

		dvars::codol_rcon_port = dvars::register_int(
			/* name		*/ "codol_rcon_port",
			/* default	*/ 17733,
			/* mins		*/ 1,
			/* maxs		*/ 65535,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "udp port of the local CoDOL dedicated server");

		dvars::codol_rcon_password = dvars::register_string(
			/* name		*/ "codol_rcon_password",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "rcon password ( not saved ). Leave empty to read it from codol_rcon_password_file / the state files of the install");

		dvars::codol_rcon_password_file = dvars::register_string(
			/* name		*/ "codol_rcon_password_file",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "file that holds the rcon password of the local server ( native_server_rcon.txt )");

		dvars::codol_rcon_log = dvars::register_bool(
			/* name		*/ "codol_rcon_log",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "print rcon answers to the console");

		dvars::codol_reload_on_save = dvars::register_bool(
			/* name		*/ "codol_reload_on_save",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "reload the running level when a script is saved in the script workbench");

		dvars::codol_reload_mode = dvars::register_int(
			/* name		*/ "codol_reload_mode",
			/* default	*/ RELOAD_MAP,
			/* mins		*/ 0,
			/* maxs		*/ 2,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "how scripts are reloaded: 0 = fast_restart, 1 = map_restart, 2 = map <current map> ( recompiles scripts )");

		dvars::codol_reload_precheck_cmd = dvars::register_string(
			/* name		*/ "codol_reload_precheck_cmd",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "optional command that has to exit with 0 before the game is reloaded ( eg. a script resolver )");

		dvars::codol_reload_precheck_cwd = dvars::register_string(
			/* name		*/ "codol_reload_precheck_cwd",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "working directory of codol_reload_precheck_cmd");

		dvars::codol_asset_tool = dvars::register_string(
			/* name		*/ "codol_asset_tool",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "path to CodolSoundDumper.exe ( CODOL-AssetExplorer ), used to decrypt map zones when importing their entities");

		dvars::codol_xanim_list = dvars::register_string(
			/* name		*/ "codol_xanim_list",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "optional text file with the xanims the game ships ( one name per line, eg. docs/zone_assets/ALL_xanim.txt ). Scripts that use other animations get a warning");

		dvars::codol_client_exe = dvars::register_string(
			/* name		*/ "codol_client_exe",
			/* default	*/ "codoMP_client_shipRetail.exe",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "process names of the game client ( ; separated ), used to find its window for the Game view");

		dvars::codol_launch_cmd = dvars::register_string(
			/* name		*/ "codol_launch_cmd",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "command or script that starts the game client ( eg. OFFLINE-DEV.cmd )");
	}

	codol_game::codol_game()
	{
		command::register_command("codol_rcon"s, [](const std::vector<std::string>& args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: codol_rcon <command>\n");
				return;
			}

			std::string cmd;
			for (std::size_t i = 1; i < args.size(); i++)
			{
				cmd += (i > 1 ? " " : "") + args[i];
			}

			rcon(cmd, [](const bool ok, const std::string& response)
			{
				if (ok && !(dvars::codol_rcon_log && dvars::codol_rcon_log->current.enabled))
				{
					game::printf_to_console("%s\n", response.c_str());
				}
			});
		});

		command::register_command("codol_workbench"s, [](const std::vector<std::string>&)
		{
			GET_GUI(ggui::script_workbench_dialog)->toggle();
		});

		command::register_command("codol_gameview"s, [](const std::vector<std::string>&)
		{
			GET_GUI(ggui::game_view_dialog)->toggle();
		});

		// codol_open <file> [line]
		command::register_command("codol_open"s, [](const std::vector<std::string>& args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: codol_open <file> [line]\n");
				return;
			}

			GET_GUI(ggui::script_workbench_dialog)->open_file(args[1], args.size() > 2 ? utils::try_stoi(args[2], true) : 0);
		});

		// move the window of the running game into the Game dock / give it back
		command::register_command("codol_attach"s, [](const std::vector<std::string>&)
		{
			GET_GUI(ggui::game_view_dialog)->open();

			const auto* viewport = ImGui::GetMainViewport();
			const auto parent = static_cast<HWND>(viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw : viewport->PlatformHandle);

			if (!attach_game_window(parent))
			{
				game::printf_to_console("^1[CoDOL] no running game window found ( codol_client_exe: %s )\n", dvar_string(dvars::codol_client_exe).c_str());
			}
		});

		command::register_command("codol_detach"s, [](const std::vector<std::string>&)
		{
			detach_game_window();
		});

		// codol_viewport [map]: open the native viewport, optionally loading a map zone
		command::register_command("codol_viewport"s, [](const std::vector<std::string>& args)
		{
			const auto dlg = GET_GUI(ggui::codol_viewport_dialog);
			dlg->open();

			if (args.size() > 1)
			{
				dlg->load_map(args[1]);
			}
		});

		command::register_command("codol_mapents"s, [](const std::vector<std::string>&)
		{
			GET_GUI(ggui::codol_map_ents_dialog)->toggle();
		});

		// codol_import_ents <file> [map name]
		command::register_command("codol_import_ents"s, [](const std::vector<std::string>& args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: codol_import_ents <.ff | .zone | .d3dbsp.ents | entity text> [map name]\n");
				return;
			}

			std::string map = args.size() > 2 ? args[2] : args[1];
			if (args.size() <= 2)
			{
				if (const auto slash = map.find_last_of("\\/"); slash != std::string::npos)
				{
					map.erase(0, slash + 1);
				}

				if (const auto dot = map.find('.'); dot != std::string::npos)
				{
					map.erase(dot);
				}
			}

			GET_GUI(ggui::codol_map_ents_dialog)->import_file(args[1], map);
		});

		// codol_export_ents [noreload]
		command::register_command("codol_export_ents"s, [](const std::vector<std::string>& args)
		{
			const auto dlg = GET_GUI(ggui::codol_map_ents_dialog);
			const bool keep = dlg->m_reload_after_export;

			dlg->m_reload_after_export = !(args.size() > 1 && args[1] == "noreload");
			dlg->export_current_map();
			dlg->m_reload_after_export = keep;
		});

		command::register_command("codol_reload"s, [](const std::vector<std::string>&)
		{
			reload_scripts("console");
		});

		command::register_command("codol_map"s, [](const std::vector<std::string>& args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: codol_map <mapname>\n");
				return;
			}

			load_map(args[1]);
		});
	}

	codol_game::~codol_game()
	{
		// hand the game window back, otherwise it would be destroyed together with radiant
		detach_game_window();
	}
}
