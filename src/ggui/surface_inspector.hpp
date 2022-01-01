#pragma once
#include "_ggui.hpp"

namespace ggui::surface_inspector
{
	struct patch_texdef_t
	{
		game::MaterialDef def;
		int unk3;
		float sample_size;
	};

	struct texedit_helper
	{
		void reset_values()
		{
			size_horz = 0.0f;
			size_vert = 0.0f;
			shift_horz = 0.0f;
			shift_vert = 0.0f;
			rotation = 0.0f;
			//repeatx = 0.0f;
			//repeaty = 0.0f;
		}

		float amount_size = 1.0f;
		float amount_shift = 1.0f;
		float amount_rotate = 15.0f;
		float amount_sample = 4.0f;
		float amount_repeatx = 0.5f;
		float amount_repeaty = 0.5f;

		float size_horz;
		float size_vert;
		float shift_horz;
		float shift_vert;
		float rotation;
		float repeatx = 1.0f;
		float repeaty = 1.0f;

		int	 scalar_direction;
		bool specific_mode;
	};

	void	controls();
	void	menu(ggui::imgui_context_menu& menu);
	void	register_dvars();
	void	hooks();

}
