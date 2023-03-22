#pragma once

namespace ggui
{
	class console_dialog final : public ggui::ggui_module
	{
	public:
		char					m_input_buf[256];
		int						m_old_input_but_len;
		bool					m_input_reclaim_focus;
		ImVector<char*>			m_items;
		ImVector<const char*>	m_autocomplete_candidates;
		int						m_autocomplete_pos;
		bool					m_should_search_candidates;
		game::dvar_s*			m_matched_dvar;
		ImVector<char*>			m_history;
		int						m_history_pos;    // -1: new line, 0..m_history.Size-1 browsing history.
		ImGuiTextFilter			m_filter;
		bool					m_auto_scroll;
		bool					m_scroll_to_bottom;
		ImVec2					m_post_inputbox_cursor;
		bool					m_input_focused;
		ULONGLONG				m_error_timeout_tick;

		console_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			clear_log();
			memset(m_input_buf, 0, sizeof(m_input_buf));

			m_old_input_but_len = 0;
			m_input_reclaim_focus = false;
			m_autocomplete_pos = -1;
			m_should_search_candidates = false;
			m_matched_dvar = nullptr;
			m_history_pos = -1;
			m_auto_scroll = true;
			m_scroll_to_bottom = true;
			m_post_inputbox_cursor = ImVec2(0.0f, 0.0f);
			m_input_focused = false;
			m_error_timeout_tick = 0;
		}

		~console_dialog() override;

		// *
		// public member functions

		bool gui() override;
		void	clear_log();
		int		text_edit_callback(ImGuiInputTextCallbackData* data);
		void	addline_no_format(const char* text);
		void	addline(const char* fmt, ...) IM_FMTARGS(2);

		// *
		// asm related

		static void load_raw_materials_progressbar(int index, int material_total_count);
		static void load_raw_materials_progressbar_stub();

		// *
		// init

		void	hooks();

	private:
		static void	strtrim(char* s);
		void		draw_text_with_color(const char* text, int index);
		void		exec_command(const char* command_line);

		static void on_viewconsole_command();
	};
}
