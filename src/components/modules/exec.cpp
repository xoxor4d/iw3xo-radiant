#include "std_include.hpp"

namespace components
{
	std::vector<std::function<void()>> exec::m_gui_recurring_callbacks;
	std::vector<std::function<void()>> exec::m_gui_single_callbacks;

	void exec::on_gui_recurring(const std::function<void()>& callback)
	{
		m_gui_recurring_callbacks.emplace_back(callback);
	}

	void exec::on_gui_once(const std::function<void()>& callback)
	{
		m_gui_single_callbacks.emplace_back(callback);
	}

	void exec::on_gui_execute()
	{
		for (const auto& callback : m_gui_recurring_callbacks)
		{
			callback();
		}

		for (const auto& callback : m_gui_single_callbacks)
		{
			callback();
		}

		m_gui_single_callbacks.clear();
	}

	exec::exec()
	{ }

	exec::~exec()
	{ 
		m_gui_recurring_callbacks.clear();
		m_gui_single_callbacks.clear();
    }
}
