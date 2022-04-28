#pragma once

namespace ggui
{
	class camera_dialog final : public ggui::ggui_module
	{
	public:
		bool m_toolbar_state = false;

		camera_dialog()
		{
			set_gui_type(GUI_TYPE_RTT);
			this->open(); // done so that on_open fires

		} // render-to-texture gui


		// *
		// public member functions

		void camera_gui();

		void gui() override;
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
