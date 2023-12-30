#pragma once

namespace components
{
	class gameview : public component
	{
	public:
		gameview();
		~gameview();
		const char* get_name() override { return "gameview"; };
		
		static gameview* p_this;
		static gameview* get();

		[[nodiscard]] bool get_all_geo_state() const
		{
			return this->all_geo_enabled;
		}

		[[nodiscard]] bool get_all_ents_state() const
		{
			return this->all_ents_enabled;
		}

		[[nodiscard]] bool get_all_triggers_state() const
		{
			return this->all_triggers_enabled;
		}

		[[nodiscard]] bool get_all_others_state() const
		{
			return this->all_others_enabled;
		}

		void 			toggle_all_geo(bool state);
		void 			toggle_all_entities(bool state);
		void 			toggle_all_triggers(bool state);
		void 			toggle_all_others(bool state);
		void			toggle_all_filters();
		void			toggle_all_filters(bool state);
		bool			toggle_all_get_state();

		void 			set_state(bool state);
		static void		register_dvars();


	private:

		bool all_off = false;

		// all geo
		bool all_geo_enabled = false;
		bool all_geo_states[50] = {};

		// all ents
		bool all_ents_enabled = false;
		bool all_ents_states[50] = {};

		// all triggers
		bool all_triggers_enabled = false;
		bool all_triggers_states[50] = {};

		// all others
		bool all_others_enabled = false;
		bool all_others_states[50] = {};

		struct vars_s
		{
			// gameview
			bool enabled;
			bool s_model_origin;
			bool s_patch_backface_wireframe;
			int  s_xyflags;

			// geo
			bool s_filter_mantle;
			bool s_filter_nodraw;
			bool s_filter_shadowcaster;
			bool s_filter_traverse;
			bool s_filter_foliage;
			bool s_filter_weaponclip;
			bool s_filter_goal;
			bool s_filter_lightgrid;
			bool s_filter_caulk;
			bool s_filter_nodrawdecal;
			bool s_filter_clip;
			bool s_filter_hint;
			bool s_filter_portals;

			// ents
			bool s_filter_lights;
			bool s_filter_reflectionprobes;
			bool s_filter_aispawners;
			bool s_filter_pathnodes;
			bool s_filter_actionnodes;
			bool s_filter_info;
			bool s_filter_mpentities;
			bool s_filter_script_origins;

			// triggers
			bool s_filter_trigger;
			bool s_filter_bcs;
			bool s_filter_escort;
			bool s_filter_unlock;
			bool s_filter_vehicle;
			bool s_filter_friendly_respawn;
			bool s_filter_flag;
			bool s_filter_fog;
			bool s_filter_auto_adjust;
			bool s_filter_spawner;
			bool s_filter_stopspawner;
			bool s_filter_friendlychain;
			bool s_filter_ambient;
			bool s_filter_damage;
			bool s_filter_trigger_hint;

			// other (optional if installed custom filters)
			bool s_filter_opt_ladder;
			bool s_filter_opt_fx;
		};

		vars_s var = {};
	};
}