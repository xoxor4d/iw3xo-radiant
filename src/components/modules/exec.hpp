#pragma once
using namespace std::chrono_literals;

namespace components
{
	class exec : public component
	{
	public:
		exec();
		~exec();
		const char* get_name() override { return "exec"; };

		struct task_s
		{
			std::function<void()> func;
			std::chrono::milliseconds interval = {};
			std::chrono::high_resolution_clock::time_point last_exec = {};
			bool was_executed = false;
		};

		static void on_gui_recurring(const std::function<void()>& callback, const std::chrono::milliseconds delay = 0ms)
		{
			m_gui_recurring_callbacks.emplace_back(task_s(callback, delay, std::chrono::high_resolution_clock::now()));
		}

		static void on_gui_once(const std::function<void()>& callback, const std::chrono::milliseconds delay = 0ms)
		{
			m_gui_single_callbacks.emplace_back(task_s(callback, delay, std::chrono::high_resolution_clock::now()));
		}

		static void on_gui_execute();

	private:
		static std::vector<task_s> m_gui_recurring_callbacks;
		static std::vector<task_s> m_gui_single_callbacks;
	};
}
