#pragma once

namespace dvars
{
	extern game::dvar_s* gui_menubar_bg_color;
	extern game::dvar_s* gui_window_child_bg_color;
	extern game::dvar_s* gui_window_bg_color;
	extern game::dvar_s* gui_toolbar_bg_color;
	extern game::dvar_s* gui_border_color;
	extern game::dvar_s* gui_dockingpreview_color;
	
	extern game::dvar_s* gui_toolbar_button_color;
	extern game::dvar_s* gui_toolbar_button_hovered_color;
	extern game::dvar_s* gui_toolbar_button_active_color;

	extern game::dvar_s* gui_rtt_padding_enabled;
	extern game::dvar_s* gui_rtt_padding_size;
	extern game::dvar_s* gui_rtt_padding_color;
	
	
	extern game::dvar_s* gui_floating_toolbar;
	extern game::dvar_s* gui_resize_dockspace;
	extern game::dvar_s* gui_mainframe_background;

	extern game::dvar_s* gui_saved_state_console;
	extern game::dvar_s* gui_saved_state_filter;
	extern game::dvar_s* gui_saved_state_entity;
	extern game::dvar_s* gui_saved_state_textures;
	
	//
	extern game::dvar_s* mainframe_show_console;
	extern game::dvar_s* mainframe_show_zview;
	extern game::dvar_s* mainframe_show_toolbar;
	extern game::dvar_s* mainframe_show_menubar;

	// radiant-live
	extern game::dvar_s* radiant_live;
	extern game::dvar_s* radiant_livePort;
	extern game::dvar_s* radiant_liveDebug;

	// stock dvars (not registered but ptr assigned)
	extern game::dvar_s* fs_homepath;

	// -----------------------------------------------------

	game::dvar_s* register_int(const char* dvar_name, int value, int mins, int maxs, __int16 flags, const char* description);
	game::dvar_s* register_bool(const char* dvar_name, char value, __int16 flags, const char* description);
	game::dvar_s* register_float(const char* dvar_name, float value, float mins, float maxs, __int16 flags, const char* description);
	game::dvar_s* register_vec4(const char* dvar_name, float x, float y, float z, float w, float mins, float maxs, __int16 flags, const char* description);

	void set_bool(game::dvar_s* dvar, bool value);
	void set_int(game::dvar_s* dvar, int value);
	void set_float(game::dvar_s* dvar, float value);
	
	// -----------------------------------------------------

	void register_addon_dvars(); // registers all addon dvars
}