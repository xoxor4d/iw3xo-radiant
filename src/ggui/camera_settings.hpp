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

		int		active_tab = -1;
		bool	refocus_active_tab = false;

		enum tab_state_
		{
			tab_state_fakesun = 0,
			tab_state_effects = 1,
		};

		inline static const char* tab_names[]
		{
			"Fakesun /PostFX",
			"Effects",
		};

		bool tab_states[2]
		{
			false,
			false,
		};

		

		camera_settings_dialog() { set_gui_type(GUI_TYPE_DEF); }


		// *
		// public member functions

		void set_tabstate_effects(bool _state)
		{
			tab_states[tab_state_effects] = _state;
		}

		[[nodiscard]] bool get_tabstate_effects() const
		{
			return tab_states[tab_state_effects];
		}

		[[nodiscard]] bool is_tabstate_effects_active() const 
		{
			return active_tab == tab_state_effects;
		}

		void focus_effects()
		{
			refocus_active_tab = true;
			active_tab = tab_state_effects;
		}


		void set_tabstate_fakesun(bool _state)
		{
			tab_states[tab_state_fakesun] = _state;
		}

		[[nodiscard]]  bool get_tabstate_fakesun() const
		{
			return tab_states[tab_state_fakesun];
		}

		[[nodiscard]] bool is_tabstate_fakesun_active() const
		{
			return active_tab == tab_state_fakesun;
		}

		void focus_fakesun()
		{
			refocus_active_tab = true;
			active_tab = tab_state_fakesun;
		}

		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related



		// *
		// init

	private:
		void fakesun_settings();
		void effect_settings();
	};
}
