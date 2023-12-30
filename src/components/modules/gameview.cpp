#include "std_include.hpp"

namespace components
{
	gameview* gameview::p_this = nullptr;

	gameview* gameview::get()
	{
		return p_this;
	}

	void gameview::toggle_all_geo(bool state)
	{
		if (!gameview::p_this)
		{
			game::printf_to_console("[ERR] Gameview module seems to be disabled ...");
			return;
		}

		// t off
		if (!state)
		{
			int i_geo = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for(const auto& f_entry : filter->_geofilters)
			{
				if(i_geo < 50)
				{
					if (this->all_geo_states[i_geo])
					{
						filter->toggle_by_name(f_entry->name, ggui::filter_dialog::GEOMETRY, true);
					}

					i_geo++;
				}
				else
				{
					break;
				}
			}

			this->all_geo_enabled = false;
		}

		// toggle on
		else
		{
			this->set_state(false); // turn off gameview

			int i_geo = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for (const auto& f_entry : filter->_geofilters)
			{
				if (i_geo < 50)
				{
					if(f_entry->name != "NonDecals"s)
					{
						this->all_geo_states[i_geo] = filter->toggle_by_name(f_entry->name, ggui::filter_dialog::GEOMETRY, false);
					}

					i_geo++;
				}
				else
				{
					break;
				}
			}

			this->all_geo_enabled = true;
		}
	}

	void gameview::toggle_all_entities(bool state)
	{
		if (!gameview::p_this)
		{
			game::printf_to_console("[ERR] Gameview module seems to be disabled ...");
			return;
		}

		// t off
		if (!state)
		{
			int i_ents = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for (const auto& f_entry : filter->_entityfilters)
			{
				if (i_ents < 50)
				{
					if (this->all_ents_states[i_ents])
					{
						filter->toggle_by_name(f_entry->name, ggui::filter_dialog::ENTITY, true);
					}

					i_ents++;
				}
				else
				{
					break;
				}
			}

			this->all_ents_enabled = false;
		}

		// toggle on
		else
		{
			this->set_state(false); // turn off gameview

			int i_ents = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for (const auto& f_entry : filter->_entityfilters)
			{
				if (i_ents < 50)
				{
					if (f_entry->name != "Entities"s)
					{
						this->all_ents_states[i_ents] = filter->toggle_by_name(f_entry->name, ggui::filter_dialog::ENTITY, false);
					}
					
					i_ents++;
				}
				else
				{
					break;
				}
			}

			this->all_ents_enabled = true;
		}
	}

	void gameview::toggle_all_triggers(bool state)
	{
		if (!gameview::p_this)
		{
			game::printf_to_console("[ERR] Gameview module seems to be disabled ...");
			return;
		}

		// t off
		if (!state)
		{
			int i_triggers = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for (const auto& f_entry : filter->_triggerfilters)
			{
				if (i_triggers < 50)
				{
					if (this->all_triggers_states[i_triggers])
					{
						filter->toggle_by_name(f_entry->name, ggui::filter_dialog::TRIGGER, true);
					}

					i_triggers++;
				}
				else
				{
					break;
				}
			}

			this->all_triggers_enabled = false;
		}

		// toggle on
		else
		{
			this->set_state(false); // turn off gameview

			int i_triggers = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for (const auto& f_entry : filter->_triggerfilters)
			{
				if (i_triggers < 50)
				{
					this->all_triggers_states[i_triggers] = filter->toggle_by_name(f_entry->name, ggui::filter_dialog::TRIGGER, false);
					i_triggers++;
				}
				else
				{
					break;
				}
			}

			this->all_triggers_enabled = true;
		}
	}

	void gameview::toggle_all_others(bool state)
	{
		if (!gameview::p_this)
		{
			game::printf_to_console("[ERR] Gameview module seems to be disabled ...");
			return;
		}

		// t off
		if (!state)
		{
			int i_others = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for (const auto& f_entry : filter->_otherfilters)
			{
				if (i_others < 50)
				{
					if (this->all_others_states[i_others])
					{
						filter->toggle_by_name(f_entry->name, ggui::filter_dialog::OTHER, true);
					}

					i_others++;
				}
				else
				{
					break;
				}
			}

			this->all_others_enabled = false;
		}

		// toggle on
		else
		{
			this->set_state(false); // turn off gameview

			int i_others = 0;
			const auto filter = GET_GUI(ggui::filter_dialog);

			for (const auto& f_entry : filter->_otherfilters)
			{
				if (i_others < 50)
				{
					if (f_entry->name != "FX"s)
					{
						this->all_others_states[i_others] = filter->toggle_by_name(f_entry->name, ggui::filter_dialog::OTHER, false);
					}

					i_others++;
				}
				else
				{
					break;
				}
			}

			this->all_others_enabled = true;
		}
	}

	void gameview::toggle_all_filters()
	{
		// restore states if was toggled manually
		if (gameview::get_all_geo_state())		gameview::toggle_all_geo(false);
		if (gameview::get_all_ents_state())		gameview::toggle_all_entities(false);
		if (gameview::get_all_triggers_state()) gameview::toggle_all_triggers(false);
		if (gameview::get_all_others_state())	gameview::toggle_all_others(false);

		// toggle global
		gameview::toggle_all_geo(!gameview::all_off);
		gameview::toggle_all_entities(!gameview::all_off);
		gameview::toggle_all_triggers(!gameview::all_off);
		gameview::toggle_all_others(!gameview::all_off);

		gameview::all_off = !gameview::all_off;
	}

	void gameview::toggle_all_filters(bool state)
	{
		// restore states if was toggled manually
		if (gameview::get_all_geo_state())		gameview::toggle_all_geo(false);
		if (gameview::get_all_ents_state())		gameview::toggle_all_entities(false);
		if (gameview::get_all_triggers_state()) gameview::toggle_all_triggers(false);
		if (gameview::get_all_others_state())	gameview::toggle_all_others(false);

		// toggle global
		gameview::toggle_all_geo(state);
		gameview::toggle_all_entities(state);
		gameview::toggle_all_triggers(state);
		gameview::toggle_all_others(state);

		gameview::all_off = state;
	}

	bool gameview::toggle_all_get_state()
	{
		return all_off;
	}


	void gameview::set_state(bool state)
	{
		const auto filter = GET_GUI(ggui::filter_dialog);
		filter->build_radiant_filterlists();

		if(!gameview::p_this)
		{
			game::printf_to_console("[ERR] Gameview module seems to be disabled ...");
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
				if (this->var.s_filter_opt_ladder)			filter->toggle_by_name("ladder", ggui::filter_dialog::OTHER, true);
				//if (this->var.s_filter_opt_fx)				filter->toggle_by_name("FX", ggui::filter_dialog::OTHER, true);
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
				this->var.s_filter_opt_ladder		= filter->toggle_by_name("ladder", ggui::filter_dialog::OTHER, false);
				//this->var.s_filter_opt_fx			= filter->toggle_by_name("FX", ggui::filter_dialog::OTHER, false);
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
			gameview::set_state(!dvars::radiant_gameview->current.enabled);
		});

		command::register_command_with_hotkey("toggle_filter_geo"s, [this](auto)
		{
			gameview::toggle_all_geo(!gameview::get_all_geo_state());
		});

		command::register_command_with_hotkey("toggle_filter_ents"s, [this](auto)
		{
			gameview::toggle_all_entities(!gameview::get_all_ents_state());
		});

		command::register_command_with_hotkey("toggle_filter_trigger"s, [this](auto)
		{
			gameview::toggle_all_triggers(!gameview::get_all_triggers_state());
		});

		command::register_command_with_hotkey("toggle_filter_others"s, [this](auto)
		{
			gameview::toggle_all_others(!gameview::get_all_others_state());
		});

		command::register_command_with_hotkey("toggle_filter_all"s, [this](auto)
		{
			gameview::toggle_all_filters();
		});
	}

	gameview::~gameview()
	{ }
}