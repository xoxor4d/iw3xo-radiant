#pragma once

namespace dvars
{
	extern game::dvar_s* parse_debug;

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

	extern game::dvar_s* gui_menubar_show_mouseorigin;
	extern game::dvar_s* gui_draw_fps;

	extern game::dvar_s* gui_texwnd_draw_scrollbar;
	extern game::dvar_s* gui_texwnd_draw_scrollpercent;
	
	extern game::dvar_s* gui_props_classlist_defaultopen;
	extern game::dvar_s* gui_props_comments_defaultopen;
	extern game::dvar_s* gui_props_spawnflags_defaultopen;
	extern game::dvar_s* gui_props_surfinspector;
	extern game::dvar_s* gui_use_new_surfinspector;
	extern game::dvar_s* gui_use_new_vertedit_dialog;
	extern game::dvar_s* gui_use_new_context_menu;
	extern game::dvar_s* gui_use_new_filedialog;

	extern game::dvar_s* gui_grid_context_show_select;

	extern game::dvar_s* gui_camera_toolbar_defaultopen;
	extern game::dvar_s* gui_camera_toolbar_merge_bsp_buttons;

	extern game::dvar_s* gui_saved_state_console;
	extern game::dvar_s* gui_saved_state_filter;
	extern game::dvar_s* gui_saved_state_entity;
	extern game::dvar_s* gui_saved_state_textures;
	extern game::dvar_s* gui_saved_state_modelselector;
	extern game::dvar_s* gui_saved_state_surfinspector;

	extern game::dvar_s* mainframe_show_console;
	extern game::dvar_s* mainframe_show_zview;
	extern game::dvar_s* mainframe_show_toolbar;
	extern game::dvar_s* mainframe_show_menubar;

	extern game::dvar_s* grid_zoom_to_cursor;
	extern game::dvar_s* grid_draw_edge_coordinates;

	extern game::dvar_s* bsp_load_entities;

	extern game::dvar_s* r_draw_bsp;
	extern game::dvar_s* r_draw_bsp_overwrite_sunlight;
	extern game::dvar_s* r_draw_bsp_overwrite_sundir;
	extern game::dvar_s* r_draw_bsp_overwrite_sunspecular;

	extern game::dvar_s* r_draw_model_origin;
	extern game::dvar_s* r_draw_model_shadowcaster;
	extern game::dvar_s* r_draw_patch_backface_wireframe;
	
	extern game::dvar_s* r_fakesun_preview;
	extern game::dvar_s* r_fakesun_use_worldspawn;
	extern game::dvar_s* r_fakesun_fog_enabled;
	extern game::dvar_s* r_fakesun_fog_start;
	extern game::dvar_s* r_fakesun_fog_half;
	extern game::dvar_s* r_fakesun_fog_color;

	extern game::dvar_s* r_sunpreview_shadow_dist;
	extern game::dvar_s* r_sunpreview_shadow_enable;

	extern game::dvar_s* r_generate_reflectionprobes;

	extern game::dvar_s* guizmo_enable;
	extern game::dvar_s* guizmo_snapping;
	extern game::dvar_s* guizmo_brush_mode;
	
	extern game::dvar_s* radiant_gameview;
	extern game::dvar_s* radiant_maxfps_grid;
	extern game::dvar_s* radiant_maxfps_camera;
	extern game::dvar_s* radiant_maxfps_textures;
	extern game::dvar_s* radiant_maxfps_modelselector;
	extern game::dvar_s* radiant_maxfps_mainframe;
	
	// radiant-live
	extern game::dvar_s* radiant_live;
	extern game::dvar_s* radiant_livePort;
	extern game::dvar_s* radiant_liveDebug;
	extern game::dvar_s* radiant_liveWorldspawn;

	// stock dvars (not registered but ptr assigned)
	extern game::dvar_s* fs_homepath;
	extern game::dvar_s* r_filmtweakenable;
	extern game::dvar_s* r_filmtweakdesaturation;
	extern game::dvar_s* r_filmtweakbrightness;
	extern game::dvar_s* r_filmtweakcontrast;
	extern game::dvar_s* r_filmtweakdarktint;
	extern game::dvar_s* r_filmtweaklighttint;


	// -----------------------------------------------------

	game::dvar_s* register_int(const char* dvar_name, int value, int mins, int maxs, __int16 flags, const char* description);
	game::dvar_s* register_bool(const char* dvar_name, char value, __int16 flags, const char* description);
	game::dvar_s* register_float(const char* dvar_name, float value, float mins, float maxs, __int16 flags, const char* description);
	game::dvar_s* register_vec4(const char* dvar_name, float x, float y, float z, float w, float mins, float maxs, __int16 flags, const char* description);

	void set_bool(game::dvar_s* dvar, bool value);
	void set_int(game::dvar_s* dvar, int value);
	void set_float(game::dvar_s* dvar, float value);

	void assign_stock_dvars();

	// -----------------------------------------------------

	void register_addon_dvars(); // registers all addon dvars
}