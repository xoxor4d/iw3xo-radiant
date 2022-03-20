#pragma once
#include "_ggui.hpp"

namespace ggui::hotkeys
{
	void			menu(ggui::imgui_context_menu& menu);
	void			helper_menu(ggui::imgui_context_menu& menu);
	std::string		cmdbinds_ascii_to_keystr(int key);
	int				cmdbinds_key_to_ascii(std::string key);
	std::string		get_hotkey_for_command(const char* command);
	void			on_close();
	void			hooks();
	
}
