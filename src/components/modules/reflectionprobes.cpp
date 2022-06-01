#include "std_include.hpp"

namespace components
{
	std::uint8_t* reflectionprobes::cubemapshot_image_ptr[6];
	IDirect3DTexture9* reflectionprobes::imgui_cube_surfaces[6];

	int get_cubemapshot_res()
	{
		int res = GET_GUI(ggui::preferences_dialog)->dev_num_01; // 256
		return res;
	}

	int get_pixel_border()
	{
		const int border = GET_GUI(ggui::preferences_dialog)->dev_num_02; // 1
		return border;
	}

	void R_TakeCubeMapShot(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height, std::int32_t bytes_per_pixel, std::uint8_t* buffer)
	{
		IDirect3DSurface9* back_buffer = nullptr;
		IDirect3DSurface9* surface = nullptr;
		D3DSURFACE_DESC desc = {};
		D3DLOCKED_RECT locked_rect = {};
		const tagRECT source_rect { x, y, width + x, height + y };

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
					buffer += (4 * width);
					bits += locked_rect.Pitch;
				}
			}
		}

		surface->UnlockRect();
		surface->Release();
		back_buffer->Release();
	}

	// https://gist.github.com/maluoi/ade07688e741ab188841223b8ffeed22 (MIT!)
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

	// no ext
	void export_cubeshot_side_as_targa(const std::string& filename, uint8_t* dataBGRA)
	{
		if (dvars::fs_homepath = game::Dvar_FindVar("fs_homepath"); 
			dvars::fs_homepath)
		{
			std::string filePath = dvars::fs_homepath->current.string;
			filePath += "\\IW3xRadiant\\reflection_probes\\";

			std::filesystem::create_directories(filePath);
			filePath += filename + ".tga"s;

			tga_write(
				filePath.c_str(),
				get_cubemapshot_res(),
				get_cubemapshot_res(),
				dataBGRA,
				4,
				4);
		}
	}

	// ouch
	void R_CopyCubemapShot(uint8_t* buffer)
	{
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
	
	struct __declspec(align(4)) cubeMapUtilsGlob_s
	{
		float cubeDirs[6][64][64][3];
		bool cubeDirsInited;
	}; cubeMapUtilsGlob_s cubeMapUtilsGlob = {};

	// not needed
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

	// not needed
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
						Image_CubeMapDir(64, cubeMapUtilsGlob.cubeDirs[face][v][u], face, u, v);
					}
				}
			}
		}
	}

	// not needed
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

	// not needed
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

	// ouch - not needed
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

	// ouch - not needed
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

	// ouch - not needed
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

	// ouch
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

	// ouch :: waw-mp
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

	// ouch
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

	// ouch :: waw-mp
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

	// ouch
	void Image_BlendCubeCorner(int a1, int a2, int a3, int a4, int a5, int a6)
	{
		int v6; // ecx
		int v7; // ebx
		int v8; // edi
		int v9; // edx
		int v10; // ebp
		int v11; // esi
		int v12; // eax
		int v13; // edx
		unsigned __int8* v14; // esi
		unsigned __int8* v15; // ecx
		int v16; // kr00_4
		int v17; // kr04_4
		int v18; // eax
		int v19; // eax
		int v20; // [esp+4h] [ebp-28h]
		int v21; // [esp+8h] [ebp-24h]
		int v22; // [esp+Ch] [ebp-20h]
		int v23; // [esp+10h] [ebp-1Ch]
		int v24; // [esp+14h] [ebp-18h]
		int v25; // [esp+18h] [ebp-14h]
		int v26; // [esp+1Ch] [ebp-10h]
		int v27; // [esp+20h] [ebp-Ch]
		int v28; // [esp+28h] [ebp-4h]

		v6 = a3;
		v7 = 0;
		if (a3 > 0)
		{
			v24 = a3 - 1;
			v8 = 4 * a3;
			v9 = 4 * a3 - 4;
			v26 = 4 * a3 + a1 - 4;
			v10 = -4 * a3;
			v20 = 0;
			v25 = 0;
			v23 = v9;
			v22 = a3 * v9;
			v21 = a3 * v9;
			while (1)
			{
				v11 = a1;
				if (a6 == 1)
				{
					v12 = v24;
					v27 = v21;
					v11 = a1;
					v28 = v9;
					v13 = v22;
				}
				else
				{
					v27 = v20;
					v28 = 4 * v7;
					v13 = v25;
					v12 = v7;
				}
				switch (a4)
				{
				case 0:
					v14 = (unsigned __int8*)(v20 + v11);
					break;
				case 1:
					v14 = (unsigned __int8*)v26;
					break;
				case 2:
					v14 = (unsigned __int8*)(v11 + 4 * v7);
					break;
				case 3:
					v14 = (unsigned __int8*)(v11 + 4 * (v7 + v6 * (v6 - 1)));
					break;
				default:
					v14 = 0;
					break;
				}
				switch (a5)
				{
				case 0:
					v15 = (unsigned __int8*)(a2 + v27);
					break;
				case 1:
					v15 = (unsigned __int8*)(v8 + v13 + a2 - 4);
					break;
				case 2:
					v15 = (unsigned __int8*)(v28 + a2);
					break;
				case 3:
					v15 = (unsigned __int8*)(a2 + 4 * (v12 + v6 * (v6 - 1)));
					break;
				default:
					v15 = 0;
					break;
				}
				v16 = *v14 + *v15;
				*v14 = v16 / 2;
				*v15 = v16 / 2;
				v17 = v14[1] + v15[1];
				v14[1] = v17 / 2;
				v15[1] = v17 / 2;
				v21 += v10;
				v22 += v10;
				v23 -= 4;
				--v24;
				v20 += v8;
				v25 += v8;
				v26 += v8;
				v18 = (v14[2] + v15[2]) / 2;
				v14[2] = v18;
				v15[2] = v18;
				v19 = (v14[3] + v15[3]) / 2;
				++v7;
				v14[3] = v19;
				v15[3] = v19;
				if (v7 >= a3)
				{
					break;
				}
				v6 = a3;
				v9 = v23;
			}
		}
	}

	// ouch
	void CubeMap_BlendFaceEdges_sub(int a1, int a2)
	{
		int v2; // ebp
		int* v3; // edi
		int v4; // ecx
		int v5; // esi
		int* result; // eax
		int v7; // edx
		float v8; // [esp+10h] [ebp-1Ch]
		float v9; // [esp+10h] [ebp-1Ch]
		float v10; // [esp+10h] [ebp-1Ch]
		float v11; // [esp+10h] [ebp-1Ch]
		float v12; // [esp+10h] [ebp-1Ch]
		float v13; // [esp+10h] [ebp-1Ch]
		float v14; // [esp+10h] [ebp-1Ch]
		int v15; // [esp+14h] [ebp-18h]
		int v16; // [esp+18h] [ebp-14h]
		int v17; // [esp+1Ch] [ebp-10h]
		int v18; // [esp+20h] [ebp-Ch]
		int v19; // [esp+24h] [ebp-8h]

		v2 = *(DWORD*)(a2 + 4 * a1);
		v15 = *(DWORD*)(a2 + 4 * a1 + 60);
		v16 = *(DWORD*)(a2 + 4 * a1 + 120);
		v3 = (int*)(a2 + 4 * a1);
		v17 = v3[45];
		v4 = 0;
		v18 = v3[60];
		v19 = v3[75];
		do
		{
			v5 = 6;
			v8 = (double)*(unsigned __int8*)(v4 + v2) + 0.0;
			v9 = v8 + (double)*(unsigned __int8*)(v15 + v4);
			v10 = v9 + (double)*(unsigned __int8*)(v16 + v4);
			v11 = v10 + (double)*(unsigned __int8*)(v17 + v4);
			v12 = v11 + (double)*(unsigned __int8*)(v18 + v4);
			v13 = v12 + (double)*(unsigned __int8*)(v19 + v4);
			v14 = v13 / 6.0;
			result = v3;
			do
			{
				v7 = *result;
				result += 15;
				--v5;
				*(BYTE*)(v4 + v7) = (int)v14;
			} while (v5);
			++v4;
		} while (v4 < 4);
	}

	// ouch
	void CubeMap_BlendFaceEdges(int mip, unsigned char* pixels, int imgIndex)
	{
		unsigned __int8* v3; // ebx
		unsigned __int8* v4; // ecx
		int v5; // esi
		int v6; // eax
		bool v7; // zf
		unsigned __int8 v8; // al
		unsigned __int8* v9; // ecx
		double v10; // st6
		unsigned __int8 v11; // al
		unsigned __int8* v12; // ecx
		int v13; // eax
		double v14; // st6
		unsigned __int8 v15; // al
		unsigned __int8* v16; // ecx
		unsigned __int8 v17; // al
		unsigned __int8* v18; // ecx
		int v19; // eax
		unsigned __int8 v20; // al
		unsigned __int8* v21; // ecx
		int v22; // eax
		unsigned __int8 v23; // al
		unsigned __int8* v24; // ecx
		int v25; // eax
		unsigned __int8 v26; // al
		unsigned __int8* v27; // ecx
		int v28; // eax
		unsigned __int8 v29; // al
		unsigned __int8* v30; // ecx
		unsigned __int8* v31; // ebx
		int v32; // kr00_4
		int v33; // kr04_4
		int v34; // kr08_4
		int v35; // eax
		unsigned __int8* v36; // ecx
		unsigned __int8* v37; // ebx
		int v38; // kr10_4
		int v39; // kr14_4
		int v40; // kr18_4
		int v41; // eax
		int v42; // ebp
		unsigned __int8* v43; // ebx
		unsigned __int8* v44; // ecx
		int v45; // esi
		int v46; // kr20_4
		int v47; // kr24_4
		int v48; // edx
		int v49; // kr28_4
		int v50; // edx
		int v51; // eax
		int v52; // [esp+8h] [ebp-10h]
		int v53; // [esp+8h] [ebp-10h]
		int v54; // [esp+8h] [ebp-10h]
		int v55; // [esp+8h] [ebp-10h]
		int v56; // [esp+8h] [ebp-10h]
		int v57; // [esp+8h] [ebp-10h]
		int v58; // [esp+8h] [ebp-10h]
		int v59; // [esp+8h] [ebp-10h]
		int v60; // [esp+Ch] [ebp-Ch]
		int v61; // [esp+Ch] [ebp-Ch]
		int v62; // [esp+Ch] [ebp-Ch]
		int v63; // [esp+Ch] [ebp-Ch]
		int v64; // [esp+Ch] [ebp-Ch]
		int v65; // [esp+Ch] [ebp-Ch]
		int v66; // [esp+Ch] [ebp-Ch]
		int v67; // [esp+Ch] [ebp-Ch]
		int v68; // [esp+Ch] [ebp-Ch]
		int v69; // [esp+10h] [ebp-8h]
		int v70; // [esp+10h] [ebp-8h]
		int v71; // [esp+10h] [ebp-8h]
		int v72; // [esp+10h] [ebp-8h]
		int v73; // [esp+10h] [ebp-8h]
		int v74; // [esp+10h] [ebp-8h]
		int v75; // [esp+10h] [ebp-8h]
		int v76; // [esp+10h] [ebp-8h]
		int v77; // [esp+10h] [ebp-8h]
		int v78; // [esp+10h] [ebp-8h]
		int v79; // [esp+14h] [ebp-4h]
		int v80; // [esp+14h] [ebp-4h]
		int v81; // [esp+14h] [ebp-4h]
		int v82; // [esp+14h] [ebp-4h]
		int v83; // [esp+14h] [ebp-4h]
		int v84; // [esp+14h] [ebp-4h]
		int v85; // [esp+14h] [ebp-4h]

		if (mip == 1)
		{
			CubeMap_BlendFaceEdges_sub(imgIndex, (int)pixels);
		}
		else
		{
			v3 = pixels;
			v4 = *(unsigned __int8**)&pixels[4 * imgIndex];
			v5 = mip - 1;
			v52 = *(DWORD*)&pixels[4 * imgIndex + 240] + 4 * (mip - 1) - (DWORD)v4;
			v6 = *(DWORD*)&pixels[4 * imgIndex + 120] + 4 * (mip - 1 + mip * (mip - 1)) - (DWORD)v4;
			v79 = v6;
			v60 = 4;
			do
			{
				v69 = *v4 + v4[v6] + v4[v52];
				++v4;
				v7 = v60-- == 1;
				v8 = (int)((double)v69 / 3.0);
				*(v4 - 1) = v8;
				v4[v52 - 1] = v8;
				v6 = v79;
				v4[v79 - 1] = *(v4 - 1);
			} while (!v7);
			v9 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex] + 4 * mip * v5);
			v61 = *(DWORD*)&pixels[4 * imgIndex + 240] + 4 * (v5 + mip * v5) - (DWORD)v9;
			v53 = *(DWORD*)&pixels[4 * imgIndex + 180] + 4 * v5 - (DWORD)v9;
			v70 = 4;
			do
			{
				v10 = (double)(v9[v53] + v9[v61] + *v9) / 3.0;
				++v9;
				v7 = v70-- == 1;
				v11 = (int)v10;
				*(v9 - 1) = v11;
				v9[v61 - 1] = v11;
				v9[v53 - 1] = *(v9 - 1);
			} while (!v7);
			v12 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex] + 4 * v5);
			v71 = *(DWORD*)&pixels[4 * imgIndex + 300] - (DWORD)v12;
			v13 = *(DWORD*)&pixels[4 * imgIndex + 120] + 4 * v5 - (DWORD)v12;
			v54 = v13;
			v62 = 4;
			do
			{
				v14 = (double)(v12[v71] + *v12 + v12[v13]) / 3.0;
				++v12;
				v7 = v62-- == 1;
				v15 = (int)v14;
				*(v12 - 1) = v15;
				v12[v71 - 1] = v15;
				v13 = v54;
				v12[v54 - 1] = *(v12 - 1);
			} while (!v7);
			v16 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex] + 4 * (v5 + mip * v5));
			v72 = *(DWORD*)&pixels[4 * imgIndex + 300] + 4 * mip * v5 - (DWORD)v16;
			v63 = *(DWORD*)&pixels[4 * imgIndex + 180] + 4 * (v5 + mip * v5) - (DWORD)v16;
			v55 = 4;
			do
			{
				v80 = v16[v63] + v16[v72] + *v16;
				++v16;
				v7 = v55-- == 1;
				v17 = (int)((double)v80 / 3.0);
				*(v16 - 1) = v17;
				v16[v72 - 1] = v17;
				v16[v63 - 1] = *(v16 - 1);
			} while (!v7);
			v18 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex + 60] + 4 * v5);
			v73 = *(DWORD*)&pixels[4 * imgIndex + 240] - (DWORD)v18;
			v19 = *(DWORD*)&pixels[4 * imgIndex + 120] + 4 * mip * v5 - (DWORD)v18;
			v56 = v19;
			v64 = 4;
			do
			{
				v81 = v18[v73] + *v18 + v18[v19];
				++v18;
				v7 = v64-- == 1;
				v20 = (int)((double)v81 / 3.0);
				*(v18 - 1) = v20;
				v18[v73 - 1] = v20;
				v19 = v56;
				v18[v56 - 1] = *(v18 - 1);
			} while (!v7);
			v21 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex + 60] + 4 * (v5 + mip * v5));
			v22 = *(DWORD*)&pixels[4 * imgIndex + 240] + 4 * mip * v5 - (DWORD)v21;
			v74 = v22;
			v65 = *(DWORD*)&pixels[4 * imgIndex + 180] - (DWORD)v21;
			v57 = 4;
			while (1)
			{
				v82 = v21[v65] + v21[v22] + *v21;
				++v21;
				v7 = v57-- == 1;
				v23 = (int)((double)v82 / 3.0);
				*(v21 - 1) = v23;
				v21[v74 - 1] = v23;
				v21[v65 - 1] = *(v21 - 1);
				if (v7)
				{
					break;
				}
				v22 = v74;
			}
			v24 = *(unsigned __int8**)&pixels[4 * imgIndex + 60];
			v75 = *(DWORD*)&pixels[4 * imgIndex + 300] + 4 * v5 - (DWORD)v24;
			v25 = *(DWORD*)&pixels[4 * imgIndex + 120] - (DWORD)v24;
			v58 = v25;
			v66 = 4;
			do
			{
				v83 = *v24 + v24[v25] + v24[v75];
				++v24;
				v7 = v66-- == 1;
				v26 = (int)((double)v83 / 3.0);
				*(v24 - 1) = v26;
				v24[v75 - 1] = v26;
				v25 = v58;
				v24[v58 - 1] = *(v24 - 1);
			} while (!v7);
			v27 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex + 60] + 4 * mip * v5);
			v76 = *(DWORD*)&pixels[4 * imgIndex + 300] + 4 * (v5 + mip * v5) - (DWORD)v27;
			v28 = *(DWORD*)&pixels[4 * imgIndex + 180] + 4 * mip * v5 - (DWORD)v27;
			v59 = v28;
			v67 = 4;
			do
			{
				v84 = *v27 + v27[v28] + v27[v76];
				++v27;
				v7 = v67-- == 1;
				v29 = (int)((double)v84 / 3.0);
				*(v27 - 1) = v29;
				v27[v76 - 1] = v29;
				v28 = v59;
				v27[v59 - 1] = *(v27 - 1);
			} while (!v7);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex], *(DWORD*)&pixels[4 * imgIndex + 240], mip, 0, 1, 0);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex], *(DWORD*)&pixels[4 * imgIndex + 300], mip, 1, 0, 0);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex], *(DWORD*)&pixels[4 * imgIndex + 120], mip, 2, 1, 1);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex], *(DWORD*)&pixels[4 * imgIndex + 180], mip, 3, 1, 0);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex + 60], *(DWORD*)&pixels[4 * imgIndex + 120], mip, 2, 0, 0);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex + 60], *(DWORD*)&pixels[4 * imgIndex + 180], mip, 3, 0, 1);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex + 60], *(DWORD*)&pixels[4 * imgIndex + 240], mip, 1, 0, 0);
			Image_BlendCubeCorner(*(DWORD*)&pixels[4 * imgIndex + 60], *(DWORD*)&pixels[4 * imgIndex + 300], mip, 0, 1, 0);
			v85 = *(DWORD*)&pixels[4 * imgIndex + 240];
			if (mip > 0)
			{
				v30 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex + 180] + 2);
				v31 = (unsigned __int8*)(*(DWORD*)&pixels[4 * imgIndex + 240] + 4 * mip * v5 + 2);
				v77 = mip;
				do
				{
					v32 = *(v30 - 2) + *(v31 - 2);
					*(v30 - 2) = v32 / 2;
					*(v31 - 2) = v32 / 2;
					v33 = *(v30 - 1) + *(v31 - 1);
					*(v30 - 1) = v33 / 2;
					*(v31 - 1) = v33 / 2;
					v34 = *v30 + *v31;
					*v30 = v34 / 2;
					*v31 = v34 / 2;
					v35 = (v30[1] + v31[1]) / 2;
					v30[1] = v35;
					v31[1] = v35;
					v30 += 4;
					v31 += 4;
					--v77;
				} while (v77);
				v3 = pixels;
			}
			v68 = *(DWORD*)&v3[4 * imgIndex + 300];
			if (mip > 0)
			{
				v36 = (unsigned __int8*)(*(DWORD*)&v3[4 * imgIndex + 180] + 4 * mip * v5 + 2);
				v37 = (unsigned __int8*)(*(DWORD*)&v3[4 * imgIndex + 300] + 4 * mip * mip - 2);
				v78 = mip;
				do
				{
					v38 = *(v37 - 2) + *(v36 - 2);
					*(v36 - 2) = v38 / 2;
					*(v37 - 2) = v38 / 2;
					v39 = *(v36 - 1) + *(v37 - 1);
					*(v36 - 1) = v39 / 2;
					*(v37 - 1) = v39 / 2;
					v40 = *v36 + *v37;
					*v36 = v40 / 2;
					*v37 = v40 / 2;
					v41 = (v36[1] + v37[1]) / 2;
					v36[1] = v41;
					v37[1] = v41;
					v36 += 4;
					v37 -= 4;
					--v78;
				} while (v78);
				v3 = pixels;
			}
			v42 = *(DWORD*)&v3[4 * imgIndex + 120];
			if (mip > 0)
			{
				v43 = (unsigned __int8*)(v85 + 2);
				v44 = (unsigned __int8*)(v42 + 4 * mip * v5 + 2);
				v45 = mip;
				do
				{
					v46 = *(v43 - 2) + *(v44 - 2);
					*(v44 - 2) = v46 / 2;
					*(v43 - 2) = v46 / 2;
					v47 = *(v44 - 1) + *(v43 - 1);
					*(v44 - 1) = v47 / 2;
					v48 = *v43;
					*(v43 - 1) = v47 / 2;
					v49 = v48 + *v44;
					*v44 = v49 / 2;
					v50 = v43[1];
					*v43 = v49 / 2;
					v51 = (v50 + v44[1]) / 2;
					v44[1] = v51;
					v43[1] = v51;
					v44 += 4;
					v43 += 4;
					--v45;
				} while (v45);
			}
			Image_BlendCubeCorner(v42, v68, mip, 2, 2, 1);
		}
	}


	void R_CreateReflectionRawDataFromCubemapShot(game::DiskGfxReflectionProbe* probeRawData)
	{
		for (auto f = 0; f < 6; f++)
		{
			R_CubemapShotRotateClockwise(get_cubemapshot_res(), reflectionprobes::cubemapshot_image_ptr[f]);
			Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[f]);
		}

		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[0]);
		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[1]);

		R_CubemapShotRotateAntiClockwise(reflectionprobes::cubemapshot_image_ptr[1]);

		Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[1]);
		Image_FlipVertically(reflectionprobes::cubemapshot_image_ptr[2]);

		R_CubemapShotRotateAntiClockwise(reflectionprobes::cubemapshot_image_ptr[3]);

		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[4]);
		R_CubemapShotFlipVertical(reflectionprobes::cubemapshot_image_ptr[5]);

		std::uint8_t* pixels[6][15];
		const int down_sample_res = get_cubemapshot_res() / 4; // 64 for 256 size

		int mipmap_level_size = down_sample_res * 4 * down_sample_res;
		auto raw_pixels = reinterpret_cast<std::uint8_t*>(probeRawData->pixels);

		for (auto imgIndex = 0; imgIndex < 6; ++imgIndex)
		{
			pixels[imgIndex][0] = raw_pixels;
			raw_pixels += mipmap_level_size;
			memcpy(pixels[imgIndex][0], reflectionprobes::cubemapshot_image_ptr[imgIndex], mipmap_level_size);
		}

#if 0
		// mip generation .. needed?

		int mipLevel = 0;
		CubeMap_CacheHighMipDirs();

		for (auto f = 0; f < 6; ++f)
		{
			mipLevel = 1;

			int scaledSize = down_sample_res;
			std::uint8_t* data = reflectionprobes::cubemapshot_image_ptr[f];

			do
			{
				int size = 1;
				CubeMap_GenerateMipMap32(data, scaledSize, (std::uint8_t*)pixels, f);

				if (scaledSize >> 1 >= 1)
				{
					size = scaledSize >> 1;
				}

				scaledSize = size;
				mipmap_level_size = size * 4 * size;
				pixels[f][mipLevel] = raw_pixels;
				raw_pixels += mipmap_level_size;

				memcpy(pixels[f][mipLevel++], data, mipmap_level_size);

			} while (scaledSize != 1);
		}

		for (auto mip = 0; mip < mipLevel; ++mip)
		{
			CubeMap_BlendFaceEdges(down_sample_res >> mip, (unsigned __int8*) pixels, mip);
		}
#endif

		const auto prefs = GET_GUI(ggui::preferences_dialog);
		if(prefs->dev_bool_01)
		{
			// would normally downsample all mips
			CubeMap_BlendFaceEdges(down_sample_res, (unsigned __int8*)pixels, 0);
		}
	}


	void R_EndCubemapShot(game::CubemapShot shotIndex)
	{
		const int img_index = shotIndex - 1;
		const int size_in_bytes = 4 * get_cubemapshot_res() * get_cubemapshot_res();

		reflectionprobes::cubemapshot_image_ptr[img_index] = static_cast<std::uint8_t*>(game::Z_VirtualAlloc(size_in_bytes));
		R_TakeCubeMapShot(get_pixel_border(), get_pixel_border(), get_cubemapshot_res(), get_cubemapshot_res(), 4, reflectionprobes::cubemapshot_image_ptr[img_index]);

		// take screenshot :: TODO pref
		export_cubeshot_side_as_targa(
			"cubemap_shot_" + std::to_string(shotIndex), 
			reflectionprobes::cubemapshot_image_ptr[img_index]);

		// flips images upside down (vertical mirror)
		R_CopyCubemapShot(reflectionprobes::cubemapshot_image_ptr[img_index]);
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
		utils::hook::call<void(__cdecl)(HWND)>(0x501A70)(cmainframe::activewnd->m_pCamWnd->GetWindow()); // R_SetupRendertarget


		// #
		// take shots

		for (int i = game::CUBEMAPSHOT_RIGHT; i < game::CUBEMAPSHOT_COUNT; ++i)
		{
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
		}

		R_CreateReflectionRawDataFromCubemapShot(probe);


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
		utils::hook::call<void(__cdecl)(HWND)>(0x501A70)(cmainframe::activewnd->m_pCamWnd->GetWindow()); // R_SetupRendertarget
	}

	void R_GenerateReflectionRawDataAll(game::DiskGfxReflectionProbe* probe_raw_data, unsigned int probe_count, bool* generate_probe)
	{
		for (auto index = 0u; index < probe_count; ++index)
		{
			if (generate_probe[index])
			{
				R_GenerateReflectionRawData(&probe_raw_data[index]);
			}
		}
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

			R_GenerateReflectionRawDataAll(probe_raw_generated_data, lump_probe_count, generate_probe);

			d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, probe_raw_generated_data, lump_size);
			game::R_GenerateReflectionImages(game::rgp->world->reflectionProbes, probe_raw_generated_data, lump_probe_count, 0);
			game::Z_Free(probe_raw_generated_data);
		}
		else
		{
			d3dbsp::Com_SaveLump(d3dbsp::LUMP_REFLECTION_PROBES, nullptr, 0);
		}
	}

	// call after R_IssueRenderCommands
	void reflectionprobes::generate_reflections_for_bsp()
	{
		if(game::rgp->world)
		{
			reflectionprobes::generate_reflections(game::rgp->world->reflectionProbes + 1, game::rgp->world->reflectionProbeCount - 1);
		}
	}

	void check_for_reflection_generation()
	{
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
		// hk 'R_SortMaterials' call after 'R_IssueRenderCommands' in 'CCamWnd::OnPaint'
		utils::hook(0x40306B, check_for_reflection_generation_stub, HOOK_JUMP).install()->quick();
	}

	reflectionprobes::~reflectionprobes()
	{ }
}
