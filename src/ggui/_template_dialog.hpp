#pragma once
#include "_ggui.hpp"

namespace ggui
{
	class template_dialog final : public ggui::ggui_module
	{
	public:
		template_dialog() { set_gui_type(GUI_TYPE_RTT); }
		void gui() override;
	};
}
