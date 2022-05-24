#include "std_include.hpp"

namespace components
{
	process* process::pthis = nullptr;

	// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
	DWORD WINAPI process::t_create_process(LPVOID)
	{
		if(pthis->m_arguments.empty())
		{
			game::printf_to_console("^1[PROCESS] Tried to spawn a process with no arguments!");
			return true;
		}
		
		HANDLE hPipeRead, hPipeWrite;

		SECURITY_ATTRIBUTES sa_attr = { sizeof(SECURITY_ATTRIBUTES) };
		sa_attr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
		sa_attr.lpSecurityDescriptor = nullptr;

		// Create a pipe to get results from child's stdout.
		if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa_attr, 0))
		{
			game::printf_to_console("^1[PROCESS] Failed to create a pipe!");
			return true;
		}

		STARTUPINFOA si = { sizeof(STARTUPINFOA) };
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.hStdOutput = hPipeWrite;
		si.hStdError = hPipeWrite;
		si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
								  // Requires STARTF_USESHOWWINDOW in dwFlags.

		PROCESS_INFORMATION pi = { nullptr };

		const auto result = CreateProcessA(nullptr, (char*)pthis->m_arguments.c_str(), nullptr, nullptr, TRUE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
		if (!result)
		{
			const auto err = GetLastError();
			std::string err_msg = "Error Code: " + std::to_string(err) + " ";
			switch(err)
			{
			case 2:
				err_msg += "(The system cannot find the file specified)";
				break;

			case 3:
				err_msg += "(The system cannot find the path specified)";
				break;

			case 4:
				err_msg += "(The system cannot open the file)";
				break;

			default:
				break;
				
			}

			game::printf_to_console("^1[PROCESS] Failed to spawn the process! %s", err_msg.c_str());
			game::printf_to_console("|> arguments: %s", pthis->m_arguments.c_str());
			CloseHandle(hPipeWrite);
			CloseHandle(hPipeRead);
			return true;
		}
		
		pthis->m_is_running = true;
		bool process_ended = false;

		game::printf_to_console("^2[PROCESS] Starting process.");

		const auto console = GET_GUI(ggui::console_dialog);

		while (!process_ended && !pthis->m_kill_thread)
		{
			// Give some timeslice (50 ms), so we won't waste 100% CPU.
			process_ended = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

			// Even if process exited - we continue reading, if there is some data available over pipe.
			while (true)
			{
				char buf[2048];
				DWORD bytes_read = 0;
				DWORD bytes_avail = 0;

				if (!::PeekNamedPipe(hPipeRead, nullptr, 0, nullptr, &bytes_avail, nullptr))
				{
					break;
				}

				if (!bytes_avail)
				{
					break; // No data available, return
				}

				if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, bytes_avail), &bytes_read, nullptr) || !bytes_read)
				{
					break; // Error, the child process might ended
				}

				buf[bytes_read] = 0;

				if(pthis->m_output_to_console)
				{
					if (console)
					{
						console->addline_no_format(buf);
					}
				}
			}
		}

		CloseHandle(hPipeWrite);
		CloseHandle(hPipeRead);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		if(!pthis->m_disable_callback)
		{
			// execute callback on main thread
			exec::on_gui_once([]
			{
				process::pthis->m_callback();
				process::pthis->m_callback = nullptr;
			});
		}

		process::pthis->reset();

		game::printf_to_console(pthis->m_kill_thread ? "^1[PROCESS] Process was killed" : "^2[PROCESS] Process ended successfully!");
		return true;
	}

	void process::create_process()
	{
		if (!this->is_active())
		{
			CreateThread(nullptr, 0, process::t_create_process, nullptr, 0, nullptr);
		}
		else
		{
			game::printf_to_console("[ERR][PROCESS] Tried to spawn a process while one is already running!");
		}
	}

	process::process()
	{
		process::pthis = this;

		this->m_is_running = false;
		this->m_kill_thread = false;
		this->m_disable_callback = false;
		this->m_callback = nullptr;
		this->m_output_to_console = false;
	}

	process::~process()
	{
		this->kill_process();
	}
}
