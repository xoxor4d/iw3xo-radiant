#pragma once
#include "_ggui.hpp"

namespace ggui::effects_editor_gui
{
	static inline int selected_editor_elemdef = 0;
	static inline bool editor_effect_was_modified = false;

	bool		Modal_UnsavedChanges(const char* label);
	void		menu(ggui::imgui_context_menu& menu);

}
