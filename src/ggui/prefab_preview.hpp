#pragma once

namespace ggui
{
	extern std::filesystem::path prefab_browser_generate_thumbnails_folder;
	extern bool prefab_browser_generate_thumbnails;

	class prefab_preview_dialog final : public ggui::ggui_module
	{
	private:
		std::filesystem::path m_current_directory;
		std::filesystem::path m_prefab_directory;
		std::filesystem::path m_mapsource_directory;

		std::vector<std::string> m_curr_dir_folders;
		std::map<std::string, std::filesystem::path> m_curr_dir_files;
		//std::vector<std::string> m_curr_dir_files;

		ImGuiID m_dragdrop_id = 0;
		std::string m_dragdrop_prefab_name;
		std::string m_dragdrop_prefab_path;

		ImGuiTextFilter	m_filter;

	public:
		prefab_preview_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		std::string get_thumbnail_string(const std::filesystem::path& dir, const std::string& filename);
		void handle_drag_drop();

		void update_directory();
		bool image_button(const game::GfxImage* img, const float size, const float scale, const char* label, bool enable_highlight = false, int padding = 0.0f, ImVec2 uv0 = ImVec2(0.0f, 0.0f), ImVec2 uv1 = ImVec2(1.0f, 1.0f));


		bool gui() override;
		void on_init() override;
		void on_open() override;
		void on_close() override;

		static void register_dvars();

	};
}
