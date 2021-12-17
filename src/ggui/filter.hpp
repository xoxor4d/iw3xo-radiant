#pragma once
#include "_ggui.hpp"

namespace ggui::filter
{
	enum E_FILTERS
	{
		GEOMETRY,
		ENTITY,
		TRIGGER,
		OTHER,
	};

	extern	bool input_focused;
	extern	bool window_hovered;
	
	extern	std::vector<game::filter_entry_s*> _geofilters, _entityfilters, _triggerfilters, _otherfilters;
	
	bool	toggle_by_name(const char* name, E_FILTERS filter_type, bool on_off);
	
	void	menu(ggui::imgui_context_menu& menu);
	void	hooks();
}
