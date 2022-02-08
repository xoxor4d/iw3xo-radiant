#pragma once

namespace fx_system
{
	extern game::dvar_s* fx_enable;
	extern game::dvar_s* fx_draw;
	extern game::dvar_s* fx_drawClouds;
	extern game::dvar_s* fx_cull_elem_draw;
	extern game::dvar_s* fx_cull_effect_spawn;
	extern game::dvar_s* fx_cull_elem_spawn;

	void	register_dvars();

}