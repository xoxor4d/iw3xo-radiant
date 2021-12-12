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
				if (this->var.s_filter_mantle)				ggui::filter::toggle_by_name("mantle", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_nodraw)				ggui::filter::toggle_by_name("nodraw_notsolid", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_shadowcaster)		ggui::filter::toggle_by_name("shadowcaster", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_traverse)			ggui::filter::toggle_by_name("traverse", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_foliage)				ggui::filter::toggle_by_name("foliage", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_weaponclip)			ggui::filter::toggle_by_name("weaponclip", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_goal)				ggui::filter::toggle_by_name("goalvolumes", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_lightgrid)			ggui::filter::toggle_by_name("lightgrid", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_caulk)				ggui::filter::toggle_by_name("caulk", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_nodrawdecal)			ggui::filter::toggle_by_name("nodrawdecal", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_clip)				ggui::filter::toggle_by_name("clip", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_hint)				ggui::filter::toggle_by_name("hintskip", ggui::filter::GEOMETRY, true);
				if (this->var.s_filter_portals)				ggui::filter::toggle_by_name("portals", ggui::filter::GEOMETRY, true);
			}

			// entity filters
			{
				if (this->var.s_filter_lights)				ggui::filter::toggle_by_name("lights", ggui::filter::ENTITY, true);
				if (this->var.s_filter_reflectionprobes)	ggui::filter::toggle_by_name("reflectionprobes", ggui::filter::ENTITY, true);
				if (this->var.s_filter_aispawners)			ggui::filter::toggle_by_name("aispawners", ggui::filter::ENTITY, true);
				if (this->var.s_filter_pathnodes)			ggui::filter::toggle_by_name("pathnodes", ggui::filter::ENTITY, true);
				if (this->var.s_filter_actionnodes)			ggui::filter::toggle_by_name("actionnodes", ggui::filter::ENTITY, true);
				if (this->var.s_filter_info)				ggui::filter::toggle_by_name("info", ggui::filter::ENTITY, true);
				if (this->var.s_filter_mpentities)			ggui::filter::toggle_by_name("mp_entities", ggui::filter::ENTITY, true);
				if (this->var.s_filter_script_origins)		ggui::filter::toggle_by_name("script_origins", ggui::filter::ENTITY, true);
			}
			
			// trigger filters
			{
				if (this->var.s_filter_trigger)				ggui::filter::toggle_by_name("triggers", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_bcs)					ggui::filter::toggle_by_name("bcs", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_escort)				ggui::filter::toggle_by_name("escort", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_unlock)				ggui::filter::toggle_by_name("unlock", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_vehicle)				ggui::filter::toggle_by_name("vehicle", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_friendly_respawn)	ggui::filter::toggle_by_name("friendly_respawn", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_flag)				ggui::filter::toggle_by_name("flag", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_fog)					ggui::filter::toggle_by_name("fog", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_auto_adjust)			ggui::filter::toggle_by_name("auto_adjust", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_spawner)				ggui::filter::toggle_by_name("spawner", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_stopspawner)			ggui::filter::toggle_by_name("stopspawner", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_friendlychain)		ggui::filter::toggle_by_name("friendlychain", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_ambient)				ggui::filter::toggle_by_name("ambient", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_damage)				ggui::filter::toggle_by_name("damage", ggui::filter::TRIGGER, true);
				if (this->var.s_filter_trigger_hint)		ggui::filter::toggle_by_name("hint", ggui::filter::TRIGGER, true);
			}

			// other filters
			{
				if (this->var.s_filter_opt_ladder)		ggui::filter::toggle_by_name("ladder", ggui::filter::OTHER, true);
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
				this->var.s_filter_mantle			= ggui::filter::toggle_by_name("mantle", ggui::filter::GEOMETRY, false);
				this->var.s_filter_nodraw			= ggui::filter::toggle_by_name("nodraw_notsolid", ggui::filter::GEOMETRY, false);
				this->var.s_filter_shadowcaster		= ggui::filter::toggle_by_name("shadowcaster", ggui::filter::GEOMETRY, false);
				this->var.s_filter_traverse			= ggui::filter::toggle_by_name("traverse", ggui::filter::GEOMETRY, false);
				this->var.s_filter_foliage			= ggui::filter::toggle_by_name("foliage", ggui::filter::GEOMETRY, false);
				this->var.s_filter_weaponclip		= ggui::filter::toggle_by_name("weaponclip", ggui::filter::GEOMETRY, false);
				this->var.s_filter_goal				= ggui::filter::toggle_by_name("goalvolumes", ggui::filter::GEOMETRY, false);
				this->var.s_filter_lightgrid		= ggui::filter::toggle_by_name("lightgrid", ggui::filter::GEOMETRY, false);
				this->var.s_filter_caulk			= ggui::filter::toggle_by_name("caulk", ggui::filter::GEOMETRY, false);
				this->var.s_filter_nodrawdecal		= ggui::filter::toggle_by_name("nodrawdecal", ggui::filter::GEOMETRY, false);
				this->var.s_filter_clip				= ggui::filter::toggle_by_name("clip", ggui::filter::GEOMETRY, false);
				this->var.s_filter_hint				= ggui::filter::toggle_by_name("hintskip", ggui::filter::GEOMETRY, false);
				this->var.s_filter_portals			= ggui::filter::toggle_by_name("portals", ggui::filter::GEOMETRY, false);
			}
			
			// entity filters
			{
				this->var.s_filter_lights			= ggui::filter::toggle_by_name("lights", ggui::filter::ENTITY, false);
				this->var.s_filter_reflectionprobes = ggui::filter::toggle_by_name("reflectionprobes", ggui::filter::ENTITY, false);
				this->var.s_filter_aispawners		= ggui::filter::toggle_by_name("aispawners", ggui::filter::ENTITY, false);
				this->var.s_filter_pathnodes		= ggui::filter::toggle_by_name("pathnodes", ggui::filter::ENTITY, false);
				this->var.s_filter_actionnodes		= ggui::filter::toggle_by_name("actionnodes", ggui::filter::ENTITY, false);
				this->var.s_filter_info				= ggui::filter::toggle_by_name("info", ggui::filter::ENTITY, false);
				this->var.s_filter_mpentities		= ggui::filter::toggle_by_name("mp_entities", ggui::filter::ENTITY, false);
				this->var.s_filter_script_origins	= ggui::filter::toggle_by_name("script_origins", ggui::filter::ENTITY, false);
			}
			
			// trigger filters
			{
				this->var.s_filter_trigger			= ggui::filter::toggle_by_name("triggers", ggui::filter::TRIGGER, false);
				this->var.s_filter_bcs				= ggui::filter::toggle_by_name("bcs", ggui::filter::TRIGGER, false);
				this->var.s_filter_escort			= ggui::filter::toggle_by_name("escort", ggui::filter::TRIGGER, false);
				this->var.s_filter_unlock			= ggui::filter::toggle_by_name("unlock", ggui::filter::TRIGGER, false);
				this->var.s_filter_vehicle			= ggui::filter::toggle_by_name("vehicle", ggui::filter::TRIGGER, false);
				this->var.s_filter_friendly_respawn = ggui::filter::toggle_by_name("friendly_respawn", ggui::filter::TRIGGER, false);
				this->var.s_filter_flag				= ggui::filter::toggle_by_name("flag", ggui::filter::TRIGGER, false);
				this->var.s_filter_fog				= ggui::filter::toggle_by_name("fog", ggui::filter::TRIGGER, false);
				this->var.s_filter_auto_adjust		= ggui::filter::toggle_by_name("auto_adjust", ggui::filter::TRIGGER, false);
				this->var.s_filter_spawner			= ggui::filter::toggle_by_name("spawner", ggui::filter::TRIGGER, false);
				this->var.s_filter_stopspawner		= ggui::filter::toggle_by_name("stopspawner", ggui::filter::TRIGGER, false);
				this->var.s_filter_friendlychain	= ggui::filter::toggle_by_name("friendlychain", ggui::filter::TRIGGER, false);
				this->var.s_filter_ambient			= ggui::filter::toggle_by_name("ambient", ggui::filter::TRIGGER, false);
				this->var.s_filter_damage			= ggui::filter::toggle_by_name("damage", ggui::filter::TRIGGER, false);
				this->var.s_filter_trigger_hint		= ggui::filter::toggle_by_name("hint", ggui::filter::TRIGGER, false);
			}

			// other filters
			{
				this->var.s_filter_opt_ladder = ggui::filter::toggle_by_name("ladder", ggui::filter::OTHER, false);
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