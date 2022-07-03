#pragma once

namespace ggui
{
	class vertex_edit_dialog final : public ggui::ggui_module
	{
	public:
		vertex_edit_dialog() { set_gui_type(GUI_TYPE_DEF); }


		// *
		// public member functions

		bool gui() override;

		// *
		// asm related

		static void on_vertex_edit_dialog_command();

		// *
		// init

		static void hooks();
		static void register_dvars();

	private:
		void feature_noise();
	};
}
