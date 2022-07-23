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

		if(components::d3dbsp::Com_IsBspLoaded() && dvars::r_draw_bsp->current.enabled)
		{
			ImGui::Checkbox("d3dbsp: overwrite sundir", &dvars::r_draw_bsp_overwrite_sundir->current.enabled);
			TT(dvars::r_draw_bsp_overwrite_sundir->description);

			ImGui::Checkbox("d3dbsp: overwrite sunlight", &dvars::r_draw_bsp_overwrite_sunlight->current.enabled);
			TT(dvars::r_draw_bsp_overwrite_sunlight->description);

			ImGui::Checkbox("d3dbsp: overwrite sunspecular", &dvars::r_draw_bsp_overwrite_sunspecular->current.enabled);
			TT(dvars::r_draw_bsp_overwrite_sunspecular->description);
		}

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

	// --------------------

	void camera_settings_dialog::effect_settings()
	{
		//const auto& style = ImGui::GetStyle();

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

		float general_widget_width;

		ImGui::BeginDisabled(!components::effects::effect_can_play());
		{
			static int tick_rate = 50;
			static int held_timeout = 0;

			ImGui::DragInt("##tick_rate", &tick_rate, 1, 1, INT16_MAX);
			TT("Tick Increase Amount\nIncrease effect ticks by hand. Useful when effect is paused");

			general_widget_width = ImGui::GetItemRectSize().x;
			ImGui::SameLine();
			ImGui::TextUnformatted("Tick Amount");

			if (ImGui::Button("Advance Tick", ImVec2(general_widget_width, ImGui::GetFrameHeight())))
			{
				fx_system::ed_playback_tick += tick_rate;
			}
			else if (ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{
					if (held_timeout > 50)
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

			ImGui::EndDisabled();
		}

		// -----------------
		ImGui::title_with_seperator("PhysX :: Groundplane Material Properties", true, 0.0f, 2.0f, 8.0f);

		const auto phys = components::physx_impl::get();
		//const auto separator_width = ImGui::GetContentRegionAvail().x - 8.0f;
		//ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));

		ImGui::DragFloat("Static Friction", &phys_material[0], 0.05f, 0.0f, 100.0f, "%.2f");
		ImGui::DragFloat("Dynamic Friction", &phys_material[1], 0.05f, 0.0f, 100.0f, "%.2f");
		ImGui::DragFloat("Restitution", &phys_material[2], 0.05f, 0.0f, 100.0f, "%.2f");

		SPACING(0.0f, 2.0f);

		ImGui::DragFloat3("Plane XYZ", phys_plane, 0.05f, 0.0f, 1.0f, "%.2f");
		ImGui::DragFloat("Plane Distance", &phys_plane[3], 0.05f, 0.0f, 1000.0f, "%.2f");

		if (ImGui::Button("Create Plane", ImVec2(general_widget_width, ImGui::GetFrameHeight())))
		{
			components::command::execute("physx_plane");
		}


		// #
		ImGui::title_with_seperator("PhysX :: General World Settings", true, 0.0f, 2.0f, 8.0f);

		static float physx_gravity[3] = { 0.0f, 0.0f, -800.0f };
		if (ImGui::DragFloat3("Gravity", physx_gravity, 0.05f, -4000.0f, 4000.0f, "%.2f"))
		{
			phys->mScene->setGravity(physx::PxVec3(physx_gravity[0], physx_gravity[1], physx_gravity[2]));
		}

		auto bounce_threshold = phys->mScene->getBounceThresholdVelocity(); // 20
		if (ImGui::DragFloat("Bounce Threshold", &bounce_threshold, 0.025f, 0.0f, 1000.0f, "%.2f"))
		{
			phys->mScene->setBounceThresholdVelocity(bounce_threshold);
		}

		auto friction_threshold = phys->mScene->getFrictionOffsetThreshold(); // 0.0399999991
		ImGui::DragFloat("Friction Threshold", &friction_threshold, 0.025f, 0.0f, 1000.0f, "%.2f");


		// #
		ImGui::title_with_seperator("PhysX :: Debug Visuals", true, 0.0f, 2.0f, 8.0f);

		static bool physx_draw_debug = false;
		if (ImGui::Checkbox("Enable debug visuals", &physx_draw_debug))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, physx_draw_debug ? 1.0f : 0.0f);
		}

		static bool physx_draw_debug_shapes = false;
		if (ImGui::Checkbox("Draw shapes", &physx_draw_debug_shapes))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, physx_draw_debug_shapes ? phys_debug_vis_scale : 0.0f);
		}

		static bool physx_draw_debug_aabbs = false;
		if (ImGui::Checkbox("Draw aabbs", &physx_draw_debug_aabbs))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, physx_draw_debug_aabbs ? phys_debug_vis_scale : 0.0f);
		}

		static bool physx_draw_debug_contacts = false;
		if (ImGui::Checkbox("Draw contacts", &physx_draw_debug_contacts))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCONTACT_POINT, physx_draw_debug_contacts ? phys_debug_vis_scale : 0.0f);
		}

		const auto text_width = ImGui::CalcTextSize("Visualization Box Size").x;
		ImGui::SetNextItemWidth(general_widget_width - text_width - 8.0f);
		if (ImGui::DragFloat("Visualization Box Size", &dvars::physx_debug_visualization_box_size->current.value, 0.5f, 0.0f, FLT_MAX, "%.2f"))
		{
			const auto cam = &cmainframe::activewnd->m_pCamWnd->camera;
			const auto cbox_size = dvars::physx_debug_visualization_box_size->current.value * 0.5f;

			const auto cbox = physx::PxBounds3(
				physx::PxVec3(cam->origin[0] - cbox_size, cam->origin[1] - cbox_size, cam->origin[2] - cbox_size),
				physx::PxVec3(cam->origin[0] + cbox_size, cam->origin[1] + cbox_size, cam->origin[2] + cbox_size));

			components::physx_impl::get()->mScene->setVisualizationCullingBox(cbox);
		}

		// #
		// TODO:

		if (ImGui::Button("Create Static Collision", ImVec2(general_widget_width, ImGui::GetFrameHeight())))
		{
			std::thread(components::physx_impl::create_static_collision).detach();
		}

		//ImGui::PopStyleColor(); // Separator
	}

	// --------------------


	void camera_settings_dialog::bsp_settings()
	{
		const auto& style = ImGui::GetStyle();
		const float second_column = ImGui::GetWindowContentRegionWidth() * 0.5f;

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		// -----------------
		ImGui::title_with_seperator("BSP", false, 0, 2, 6.0f);

		ImGui::Dvar("Compile BSP", dvars::bsp_compile_bsp);
		ImGui::SameLine(second_column);
		ImGui::Dvar("Only Ents", dvars::bsp_compile_onlyents);

		ImGui::Dvar("Samplescale", dvars::bsp_compile_samplescale_enabled);
		ImGui::SameLine(second_column);
		ImGui::BeginDisabled(!dvars::bsp_compile_samplescale_enabled->current.enabled);
		{
			ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX() - 4.0f);
			ImGui::Dvar("##num_samplescale", dvars::bsp_compile_samplescale);
			ImGui::EndDisabled();
		}

		SPACING(0.0f, 4.0f);

		ImGui::Dvar("Custom Commandline Settings - BSP", dvars::bsp_compile_custom_cmd_enabled);
		ImGui::BeginDisabled(!dvars::bsp_compile_custom_cmd_enabled->current.enabled);
		{
			std::string temp_string = dvars::bsp_compile_custom_cmd->current.string;
			ImGui::SetNextItemWidth(-8);
			if(ImGui::InputText("##bsp_commandline", &temp_string, ImGuiInputTextFlags_None))
			{
				dvars::set_string(dvars::bsp_compile_custom_cmd, temp_string.c_str());
			}
			ImGui::EndDisabled();
		}


		// -----------------
		ImGui::title_with_seperator("Light", true, 0, 2, 6.0f);

		ImGui::Dvar("Compile Light", dvars::bsp_compile_light);

		ImGui::Dvar("Fast", dvars::bsp_compile_light_fast);
		ImGui::SameLine(second_column);
		ImGui::Dvar("Extra", dvars::bsp_compile_light_extra);

		ImGui::Dvar("Modelshadow", dvars::bsp_compile_light_modelshadow);
		ImGui::SameLine(second_column);
		ImGui::Dvar("Dump Options", dvars::bsp_compile_light_dump);

		ImGui::Dvar("Traces", dvars::bsp_compile_light_traces_enabled);
		ImGui::SameLine(second_column);
		ImGui::BeginDisabled(!dvars::bsp_compile_light_traces_enabled->current.enabled);
		{
			ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX() - 4.0f);
			ImGui::Dvar("##num_traces", dvars::bsp_compile_light_traces);
			ImGui::EndDisabled();
		}

		SPACING(0.0f, 4.0f);

		ImGui::Dvar("Custom Commandline Settings - Light", dvars::bsp_compile_light_custom_cmd_enabled);
		ImGui::BeginDisabled(!dvars::bsp_compile_light_custom_cmd_enabled->current.enabled);
		{
			std::string temp_string = dvars::bsp_compile_light_custom_cmd->current.string;
			ImGui::SetNextItemWidth(-style.FramePadding.x);

			if(ImGui::InputText("##light_commandline", &temp_string, ImGuiInputTextFlags_None))
			{
				dvars::set_string(dvars::bsp_compile_light_custom_cmd, temp_string.c_str());
			}
			ImGui::EndDisabled();
		}

		// -----------------
		ImGui::title_with_seperator("Reflections", true, 0, 2, 6.0f);

		ImGui::Checkbox("Automatically compile reflections when building bsp", &dvars::bsp_gen_reflections_on_compile->current.enabled); TT(dvars::bsp_gen_reflections_on_compile->description);

		if (ImGui::Button("Generate Reflections", ImVec2(-style.FramePadding.x, ImGui::GetFrameHeight())))
		{
			dvars::set_bool(dvars::r_reflectionprobe_generate, true);
		} TT("Probes within the loaded bsp will be used to take screenshots ..\nso make sure your bsp is up-to-date.");


		// -----------------
		ImGui::title_with_seperator("Compiling", true, 0, 2, 6.0f);


		const bool can_compile = true; //components::d3dbsp::Com_IsBspLoaded() && !components::d3dbsp::loaded_bsp_path.empty();
		ImGui::BeginDisabled(!can_compile);
		{
			//const std::string d3dbsp_name = components::d3dbsp::loaded_bsp_path.substr(components::d3dbsp::loaded_bsp_path.find_last_of("\\") + 1);
			std::string d3dbsp_name = std::string(game::current_map_filepath).substr(std::string(game::current_map_filepath).find_last_of("\\") + 1);
			utils::erase_substring(d3dbsp_name, ".map");

			const std::string button_str = can_compile ? ("Compile " + d3dbsp_name) : "Compile BSP";

			if(ImGui::Button(button_str.c_str(), ImVec2(-style.FramePadding.x, ImGui::GetFrameHeight())))
			{
				components::d3dbsp::compile_bsp(d3dbsp_name);
			}

			ImGui::EndDisabled();
		}
	}
	
	// --------------------

	bool camera_settings_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 220.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 720.0f); 
		
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
			return false;
		}

		if (!ImGui::Begin("Cam Toolbar Settings##cam_settings_window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings /*| ImGuiWindowFlags_NoTitleBar*/))
		{
			ImGui::End();
			return false;
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

					case tab_state_bsp:
						bsp_settings();
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

		return true;
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

	void camera_settings_dialog::register_dvars()
	{
		dvars::bsp_compile_bsp = dvars::register_bool(
			/* name		*/ "bsp_compile_bsp",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp compile setting");

		dvars::bsp_compile_onlyents = dvars::register_bool(
			/* name		*/ "bsp_compile_onlyents",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp compile setting");

		dvars::bsp_compile_samplescale_enabled = dvars::register_bool(
			/* name		*/ "bsp_compile_samplescale_enabled",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp compile setting");

		dvars::bsp_compile_samplescale = dvars::register_float(
			/* name		*/ "bsp_compile_samplescale",
			/* default	*/ 1.0f,
			/* mins		*/ 0.1f,
			/* maxs		*/ 16.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp compile setting");

		dvars::bsp_compile_custom_cmd_enabled = dvars::register_bool(
			/* name		*/ "bsp_compile_custom_cmd_enabled",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp compile setting");

		dvars::bsp_compile_custom_cmd = dvars::register_string(
			/* name		*/ "bsp_compile_custom_cmd",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp compile setting");




		dvars::bsp_compile_light = dvars::register_bool(
			/* name		*/ "bsp_compile_light",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_fast = dvars::register_bool(
			/* name		*/ "bsp_compile_light_fast",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_extra = dvars::register_bool(
			/* name		*/ "bsp_compile_light_extra",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_modelshadow = dvars::register_bool(
			/* name		*/ "bsp_compile_light_modelshadow",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_dump = dvars::register_bool(
			/* name		*/ "bsp_compile_light_dump",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_traces_enabled = dvars::register_bool(
			/* name		*/ "bsp_compile_light_traces_enabled",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_traces = dvars::register_int(
			/* name		*/ "bsp_compile_light_traces",
			/* default	*/ 64,
			/* min		*/ 1,
			/* max		*/ 512,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_custom_cmd_enabled = dvars::register_bool(
			/* name		*/ "bsp_compile_light_custom_cmd_enabled",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");

		dvars::bsp_compile_light_custom_cmd = dvars::register_string(
			/* name		*/ "bsp_compile_light_custom_cmd",
			/* default	*/ "",
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "bsp light compile setting");
	}

	REGISTER_GUI(camera_settings_dialog);
}