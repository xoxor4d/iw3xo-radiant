#pragma once

namespace ggui
{
	class effects_browser final : public ggui::ggui_module
	{
		ImGuiTextFilter	m_filter;
		bool			m_scene_texture_hovered = false;

	public:
		int				m_effect_filecount = 0;
		const char**	m_effect_filelist = nullptr;
		int				m_effect_selection = -1;
		int				m_effect_selection_old = -1;

		float			m_camera_distance = 0.0f;

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

	private:
		void effect_listbox_elem(int index);
	};
}
