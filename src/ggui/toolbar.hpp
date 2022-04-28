#pragma once

namespace ggui
{
	class toolbar_dialog final : public ggui::ggui_module
	{
		struct tb_order_element_s
		{
			std::string name;
			int			id;
			bool		visible;
			bool		is_separator;
			bool		debug;
		};

		int m_element_id = 0;
		int m_sorted_element_id = 0;

	public:

		ImVec2		m_toolbar_pos {};
		ImVec2		m_toolbar_size {};
		ImGuiAxis	m_toolbar_axis = ImGuiAxis_X;
		bool		m_toolbar_reset = false;
		ImGuiID		m_toolbar_dock_top = 0;
		ImGuiID		m_toolbar_dock_left = 0;

		nlohmann::fifo_map<std::string, std::function<void()>> m_registered_elements_callbacks;
		std::vector<tb_order_element_s> m_registered_elements;
		std::vector<tb_order_element_s> m_sorted_elements;


		toolbar_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);
			this->open(); // done so that on_open fires
		}
		
		// *
		// public member functions

		static void image_button(const char* image_name, bool& hovered_state, E_CALLTYPE calltype, uint32_t func_addr, const char* tooltip);
		static bool image_togglebutton(const char* image_name, bool& hovered_state, bool toggle_state, const char* tooltip, ImVec4* bg_col = nullptr, ImVec4* bg_col_hovered = nullptr, ImVec4* bg_col_active = nullptr, ImVec2* btn_size = nullptr);

		void register_element(const std::string& name, bool default_visible, std::function<void()> callback);
		void register_element(const std::string& name, std::function<void()> callback);
		void save_settings_ini();

		void toolbar();
		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related

		// *
		// init

	private:
		void toolbar_elements_init();
		void load_settings_ini();
	};


	// ---------------------------------


	class toolbar_edit_dialog final : public ggui::ggui_module
	{
		struct tb_selection_s
		{
			bool is_selected;
			uint32_t element_id;
			uint32_t element_pos;
		};

	public:
		tb_selection_s tbedit_selection {};

		toolbar_edit_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related

		// *
		// init
	};
}
