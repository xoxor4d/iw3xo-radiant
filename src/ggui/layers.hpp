#pragma once

namespace ggui
{
	class layer_dialog final : public ggui::ggui_module
	{
	public:
		layer_dialog() { set_gui_type(GUI_TYPE_DEF); }


		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related

		static void on_layerdialog_command(); // not a callable method

		// *
		// init

		static void hooks();
	};
}
