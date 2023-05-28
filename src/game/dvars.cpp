#include "std_include.hpp"

namespace dvars
{
	game::dvar_s* parse_debug = nullptr;

	game::dvar_s* gui_menubar_bg_color = nullptr;
	game::dvar_s* gui_window_child_bg_color = nullptr;
	game::dvar_s* gui_window_bg_color = nullptr;
	game::dvar_s* gui_toolbar_bg_color = nullptr;
	game::dvar_s* gui_border_color = nullptr;
	game::dvar_s* gui_dockingpreview_color = nullptr;
	
	game::dvar_s* gui_toolbar_button_color = nullptr;
	game::dvar_s* gui_toolbar_button_hovered_color = nullptr;
	game::dvar_s* gui_toolbar_button_active_color = nullptr;

	game::dvar_s* gui_rtt_padding_enabled = nullptr;
	game::dvar_s* gui_rtt_padding_size = nullptr;
	game::dvar_s* gui_rtt_padding_color = nullptr;

	game::dvar_s* gui_menubar_show_mouseorigin = nullptr;
	game::dvar_s* gui_draw_fps = nullptr;

	game::dvar_s* gui_texwnd_draw_scrollbar = nullptr;
	game::dvar_s* gui_texwnd_draw_scrollpercent = nullptr;

	game::dvar_s* gui_prefab_browser_img_size = nullptr;

	game::dvar_s* gui_props_classlist_defaultopen = nullptr;
	game::dvar_s* gui_props_comments_defaultopen = nullptr;
	game::dvar_s* gui_props_spawnflags_defaultopen = nullptr;
	game::dvar_s* gui_props_surfinspector = nullptr;
	game::dvar_s* gui_props_toolbox = nullptr;
	game::dvar_s* gui_toolbox_integrate_cam_toolbar = nullptr;
	game::dvar_s* gui_toolbox_integrate_filter = nullptr;
	game::dvar_s* gui_toolbox_integrate_layers = nullptr;
	game::dvar_s* gui_use_new_surfinspector = nullptr;
	game::dvar_s* gui_use_new_vertedit_dialog = nullptr;
	game::dvar_s* gui_use_new_context_menu = nullptr;
	game::dvar_s* gui_use_new_filedialog = nullptr;

	game::dvar_s* gui_grid_context_show_select = nullptr;

	game::dvar_s* gui_camera_toolbar_defaultopen = nullptr;
	game::dvar_s* gui_camera_toolbar_merge_bsp_buttons = nullptr;

	game::dvar_s* gui_saved_state_console = nullptr;
	game::dvar_s* gui_saved_state_filter = nullptr;
	game::dvar_s* gui_saved_state_entity = nullptr;
	game::dvar_s* gui_saved_state_effects_browser = nullptr;
	game::dvar_s* gui_saved_state_textures = nullptr;
	game::dvar_s* gui_saved_state_modelselector = nullptr;
	game::dvar_s* gui_saved_state_prefab_browser = nullptr;
	game::dvar_s* gui_saved_state_surfinspector = nullptr;
	game::dvar_s* gui_saved_state_toolbox = nullptr;
	game::dvar_s* gui_saved_state_toolbox_child = nullptr;

	game::dvar_s* mainframe_show_console = nullptr;
	game::dvar_s* mainframe_show_zview = nullptr;
	game::dvar_s* mainframe_show_toolbar = nullptr;
	game::dvar_s* mainframe_show_menubar = nullptr;

	game::dvar_s* grid_zoom_to_cursor = nullptr;
	game::dvar_s* grid_draw_edge_coordinates = nullptr;
	game::dvar_s* grid_new_patch_drag = nullptr;

	game::dvar_s* bsp_load_entities = nullptr;
	game::dvar_s* bsp_show_bsp_after_compile = nullptr;
	game::dvar_s* bsp_gen_reflections_on_compile = nullptr;
	game::dvar_s* bsp_gen_createfx_on_compile = nullptr;

	game::dvar_s* bsp_compile_bsp = nullptr;
	game::dvar_s* bsp_compile_onlyents = nullptr;
	game::dvar_s* bsp_compile_samplescale_enabled = nullptr;
	game::dvar_s* bsp_compile_samplescale = nullptr;
	game::dvar_s* bsp_compile_custom_cmd_enabled = nullptr;
	game::dvar_s* bsp_compile_custom_cmd = nullptr;

	game::dvar_s* bsp_compile_light = nullptr;
	game::dvar_s* bsp_compile_light_fast = nullptr;
	game::dvar_s* bsp_compile_light_extra = nullptr;
	game::dvar_s* bsp_compile_light_modelshadow = nullptr;
	game::dvar_s* bsp_compile_light_dump = nullptr;
	game::dvar_s* bsp_compile_light_traces_enabled = nullptr;
	game::dvar_s* bsp_compile_light_traces = nullptr;
	game::dvar_s* bsp_compile_light_custom_cmd_enabled = nullptr;
	game::dvar_s* bsp_compile_light_custom_cmd = nullptr;

	game::dvar_s* physx_debug_visualization_box_size = nullptr;
	game::dvar_s* physx_camera_sensitivity = nullptr;

	game::dvar_s* r_draw_bsp = nullptr;
	game::dvar_s* r_draw_bsp_overwrite_sunlight = nullptr;
	game::dvar_s* r_draw_bsp_overwrite_sundir = nullptr;
	game::dvar_s* r_draw_bsp_overwrite_sunspecular = nullptr;

	game::dvar_s* r_draw_model_origin = nullptr;
	game::dvar_s* r_draw_model_shadowcaster = nullptr;
	game::dvar_s* r_draw_patch_backface_wireframe = nullptr;

	game::dvar_s* r_fakesun_preview = nullptr;
	game::dvar_s* r_fakesun_use_worldspawn = nullptr;
	game::dvar_s* r_fakesun_fog_enabled = nullptr;
	game::dvar_s* r_fakesun_fog_start = nullptr;
	game::dvar_s* r_fakesun_fog_half = nullptr;
	game::dvar_s* r_fakesun_fog_color = nullptr;

	game::dvar_s* r_sunpreview_shadow_dist = nullptr;
	game::dvar_s* r_sunpreview_shadow_enable = nullptr;

	game::dvar_s* r_reflectionprobe_generate = nullptr;
	game::dvar_s* r_reflectionprobe_export_tga = nullptr;

	game::dvar_s* fx_browser_grid_sections = nullptr;
	game::dvar_s* fx_browser_grid_scale = nullptr;
	game::dvar_s* fx_browser_grid_color = nullptr;
	game::dvar_s* fx_browser_grid_line_width = nullptr;
	game::dvar_s* fx_browser_grid_font_scale = nullptr;
	game::dvar_s* fx_browser_grid_font_color = nullptr;
	game::dvar_s* fx_browser_use_camera_for_distortion = nullptr;

	game::dvar_s* guizmo_enable = nullptr;
	game::dvar_s* guizmo_snapping = nullptr;
	//game::dvar_s* guizmo_brush_mode = nullptr;
	
	game::dvar_s* radiant_gameview = nullptr;
	game::dvar_s* radiant_maxfps_grid = nullptr;
	game::dvar_s* radiant_maxfps_camera = nullptr;
	game::dvar_s* radiant_maxfps_textures = nullptr;
	game::dvar_s* radiant_maxfps_modelselector = nullptr;
	game::dvar_s* radiant_maxfps_mainframe = nullptr;
	
	// radiant-live
	game::dvar_s* radiant_live = nullptr;
	game::dvar_s* radiant_livePort = nullptr;
	game::dvar_s* radiant_liveDebug = nullptr;
	game::dvar_s* radiant_liveWorldspawn = nullptr;

	// stock dvars (not registered but ptr assigned)
	game::dvar_s* fs_homepath = nullptr;
	game::dvar_s* r_filmtweakenable = nullptr;
	game::dvar_s* r_filmtweakdesaturation = nullptr;
	game::dvar_s* r_filmtweakbrightness = nullptr;
	game::dvar_s* r_filmtweakcontrast = nullptr;
	game::dvar_s* r_filmtweakdarktint = nullptr;
	game::dvar_s* r_filmtweaklighttint = nullptr;
	
	// ---------------------------------------------

	game::dvar_s* register_int(const char* dvar_name, int value, int mins, int maxs, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterInt(dvar_name, value, mins, maxs, flags, description);
		game::printf_to_console(utils::va("|-> %s <int>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_bool(const char* dvar_name, char value, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterBool(dvar_name, value, flags, description);
		game::printf_to_console(utils::va("|-> %s <bool>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_float(const char* dvar_name, float value, float mins, float maxs, __int16 flags, const char* description)
	{
		game::dvar_s*  dvar = game::Dvar_RegisterFloat(dvar_name, value, mins, maxs, flags, description);
		game::printf_to_console(utils::va("|-> %s <float>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_string(const char* dvar_name, const char* value, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterString(dvar_name, value, flags, description);
		game::printf_to_console(utils::va("|-> %s <string>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	game::dvar_s* register_vec4(const char* dvar_name, float x, float y, float z, float w, float mins, float maxs, __int16 flags, const char* description)
	{
		game::dvar_s* dvar = game::Dvar_RegisterVec4(dvar_name, x, y, z, w, mins, maxs, flags, description);
		game::printf_to_console(utils::va("|-> %s <vec4>\n", dvar_name));

		// return a pointer to our dvar
		return dvar;
	}

	void set_bool(game::dvar_s* dvar, bool value)
	{
		game::Dvar_SetBool(dvar, value);
	}

	void set_int(game::dvar_s* dvar, int value)
	{
		game::Dvar_SetInt(dvar, value);
	}

	void set_float(game::dvar_s* dvar, float value)
	{
		game::Dvar_SetFloat(dvar, value);
	}

	void set_vec(game::dvar_s* dvar, float* value, int len)
	{
		switch (len)
		{
		case 2:
			game::Dvar_SetVec2(dvar, value[0], value[1]);
			break;

		case 3:
			game::Dvar_SetVec3(dvar, value[0], value[1], value[2]);
			break;

		case 4:
			game::Dvar_SetVec4(dvar, value[0], value[1], value[2], value[3]);
			break;

		default:
			game::printf_to_console("[set_vec] unsupported len!");
		}
	}

	void set_string(game::dvar_s* dvar, const char* value)
	{
		game::Dvar_SetString(dvar, value);
	}

	bool assign_single_dvar(game::dvar_s*& dest, const char* stock_dvar_name)
	{
		if (!dest)
		{
			if (const auto	var = game::Dvar_FindVar(stock_dvar_name); 
							var) 
			{
				dest = var;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	void assign_stock_dvars()
	{
		bool valid_dvars = true;

		if (!assign_single_dvar(dvars::r_filmtweakenable, "r_filmtweakenable")) valid_dvars = false;
		if (!assign_single_dvar(dvars::r_filmtweakdesaturation, "r_filmtweakdesaturation")) valid_dvars = false;
		if (!assign_single_dvar(dvars::r_filmtweakbrightness, "r_filmtweakbrightness")) valid_dvars = false;
		if (!assign_single_dvar(dvars::r_filmtweakcontrast, "r_filmtweakcontrast")) valid_dvars = false;
		if (!assign_single_dvar(dvars::r_filmtweakdarktint, "r_filmtweakdarktint")) valid_dvars = false;
		if (!assign_single_dvar(dvars::r_filmtweaklighttint, "r_filmtweaklighttint")) valid_dvars = false;

		if(!valid_dvars)
		{
			game::Com_Error("assign_stock_dvars: failed to assign stock dvars");
		}
	}

	// --------------------------------------------------

	// register all new dvars here (exec. after config was loaded)
	void register_addon_dvars()
	{
		game::printf_to_console("[DVARS] register_addon_dvars() start ...\n");

		components::d3dbsp::register_dvars();
		components::gameview::register_dvars();
		components::gui::register_dvars();
		components::reflectionprobes::register_dvars();
		components::remote_net::register_dvars();
		components::renderer::register_dvars();
		components::physx_impl::register_dvars();

		cmainframe::register_dvars();
		cxywnd::register_dvars();
		ccamwnd::register_dvars();
		ctexwnd::register_dvars();
		cfxwnd::register_dvars();

		ggui::entity_dialog::register_dvars();
		ggui::preferences_dialog::register_dvars();
		ggui::surface_dialog::register_dvars();
		ggui::vertex_edit_dialog::register_dvars();
		ggui::camera_settings_dialog::register_dvars();
		ggui::prefab_preview_dialog::register_dvars();
		ggui::toolbox_dialog::register_dvars();

		fx_system::register_dvars();

		// ---------------------

		dvars::parse_debug = dvars::register_bool(
			/* name		*/ "parse_debug",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "show debug info while parsing a map file (parsed entity and brush amount)");

		game::printf_to_console("\n");
	}
}