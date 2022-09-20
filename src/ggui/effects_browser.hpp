#pragma once

namespace ggui
{
	class effects_browser final : public ggui::ggui_module
	{
		bool m_scene_texture_hovered = false;

	public:
		float m_camera_distance = 0.0f;

		effects_browser() { set_gui_type(GUI_TYPE_RTT); } // render-to-texture gui

		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related



		// *
		// init

		
	};
}
