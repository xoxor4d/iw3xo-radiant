#pragma once

namespace ggui
{
	class gui_colors_dialog final : public ggui::ggui_module
	{
	public:
		gui_colors_dialog() { set_gui_type(GUI_TYPE_DEF); }
		void gui() override;
	};
}
