#pragma once

namespace ggui
{
	class effects_browser final : public ggui::ggui_module
	{
		ImGuiTextFilter	m_filter;
		bool			m_scene_texture_hovered = false;

		std::filesystem::path						 m_fx_directory;
		std::filesystem::path						 m_current_directory;
		std::vector<std::string>					 m_curr_dir_folders;
		std::map<std::string, std::filesystem::path> m_curr_dir_files;

		ImGuiID m_dragdrop_id = 0;
		std::string m_dragdrop_fx_name;
		std::string m_dragdrop_fx_path;

	public:
		int				m_effect_filecount = 0;
		//const char**	m_effect_filelist = nullptr;
		std::string		m_effect_selection;
		std::string		m_effect_selection_old;

		float			m_camera_distance = -300.0f;

		effects_browser() { set_gui_type(GUI_TYPE_RTT); } // render-to-texture gui

		// *
		// public member functions

		int image_button(const game::GfxImage* img, const float img_size, const char* label, const char* img_ico_awesomefont = ICON_FA_CARET_RIGHT, ImVec2 uv0 = ImVec2(0.0f, 0.0f), ImVec2 uv1 = ImVec2(1.0f, 1.0f));
		void handle_drag_drop();
		void update_directory();

		bool gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related



		// *
		// init

	private:
		//void effect_listbox_elem(int index);
	};
}
