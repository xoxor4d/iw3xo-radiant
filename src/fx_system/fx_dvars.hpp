#pragma once

namespace dvars
{
	extern	game::dvar_s* fx_draw;
	extern	game::dvar_s* fx_enable;
	extern	game::dvar_s* fx_cull_elem_spawn;
	extern	game::dvar_s* fx_cull_elem_draw;
	extern	game::dvar_s* fx_cull_effect_spawn;
}

namespace fx_system
{
	void	register_dvars();
}