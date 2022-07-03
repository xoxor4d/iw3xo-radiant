#pragma once

namespace ggui
{
	class help_dialog final : public ggui::ggui_module
	{
	public:
		help_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		bool gui() override;

		// *
		// asm related

		// *
		// init
	};
}
