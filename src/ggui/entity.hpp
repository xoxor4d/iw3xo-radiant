#pragma once
#include "_ggui.hpp"

namespace ggui::entity
{
	struct addprop_helper_s
	{
		bool is_origin;
		bool is_angle;
		bool is_generic_slider;
		bool is_color;
		bool add_undo;
	};
	
	void	AddProp(const char* key, const char* value, addprop_helper_s* helper = nullptr);
	bool	Entity_GetValueForKey(game::entity_s* ent, float* value, const char* keyname);
	bool	Entity_GetVec3ForKey(game::entity_s* ent, float* vec3, const char* keyname);
	bool	HasKeyValuePair(game::entity_s_def* ent, const char* key);
	
	void	menu(ggui::imgui_context_menu& menu);
	void	register_dvars();
	void	hooks();
}
