#include "std_include.hpp"

namespace components
{
	gameview* gameview::p_this = nullptr;

	void gameview::set_state(bool state)
	{
		if(!gameview::p_this)
		{
			game::printf_to_console("[RED]Gameview module seems to be disabled, returning ...");
			return;
		}

		const auto filter = GET_GUI(ggui::filter_dialog);

		// toggle off
		if(!state)
		{
			dvars::set_bool(dvars::radiant_gameview, false);
			
			if(!this->var.enabled)
			{
				return;
			}

			if((this->var.s_xyflags & 2) == 0) {
				game::g_qeglobals->d_savedinfo.d_xyShowFlags &= ~2;
			}

			if ((this->var.s_xyflags & 4) == 0) {
				game::g_qeglobals->d_savedinfo.d_xyShowFlags &= ~4;
			}

			dvars::set_bool(dvars::r_draw_model_origin, this->var.s_model_origin);
			dvars::set_bool(dvars::r_draw_patch_backface_wireframe, this->var.s_patch_backface_wireframe);

			// geometry filters
			{
				if (this->var.s_filter_mantle)				filter->toggle_by_name("mantle", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_nodraw)				filter->toggle_by_name("nodraw_notsolid", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_shadowcaster)		filter->toggle_by_name("shadowcaster", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_traverse)			filter->toggle_by_name("traverse", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_foliage)				filter->toggle_by_name("foliage", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_weaponclip)			filter->toggle_by_name("weaponclip", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_goal)				filter->toggle_by_name("goalvolumes", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_lightgrid)			filter->toggle_by_name("lightgrid", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_caulk)				filter->toggle_by_name("caulk", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_nodrawdecal)			filter->toggle_by_name("nodrawdecal", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_clip)				filter->toggle_by_name("clip", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_hint)				filter->toggle_by_name("hintskip", ggui::filter_dialog::GEOMETRY, true);
				if (this->var.s_filter_portals)				filter->toggle_by_name("portals", ggui::filter_dialog::GEOMETRY, true);
			}

			// entity filters
			{
				if (this->var.s_filter_lights)				filter->toggle_by_name("lights", ggui::filter_dialog::ENTITY, true);
				if (this->var.s_filter_reflectionprobes)	filter->toggle_by_name("reflectionprobes", ggui::filter_dialog::ENTITY, true);
				if (this->var.s_filter_aispawners)			filter->toggle_by_name("aispawners", ggui::filter_dialog::ENTITY, true);
				if (this->var.s_filter_pathnodes)			filter->toggle_by_name("pathnodes", ggui::filter_dialog::ENTITY, true);
				if (this->var.s_filter_actionnodes)			filter->toggle_by_name("actionnodes", ggui::filter_dialog::ENTITY, true);
				if (this->var.s_filter_info)				filter->toggle_by_name("info", ggui::filter_dialog::ENTITY, true);
				if (this->var.s_filter_mpentities)			filter->toggle_by_name("mp_entities", ggui::filter_dialog::ENTITY, true);
				if (this->var.s_filter_script_origins)		filter->toggle_by_name("script_origins", ggui::filter_dialog::ENTITY, true);
			}
			
			// trigger filters
			{
				if (this->var.s_filter_trigger)				filter->toggle_by_name("triggers", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_bcs)					filter->toggle_by_name("bcs", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_escort)				filter->toggle_by_name("escort", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_unlock)				filter->toggle_by_name("unlock", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_vehicle)				filter->toggle_by_name("vehicle", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_friendly_respawn)	filter->toggle_by_name("friendly_respawn", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_flag)				filter->toggle_by_name("flag", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_fog)					filter->toggle_by_name("fog", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_auto_adjust)			filter->toggle_by_name("auto_adjust", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_spawner)				filter->toggle_by_name("spawner", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_stopspawner)			filter->toggle_by_name("stopspawner", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_friendlychain)		filter->toggle_by_name("friendlychain", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_ambient)				filter->toggle_by_name("ambient", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_damage)				filter->toggle_by_name("damage", ggui::filter_dialog::TRIGGER, true);
				if (this->var.s_filter_trigger_hint)		filter->toggle_by_name("hint", ggui::filter_dialog::TRIGGER, true);
			}

			// other filters
			{
				if (this->var.s_filter_opt_ladder)		filter->toggle_by_name("ladder", ggui::filter_dialog::OTHER, true);
			}

			memset(&this->var, 0, sizeof(vars_s));
		}

		// toggle on
		else
		{
			dvars::set_bool(dvars::radiant_gameview, true);
			
			if (!this->var.enabled)
			{
				this->var.s_xyflags = game::g_qeglobals->d_savedinfo.d_xyShowFlags;
				this->var.s_model_origin = dvars::r_draw_model_origin->current.enabled;
				this->var.s_patch_backface_wireframe = dvars::r_draw_patch_backface_wireframe->current.enabled;
				this->var.enabled = true;
			}

			game::g_qeglobals->d_savedinfo.d_xyShowFlags |= (2|4); // add angles and connection lines flags to disable rendering
			dvars::set_bool(dvars::r_draw_model_origin, false);
			dvars::set_bool(dvars::r_draw_patch_backface_wireframe, false);
			
			// geometry filters
			{
				this->var.s_filter_mantle			= filter->toggle_by_name("mantle", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_nodraw			= filter->toggle_by_name("nodraw_notsolid", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_shadowcaster		= filter->toggle_by_name("shadowcaster", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_traverse			= filter->toggle_by_name("traverse", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_foliage			= filter->toggle_by_name("foliage", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_weaponclip		= filter->toggle_by_name("weaponclip", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_goal				= filter->toggle_by_name("goalvolumes", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_lightgrid		= filter->toggle_by_name("lightgrid", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_caulk			= filter->toggle_by_name("caulk", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_nodrawdecal		= filter->toggle_by_name("nodrawdecal", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_clip				= filter->toggle_by_name("clip", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_hint				= filter->toggle_by_name("hintskip", ggui::filter_dialog::GEOMETRY, false);
				this->var.s_filter_portals			= filter->toggle_by_name("portals", ggui::filter_dialog::GEOMETRY, false);
			}
			
			// entity filters
			{
				this->var.s_filter_lights			= filter->toggle_by_name("lights", ggui::filter_dialog::ENTITY, false);
				this->var.s_filter_reflectionprobes = filter->toggle_by_name("reflectionprobes", ggui::filter_dialog::ENTITY, false);
				this->var.s_filter_aispawners		= filter->toggle_by_name("aispawners", ggui::filter_dialog::ENTITY, false);
				this->var.s_filter_pathnodes		= filter->toggle_by_name("pathnodes", ggui::filter_dialog::ENTITY, false);
				this->var.s_filter_actionnodes		= filter->toggle_by_name("actionnodes", ggui::filter_dialog::ENTITY, false);
				this->var.s_filter_info				= filter->toggle_by_name("info", ggui::filter_dialog::ENTITY, false);
				this->var.s_filter_mpentities		= filter->toggle_by_name("mp_entities", ggui::filter_dialog::ENTITY, false);
				this->var.s_filter_script_origins	= filter->toggle_by_name("script_origins", ggui::filter_dialog::ENTITY, false);
			}
			
			// trigger filters
			{
				this->var.s_filter_trigger			= filter->toggle_by_name("triggers", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_bcs				= filter->toggle_by_name("bcs", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_escort			= filter->toggle_by_name("escort", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_unlock			= filter->toggle_by_name("unlock", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_vehicle			= filter->toggle_by_name("vehicle", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_friendly_respawn = filter->toggle_by_name("friendly_respawn", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_flag				= filter->toggle_by_name("flag", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_fog				= filter->toggle_by_name("fog", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_auto_adjust		= filter->toggle_by_name("auto_adjust", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_spawner			= filter->toggle_by_name("spawner", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_stopspawner		= filter->toggle_by_name("stopspawner", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_friendlychain	= filter->toggle_by_name("friendlychain", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_ambient			= filter->toggle_by_name("ambient", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_damage			= filter->toggle_by_name("damage", ggui::filter_dialog::TRIGGER, false);
				this->var.s_filter_trigger_hint		= filter->toggle_by_name("hint", ggui::filter_dialog::TRIGGER, false);
			}

			// other filters
			{
				this->var.s_filter_opt_ladder = filter->toggle_by_name("ladder", ggui::filter_dialog::OTHER, false);
			}
		}
	}

	void gameview::register_dvars()
	{
		dvars::radiant_gameview = dvars::register_bool(
			/* name		*/ "radiant_gameview",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "hides everything thats not visible in-game");
	}

	gameview::gameview()
	{
		gameview::p_this = this;

		command::register_command_with_hotkey("xo_gameview"s, [this](auto)
		{
			components::gameview::set_state(!dvars::radiant_gameview->current.enabled);
		});
	}

	gameview::~gameview()
	{ }
}