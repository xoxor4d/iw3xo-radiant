#include "std_include.hpp"

namespace ggui
{
	void camera_settings_dialog::fakesun_settings()
	{
		ImGui::Indent(8.0f);
		ImGui::Spacing();

		// -----------------
		ImGui::title_with_seperator("Fakesun Settings", false, 0, 2, 6.0f);

		ImGui::Checkbox("Use Worldspawn Settings", &dvars::r_fakesun_use_worldspawn->current.enabled);
		TT("Uses some of the default values below if a required worldspawn key can not be found");

		ImGui::DragFloat3("Sun Dir", sun_dir, 0.1f);
		ImGui::ColorEdit3("Sun Diffuse", sun_diffuse, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		ImGui::ColorEdit4("Sun Specular", sun_specular, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

		ImGui::ColorEdit4("EnvMapParms", material_specular, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		TT(" controls specular highlights/reflections (lightSpotDir constant) \n" \
			" x: Amount of reflection looking directly at a surface  \n" \
			" y: Amount of reflection at full glancing angle \n" \
			" z: How quickly spec-strength goes from min to max as the angle approaches glancing \n" \
			" w: Sun Intensity ");

		ImGui::ColorEdit4("Ambient", ambient, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		TT(" general ambient color (lightSpotFactors constant) \n" \
			" rgb: base ambient color \n" \
			" alpha: sunlight strength ");

		// -----------------
		ImGui::title_with_seperator("Fog Settings");

		ImGui::Checkbox("Enable Fog", &dvars::r_fakesun_fog_enabled->current.enabled);
		ImGui::DragFloat("Fog Start", &dvars::r_fakesun_fog_start->current.value);
		ImGui::DragFloat("Fog Half Dist", &dvars::r_fakesun_fog_half->current.value);
		ImGui::ColorEdit4("Fog Color", dvars::r_fakesun_fog_color->current.vector, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);


		// -----------------
		ImGui::title_with_seperator("Filmtweaks");

		dvars::assign_stock_dvars();

		ImGui::Checkbox("Enable Filmtweaks", &dvars::r_filmtweakenable->current.enabled);
		ImGui::DragFloat("Brightness", &dvars::r_filmtweakbrightness->current.value, 0.005f, -1.0f, 1.0f);
		ImGui::DragFloat("Contrast", &dvars::r_filmtweakcontrast->current.value, 0.005f, 0.0f, 4.0f);
		ImGui::DragFloat("Desaturation", &dvars::r_filmtweakdesaturation->current.value, 0.005f, 0.0f, 1.0f);
		ImGui::ColorEdit3("Light Tint", dvars::r_filmtweaklighttint->current.vector, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		ImGui::ColorEdit3("Dark Tint", dvars::r_filmtweakdarktint->current.vector, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

		SPACING(0.0f, 2.0f);
	}

	void camera_settings_dialog::effect_settings()
	{
		const auto& style = ImGui::GetStyle();

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		// -----------------
		ImGui::title_with_seperator("General", false, 0, 2, 6.0f);

		ImGui::BeginDisabled(!components::effects::effect_can_play());
		{
			if (ImGui::Button("Reload Effect"))
			{
				if (components::effects_editor::is_editor_active())
				{
					GET_GUI(ggui::effects_editor_dialog)->m_pending_reload = true;
				}
				else
				{
					components::command::execute("fx_reload");
				}
			}

			ImGui::EndDisabled();
		}
		

		ImGui::SameLine();
		if (ImGui::Button("Toggle Show Tris"))
		{
			auto tris = game::Dvar_FindVar("r_showTris");
			if (tris)
			{
				dvars::set_int(tris, tris->current.integer ? 0 : 1);
			}
		}

		ImGui::DragFloat("Timescale", &fx_system::ed_timescale, 0.005f, 0.001f, 50.0f);
		ImGui::DragFloat("Repeat Delay", &fx_system::ed_looppause, 0.01f, 0.05f, FLT_MAX, "%.2f");

		ImGui::BeginDisabled(!components::effects::effect_can_play());
		{
			static int tick_rate = 50;
			static int held_timeout = 0;

			if (ImGui::Button("Advance Tick")) 
			{
				fx_system::ed_playback_tick += tick_rate;
			}
			else if(ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{ 
					if(held_timeout > 50)
					{
						fx_system::ed_playback_tick += tick_rate;
					}
					else
					{
						held_timeout++;
					}
				}
				else
				{
					held_timeout = 0;
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(-style.FramePadding.x);
			ImGui::DragInt("##tick_rate", &tick_rate, 1, 1, INT16_MAX);
			TT("Tick Increase Amount\nIncrease effect ticks by hand. Useful when effect is paused");

			ImGui::EndDisabled();
		}
	}

	void camera_settings_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 220.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 700.0f);
		
		auto initial_window_pos = ggui::get_initial_window_pos();
		
		if(const auto	camerawnd = GET_GUI(ggui::camera_dialog);
						camerawnd)
		{
			initial_window_pos = ImVec2(
				camerawnd->rtt_get_position().x + camerawnd->rtt_get_size().x - INITIAL_WINDOW_SIZE.x - 48.0f,
				camerawnd->rtt_get_position().y + 32.0f);
		}

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(initial_window_pos, ImGuiCond_FirstUseEver);

		// do not show window if no tabs are open

		bool any_open = false;
		for (int n = 0; n < IM_ARRAYSIZE(tab_states); n++)
		{
			if(tab_states[n])
			{
				any_open = true;
				break;
			}
		}

		if(!any_open)
		{
			return;
		}

		if (!ImGui::Begin("Cam Toolbar Settings##cam_settings_window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings /*| ImGuiWindowFlags_NoTitleBar*/))
		{
			ImGui::End();
			return;
		}

		static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyResizeDown;

		if (ImGui::BeginTabBar("##camera_window_tabbar", tab_bar_flags))
		{
			for (int n = 0; n < IM_ARRAYSIZE(tab_states); n++)
			{
				ImGuiTabBarFlags flags = ImGuiTabItemFlags_None;
				if(refocus_active_tab && n == active_tab)
				{
					flags |= ImGuiTabItemFlags_SetSelected;
					refocus_active_tab = false;
				}

				if (tab_states[n] && ImGui::BeginTabItem(tab_names[n], &tab_states[n], flags))
				{
					if(!refocus_active_tab)
					{
						active_tab = n;
					}

					switch(n)
					{
					case tab_state_fakesun:
						fakesun_settings();
						break;

					case tab_state_effects:
						effect_settings();
						break;

					default:
						ImGui::Text("Unhandled Tab");
					}

					
					ImGui::EndTabItem();
				}
			}
				
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	void camera_settings_dialog::on_open()
	{ }

	void camera_settings_dialog::on_close()
	{
		for (int n = 0; n < IM_ARRAYSIZE(tab_states); n++)
		{
			tab_states[n] = false;
		}

		active_tab = -1;
	}

	REGISTER_GUI(camera_settings_dialog);
}