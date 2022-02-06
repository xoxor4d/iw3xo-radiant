#include "std_include.hpp"

namespace components
{
	game::vec3_t editor_origin_from_fx_origin = {};

	const float spawn_axis[][3] =
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

		fx_system::ed_playback_tick_old = playback_tick;

		// quickly switch axis
		const int AX = 0;
		float v1[3], v2[3], v3[3];

		v1[0] = spawn_axis[AX][0];
		v1[1] = spawn_axis[AX][1];
		v1[2] = spawn_axis[AX][2];
		v3[0] = spawn_axis[AX + 5][0];
		v3[1] = spawn_axis[AX + 5][1];
		v3[2] = spawn_axis[AX + 5][2];
		fx_system::Vec3Cross(v3, v1, v2);

		const float effect_axis[3][3] =
		{
			{ v1[0], v1[1], v1[2] },
			{ v2[0], v2[1], v2[2] },
			{ v3[0], v3[1], v3[2] },
		};

		const auto effect = Editor_SpawnEffect(0, def, playback_tick, editor_origin_from_fx_origin, effect_axis, fx_system::FX_SPAWN_MARK_ENTNUM);
		fx_system::ed_active_effect = effect;
	}


	// Commandline_LoadEffect
	bool effects::load_test_effect(const char* effect_name)
	{
		fx_system::FX_UnregisterAll();

		std::string effect_to_load;

		if(!effect_name && !effects::last_fx_name_.empty())
		{
			effect_to_load = effects::last_fx_name_;
		}
		else
		{
			effect_to_load = effect_name;
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

		if(effects::effect_is_playing()) // restart effect when it was not paused
		{
			on_effect_stop();
		}

		if(effects::effect_is_repeating())
		{
			__debugbreak();
			game::Com_Error("Already repeating effect?");
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

		// would normally check moving spawn origin here?
		fx_system::FX_RetriggerEffect(0, fx_system::ed_active_effect, msecBegin);
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
		return fx_system::ed_is_editor_effect_valid && effects::is_fx_origin_selected();
	}

	bool effects::is_fx_origin_selected()
	{
		return is_fx_origin_selected_;
	}

	void effects::fx_origin_frame()
	{
		const auto edit_ent = game::g_edit_entity();

		if(edit_ent && edit_ent->eclass->classtype == game::ECLASS_RADIANT_NODE)
		{
			if(utils::string_equals(ggui::entity::ValueForKey(edit_ent->epairs, "classname"), "fx_origin"))
			{
				is_fx_origin_selected_ = true;

				memcpy(editor_origin_from_fx_origin, edit_ent->origin, sizeof(game::vec3_t));
				if (fx_system::ed_active_effect && (effects::effect_is_playing() || effects::effect_is_paused()))
				{
					memcpy(fx_system::ed_active_effect->frameNow.origin, edit_ent->origin, sizeof(game::vec3_t));
				}

				const auto fx_name = ggui::entity::ValueForKey(edit_ent->epairs, "fx");
				if(fx_name && !utils::string_equals(fx_system::ed_editor_effect.name, fx_name))
				{
					effects::load_test_effect(fx_name);
				}

				return;
			}
		}
		else if(edit_ent && utils::string_equals(edit_ent->eclass->name, "worldspawn"))
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
		fx_system::ed_is_playing = false;
		fx_system::ed_is_repeating = false;
		fx_system::ed_is_paused = false;

		const auto system = fx_system::FX_GetSystem(0);

		if(fx_system::ed_active_effect)
		{
			const auto stat = fx_system::FX_GetEffectStatus(fx_system::ed_active_effect);
			fx_system::FX_DelRefToEffect(system, fx_system::ed_active_effect);

			if(!stat)
			{
				fx_system::FX_KillEffect(system, fx_system::ed_active_effect);
			}

			fx_system::ed_active_effect = nullptr;
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
	
	effects::effects()
	{
		// hook nop'd r_clear in CCamWnd::OnPaint to integrate fx
		utils::hook(0x40304D, camera_onpaint_stub, HOOK_JUMP).install()->quick();

		command::register_command_with_hotkey("fx_play"s, [this](auto)
		{
			effects::editor_on_effect_play_repeat();
		});

		command::register_command_with_hotkey("fx_repeat"s, [this](auto)
		{
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
				components::command::execute("fx_play");

				if (components::effects::effect_is_playing() && !components::effects::effect_is_repeating())
				{
					fx_system::ed_is_repeating = true;
				}
				else
				{
					fx_system::ed_is_repeating = false;
				}
			}
		});

		command::register_command_with_hotkey("fx_pause"s, [this](auto)
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
		});

		command::register_command_with_hotkey("fx_stop"s, [this](auto)
		{
			effects::on_effect_stop();
		});

		command::register_command("fx_reload"s, [this](auto)
		{
			effects::load_test_effect(nullptr);
		});
	}

	effects::~effects()
	{ }
}
