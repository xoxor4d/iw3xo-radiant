#include "std_include.hpp"

namespace components
{
	//renderer::postfx_state_vars postfx_state = {};

	//bool g_any_postfx_enabled;
	//bool g_disable_postfx;

	//postfx_state_vars postfx_state = {};


	// *
	// enable/disable drawing of boxes around the origin on entities

	bool should_render_origin()
	{
		if (dvars::r_draw_model_origin && !dvars::r_draw_model_origin->current.enabled)
		{
			return false;
		}

		return true;
	}

	void __declspec(naked) render_origin_boxes_stub()
	{
		const static uint32_t continue_pt = 0x478E38;
		__asm
		{
			pushad;
			call    should_render_origin;
			test    al, al;
			popad;

			je      SKIP;
			mov     eax, 0x10B0;
			jmp		continue_pt;

		SKIP:
			pop     ebp;
			retn;
		}
	}

	
	// *
	// enable/disable drawing of backface wireframe on patches
	
	bool should_render_patch_backface_wireframe()
	{
		if (dvars::r_draw_patch_backface_wireframe && !dvars::r_draw_patch_backface_wireframe->current.enabled)
		{
			return false;
		}

		return true;
	}

	void __declspec(naked) patch_backface_wireframe_stub()
	{
		const static uint32_t func_addr = 0x440670;
		const static uint32_t retn_pt = 0x441577;
		__asm
		{
			pushad;
			call    should_render_patch_backface_wireframe;
			test    al, al;
			popad;

			je      SKIP;
			push	29;
			mov     eax, esi;
			mov     ecx, 1;
			mov     edx, edi;

			call	func_addr;
			jmp		retn_pt;

		SKIP:
			// we need atleast 1 push because of the add, esp 4 after we jump back
			push	1; // random push
			jmp		retn_pt;
		}
	}

	// *
	// *
	
	void renderer::R_ConvertColorToBytes(float* from, game::GfxColor* gfx_col)
	{
		if (from)
		{
			return game::Byte4PackPixelColor(from, gfx_col);
		}

		gfx_col->packed = 0; //-1;
	}
	
	void renderer::R_AddCmdDrawTextAtPosition(const char* text, game::Font_s* font, float* origin, float* pixel_step_x, float* pixel_step_y, float* color)
	{
		if (text && *text)
		{
			const size_t t_size = strlen(text);
			auto cmd = reinterpret_cast<game::GfxCmdDrawText3D*>(game::R_GetCommandBuffer((t_size + 0x34) & 0xFFFFFFFC, 16));
			if (cmd)
			{
				cmd->org[0] = origin[0];
				cmd->org[1] = origin[1];
				cmd->org[2] = origin[2];
				cmd->font = font;
				cmd->xPixelStep[0] = pixel_step_x[0];
				cmd->xPixelStep[1] = pixel_step_x[1];
				cmd->xPixelStep[2] = pixel_step_x[2];
				cmd->yPixelStep[0] = pixel_step_y[0];
				cmd->yPixelStep[1] = pixel_step_y[1];
				cmd->yPixelStep[2] = pixel_step_y[2];

				R_ConvertColorToBytes(color, &cmd->color);
				memcpy(cmd->text, text, t_size);
				cmd->text[t_size] = 0;
			}
		}
	}

	void renderer::copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest, bool no_release)
	{
		// get the backbuffer surface
		IDirect3DSurface9* surf_backbuffer = nullptr;
		game::dx->windows[wnd].swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surf_backbuffer);

		// write surface to file (test)
		//D3DXSaveSurfaceToFileA("surface_to_file.png", D3DXIFF_PNG, surf_backbuffer, NULL, NULL);

		// check if window size was changed -> release and recreate the texture surface
		if (dest)
		{
			D3DSURFACE_DESC desc;
			dest->GetLevelDesc(0, &desc);

			if(!no_release)
			{
				if (desc.Width != static_cast<unsigned int>(game::dx->windows[wnd].width) || desc.Height != static_cast<unsigned int>(game::dx->windows[wnd].height))
				{
					dest->Release();
					dest = nullptr;
				}
			}
		}

		// create or re-create ^ the texture surface
		if (!dest)
		{
			D3DXCreateTexture(game::dx->device, game::dx->windows[wnd].width, game::dx->windows[wnd].height, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_R8G8B8, D3DPOOL_DEFAULT, &dest);
		}

		// "bind" texture to surface
		IDirect3DSurface9* surf_texture;
		dest->GetSurfaceLevel(0, &surf_texture);

		// "copy" backbuffer to our texture surface
		game::dx->device->StretchRect(surf_backbuffer, NULL, surf_texture, NULL, D3DTEXF_NONE);

		// release the backbuffer surface or we'll leak memory
		surf_backbuffer->Release();

		// write texture to file (test)
		//D3DXSaveTextureToFileA("texture_to_file.png", D3DXIFF_PNG, dest, NULL);
	}

	/*
	 *  vertexshader + pixelshader loading (allows to load shaders that are not included in the shader_names file)
	 */

	DWORD FindShaderHashInShaderNamesFile(int is_pixelshader /*eax*/, const char* shader_name)
	{
		const static uint32_t func_addr = 0x518170;
		__asm
		{
			push	shader_name;
			mov		eax, is_pixelshader;
			call	func_addr;
			add		esp, 4;
		}
	}

	bool R_CreateShaderBuffer(ID3DXBuffer** shader /*edx*/, int hash /*ecx*/, const char* prefix_string)
	{
		const static uint32_t func_addr = 0x517F70;
		__asm
		{
			push	prefix_string;
			mov		ecx, hash;
			mov		edx, shader; // maybe lea
			call	func_addr;
			add		esp, 4;
		}
	}

	void Material_GetShaderTargetString(int always_one, signed int name_len_no_ext, size_t always_16, const char* ps, char* buff_out)
	{
		if (!always_one)
		{
			sprintf_s(buff_out, always_16, "%s_%i_%i", ps, name_len_no_ext / 10, name_len_no_ext % 10);
			return;
		}
		if (always_one != 1)
		{
			game::Com_Error("Material_GetShaderTargetString :: (renderer == GFX_RENDERER_SHADER_3)");
		}

		sprintf_s(buff_out, always_16, "%s_3_0", ps);
	}

	bool r_load_shader_programm(unsigned int* shader_len, unsigned int* cached_shader_out, const char* filename)
	{
		*shader_len = 0;
		*cached_shader_out = 0;

		// open the shader file
		auto shader_binary = fopen(filename, "rb");
		if (!shader_binary)
		{
			return false;
		}

		// first 4 bytes (int) = programm size in bytes
		if (fread(shader_len, 4u, 1u, shader_binary) != 1)
		{
			fclose(shader_binary);
			return false;
		}

		// Z_Malloc :: alloc memory (programm size in bytes)
		auto chached_shader = utils::hook::call<unsigned int* (__cdecl)(size_t)>(0x4AC330)(*shader_len);
		if (chached_shader)
		{
			// assign buffer addr to our ptr
			*cached_shader_out = reinterpret_cast<unsigned int>(chached_shader);

			// read and write programm to buffer
			fread(chached_shader, 1u, *shader_len, shader_binary);
			fclose(shader_binary);

			return true;
		}

		game::Com_Error("r_load_shader_programm :: failed to alloc %d amount of bytes", *shader_len);
		return false;
	}

	bool Material_CopyTextToDXBuffer(ID3DXBuffer** shader, DWORD shader_hash, const char* prefix_str)
	{
		char shader_path[260];
		unsigned int shader_len = 0;
		unsigned int chachedShader = 0;

		sprintf_s(shader_path, 260u, "%s/raw/shader_bin/%s_%8.8x", game::Dvar_FindVar("fs_basepath")->current.string, prefix_str, shader_hash);
		if (!r_load_shader_programm(&shader_len, &chachedShader, shader_path))
		{
			// look for addon shaders in bin/IW3xRadiant/shader_bin
			memset(&shader_path, 0, sizeof(shader_path));
			sprintf_s(shader_path, 260u, "%s/bin/IW3xRadiant/shader_bin/%s_%8.8x", game::Dvar_FindVar("fs_basepath")->current.string, prefix_str, shader_hash);

			if (!r_load_shader_programm(&shader_len, &chachedShader, shader_path))
			{
				return false;
			}
		}

		// create shader buffer
		if (D3DXCreateBuffer(shader_len, shader) < 0)
		{
			game::Com_Error("ERROR: Material_CopyTextToDXBuffer: D3DXCreateBuffer(%d) failed\n", shader_len);
		}

		// copy shader programm to shader buffer
		memcpy((*shader)->GetBufferPointer(), (void*)chachedShader, shader_len);

		return true;
	}


	// *
	//  hashing

	std::uint32_t swap_byte_order32(std::uint32_t uint)
	{
		return	((uint & 0x000000ff) << 24) |
				((uint & 0x0000ff00) << 8)  |
				((uint & 0x00ff0000) >> 8)  |
				((uint & 0xff000000) >> 24);
	}

	// hashing func shader_tool is using (this is not whats used for original shaders)
	std::uint32_t generate_hash_value(char const* str)
	{
		std::uint32_t esi, eax, ebx, ecx;

		ebx = ebx ^ ebx;
		for (esi = 0; esi < strlen(str); ++esi)
		{
			ecx = str[esi];
			eax = ebx;
			eax = eax << 5;
			ebx = ebx + eax;
			ebx = ebx ^ ecx;
		}

		return ebx;
	}

	// same as the above
	std::uint32_t generate_hash_value_original(const char* str)
	{
		std::uint32_t v1 = 0, v3 = 0;

		for (auto i = str; *i; v1 = v3 ^ (33 * v1))
		{
			if (*i >= 65 && *i <= 90)
				__debugbreak(); // (*pos < 'A' || *pos > 'Z')

			if (*i == 92)
				__debugbreak(); // "(*pos != '\\\\' || *pos == '/')

			v3 = *i++;
		}

		return v1;
	}

	// *
	// pixelshader

	game::MaterialPixelShader* r_create_pixelshader(const char* shader_name, signed int name_len, int always_one)
	{
		ID3DXBuffer* shader = nullptr;
		char prefix_buffer[16];

		Material_GetShaderTargetString(always_one, name_len, 16u, "ps", prefix_buffer);
		auto shader_hash = FindShaderHashInShaderNamesFile(1, shader_name);

		if(!shader_hash)
		{
			// calculate shader hash if not found in the shader_names file .. always calculate?
			shader_hash = 0xFFFFFFFF - swap_byte_order32(generate_hash_value(shader_name));
		}
		
		if(!Material_CopyTextToDXBuffer(&shader, shader_hash, prefix_buffer)) // R_CreateShaderBuffer (original func)
		{
			game::Com_Error("r_create_pixelshader :: Can't find shader: shader_bin/%s_%8.8x (%s)\n", prefix_buffer, shader_hash, shader_name);
		}

		if(!shader)
		{
			return nullptr;
		}

		const UINT programSize = shader->GetBufferSize();
		if (!programSize)
		{
			game::Com_Error("r_create_pixelshader :: programSize = 0");
		}

		// alloc MaterialPixelShader + programm bytes + shader name char array
		// Memory layout:
		// * variable mtlShader (0x10)
		// * shader programm bytes
		// * shader name bytes

		const std::uint32_t shader_name_len = strlen(shader_name) + 1;
		const std::uint32_t total_size = programSize + shader_name_len + sizeof(game::MaterialPixelShader);
		auto mtlShader = reinterpret_cast<game::MaterialPixelShader*>(game::Hunk_Alloc(total_size)); 

		// assign name ptr and copy string
		char* addr_shader_name = (char*)&mtlShader[1] + programSize;
		mtlShader->name = addr_shader_name;
		memcpy(addr_shader_name, shader_name, shader_name_len);

		// assign ptr and copy shader prog
		char* addr_shader_prog = (char*)&mtlShader[1];
		DWORD* shader_prog_ptr = (DWORD*)shader->GetBufferPointer();
		memcpy(addr_shader_prog, shader_prog_ptr, programSize);

		// create pixelshader
		if (game::dx->device->CreatePixelShader((DWORD*)addr_shader_prog, (IDirect3DPixelShader9**)&mtlShader->prog.ps) >= 0)
		{
			mtlShader->prog.loadDef.programSize		= static_cast<unsigned short>(programSize >> 2);
			mtlShader->prog.loadDef.loadForRenderer = static_cast<unsigned short>(always_one);

			if (mtlShader->prog.loadDef.programSize * sizeof(mtlShader->prog.loadDef.program[0]) != programSize)
			{
				game::Com_Error("r_create_pixelshader :: mtlShader->prog.loadDef.programSize * sizeof( mtlShader->prog.loadDef.program[0] ) == programSize");
			}

			mtlShader->prog.loadDef.program = (unsigned int*)&mtlShader[1];
			shader->Release();

			return mtlShader;
		}

		game::Com_Error("r_create_pixelshader :: pixel shader creation failed for %s\n", shader_name);
		return nullptr;
	}


	// *
	// vertexshader

	game::MaterialVertexShader* r_create_vertexshader(const char* shader_name, signed int name_len, int always_one)
	{
		ID3DXBuffer* shader = nullptr;
		char prefix_buffer[16];

		Material_GetShaderTargetString(always_one, name_len, 16u, "vs", prefix_buffer);
		auto shader_hash = FindShaderHashInShaderNamesFile(0, shader_name);

		if (!shader_hash)
		{
			// calculate shader hash if not found in the shader_names file
			shader_hash = 0xFFFFFFFF - swap_byte_order32(generate_hash_value(shader_name));
		}

		if (!Material_CopyTextToDXBuffer(&shader, shader_hash, prefix_buffer))
		{
			game::Com_Error("r_create_vertexshader :: Can't find shader: shader_bin/%s_%8.8x (%s)\n", prefix_buffer, shader_hash, shader_name);
		}

		if (!shader)
		{
			return nullptr;
		}

		const UINT programSize = shader->GetBufferSize();
		if (!programSize)
		{
			game::Com_Error("r_create_vertexshader :: programSize = 0");
		}

		// alloc MaterialVertexShader + programm bytes + shader name char array
		// Memory layout:
		// * variable mtlShader (0x10)
		// * shader programm bytes
		// * shader name bytes

		const std::uint32_t shader_name_len = strlen(shader_name) + 1;
		const std::uint32_t total_size = programSize + shader_name_len + sizeof(game::MaterialVertexShader);
		auto mtlShader = reinterpret_cast<game::MaterialVertexShader*>(game::Hunk_Alloc(total_size));

		// assign name ptr and copy string
		char* addr_shader_name = (char*)&mtlShader[1] + programSize;
		mtlShader->name = addr_shader_name;
		memcpy(addr_shader_name, shader_name, shader_name_len);

		// assign ptr and copy shader prog
		char* addr_shader_prog = (char*)&mtlShader[1];
		DWORD* shader_prog_ptr = (DWORD*)shader->GetBufferPointer();
		memcpy(addr_shader_prog, shader_prog_ptr, programSize);

		// create vertexshader
		if (game::dx->device->CreateVertexShader((DWORD*)addr_shader_prog, (IDirect3DVertexShader9**)&mtlShader->prog.vs) >= 0)
		{
			mtlShader->prog.loadDef.programSize = static_cast<unsigned short>(programSize >> 2);
			mtlShader->prog.loadDef.loadForRenderer = static_cast<unsigned short>(always_one);

			if (mtlShader->prog.loadDef.programSize * sizeof(mtlShader->prog.loadDef.program[0]) != programSize)
			{
				game::Com_Error("r_create_vertexshader :: mtlShader->prog.loadDef.programSize * sizeof( mtlShader->prog.loadDef.program[0] ) == programSize");
			}

			mtlShader->prog.loadDef.program = (unsigned int*)&mtlShader[1];
			shader->Release();

			return mtlShader;
		}

		game::Com_Error("r_create_vertexshader :: pixel shader creation failed for %s\n", shader_name);
		return nullptr;
	}

	void __declspec(naked) r_create_vertexshader_stub()
	{
		const static uint32_t retn_pt = 0x518750;
		__asm
		{
			movzx   ecx, byte ptr[ebp + 8];
			push	esi; // always_one
			push	ecx; // name_len
			push	edi; // shader_name
			call	r_create_vertexshader;
			mov     edi, eax;
			add		esp, 12;

			// jump back to test edi, edi
			jmp		retn_pt;
		}
	}




	struct front_backend_variable_helper
	{
		float float_time = 0.0f;
	};

	front_backend_variable_helper gfx_fbv = {};

	// *
	// pixelshader custom constants

	void set_custom_pixelshader_constants([[maybe_unused]] game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		// there is no proper scene setup for the backend frame ..
		// grab required stuff from the frontend frame

		if(source && source->viewMode == game::VIEW_MODE_3D)
		{
			gfx_fbv.float_time = source->sceneDef.floatTime;
		}

		if (source && state && state->pass)
		{
			// 2D: set required shader constants for backend passes
			if(renderer::is_rendering_camerawnd() && source->viewMode == game::VIEW_MODE_2D)
			{
				for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
				{
					const auto arg_def = &state->pass->args[arg];
					if (arg_def && arg_def->type == 5)
					{
						if (state->pass->pixelShader)
						{
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_GAMETIME)
							{
								const float clamped_second = gfx_fbv.float_time - floorf(gfx_fbv.float_time);

								game::vec4_t temp = 
								{
									sinf(clamped_second * 6.283185482025146f),
									cosf(clamped_second * 6.283185482025146f),
									clamped_second,
									gfx_fbv.float_time
								};

								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}

							// filmtweaks
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_BIAS ||
								arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_TINT_BASE || 
								arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_TINT_DELTA)
							{
								dvars::assign_stock_dvars();

								//float v12 = (1.0f - r_filmtweakdesaturation) * r_desaturation + r_filmtweakdesaturation;
								const float filmtweakdesaturation = dvars::r_filmtweakdesaturation->current.value; //v12 * r_filmtweakdesaturation;

								float desaturation = 0.00024414062f;

								if (0.000244140625f - filmtweakdesaturation < 0.0f)
								{
									desaturation = filmtweakdesaturation;
								}

								const float dc = desaturation * dvars::r_filmtweakcontrast->current.value;

								if(arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_BIAS)
								{
									const float bc = dvars::r_filmtweakbrightness->current.value + 0.5f - 0.5f * dvars::r_filmtweakcontrast->current.value;

									const game::vec4_t temp =
									{
										bc, bc, bc, 1.0f / desaturation - 1.0f
									};

									game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
								}

								if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_TINT_BASE)
								{
									const game::vec4_t temp =
									{
										dvars::r_filmtweakdarktint->current.vector[0] * dc,
										dvars::r_filmtweakdarktint->current.vector[1] * dc,
										dvars::r_filmtweakdarktint->current.vector[2] * dc,
										0.0f
									};

									game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
								}

								if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_COLOR_TINT_DELTA)
								{
									const game::vec4_t temp =
									{
										(dvars::r_filmtweaklighttint->current.vector[0] - dvars::r_filmtweakdarktint->current.vector[0])* dc,
										(dvars::r_filmtweaklighttint->current.vector[1] - dvars::r_filmtweakdarktint->current.vector[1])* dc,
										(dvars::r_filmtweaklighttint->current.vector[2] - dvars::r_filmtweakdarktint->current.vector[2])* dc,
										0.0f
									};

									game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
								}
							}
						}
					}
				}
			}

			// 3D
			if ((renderer::is_rendering_layeredwnd() && layermatwnd::rendermethod_preview == layermatwnd::FAKESUN_DAY) ||
				(!renderer::is_rendering_layeredwnd() && dvars::r_fakesun_preview->current.enabled))
			{
				for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
				{
					const auto arg_def = &state->pass->args[arg];
					if (arg_def && arg_def->type == 5)
					{
						if (state->pass->pixelShader)
						{
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_ENVMAP_PARMS)
							{
								game::vec4_t temp;
								game::dx->device->GetPixelShaderConstantF(arg_def->dest, temp, 1);

								// set envmapparams from material
								if(state->material && state->material->constantCount && state->material->constantTable)
								{
									for(auto constant = 0; constant < state->material->constantCount; constant++)
									{
										if(state->material->constantTable[constant].name)
										{
											if (utils::string_equals(state->material->constantTable[constant].name, "envMapParms"))
											{
												game::dx->device->SetPixelShaderConstantF(arg_def->dest, state->material->constantTable[constant].literal, 1);
												break;
											}
										}
										
									}
								}
							}

							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FOG_COLOR)
							{
								game::vec4_t temp = { dvars::r_fakesun_fog_color->current.vector[0], dvars::r_fakesun_fog_color->current.vector[1], dvars::r_fakesun_fog_color->current.vector[2], dvars::r_fakesun_fog_color->current.vector[3] };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}
							
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_POSITION)
							{
								bool worldspawn_valid = false;
								game::vec3_t sun_dir = {};
								
								if(dvars::r_fakesun_use_worldspawn->current.enabled)
								{
									const auto world_ent = game::g_world_entity();
									if(world_ent && world_ent->firstActive && ggui::entity::Entity_GetVec3ForKey(world_ent->firstActive, sun_dir, "sundirection"))
									{
										worldspawn_valid = true;
									}
								}

								game::vec4_t temp = { 0.0f, 0.0f, 0.0f, 0.0f };
								
								// AngleVectors(float* angles, float* vpn, float* right, float* up)
								utils::hook::call<void(__cdecl)(float* angles, float* vpn, float* right, float* up)>(0x4ABD70)(worldspawn_valid ? sun_dir : ggui::fakesun_settings::sun_dir, temp, nullptr, nullptr);
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}
							
							else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_DIFFUSE)
							{
								bool worldspawn_valid = false;
								game::vec4_t sun_diffuse = {};

								if (dvars::r_fakesun_use_worldspawn->current.enabled)
								{
									const auto world_ent = game::g_world_entity();

									float sunlight = 0.0f;
									game::vec3_t suncolor = {};

									if(world_ent && world_ent->firstActive && ggui::entity::Entity_GetVec3ForKey(world_ent->firstActive, suncolor, "suncolor"))
									{
										if (!ggui::entity::Entity_GetValueForKey(world_ent->firstActive, &sunlight, "sunlight"))
										{
											// default value
											sunlight = 1.35f;
										}

										sunlight *= 1.5f;
										utils::vector::scale(suncolor, sunlight, sun_diffuse);

										worldspawn_valid = true;
									}
								}
								
								game::vec4_t temp = { ggui::fakesun_settings::sun_diffuse[0], ggui::fakesun_settings::sun_diffuse[1], ggui::fakesun_settings::sun_diffuse[2], 1.0f };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, worldspawn_valid ? sun_diffuse : temp, 1);
							}
							
							else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_SPECULAR)
							{
								bool worldspawn_valid = false;
								game::vec4_t sun_specular = {};

								if (dvars::r_fakesun_use_worldspawn->current.enabled)
								{
									const auto world_ent = game::g_world_entity();
									if (world_ent && world_ent->firstActive && ggui::entity::Entity_GetVec3ForKey(world_ent->firstActive, sun_specular, "suncolor"))
									{
										// worldspawn suncolor
										utils::vector::ma(game::vec3_t(1.0f, 1.0f, 1.0f), 2.0f, sun_specular, sun_specular);
										worldspawn_valid = true;
									}
								}
								
								game::vec4_t temp = { ggui::fakesun_settings::sun_specular[0], ggui::fakesun_settings::sun_specular[1], ggui::fakesun_settings::sun_specular[2], ggui::fakesun_settings::sun_specular[3] };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, worldspawn_valid ? sun_specular : temp, 1);
							}
							
							else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_LIGHT_SPOTDIR)
							{
								game::vec4_t temp = { ggui::fakesun_settings::material_specular[0], ggui::fakesun_settings::material_specular[1], ggui::fakesun_settings::material_specular[2], ggui::fakesun_settings::material_specular[3] };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}
							
							else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_LIGHT_SPOTFACTORS)
							{
								//bool worldspawn_valid = false;
								//game::vec4_t ambient = {};

								//if (dvars::r_fakesun_use_worldspawn->current.enabled)
								//{
								//	const auto world_ent = reinterpret_cast<game::entity_s_def*>(game::g_world_entity()->firstActive);

								//	// not proper at all
								//	
								//	float ambient_scale = 0.0f;
								//	if (!ggui::entity::Entity_GetValueForKey(reinterpret_cast<game::entity_s*>(world_ent), &ambient_scale, "ambient"))
								//	{
								//		// default value
								//		ambient_scale = 0.2f;
								//	}
								//	
								//	utils::vector::set_vec4(ambient, ambient_scale);
	
								//	worldspawn_valid = true;
								//}
								
								game::vec4_t temp = { ggui::fakesun_settings::ambient[0], ggui::fakesun_settings::ambient[1], ggui::fakesun_settings::ambient[2], ggui::fakesun_settings::ambient[3] };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}
						}
					}
				}
			}
		}
	}

	void __declspec(naked) R_SetPassPixelShaderStableArguments_stub()
	{
		__asm
		{
			pop     edi;
			pop     esi;
			mov     esp, ebp;
			pop     ebp;

			pushad;
			mov		ecx, [esp + 0x24]; // source
			mov		edx, [esp + 0x28]; // state
			push	edx; // state
			push	ecx; // source
			call	set_custom_pixelshader_constants;
			add		esp, 8;
			popad;

			retn;
		}
	}


	// *
	// vertexshader custom constants

	void set_custom_vertexshader_constants([[maybe_unused]] game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		if (renderer::is_rendering_camerawnd() && dvars::r_fakesun_fog_enabled->current.enabled)
		{
			if (state && state->pass)
			{
				for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
				{
					const auto arg_def = &state->pass->args[arg];
					if (arg_def && arg_def->type == 3)
					{
						if (state->pass->vertexShader)
						{
							// set fog
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FOG)
							{
								const float density = 0.69314f / dvars::r_fakesun_fog_half->current.value;
								const float start = dvars::r_fakesun_fog_start->current.value * density;
								
								game::vec4_t temp = { 0.0f, 1.0f, -density, start };
								game::dx->device->SetVertexShaderConstantF(arg_def->dest, temp, 1);
							}

							// needed for vertcol_mul_fog (2 pass technique for some decals)
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FOG_COLOR)
							{
								game::vec4_t temp = { dvars::r_fakesun_fog_color->current.vector[0], dvars::r_fakesun_fog_color->current.vector[1], dvars::r_fakesun_fog_color->current.vector[2], dvars::r_fakesun_fog_color->current.vector[3] };
								game::dx->device->SetVertexShaderConstantF(arg_def->dest, temp, 1);
							}
						}
					}
				}
			}
		}
	}

	void __declspec(naked) R_SetPassShaderObjectArguments_stub()
	{
		__asm
		{
			pop     edi;
			pop     esi;
			pop     ebx;
			mov     esp, ebp;
			pop     ebp;

			pushad;
			mov		ecx, [esp + 0x24]; // source
			mov		edx, [esp + 0x28]; // state
			push	edx; // state
			push	ecx; // source
			call	set_custom_vertexshader_constants;
			add		esp, 8;
			popad;

			retn;
		}
	}


	// --------------

	void R_SetPassShaderStableArguments(unsigned __int16* arg_type /*eax*/, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state, int stableArgCount)
	{
		const static uint32_t func_addr = 0x53BF60;
		__asm
		{
			pushad;
			push	stableArgCount;
			push	state;
			push	src;

			mov		eax, arg_type;
			
			call	func_addr;
			add		esp, 12;
			
			popad;
		}
	}

	// return handle for loaded technique, loads new technique from "raw/techniques" or "bin/IW3xRadiant/techniques" otherwise
	game::MaterialTechnique* Material_RegisterTechnique(const char* name /*eax*/, int is_renderer_in_use /*edi*/)
	{
		const static uint32_t func_addr = 0x519790;
		__asm
		{
			mov		eax, name;
			mov		edi, is_renderer_in_use;
			call	func_addr;
		}
	}

	void r_setup_pass_general(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, int passIndex)
	{
		const auto pass = &state->technique->passArray[passIndex];
		const auto material = state->material;

		state->pass = pass;
		state->passIndex = passIndex;

		if (material->stateBitsEntry[state->techType] >= material->stateBitsCount)
		{
			game::Com_Error("r_setup_pass :: material->stateBitsEntry[context.state->techType] doesn't index material->stateBitsCount");
		}

		const int bit = passIndex + material->stateBitsEntry[state->techType];
		unsigned int loadbit_0 = material->stateBitsTable[bit].loadBits[0];
		unsigned int loadbit_1 = material->stateBitsTable[bit].loadBits[1];

		if (source->viewMode == game::VIEW_MODE_NONE)
		{
			game::Com_Error("r_setup_pass :: context.source->viewMode != VIEW_MODE_NONE");
		}

		if (source->viewMode == game::VIEW_MODE_2D)
		{
			loadbit_1 = loadbit_1 & 0xFFFFFFC2 | 2;
		}

		if (loadbit_0 != state->refStateBits[0])
		{
			// void __cdecl R_ChangeState_0(GfxCmdBufState *state, unsigned int statebit)
			utils::hook::call<void(__cdecl)(game::GfxCmdBufState* _state, unsigned int _statebit)>(0x538AF0)(state, loadbit_0);
			state->refStateBits[0] = loadbit_0;
		}

		if (loadbit_1 != state->refStateBits[1])
		{
			// void __cdecl R_ChangeState_1(GfxCmdBufState *state, unsigned int statebit)
			utils::hook::call<void(__cdecl)(game::GfxCmdBufState* _state, unsigned int _statebit)>(0x53A170)(state, loadbit_1);
			state->refStateBits[1] = loadbit_1;
		}

		if (!pass->pixelShader)
		{
			game::Com_Error("r_setup_pass :: pass->pixelShader");
		}

		// void __cdecl R_SetPixelShader(GfxCmdBufState *state, MaterialPixelShader *pixelShader)
		utils::hook::call<void(__cdecl)(game::GfxCmdBufState* _state, game::MaterialPixelShader* _pixelShader)>(0x53C2C0)(state, pass->pixelShader);

		if (pass->stableArgCount)
		{
			R_SetPassShaderStableArguments(&pass->args[pass->perPrimArgCount + pass->perObjArgCount].type, source, state, pass->stableArgCount);
		}
	}

	void r_setup_pass_xmodel(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, int passIndex)
	{
		if ((renderer::is_rendering_layeredwnd() && layermatwnd::rendermethod_preview == layermatwnd::FAKESUN_DAY) || 
			(!renderer::is_rendering_layeredwnd() && dvars::r_fakesun_preview->current.enabled))
		{
			if (utils::string_equals(state->technique->name, "fakelight_normal_dtex") ||
				utils::string_equals(state->technique->name, "fakelight_normal_d0_dtex"))
			{
				bool has_normal = false;
				bool has_spec = false;

				for (auto tex = 0; tex < state->material->textureCount; tex++)
				{
					if (state->material->textureTable[tex].u.image->semantic == 0x5 || state->material->textureTable[tex].u.image->semantic == 0x1) // or identitynormal
					{
						has_normal = true;
					}

					has_spec = state->material->textureTable[tex].u.image->semantic == 0x8 ? true : has_spec;
				}

				if (has_spec && has_normal)
				{
					if (const auto	tech = Material_RegisterTechnique("radiant_fakesun_dtex", 1); // fakesun_normal_dtex
									tech)
					{
						state->technique = tech;

						// set reflection probe sampler (index needs to be the same as the index defined in shader_vars.h)
						if (const auto	image = game::Image_RegisterHandle("_default_cubemap");
										image && image->texture.data)
						{
							game::R_SetSampler(0, state, 1, (char)114, image);
						}
					}
				}
				else if (!has_spec && has_normal)
				{
					if (const auto	tech = Material_RegisterTechnique("radiant_fakesun_no_spec_dtex", 1); // fakesun_normal_no_spec_img_dtex
									tech)
					{
						state->technique = tech;

						// set reflection probe sampler (index needs to be the same as the index defined in shader_vars.h)
						if (const auto	image = game::Image_RegisterHandle("_default_cubemap");
										image && image->texture.data)
						{
							// R_SetSampler(int a1, GfxCmdBufState *state, int sampler, char sampler_state, GfxImage *img)
							utils::hook::call<void(__cdecl)(int unused, game::GfxCmdBufState* _state, int _sampler, char _sampler_state, game::GfxImage* _img)>
								(0x538D70)(0, state, 1, 114, image);
						}
					}
				}
			}
		}

		if(dvars::r_draw_model_shadowcaster && !dvars::r_draw_model_shadowcaster->current.enabled)
		{
			// replace shadow caster technique with a none visible one
			if (utils::string_equals(state->material->info.name, "mc/mtl_tree_shadow_caster"))
			{
				if (const auto	tech = Material_RegisterTechnique("cinematic", 1);
								tech)
				{
					state->technique = tech;
				}
			}
		}
		
		r_setup_pass_general(source, state, passIndex);
	}

	void r_setup_pass_brush(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, int passIndex)
	{
		if ((renderer::is_rendering_layeredwnd() && layermatwnd::rendermethod_preview == layermatwnd::FAKESUN_DAY) ||
			(!renderer::is_rendering_layeredwnd() && dvars::r_fakesun_preview->current.enabled))
		{
			if (utils::string_equals(state->technique->name, "fakelight_normal") ||
				utils::string_equals(state->technique->name, "fakelight_normal_d0"))
			{
				bool has_normal = false;
				bool has_spec = false;

				for (auto tex = 0; tex < state->material->textureCount; tex++)
				{
					if (state->material->textureTable[tex].u.image->semantic == 0x5 || state->material->textureTable[tex].u.image->semantic == 0x1) // or identitynormal
					{
						has_normal = true;
					}
					has_spec = state->material->textureTable[tex].u.image->semantic == 0x8 ? true : has_spec;
				}

				if (has_spec && has_normal)
				{
					if (const auto	tech = Material_RegisterTechnique("radiant_fakesun", 1); // fakesun_normal
						tech)
					{
						state->technique = tech;

						// set reflection probe sampler
						if (const auto	image = game::Image_RegisterHandle("_default_cubemap");
										image && image->texture.data)
						{
							game::R_SetSampler(0, state, 1, (char)114, image);
						}
					}
				}
				else if (!has_spec && has_normal)
				{
					if (const auto	tech = Material_RegisterTechnique("radiant_fakesun_no_spec", 1); // fakesun_normal_no_spec_img
						tech)
					{
						state->technique = tech;

						// set reflection probe sampler
						if (const auto	image = game::Image_RegisterHandle("_default_cubemap");
										image && image->texture.data)
						{
							// R_SetSampler(int a1, GfxCmdBufState *state, int sampler, char sampler_state, GfxImage *img)
							utils::hook::call<void(__cdecl)(int unused, game::GfxCmdBufState* _state, int _sampler, char _sampler_state, game::GfxImage* _img)>
								(0x538D70)(0, state, 1, 114, image);
						}
					}
				}
			}
		}

		r_setup_pass_general(source, state, passIndex);
	}

	void r_setup_pass_2d(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, int passIndex)
	{
		bool is_filmtweak_tech = false;

		if(renderer::is_rendering_camerawnd())
		{
			// filmtweaks :: using pixelCostColorCodeMaterial as a proxy
			if (utils::string_equals(state->material->info.name, "pixel_cost_color_code"))
			{
				// replace the technique used by pixelCostColorCodeMaterial
				if (const auto	tech = Material_RegisterTechnique("radiant_filmtweaks", 1);
								tech)
				{
					// allows us to reload the shader using the reload_shaders command
					state->technique = tech;
					is_filmtweak_tech = true;
				}
			}
		}

		r_setup_pass_general(source, state, passIndex);

		if (renderer::is_rendering_camerawnd())
		{
			// filmtweaks :: set colorMapPostSunSampler (uses pre-postfx scene texture)
			if (is_filmtweak_tech)
			{
				if (const auto	cam = ggui::get_rtt_camerawnd();
								cam && cam->scene_texture)
				{
					game::GfxImage postsun = {};
					postsun.texture.data = cam->scene_texture;

					game::R_SetSampler(0, state, 4, (char)114, &postsun);
				}
			}
		}
	}
	
	// *
	// sun preview

	void sunpreview_drawbrush(game::selbrush_def_t* brush /*edi*/)
	{
		const static uint32_t func_addr = 0x406960;
		__asm
		{
			pushad;
			mov		edi, brush;
			call	func_addr;
			popad;
		}
	}

	void renderer::R_AddCmdSetCustomShaderConstant(game::ShaderCodeConstants constant, float x, float y, float z, float w)
	{
		auto setconstant_cmd = reinterpret_cast<game::GfxCmdSetCustomConstant*>(game::R_GetCommandBuffer(sizeof(game::GfxCmdSetCustomConstant), game::GfxRenderCommand::RC_SET_CUSTOM_CONSTANT));
		if(setconstant_cmd)
		{
			setconstant_cmd->type = constant;
			setconstant_cmd->vec[0] = x;
			setconstant_cmd->vec[1] = y;
			setconstant_cmd->vec[2] = z;
			setconstant_cmd->vec[3] = w;
		}
	}

	// parses worldspawn and light settings, sets custom shader constants and returns the sun direction in "sun_dir"
	bool sunpreview_set_shader_constants(float* sun_dir)
	{
		const auto prefs = game::g_PrefsDlg();
		const auto world_ent = reinterpret_cast<game::entity_s_def*>(game::g_world_entity()->firstActive);

		if (!ggui::entity::HasKeyValuePair(world_ent, "sundirection"))
		{
			game::printf_to_console("[Sunpreview] disabled. Missing worldspawn kvp: \"sundirection\"");
			prefs->preview_sun_aswell = false;
			return false;
		}

		if (!ggui::entity::HasKeyValuePair(world_ent, "sunlight"))
		{
			game::printf_to_console("[Sunpreview] disabled. Missing worldspawn kvp: \"sunlight\"");
			prefs->preview_sun_aswell = false;
			return false;
		}

		if (!ggui::entity::HasKeyValuePair(world_ent, "suncolor"))
		{
			game::printf_to_console("[Sunpreview] disabled. Missing worldspawn kvp: \"suncolor\"");
			prefs->preview_sun_aswell = false;
			return false;
		}

		// R_ParseSunLight expects the worldspawn in the following format:
		/* {
			"classname" "worldspawn"
			"sundirection" "%f %f %f"
			"suncolor" "%f %f %f"
			"sunlight" "2.4"
		}*/

		std::string worldspawn;
		worldspawn += "{\n";
		for (auto ep = world_ent->epairs; ep; ep = ep->next)
		{
			worldspawn += utils::va("\"%s\" \"%s\"\n", ep->key, ep->value);
		}
		worldspawn += "}\n";

		game::SunLightParseParams parms = {};
		// R_ParseSunLight(SunLightParseParams *params, const char *str)
		utils::hook::call<void(__cdecl)(game::SunLightParseParams* _params, const char* _str)>(0x50A740)(&parms, worldspawn.c_str());

		game::GfxLight light = {};
		//R_InterpretSunLightParseParamsIntoLights(SunLightParseParams *params, GfxLight *light)
		utils::hook::call<void(__cdecl)(game::SunLightParseParams* _params, game::GfxLight* _light)>(0x50C400)(&parms, &light);

		renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_SUN_POSITION, light.dir[0], light.dir[1], light.dir[2], 0.0f);
		renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_SUN_DIFFUSE, light.color[0], light.color[1], light.color[2], 1.0f);
		renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_SUN_SPECULAR, light.color[0], light.color[1], light.color[2], 1.0f);

		if(sun_dir)
		{
			utils::vector::copy(light.dir, sun_dir);
		}

		return true;
	}

	// draws brushes and entities with "sunpre_" techniques, would normally show rudimentary shadows (they kill fps tho)
	bool sunpreview()
	{
		if(!sunpreview_set_shader_constants(nullptr))
		{
			return false;
		}
		
		auto projection_cmd = reinterpret_cast<game::GfxCmdProjectionSet*>(game::R_GetCommandBuffer(sizeof(game::GfxCmdProjectionSet), game::GfxRenderCommand::RC_PROJECTION_SET));
		if(projection_cmd)
		{
			projection_cmd->projection = game::GfxProjectionTypes::GFX_PROJECTION_2D;
		}

		// this was within the original func, only here for reference
		//game::R_AddCmdDrawFullScreenColoredQuad(0.0f, 0.0f, 1.0f, 1.0f, some_color_from_func_above, material_white_multiply);
		game::R_AddCmdDrawFullScreenColoredQuad(0.0f, 0.0f, 1.0f, 1.0f, game::color_white, game::rgp->clearAlphaStencilMaterial);

		// + shadow related +
		// int& active_sunpreview01 = *reinterpret_cast<int*>(0x23F15B8); active_sunpreview01 = 3;
		// int& active_sunpreview02 = *reinterpret_cast<int*>(0x23F15B4); active_sunpreview02 = 3;

		// SunLightPreview_BrushShadow(&active_brushes, cam_some_matrix, sundir);// nop to disable shadows ++ MUCH FPS
		// SunLightPreview_BrushShadow(&selected_brushes, cam_some_matrix, sundir);// nop to disable shadows ++ MUCH FPS
		// SunLightPreview_PolyOffsetShadows();
		game::R_SortMaterials(); // needed when shadows are active
		
		sunpreview_drawbrush(game::g_selected_brushes());
		sunpreview_drawbrush(game::g_active_brushes());

		// sorts surfaces and adds RC_DRAW_EDITOR_SKINNEDCACHED rendercmd
		utils::hook::call<void(__cdecl)()>(0x4FDA10)();
		
		return true;
	}

	// camera post effects
	// * copy scene without post effects
	// * draw fullscreen quad with shader or proxy material
	// * r_setup_pass_2d() :: setup technique and assign scene texture to the colorMapPostSunSampler
	// * shader logic
	// * copy scene with post effects for imgui
	// *
	// ! implemented "disable for duration logic" (ccamwnd::windowproc()) because resizing the window was causing short black screens
	// ! ^ fixed by moving the camera_postfx hook

	void camera_postfx()
	{
		if (game::dx->targetWindowIndex == ggui::CCAMERAWND)
		{
			const auto r_filmtweakenable = game::Dvar_FindVar("r_filmtweakenable");

			// each frame
			renderer::postfx::set_state(false);
			renderer::postfx::frame();

			// if postfx are disabled for this frame
			if(renderer::postfx::is_disabled())
			{
				return;
			}

			const bool filmtweaks_enabled = r_filmtweakenable && r_filmtweakenable->current.enabled && game::rgp->pixelCostColorCodeMaterial;
			renderer::postfx::set_state(filmtweaks_enabled);
			
			if(renderer::postfx::is_any_active())
			{
				// get scene without postfx -> used for colorMapPostSunSampler
				renderer::copy_scene_to_texture(ggui::CCAMERAWND, ggui::get_rtt_camerawnd()->scene_texture);
			}

			// register_material
			//auto mat_test = utils::hook::call<game::Material* (__cdecl)(const char* _name, int _unk)>(0x511BE0)("hud_font_rendering", 1);

			// filmtweaks :: draw fullscreen quad using pixelCostColorCodeMaterial (we replace its technique in r_setup_pass_2d -> no need to create a custom material this way)
			if (filmtweaks_enabled)
			{
				// RB_FullScreenFilter
				utils::hook::call<void(__cdecl)(game::Material* _material)>(0x531450)(game::rgp->pixelCostColorCodeMaterial);

				// get scene with postfx -> render with imgui
				renderer::copy_scene_to_texture(ggui::CCAMERAWND, ggui::get_rtt_camerawnd()->scene_texture);
			}
		}
	}

	void __declspec(naked) rb_initscene_vp_stub()
	{
		const static uint32_t func_addr = 0x5358D0;
		const static uint32_t retn_addr = 0x535B1F;

		__asm
		{
			call	func_addr;
			add		esp, 4;

			pushad;
			call	camera_postfx;
			popad;

			jmp		retn_addr;
		}
	}

	// *
	// *

	void renderer::register_dvars()
	{
		dvars::r_draw_model_origin = dvars::register_bool(
			/* name		*/ "r_draw_model_origin",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "render model origins");

		dvars::r_draw_model_shadowcaster = dvars::register_bool(
			/* name		*/ "r_draw_model_shadowcaster",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "render shadowcaster materials on xmodels");

		dvars::r_draw_patch_backface_wireframe = dvars::register_bool(
			/* name		*/ "r_draw_patch_backface_wireframe",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "render backface of patches using wireframe");

		dvars::r_fakesun_preview = dvars::register_bool(
			/* name		*/ "r_fakesun_preview",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "render world using a custom fakesun shader (supports specular and bump mapping)");

		dvars::r_fakesun_use_worldspawn = dvars::register_bool(
			/* name		*/ "r_fakesun_use_worldspawn",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "fakesun shader uses the current worldspawn settings (default values for missing keys)");
		
		dvars::r_fakesun_fog_enabled = dvars::register_bool(
			/* name		*/ "r_fakesun_fog_enabled",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "enable fog");
		
		dvars::r_fakesun_fog_start = dvars::register_float(
			/* name		*/ "r_fakesun_fog_start",
			/* default	*/ 900.0f,
			/* mins		*/ 0.0f,
			/* maxs		*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "fog start dist");

		dvars::r_fakesun_fog_half = dvars::register_float(
			/* name		*/ "r_fakesun_fog_half",
			/* default	*/ 2200.0f,
			/* mins		*/ 0.0f,
			/* maxs		*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "fog half way dist");
		
		dvars::r_fakesun_fog_color = dvars::register_vec4(
			/* name		*/ "r_fakesun_fog_color",
			/* x		*/ 0.63f,
			/* y		*/ 0.56f,
			/* z		*/ 0.46f,
			/* w		*/ 1.0f,
			/* minVal	*/ 0.0f,
			/* maxVal	*/ 1.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "fog color");
	}

	renderer::renderer()
	{
		// set default value for r_vsync to false
		utils::hook::set<BYTE>(0x51FB1A + 1, 0x0);

		// silence "gfxCmdBufState.prim.vertDeclType == VERTDECL_PACKED" assert
		utils::hook::nop(0x53AB4A, 5);

		// enable/disable drawing of boxes around the origin on entities
		utils::hook(0x478E33, render_origin_boxes_stub, HOOK_JUMP).install()->quick();

		// enable/disable drawing of backface wireframe on patches
		utils::hook::nop(0x441567, 16);
			 utils::hook(0x441567, patch_backface_wireframe_stub).install()->quick();
		
		// do not force spec and bump picmip
		utils::hook::nop(0x420A73, 30);
		utils::hook::nop(0x4208B0, 30);
		utils::hook::nop(0x41656E, 30);

		// custom shader constants
		utils::hook(0x53BC39, R_SetPassPixelShaderStableArguments_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x53B9E3, R_SetPassShaderObjectArguments_stub, HOOK_JUMP).install()->quick();

		// vertex and pixelshader loading (allows to load shaders that are not included in the shader_names file)
		utils::hook(0x5188A6, r_create_pixelshader, HOOK_CALL).install()->quick();

		utils::hook::nop(0x51873F, 17);
		 	 utils::hook(0x51873F, r_create_vertexshader_stub, HOOK_JUMP).install()->quick();

		
		// fix sun preview (selecting brushes keeps sunpreview active; sun no longer casts shadows -> FPS ++)
		utils::hook(0x406706, sunpreview, HOOK_CALL).install()->quick();

		// disable world darkening when selecting light entities with light preview enabled
		utils::hook::nop(0x407099, 5);
		
		// R_SetupPass @ 0x4FE646 for brushes
		// hook R_SetupPass in R_DrawXModelSkinnedUncached to set custom techniques for xmodels
		utils::hook(0x53AC4F, r_setup_pass_xmodel, HOOK_CALL).install()->quick(); // xmodels
		utils::hook(0x4FE646, r_setup_pass_brush, HOOK_CALL).install()->quick(); // brushes
		utils::hook(0x53A7BA, r_setup_pass_2d, HOOK_CALL).install()->quick(); // 2d and translucent

		// hook RB_InitSceneViewport in RB_CallExecuteRenderCommands to implement camera postfx
		//utils::hook(0x535AFD, rb_initscene_vp_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x535B17, rb_initscene_vp_stub, HOOK_JUMP).install()->quick();

		/*command::register_command_with_hotkey("reload_techniques"s, [this](auto)
		{
			auto& tech_count = *reinterpret_cast<int*>(0x14EFCB8);
			tech_count = 0;
			
			const auto tech_hashtable = reinterpret_cast<void*>(0x14EFCBC);
			memset(tech_hashtable, 0, sizeof(int[8192]));
		});*/

		command::register_command_with_hotkey("reload_shaders"s, [this](auto)
		{
			auto& vs_count = *reinterpret_cast<int*>(0x14E7C2C);
			vs_count = 0;

			const auto vs_hashtable = reinterpret_cast<void*>(0x14E7C30);
			memset(vs_hashtable, 0, sizeof(int[4096]));

			auto& ps_count = *reinterpret_cast<int*>(0x14EBC30);
			ps_count = 0;
			
			const auto ps_hashtable = reinterpret_cast<void*>(0x14EBC34);
			memset(ps_hashtable, 0, sizeof(int[4096]));

			// also reload techniques (loadcode loads shaders)

			auto& tech_count = *reinterpret_cast<int*>(0x14EFCB8);
			tech_count = 0;

			const auto tech_hashtable = reinterpret_cast<void*>(0x14EFCBC);
			memset(tech_hashtable, 0, sizeof(int[8192]));
		});

		command::register_command("dump_shaders_technique", [this](std::vector<std::string> args)
		{
			if (args.size() != 2)
			{
				game::printf_to_console("Usage: dump_shaders_technique <technique_name>");
				return;
			}

			if (const auto	tech = Material_RegisterTechnique(args[1].c_str(), 1); // passthru_film
							tech)
			{
				dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
				if (dvars::fs_homepath)
				{
					std::string filePath = dvars::fs_homepath->current.string;
								filePath += "\\shader_dumps\\";

					std::filesystem::create_directories(filePath);

					std::string technique_name = args[1];

					// vertex shader
					std::ofstream vs_shader_file(filePath + "vs_" + technique_name + ".bin", std::ios::binary);
					if (!vs_shader_file.is_open())
					{
						game::printf_to_console("Could not create vertexshader binary file. Aborting!\n");
						return;
					}

					const std::uint16_t vs_bin_size = tech->passArray->vertexShader->prog.loadDef.programSize;
					vs_shader_file.write(reinterpret_cast<char*>(tech->passArray->vertexShader->prog.loadDef.program), vs_bin_size * 4);
					vs_shader_file.close();

					// pixelshader
					std::ofstream ps_shader_file(filePath + "ps_" + technique_name + ".bin", std::ios::binary);
					if (!ps_shader_file.is_open())
					{
						game::printf_to_console("Could not create pixelshader binary file. Aborting!\n");
						return;
					}

					const std::uint16_t ps_bin_size = tech->passArray->pixelShader->prog.loadDef.programSize;
					ps_shader_file.write(reinterpret_cast<char*>(tech->passArray->pixelShader->prog.loadDef.program), ps_bin_size * 4);
					ps_shader_file.close();
				}
				else
				{
					game::printf_to_console("[!] Could not find technique");
				}
			}
		});

		command::register_command_with_hotkey("fakesun_toggle"s, [this](auto)
		{
			const bool state = !dvars::r_fakesun_preview->current.enabled;

			if(state)
			{
				const auto r_picmip = game::Dvar_FindVar("r_picmip");
				const auto r_picmip_spec = game::Dvar_FindVar("r_picmip_spec");
				const auto r_picmip_bump = game::Dvar_FindVar("r_picmip_bump");

				// use high quality materials if already in-use
				if(r_picmip->current.integer != 0 || r_picmip_spec->current.integer != 0 || r_picmip_bump->current.integer != 0)
				{
					dvars::set_int(r_picmip, 0);
					dvars::set_int(r_picmip_spec, 0);
					dvars::set_int(r_picmip_bump, 0);

					// reload textures
					cdeclcall(void, 0x5139A0); // R_UpdateMipMap
					game::R_ReloadImages();
				}

				// set normal fakelight method
				cmainframe::activewnd->m_pCamWnd->camera.draw_mode = game::RM_NORMALFAKELIGHT;
				game::g_qeglobals->d_savedinfo.iTextMenu = 0x80E0;

				dvars::set_bool(dvars::r_fakesun_preview, true);
			}
			else
			{
				int orig_rendermethod = 2;
				if(game::g_qeglobals->d_savedinfo.iTextMenu >= 0x80DE && game::g_qeglobals->d_savedinfo.iTextMenu <= 0x80E2)
				{
					orig_rendermethod = game::g_qeglobals->d_savedinfo.iTextMenu - 0x80DE;
				}

				dvars::set_bool(dvars::r_fakesun_preview, false);
				cmainframe::activewnd->m_pCamWnd->camera.draw_mode = orig_rendermethod;
			}
		});
	}

	renderer::~renderer()
	{ }
}
