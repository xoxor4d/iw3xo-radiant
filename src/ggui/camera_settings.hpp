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

		bool	m_bsp_bsp_compile = true;
		bool	m_bsp_bsp_only_ents = false;
		bool	m_bsp_bsp_custom_cmd_enabled = false;
		bool	m_bsp_bsp_samplescale_enabled = false;
		float	m_bsp_bsp_samplescale = 1.0f;
		std::string m_bsp_bsp_custom_cmd;

		bool	m_bsp_light_compile = true;
		bool	m_bsp_light_fast = true;
		bool	m_bsp_light_extra = false;
		bool	m_bsp_light_modelshadow = false;
		bool	m_bsp_light_dump = false;
		bool	m_bsp_light_custom_cmd_enabled = false;
		bool	m_bsp_light_traces_enabled = false;
		int		m_bsp_light_traces = 64;
		std::string m_bsp_light_custom_cmd;

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

		

		camera_settings_dialog() { set_gui_type(GUI_TYPE_DEF); }


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

	private:
		void fakesun_settings();
		void effect_settings();
		void bsp_settings();
	};
}
