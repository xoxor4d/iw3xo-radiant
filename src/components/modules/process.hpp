#pragma once

namespace components
{
	class process : public component
	{
	private:
		bool m_is_running;
		bool m_kill_thread;
		bool m_disable_callback;

		// prefs
		bool m_output_to_console;
		std::string m_arguments;
		std::function<void()> m_callback;

		static DWORD WINAPI t_create_process(LPVOID arguments);


	public:
		void reset(bool clear_callback = false)
		{
			m_arguments.clear();

			if(clear_callback)
			{
				m_callback = nullptr;
			}
			
			m_is_running = false;
			m_kill_thread = false;
			m_disable_callback = false;
			m_output_to_console = false;
		}
		
		[[nodiscard]] bool is_active() const
		{
			return m_is_running;
		}

		// process arguments :: <path_to_file> <file arguments>
		void set_arguments(const std::string& args)
		{
			this->m_arguments = args;
		}

		// enable/disable console output
		void set_output(bool state)
		{
			this->m_output_to_console = state;
		}

		// callback function after process finished
		void set_callback(const std::function<void()>& _callback)
		{
			if (!this->is_active())
			{
				this->m_callback = _callback;
			}
		}

		// kill process
		// <run_callback> run callback after termination
		void kill_process(bool disable_callback = false)
		{
			if (this->is_active())
			{
				m_kill_thread = true;
				m_disable_callback = disable_callback;
			}
		}
		
		void create_process();
		static process* pthis;

		process();
		~process() override;
		const char* get_name() override { return "process"; };
	};
}
