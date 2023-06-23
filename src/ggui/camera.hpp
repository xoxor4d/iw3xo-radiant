#pragma once

namespace ggui
{
	class camera_dialog final : public ggui::ggui_module
	{
	public:
		bool m_toolbar_state = false;
		bool m_rtt_focused = false;

		camera_dialog()
		{
			set_gui_type(GUI_TYPE_RTT);
			this->open(); // done so that on_open fires

		} // render-to-texture gui


		// *
		// public member functions

		static void convert_selection_to_prefab_imgui_menu();
		static bool stamp_prefab_imgui_imgui_menu(game::selbrush_def_t* sb = nullptr);

		void camera_gui();

		bool gui() override;
		void on_init() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related


		// *
		// init

	private:
		void toolbar();
		void context_menu();
		void drag_drop_target(bool& accepted_dragdrop);
	};
}
