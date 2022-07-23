#pragma once

namespace components
{
	class process : public component
	{
	public:
		enum PROC_TYPE : int
		{
			PROC_TYPE_GENERIC = 0,
			PROC_TYPE_BATCH = 1,
		};

		enum INDICATOR_TYPE : int
		{
			INDICATOR_TYPE_SPINNER = 0,
			INDICATOR_TYPE_PROGRESS = 1,
		};
		
	private:
		bool m_is_running;
		bool m_kill_thread;
		bool m_disable_callback;

		// prefs
		bool m_output_to_console;

		PROC_TYPE m_proc_type;
		INDICATOR_TYPE m_indicator_type;

		std::string m_arguments;

		bool m_thread_callback_finished;
		std::function<void()> m_thread_callback;

		bool m_post_process_callback_finished;
		std::function<void()> m_post_process_callback;

		std::function<void()> m_progress_callback;

		static void create_batch_process();
		static void create_generic_process();

	public:
		static process* pthis;
		static process* get() { return pthis; }

		std::string m_indicator_str;
		float m_indicator_progress;

		static void run_thread_callback();

		void reset(bool clear_callback = false)
		{
			m_arguments.clear();

			m_proc_type = PROC_TYPE_GENERIC;
			m_indicator_type = INDICATOR_TYPE_SPINNER;
			m_indicator_progress = 0.0f;
			m_indicator_str = "";

			if (clear_callback)
			{
				m_post_process_callback = nullptr;
			}

			m_thread_callback_finished = false;
			m_thread_callback = nullptr;

			m_post_process_callback_finished = false;

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

		// type of process
		void set_process_type(PROC_TYPE type)
		{
			this->m_proc_type = type;
		}


		// type of indicator displayed within the menubar
		void set_indicator(INDICATOR_TYPE type)
		{
			this->m_indicator_type = type;
		}

		// returns the indicator type
		[[nodiscard]] int get_indicator_type() const
		{
			return this->m_indicator_type;
		}


		// text displayed within the menubar
		void set_indicator_string(const std::string& str)
		{
			this->m_indicator_str = str;
		}

		// batch: enable/disable console output
		void set_output(bool state)
		{
			this->m_output_to_console = state;
		}

		// generic: main process thread callback
		void set_thread_callback(const std::function<void()>& _callback)
		{
			if (!this->is_active())
			{
				this->m_thread_callback = _callback;
			}
		}

		// callback function after process finished
		void set_post_process_callback(const std::function<void()>& _callback)
		{
			if (!this->is_active())
			{
				this->m_post_process_callback = _callback;
			}
		}

		// generic: called every 100ms until process thread finished :: generic and progressbar only
		void set_progress_callback(const std::function<void()>& _callback)
		{
			if (!this->is_active())
			{
				this->m_progress_callback = _callback;
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

		// was kill_process called?
		[[nodiscard]] bool pending_termination() const
		{
			return this->m_kill_thread;
		}
		
		void create_process();

		process();
		~process() override;
		const char* get_name() override { return "process"; }
	};
}
