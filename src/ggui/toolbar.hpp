#pragma once
#include "_ggui.hpp"

namespace ggui::toolbar
{
	void	menu_new(ggui::imgui_context_menu& toolbar, ggui::imgui_context_menu& toolbar_edit);
	void	menu_toolbar_edit(ggui::imgui_context_menu& menu);
	void	save_settings_ini();

	void	register_dvars();
	
}
