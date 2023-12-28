#include "std_include.hpp"

namespace ggui
{
#define SET_WIDGET_WIDTH_WITH_LABEL(label)			 \
		label_size = imgui::CalcTextSize((label)).x; \
		imgui::SetNextItemWidth(general_widget_width - label_size + label_left_offset);

#define SET_WIDGET_WIDTH_REPEAT()					 \
		imgui::SetNextItemWidth(general_widget_width - label_size + label_left_offset);

#define GET_WIDGET_WIDTH()							 \
		general_widget_width - label_size + label_left_offset

	void camera_settings_dialog::fakesun_settings()
	{
		imgui::Indent(8.0f);
		imgui::Spacing();

		// -----------------
		imgui::title_with_seperator("Fakesun Settings", false, 0, 2, 6.0f);

		imgui::Checkbox("Use Worldspawn Settings", &dvars::r_fakesun_use_worldspawn->current.enabled);
		TT("Uses some of the default values below if a required worldspawn key can not be found");

		if(components::d3dbsp::Com_IsBspLoaded() && dvars::r_draw_bsp->current.enabled)
		{
			imgui::Checkbox("d3dbsp: overwrite sundir", &dvars::r_draw_bsp_overwrite_sundir->current.enabled);
			TT(dvars::r_draw_bsp_overwrite_sundir->description);

			imgui::Checkbox("d3dbsp: overwrite sunlight", &dvars::r_draw_bsp_overwrite_sunlight->current.enabled);
			TT(dvars::r_draw_bsp_overwrite_sunlight->description);

			imgui::Checkbox("d3dbsp: overwrite sunspecular", &dvars::r_draw_bsp_overwrite_sunspecular->current.enabled);
			TT(dvars::r_draw_bsp_overwrite_sunspecular->description);
		}

		imgui::DragFloat3("Sun Dir", sun_dir, 0.1f);
		imgui::ColorEdit3("Sun Diffuse", sun_diffuse, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		imgui::ColorEdit4("Sun Specular", sun_specular, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

		imgui::ColorEdit4("EnvMapParms", material_specular, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		TT(" controls specular highlights/reflections (lightSpotDir constant) \n" \
			" x: Amount of reflection looking directly at a surface  \n" \
			" y: Amount of reflection at full glancing angle \n" \
			" z: How quickly spec-strength goes from min to max as the angle approaches glancing \n" \
			" w: Sun Intensity ");

		imgui::ColorEdit4("Ambient", ambient, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		TT(" general ambient color (lightSpotFactors constant) \n" \
			" rgb: base ambient color \n" \
			" alpha: sunlight strength ");

		// -----------------
		imgui::title_with_seperator("Fog Settings");

		imgui::Checkbox("Enable Fog", &dvars::r_fakesun_fog_enabled->current.enabled);
		imgui::DragFloat("Fog Start", &dvars::r_fakesun_fog_start->current.value);
		imgui::DragFloat("Fog Half Dist", &dvars::r_fakesun_fog_half->current.value);
		imgui::ColorEdit4("Fog Color", dvars::r_fakesun_fog_color->current.vector, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);


		// -----------------
		imgui::title_with_seperator("Filmtweaks");

		dvars::assign_stock_dvars();

		imgui::Checkbox("Enable Filmtweaks", &dvars::r_filmtweakenable->current.enabled);
		imgui::DragFloat("Brightness", &dvars::r_filmtweakbrightness->current.value, 0.005f, -1.0f, 1.0f);
		imgui::DragFloat("Contrast", &dvars::r_filmtweakcontrast->current.value, 0.005f, 0.0f, 4.0f);
		imgui::DragFloat("Desaturation", &dvars::r_filmtweakdesaturation->current.value, 0.005f, 0.0f, 1.0f);
		imgui::ColorEdit3("Light Tint", dvars::r_filmtweaklighttint->current.vector, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
		imgui::ColorEdit3("Dark Tint", dvars::r_filmtweakdarktint->current.vector, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

		SPACING(0.0f, 2.0f);
	}

	// --------------------

	void camera_settings_dialog::effect_settings()
	{
		//const auto& style = imgui::GetStyle();
		const auto phys = components::physx_impl::get();

		const float label_left_offset = 84.0f;
		const auto separator_with_text_color = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);

		static float general_widget_width = 160.0f;
		float label_size;

		imgui::Indent(8.0f);
		imgui::Spacing();

		// -----------------
		imgui::title_with_seperator("Effect Settings", false, 0, 2, 6.0f);

		imgui::BeginDisabled(!components::effects::effect_can_play());
		{
			if (imgui::Button("Reload Effect", ImVec2(general_widget_width * 0.5f - 5.0f, 0.0f)))
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

			imgui::EndDisabled();
		}
		

		imgui::SameLine();
		if (imgui::Button("Toggle Show Tris", ImVec2(general_widget_width * 0.5f - 5.0f, 0.0f)))
		{
			if (const auto& tris = game::Dvar_FindVar("r_showTris"); 
				tris)
			{
				dvars::set_int(tris, tris->current.integer ? 0 : 1);
			}
		}

		imgui::DragFloat("Repeat Delay", &fx_system::ed_looppause, 0.01f, 0.05f, FLT_MAX, "%.2f");

		SPACING(0.0f, 8.0f);

		imgui::PushStyleColor(ImGuiCol_Separator, separator_with_text_color);
		ImGui::title_inside_seperator("Effects & PhysX", false, ImGui::GetContentRegionAvail().x - 16.0f, 24.0f, 1.0f);
		imgui::PopStyleColor();

		imgui::DragFloat("Timescale", &fx_system::ed_timescale, 0.005f, 0.001f, 50.0f);

		//imgui::BeginDisabled(!components::effects::effect_can_play());
		{
			static int tick_rate = 5;
			static int held_timeout = 0;

			imgui::DragInt("##tick_rate", &tick_rate, 1, 1, INT16_MAX);
			TT("Tick Increase Amount\nIncrease effect ticks by hand. Useful when effect is paused");

			general_widget_width = imgui::GetItemRectSize().x;
			imgui::SameLine();
			imgui::TextUnformatted("Tick Amount");

			if (imgui::Button("Advance Tick", ImVec2(general_widget_width, imgui::GetFrameHeight())))
			{
				fx_system::ed_playback_tick += tick_rate;
				phys->m_phys_sim_tick += tick_rate;
			}
			else if (imgui::IsItemHovered())
			{
				if (imgui::IsMouseDown(ImGuiMouseButton_Left))
				{
					if (held_timeout > 30) 
					{
						fx_system::ed_playback_tick += tick_rate;
						phys->m_phys_sim_tick += tick_rate;
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
			} TT("hold left mouse to continuously increase ticks by x amount");

			//imgui::EndDisabled();
		}

		// -----------------
		imgui::title_with_seperator("PhysX :: General", true, 0.0f, 2.0f, 8.0f);

		bool is_prefab_selected = false;

		if (const auto sb = game::g_selected_brushes(); 
			sb && sb->owner && sb->owner->prefab)
		{
			is_prefab_selected = true;
		}

		const bool single_brush_selected = game::is_single_brush_selected(false);

		SET_WIDGET_WIDTH_WITH_LABEL("");

		//SPACING(0.0f, 6.0f);
		imgui::Indent(4.0f);

		imgui::Text("Static Brushes: %d", phys->m_static_brush_count);

		const char* num_patches_str = utils::va("Static Patches: %d", phys->m_static_terrain_count);
		imgui::SameLine(GET_WIDGET_WIDTH() - imgui::CalcTextSize(num_patches_str).x);
		imgui::Text(num_patches_str);

		imgui::Unindent(4.0f);
		SPACING(0.0f, 6.0f);

		if (imgui::Button("Generate Static Collision", ImVec2(GET_WIDGET_WIDTH(), imgui::GetFrameHeight())))
		{
			const auto process = components::process::get();

			process->set_indicator(components::process::INDICATOR_TYPE_PROGRESS);
			process->set_indicator_string("Building Static Collision");
			process->set_process_type(components::process::PROC_TYPE_GENERIC);
			process->set_success_toast_string("Successfully built static collision");

			process->set_thread_callback([]
				{
					components::physx_impl::create_static_collision();
				});

			process->set_progress_callback([]
				{
					const auto current = static_cast<float>(components::physx_impl::get()->m_static_brush_count + components::physx_impl::get()->m_static_terrain_count);
					const auto total = static_cast<float>(components::physx_impl::get()->m_static_brush_estimated_count + components::physx_impl::get()->m_static_terrain_estimated_count);
					components::process::get()->m_indicator_progress = current / total;
				});

			process->create_process();
		} TT(	"generate collision data for all non-selected brushes,\n"
				"terrain and curve patches, so that dynamic actors collide with the world");

		


		// ---------------------------

		SPACING(0.0f, 8.0f);

		imgui::PushStyleColor(ImGuiCol_Separator, separator_with_text_color);
		ImGui::title_inside_seperator("Prefab PhysX", false, ImGui::GetContentRegionAvail().x - 16.0f, 24.0f, 1.0f);
		imgui::PopStyleColor();


		const auto icon_fa_size = ImVec2(36.0f, 32.0f);
		imgui::BeginDisabled(phys->m_dynamic_prefabs.empty());
		imgui::PushCompactButtonInvBg();
		{
			bool& run_sim = components::physx_impl::get()->m_phys_sim_run;
			imgui::PushStyleColor(ImGuiCol_Text, run_sim ? ImVec4(0.44f, 0.69f, 0.0f, 1.0f) : ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
			{
				if (imgui::Button(ICON_FA_PLAY, icon_fa_size))
				{

					run_sim = run_sim ? false : true;
				}

				imgui::PopStyleColor();
			} TT("run PhysX simulation for all PhysX prefab actors in the world");


			imgui::SameLine();
			bool& pause_sim = components::physx_impl::get()->m_phys_sim_pause;
			imgui::PushStyleColor(ImGuiCol_Text, pause_sim ? ImVec4(0.28f, 0.5f, 0.75f, 1.0f) : ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
			{
				if (imgui::ButtonEx(ICON_FA_PAUSE, icon_fa_size))
				{

					pause_sim = pause_sim ? false : true;
				}

				imgui::PopStyleColor();
			} TT("pause prefab PhysX simulation (useful to manually advance Ticks)");


			imgui::SameLine();
			if (imgui::Button(ICON_FA_UNDO_ALT, icon_fa_size))
			{
				phys->reset_dynamic_prefabs();
			} TT("resets all prefab entites to their initial position/angles upon creation");

			imgui::PopCompactButtonInvBg();
			imgui::EndDisabled();
		}

		imgui::SameLine();

		imgui::BeginGroup(); // tooltip
		imgui::BeginDisabled(!(is_prefab_selected && !components::effects::effect_is_playing()));
		{
			if (imgui::Button("Convert Prefabs to PhysX actors", ImVec2(general_widget_width - 34.0f, icon_fa_size.y)))
			{
				phys->clear_dynamic_prefabs(false);

				FOR_ALL_SELECTED_BRUSHES(sb)
				{
					if (sb && sb->def)
					{
						phys->create_dynamic_prefab(sb);
					}
				}

			} 

			imgui::EndDisabled();
			imgui::EndGroup();
			TT(	"convert all selected prefabs to PhysX actors\n"
				"@ make sure to re-generate static collision to exclude the selected prefabs\n"
				"@ only available if effects are stopped or paused");
		}
		
		

		// ---------------------------

		SPACING(0.0f, 8.0f);

		imgui::PushStyleColor(ImGuiCol_Separator, separator_with_text_color);
		ImGui::title_inside_seperator("Effect PhysX", false, ImGui::GetContentRegionAvail().x - 16.0f, 24.0f, 1.0f);
		imgui::PopStyleColor();

		if (imgui::Button("Convert FX PhysX actors to misc_models", ImVec2(GET_WIDGET_WIDTH(), imgui::GetFrameHeight())))
		{
			phys->convert_phys_to_misc_models();
		} TT("convert all dynamic FX PhysX actors to misc_models");

		imgui::SetNextItemWidth(GET_WIDGET_WIDTH());
		if (imgui::BeginCombo("##combo_physx_shape", phys->m_effect_shape.strings[phys->m_effect_shape.index], ImGuiComboFlags_HeightLarge))
		{
			for (auto i = 0; i < IM_ARRAYSIZE(phys->m_effect_shape.strings); i++)
			{
				if (imgui::Selectable(phys->m_effect_shape.strings[i], phys->m_effect_shape.index == i)) 
				{
					phys->m_effect_shape.index = i;
				}

				if (phys->m_effect_shape.index == i)
				{
					imgui::SetItemDefaultFocus();
				}
			}

			imgui::EndCombo();
		} TT("sets the shape type for PhysX actors spawned via effects");

		if (phys->m_effect_shape.index != components::physx_impl::EFFECT_PHYSX_SHAPE::CUSTOM)
		{
			SET_WIDGET_WIDTH_WITH_LABEL("Shape Scale   ");
			if (imgui::DragFloat("Shape Scale", &phys->m_effect_shape.scalar, 0.05f, 0.05f, 100.0f, "%.2f"))
			{
				if (phys->m_effect_shape.scalar <= 0.0f)
				{
					phys->m_effect_shape.scalar = 0.05f;
				}
			}
			TT("a general scalar for the shape used by PhysX actors spawned via effects");
		}
		else
		{
			imgui::BeginDisabled(!single_brush_selected);
			{
				if (imgui::Button("Use selected brush as shape", ImVec2(GET_WIDGET_WIDTH(), imgui::GetFrameHeight())))
				{
					const auto sb = game::g_selected_brushes();
					if (sb && sb->def)
					{
						phys->create_custom_shape_from_selection(sb);
					}
					
				} TT("uses the currently selected brush as the collision shape\nfor PhysX actors spawned via effects");

				imgui::EndDisabled();
			}
		}


		// -----------------
		imgui::title_with_seperator("PhysX :: Static Collision Material Properties", true, 0.0f, 2.0f, 8.0f);

		//const auto separator_width = imgui::GetContentRegionAvail().x - 8.0f;
		//imgui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));

		SET_WIDGET_WIDTH_WITH_LABEL("Dynamic Friction");
		imgui::DragFloat("Static Friction", &phys_material[0], 0.05f, 0.0f, 100.0f, "%.2f");

		SET_WIDGET_WIDTH_REPEAT();
		imgui::DragFloat("Dynamic Friction", &phys_material[1], 0.05f, 0.0f, 100.0f, "%.2f");

		SET_WIDGET_WIDTH_REPEAT();
		imgui::DragFloat("Restitution", &phys_material[2], 0.05f, 0.0f, 100.0f, "%.2f");

		if (imgui::Button("Update", ImVec2(GET_WIDGET_WIDTH(), imgui::GetFrameHeight())))
		{
			components::physx_impl::get()->update_static_collision_material();
		} TT("Update material properties - changes properties of all static collision and prefab actors");


		// #
		imgui::title_with_seperator("PhysX :: World Settings", true, 0.0f, 2.0f, 8.0f);

		SET_WIDGET_WIDTH_WITH_LABEL("Gravity");
		static float physx_gravity[3] = { 0.0f, 0.0f, -800.0f };
		if (imgui::DragFloat3("Gravity", physx_gravity, 0.5f, -4000.0f, 4000.0f, "%.2f"))
		{
			phys->mScene->setGravity(physx::PxVec3(physx_gravity[0], physx_gravity[1], physx_gravity[2]));
		}

		SET_WIDGET_WIDTH_WITH_LABEL("Bounce Threshold");
		auto bounce_threshold = phys->mScene->getBounceThresholdVelocity(); // 20
		if (imgui::DragFloat("Bounce Threshold", &bounce_threshold, 0.025f, 0.0f, 50000.0f, "%.2f"))
		{
			phys->mScene->setBounceThresholdVelocity(bounce_threshold);
		}

#if DEBUG
		SET_WIDGET_WIDTH_WITH_LABEL("Friction Threshold");
		auto friction_threshold = phys->mScene->getFrictionOffsetThreshold(); // 0.0399999991
		imgui::DragFloat("Friction Threshold", &friction_threshold, 0.025f, 0.0f, 1000.0f, "%.2f");
#endif

		// #
		imgui::title_with_seperator("PhysX :: Debug Visuals", true, 0.0f, 2.0f, 8.0f);

#if DEBUG
		imgui::Checkbox("DEBUG: Keep physics code running", &phys_force_frame_logic);
		TT("Enable to always run physics code even if no effect is playing (useful for debug visualization)");
#endif
		static bool physx_draw_debug = false;
		if (imgui::Checkbox("Enable debug visuals", &physx_draw_debug))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, physx_draw_debug ? 1.0f : 0.0f);
		}

		static bool physx_draw_debug_shapes = false;
		if (imgui::Checkbox("Draw shapes", &physx_draw_debug_shapes))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, physx_draw_debug_shapes ? phys_debug_vis_scale : 0.0f);
		}

		static bool physx_draw_debug_aabbs = false;
		if (imgui::Checkbox("Draw aabbs", &physx_draw_debug_aabbs))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS, physx_draw_debug_aabbs ? phys_debug_vis_scale : 0.0f);
		}

		static bool physx_draw_debug_contacts = false;
		if (imgui::Checkbox("Draw contacts", &physx_draw_debug_contacts))
		{
			phys->mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCONTACT_POINT, physx_draw_debug_contacts ? phys_debug_vis_scale : 0.0f);
		}

		label_size = imgui::CalcTextSize("Visualization Box Size").x;
		imgui::SetNextItemWidth(general_widget_width - label_size + 74.0f);
		if (imgui::DragFloat("Visualization Box Size", &dvars::physx_debug_visualization_box_size->current.value, 0.5f, 0.0f, FLT_MAX, "%.2f"))
		{
			const auto cam = &cmainframe::activewnd->m_pCamWnd->camera;
			const auto cbox_size = dvars::physx_debug_visualization_box_size->current.value * 0.5f;

			const auto cbox = physx::PxBounds3(
				physx::PxVec3(cam->origin[0] - cbox_size, cam->origin[1] - cbox_size, cam->origin[2] - cbox_size),
				physx::PxVec3(cam->origin[0] + cbox_size, cam->origin[1] + cbox_size, cam->origin[2] + cbox_size));

			components::physx_impl::get()->mScene->setVisualizationCullingBox(cbox);
		}

		SPACING(0.0f, 2.0f);

		//imgui::PopStyleColor(); // Separator
	}

	// --------------------


	void camera_settings_dialog::bsp_settings()
	{
		const auto& style = imgui::GetStyle();
		const float second_column = imgui::GetWindowContentRegionWidth() * 0.5f;

		imgui::Indent(8.0f);
		imgui::Spacing();

		// -----------------
		imgui::title_with_seperator("BSP", false, 0, 2, 6.0f);

		imgui::Dvar("Compile BSP", dvars::bsp_compile_bsp);
		imgui::SameLine(second_column);
		imgui::Dvar("Only Ents", dvars::bsp_compile_onlyents);

		imgui::Dvar("Samplescale", dvars::bsp_compile_samplescale_enabled);
		imgui::SameLine(second_column);
		imgui::BeginDisabled(!dvars::bsp_compile_samplescale_enabled->current.enabled);
		{
			imgui::SetNextItemWidth(imgui::GetWindowContentRegionWidth() - imgui::GetCursorPosX() - 4.0f);
			imgui::Dvar("##num_samplescale", dvars::bsp_compile_samplescale);
			imgui::EndDisabled();
		}

		SPACING(0.0f, 4.0f);

		imgui::Dvar("Custom Commandline Settings - BSP", dvars::bsp_compile_custom_cmd_enabled);
		imgui::BeginDisabled(!dvars::bsp_compile_custom_cmd_enabled->current.enabled);
		{
			std::string temp_string = dvars::bsp_compile_custom_cmd->current.string;
			imgui::SetNextItemWidth(-8);
			if (imgui::InputText("##bsp_commandline", &temp_string, ImGuiInputTextFlags_None))
			{
				dvars::set_string(dvars::bsp_compile_custom_cmd, temp_string.c_str());
			}
			imgui::EndDisabled();
		}


		// -----------------
		imgui::title_with_seperator("Light", true, 0, 2, 6.0f);

		imgui::Dvar("Compile Light", dvars::bsp_compile_light);

		imgui::Dvar("Fast", dvars::bsp_compile_light_fast);
		imgui::SameLine(second_column);
		imgui::Dvar("Extra", dvars::bsp_compile_light_extra);

		imgui::Dvar("Modelshadow", dvars::bsp_compile_light_modelshadow);
		imgui::SameLine(second_column);
		imgui::Dvar("Dump Options", dvars::bsp_compile_light_dump);

		imgui::Dvar("Traces", dvars::bsp_compile_light_traces_enabled);
		imgui::SameLine(second_column);
		imgui::BeginDisabled(!dvars::bsp_compile_light_traces_enabled->current.enabled);
		{
			imgui::SetNextItemWidth(imgui::GetWindowContentRegionWidth() - imgui::GetCursorPosX() - 4.0f);
			imgui::Dvar("##num_traces", dvars::bsp_compile_light_traces);
			imgui::EndDisabled();
		}

		SPACING(0.0f, 4.0f);

		imgui::Dvar("Custom Commandline Settings - Light", dvars::bsp_compile_light_custom_cmd_enabled);
		imgui::BeginDisabled(!dvars::bsp_compile_light_custom_cmd_enabled->current.enabled);
		{
			std::string temp_string = dvars::bsp_compile_light_custom_cmd->current.string;
			imgui::SetNextItemWidth(-style.FramePadding.x);

			if (imgui::InputText("##light_commandline", &temp_string, ImGuiInputTextFlags_None))
			{
				dvars::set_string(dvars::bsp_compile_light_custom_cmd, temp_string.c_str());
			}
			imgui::EndDisabled();
		}

		// -----------------
		imgui::title_with_seperator("Compiling", true, 0, 2, 6.0f);

		imgui::Checkbox("Automatically compile reflections when building bsp", &dvars::bsp_gen_reflections_on_compile->current.enabled); TT(dvars::bsp_gen_reflections_on_compile->description);
		imgui::Checkbox("Automatically turn on bsp-view after compiling", &dvars::bsp_show_bsp_after_compile->current.enabled); TT(dvars::bsp_show_bsp_after_compile->description);
		imgui::Checkbox("Automatically compile fastfile after bulding the bsp", &dvars::bsp_gen_fastfile_on_compile->current.enabled); TT(dvars::bsp_gen_fastfile_on_compile->description);

		/*if (imgui::Button("Generate Reflections", ImVec2(-style.FramePadding.x, imgui::GetFrameHeight())))
		{
			dvars::set_bool(dvars::r_reflectionprobe_generate, true);
		} TT("Probes within the loaded bsp will be used to take screenshots ..\nso make sure your bsp is up-to-date.");*/

		imgui::Checkbox("Generate CreateFX", &dvars::bsp_gen_createfx_on_compile->current.enabled);
		TT(	"Automatically generate CreateFX files for the current map when compiling bsp.\n"
			"This will copy the files to the correct location in raw/maps/...");

		SPACING(0, 0);

		// -----------------
		//imgui::title_with_seperator("Compiling", true, 0, 2, 6.0f);


		const bool can_compile = !game::g_prefab_stack_level; //components::d3dbsp::Com_IsBspLoaded() && !components::d3dbsp::loaded_bsp_path.empty();
		imgui::BeginDisabled(!can_compile);
		{
			//const std::string d3dbsp_name = components::d3dbsp::loaded_bsp_path.substr(components::d3dbsp::loaded_bsp_path.find_last_of("\\") + 1);
			std::string d3dbsp_name;

			if (can_compile)
			{
				d3dbsp_name = std::string(game::current_map_filepath).substr(std::string(game::current_map_filepath).find_last_of("\\") + 1);
				utils::erase_substring(d3dbsp_name, ".map");
			}

			const std::string button_str = can_compile ? ("Compile " + d3dbsp_name) : "Compile BSP";

			if (imgui::Button(button_str.c_str(), ImVec2(-style.FramePadding.x, imgui::GetFrameHeight())))
			{
				components::d3dbsp::last_compiled_map = d3dbsp_name;
				components::d3dbsp::compile_bsp(d3dbsp_name, dvars::bsp_gen_createfx_on_compile->current.enabled);
			}

			imgui::EndDisabled();
		}
	}
	
	// --------------------

	bool camera_settings_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 220.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 730.0f); 
		
		auto initial_window_pos = ggui::get_initial_window_pos();
		
		if (const auto	camerawnd = GET_GUI(ggui::camera_dialog);
						camerawnd)
		{
			initial_window_pos = ImVec2(
				camerawnd->rtt_get_position().x + camerawnd->rtt_get_size().x - INITIAL_WINDOW_SIZE.x - 48.0f,
				camerawnd->rtt_get_position().y + 32.0f);
		}

		imgui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		imgui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		imgui::SetNextWindowPos(initial_window_pos, ImGuiCond_FirstUseEver);

		// do not show window if no tabs are open

		bool any_open = false;
		for (int n = 0; n < IM_ARRAYSIZE(tab_states); n++)
		{
			if (tab_states[n])
			{
				any_open = true;
				break;
			}
		}

		if (!any_open)
		{
			return false;
		}

		imgui::PushStyleColor(ImGuiCol_WindowBg, imgui::ToImVec4(dvars::gui_menubar_bg_color->current.vector));

		if (!imgui::Begin("Cam Toolbar Settings##cam_settings_window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings /*| ImGuiWindowFlags_NoTitleBar*/))
		{
			imgui::PopStyleColor();
			imgui::End();
			return false;
		}

		imgui::PopStyleColor();

		static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyResizeDown;

		int pushed_styles = 0;
		imgui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 0.0f)); pushed_styles++;

		if (imgui::BeginTabBar("##camera_window_tabbar", tab_bar_flags))
		{
			for (int n = 0; n < IM_ARRAYSIZE(tab_states); n++)
			{
				ImGuiTabBarFlags flags = ImGuiTabItemFlags_None;
				if (refocus_active_tab && n == active_tab)
				{
					flags |= ImGuiTabItemFlags_SetSelected;
					refocus_active_tab = false;
				}

				if (tab_states[n] && imgui::BeginTabItem(tab_names[n], /*&tab_states[n]*/ nullptr, flags))
				{
					imgui::BeginChild("##tab_child", ImVec2(0.0f, 0.0f), false);
					if (!refocus_active_tab)
					{
						active_tab = n;
					}

					imgui::PopStyleVar(); pushed_styles--;

					switch (n)
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
						imgui::Text("Unhandled Tab");
					}

					imgui::EndChild();
					imgui::EndTabItem();
				}
			}

			imgui::EndTabBar();
		}

		imgui::PopStyleVar(pushed_styles);
		imgui::End();

		return true;
	}

	void camera_settings_dialog::on_open()
	{ }

	void camera_settings_dialog::on_close()
	{
		/*for (int n = 0; n < IM_ARRAYSIZE(tab_states); n++)
		{
			tab_states[n] = false;
		}*/

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