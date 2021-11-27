#include "std_include.hpp"

namespace components
{
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

	void renderer::copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest)
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

			if (desc.Width != static_cast<unsigned int>(game::dx->windows[wnd].width) || desc.Height != static_cast<unsigned int>(game::dx->windows[wnd].height))
			{
				dest->Release();
				dest = nullptr;
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
		//const static uint32_t draw_origin_func = 0x478E30;
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

		if(!R_CreateShaderBuffer(&shader, shader_hash, prefix_buffer))
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
		memcpy(addr_shader_prog, shader_prog_ptr, programSize); // <- the fuck? size 4 ?

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

		if (!R_CreateShaderBuffer(&shader, shader_hash, prefix_buffer))
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
		memcpy(addr_shader_prog, shader_prog_ptr, programSize); // <- the fuck? size 4 ?

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

	/* reload materials / techsets?
	 * R_LoadMaterialHashTable 0x511CD0 ?
	 */

	void Material_LoadTechnique(const char* name, int unk)
	{
		const static uint32_t func_addr = 0x519440;
		__asm
		{
			push	unk;
			mov		ecx, name;
			call	func_addr;
			add		esp, 4;
		}
	}


	/*
	 * custom shader constants within model previewer:
	 * hook call to "R_DrawXModelSkinnedUncached_2" @ 0x4FEB29 to flip bool on
	 * ^ call, then flip bool off
	 * hooks for vertex and shader constants - check if bool is true -> only affects model previewer
	 */

	// only true if we're in the model previewer
	bool _in_draw_xmodelpreview = false;

	void __declspec(naked) r_draw_xmodel_skinned_uncached_stub()
	{
		const static uint32_t func_addr = 0x53AA30;
		const static uint32_t retn_pt = 0x4FEB2E;
		__asm
		{
			mov		_in_draw_xmodelpreview, 1;
			call	func_addr;
			mov		_in_draw_xmodelpreview, 0;
			jmp		retn_pt;
		}
	}

	// *
	// pixelshader custom constants

	void set_custom_pixelshader_constants(game::GfxCmdBufState* state)
	{
		if (_in_draw_xmodelpreview)
		{
			if (state && state->pass)
			{
				// loop through all argument defs to find custom codeconsts
				for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
				{
					const auto arg_def = &state->pass->args[arg];

					if (arg_def && arg_def->type == 5)
					{
						if (state->pass->pixelShader)
						{
							/*if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_MATERIAL_COLOR)
							{
								game::vec4_t temp = { 1.0f, 0.0f, 0.0f, 1.0f };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}*/

							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_LIGHT_DIFFUSE)
							{
								//game::vec4_t get_temp;
								//game::dx->device->GetVertexShaderConstantF(arg_def->dest, get_temp, 1);

								// check in hlsl shader if custom constant is set :: if(lightDiffuse.w == 0.1337f)
								game::vec4_t temp = { 1.0f, 0.4f, 0.4f, 0.1337f };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}

							/*if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FOG_COLOR)
							{
								game::vec4_t temp = { 1.0f, 0.0f, 0.0f, 1.0f };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}*/
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
			//mov		ecx, [esp + 0x24]; // source
			mov		edx, [esp + 0x28]; // state
			push	edx; // state
			//push	ecx; // source
			call	set_custom_pixelshader_constants;
			add		esp, 4;
			popad;

			retn;
		}
	}


	// *
	// vertexshader custom constants

	void set_custom_vertexshader_constants([[maybe_unused]] game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		if(_in_draw_xmodelpreview)
		{
			if (state && state->pass)
			{
				// loop through all argument defs to find custom codeconsts
				for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
				{
					const auto arg_def = &state->pass->args[arg];

					if (arg_def && arg_def->type == 3)
					{
						if (state->pass->vertexShader)
						{
							// set fog
							/*if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FOG)
							{
								game::vec4_t temp;
								game::dx->device->GetVertexShaderConstantF(arg_def->dest, temp, 1);

								temp[2] = -0.005f; // density
								temp[3] = 1.0f; // start
								game::dx->device->SetVertexShaderConstantF(arg_def->dest, temp, 1);
							}*/
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

	// *
	// *

	void renderer::register_dvars()
	{
		dvars::r_draw_model_origin = dvars::register_bool(
			/* name		*/ "r_draw_model_origin",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "render model origins");
	}
	
	renderer::renderer()
	{
		// set default value for r_vsync to false
		utils::hook::set<BYTE>(0x51FB1A + 1, 0x0);

		// silence "gfxCmdBufState.prim.vertDeclType == VERTDECL_PACKED" assert
		utils::hook::nop(0x53AB4A, 5);

		// enable/disable drawing of boxes around the origin on entities
		utils::hook(0x478E33, render_origin_boxes_stub, HOOK_JUMP).install()->quick();


		// custom shader constants
		utils::hook(0x4FEB29, r_draw_xmodel_skinned_uncached_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x53BC39, R_SetPassPixelShaderStableArguments_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x53B9E3, R_SetPassShaderObjectArguments_stub, HOOK_JUMP).install()->quick();

		// vertex and pixelshader loading (allows to load shaders that are not included in the shader_names file)
		utils::hook(0x5188A6, r_create_pixelshader, HOOK_CALL).install()->quick();

		utils::hook::nop(0x51873F, 17);
		 	 utils::hook(0x51873F, r_create_vertexshader_stub, HOOK_JUMP).install()->quick();
	}

	renderer::~renderer()
	{ }
}
