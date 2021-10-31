#pragma once
#include "_ggui.hpp"

namespace ggui::toolbar
{
	void	menu_new(ggui::imgui_context_menu& toolbar, ggui::imgui_context_menu& toolbar_edit);
	void	menu_toolbar_edit(ggui::imgui_context_menu& menu);
	void	save_settings_ini();

	void	register_dvars();

	void	image_button(const char* image_name, bool& hovered_state, E_CALLTYPE calltype, uint32_t func_addr, const char* tooltip);
	bool	image_togglebutton(const char* image_name, bool& hovered_state, bool toggle_state, const char* tooltip, ImVec4* bg_col = nullptr, ImVec4* bg_col_hovered = nullptr, ImVec4* bg_col_active = nullptr, ImVec2* btn_size = nullptr);
	
}
