#include "std_include.hpp"

namespace ggui::camera_settings
{
	void on_close()
	{
		for (int n = 0; n < IM_ARRAYSIZE(tab_states); n++)
		{
			tab_states[n] = false;
		}

		active_tab = -1;
	}

	void fakesun_settings()
	{
		ImGui::Indent(8.0f);

		// -----------------
		ImGui::title_with_seperator("Fakesun Settings", false);

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

	void effect_settings()
	{
		ImGui::Indent(8.0f);

		// -----------------
		ImGui::title_with_seperator("General Settings", false);

		if (ImGui::Button("Fx Reload Last Def"))
		{
			components::command::execute("fx_reload");
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

		ImGui::DragFloat("Fx Timescale", &fx_system::ed_timescale, 0.005f, 0.001f, 50.0f);

	}

	void menu(ggui::imgui_context_menu& menu)
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 220.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 700.0f);
		
		auto initial_window_pos = ggui::get_initial_window_pos();
		
		if(auto camerawnd = ggui::get_rtt_camerawnd(); 
				camerawnd)
		{
			initial_window_pos = ImVec2(camerawnd->scene_pos_imgui.x + camerawnd->scene_size_imgui.x - INITIAL_WINDOW_SIZE.x - 48.0f, camerawnd->scene_pos_imgui.y + 32.0f);
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

		if (!ImGui::Begin("Cam Toolbar Settings##cam_settings_window", &menu.menustate, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
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
}