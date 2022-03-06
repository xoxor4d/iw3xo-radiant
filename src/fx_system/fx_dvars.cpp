#include "std_include.hpp"

namespace dvars
{
	game::dvar_s* fx_draw = nullptr;
	game::dvar_s* fx_enable = nullptr;
	game::dvar_s* fx_cull_elem_spawn = nullptr;
	game::dvar_s* fx_cull_elem_draw = nullptr;
	game::dvar_s* fx_cull_effect_spawn = nullptr;
}

namespace fx_system
{
	void register_dvars()
	{
		dvars::fx_enable = dvars::register_bool(
			/* name		*/ "fx_enable",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Toggles all effects processing");

		dvars::fx_draw = dvars::register_bool(
			/* name		*/ "fx_draw",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Toggles drawing of effects after processing");

		dvars::fx_cull_elem_spawn = dvars::register_bool(
			/* name		*/ "fx_cull_elem_spawn",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "Culls effect elems for spawning");

		dvars::fx_cull_elem_draw = dvars::register_bool(
			/* name		*/ "fx_cull_elem_draw",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Culls effect elems for drawing");

		dvars::fx_cull_effect_spawn = dvars::register_bool(
			/* name		*/ "fx_cull_effect_spawn",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "Culls entire effects for spawning");

		
	}
}