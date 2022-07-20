#pragma once

namespace ggui
{
	class camera_settings_dialog final : public ggui::ggui_module
	{
	public:
		float	sun_dir[3] = { 210.0f, 60.0f, 0.0f };
		float	sun_diffuse[3] = { 2.45f, 2.1f, 1.8f };
		float	sun_specular[4] = { 3.0f, 3.2f, 3.0f, 0.0f };
		float	material_specular[4] = { 0.35f, 0.45f, 1.0f, 4.0f };
		float	ambient[4] = { 0.3f, 0.3f, 0.3f, 0.35f };

		// *
		// physX

		float	phys_material[4];
		float	phys_plane[4];

		float	phys_debug_vis_scale;

		// *
		// *

		int		active_tab = -1;
		bool	refocus_active_tab = false;

		enum tab_state_
		{
			tab_state_fakesun = 0,
			tab_state_effects = 1,
			tab_state_bsp = 2,
		};

		inline static const char* tab_names[]
		{
			"Fakesun /PostFX",
			"Effects",
			"BSP",
		};

		bool tab_states[3]
		{
			false,
			false,
			false,
		};

		

		camera_settings_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			utils::vector::set_vec4(phys_material, 0.5f);
			utils::vector::set_vec4(phys_plane, 0.0f);
			phys_plane[2] = 1.0f;

			phys_debug_vis_scale = 1.0f;
		}


		// *
		// public member functions

		[[nodiscard]] bool get_tabstate(tab_state_ tab) const
		{
			return tab_states[tab];
		}

		[[nodiscard]] bool is_tabstate_active(tab_state_ tab) const
		{
			return active_tab == tab;
		}

		void focus_tab(tab_state_ tab)
		{
			refocus_active_tab = true;
			active_tab = tab;
		}

		void set_tabstate(tab_state_ tab, bool state)
		{
			tab_states[tab] = state;
		}

		void handle_toggle_request(tab_state_ tab)
		{
			if (get_tabstate(tab) && is_tabstate_active(tab))
			{
				// close entire window if tab is in-front
				close(); // toggle
			}
			else if (!is_active())
			{
				// open window with focused effects tab
				set_tabstate(tab, true);
				open(); // toggle
			}
			else
			{
				// window is open but tab not focused
				set_tabstate(tab, true);
				focus_tab(tab);
			}
		}

		bool gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related



		// *
		// init

		static void register_dvars();

	private:
		void fakesun_settings();
		void effect_settings();
		void bsp_settings();
	};
}
