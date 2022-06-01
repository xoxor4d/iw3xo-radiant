#include "std_include.hpp"

namespace components
{
	//int cubemapshot_res = 256;
	//int cubemapshot_pixel_border = 1;

	/*struct cubeShotGlob_s
	{
		unsigned __int8* pixels[6];
		int end;
	};*/
	//cubeShotGlob_s cubeShotGlob;


	std::uint8_t* reflectionprobes::cubemapshot_image_ptr[6];
	std::uint8_t* reflectionprobes::cubemapshot_temp_image_ptr;

	IDirect3DTexture9* reflectionprobes::imgui_cube_surfaces[6];

	int get_cubemapshot_res()
	{
		int res = GET_GUI(ggui::preferences_dialog)->dev_num_01; //256;
		return res;
	}

	int get_pixel_border()
	{
		int border = GET_GUI(ggui::preferences_dialog)->dev_num_02; //256;
		return border;
	}

	IDirect3DTexture9* cube_textures[6];

	// no diff
	void R_TakeCubeMapShot2(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, std::uint8_t* buffer)
	{
		IDirect3DSurface9* back_buffer = nullptr;
		if (!SUCCEEDED(game::dx->windows[ggui::CCAMERAWND].swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back_buffer)))
		{
			game::Com_Error("ERROR: cannot take screenshot: couldn't get back buffer surface\n");
			back_buffer->Release();
			return;
		}

		IDirect3DTexture9* tex = nullptr;
		D3DXCreateTexture(game::dx->device, width + x, height + y, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_R8G8B8, D3DPOOL_DEFAULT, &tex);

		IDirect3DSurface9* surface;
		tex->GetSurfaceLevel(0, &surface);

		// "copy" backbuffer to our texture surface
		game::dx->device->StretchRect(back_buffer, NULL, surface, NULL, D3DTEXF_NONE);



		D3DSURFACE_DESC desc{};

		if (!SUCCEEDED(surface->GetDesc(&desc)))
		{
			game::Com_Error("ERROR: cannot take screenshot: couldn't get desc\n");
			back_buffer->Release();
			return;
		}

		IDirect3DSurface9* off;
		if (!SUCCEEDED(game::dx->device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &off, nullptr)))
		{
			game::Com_Error("ERROR: cannot take screenshot: couldn't create the off-screen surface\n");
			surface->Release();
			back_buffer->Release();
			return;
		}

		if (!SUCCEEDED(game::dx->device->GetRenderTargetData(surface, off)))
		{
			game::Com_Error("ERROR: cannot take screenshot: GetRenderTargetData failed\n");
			surface->Release();
			back_buffer->Release();
			return;
		}

		tagRECT source_rect{ x, y, width + x, height + y };
		D3DLOCKED_RECT locked_rect{};

		if (!SUCCEEDED(off->LockRect(&locked_rect, &source_rect, D3DLOCK_READONLY)))
		{
			surface->Release();
			back_buffer->Release();
		}

		if (auto bits = static_cast<std::uint8_t*>(locked_rect.pBits))
		{
			//BYTE* pBits = bits;
			if (height > 0)
			{
				int v17 = locked_rect.Pitch - 4 * width;
				int i_height = height;
				do
				{
					if (width > 0)
					{
						int i_width = width;
						do
						{
							buffer[0] = bits[3];
							buffer[1] = bits[2];
							buffer[2] = bits[1];
							buffer[3] = bits[0];
							buffer += 4;
							bits += 4;
							--i_width;
						} while (i_width);
					}
					bits += v17;
					--i_height;
				} while (i_height);
			}

			/*if (locked_rect.Pitch == (4 * width))
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
			}*/
		}

		back_buffer->Release();
	}

	void R_TakeCubeMapShot(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, std::int32_t bytes_per_pixel, std::uint8_t* buffer)
	{
		IDirect3DSurface9* back_buffer = nullptr;
		D3DSURFACE_DESC desc{};
		IDirect3DSurface9* surface = nullptr;
		tagRECT source_rect{ x, y, width + x, height + y };
		D3DLOCKED_RECT locked_rect{};

		if (!SUCCEEDED(game::dx->windows[ggui::CCAMERAWND].swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back_buffer)))
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

		// D3DFMT_A8R8G8B8 --- imgui renderer doing D3DFMT_R8G8B8
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
			//BYTE* pBits = bits;
			// works fine
			if (height > 0)
			{
				int v17 = locked_rect.Pitch - 4 * width;
				int i_height = height;
				do
				{
					if (width > 0)
					{
						int i_width = width;
						do
						{
							buffer[0] = bits[0];
							buffer[1] = bits[1];
							buffer[2] = bits[2];
							buffer[3] = bits[3];
							buffer += 4;
							bits += 4;
							--i_width;
						} while (i_width);
					}
					bits += v17;
					--i_height;
				} while (i_height);
			}

			/*if (bytes_per_pixel == 3)
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
			}*/
		}

		surface->UnlockRect();
		surface->Release();
		back_buffer->Release();
	}

	void tga_write(const char* filename, uint32_t width, uint32_t height, uint8_t* dataBGRA, uint8_t dataChannels = 4, uint8_t fileChannels = 3)
	{
		FILE* fp = NULL;
		// MSVC prefers fopen_s, but it's not portable
		//fp = fopen(filename, "wb");
		fopen_s(&fp, filename, "wb");
		if (fp == NULL) return;

		// You can find details about TGA headers here: http://www.paulbourke.net/dataformats/tga/
		uint8_t header[18] = { 0,0,2,0,0,0,0,0,0,0,0,0, (uint8_t)(width % 256), (uint8_t)(width / 256), (uint8_t)(height % 256), (uint8_t)(height / 256), (uint8_t)(fileChannels * 8), 0x20 };
		fwrite(&header, 18, 1, fp);

		for (uint32_t i = 0; i < width * height; i++)
		{
			for (uint32_t b = 0; b < fileChannels; b++)
			{
				fputc(dataBGRA[(i * dataChannels) + (b % dataChannels)], fp);
			}
		}
		fclose(fp);
	}

	void R_CopyCubemapShot(uint8_t* buffer)
	{
		/*auto cube_res = get_cubemapshot_res();
		if ((cube_res & 0xFFFE) != 0)
		{
			int v13 = 0;
			do
			{
				int v14 = cube_res;
				if (cube_res)
				{
					int v15 = 0;
					do
					{
						int* v16 = (int*)&buffer[4 * v15 + 4 * v14 * v13];
						int v32 = *v16;
						int* v17 = (int*)&buffer[4 * v15 + 4 * (v14 - v13 - 1) * v14];
						*v16 = *v17;
						*v17 = v32;
						++v15;
						v14 = cube_res;
					} while (v15 < cube_res);
				}
				++v13;
			} while (v13 < cube_res >> 1);
		}*/

		// same as above
		int v1; // ebx
		uint8_t* v2; // esi
		uint8_t* v3; // edx
		uint8_t* v4; // eax
		uint8_t* v5; // ecx
		int v6; // edi
		int v7; // esi
		uint8_t* v8; // [esp+8h] [ebp-Ch]
		int v9; // [esp+Ch] [ebp-8h]
		uint8_t* v10; // [esp+10h] [ebp-4h]

		auto cube_res = get_cubemapshot_res();

		v1 = cube_res;
		v2 = buffer;
		if (cube_res >> 1)
		{
			buffer = (uint8_t*)(-4 * cube_res);
			v8 = v2;
			v10 = buffer;
			v3 = &v2[cube_res * (4 * cube_res - 4)];
			v9 = cube_res >> 1;
			do
			{
				if (v1 > 0)
				{
					v4 = v8;
					v5 = v3;
					v6 = v1;
					do
					{
						v7 = *(DWORD*)v4;
						*(DWORD*)v4 = *(DWORD*)v5;
						*(DWORD*)v5 = v7;
						v4 += 4;
						v5 += 4;
						--v6;
					} while (v6);
					buffer = v10;
				}
				v8 += 4 * v1;
				v3 = &v3[(DWORD)buffer];
				--v9;
			} while (v9);
		}
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

	struct __declspec(align(4)) cubeMapUtilsGlob_s
	{
		float cubeDirs[6][64][64][3];
		bool cubeDirsInited;
	};

	cubeMapUtilsGlob_s cubeMapUtilsGlob = {};

	void Image_CubeMapDir(int pixel, float* dir, int face, signed int u, signed int v)
	{
		switch (face)
		{
		case 0:
			dir[0] = (float)(pixel - 1);
			dir[1] = (float)(pixel - v - 1);
			dir[2] = (float)(pixel - u - 1);
			break;
		case 1:
			dir[0] = 0.0;
			dir[1] = (float)(pixel - v - 1);
			dir[2] = (float)u;
			break;
		case 2:
			dir[0] = (float)u;
			dir[1] = (float)(pixel - 1);
			dir[2] = (float)v;
			break;
		case 3:
			dir[0] = (float)u;
			dir[1] = 0.0f;
			dir[2] = (float)(pixel - v - 1);
			break;
		case 4:
			dir[0] = (float)u;
			dir[1] = (float)(pixel - v - 1);
			dir[2] = (float)(pixel - 1);
			break;
		case 5:
			dir[0] = (float)(pixel - u - 1);
			dir[1] = (float)(pixel - v - 1);
			dir[2] = 0.0;
			break;
		default:
			break;
		}

		const float f_pix = (float)(pixel - 1);
		dir[0] = dir[0] / f_pix * 2.0f - 1.0f;
		dir[1] = dir[1] / f_pix * 2.0f - 1.0f;
		dir[2] = dir[2] / f_pix * 2.0f - 1.0f;
		fx_system::Vec3Normalize(dir);
	}

	void CubeMap_CacheHighMipDirs()
	{
		if (!cubeMapUtilsGlob.cubeDirsInited)
		{
			cubeMapUtilsGlob.cubeDirsInited = true;
			for (auto face = 0; face < 6; ++face)
			{
				for (auto v = 0; v < 64; ++v)
				{
					for (auto u = 0; u < 64; ++u)
					{
						// Image_CubeMapDir(face, u, v, 64, cubeMapUtilsGlob.cubeDirs[face][v][u]);
						Image_CubeMapDir(64, cubeMapUtilsGlob.cubeDirs[face][v][u], face, u, v);
					}
				}
			}
		}
	}

	void CubeMap_InitSideForIndex(float* dir, int imgIndex)
	{
		switch (imgIndex)
		{
		case 0:
			dir[0] = 1.0f;
			dir[1] = 0.0f;
			dir[2] = 0.0f;
			break;
		case 1:
			dir[0] = -1.0f;
			dir[1] = 0.0f;
			dir[2] = 0.0f;
			break;
		case 2:
			dir[0] = 0.0f;
			dir[1] = 1.0f;
			dir[2] = 0.0f;
			break;
		case 3:
			dir[0] = 0.0f;
			dir[1] = -1.0f;
			dir[2] = 0.0f;
			break;
		case 4:
			dir[0] = 0.0f;
			dir[1] = 0.0f;
			dir[2] = 1.0f;
			break;
		case 5:
			dir[0] = 0.0f;
			dir[1] = 0.0f;
			dir[2] = -1.0f;
			break;
		default:
			return;
		}
	}

	float CubeMap_GenerateMipMap32_sub(int size)
	{
		switch (size)
		{
		case 1:
			return cosf(3.141592741012573f);

		case 2:
			return cosf(1.570796370506287f);

		case 4:
			return cosf(0.7853981852531433f);

		case 8:
			return cosf(0.3839724361896515f);

		case 16:
			return cosf(0.1919862180948257f);

		default:
			return cosf(0.0872664600610733f);
		}
	}

	void Image_CalcCubeMipMapTexel32_sub(int a1, float* a2, int a3, int a4, int a5, int a6, float a7, int a8)
	{
		unsigned __int8* v9; // eax
		float v10; // [esp+Ch] [ebp-10h]
		float v11; // [esp+10h] [ebp-Ch]
		float v12; // [esp+14h] [ebp-8h]
		float v13; // [esp+18h] [ebp-4h]
		float v14; // [esp+20h] [ebp+4h]
		int v15; // [esp+24h] [ebp+8h]

		*(float*)&v15 = cubeMapUtilsGlob.cubeDirs[0][a1 + (a3 << 6)][a4][1] * *(float*)(a5 + 4) + cubeMapUtilsGlob.cubeDirs[0][a1 + (a3 << 6)][a4][0] * *(float*)a5 + cubeMapUtilsGlob.cubeDirs[0][a1 + (a3 << 6)][a4][2] * *(float*)(a5 + 8);
		if (a7 < (double)*(float*)&v15)
		{
			v14 = *(float*)&v15 - a7;
			v9 = (unsigned __int8*)(*(DWORD*)(a6 + 60 * a3) + 4 * (a4 + (a1 << 6)));
			v10 = (double)*v9 / 255.0;
			v11 = (double)v9[1] / 255.0;
			v12 = (double)v9[2] / 255.0;
			v13 = (double)v9[3] / 255.0;
			*a2 = v10 * v14 + *a2;
			a2[1] = v11 * v14 + a2[1];
			a2[2] = v12 * v14 + a2[2];
			a2[3] = v13 * v14 + a2[3];
			*(float*)a8 = v14 + *(float*)a8;
		}
	}

	void Image_CalcCubeMipMapTexel32(unsigned __int8* data, float* dir, unsigned __int8* pixels, float angle)
	{
		int face; // edi
		int v; // ebx
		int u; // esi
		double v7; // st7
		int l; // ecx
		double v9; // st4
		double v10; // st4
		unsigned __int8* result; // eax
		int v12; // [esp+18h] [ebp-18h] BYREF
		int v13; // [esp+1Ch] [ebp-14h]
		float vec[4]; // [esp+20h] [ebp-10h] BYREF

		*(float*)&v12 = 0.0;
		vec[0] = 0.0;
		vec[1] = 0.0;
		vec[2] = 0.0;
		vec[3] = 0.0;
		for (face = 0; face < 6; ++face)
		{
			for (v = 0; v < 64; ++v)
			{
				for (u = 0; u < 64; ++u)
				{
					Image_CalcCubeMipMapTexel32_sub(v, vec, face, u, (int)dir, (int)pixels, angle, (int)&v12);
				}
			}
		}
		v7 = *(float*)&v12;
		for (l = 0;
			l < 4;
			data[l - 1] = v13)
		{
			*(float*)&v12 = vec[l] / v7;
			v9 = *(float*)&v12;
			vec[l] = *(float*)&v12;
			if (v9 > 1.0)
			{
				vec[l] = 1.0;
			}
			v10 = vec[l++];
			result = data;
			v13 = (int)(v10 * 255.0);
		}
	}

	void CubeMap_GenerateMipMap32(unsigned __int8* data, int scaledSize, unsigned __int8* pixels, int imgIndex)
	{
		int pix; // edi
		signed int v; // ebp
		unsigned __int8* data_next_x;
		signed int u; // esi
		unsigned __int8* data_next; // [esp+14h] [ebp-14h]
		float v10; // [esp+18h] [ebp-10h]
		float dir[3]; // [esp+1Ch] [ebp-Ch] BYREF

		pix = scaledSize >> 1;
		v10 = CubeMap_GenerateMipMap32_sub(scaledSize >> 1);
		if (pix == 1)
		{
			CubeMap_InitSideForIndex(dir, imgIndex);
			Image_CalcCubeMipMapTexel32(data, dir, pixels, v10);
		}
		else
		{
			v = 0;
			if (pix > 0)
			{
				data_next = data;
				do
				{
					data_next_x = data_next;
					for (u = 0;
						u < pix;
						++u)
					{
						Image_CubeMapDir(pix, dir, imgIndex, u, v);
						Image_CalcCubeMipMapTexel32(data_next_x, dir, pixels, v10);
						data_next_x += 4;
					}
					data_next += 4 * pix;
					++v;
				} while (v < pix);
			}
		}
	}

	// working
	void Image_FlipVertically(uint8_t* pbuff)
	{
		int v2; // ebx
		uint8_t* result; // eax
		uint8_t* v4; // ecx
		int v5; // esi
		int v6; // edx

		v2 = 64;
		do
		{
			result = pbuff;
			v4 = pbuff + 16128;
			v5 = 32;
			do
			{
				v6 = *(DWORD*)result;
				*(DWORD*)result = *(DWORD*)v4;
				*(DWORD*)v4 = v6;
				result += 256;
				v4 -= 256;
				--v5;
			} while (v5);
			pbuff += 4;
			--v2;
		} while (v2);
	}

	// working (waw-mp)
	void R_CubemapShotFlipVertical(uint8_t* pbuff)
	{
		uint8_t* v1; // ecx
		int v2; // edi
		uint8_t* v3; // ebx
		uint8_t* v4; // eax
		int v5; // esi
		int v6; // edx
		uint8_t* v7; // [esp+10h] [ebp-4h]

		v1 = pbuff + 4;
		v2 = 1;
		v7 = pbuff + 4;
		v3 = pbuff + 256;
		do
		{
			if (v2 > 0)
			{
				v4 = v3;
				v5 = v2;
				do
				{
					v6 = *(DWORD*)v1;
					*(DWORD*)v1 = *(DWORD*)v4;
					*(DWORD*)v4 = v6;
					v4 += 4;
					v1 += 256;
					--v5;
				} while (v5);
			}
			++v2;
			v1 = v7 + 4;
			v3 += 256;
			v7 += 4;
		} while (v2 < 64);
	}

	// working
	void R_CubemapShotRotateAntiClockwise(uint8_t* pbuff)
	{
		int v2; // edi
		signed int result; // eax
		uint8_t* v4; // ecx
		int v5; // esi

		v2 = 64;
		do
		{
			result = 0;
			v4 = pbuff + 252;
			do
			{
				v5 = *(DWORD*)&pbuff[4 * result];
				*(DWORD*)&pbuff[4 * result] = *(DWORD*)v4;
				*(DWORD*)v4 = v5;
				++result;
				v4 -= 4;
			} while (result < 32);
			pbuff += 256;
			--v2;
		} while (v2);
	}

	// pseudo from waw-mp seems to work
	void R_CubemapShotRotateClockwise(int cubemapShotRes, uint8_t* pbuff)
	{
		uint8_t* v2; // ebx
		uint8_t* v3; // esi
		float v4; // xmm0_4
		float v5; // xmm1_4
		float v6; // xmm2_4
		float v7; // xmm3_4
		uint8_t* v8; // edx
		int v9; // edi
		uint8_t* v10; // eax
		int v11; // ecx
		int v12; // ebp
		float v13; // xmm6_4
		int result; // eax
		int v15; // xmm0_4
		bool v16; // zf
		int v17; // [esp+0h] [ebp-24h]
		uint8_t* v18; // [esp+4h] [ebp-20h]
		int v19; // [esp+8h] [ebp-1Ch]
		int v20; // [esp+Ch] [ebp-18h]
		int v21; // [esp+10h] [ebp-14h]
		int v22[4]; // [esp+14h] [ebp-10h]

		v2 = pbuff + 2;
		v20 = 4 * cubemapShotRes;
		v21 = 16 * cubemapShotRes;
		v18 = pbuff + 2;
		v19 = 64;
		do
		{
			v3 = pbuff;
			v17 = 64;
			do
			{
				v4 = 0.0;
				v5 = 0.0;
				v6 = 0.0;
				v7 = 0.0;
				v8 = v2;
				v9 = 4;
				do
				{
					v10 = v8;
					v11 = 4;
					do
					{
						v4 = (float)*(v10 - 2) + v4;
						v5 = (float)*(v10 - 1) + v5;
						v12 = v10[1];
						v6 = (float)*v10 + v6;
						v10 += 4;
						--v11;
						v13 = (float)v12 + v7;
						v7 = v13;
					} while (v11);
					v8 += v20;
					--v9;
				} while (v9);
				*(float*)&v22[3] = v13;
				*(float*)&v22[2] = v6;
				*(float*)&v22[1] = v5;
				*(float*)v22 = v4;
				for (result = 0; result < 4; ++result)
				{
					*(float*)&v15 = *(float*)&v22[result] * 0.0625;
					v22[result] = v15;
					v3[result] = (int)*(float*)&v15;
				}
				v2 += 16;
				v3 += 4;
				--v17;
			} while (v17);
			v2 = &v18[v21];
			v16 = v19-- == 1;
			pbuff = v3;
			v18 += v21;
		} while (!v16);
	}

	void R_CreateReflectionRawDataFromCubemapShot(game::DiskGfxReflectionProbe* probeRawData)
	{
		for(auto face = 0; face < 6; face++)
		{
			R_CubemapShotRotateClockwise(get_cubemapshot_res(), reflectionprobes::cubemapshot_image_ptr[face]);
			Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[face]);
		}


		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[0]);
		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[1]);

		R_CubemapShotRotateAntiClockwise(reflectionprobes::cubemapshot_image_ptr[1]);

		Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[1]);
		Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[2]);

		R_CubemapShotRotateAntiClockwise(reflectionprobes::cubemapshot_image_ptr[3]);

		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[4]);
		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[5]);

		for (auto face = 0; face < 6; face++)
		{
			if (dvars::fs_homepath)
			{
				std::string filePath = dvars::fs_homepath->current.string;
				filePath += "\\lump_dumps\\";

				std::filesystem::create_directories(filePath);
				filePath += "post_copy_cubemap_shot_" + std::to_string(face) + ".tga";

				tga_write(
					filePath.c_str(),
					get_cubemapshot_res(),
					get_cubemapshot_res(),
					reflectionprobes::cubemapshot_image_ptr[face],
					4,
					4);
			}
		}
		

		

		

//#if 0
//		unsigned __int8* pixels[6][15];
//		char* _pixels = probeRawData->pixels;
//		void** dest = (void**)pixels;
//		cubeShotGlob_s* probe_face_img = &cubeShotGlob;
//		do
//		{
//			unsigned __int8* src = probe_face_img->pixels[0];
//			*dest = _pixels;
//			_pixels += 16384;
//			memcpy(*dest, src, 16384u);
//			probe_face_img = (cubeShotGlob_s*)((char*)probe_face_img + 4);
//			dest += 15;
//		} while ((int)probe_face_img < (int)&cubeShotGlob.end);
//#endif

//#if 1
		int downSampleRes = get_cubemapshot_res() / 4; //get_cubemapshot_res(); // 64 for 256 size
		int v2; // [esp+0h] [ebp-190h]
		int mip; // [esp+4h] [ebp-18Ch]
		unsigned __int8* data; // [esp+Ch] [ebp-184h]
		unsigned __int8* pixels[6][15]; // [esp+10h] [ebp-180h]
		unsigned __int8* rawPixels; // [esp+17Ch] [ebp-14h]
		int mipmapLevelSize; // [esp+180h] [ebp-10h]
		int imgIndex; // [esp+184h] [ebp-Ch]
		int scaledSize; // [esp+188h] [ebp-8h]
		int mipLevel = 0; // [esp+18Ch] [ebp-4h]

		mipmapLevelSize = downSampleRes * 4 * downSampleRes;
		rawPixels = reinterpret_cast<unsigned __int8*>(probeRawData->pixels);

		for (imgIndex = 0; imgIndex < 6; ++imgIndex)
		{
			pixels[imgIndex][0] = rawPixels;
			rawPixels += mipmapLevelSize;
			memcpy(pixels[imgIndex][0], reflectionprobes::cubemapshot_image_ptr[imgIndex], mipmapLevelSize);
		}
//#endif

		// does not change a thing

		//CubeMap_CacheHighMipDirs();
		//for (imgIndex = 0; imgIndex < 6; ++imgIndex)
		//{
		//	scaledSize = downSampleRes;
		//	data = reflectionprobes::cubemapshot_image_ptr[imgIndex];
		//	mipLevel = 1; // was 1
		//	do
		//	{
		//		//CubeMap_GenerateMipMap32(data, scaledSize, pixels, downSampleRes, imgIndex);
		//		CubeMap_GenerateMipMap32(data, scaledSize, (unsigned __int8*)pixels, imgIndex);
		//		if (scaledSize >> 1 >= 1)
		//			v2 = scaledSize >> 1;
		//		else
		//			v2 = 1;
		//		scaledSize = v2;
		//		mipmapLevelSize = v2 * 4 * v2;
		//		pixels[imgIndex][mipLevel] = rawPixels;
		//		rawPixels += mipmapLevelSize;
		//		memcpy(pixels[imgIndex][mipLevel++], data, mipmapLevelSize);
		//	} while (scaledSize != 1);
		//}

		//for (mip = 0; mip < mipLevel; ++mip)
		//	CubeMap_BlendFaceEdges(pixels, mip, downSampleRes >> mip);

		//for (imgIndex = 0; imgIndex < 6; ++imgIndex)
		//	game::Z_Free(reflectionprobes::cubemapshot_image_ptr[imgIndex]);
	}

	

	void R_EndCubemapShot(game::CubemapShot shotIndex)
	{
		const int img_index = shotIndex - 1;
		const int size_in_bytes = 4 * (get_cubemapshot_res()) * (get_cubemapshot_res());

		reflectionprobes::cubemapshot_image_ptr[img_index] = static_cast<std::uint8_t*>(game::Z_VirtualAlloc(size_in_bytes));

		if(!reflectionprobes::cubemapshot_temp_image_ptr)
		{
			reflectionprobes::cubemapshot_temp_image_ptr = static_cast<std::uint8_t*>(game::Z_VirtualAlloc(size_in_bytes));
		}
		
		//cubeShotGlob.pixels[img_index] = static_cast<std::uint8_t*>(game::Z_VirtualAlloc(size_in_bytes));

		R_TakeCubeMapShot(get_pixel_border(), get_pixel_border(), get_cubemapshot_res() /*+ 2*/, get_cubemapshot_res() /*+ 2*/, 4, reflectionprobes::cubemapshot_image_ptr[img_index]);
		//R_TakeCubeMapShot2(get_pixel_border(), get_pixel_border(), get_cubemapshot_res(), get_cubemapshot_res(), reflectionprobes::cubemapshot_image_ptr[img_index]);

		dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
		if (dvars::fs_homepath)
		{
			std::string filePath = dvars::fs_homepath->current.string;
			filePath += "\\lump_dumps\\";

			std::filesystem::create_directories(filePath);
			filePath += "cubemap_shot_" + std::to_string(shotIndex) + ".tga";

			tga_write(
				filePath.c_str(),
				get_cubemapshot_res(),
				get_cubemapshot_res(),
				reflectionprobes::cubemapshot_image_ptr[img_index],
				4,
				4);
		}

		// flips images upside down (vertical mirror)
		R_CopyCubemapShot(reflectionprobes::cubemapshot_image_ptr[img_index]);

		

		//R_CopyCubemapShot(cubeShotGlob.pixels[img_index]);

		if(reflectionprobes::cubemapshot_temp_image_ptr)
		{
			game::Z_Free(reflectionprobes::cubemapshot_temp_image_ptr);
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
		refdef.useScissorViewport = true;
		refdef.blurRadius = 0.0f;
		memcpy(&refdef.primaryLights, &d3dbsp::scene_lights, sizeof(d3dbsp::scene_lights));


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

		//game::dx->windows[ggui::CCAMERAWND].width = 128;
		//game::dx->windows[ggui::CCAMERAWND].height = 128;

		// R_SetupRendertarget
		game::dx->targetWindowIndex = -1;
		if (utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(cmainframe::activewnd->m_pCamWnd->GetWindow()))
		{
			//int x = 1;
		}

		for (int i = game::CUBEMAPSHOT_RIGHT; i < game::CUBEMAPSHOT_COUNT; ++i)
		{
			//R_CubemapShotSetInitialState();
			game::R_BeginFrame();

			game::get_frontenddata()->cmds = &game::get_cmdlist()->cmds[game::get_cmdlist()->usedTotal];
			game::R_ClearScene(0);

			R_CalcCubeMapViewValues(&refdef, (game::CubemapShot)i, get_cubemapshot_res());
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

			renderer::copy_scene_to_texture(ggui::CCAMERAWND, reflectionprobes::imgui_cube_surfaces[i-1]);
			R_EndCubemapShot((game::CubemapShot)i);

			// R_CubemapShotRestoreState
			//game::R_SetupRendertarget(game::gfxCmdBufSourceState, game::R_RENDERTARGET_FRAME_BUFFER);
			//game::R_SetRenderTarget(game::gfxCmdBufSourceState, game::gfxCmdBufState, game::R_RENDERTARGET_FRAME_BUFFER);
		}

		R_CreateReflectionRawDataFromCubemapShot(probe);

		// same as above
		/*unsigned __int8* pixels[6][15] = {};
		int mipmapLevelSize = 4 * 64 * 64;
		char* rawPixels = probe->pixels;
		int* v13 = (int*)pixels;
		for (auto face = 0; face < 6; face++)
		{
			*v13 = (int)rawPixels;
			char* v15 = &rawPixels[mipmapLevelSize];
			memcpy(rawPixels, reflectionprobes::cubemapshot_image_ptr[face], mipmapLevelSize);
			v13 += 15;
			rawPixels = v15;
		}*/

		// same as above
		//int v18;
		//auto _pixels = probe->pixels;
		//void** dest = (void**)&v18;
		//auto probe_face_img = (const void**)&reflectionprobes::cubemapshot_image_ptr;

		//for(auto face = 0; face < 6; face++)
		//{
		//	const void* src = *probe_face_img;
		//	*dest = _pixels;
		//	_pixels += 16384; //16384;
		//	memcpy(*dest, src, 16384u); // if 512: 65920u
		//	++probe_face_img;
		//	dest += 15;
		//}


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
		
		//game::dx->windows[ggui::CCAMERAWND].width = saved_width;
		//game::dx->windows[ggui::CCAMERAWND].height = saved_height;

		// R_SetupRendertarget
		game::dx->targetWindowIndex = -1;
		if (utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(cmainframe::activewnd->m_pCamWnd->GetWindow()))
		{
			//int x = 1;
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


		dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
		if (dvars::fs_homepath)
		{
			std::string filePath = dvars::fs_homepath->current.string;
			filePath += "\\lump_dumps\\";

			std::filesystem::create_directories(filePath);

			std::ofstream reflection_lump(filePath + "orig_reflection_lump.bin", std::ios::binary);
			if (!reflection_lump.is_open())
			{
				game::printf_to_console("Could not create reflection_lump binary file. Aborting!\n");
				return;
			}

			reflection_lump.write(reinterpret_cast<const char*>(probe_raw_lump_data), sizeof(game::DiskGfxReflectionProbe));
			reflection_lump.close();
		}



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


			dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
			if (dvars::fs_homepath)
			{
				std::string filePath = dvars::fs_homepath->current.string;
				filePath += "\\lump_dumps\\";

				std::filesystem::create_directories(filePath);

				std::ofstream reflection_lump(filePath + "new_reflection_lump.bin", std::ios::binary);
				if (!reflection_lump.is_open())
				{
					game::printf_to_console("Could not create reflection_lump binary file. Aborting!\n");
					return;
				}

				reflection_lump.write(reinterpret_cast<const char*>(probe_raw_generated_data), sizeof(game::DiskGfxReflectionProbe));
				reflection_lump.close(); 
			}

			d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, probe_raw_generated_data, lump_size);

			// lump saving works fine -> test with mp_tool probe data
			//d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, probe_raw_lump_data, lump_size); // custom one

			game::R_GenerateReflectionImages(game::rgp->world->reflectionProbes, probe_raw_generated_data, lumpProbeCount, 0); //
			game::Z_Free(probe_raw_generated_data);
		}
		else
		{
			d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, nullptr, 0);
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
