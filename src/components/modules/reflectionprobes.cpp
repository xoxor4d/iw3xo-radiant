#include "std_include.hpp"

namespace components
{
	int cubemapshot_res = 256;
	int cubemapshot_pixel_border = 1;
	std::uint8_t* reflectionprobes::cubemapshot_image_ptr[6];

	void R_TakeCubeMapShot(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, std::int32_t bytes_per_pixel, std::uint8_t* buffer)
	{
		IDirect3DSurface9* back_buffer = nullptr;
		D3DSURFACE_DESC desc{};
		IDirect3DSurface9* surface = nullptr;
		tagRECT source_rect{ x, y, width + x, height + y };
		D3DLOCKED_RECT locked_rect{};

		if (!SUCCEEDED(game::dx->device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer)))
		{
			game::Com_Error("ERROR: cannot take screenshot: couldn't get back buffer surface\n");
			back_buffer->Release();
			return;
		}

		if (!SUCCEEDED(back_buffer->GetDesc(&desc)))
		{
			game::Com_Error("ERROR: cannot take screenshot: couldn't get desc\n");
			back_buffer->Release();
			return;
		}

		if (!SUCCEEDED(game::dx->device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &surface, nullptr)))
		{
			game::Com_Error("ERROR: cannot take screenshot: couldn't create the off-screen surface\n");
			surface->Release();
			back_buffer->Release();
			return;
		}

		if (!SUCCEEDED(game::dx->device->GetRenderTargetData(back_buffer, surface)))
		{
			game::Com_Error("ERROR: cannot take screenshot: GetRenderTargetData failed\n");
			surface->Release();
			back_buffer->Release();
			return;
		}

		if (!SUCCEEDED(surface->LockRect(&locked_rect, &source_rect, D3DLOCK_READONLY)))
		{
			surface->Release();
			back_buffer->Release();
		}

		if (auto bits = static_cast<std::uint8_t*>(locked_rect.pBits))
		{
			if (bytes_per_pixel == 3)
			{
				for (auto i = 0; i < height; i++)
				{
					for (auto j = 0; j < width; j++)
					{
						buffer[0] = bits[0];
						buffer[1] = bits[1];
						buffer[2] = bits[2];

						buffer += 3;
						bits += 4;
					}
				}
			}
			else if (locked_rect.Pitch == (4 * width))
			{
				memcpy(buffer, bits, 4 * height * width);
			}
			else
			{
				for (auto i = 0; i < height; i++)
				{
					memcpy(buffer, bits, (4 * width));
					bits += locked_rect.Pitch;
					buffer += (4 * width);
				}
			}
		}

		surface->UnlockRect();
		surface->Release();
		back_buffer->Release();
	}

	void R_EndCubemapShot(game::CubemapShot shotIndex)
	{
		const int img_index = shotIndex - 1;
		const int size_in_bytes = 4 * cubemapshot_res * cubemapshot_res;

		reflectionprobes::cubemapshot_image_ptr[img_index] = static_cast<std::uint8_t*>(game::Z_VirtualAlloc(size_in_bytes));
		R_TakeCubeMapShot(cubemapshot_pixel_border, cubemapshot_pixel_border, cubemapshot_res, cubemapshot_res, 4, reflectionprobes::cubemapshot_image_ptr[img_index]);
	}

	void R_CubemapShotSetInitialState()
	{
		const game::vec4_t clear_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::R_SetupRendertarget(game::gfxCmdBufSourceState, game::R_RENDERTARGET_FRAME_BUFFER);
		game::R_SetRenderTarget(game::gfxCmdBufSourceState, game::gfxCmdBufState, game::R_RENDERTARGET_FRAME_BUFFER);
		game::R_ClearScreen(game::gfxCmdBufState->prim.device, 7, clear_color, 1.0f, false, nullptr);
	}

	void R_CalcCubeMapViewValues(game::refdef_s* refdef, game::CubemapShot shot, int shot_size)
	{
		refdef->width = shot_size + 2;
		refdef->height = shot_size + 2;
		refdef->tanHalfFovX = (static_cast<float>(shot_size) + 2) / static_cast<float>(shot_size);
		refdef->tanHalfFovY = refdef->tanHalfFovX;
		refdef->zNear = 0.0f;
		refdef->x = 0;
		refdef->y = 0;

		switch (shot)
		{
		case game::CUBEMAPSHOT_RIGHT:
			refdef->viewaxis[0][0] = 1.0f;
			refdef->viewaxis[0][1] = 0.0f;
			refdef->viewaxis[0][2] = 0.0f;
			refdef->viewaxis[1][0] = 0.0f;
			refdef->viewaxis[1][1] = 1.0f;
			refdef->viewaxis[1][2] = 0.0f;
			refdef->viewaxis[2][0] = 0.0f;
			refdef->viewaxis[2][1] = 0.0f;
			refdef->viewaxis[2][2] = 1.0f;
			break;

		case game::CUBEMAPSHOT_LEFT:
			refdef->viewaxis[0][0] = -1.0f;
			refdef->viewaxis[0][1] = 0.0f;
			refdef->viewaxis[0][2] = 0.0f;
			refdef->viewaxis[1][0] = 0.0f;
			refdef->viewaxis[1][1] = -1.0f;
			refdef->viewaxis[1][2] = 0.0f;
			refdef->viewaxis[2][0] = 0.0f;
			refdef->viewaxis[2][1] = 0.0f;
			refdef->viewaxis[2][2] = 1.0f;
			break;

		case game::CUBEMAPSHOT_FRONT:
			refdef->viewaxis[0][0] = 0.0f;
			refdef->viewaxis[0][1] = -1.0f;
			refdef->viewaxis[0][2] = 0.0f;
			refdef->viewaxis[1][0] = 1.0f;
			refdef->viewaxis[1][1] = 0.0f;
			refdef->viewaxis[1][2] = 0.0f;
			refdef->viewaxis[2][0] = 0.0f;
			refdef->viewaxis[2][1] = 0.0f;
			refdef->viewaxis[2][2] = 1.0f;
			break;

		case game::CUBEMAPSHOT_UP:
			refdef->viewaxis[0][0] = 0.0f;
			refdef->viewaxis[0][1] = 0.0f;
			refdef->viewaxis[0][2] = 1.0f;
			refdef->viewaxis[1][1] = 1.0f;
			refdef->viewaxis[1][0] = 0.0f;
			refdef->viewaxis[1][2] = 0.0f;
			refdef->viewaxis[2][0] = -1.0f;
			refdef->viewaxis[2][1] = 0.0f;
			refdef->viewaxis[2][2] = 0.0f;
			break;
		case game::CUBEMAPSHOT_DOWN:
			refdef->viewaxis[0][0] = 0.0f;
			refdef->viewaxis[0][1] = 0.0f;
			refdef->viewaxis[0][2] = -1.0f;
			refdef->viewaxis[1][0] = 0.0f;
			refdef->viewaxis[1][1] = 1.0f;
			refdef->viewaxis[1][2] = 0.0f;
			refdef->viewaxis[2][0] = 1.0f;
			refdef->viewaxis[2][1] = 0.0f;
			refdef->viewaxis[2][2] = 0.0f;
			break;

		default:
			refdef->viewaxis[0][0] = 0.0f;
			refdef->viewaxis[0][1] = 1.0f;
			refdef->viewaxis[0][2] = 0.0f;
			refdef->viewaxis[1][0] = -1.0f;
			refdef->viewaxis[1][1] = 0.0f;
			refdef->viewaxis[1][2] = 0.0f;
			refdef->viewaxis[2][0] = 0.0f;
			refdef->viewaxis[2][1] = 0.0f;
			refdef->viewaxis[2][2] = 1.0f;
			break;
		}
	}

	void R_GenerateReflectionRawData(game::DiskGfxReflectionProbe* probe)
	{
		game::refdef_s refdef = {}; 
		memset(&refdef, 0, sizeof(refdef));

		refdef.vieworg[0] = probe->origin[0];
		refdef.vieworg[1] = probe->origin[1];
		refdef.vieworg[2] = probe->origin[2];
		refdef.localClientNum = 0;
		refdef.time = 0;
		refdef.useScissorViewport = false;
		refdef.blurRadius = 0.0f;
		memcpy(&refdef.primaryLights, &d3dbsp::scene_lights, sizeof(d3dbsp::scene_lights));

		auto saved_width = game::dx->windows[ggui::CCAMERAWND].width;
		auto saved_height = game::dx->windows[ggui::CCAMERAWND].height;

		game::dx->windows[ggui::CCAMERAWND].width = 514;
		game::dx->windows[ggui::CCAMERAWND].height = 514;
		game::dx->targetWindowIndex = -1;

		game::GfxRenderTarget* targets = reinterpret_cast<game::GfxRenderTarget*>(0x174F4A8);
		targets[game::R_RENDERTARGET_FRAME_BUFFER].width = 514;
		targets[game::R_RENDERTARGET_FRAME_BUFFER].height = 514;

		if (utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(cmainframe::activewnd->m_pCamWnd->GetWindow()))
		{
			int x = 1;
		}

		for (int i = game::CUBEMAPSHOT_RIGHT; i < game::CUBEMAPSHOT_COUNT; ++i)
		{
			//R_CubemapShotSetInitialState();
			game::R_BeginFrame();

			game::get_frontenddata()->cmds = &game::get_cmdlist()->cmds[game::get_cmdlist()->usedTotal];
			game::R_ClearScene(0);

			R_CalcCubeMapViewValues(&refdef, (game::CubemapShot)i, 512);
			game::R_SetLodOrigin(&refdef);

			// #
			// fx

			const auto system = fx_system::FX_GetSystem(0);
			fx_system::FX_SetupCamera(&system->camera, refdef.vieworg, refdef.viewaxis, refdef.tanHalfFovX, refdef.tanHalfFovY, game::Dvar_FindVar("r_zfar")->current.value);

			fx_system::FxCmd cmd = {};
			FX_FillUpdateCmd(0, &cmd);
			Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_NON_DEPENDENT, &cmd);
			Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_SPOT_LIGHT, &cmd);
			Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_REMAINING, &cmd);

			// ----

			game::CL_RenderScene(&refdef);
			game::R_EndFrame();
			game::R_IssueRenderCommands(-1);

			R_EndCubemapShot((game::CubemapShot)i);

			

			// R_CubemapShotRestoreState
			//game::R_SetupRendertarget(game::gfxCmdBufSourceState, game::R_RENDERTARGET_FRAME_BUFFER);
			//game::R_SetRenderTarget(game::gfxCmdBufSourceState, game::gfxCmdBufState, game::R_RENDERTARGET_FRAME_BUFFER);
		}

		game::dx->windows[ggui::CCAMERAWND].width = saved_width;
		game::dx->windows[ggui::CCAMERAWND].height = saved_height;

		if (utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(cmainframe::activewnd->m_pCamWnd->GetWindow()))
		{
			int x = 1;
		}
	}

	void R_GenerateReflectionRawDataAll(game::DiskGfxReflectionProbe* probeRawData, int probeCount, bool* generateProbe)
	{
		for (auto probeIndex = 0; probeIndex < probeCount; ++probeIndex)
		{
			if (generateProbe[probeIndex])
			{
				R_GenerateReflectionRawData(&probeRawData[probeIndex]);
			}
		}
	}

	void reflectionprobes::generate_reflections(game::GfxReflectionProbe* probes, const unsigned int probe_count)
	{
		bool generate_probe[256];

		unsigned int lumpProbeCount;
		const auto probe_raw_lump_data = static_cast<const game::DiskGfxReflectionProbe*>(d3dbsp::Com_GetBspLump(d3dbsp::LUMP_REFLECTION_PROBES, sizeof(game::DiskGfxReflectionProbe), &lumpProbeCount));

		if (probe_count != lumpProbeCount)
		{
			__debugbreak();
		}

		if (probe_count)
		{
			unsigned int lump_size = sizeof(game::DiskGfxReflectionProbe) * probe_count;
			auto probe_raw_generated_data = reinterpret_cast<game::DiskGfxReflectionProbe*>(game::Z_Malloc(lump_size));

			for (auto probe_index = 0u; probe_index < lumpProbeCount; ++probe_index)
			{
				memcpy(probe_raw_generated_data[probe_index].colorCorrectionFilename, probe_raw_lump_data[probe_index].colorCorrectionFilename, sizeof(probe_raw_generated_data[probe_index].colorCorrectionFilename));
				probe_raw_generated_data[probe_index].origin[0] = probes[probe_index].origin[0];
				probe_raw_generated_data[probe_index].origin[1] = probes[probe_index].origin[1];
				probe_raw_generated_data[probe_index].origin[2] = probes[probe_index].origin[2];

				//if (r_reflectionProbeRegenerateAll->current.enabled)
				//{
					generate_probe[probe_index] = true;
				//}
				/*else
				{
					v3 = R_CopyReflectionsFromLumpData(&probe_raw_generated_data[probe_index], probeRawLumpData, probe_count);
					generateProbe[probe_index] = v3 == 0;
				}*/
			}

			R_GenerateReflectionRawDataAll(probe_raw_generated_data, lumpProbeCount, generate_probe);
			//d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, probe_raw_generated_data, lump_size);
			//R_GenerateReflectionImages(probes, probe_raw_generated_data, probe_count, 0, 256);
			game::R_GenerateReflectionImages(game::s_world->reflectionProbes, probe_raw_generated_data, lumpProbeCount, 0);
			game::Z_Free(probe_raw_generated_data);
		}
		else
		{
			//d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, nullptr, 0);
		}
	}

	// call after R_EndFrame and IssueRenderCommands from camera
	void reflectionprobes::generate_reflections_for_bsp()
	{
		if(game::rgp->world)
		{
			reflectionprobes::generate_reflections(game::rgp->world->reflectionProbes + 1, game::rgp->world->reflectionProbeCount - 1);
		}
	}

	void check_for_reflection_generation()
	{
		//game::R_SortMaterials();

		if (dvars::r_generate_reflectionprobes->current.enabled)
		{
			reflectionprobes::generate_reflections_for_bsp();
			dvars::set_bool(dvars::r_generate_reflectionprobes, false);
		}
	}

	void __declspec(naked) check_for_reflection_generation_stub()
	{
		const static uint32_t func_addr = 0x4FD910; // R_SortMaterials
		const static uint32_t retn_addr = 0x403070;
		__asm
		{
			pushad;
			call	check_for_reflection_generation;
			popad;

			call	func_addr;

			

			jmp		retn_addr;
		}
	}


	reflectionprobes::reflectionprobes()
	{
		utils::hook(0x40306B, check_for_reflection_generation_stub, HOOK_JUMP).install()->quick();
	}

	reflectionprobes::~reflectionprobes()
	{ }
}
