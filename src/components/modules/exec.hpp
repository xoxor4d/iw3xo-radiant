#pragma once

namespace components
{
	class exec : public component
	{
	public:
		exec();
		~exec();
		const char* get_name() override { return "exec"; };

		static void on_gui_recurring(const std::function<void()>& callback);
		static void on_gui_once(const std::function<void()>& callback);
		static void on_gui_execute();

	private:
		static std::vector<std::function<void()>> m_gui_recurring_callbacks;
		static std::vector<std::function<void()>> m_gui_single_callbacks;
	};
}
