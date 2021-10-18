#pragma once
#include "_ggui.hpp"

namespace ggui::hotkeys
{
	void			menu(ggui::imgui_context_menu& menu);
	void			helper_menu(ggui::imgui_context_menu& menu);
	std::string		get_hotkey_for_command(const char* command);
	void			on_close();
	void			hooks();
	
}
