#pragma once
#include "_ggui.hpp"

namespace ggui
{
	struct console
	{
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

	public:
		console();
		~console();

		void    clear_log();
		void	draw_text_with_color(const char* text);
		void	addline_no_format(const char* text);
		void	addline(const char* fmt, ...) IM_FMTARGS(2);
		void	draw(const char* title, ggui::imgui_context_menu& menu);
		void	exec_command(const char* command_line);

		static int	text_edit_callback_stub(ImGuiInputTextCallbackData* data);
		int			text_edit_callback(ImGuiInputTextCallbackData* data);

		static void menu(ggui::imgui_context_menu& menu);
		static void	hooks();
	};

	extern	console* _console;
	
}
