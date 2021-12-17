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

		void 			set_state(bool state);
		static void		register_dvars();


	private:
		struct vars_s
		{
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

			// tiggers
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
		};

		vars_s var = {};
	};
}