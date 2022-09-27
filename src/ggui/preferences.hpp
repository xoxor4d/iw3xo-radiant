#pragma once

namespace ggui
{
	class preferences_dialog final : public ggui::ggui_module
	{
	private:
		struct pref_child_s
		{
			unsigned int index;
			std::function<void()> callback;
		};

		nlohmann::fifo_map<std::string, pref_child_s> _pref_childs;

		bool m_update_scroll;
		int m_child_current;
		unsigned int m_child_count;

	public:
		bool	dev_bool_01;
		int		dev_num_01;
		int		dev_num_02;
		float	dev_vec_01[4];
		float	dev_color_01[4];
		float	dev_color_02[4];
		float	dev_color_03[4];

		preferences_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			m_update_scroll = false;
			m_child_current = 0;
			m_child_count = 0;

			dev_bool_01 = false;
			dev_num_01 = 256;
			dev_num_02 = 1;
			utils::vector::set_vec4(dev_vec_01, 1.0f);
			utils::vector::set_vec4(dev_color_01, 1.0f);
			utils::vector::set_vec4(dev_color_02, 1.0f);
			utils::vector::set_vec4(dev_color_03, 1.0f);
		}


		// *
		// public member functions

		bool gui() override;

		// *
		// asm related

		static void on_prefsdialog_command();


		// *
		// init

		static void	register_dvars();
		static void	hooks();

	private:
		const std::string CAT_GENERAL = "General";
		const std::string CAT_GUI = "Gui";
		const std::string CAT_GRID = "Grid";
		const std::string CAT_CAMERA = "Camera";
		const std::string CAT_EFFECTS_BROWSER = "Effects Browser";
		const std::string CAT_TEXTURES = "Textures";
		const std::string CAT_RENDERER = "Renderer / BSP";
		const std::string CAT_LIVELINK = "Live Link";
		const std::string CAT_DEVELOPER = "Developer";

		const float m_child_bg_col[4] = { 0.7f, 0.7f, 0.7f, 0.1f };
		const float m_child_bg_highlight_col[4] = { 0.7f, 0.7f, 0.7f, 0.1f };

		void  register_child(const std::string& _child_name, const std::function<void()>& _callback);
		float pref_child_lambda(const std::string& child_name, const float child_height, const float* bg_color, const float* border_color, const std::function<void()>& cb);

		void child_general();
		void child_gui();
		void child_grid();
		void child_camera();
		void child_effects_browser();
		void child_textures();
		void child_renderer_bsp();
		void child_livelink();
		void child_developer();
	};
}
