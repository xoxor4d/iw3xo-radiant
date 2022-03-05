#include "std_include.hpp"

namespace components
{
	game::vec3_t editor_origin_from_fx_origin = {};
	game::vec3_t editor_angles_from_fx_origin = {};

	/*const float spawn_axis[][3] =
	{
		{  0.0f,  0.0f,  1.0f },
		{  1.0f,  0.0f,  0.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
	};*/

	float spawn_axis_rotated[3][3] =
	{
		{  0.0f,  0.0f,  1.0f },
		{  0.0f, -1.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f }
	};

	fx_system::FxEffect* Editor_SpawnEffect(int localClientNum, fx_system::FxEffectDef* remoteDef, int msecBegin, const float* origin, const float(*axis)[3], int markEntnum)
	{

		if (!game::Dvar_FindVar("fx_enable")->current.enabled)
		{
			return nullptr;
		}

		const auto system = fx_system::FX_GetSystem(localClientNum);
		if (!system)
		{
			__debugbreak();
			game::Com_Error("system");
		}

		return FX_SpawnEffect(system, remoteDef, msecBegin, origin, axis, fx_system::FX_SPAWN_DOBJ_HANDLES, fx_system::FX_SPAWN_BONE_INDEX, 255, 0xFFFFu, markEntnum);
	}

	void Editor_SetupAndSpawnEffect([[maybe_unused]] unsigned int rand, int playback_tick)
	{
		fx_system::FxEffectDef* def = fx_system::FX_Convert(&fx_system::ed_editor_effect, fx_system::FX_AllocMem);

		if(!def)
		{
			// can happen if all elemdefs are disabled
			return;
		}

		fx_system::ed_playback_tick_old = playback_tick;
		fx_system::ed_onspawn_tick = playback_tick;
		fx_system::ed_onspawn_rand = rand;

		// quickly switch axis
		//const int AX = 0;
		//float v1[3], v2[3], v3[3];

		//v1[0] = spawn_axis[AX][0];
		//v1[1] = spawn_axis[AX][1];
		//v1[2] = spawn_axis[AX][2];
		//v3[0] = spawn_axis[AX + 5][0];
		//v3[1] = spawn_axis[AX + 5][1];
		//v3[2] = spawn_axis[AX + 5][2];
		//fx_system::Vec3Cross(v3, v1, v2);

		//float effect_axis[3][3] =
		//{
		//	{ v1[0], v1[1], v1[2] },
		//	{ v2[0], v2[1], v2[2] },
		//	{ v3[0], v3[1], v3[2] },
		//};

		utils::vector::angle_vectors(editor_angles_from_fx_origin, spawn_axis_rotated[2], spawn_axis_rotated[1], spawn_axis_rotated[0]);

		const auto effect = Editor_SpawnEffect(0, def, playback_tick, editor_origin_from_fx_origin, spawn_axis_rotated, fx_system::FX_SPAWN_MARK_ENTNUM);
		fx_system::ed_active_effect = effect;
	}


	// Commandline_LoadEffect
	bool effects::load_effect(const char* effect_name)
	{
		effects::stop();
		fx_system::FX_UnregisterAll();

		std::string effect_to_load;
		if(!effect_name && !effects::last_fx_name_.empty())
		{
			effect_to_load = effects::last_fx_name_;
		}
		else
		{
			if(effect_name)
			{
				effect_to_load = effect_name;
			}
			else
			{
				return false;
			}
			
		}

		if (fx_system::FX_LoadEditorEffect(effect_to_load.c_str(), &fx_system::ed_editor_effect))
		{
			fx_system::ed_is_editor_effect_valid = true;
			game::printf_to_console("[FX] loaded editor effect [%s]", effect_to_load.c_str());

			effects::last_fx_name_ = effect_to_load;

			return true;
		}

		fx_system::ed_is_editor_effect_valid = false;
		game::printf_to_console("[FX] failed to load editor effect [%s]", effect_to_load.c_str());

		reset_editor_effect();
		return false;
		
	}

	void effects::editor_on_effect_play_repeat()
	{
		if(effects::effect_is_paused()) // un-pause effect
		{
			if(effects::effect_can_play())
			{
				fx_system::ed_is_playing = true;
				fx_system::ed_is_paused = false;
				return;
			}
		}

		if(effects::effect_is_playing())
		{
			// re-trigger effect without stopping already spawned elements
			effects::editor_trigger_effect(fx_system::ed_playback_tick);
		}

		if(effects::effect_is_repeating())
		{
			// should not happen
			fx_system::ed_is_repeating = false;
			fx_system::ed_is_repeating_old = false;
		}

		fx_system::ed_is_playing = true;
		fx_system::ed_repeat_tickcount = GetTickCount();

		if (!fx_system::ed_active_effect)
		{
			Editor_SetupAndSpawnEffect(rand(), fx_system::ed_playback_tick);
			if (!fx_system::ed_active_effect)
			{
				fx_system::ed_is_playing = false;
			}
		}
	}

	void effects::editor_trigger_effect(int msecBegin)
	{
		if(!fx_system::ed_active_effect)
		{
			game::Com_Error("no active editor effect to trigger!");
		}

		fx_system::FX_RetriggerEffect(0, fx_system::ed_active_effect, msecBegin);
	}

	void effects::play_retrigger()
	{
		if (effects::effect_can_play())
		{
			if (effects::effect_is_playing())
			{
				fx_system::FX_RetriggerEffect(0, fx_system::ed_active_effect, fx_system::ed_playback_tick);
			}
			else
			{
				effects::set_initial_state();

				fx_system::ed_is_playing = true;
				fx_system::ed_repeat_tickcount = GetTickCount();

				if (!fx_system::ed_active_effect)
				{
					Editor_SetupAndSpawnEffect(rand(), fx_system::ed_playback_tick);
					if (!fx_system::ed_active_effect)
					{
						fx_system::ed_is_playing = false;
					}
				}
			}
		}
	}

	void effects::apply_changes()
	{
		if (fx_system::ed_is_editor_effect_valid)
		{
			Editor_SetupAndSpawnEffect(fx_system::ed_onspawn_rand, fx_system::ed_onspawn_tick);

			if (effects::effect_is_playing())
			{
				__debugbreak();
			}

			if (effects::effect_is_repeating())
			{
				__debugbreak();
			}

			if (fx_system::ed_is_playing_old)
			{
				effects::play_retrigger();
			}

			// re-set if effect was paused while it was edited (effect not visible otherwise)
			if (fx_system::ed_is_paused_old && fx_system::ed_active_effect)
			{
				fx_system::ed_is_paused = true;
			}

			if (fx_system::ed_is_repeating_old && fx_system::ed_active_effect)
			{
				fx_system::ed_is_repeating = true;
			}
		}
	}

	void effects::play()
	{
		effects::set_initial_state();
		effects::editor_on_effect_play_repeat();
	}

	void effects::repeat()
	{
		effects::set_initial_state();
		fx_system::ed_is_paused = false;

		if (components::effects::effect_is_repeating())
		{
			fx_system::ed_is_repeating = false;

		}
		else if (components::effects::effect_is_playing())
		{
			fx_system::ed_is_repeating = true;
		}
		else
		{
			effects::play();

			if (components::effects::effect_is_playing() && !components::effects::effect_is_repeating())
			{
				fx_system::ed_is_repeating = true;
			}
			else
			{
				fx_system::ed_is_repeating = false;
			}
		}
	}

	void effects::pause()
	{
		if (fx_system::ed_is_playing)
		{
			fx_system::ed_is_playing = false;
			fx_system::ed_is_paused = true;
		}
		else
		{
			if (components::effects::effect_can_play() && fx_system::ed_active_effect)
			{
				fx_system::ed_is_playing = true;
				fx_system::ed_is_paused = false;
			}
		}
	}

	void effects::stop()
	{
		effects::set_initial_state();
		effects::on_effect_stop();
	}

	void effects::edit()
	{
		// on open
		if(!ggui::state.czwnd.m_effects_editor.menustate)
		{
			ggui::state.czwnd.m_effects_editor.menustate = true;
		}

		// on close
		else
		{
			if(effects_editor::has_unsaved_changes())
			{
				ggui::effects_editor_gui::editor_pending_close = true;
			}
			else
			{
				ggui::state.czwnd.m_effects_editor.menustate = false;
			}
		}
	}

	bool effects::effect_is_paused()
	{
		return fx_system::ed_is_paused && !fx_system::ed_is_playing;
	}

	bool effects::effect_is_playing()
	{
		return fx_system::ed_is_playing;
	}

	bool effects::effect_is_repeating()
	{
		return fx_system::ed_is_repeating;
	}

	bool effects::effect_can_play()
	{
		return fx_system::ed_is_editor_effect_valid && (effects::is_fx_origin_selected() || effects_editor::is_editor_active());
	}

	bool effects::is_fx_origin_selected()
	{
		return is_fx_origin_selected_;
	}

	void effects::fx_origin_frame()
	{
		const auto edit_ent = game::g_edit_entity();

		if(edit_ent && edit_ent->eclass && edit_ent->eclass->classtype == game::ECLASS_RADIANT_NODE)
		{
			if(utils::string_equals(ggui::entity::ValueForKey(edit_ent->epairs, "classname"), "fx_origin"))
			{
				is_fx_origin_selected_ = true;

				memcpy(editor_origin_from_fx_origin, edit_ent->origin, sizeof(game::vec3_t));

				ggui::entity::Entity_GetVec3ForKey(reinterpret_cast<game::entity_s*>(edit_ent), editor_angles_from_fx_origin, "angles");

				const auto fx_name = ggui::entity::ValueForKey(edit_ent->epairs, "fx");


				if (fx_name && !utils::string_equals(fx_system::ed_editor_effect.name, fx_name))
				{
					if (!effects_editor::has_unsaved_changes())
					{
						effects::load_effect(fx_name);
					}
				}
				else if (fx_system::ed_active_effect && (effects::effect_is_playing() || effects::effect_is_paused()))
				{
					// use "fx_origin" origin to update effect position 
					memcpy(fx_system::ed_active_effect->frameAtSpawn.origin, edit_ent->origin, sizeof(game::vec3_t));
					memcpy(fx_system::ed_active_effect->frameNow.origin, edit_ent->origin, sizeof(game::vec3_t));


					// use "fx_origin" angles to update effect angles - update spawn axis
					utils::vector::angle_vectors(editor_angles_from_fx_origin, spawn_axis_rotated[2], spawn_axis_rotated[1], spawn_axis_rotated[0]);

					float quat[4] = {};
					fx_system::AxisToQuat(spawn_axis_rotated, quat);

					memcpy(fx_system::ed_active_effect->frameAtSpawn.quat, quat, sizeof(game::vec4_t));
					memcpy(fx_system::ed_active_effect->frameNow.quat, quat, sizeof(game::vec4_t));
				}

				return;
			}
		}
		else if(edit_ent && edit_ent->eclass && utils::string_equals(edit_ent->eclass->name, "worldspawn"))
		{
			if(!game::g_selected_brushes()->def)
			{
				return;
			}
		}

		if(effects::effect_is_playing())
		{
			effects::on_effect_stop();
		}

		is_fx_origin_selected_ = false;
	}

	void effects::tick_playback()
	{
		auto saved_tick = static_cast<int>(GetTickCount());
		auto tick_cmp = static_cast<int>(GetTickCount()) - fx_system::ed_repeat_tickcount;

		if(tick_cmp > 200)
		{
			tick_cmp = 200;
		}

		fx_system::ed_repeat_tickcount = saved_tick;

		if (effects::effect_is_playing())
		{
			auto tick_inc = static_cast<int>( static_cast<double>(tick_cmp) * static_cast<double>(fx_system::ed_timescale) + 9.313225746154785e-10);
			if(tick_inc <= 1)
			{
				tick_inc = 1;
			}

			fx_system::ed_playback_tick += tick_inc;
		}
		
	}

	void effects::tick_repeat()
	{
		if(fx_system::ed_is_repeating)
		{
			if(!fx_system::ed_active_effect)
			{
				game::Com_Error("No active editor effect!");
			}

			const auto tick = static_cast<int>((static_cast<double>((fx_system::ed_looppause * 1000.0f)) + 9.313225746154785e-10));
			while (fx_system::ed_playback_tick - fx_system::ed_playback_tick_old >= tick)
			{
				fx_system::ed_playback_tick_old += tick;
				editor_trigger_effect(fx_system::ed_playback_tick_old);
			}
		}
	}

	void effects::on_effect_stop()
	{
		if (fx_system::ed_active_effect)
		{
			// save in case effect is getting recompiled and retriggered
			fx_system::ed_is_paused_old = fx_system::ed_is_paused;
			fx_system::ed_is_playing_old = fx_system::ed_is_playing;
			fx_system::ed_is_repeating_old = fx_system::ed_is_repeating;

			// -

			fx_system::ed_is_playing = false;
			fx_system::ed_is_repeating = false;
			fx_system::ed_is_paused = false;

			const auto system = fx_system::FX_GetSystem(0);

			if (fx_system::ed_active_effect)
			{
				const auto stat = fx_system::FX_GetEffectStatus(fx_system::ed_active_effect);
				fx_system::FX_DelRefToEffect(system, fx_system::ed_active_effect);

				if (!stat)
				{
					fx_system::FX_KillEffect(system, fx_system::ed_active_effect);
				}

				fx_system::ed_active_effect = nullptr;
			}
		}
	}

	void effects::reset_editor_effect()
	{
		fx_system::ed_is_filename_valid = false;
		fx_system::ed_is_playing = false;

		fx_system::ed_editor_effect.name[0] = 0;
		fx_system::ed_editor_effect.elemCount = 0;
	}

	// called from radiantapp::MFCCreate()
	void effects::radiant_init_fx()
	{
		on_effect_stop();
		reset_editor_effect();
		fx_system::FX_InitSystem(0);
	}

	void camera_onpaint_intercept()
	{
		effects::fx_origin_frame();

		effects::tick_playback();
		effects::tick_repeat();

		fx_system::FX_SetNextUpdateTime(0, fx_system::ed_playback_tick);
		fx_system::FX_SetupCamera_Radiant();

		if(fx_system::ed_active_effect)
		{
			if(!effects::effect_is_repeating())
			{
				if(fx_system::FX_GetEffectStatus(fx_system::ed_active_effect))
				{
					effects::on_effect_stop();
				}
			}
		}

		fx_system::FxCmd cmd = {};
		FX_FillUpdateCmd(0, &cmd);
		Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_NON_DEPENDENT, &cmd);
		//Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_SPOT_LIGHT, &cmd);
		Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_REMAINING, &cmd);
	}

	__declspec (naked) void camera_onpaint_stub()
	{
		const static uint32_t retn_addr = 0x403052;
		__asm
		{
			pushad;
			call	camera_onpaint_intercept;
			popad;

			jmp		retn_addr;
		}
	}

	void menu(ggui::imgui_context_menu& menu)
	{
		ImGui::SetNextWindowSize(ImVec2(400.0f, 390.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Effects-Debug##window", &menu.menustate, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
		}

		ImGui::End();
	}

	void effects::generate_createfx()
	{
		std::ofstream def;
		std::ofstream createfx;
		dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");

		if (dvars::fs_homepath)
		{
			std::string filepath = dvars::fs_homepath->current.string;
						filepath += "\\IW3xRadiant\\createfx\\"s;

			std::filesystem::create_directories(filepath + "createfx\\");

			std::string mapname = "unnamed_map";
			if(game::current_map_filepath)
			{
				mapname = std::string(game::current_map_filepath).substr(std::string(game::current_map_filepath).find_last_of("\\") + 1);
				utils::erase_substring(mapname, ".map");
			}

			const std::string fx_def_path = filepath + mapname + "_fx.gsc"s;
			const std::string createfx_path = filepath + "createfx\\" + mapname + "_fx.gsc"s;

			def.open(fx_def_path);
			createfx.open(createfx_path);

			if (!def.is_open() && !createfx.is_open())
			{
				game::printf_to_console("[!] failed to export createfx files\n");
				return;
			}

			int effect_count = 0;

			def << "main()" << std::endl << "{" << std::endl;
			createfx << "//_createfx generated. Do not touch!!" << std::endl; // kek
			createfx << "main()" << std::endl << "{" << std::endl;


			for (auto sb = game::g_selected_brushes_next(); ; sb = sb->next)
			{
				if(sb && sb->def && sb->def->owner)
				{
					const auto owner = reinterpret_cast<game::entity_s_def*>(sb->def->owner);
					if (utils::string_equals(owner->eclass->name, "fx_origin"))
					{
						float fx_angles[3] = {};
						ggui::entity::Entity_GetVec3ForKey(reinterpret_cast<game::entity_s*>(owner), fx_angles, "angles");

						float temp_rotation_matrix[3][3] = {};
						utils::vector::angle_vectors(fx_angles, temp_rotation_matrix[2], temp_rotation_matrix[1], temp_rotation_matrix[0]);

						float world_angles[3] = {};
						game::AxisToAngles(temp_rotation_matrix, world_angles);

						std::string fx_path = ggui::entity::ValueForKey(owner->epairs, "fx");
						utils::replace(fx_path, "\\", "/");

						def << "\tlevel._effect[ \"effect_" << effect_count << "\" ] = loadfx( \"" << fx_path << "\" );" << std::endl;

						createfx << "\tent = maps\\mp\\_utility::createOneshotEffect( \"effect_" << effect_count << "\" );" << std::endl;
						createfx << "\tent.v[ \"origin\" ] = ( " << std::fixed << std::setprecision(2) << owner->origin[0] << ", " << owner->origin[1] << ", " << owner->origin[2] << " );" << std::endl;
						createfx << "\tent.v[ \"angles\" ] = ( " << std::fixed << std::setprecision(2) << world_angles[0] << ", " << world_angles[1] << ", " << world_angles[2] << " );" << std::endl;
						createfx << "\tent.v[ \"fxid\" ] = \"effect_" << effect_count << "\";" << std::endl;
						createfx << "\tent.v[ \"delay\" ] = -15;" << std::endl << std::endl;

						effect_count++;
					}

					if(sb == game::g_selected_brushes())
					{
						break;
					}
				}
				else
				{
					break;
				}
			}


			for (auto sb = game::g_active_brushes_next(); ; sb = sb->next)
			{
				if (sb && sb->def && sb->def->owner)
				{
					const auto owner = reinterpret_cast<game::entity_s_def*>(sb->def->owner);
					if (utils::string_equals(owner->eclass->name, "fx_origin"))
					{
						float fx_angles[3] = {};
						ggui::entity::Entity_GetVec3ForKey(reinterpret_cast<game::entity_s*>(owner), fx_angles, "angles");

						float temp_rotation_matrix[3][3] = {};
						utils::vector::angle_vectors(fx_angles, temp_rotation_matrix[2], temp_rotation_matrix[1], temp_rotation_matrix[0]);

						float world_angles[3] = {};
						game::AxisToAngles(temp_rotation_matrix, world_angles);

						std::string fx_path = ggui::entity::ValueForKey(owner->epairs, "fx");
						utils::replace(fx_path, "\\", "/");

						def << "\tlevel._effect[ \"effect_" << effect_count << "\" ] = loadfx( \"" << fx_path << "\" );" << std::endl;

						createfx << "\tent = maps\\mp\\_utility::createOneshotEffect( \"effect_" << effect_count << "\" );" << std::endl;
						createfx << "\tent.v[ \"origin\" ] = ( " << std::fixed << std::setprecision(2) << owner->origin[0] << ", " << owner->origin[1] << ", " << owner->origin[2] << " );" << std::endl;
						createfx << "\tent.v[ \"angles\" ] = ( " << std::fixed << std::setprecision(2) << world_angles[0] << ", " << world_angles[1] << ", " << world_angles[2] << " );" << std::endl;
						createfx << "\tent.v[ \"fxid\" ] = \"effect_" << effect_count << "\";" << std::endl;
						createfx << "\tent.v[ \"delay\" ] = -15;" << std::endl << std::endl;

						effect_count++;
					}

					if(sb == game::g_active_brushes())
					{
						break;
					}
				}
				else
				{
					break;
				}
			}

			def << "}" << std::endl;
			createfx << "}" << std::endl;

			def.close();
			createfx.close();

			game::printf_to_console("[Generate Createfx] for [%d] effects", effect_count);
			game::printf_to_console("|> createfx def file :: [%s]", fx_def_path.c_str());
			game::printf_to_console("|> createfx file :: [%s]", createfx_path.c_str());
		}
	}

	void effects::set_initial_state()
	{
		fx_system::g_warning_outdoor_material = false;
	}

	effects::effects()
	{
		// hook nop'd r_clear in CCamWnd::OnPaint to integrate fx
		utils::hook(0x40304D, camera_onpaint_stub, HOOK_JUMP).install()->quick();

		command::register_command_with_hotkey("fx_play"s, [this](auto)
		{
				effects::play();
		});

		command::register_command_with_hotkey("fx_repeat"s, [this](auto)
		{
				effects::repeat();
		});

		command::register_command_with_hotkey("fx_pause"s, [this](auto)
		{
				effects::pause();
		});

		command::register_command_with_hotkey("fx_stop"s, [this](auto)
		{
				effects::stop();
		});

		command::register_command("fx_reload"s, [this](auto)
		{
			effects::set_initial_state();
			effects::load_effect(nullptr);
		});

		command::register_command("fx_edit"s, [this](auto)
		{
			effects::edit();
		});

		command::register_command("generate_createfx"s, [this](auto)
		{
			effects::generate_createfx();
		});
	}

	effects::~effects()
	{ }
}
