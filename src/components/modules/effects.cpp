#include "std_include.hpp"

namespace components
{
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

	void Editor_SetupAndSpawnEffect(unsigned int rand, int playback_tick)
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

		const auto effect = Editor_SpawnEffect(0, def, playback_tick, game::vec3_origin, effect_axis, fx_system::FX_SPAWN_MARK_ENTNUM);
		fx_system::ed_active_effect = effect;
	}


	// Commandline_LoadEffect
	bool effects::load_test_effect()
	{
		fx_system::FX_UnregisterAll();

		if (fx_system::FX_LoadEditorEffect("1_reverse", &fx_system::ed_editor_effect))
		{
			fx_system::ed_is_editor_effect_valid = true;
			game::printf_to_console("[FX] loaded editor effect");

			return true;
		}

		fx_system::ed_is_editor_effect_valid = false;
		game::printf_to_console("[FX] failed to load editor effect");
		reset_editor_effect();

		return false;
		
	}

	void effects::editor_on_effect_play_repeat()
	{
		if(effects::effect_is_playing())
		{
			on_effect_stop();

		}

		if(effect_is_repeating())
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
		return /*fx_system::ed_is_filename_valid &&*/ !fx_system::ed_is_repeating;
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

		if (effect_is_playing())
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
	
	effects::effects()
	{
		// hook nop'd r_clear in CCamWnd::OnPaint to integrate fx
		utils::hook(0x40304D, camera_onpaint_stub, HOOK_JUMP).install()->quick();

		command::register_command_with_hotkey("fx_play"s, [this](auto)
		{
			effects::editor_on_effect_play_repeat();
		});

		command::register_command_with_hotkey("fx_stop"s, [this](auto)
		{
			effects::on_effect_stop();
		});

		command::register_command_with_hotkey("fx_load"s, [this](auto)
		{
			effects::load_test_effect();
		});
	}

	effects::~effects()
	{ }
}
