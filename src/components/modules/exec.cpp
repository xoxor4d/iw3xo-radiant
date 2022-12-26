#include "std_include.hpp"

namespace components
{

	std::vector<exec::task_s> exec::m_gui_recurring_callbacks;
	std::vector<exec::task_s> exec::m_gui_single_callbacks;

	bool do_task(exec::task_s& task)
	{
		const auto now = std::chrono::high_resolution_clock::now();
		const auto diff = now - task.last_exec;

		if (diff < task.interval)
		{
			return false;
		}

		task.last_exec = now;
		task.func();
		task.was_executed = true;

		return true;
	}

	void exec::on_gui_execute()
	{
		for (auto& callback : m_gui_recurring_callbacks)
		{
			do_task(callback);
		}

		int i = 0;
		for (auto& callback : m_gui_single_callbacks)
		{
			do_task(callback);

			if (callback.was_executed)
			{
				m_gui_single_callbacks.erase(m_gui_single_callbacks.begin() + i);
			}

			i++;
		}

		//m_gui_single_callbacks.clear();
	}

	exec::exec()
	{ }

	exec::~exec()
	{ 
		m_gui_recurring_callbacks.clear();
		m_gui_single_callbacks.clear();
    }
}
