#pragma once
#include "_ggui.hpp"

namespace ggui::camera_settings
{
	enum tab_state_
	{
		tab_state_fakesun = 0,
		tab_state_effects = 1,
	};

	inline static const char* tab_names[]
	{
		"Fakesun /PostFX",
		"Effects",
	};

	inline bool tab_states[]
	{
		false,
		false,
	};

	inline int	active_tab = -1;
	inline bool refocus_active_tab = false;

	inline void set_tabstate_effects(bool _state) { tab_states[tab_state_effects] = _state; }
	inline bool get_tabstate_effects() { return tab_states[tab_state_effects]; }
	inline bool is_tabstate_effects_active() { return active_tab == tab_state_effects; }

	inline void focus_effects()
	{
		refocus_active_tab = true;
		active_tab = tab_state_effects;
	}


	inline void set_tabstate_fakesun(bool _state) { tab_states[tab_state_fakesun] = _state; }
	inline bool get_tabstate_fakesun() { return tab_states[tab_state_fakesun]; }
	inline bool is_tabstate_fakesun_active() { return active_tab == tab_state_fakesun; }

	inline void focus_fakesun()
	{
		refocus_active_tab = true;
		active_tab = tab_state_fakesun;
	}

	// * -----------------

	inline float	sun_dir[3] = { 210.0f, 60.0f, 0.0f };
	inline float	sun_diffuse[3] = { 2.45f, 2.1f, 1.8f };
	inline float	sun_specular[4] = { 3.0f, 3.2f, 3.0f, 0.0f };
	inline float	material_specular[4] = { 0.35f, 0.45f, 1.0f, 4.0f };
	inline float	ambient[4] = { 0.3f, 0.3f, 0.3f, 0.35f };

	void	on_close();
	void	menu(ggui::imgui_context_menu& menu);

}
