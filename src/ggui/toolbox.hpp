#pragma once

namespace ggui
{
	class toolbox_dialog final : public ggui::ggui_module
	{
	private:
		struct toolbox_child_s
		{
			unsigned int index;
			std::function<void()> callback;
		};

		nlohmann::fifo_map<std::string, toolbox_child_s> _toolbox_childs;

		bool m_update_scroll;
		int m_child_current;
		unsigned int m_child_count;

	public:
		toolbox_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			m_update_scroll = false;
			m_child_current = 0;
			m_child_count = 0;
		}

		// *
		// public member functions
		void gui() override;

		// *
		// init
		void	init();

	private:
		const std::string CAT_BRUSH = "Brush";
		const std::string CAT_PATCH = "Patch";

		void  register_child(const std::string& _child_name, const std::function<void()>& _callback);

		void child_brush();
		void child_patch();
	};
}
