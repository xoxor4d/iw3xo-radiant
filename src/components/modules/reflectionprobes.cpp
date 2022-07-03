#include "std_include.hpp"

namespace components
{
	std::uint8_t* reflectionprobes::cubemapshot_image_ptr[6];
	IDirect3DTexture9* reflectionprobes::imgui_cube_surfaces[6];

	enum IMG_CUBE_COORD
	{
		CUBE_X0,
		CUBE_X1,
		CUBE_Y0,
		CUBE_Y1,
	};

	int get_cubemapshot_res()
	{
		return 256;
	}

	int get_pixel_border()
	{
		return 1;
	}

	

	// no ext
	void export_cubeshot_side_as_targa(const std::string& filename, uint8_t* dataBGRA)
	{
		if (dvars::fs_homepath = game::Dvar_FindVar("fs_homepath"); 
			dvars::fs_homepath)
		{
			std::string filePath = dvars::fs_homepath->current.string;
			filePath += R"(\IW3xRadiant\reflection_probes\)";

			std::filesystem::create_directories(filePath);
			filePath += filename + ".tga"s;

			utils::tga_write(
				filePath.c_str(),
				get_cubemapshot_res(),
				get_cubemapshot_res(),
				dataBGRA,
				4,
				4);
		}
	}

	bool R_TakeCubeMapShot(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, std::int32_t bytes_per_pixel, std::uint8_t* buffer)
	{
		IDirect3DSurface9* back_buffer = nullptr;
		IDirect3DSurface9* surface = nullptr;
		D3DSURFACE_DESC desc = {};
		D3DLOCKED_RECT locked_rect = {};
		const tagRECT source_rect{ x, y, width + x, height + y };

		if (!SUCCEEDED(game::dx->windows[ggui::CCAMERAWND].swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back_buffer)))
		{
			AssertS("ERROR: cannot take screenshot: couldn't get back buffer surface\n");
			back_buffer->Release();
			return false;
		}

		if (!SUCCEEDED(back_buffer->GetDesc(&desc)))
		{
			AssertS("ERROR: cannot take screenshot: couldn't get desc\n");
			back_buffer->Release();
			return false;
		}

		if (!SUCCEEDED(game::dx->device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &surface, nullptr)))
		{
			AssertS("ERROR: cannot take screenshot: couldn't create the off-screen surface\n");
			surface->Release();
			back_buffer->Release();
			return false;
		}

		if (!SUCCEEDED(game::dx->device->GetRenderTargetData(back_buffer, surface)))
		{
			AssertS("ERROR: cannot take screenshot: GetRenderTargetData failed\n");
			surface->Release();
			back_buffer->Release();
			return false;
		}

		if (!SUCCEEDED(surface->LockRect(&locked_rect, &source_rect, D3DLOCK_READONLY)))
		{
			surface->Release();
			back_buffer->Release();
			return false;
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
					buffer += (4 * width);
					bits += locked_rect.Pitch;
				}
			}
		}

		surface->UnlockRect();
		surface->Release();
		back_buffer->Release();

		return true;
	}

	void Image_FlipVertically(std::uint8_t* pic, const int size)
	{
		for (auto s = 0; s < size; ++s)
		{
			const auto p = reinterpret_cast<int*>(&pic[4 * s]);

			for (auto t = 0; t < size / 2; ++t)
			{
				const auto np  = &p[size * (size - 1) - size * t];
				const auto cache = p[size * t];

				p[size * t] = *np;
				*np = cache;
			}
		}
	}

	void Image_FlipDiagonally(std::uint8_t* pic, const int size)
	{
		for (auto s = 1; s < size; ++s)
		{
			for (auto t = 0; t < s; ++t)
			{
				const int cache = *reinterpret_cast<int*>(&pic[4 * s + 4 * size * t]);
				*reinterpret_cast<int*>(&pic[4 * s + 4 * size * t]) = *reinterpret_cast<int*>(&pic[4 * t + 4 * size * s]);
				*reinterpret_cast<int*>(&pic[4 * t + 4 * size * s]) = cache;
			}
		}
	}

	void Image_FlipHorizontally(std::uint8_t* pic, const int size)
	{
		for (auto t = 0; t < size; ++t)
		{
			const auto pa = reinterpret_cast<int*>(&pic[4 * size * t]);
			const auto pb = reinterpret_cast<int*>(&pic[4 * size * t - 4 + 4 * size]);

			for (auto s = 0; s < size / 2; ++s)
			{
				const int cache = pa[s];
				pa[s] = pb[-s];
				pb[-s] = cache;
			}
		}
	}
	
	void R_CubemapShotDownSample(std::uint8_t* pixels, const int base_res, const int down_sample_size)
	{
		float total[4];
		std::uint8_t* pixel;
		
		for (auto y = 0; y < down_sample_size; ++y)
		{
			for (auto x = 0; x < down_sample_size; ++x)
			{
				total[0] = 0.0f;
				total[1] = 0.0f;
				total[2] = 0.0f;
				total[3] = 0.0f;

				for (auto suby = 0; suby < 4; ++suby)
				{
					for (auto subx = 0; subx < 4; ++subx)
					{
						pixel = &pixels[4 * (subx + base_res * (suby + 4 * y) + 4 * x)];

						for (auto col_index = 0; col_index < 4; ++col_index)
						{
							total[col_index] = static_cast<float>(pixel[col_index]) + total[col_index];
						}
							
					}
				}

				pixel = &pixels[4 * (x + down_sample_size * y)];

				for (auto i = 0; i < 4; ++i)
				{
					total[i] = total[i] / 16.0f;
					pixel[i] = static_cast<std::uint8_t>(total[i]);
				}
			}
		}
	}

	void Image_Blend1x1Faces(std::uint8_t* (*pixels)[15], const int mip_level)
	{
		for (auto color = 0; color < 4; ++color)
		{
			float value = 0.0f;
			for (auto f = 0; f < 6; ++f)
			{
				value = (float)(&(*pixels)[15 * f])[mip_level][color] + value;
			}

			value = value / 6.0f;
			for (auto i = 0; i < 6; ++i)
			{
				(&(*pixels)[15 * i])[mip_level][color] = static_cast<std::uint8_t>(value);
			}
		}
	}

	std::uint8_t* Image_GetCubeCornerPixel(std::uint8_t* face_pixels, const int coordx, const int coordy, const int edge_size)
	{
		int x;
		int y;

		if (coordx)
		{
			if (coordx != 1)
			{
				return nullptr;
			}

			x = edge_size - 1;
		}
		else
		{
			x = 0;
		}

		if (coordy == 2)
		{
			y = 0;
			return &face_pixels[4 * (x + y * edge_size)];
		}

		if (coordy == 3)
		{
			y = edge_size - 1;
			return &face_pixels[4 * (x + y * edge_size)];
		}

		return nullptr;
	}

	std::uint8_t* Image_GetCubeFaceEdgePixel(std::uint8_t* face_pixels, const int pixel, const int edge_size, IMG_CUBE_COORD edge)
	{
		std::uint8_t* result;

		switch (edge)
		{
		case CUBE_X0:
			result = &face_pixels[pixel * 4 * edge_size];
			break;
		case CUBE_X1:
			result = &face_pixels[4 * (edge_size + pixel * edge_size) - 4];
			break;
		case CUBE_Y0:
			result = &face_pixels[4 * pixel];
			break;
		case CUBE_Y1:
			result = &face_pixels[4 * (pixel + edge_size * (edge_size - 1))];
			break;
		default:
			result = nullptr;
			break;
		}

		return result;
	}

	void Image_BlendCubeCorner(std::uint8_t* face_pixels0, std::uint8_t* face_pixels1, std::uint8_t* face_pixels2, const int edge_size, IMG_CUBE_COORD coord0x, IMG_CUBE_COORD coord0y, IMG_CUBE_COORD coord1x, IMG_CUBE_COORD coord1y, IMG_CUBE_COORD coord2x, IMG_CUBE_COORD coord2y)
	{
		const auto pixel0 = Image_GetCubeCornerPixel(face_pixels0, coord0x, coord0y, edge_size);
		const auto pixel1 = Image_GetCubeCornerPixel(face_pixels1, coord1x, coord1y, edge_size);
		const auto pixel2 = Image_GetCubeCornerPixel(face_pixels2, coord2x, coord2y, edge_size);

		for (auto color = 0; color < 4; ++color)
		{
			pixel0[color] = static_cast<std::uint8_t>( (static_cast<float>( (pixel2[color] + pixel1[color] + pixel0[color]) ) / 3.0f) );
			pixel1[color] = pixel0[color];
			pixel2[color] = pixel0[color];
		}
	}

	void Image_BlendCubeFaceEdge(std::uint8_t* this_face_pixels, std::uint8_t* other_face_pixels, const int edge_size, IMG_CUBE_COORD this_edge, IMG_CUBE_COORD other_edge, bool flip)
	{
		for (auto i = 0; i < edge_size; ++i)
		{
			int iother = i;

			if (flip)
			{
				iother = edge_size - i - 1;
			}

			const auto this_pixel = Image_GetCubeFaceEdgePixel(this_face_pixels, i, edge_size, this_edge);
			const auto other_pixel = Image_GetCubeFaceEdgePixel(other_face_pixels, iother, edge_size, other_edge);

			for (auto color = 0; color < 4; ++color)
			{
				this_pixel[color] = (other_pixel[color] + this_pixel[color]) / 2;
				other_pixel[color] = this_pixel[color];
			}
		}
	}

	void CubeMap_BlendFaceEdges(std::uint8_t* (*pixels)[15], const int mip_level, const int edge_size)
	{
		if (edge_size == 1)
		{
			Image_Blend1x1Faces(pixels, mip_level);
		}
		else
		{
			Image_BlendCubeCorner(
				(*pixels)[mip_level], (*pixels)[mip_level + 60], (*pixels)[mip_level + 30], edge_size,
				CUBE_X0, CUBE_Y0, CUBE_X1, CUBE_Y0, CUBE_X1, CUBE_Y1);

			Image_BlendCubeCorner(
				(*pixels)[mip_level], (*pixels)[mip_level + 60], (*pixels)[mip_level + 45], edge_size,
				CUBE_X0, CUBE_Y1, CUBE_X1, CUBE_Y1, CUBE_X1, CUBE_Y0);

			Image_BlendCubeCorner(
				(*pixels)[mip_level], (*pixels)[mip_level + 75], (*pixels)[mip_level + 30], edge_size,
				CUBE_X1, CUBE_Y0,CUBE_X0, CUBE_Y0, CUBE_X1, CUBE_Y0);

			Image_BlendCubeCorner(
				(*pixels)[mip_level], (*pixels)[mip_level + 75], (*pixels)[mip_level + 45], edge_size,
				CUBE_X1, CUBE_Y1, CUBE_X0, CUBE_Y1, CUBE_X1, CUBE_Y1);

			Image_BlendCubeCorner(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 60], (*pixels)[mip_level + 30], edge_size,
				CUBE_X1, CUBE_Y0, CUBE_X0, CUBE_Y0, CUBE_X0, CUBE_Y1);

			Image_BlendCubeCorner(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 60], (*pixels)[mip_level + 45], edge_size,
				CUBE_X1, CUBE_Y1, CUBE_X0, CUBE_Y1, CUBE_X0, CUBE_Y0);

			Image_BlendCubeCorner(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 75], (*pixels)[mip_level + 30], edge_size,
				CUBE_X0, CUBE_Y0, CUBE_X1, CUBE_Y0, CUBE_X0, CUBE_Y0);

			Image_BlendCubeCorner(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 75], (*pixels)[mip_level + 45], edge_size,
				CUBE_X0, CUBE_Y1, CUBE_X1, CUBE_Y1, CUBE_X0, CUBE_Y1);

			Image_BlendCubeFaceEdge((*pixels)[mip_level], (*pixels)[mip_level + 60], edge_size, CUBE_X0, CUBE_X1, false);
			Image_BlendCubeFaceEdge((*pixels)[mip_level], (*pixels)[mip_level + 75], edge_size, CUBE_X1, CUBE_X0, false);
			Image_BlendCubeFaceEdge((*pixels)[mip_level], (*pixels)[mip_level + 30], edge_size, CUBE_Y0, CUBE_X1, true);
			Image_BlendCubeFaceEdge((*pixels)[mip_level], (*pixels)[mip_level + 45], edge_size, CUBE_Y1, CUBE_X1, false);

			Image_BlendCubeFaceEdge(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 30], edge_size,
				CUBE_Y0, CUBE_X0, false);

			Image_BlendCubeFaceEdge(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 45], edge_size, 
				CUBE_Y1, CUBE_X0, true);

			Image_BlendCubeFaceEdge(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 60], edge_size,
				CUBE_X1, CUBE_X0, false);

			Image_BlendCubeFaceEdge(
				(*pixels)[mip_level + 15], (*pixels)[mip_level + 75], edge_size,
				CUBE_X0, CUBE_X1, false);

			Image_BlendCubeFaceEdge(
				(*pixels)[mip_level + 45], (*pixels)[mip_level + 60], edge_size,
				CUBE_Y0, CUBE_Y1, false);

			Image_BlendCubeFaceEdge((*pixels)[mip_level + 45], 
				(*pixels)[mip_level + 75], edge_size, 
				CUBE_Y1, CUBE_Y1, true);

			Image_BlendCubeFaceEdge(
				(*pixels)[mip_level + 30], (*pixels)[mip_level + 60], edge_size,
				CUBE_Y1, CUBE_Y0, false);

			Image_BlendCubeFaceEdge(
				(*pixels)[mip_level + 30], (*pixels)[mip_level + 75], edge_size, 
				CUBE_Y0, CUBE_Y0, true);
		}
	}

	void R_CreateReflectionRawDataFromCubemapShot(game::DiskGfxReflectionProbe* probe_raw_data, const int down_sample_size)
	{
		for (auto f = 0; f < 6; f++)
		{
			R_CubemapShotDownSample(reflectionprobes::cubemapshot_image_ptr[f], get_cubemapshot_res(), down_sample_size);
			Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[f], down_sample_size);
		}

		Image_FlipDiagonally(reflectionprobes::cubemapshot_image_ptr[0], down_sample_size);
		Image_FlipDiagonally(reflectionprobes::cubemapshot_image_ptr[1], down_sample_size);

		Image_FlipHorizontally(reflectionprobes::cubemapshot_image_ptr[1], down_sample_size);

		Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[1], down_sample_size);
		Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[2], down_sample_size);

		Image_FlipHorizontally(reflectionprobes::cubemapshot_image_ptr[3], down_sample_size);

		Image_FlipDiagonally(reflectionprobes::cubemapshot_image_ptr[4], down_sample_size);
		Image_FlipDiagonally(reflectionprobes::cubemapshot_image_ptr[5], down_sample_size);


		std::uint8_t* pixels[6][15];
		const int mipmap_level_size = down_sample_size * 4 * down_sample_size;
		auto raw_pixels = reinterpret_cast<std::uint8_t*>(probe_raw_data->pixels);

		for (auto img_index = 0; img_index < 6; ++img_index)
		{
			pixels[img_index][0] = raw_pixels;
			raw_pixels += mipmap_level_size;
			memcpy(pixels[img_index][0], reflectionprobes::cubemapshot_image_ptr[img_index], mipmap_level_size);
		}


		// # 
		// would normally generate mip maps here .. no need


		// only blend edges of mip 0
		CubeMap_BlendFaceEdges(pixels, 0, down_sample_size);
	}


	bool R_EndCubemapShot(game::CubemapShot shot_index, int probe_num)
	{
		const int img_index = shot_index - 1;
		const int size_in_bytes = 4 * get_cubemapshot_res() * get_cubemapshot_res();

		reflectionprobes::cubemapshot_image_ptr[img_index] = static_cast<std::uint8_t*>(game::Z_VirtualAlloc(size_in_bytes));
		if(!R_TakeCubeMapShot(get_pixel_border(), get_pixel_border(), get_cubemapshot_res(), get_cubemapshot_res(), 4, reflectionprobes::cubemapshot_image_ptr[img_index]))
		{
			return false;
		}

		if(dvars::r_reflectionprobe_export_tga->current.enabled)
		{
			export_cubeshot_side_as_targa(
				"cubemap_shot_n" + std::to_string(probe_num) + "_" + std::to_string(shot_index),
				reflectionprobes::cubemapshot_image_ptr[img_index]);
		}

		Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[img_index], get_cubemapshot_res());

		return true;
	}

	void R_CalcCubeMapViewValues(game::refdef_s* refdef, game::CubemapShot shot, int shot_size)
	{
		refdef->width = shot_size + 2;
		refdef->height = shot_size + 2;
		refdef->tanHalfFovX = (static_cast<float>(shot_size) + 2) / static_cast<float>(shot_size);
		refdef->tanHalfFovY = refdef->tanHalfFovX;
		refdef->zNear = 4.0f; // was 0
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


	// not in use
	void R_CubemapShotSetInitialState()
	{
		const game::vec4_t clear_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::R_SetupRendertarget(game::gfxCmdBufSourceState, game::R_RENDERTARGET_FRAME_BUFFER);
		game::R_SetRenderTarget(game::gfxCmdBufSourceState, game::gfxCmdBufState, game::R_RENDERTARGET_FRAME_BUFFER);
		game::R_ClearScreen(game::gfxCmdBufState->prim.device, 7, clear_color, 1.0f, false, nullptr);
	}

	bool R_GenerateReflectionRawData(game::DiskGfxReflectionProbe* probe, int probe_num)
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

		// final cubemap resolution
		const int down_sample_res = get_cubemapshot_res() / 4; // 64 for 256 size

		// #
		// resize window to cubemap size
		const auto camgui = GET_GUI(ggui::camera_dialog);
		
		const auto saved_width = static_cast<int>(camgui->rtt_get_size().x); //game::dx->windows[ggui::CCAMERAWND].width;
		const auto saved_height = static_cast<int>(camgui->rtt_get_size().y); //game::dx->windows[ggui::CCAMERAWND].height;

		SetWindowPos(
			cmainframe::activewnd->m_pCamWnd->GetWindow(),
			HWND_BOTTOM,
			static_cast<int>(camgui->rtt_get_position().x),
			static_cast<int>(camgui->rtt_get_position().y),
			get_cubemapshot_res() + get_pixel_border(), //+ 2,
			get_cubemapshot_res() + get_pixel_border(), //+ 2,
			SWP_NOZORDER);

		game::dx->targetWindowIndex = -1; // needed or R_SetupRendertarget does not set the resolution
		game::R_SetupRendertarget_CheckDevice(cmainframe::activewnd->m_pCamWnd->GetWindow());


		// #
		// take shots

		for (int i = game::CUBEMAPSHOT_RIGHT; i < game::CUBEMAPSHOT_COUNT; ++i)
		{
			game::R_BeginFrame();

			game::get_frontenddata()->cmds = &game::get_cmdlist()->cmds[game::get_cmdlist()->usedTotal];
			game::R_ClearScene(0);

			R_CalcCubeMapViewValues(&refdef, (game::CubemapShot)i, get_cubemapshot_res());
			game::R_SetLodOrigin(&refdef);

			d3dbsp::add_entities_to_scene();

			// #
			// fx (will only capture them if they are played back)

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

			// # for imgui : save probe sides
			//renderer::copy_scene_to_texture(ggui::CCAMERAWND, reflectionprobes::imgui_cube_surfaces[i-1]);

			if(!R_EndCubemapShot((game::CubemapShot)i, probe_num))
			{
				// #
				// restore orig resolution

				SetWindowPos(
					cmainframe::activewnd->m_pCamWnd->GetWindow(),
					HWND_BOTTOM,
					static_cast<int>(camgui->rtt_get_position().x),
					static_cast<int>(camgui->rtt_get_position().y),
					saved_width,
					saved_height,
					SWP_NOZORDER);

				game::dx->targetWindowIndex = -1; // needed or R_SetupRendertarget does not set the resolution
				utils::hook::call<void(__cdecl)(HWND)>(0x501A70)(cmainframe::activewnd->m_pCamWnd->GetWindow()); // R_SetupRendertarget

				return false;
			}
		}

		R_CreateReflectionRawDataFromCubemapShot(probe, down_sample_res);


#if 0 // save screenshots after rotations
		for (auto face = 0; face < 6; face++)
		{
			export_cubeshot_side_as_targa(
				"cubemap_shot_post_rot" + std::to_string(face),
				reflectionprobes::cubemapshot_image_ptr[img_index]);
		}
#endif


		// #
		// restore orig resolution

		SetWindowPos(
			cmainframe::activewnd->m_pCamWnd->GetWindow(),
			HWND_BOTTOM,
			static_cast<int>(camgui->rtt_get_position().x),
			static_cast<int>(camgui->rtt_get_position().y),
			saved_width,
			saved_height,
			SWP_NOZORDER);

		game::dx->targetWindowIndex = -1; // needed or R_SetupRendertarget does not set the resolution
		game::R_SetupRendertarget_CheckDevice(cmainframe::activewnd->m_pCamWnd->GetWindow());

		return true;
	}

	bool R_GenerateReflectionRawDataAll(game::DiskGfxReflectionProbe* probe_raw_data, unsigned int probe_count, bool* generate_probe)
	{
		for (auto index = 0u; index < probe_count; ++index)
		{
			if (generate_probe[index])
			{
				if(!R_GenerateReflectionRawData(&probe_raw_data[index], index))
				{
					return false;
				}
			}
		}

		return true;
	}

	void reflectionprobes::generate_reflections(game::GfxReflectionProbe* probes, const unsigned int probe_count)
	{
		bool generate_probe[256];

		unsigned int lump_probe_count;
		const auto probe_raw_lump_data = static_cast<const game::DiskGfxReflectionProbe*>(d3dbsp::Com_GetBspLump(d3dbsp::LUMP_REFLECTION_PROBES, sizeof(game::DiskGfxReflectionProbe), &lump_probe_count));

		if (probe_count != lump_probe_count)
		{
			AssertS("reflection probe count does not match bsp's");
			return;
		}

		if (probe_count)
		{
			const unsigned int lump_size = sizeof(game::DiskGfxReflectionProbe) * probe_count;
			const auto probe_raw_generated_data = reinterpret_cast<game::DiskGfxReflectionProbe*>(game::Z_Malloc(lump_size));

			for (auto probe_index = 0u; probe_index < lump_probe_count; ++probe_index)
			{
				memcpy(probe_raw_generated_data[probe_index].colorCorrectionFilename, probe_raw_lump_data[probe_index].colorCorrectionFilename, sizeof(probe_raw_generated_data[probe_index].colorCorrectionFilename));
				probe_raw_generated_data[probe_index].origin[0] = probes[probe_index].origin[0];
				probe_raw_generated_data[probe_index].origin[1] = probes[probe_index].origin[1];
				probe_raw_generated_data[probe_index].origin[2] = probes[probe_index].origin[2];

				// always generate all probes
				generate_probe[probe_index] = true;
			}

			if(R_GenerateReflectionRawDataAll(probe_raw_generated_data, lump_probe_count, generate_probe))
			{
				d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, probe_raw_generated_data, lump_size);
				game::printf_to_console("[Reflections] Reflection probe lump inside bsp written successfully.");
				game::R_GenerateReflectionImages(game::rgp->world->reflectionProbes, probe_raw_generated_data, lump_probe_count, 0);
			}

			game::Z_Free(probe_raw_generated_data);
		}
		else
		{
			d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, nullptr, 0);
		}
	}

	// renderer::on_cam_paint_post_rendercommands
	void reflectionprobes::generate_reflections_for_bsp()
	{
		if(game::rgp->world)
		{
			game::printf_to_console("[Reflections] Generating reflections ...");
			reflectionprobes::generate_reflections(game::rgp->world->reflectionProbes + 1, game::rgp->world->reflectionProbeCount - 1);
		}
	}

	//void check_for_reflection_generation()
	//{
	//	if (dvars::r_reflectionprobe_generate->current.enabled)
	//	{
	//		if(!dvars::r_draw_bsp->current.enabled)
	//		{
	//			game::printf_to_console("[Reflections] Turning on bsp view ...");
	//			command::execute("toggle_bsp_radiant");
	//		}

	//		reflectionprobes::generate_reflections_for_bsp();
	//		dvars::set_bool(dvars::r_reflectionprobe_generate, false);
	//	}
	//}

	//void __declspec(naked) check_for_reflection_generation_stub()
	//{
	//	const static uint32_t func_addr = 0x4FD910; // R_SortMaterials
	//	const static uint32_t retn_addr = 0x403070;
	//	__asm
	//	{
	//		pushad;
	//		call	check_for_reflection_generation;
	//		popad;

	//		call	func_addr;
	//		jmp		retn_addr;
	//	}
	//}

	void reflectionprobes::register_dvars()
	{
		dvars::r_reflectionprobe_generate = dvars::register_bool(
			/* name		*/ "r_reflectionprobe_generate",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "generate all reflection probes for the loaded d3dbsp");

		dvars::r_reflectionprobe_export_tga = dvars::register_bool(
			/* name		*/ "r_reflectionprobe_export_tga",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "export all reflection probes as tga files when building reflections (bin/IW3xRadiant/reflection_probes)");
	}

	reflectionprobes::reflectionprobes()
	{
		// hk 'R_SortMaterials' call after 'R_IssueRenderCommands' in 'CCamWnd::OnPaint'
		//utils::hook(0x40306B, check_for_reflection_generation_stub, HOOK_JUMP).install()->quick();
	}

	reflectionprobes::~reflectionprobes()
	{ }
}
