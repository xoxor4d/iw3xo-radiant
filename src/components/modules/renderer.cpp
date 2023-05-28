
/* STOCK RENDERING PATH:
 *  > Add Camera Rendercommands
 *		|> R_SetupScene
 *		|	|> R_SetSceneParms
 *		|	|> add render command/s: clear depth buffer
 *		|
 *		|> add render command/s: brushes, models etc
 *		|> add render command/s: clear depth buffer
 *		|> add render command/s: lines (selection outlines, target->targetname) -> always on top because depthbuffer was cleared
 *
 *  > R_IssueRenderCommands
 *		|> BeginScene
 *		|> RB_Draw3D				[!] (unused because backend has no viewInfo)
 *		|	|> RB_StandardDrawCommands	(bsp, sun and emissive (effects) rendering)
 *		|		|> .....
 *		|		|> R_DrawEmissive
 *		|
 *		|> RB_CallExecuteRenderCommands
 *		|	|> RB_Draw3DCommon					[!] (unused because backend has no viewInfo)
 *		|	|	|> RB_StandardDrawCommandsCommon	(post effects)
 *		|	|
 *		|	|> RB_ExecuteRenderCommandsLoop			(render brushes(meshes), models, lines, setup viewport etc)
 *		|	|> EndScene
 *		|
 *		|> Present */

 // * ----------------------------------------------------------
 // * ----------------------------------------------------------

  /* MODIFIED RENDERING PATH:
   * > Add Camera Rendercommands
   *	|> R_SetupScene
   *	|	|> R_SetSceneParms
   *	|	|> add render command/s: clear depth buffer
   *	|	|> setup_viewinfo	[NEW] (normally part of R_RenderScene, setup viewinfo and emissive drawlists (for RB_StandardDrawCommands))
   *	|
   *	|> add render command/s: brushes, models etc
   *	|> add render command/s: lines (selection outlines, target->targetname) with disabled depth-test (since we no longer clear the depthbuffer)
   *
   *  > R_IssueRenderCommands
   *	|> BeginScene
   *	|> RB_CallExecuteRenderCommands
   *	|	|> pre_scene_command_rendering	[NEW] (only CCAMERAWND :: clear framebuffer color)
   *	|	|> RB_ExecuteRenderCommandsLoop		  (render meshes, models, lines, setup viewport etc)
   *	|	|> post_scene_command_rendering	[NEW] (only CCAMERAWND)
   *	|	|	|> RB_Draw3D
   *	|	|	|	|> RB_StandardDrawCommands
   *	|	|	|		|> R_DrawEmissive		  (draw effects)
   *	|	|	|		|> RB_EndSceneRendering	  (draw debug)
   *	|	|	|
   *	|	|	|> camera_postfx
   *	|	|		|> copy backbuffer
   *	|	|		|> draw fullscreenfilter using rgp->pixelCostColorCodeMaterial (proxy material)
   *	|	|		|	|> hooked R_SetupPass assigns "radiant_filmtweaks" technique to the proxy material
   *	|	|		|	|> hooked R_SetupPass sets postSun sampler (uses the copied backbuffer)
   *	|	|		|	|> custom filmtweak shader applies filmtweaks
   *	|	|		|
   *	|	|		|> copy backbuffer (scene with filmtweaks -> draw via ImGui::Image)
   *	|	|
   *	|	|> EndScene
   *	|
   *	|> Present */

#include "std_include.hpp"

namespace components
{
	renderer* renderer::p_this = nullptr;

	enum EDITOR_SURF_TYPE
	{
		ED_SURF_MODEL = 0x1,
		ED_SURF_MESH = 0x2,
	};

	struct editorMesh_s
	{
		game::Material* material;
		int techType;
		int unk;
		int low16_firstIndex__high16_vbOffset;
		__int16 vertexCount;
		__int16 indexCount;
		int unk3;
	};

	struct __declspec(align(4)) GfxModelSurfaceInfo
	{
		game::DObjAnimMat* baseMat;
		char boneIndex;
		char boneCount;
		unsigned __int16 gfxEntIndex;
		unsigned __int16 lightingHandle;
	};

	union $178D1D161B34F636C03EBC0CA3007D75
	{
		game::GfxPackedVertex* skinnedVert;
		int oldSkinnedCachedOffset;
	};

	struct GfxModelSkinnedSurface
	{
		int skinnedCachedOffset;
		game::XSurface* xsurf;
		GfxModelSurfaceInfo info;
		$178D1D161B34F636C03EBC0CA3007D75 ___u3;
	};

	struct editorSurf_sub
	{
		game::Material* material;
		int techType;
		int unk3;
		GfxModelSkinnedSurface* skinnedSurf;
		game::GfxScaledPlacement* placement;
	};

	struct editorSurf_s
	{
		EDITOR_SURF_TYPE type;
		editorMesh_s* mesh_or_surfSub;
	};

	// *

	editorSurf_s* edSceneGlobals_sceneSurfaces = reinterpret_cast<editorSurf_s*>(0x114565C);
	int& pixelCostMode = *reinterpret_cast<int*>(0x17D0068);

	game::materialCommands_t& tess = *reinterpret_cast<game::materialCommands_t*>(0x177A2C0);
	auto** RB_RenderCommandTable = reinterpret_cast<void(** const)(game::GfxRenderCommandExecState*)>(0x63F800);

	// *

	bool g_log_rendercommands = false;
	game::Material* g_invalid_material = nullptr;

	//bool g_line_depth_testing = true;

	// * ----------------------------------------------------------

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
	// do not cull entities with the custom no-cull flag (used phys prefabs)
	bool cubic_culling_overwrite_check(game::selbrush_def_t* sb)
	{
		if (sb && sb->owner && sb->owner->firstActive)
		{
			return sb->owner->firstActive->custom_no_cull;
		}

		return false;
	}

	void __declspec(naked) cubic_culling_overwrite_stub1()
	{
		const static uint32_t func_addr = 0x4056D0;
		const static uint32_t skip_addr = 0x407ADB;
		const static uint32_t cull_addr = 0x407AF0;
		__asm
		{
			pushad;
			push    ebx;
			call    cubic_culling_overwrite_check;
			add		esp, 4;
			test    al, al;
			popad;

			jne      SKIP;
			call    func_addr;
			test    al, al;
			jne		CULL;

		SKIP:
			jmp		skip_addr;

		CULL:
			jmp		cull_addr;
		}
	}

	void __declspec(naked) cubic_culling_overwrite_stub2()
	{
		const static uint32_t func_addr = 0x4056D0;
		const static uint32_t skip_addr = 0x407BEC;
		__asm
		{
			pushad;
			push    ebx;
			call    cubic_culling_overwrite_check;
			add		esp, 4;
			test    al, al;
			popad;

			jne     SKIP;
			call    func_addr;
			test    al, al;
			setz    al;
			test    al, al;

		SKIP:
			jmp		skip_addr;
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

	void renderer::copy_scene_to_texture(GFXWND_ GFXWND, IDirect3DTexture9*& dest, bool no_release)
	{
		const auto gfx_window = components::renderer::get_window(GFXWND);

		// get the backbuffer surface
		IDirect3DSurface9* surf_backbuffer = nullptr;
		gfx_window->swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surf_backbuffer);

		// write surface to file (test)
		//D3DXSaveSurfaceToFileA("surface_to_file.png", D3DXIFF_PNG, surf_backbuffer, NULL, NULL);

		// check if window size was changed -> release and recreate the texture surface
		if (dest)
		{
			D3DSURFACE_DESC desc;
			dest->GetLevelDesc(0, &desc);

			if(!no_release)
			{
				if (desc.Width != static_cast<unsigned int>(gfx_window->width) || desc.Height != static_cast<unsigned int>(gfx_window->height))
				{
					dest->Release();
					dest = nullptr;
				}
			}
		}

		// create or re-create ^ the texture surface
		if (!dest)
		{
			D3DXCreateTexture(game::dx->device, gfx_window->width, gfx_window->height, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_R8G8B8, D3DPOOL_DEFAULT, &dest);
		}

		// "bind" texture to surface
		IDirect3DSurface9* surf_texture;
		dest->GetSurfaceLevel(0, &surf_texture);

		// "copy" backbuffer to our texture surface
		game::dx->device->StretchRect(surf_backbuffer, NULL, surf_texture, NULL, D3DTEXF_NONE);

		// release the backbuffer surface or we'll leak memory
		surf_backbuffer->Release();
		surf_texture->Release();
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
		
		auto chached_shader = game::Z_Malloc(*shader_len); // programm size in bytes
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

		//game::Com_Error("r_create_pixelshader :: pixel shader creation failed for %s\n", shader_name);
		game::printf_to_console("[ERR] r_create_pixelshader :: pixel shader creation failed for %s\n", shader_name);
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

		//game::Com_Error("r_create_vertexshader :: vertex shader creation failed for %s\n", shader_name);
		game::printf_to_console("[ERR] r_create_vertexshader :: vertex shader creation failed for %s\n", shader_name);
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



	// *
	// pixelshader custom constants

	struct front_backend_variable_helper
	{
		float float_time = 0.0f;
	};

	front_backend_variable_helper gfx_fbv = {};

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
			// #
			// 2D: set required shader constants for backend passes
			if (renderer::is_rendering_camerawnd() && source->viewMode == game::VIEW_MODE_2D)
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


			// #
			// 3D (a bit of dupe code - makes it more readable and manageable)

			bool is_d3dbsp = false;
			if (state->viewport.x == 1)
			{
				state->viewport.x = 0;
				is_d3dbsp = true;
			}

			// model previewer - fake sun shader
			if (renderer::is_rendering_layeredwnd() && layermatwnd::rendermethod_preview == layermatwnd::FAKESUN_DAY)
			{
				for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
				{
					const auto arg_def = &state->pass->args[arg];
					if (arg_def && arg_def->type == 5 && state->pass->pixelShader)
					{
						if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_ENVMAP_PARMS)
						{
							game::vec4_t temp;
							game::dx->device->GetPixelShaderConstantF(arg_def->dest, temp, 1);

							// set envmapparams from material
							if (state->material && state->material->constantCount && state->material->constantTable)
							{
								for (auto constant = 0; constant < state->material->constantCount; constant++)
								{
									if (state->material->constantTable[constant].name[0] != 0)
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

						else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_POSITION)
						{
							bool worldspawn_valid = false;
							game::vec3_t sun_dir = {};

							if (dvars::r_fakesun_use_worldspawn->current.enabled)
							{
								const auto world_ent = game::g_world_entity();
								if (world_ent && world_ent->firstActive && GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(world_ent->firstActive, sun_dir, "sundirection"))
								{
									worldspawn_valid = true;
								}
							}

							game::vec4_t temp = { 0.0f, 0.0f, 0.0f, 0.0f };

							// AngleVectors(float* angles, float* vpn, float* right, float* up)
							utils::hook::call<void(__cdecl)(float* angles, float* vpn, float* right, float* up)>(0x4ABD70)(worldspawn_valid ? sun_dir : GET_GUI(ggui::camera_settings_dialog)->sun_dir, temp, nullptr, nullptr);
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

								if (world_ent && world_ent->firstActive && GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(world_ent->firstActive, suncolor, "suncolor"))
								{
									if (!GET_GUI(ggui::entity_dialog)->get_value_for_key_from_entity(world_ent->firstActive, &sunlight, "sunlight"))
									{
										// default value
										sunlight = 1.35f;
									}

									sunlight *= 1.5f;
									utils::vector::scale(suncolor, sunlight, sun_diffuse);

									worldspawn_valid = true;
								}
							}

							const auto cs = GET_GUI(ggui::camera_settings_dialog);

							const game::vec4_t temp = { cs->sun_diffuse[0], cs->sun_diffuse[1], cs->sun_diffuse[2], 1.0f };
							game::dx->device->SetPixelShaderConstantF(arg_def->dest, worldspawn_valid ? sun_diffuse : temp, 1);
						}

						else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_SPECULAR)
						{
							bool worldspawn_valid = false;
							game::vec4_t sun_specular = {};

							if (dvars::r_fakesun_use_worldspawn->current.enabled)
							{
								const auto world_ent = game::g_world_entity();
								if (world_ent && world_ent->firstActive && GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(world_ent->firstActive, sun_specular, "suncolor"))
								{
									// worldspawn suncolor
									utils::vector::ma(game::vec3_t(1.0f, 1.0f, 1.0f), 2.0f, sun_specular, sun_specular);
									worldspawn_valid = true;
								}
							}

							const auto cs = GET_GUI(ggui::camera_settings_dialog);

							const game::vec4_t temp = { cs->sun_specular[0], cs->sun_specular[1], cs->sun_specular[2], cs->sun_specular[3] };
							game::dx->device->SetPixelShaderConstantF(arg_def->dest, worldspawn_valid ? sun_specular : temp, 1);
						}

						// "envMapParams" in camera settings
						else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_LIGHT_SPOTDIR)
						{
							const auto cs = GET_GUI(ggui::camera_settings_dialog);

							const game::vec4_t temp = { cs->material_specular[0], cs->material_specular[1], cs->material_specular[2], cs->material_specular[3] };
							game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
						}

						else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_LIGHT_SPOTFACTORS)
						{
							const auto cs = GET_GUI(ggui::camera_settings_dialog);

							const game::vec4_t temp = { cs->ambient[0], cs->ambient[1], cs->ambient[2], cs->ambient[3] };
							game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
						}
						
					}
				}
			}

			// camera - fake sun shader - bsp
			if (renderer::is_rendering_camerawnd() || renderer::is_rendering_effectswnd())
			{
				for (auto arg = 0; arg < state->pass->perObjArgCount + state->pass->perPrimArgCount + state->pass->stableArgCount; arg++)
				{
					const auto arg_def = &state->pass->args[arg];
					if (arg_def && arg_def->type == 5 && state->pass->pixelShader)
					{
						if (!is_d3dbsp)
						{
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_ENVMAP_PARMS)
							{
								game::vec4_t temp;
								game::dx->device->GetPixelShaderConstantF(arg_def->dest, temp, 1);

								// set envmapparams from material
								if (state->material && state->material->constantCount && state->material->constantTable)
								{
									for (auto constant = 0; constant < state->material->constantCount; constant++)
									{
										if (state->material->constantTable[constant].name)
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

							else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_LIGHT_SPOTDIR)
							{
								if (!utils::starts_with(state->technique->name, "l_spot_"s)) // constant is needed for spotlight preview
								{
									const auto cs = GET_GUI(ggui::camera_settings_dialog);

									const game::vec4_t temp = { cs->material_specular[0], cs->material_specular[1], cs->material_specular[2], cs->material_specular[3] };
									game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
								}
							}

							else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_LIGHT_SPOTFACTORS)
							{
								if (!utils::starts_with(state->technique->name, "l_spot_"s)) // constant is needed for spotlight preview
								{
									const auto cs = GET_GUI(ggui::camera_settings_dialog);

									const game::vec4_t temp = { cs->ambient[0], cs->ambient[1], cs->ambient[2], cs->ambient[3] };
									game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
								}
							}
						} // !is_d3dbsp

						if (dvars::r_fakesun_fog_enabled->current.enabled)
						{
							if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_FOG_COLOR)
							{
								const game::vec4_t temp = { dvars::r_fakesun_fog_color->current.vector[0], dvars::r_fakesun_fog_color->current.vector[1], dvars::r_fakesun_fog_color->current.vector[2], dvars::r_fakesun_fog_color->current.vector[3] };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}
						}
							
						if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_POSITION)
						{
							if (!is_d3dbsp || (is_d3dbsp && dvars::r_draw_bsp_overwrite_sundir->current.enabled))
							{
								bool worldspawn_valid = false;
								game::vec3_t sun_dir = {};

								if (dvars::r_fakesun_use_worldspawn->current.enabled)
								{
									const auto world_ent = game::g_world_entity();
									if (world_ent && world_ent->firstActive && GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(world_ent->firstActive, sun_dir, "sundirection"))
									{
										worldspawn_valid = true;
									}
								}

								game::vec4_t temp = { 0.0f, 0.0f, 0.0f, 0.0f };

								// AngleVectors(float* angles, float* vpn, float* right, float* up)
								utils::hook::call<void(__cdecl)(float* angles, float* vpn, float* right, float* up)>(0x4ABD70)(worldspawn_valid ? sun_dir : GET_GUI(ggui::camera_settings_dialog)->sun_dir, temp, nullptr, nullptr);
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, temp, 1);
							}
						}
							
						else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_DIFFUSE)
						{
							if (!is_d3dbsp || (is_d3dbsp && dvars::r_draw_bsp_overwrite_sunlight->current.enabled))
							{
								bool worldspawn_valid = false;
								game::vec4_t sun_diffuse = {};

								if (dvars::r_fakesun_use_worldspawn->current.enabled)
								{
									const auto world_ent = game::g_world_entity();

									float sunlight = 0.0f;
									game::vec3_t suncolor = {};

									if (world_ent && world_ent->firstActive && GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(world_ent->firstActive, suncolor, "suncolor"))
									{
										if (!GET_GUI(ggui::entity_dialog)->get_value_for_key_from_entity(world_ent->firstActive, &sunlight, "sunlight"))
										{
											// default value
											sunlight = 1.35f;
										}

										sunlight *= 1.5f;
										utils::vector::scale(suncolor, sunlight, sun_diffuse);

										worldspawn_valid = true;
									}
								}

								const auto cs = GET_GUI(ggui::camera_settings_dialog);

								const game::vec4_t temp = { cs->sun_diffuse[0], cs->sun_diffuse[1], cs->sun_diffuse[2], 1.0f };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, worldspawn_valid ? sun_diffuse : temp, 1);
							}
						}
						
						else if (arg_def->u.codeConst.index == game::ShaderCodeConstants::CONST_SRC_CODE_SUN_SPECULAR)
						{
							if (!is_d3dbsp || (is_d3dbsp && dvars::r_draw_bsp_overwrite_sunspecular->current.enabled))
							{
								bool worldspawn_valid = false;
								game::vec4_t sun_specular = {};

								if (dvars::r_fakesun_use_worldspawn->current.enabled)
								{
									const auto world_ent = game::g_world_entity();
									if (world_ent && world_ent->firstActive && GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(world_ent->firstActive, sun_specular, "suncolor"))
									{
										// worldspawn suncolor
										utils::vector::ma(game::vec3_t(1.0f, 1.0f, 1.0f), 2.0f, sun_specular, sun_specular);
										worldspawn_valid = true;
									}
								}

								const auto cs = GET_GUI(ggui::camera_settings_dialog);

								const game::vec4_t temp = { cs->sun_specular[0], cs->sun_specular[1], cs->sun_specular[2], cs->sun_specular[3] };
								game::dx->device->SetPixelShaderConstantF(arg_def->dest, worldspawn_valid ? sun_specular : temp, 1);
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

					if (arg_def && arg_def->type == 3 && state->pass->vertexShader)
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

	// 0x558D2E renders effect xmodels

	void r_setup_pass_xmodel(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, int passIndex)
	{
		if ((renderer::is_rendering_layeredwnd() && layermatwnd::rendermethod_preview == layermatwnd::FAKESUN_DAY) || 
			(!renderer::is_rendering_layeredwnd() && dvars::r_fakesun_preview->current.enabled))
		{
			/*if (utils::string_equals(state->technique->name, "fakelight_normal_dtex") ||
				utils::string_equals(state->technique->name, "fakelight_normal_d0_dtex"))*/
			if (state->techType == game::TECHNIQUE_FAKELIGHT_NORMAL)
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
					bool has_scroll = false;
					bool has_flag = false;

					if (state->material->techniqueSet && state->material->techniqueSet->techniques[5])
					{
						has_scroll = utils::string_contains(state->material->techniqueSet->techniques[5]->name, "scroll");

						if (!has_scroll)
						{
							has_flag = utils::string_contains(state->material->techniqueSet->techniques[5]->name, "flag");
						}
					}

					if (!has_flag)
					{
						if (const auto	tech = Material_RegisterTechnique(has_scroll ? "radiant_fakesun_scroll_dtex" : "radiant_fakesun_dtex", 1); // fakesun_normal_dtex
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
				}
				else if (!has_spec && has_normal)
				{
					bool has_scroll = false;
					bool has_flag = false;

					if (state->material->techniqueSet && state->material->techniqueSet->techniques[5])
					{
						has_scroll = utils::string_contains(state->material->techniqueSet->techniques[5]->name, "scroll");

						if (!has_scroll)
						{
							has_flag = utils::string_contains(state->material->techniqueSet->techniques[5]->name, "flag");
						}
					}

					if (!has_flag)
					{
						if (const auto	tech = Material_RegisterTechnique(has_scroll ? "radiant_fakesun_no_spec_scroll_dtex" : "radiant_fakesun_no_spec_dtex", 1); // fakesun_normal_no_spec_img_dtex
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
		}

		if (dvars::r_draw_model_shadowcaster && !dvars::r_draw_model_shadowcaster->current.enabled)
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
			if(state->material->techniqueSet && state->material->techniqueSet->name == "wc_tools"s)
			{
				// dont use fakesun on tool textures
			}
			/*else if (utils::string_equals(state->technique->name, "fakelight_normal") ||
					 utils::string_equals(state->technique->name, "fakelight_normal_d0"))*/
			else if(state->techType == game::TECHNIQUE_FAKELIGHT_NORMAL)
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

		if (renderer::is_rendering_camerawnd())
		{
			if (const auto	draw_water = game::Dvar_FindVar("r_drawWater");
							draw_water && draw_water->current.enabled)
			{
				if (state && state->material && state->material->techniqueSet && 
					utils::string_equals(state->material->techniqueSet->name, "wc_water"))
				{
					if (const auto	tech = Material_RegisterTechnique("water_l_sun", 1);
									tech)
					{
						state->technique = tech;
					}
				}
			}
		}

		r_setup_pass_general(source, state, passIndex);
	}

	void r_setup_pass_2d(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, int passIndex)
	{
		bool is_filmtweak_tech = false;

		if (renderer::is_rendering_camerawnd())
		{
			// filmtweaks :: using pixelCostColorCodeMaterial as a proxy
			if (utils::string_equals(state->material->info.name, "pixel_cost_color_code"))
			{
				// replace the technique used by pixelCostColorCodeMaterial
				if (const auto	tech = Material_RegisterTechnique("radiant_filmtweaks", 1);
								tech)
				{
					// technique and shader can be reloaded with using the reload_shaders command
					state->technique = tech;
					is_filmtweak_tech = true;
				}
			}
		}

		if (renderer::is_rendering_texturewnd())
		{
			if (state->technique->name == "sky"s)
			{
				// properly display sky materials
				if (const auto	tech = Material_RegisterTechnique("radiant_sky_2d", 1);
								tech)
				{
					// technique and shader can be reloaded with using the reload_shaders command
					state->technique = tech;
				}
			}
		}

		r_setup_pass_general(source, state, passIndex);

		if (renderer::is_rendering_camerawnd())
		{
			// filmtweaks :: set colorMapPostSunSampler (uses pre-postfx scene texture)
			if (is_filmtweak_tech)
			{
				if (const auto	cam = GET_GUI(ggui::camera_dialog);
								cam && cam->rtt_get_texture())
				{
					game::GfxImage postsun = {};
					postsun.texture.data = cam->rtt_get_texture();

					game::R_SetSampler(0, state, 4, (char)114, &postsun);
				}
			}
		}
	}

	void r_setup_pass_surflists(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, int passIndex)
	{
		if (d3dbsp::Com_IsBspLoaded() && dvars::r_draw_bsp->current.enabled)
		{
			//state->depthRangeNear = 0.01337f;
			state->viewport.x = 1;
		}

		//const auto effect_browser = GET_GUI(ggui::effects_browser);
		
		if (effects::effect_is_playing() && (!d3dbsp::Com_IsBspLoaded() || (d3dbsp::Com_IsBspLoaded() && !dvars::r_draw_bsp->current.enabled)) 
			|| renderer::is_rendering_effectswnd()) /*(effect_browser->is_active() && !effect_browser->is_inactive_tab() || cfxwnd::get()->m_effect_is_playing))*/
		{
			{
				std::string tech_name = state->technique->name;
				if (utils::erase_substring(tech_name, "_outdoor"))
				{
					if (const auto	tech = Material_RegisterTechnique(tech_name.c_str(), 1);
						tech)
					{
						state->technique = tech;
					}
				}
			}
		}

		if ((renderer::is_rendering_layeredwnd() && layermatwnd::rendermethod_preview == layermatwnd::FAKESUN_DAY) ||
			(!renderer::is_rendering_layeredwnd() && dvars::r_fakesun_preview->current.enabled))
		{
			if (state->techType == game::TECHNIQUE_FAKELIGHT_NORMAL)
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
					bool has_scroll = false;
					if (state->material->techniqueSet && state->material->techniqueSet->techniques[5])
					{
						has_scroll = utils::string_contains(state->material->techniqueSet->techniques[5]->name, "scroll");
					}

					if (const auto	tech = Material_RegisterTechnique(has_scroll ? "radiant_fakesun_scroll_dtex" : "radiant_fakesun_dtex", 1); // fakesun_normal_dtex
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
					bool has_scroll = false;
					if (state->material->techniqueSet && state->material->techniqueSet->techniques[5])
					{
						has_scroll = utils::string_contains(state->material->techniqueSet->techniques[5]->name, "scroll");
					}

					if (const auto	tech = Material_RegisterTechnique(has_scroll ? "radiant_fakesun_no_spec_scroll_dtex" : "radiant_fakesun_no_spec_dtex", 1); // fakesun_normal_no_spec_img_dtex
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
		
		r_setup_pass_general(source, state, passIndex);
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

	bool sunpreview_filter_brush(game::selbrush_def_t* brush /*esi*/, int unk)
	{
		const static uint32_t func_addr = 0x46A1F0;
		__asm
		{
			mov		esi, brush;
			push	unk;
			call	func_addr;
			add		esp, 4;
		}
	}

	void sunpreview_entity_get_orientation(game::entity_s_def* ent /*eax*/, game::orientation_t* orient /*ebx*/, game::orientation_t* orient_out /*edi*/)
	{
		const static uint32_t func_addr = 0x482A70;
		__asm
		{
			pushad;
			mov		eax, ent;
			mov		ebx, orient;
			mov		edi, orient_out;
			call	func_addr;
			popad;
		}
	}

	bool sunpreview_draw_brush_shadow(float* sundir /*eax*/, game::selbrush_def_t* brush /*edx*/, game::orientation_t* orient /*ecx*/)
	{
		const static uint32_t func_addr = 0x47B2A0;
		__asm
		{
			pushad;
			mov		eax, sundir;
			mov		edx, brush;
			mov		ecx, orient;
			call	func_addr;
			popad;
		}
	}

	void sunpreview_brush_shadow(DWORD* sb, game::orientation_t* orient, float* sundir)
	{
		game::selbrush_def_t* sb_test = (game::selbrush_def_t*)sb;
		game::orientation_t orient_out = {};

		game::selbrush_def_t* x_sb = sb_test;
		game::selbrush_def_t* sb_next = sb_test->next;

		if (sb_next != sb_test)
		{
			do
			{
				if(sb_next->def)
				{
					const game::vec3_t brush_center =
					{
						((sb_next->def->mins[0] + sb_next->def->maxs[0]) * 0.5f),
						((sb_next->def->mins[1] + sb_next->def->maxs[1]) * 0.5f),
						((sb_next->def->mins[2] + sb_next->def->maxs[2]) * 0.5f)
					};

					if (utils::vector::distance(brush_center, cmainframe::activewnd->m_pCamWnd->camera.origin) < dvars::r_sunpreview_shadow_dist->current.value)
					{
						if (!sunpreview_filter_brush(sb_next, 0))
						{
							if (sb_next->owner->prefab)
							{
								sunpreview_entity_get_orientation(reinterpret_cast<game::entity_s_def*>(sb_next->owner->firstActive), orient, &orient_out);
								sunpreview_brush_shadow((DWORD*)(sb_next->owner->prefab + 12), &orient_out, sundir);
								x_sb = sb_test;
							}
							else
							{
								sunpreview_draw_brush_shadow(sundir, sb_next, orient);
							}
						}
					}
					sb_next = sb_next->next;
				}

			} while (sb_next != x_sb);
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
		const auto entity_gui = GET_GUI(ggui::entity_dialog);
		const auto prefs = game::g_PrefsDlg();
		const auto world_ent = reinterpret_cast<game::entity_s_def*>(game::g_world_entity()->firstActive);


		if (!entity_gui->has_key_value_pair(world_ent, "sundirection"))
		{
			game::printf_to_console("[Sunpreview] disabled. Missing worldspawn kvp: \"sundirection\"");
			prefs->preview_sun_aswell = false;
			return false;
		}

		if (!entity_gui->has_key_value_pair(world_ent, "sunlight"))
		{
			game::printf_to_console("[Sunpreview] disabled. Missing worldspawn kvp: \"sunlight\"");
			prefs->preview_sun_aswell = false;
			return false;
		}

		if (!entity_gui->has_key_value_pair(world_ent, "suncolor"))
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
		float sun_dir_from_worldspawn[4] = {};
		if(!sunpreview_set_shader_constants(sun_dir_from_worldspawn))
		{
			return false;
		}
		
		auto projection_cmd = reinterpret_cast<game::GfxCmdProjectionSet*>(game::R_GetCommandBuffer(sizeof(game::GfxCmdProjectionSet), game::GfxRenderCommand::RC_PROJECTION_SET));
		if(projection_cmd)
		{
			projection_cmd->projection = game::GfxProjectionTypes::GFX_PROJECTION_2D;
		}

		// + shadow related +
		const auto material_white_multiply = reinterpret_cast<game::Material*>(*(DWORD*)0x23F15C4);
		game::R_AddCmdDrawFullScreenColoredQuad(0.0f, 0.0f, 1.0f, 1.0f, nullptr, material_white_multiply);
		

		// sunpreview related
		game::R_AddCmdDrawFullScreenColoredQuad(0.0f, 0.0f, 1.0f, 1.0f, game::color_white, game::rgp->clearAlphaStencilMaterial);

		if (dvars::r_sunpreview_shadow_enable->current.enabled)
		{
			// + shadow related +
			int& active_sunpreview01 = *reinterpret_cast<int*>(0x23F15B8); active_sunpreview01 = 6;
			int& active_sunpreview02 = *reinterpret_cast<int*>(0x23F15B4); active_sunpreview02 = 6;

			const auto orient = reinterpret_cast<game::orientation_t*>(0x6DE290);
			sunpreview_brush_shadow(&*game::active_brushes_ptr, orient, sun_dir_from_worldspawn);
			sunpreview_brush_shadow(&*game::currSelectedBrushes, orient, sun_dir_from_worldspawn);

			// SunLightPreview_PolyOffsetShadows
			utils::hook::call<void(__cdecl)()>(0x4561B0)();
		}

		game::R_SortMaterials();

		sunpreview_drawbrush(game::g_selected_brushes());
		sunpreview_drawbrush(game::g_active_brushes());

		// sorts surfaces and adds RC_DRAW_EDITOR_SKINNEDCACHED rendercmd
		utils::hook::call<void(__cdecl)()>(0x4FDA10)();
		
		return true;
	}

	// check for nullptr (world_entity)
	void __declspec(naked) sunlight_preview_arg_check()
	{
		const static uint32_t retn_addr = 0x4067D0;
		const static uint32_t onzero_retn_addr = 0x4067E0;
		__asm
		{
			mov[ebp - 0x20DC], ecx; // og

			pushad;
			test	edx, edx;			// world_entity
			jz		ENT_IS_ZERO;

			popad;
			mov     esi, [edx + 8];		// og
			jmp		retn_addr;

		ENT_IS_ZERO:
			popad;
			jmp		onzero_retn_addr;
		}
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
			renderer::copy_scene_to_texture(renderer::CCAMERAWND, GET_GUI(ggui::camera_dialog)->rtt_get_texture());
		}

		// register_material
		//auto mat_test = utils::hook::call<game::Material* (__cdecl)(const char* _name, int _unk)>(0x511BE0)("hud_font_rendering", 1);

		// filmtweaks :: draw fullscreen quad using pixelCostColorCodeMaterial (we replace its technique in r_setup_pass_2d -> no need to create a custom material this way)
		if (filmtweaks_enabled)
		{
			// RB_FullScreenFilter
			utils::hook::call<void(__cdecl)(game::Material* _material)>(0x531450)(game::rgp->pixelCostColorCodeMaterial);

			// get scene with postfx -> render with imgui
			renderer::copy_scene_to_texture(renderer::CCAMERAWND, GET_GUI(ggui::camera_dialog)->rtt_get_texture());
		}
	}


	// called before rendering the command queue
	void pre_scene_command_rendering()
	{
		if (game::dx->targetWindowIndex == renderer::CCAMERAWND)
		{
			// clear framebuffer (color)
			renderer::R_SetAndClearSceneTarget(true);
			renderer::get()->set_line_depthtesting(true);
		}
	}


	// called after rendering the command queue
	void post_scene_command_rendering()
	{
		if (game::dx->targetWindowIndex == renderer::CCAMERAWND
			|| game::dx->targetWindowIndex == renderer::CFXWND)
		{
			// render emissive surfs (effects)
			renderer::RB_Draw3D();
		}

		if (game::dx->targetWindowIndex == renderer::CCAMERAWND)
		{
			// post effects logic (filmtweaks)
			camera_postfx();
		}
	}

	std::string rendercommand_id_to_string(int id)
	{
		switch(id)
		{
		case game::RC_SET_MATERIAL_COLOR:
			return "RC_SET_MATERIAL_COLOR"s;

		case game::RC_SET_LIGHT_COLOR:
			return "RC_SET_LIGHT_COLOR"s;

		case game::RC_SAVE_SCREEN:
			return "RC_SAVE_SCREEN"s;

		case game::RC_SAVE_SCREEN_SECTION:
			return "RC_SAVE_SCREEN_SECTION"s;

		case game::RC_CLEAR_SCREEN:
			return "RC_CLEAR_SCREEN"s;

		case game::RC_BEGIN_VIEW:
			return "RC_BEGIN_VIEW"s;

		case game::RC_SET_VIEWPORT:
			return "RC_SET_VIEWPORT"s;

		case game::RC_STRETCH_PIC:
			return "RC_STRETCH_PIC"s;

		case game::RC_STRETCH_PIC_FLIP_ST:
			return "RC_STRETCH_PIC_FLIP_ST"s;

		case game::RC_STRETCH_PIC_ROTATE_XY:
			return "RC_STRETCH_PIC_ROTATE_XY"s;

		case game::RC_STRETCH_PIC_ROTATE_ST:
			return "RC_STRETCH_PIC_ROTATE_ST"s;

		case game::RC_STRETCH_RAW:
			return "RC_STRETCH_RAW"s;

		case game::RC_DRAW_QUAD_PIC:
			return "RC_DRAW_QUAD_PIC"s;

		case game::RC_DRAW_FULLSCREEN_COLORED_QUAD:
			return "RC_DRAW_FULLSCREEN_COLORED_QUAD"s;

		case game::RC_DRAW_TEXT_2D:
			return "RC_DRAW_TEXT_2D"s;

		case game::RC_DRAW_TEXT_3D:
			return "RC_DRAW_TEXT_3D"s;

		case game::RC_BLEND_SAVED_SCREEN_BLURRED:
			return "RC_BLEND_SAVED_SCREEN_BLURRED"s;

		case game::RC_BLEND_SAVED_SCREEN_FLASHED:
			return "RC_BLEND_SAVED_SCREEN_FLASHED"s;

		case game::RC_DRAW_POINTS:
			return "RC_DRAW_POINTS"s;

		case game::RC_DRAW_LINES:
			return "RC_DRAW_LINES"s;

		case game::RC_DRAW_TRIANGLES:
			return "RC_DRAW_TRIANGLES"s;

		case game::RC_DRAW_EDITOR_SKINNEDCACHED:
			return "RC_DRAW_EDITOR_SKINNEDCACHED"s;

		case game::RC_SET_CUSTOM_CONSTANT:
			return "RC_SET_CUSTOM_CONSTANT"s;

		case game::RC_PROJECTION_SET:
			return "RC_PROJECTION_SET"s;

		default: 
			return "UNK";
		}
	}

	void RB_ExecuteRenderCommandsLoop(const void* cmds)
	{
		if (tess.indexCount)
		{
			Assert();
		}

		const void* prevCmd = nullptr;

		game::GfxRenderCommandExecState execState = {};
		execState.cmd = cmds;

		bool log = g_log_rendercommands && game::dx->targetWindowIndex == renderer::CCAMERAWND;
		std::ofstream log_file;
		std::uint32_t log_last_id = 0;
		std::uint32_t log_last_id_count = 1;

		if(log)
		{
			dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
			if (dvars::fs_homepath)
			{
				std::string filePath = dvars::fs_homepath->current.string;
							filePath += "\\IW3xRadiant\\LOG_rendercommands.txt"s;

				log_file.open(filePath.c_str());
				if (!log_file.is_open())
				{
					game::printf_to_console("[!] Could not create log file\n");

					log = false;
					g_log_rendercommands = false;
				}
			}
		}

		while (true)
		{
			prevCmd = execState.cmd;
			if ((reinterpret_cast<int>(execState.cmd) & 3) != 0)
			{
				Assert();
			}

			const game::GfxCmdHeader* header = (game::GfxCmdHeader*)execState.cmd;
			if (!header->id)
			{
				break;
			}

			if (header->id >= 25u)
			{
				Assert();
			}

			if (!RB_RenderCommandTable[header->id])
			{
				Assert();
			}

			RB_RenderCommandTable[header->id](&execState);

			if (execState.cmd == prevCmd)
			{
				Assert();
			}

			if (log)
			{
				const auto h_id = static_cast<std::uint32_t>(header->id);

				if(log_last_id == h_id)
				{
					log_last_id_count++;
				}
				else
				{
					log_file << "[" << rendercommand_id_to_string(h_id) << "] for [" << log_last_id_count << "] iterations." << std::endl;
					log_last_id_count = 1;
				}

				log_last_id = h_id;
			}
		}

		if (tess.indexCount)
		{
			//RB_EndTessSurface();
			utils::hook::call<void(__cdecl)()>(0x53ADC0)();
			
		}

		if (log)
		{
			log_file.close();
			g_log_rendercommands = false;
		}
	}

	void __declspec(naked) RB_ExecuteRenderCommandsLoop_stub()
	{
		const static uint32_t retn_addr = 0x535B1F;
		__asm
		{
			pushad;
			call	pre_scene_command_rendering;
			popad;

			mov     edx, [eax + 0xDE1C0]; 
			push    edx; // backEndData->execState
			call	RB_ExecuteRenderCommandsLoop;
			add		esp, 4;

			pushad;
			call	post_scene_command_rendering;
			popad;

			jmp		retn_addr;
		}
	}


	// *
	// trigger reflections / prefab preview generation

	void on_cam_paint_post_rendercommands()
	{
		if (dvars::r_reflectionprobe_generate->current.enabled)
		{
			if (!dvars::r_draw_bsp->current.enabled)
			{
				game::printf_to_console("[Reflections] Turning on bsp view ...");
				command::execute("toggle_bsp_radiant");
			}

			reflectionprobes::generate_reflections_for_bsp();
			dvars::set_bool(dvars::r_reflectionprobe_generate, false);

			if (!dvars::bsp_show_bsp_after_compile->current.enabled)
			{
				command::execute("toggle_bsp_radiant");
				dvars::set_bool(dvars::r_draw_bsp, dvars::bsp_show_bsp_after_compile->current.enabled);
			}
		}

		static int skip_frame = 0;
		static bool drawfps_state = false;

		if (ggui::prefab_browser_generate_thumbnails)
		{
			if (skip_frame < 10)
			{
				if (!skip_frame)
				{
					game::printf_to_console("[Prefab Preview] About to take thumbnails ...");
					imgui::Toast(ImGuiToastType_Info, "Generating Prefab Previews", "This might take a while ...");

					drawfps_state = dvars::gui_draw_fps->current.enabled;
					dvars::set_bool(dvars::gui_draw_fps, false);
				}
				
				skip_frame++;
			}
			else
			{
				generate_previews::generate_prefab_previews(ggui::prefab_browser_generate_thumbnails_folder);
				ggui::prefab_browser_generate_thumbnails = false;
				skip_frame = 0;

				dvars::set_bool(dvars::gui_draw_fps, drawfps_state);
			}
		}
	}

	void __declspec(naked) on_cam_paint_post_rendercommands_stub()
	{
		const static uint32_t func_addr = 0x4FD910; // R_SortMaterials
		const static uint32_t retn_addr = 0x403070;
		__asm
		{
			pushad;
			call	on_cam_paint_post_rendercommands;
			popad;

			call	func_addr;
			jmp		retn_addr;
		}
	}

	
	// *
	// part of R_RenderScene
	// setup viewInfo and drawlists so that RB_Draw3D::RB_StandardDrawCommands actually renders something (unused in radiant and would normally render the bsp and effects)

	void renderer::R_InitDrawSurfListInfo(game::GfxDrawSurfListInfo* list)
	{
		list->drawSurfs = nullptr;
		list->drawSurfCount = 0;
		list->baseTechType = game::TECHNIQUE_DEPTH_PREPASS;
		list->viewInfo = nullptr;
		list->viewOrigin[0] = 0.0f;
		list->viewOrigin[1] = 0.0f;
		list->viewOrigin[2] = 0.0f;
		list->viewOrigin[3] = 0.0f;
		list->light = nullptr;
		list->cameraView = 0;
	}

	void setup_viewinfo(game::GfxViewParms* viewParms)
	{
		if (game::dx->targetWindowIndex != renderer::CCAMERAWND)
		{
			return;
		}

		const auto renderer = renderer::get();

		const double t = clock() / 1000.0;
		renderer->m_cam_msec = static_cast<float>(t - renderer->m_cam_time);
		renderer->m_cam_time = t;


		// TODO:
		// * unify physics frame logic

		const auto physx = components::physx_impl::get();
		if (physx->m_character_controller_enabled)
		{
			physx->m_cct_camera->update(renderer->m_cam_msec);
		}
		else
		{
			//physx_cct_camera::reset_enter_controller_parms();
		}

		if (effects::effect_can_play() || GET_GUI(ggui::camera_settings_dialog)->phys_force_frame_logic)
		{
			components::physx_impl::get()->fx_frame();
		}

		if (components::physx_impl::get()->m_phys_sim_run)
		{
			components::physx_impl::get()->phys_frame();
		}

		if (d3dbsp::Com_IsBspLoaded() && dvars::r_draw_bsp->current.enabled)
		{
			d3dbsp::add_entities_to_scene();


			game::refdef_s refdef = {};
			utils::vector::copy(viewParms->origin, refdef.vieworg);
			utils::vector::copy(viewParms->axis[0], refdef.viewaxis[0]);
			utils::vector::copy(viewParms->axis[1], refdef.viewaxis[1]);
			utils::vector::copy(viewParms->axis[2], refdef.viewaxis[2]);

			refdef.width = components::renderer::get_window(components::renderer::CCAMERAWND)->width;
			refdef.height = components::renderer::get_window(components::renderer::CCAMERAWND)->height;

			const auto cam = &cmainframe::activewnd->m_pCamWnd->camera;
			refdef.tanHalfFovY = tanf(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f;
			refdef.tanHalfFovX = refdef.tanHalfFovY * (static_cast<float>(cam->width) / static_cast<float>(cam->height));

			refdef.zNear = game::Dvar_FindVar("r_zNear")->current.value;
			refdef.time = static_cast<int>(timeGetTime());

			refdef.scissorViewport.width = components::renderer::get_window(components::renderer::CCAMERAWND)->width;
			refdef.scissorViewport.height = components::renderer::get_window(components::renderer::CCAMERAWND)->height;

			memcpy(&refdef.primaryLights, &d3dbsp::scene_lights, sizeof(d3dbsp::scene_lights));

			// CL_RenderScene
			utils::hook::call<void(__cdecl)(game::refdef_s* _refdef)>(0x506030)(&refdef);
		}
		else
		{
			// a bit of R_RenderScene

			const auto frontEndDataOut = game::get_frontenddata();
			const auto viewInfo = &frontEndDataOut->viewInfo[0];

			frontEndDataOut->viewInfoIndex = 0;
			frontEndDataOut->viewInfoCount = 1;

			memcpy(&viewInfo->input, game::gfxCmdBufInput, sizeof(viewInfo->input));
			viewInfo->input.data = frontEndDataOut;
			viewInfo->sceneDef = game::scene->def;

			memcpy(&viewInfo->viewParms, viewParms, sizeof(game::GfxViewParms));
			viewInfo->viewParms.zNear = game::Dvar_FindVar("r_zNear")->current.value;

			const auto gfx_window = components::renderer::get_window(components::renderer::CCAMERAWND);
			const game::GfxViewport viewport = { 0, 0, gfx_window->width, gfx_window->height };

			viewInfo->sceneViewport = viewport;
			viewInfo->displayViewport = viewport;

			// needed for debug plumes (3D text in space)
			game::rg->debugViewParms = viewParms;

			// R_DrawAllSceneEnt - add/draw effect xmodels 
			utils::hook::call<void(__cdecl)(game::GfxViewInfo*)>(0x523E50)(viewInfo);

			// R_AddAllSceneEntSurfacesCamera (Worker CMD) - add/draw effect xmodels 
			utils::hook::call<void(__cdecl)(game::GfxViewInfo*)>(0x523660)(viewInfo);

			// R_SortDrawSurfs
			utils::hook::call<void(__cdecl)(game::GfxDrawSurf*, signed int)>(0x54D750)(game::scene->drawSurfs[1], game::scene->drawSurfCount[1]);
			utils::hook::call<void(__cdecl)(game::GfxDrawSurf*, signed int)>(0x54D750)(game::scene->drawSurfs[4], game::scene->drawSurfCount[4]);
			utils::hook::call<void(__cdecl)(game::GfxDrawSurf*, signed int)>(0x54D750)(game::scene->drawSurfs[10], game::scene->drawSurfCount[10]);

			// *
			// lit drawlist (effect xmodels)

			renderer::R_InitDrawSurfListInfo(&viewInfo->litInfo);

			viewInfo->litInfo.baseTechType = game::TECHNIQUE_FAKELIGHT_NORMAL;
			viewInfo->litInfo.viewInfo = viewInfo;
			viewInfo->litInfo.viewOrigin[0] = viewParms->origin[0];
			viewInfo->litInfo.viewOrigin[1] = viewParms->origin[1];
			viewInfo->litInfo.viewOrigin[2] = viewParms->origin[2];
			viewInfo->litInfo.viewOrigin[3] = viewParms->origin[3];
			viewInfo->litInfo.cameraView = 1;

			const int initial_lit_drawSurfCount = frontEndDataOut->drawSurfCount;

			// R_MergeAndEmitDrawSurfLists
			utils::hook::call<void(__cdecl)(int, int)>(0x549F50)(0, 3);

			viewInfo->litInfo.drawSurfs = &frontEndDataOut->drawSurfs[initial_lit_drawSurfCount];
			viewInfo->litInfo.drawSurfCount = frontEndDataOut->drawSurfCount - initial_lit_drawSurfCount;



			// R_SortDrawSurfs
			utils::hook::call<void(__cdecl)(game::GfxDrawSurf*, signed int)>(0x54D750)(game::scene->drawSurfs[6], game::scene->drawSurfCount[6]);

			renderer::R_InitDrawSurfListInfo(&viewInfo->decalInfo);
			viewInfo->decalInfo.baseTechType = game::TECHNIQUE_FAKELIGHT_NORMAL;
			viewInfo->decalInfo.viewInfo = viewInfo;
			viewInfo->decalInfo.viewOrigin[0] = viewParms->origin[0];
			viewInfo->decalInfo.viewOrigin[1] = viewParms->origin[1];
			viewInfo->decalInfo.viewOrigin[2] = viewParms->origin[2];
			viewInfo->decalInfo.viewOrigin[3] = viewParms->origin[3];
			viewInfo->decalInfo.cameraView = 1;
			const int initial_decal_drawSurfCount = frontEndDataOut->drawSurfCount;

			// R_MergeAndEmitDrawSurfLists
			utils::hook::call<void(__cdecl)(int, int)>(0x549F50)(3, 6);

			viewInfo->decalInfo.drawSurfs = &frontEndDataOut->drawSurfs[initial_decal_drawSurfCount];
			viewInfo->decalInfo.drawSurfCount = frontEndDataOut->drawSurfCount - initial_decal_drawSurfCount;

			// R_SortDrawSurfs
			utils::hook::call<void(__cdecl)(game::GfxDrawSurf*, signed int)>(0x54D750)(game::scene->drawSurfs[12], game::scene->drawSurfCount[12]);


			// *
			// emissive drawlist (effects)

			const auto emissiveList = &viewInfo->emissiveInfo;
			renderer::R_InitDrawSurfListInfo(&viewInfo->emissiveInfo);

			viewInfo->emissiveInfo.baseTechType = game::TECHNIQUE_EMISSIVE;
			viewInfo->emissiveInfo.viewInfo = viewInfo;
			viewInfo->emissiveInfo.viewOrigin[0] = viewParms->origin[0];
			viewInfo->emissiveInfo.viewOrigin[1] = viewParms->origin[1];
			viewInfo->emissiveInfo.viewOrigin[2] = viewParms->origin[2];
			viewInfo->emissiveInfo.viewOrigin[3] = viewParms->origin[3];
			viewInfo->emissiveInfo.cameraView = 1;

			const int initial_emissive_drawSurfCount = frontEndDataOut->drawSurfCount;

			// R_MergeAndEmitDrawSurfLists
			utils::hook::call<void(__cdecl)(int, int)>(0x549F50)(9, 6);

			emissiveList->drawSurfs = &frontEndDataOut->drawSurfs[initial_emissive_drawSurfCount];

			renderer::effect_drawsurf_count_ = frontEndDataOut->drawSurfCount;

			viewInfo->emissiveInfo.drawSurfCount = frontEndDataOut->drawSurfCount - initial_emissive_drawSurfCount;
		}
	}

	void __declspec(naked) setup_viewinfo_stub()
	{
		const static uint32_t og_func_addr = 0x4FCC70;
		const static uint32_t retn_addr = 0x5064B9;
		__asm
		{
			call	og_func_addr; // R_AddClearCmd, clear the depth buffer
			add     esp, 0x10;

			pushad;
			push	ebx;
			call	setup_viewinfo;
			add		esp, 4;
			popad;

			jmp		retn_addr;
		}
	}


	// *
	// Draw3D Internal

	void R_Set3D(game::GfxCmdBufSourceState* source)
	{
		if (!source->viewParms3D)
		{
			Assert();
		}

		if (source->viewMode != game::VIEW_MODE_3D)
		{
			float eye_offset = 0.0f;

			source->viewMode = game::VIEW_MODE_3D;
			memcpy(&source->viewParms, source->viewParms3D, sizeof(source->viewParms));

			if (source->viewParms.origin[3] == 0.0f)
			{
				source->eyeOffset[0] = 0.0f;
				source->eyeOffset[1] = 0.0f;
			}
			else
			{
				source->eyeOffset[0] = source->viewParms.origin[0];
				source->eyeOffset[1] = source->viewParms.origin[1];
				eye_offset = source->viewParms.origin[2];
			}

			source->eyeOffset[2] = eye_offset;
			source->eyeOffset[3] = 1.0f;
			
			game::R_CmdBufSet3D(source);
		}
	}

	void R_SetViewParms(game::GfxCmdBufSourceState* source)
	{
		if (source->viewParms.inverseViewProjectionMatrix.m[3][3] * 0.000009999999f <= fabs(source->viewParms.inverseViewProjectionMatrix.m[0][3]))
		{
			Assert();
		}

		if (source->viewParms.inverseViewProjectionMatrix.m[3][3] * 0.000009999999f <= fabs(source->viewParms.inverseViewProjectionMatrix.m[1][3]))
		{
			Assert();
		}

		if (source->viewParms.inverseViewProjectionMatrix.m[3][3] == 0.0f)
		{
			Assert();
		}

		const float near_org = 1.0f / source->viewParms.inverseViewProjectionMatrix.m[3][3];

		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_ORG][0] = source->viewParms.inverseViewProjectionMatrix.m[3][0] * near_org - source->viewParms.origin[0];
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_ORG][1] = source->viewParms.inverseViewProjectionMatrix.m[3][1] * near_org - source->viewParms.origin[1];
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_ORG][2] = source->viewParms.inverseViewProjectionMatrix.m[3][2] * near_org - source->viewParms.origin[2];
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_ORG][3] = 0.0f;
		++source->constVersions[5];

		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DX][0] = source->viewParms.inverseViewProjectionMatrix.m[0][0] * (near_org + near_org);
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DX][1] = source->viewParms.inverseViewProjectionMatrix.m[0][1] * (near_org + near_org);
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DX][2] = source->viewParms.inverseViewProjectionMatrix.m[0][2] * (near_org + near_org);
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DX][3] = 0.0f;
		++source->constVersions[6];

		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DY][0] = source->viewParms.inverseViewProjectionMatrix.m[1][0] * -(near_org + near_org);
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DY][1] = source->viewParms.inverseViewProjectionMatrix.m[1][1] * -(near_org + near_org);
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DY][2] = source->viewParms.inverseViewProjectionMatrix.m[1][2] * -(near_org + near_org);
		source->input.consts[game::CONST_SRC_CODE_NEARPLANE_DY][3] = 0.0f;
		++source->constVersions[7];
	}

	void R_BeginView(game::GfxCmdBufSourceState* source, game::GfxSceneDef* scene, game::GfxViewInfo* viewInfo)
	{
		source->sceneDef = *scene;
		source->viewParms3D = &viewInfo->viewParms; // viewParms3D gets copied into viewParms inside Set3D
		source->skinnedPlacement.base.origin[0] = scene->viewOffset[0];
		source->skinnedPlacement.base.origin[1] = scene->viewOffset[1];
		source->skinnedPlacement.base.origin[2] = scene->viewOffset[2];
		source->viewMode = game::VIEW_MODE_NONE;

		R_Set3D(source);
		source->materialTime = 0.0f;
		
		game::R_SetGameTime(source, source->sceneDef.floatTime);
		R_SetViewParms(source);
	}

	void R_SetSceneViewport(game::GfxCmdBufSourceState* source, game::GfxViewport* viewport)
	{
		if (viewport->width <= 0)
		{
			AssertS("viewport->width");
		}

		if (viewport->height <= 0)
		{
			AssertS("viewport->height");
		}

		source->sceneViewport = *viewport;
		source->viewMode = game::VIEW_MODE_NONE;
		source->viewportIsDirty = true;
	}

	void RB_EndSceneRendering(game::GfxCmdBufSourceState* source, [[maybe_unused]] game::GfxCmdBufState* state, [[maybe_unused]] game::GfxCmdBufInput* input, [[maybe_unused]] game::GfxViewInfo* viewInfo)
	{
		// R_HW_InsertFence(&backEndData->endFence);
		utils::hook::call<void(__cdecl)(IDirect3DQuery9** fence)>(0x530B30)(&game::get_backenddata()->endFence);

		// R_InitCmdBufSourceState(source, input, 0);
		game::R_InitCmdBufSourceState(source, input, 0);

		memset(state->vertexShaderConstState, 0, sizeof(state->vertexShaderConstState));
		memset(state->pixelShaderConstState, 0, sizeof(state->pixelShaderConstState));

		game::R_SetupRendertarget(source, game::R_RENDERTARGET_FRAME_BUFFER);

		//R_BeginView(source, &viewInfo->sceneDef, viewInfo);
		utils::hook::call<void(__cdecl)(game::GfxCmdBufSourceState*, game::GfxSceneDef*, game::GfxViewInfo*)>(0x53D2F0)(source, &viewInfo->sceneDef, viewInfo);

		R_SetSceneViewport(source, &viewInfo->sceneViewport);
		game::R_SetRenderTarget(source, state, game::R_RENDERTARGET_FRAME_BUFFER);

		if (!dvars::r_reflectionprobe_generate->current.enabled)
		{
			// developer
			R_Set3D(source);

			//RB_DrawDebug(&gfxCmdBufSourceState.viewParms);
			utils::hook::call<void(__cdecl)(game::GfxViewParms*)>(0x56D420)(&source->viewParms);
		}
	}

	void renderer::R_SetAndClearSceneTarget(bool clear)
	{
		const auto buf_source_state = game::gfxCmdBufSourceState;
		const auto buf_state = game::gfxCmdBufState;

		memset(buf_state->vertexShaderConstState, 0, sizeof(buf_state->vertexShaderConstState));
		memset(buf_state->pixelShaderConstState, 0, sizeof(buf_state->pixelShaderConstState));
		
		game::R_SetupRendertarget(buf_source_state, game::R_RENDERTARGET_FRAME_BUFFER);
		game::R_SetRenderTarget(buf_source_state, buf_state, game::R_RENDERTARGET_FRAME_BUFFER);

		if(clear)
		{
			game::R_ClearScreen(buf_state->prim.device, 7, game::g_qeglobals->d_savedinfo.colors[4], 1.0f, false, nullptr);
		}
	}

	void R_DepthPrepassCallback(game::GfxViewInfo* viewInfo, game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		R_Set3D(source);
		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1);

		const RECT rect =
		{
			viewInfo->sceneViewport.x,
			viewInfo->sceneViewport.y,
			viewInfo->sceneViewport.x + viewInfo->sceneViewport.width,
			viewInfo->sceneViewport.y + viewInfo->sceneViewport.height
		};

		state->prim.device->SetScissorRect(&rect);

		game::MaterialTechniqueType tech_type;

		if (viewInfo->needsFloatZ)
		{
			game::R_SetRenderTarget(source, state, game::R_RENDERTARGET_FLOAT_Z);
			tech_type = game::TECHNIQUE_BUILD_FLOAT_Z;

			//R_DrawQuadMesh(source, state, game::rgp->shadowClearMaterial, &viewInfo->fullSceneViewMesh->meshData);
			utils::hook::call<void(__cdecl)(game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::Material*, game::GfxMeshData*)>(0x52B9F0)
				(source, state, game::rgp->shadowClearMaterial, &viewInfo->fullSceneViewMesh->meshData);

			source->input.consts[game::CONST_SRC_CODE_DEPTH_FROM_CLIP][0] = 0.0f;
			source->input.consts[game::CONST_SRC_CODE_DEPTH_FROM_CLIP][1] = 0.0f;
			source->input.consts[game::CONST_SRC_CODE_DEPTH_FROM_CLIP][2] = 0.0f;
			source->input.consts[game::CONST_SRC_CODE_DEPTH_FROM_CLIP][3] = 1.0f;
			++source->constVersions[54];
		}
		else
		{
			game::R_SetRenderTarget(source, state, game::R_RENDERTARGET_DYNAMICSHADOWS);
			tech_type = game::TECHNIQUE_DEPTH_PREPASS;
		}

		game::GfxDrawSurfListInfo info;

		memcpy(&info, &viewInfo->litInfo, sizeof(info));
		info.baseTechType = tech_type;
		game::R_DrawSurfs(source, state, nullptr, &info);

		memcpy(&info, &viewInfo->decalInfo, sizeof(info));
		info.baseTechType = tech_type;
		game::R_DrawSurfs(source, state, nullptr, &info);

		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
	}

	void R_DrawLitCallback(game::GfxViewInfo* viewInfo, game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		game::R_SetRenderTarget(source, state, game::R_RENDERTARGET_FRAME_BUFFER);

		R_Set3D(source);
		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1);

		const RECT rect =
		{
			viewInfo->sceneViewport.x,
			viewInfo->sceneViewport.y,
			viewInfo->sceneViewport.x + viewInfo->sceneViewport.width,
			viewInfo->sceneViewport.y + viewInfo->sceneViewport.height
		};

		state->prim.device->SetScissorRect(&rect);

		game::R_DrawSurfs(source, state, nullptr, &viewInfo->litInfo);
		//game::R_ShowTris(source, state, &viewInfo->emissiveInfo);

		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
	}

	void R_DrawDecalCallback(game::GfxViewInfo* viewInfo, game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		game::R_SetRenderTarget(source, state, game::R_RENDERTARGET_FRAME_BUFFER);

		R_Set3D(source);
		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1);

		const RECT rect =
		{
			viewInfo->sceneViewport.x,
			viewInfo->sceneViewport.y,
			viewInfo->sceneViewport.x + viewInfo->sceneViewport.width,
			viewInfo->sceneViewport.y + viewInfo->sceneViewport.height
		};

		state->prim.device->SetScissorRect(&rect);

		game::R_DrawSurfs(source, state, nullptr, &viewInfo->decalInfo);
		//game::R_ShowTris(source, state, &viewInfo->decalInfo);

		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
	}

	void R_DrawEmissiveCallback(game::GfxViewInfo* viewInfo, game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state)
	{
		game::R_SetRenderTarget(source, state, game::R_RENDERTARGET_FRAME_BUFFER);

		R_Set3D(source);
		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1);

		const RECT rect =
		{
			viewInfo->sceneViewport.x,
			viewInfo->sceneViewport.y,
			viewInfo->sceneViewport.x + viewInfo->sceneViewport.width,
			viewInfo->sceneViewport.y + viewInfo->sceneViewport.height
		};

		state->prim.device->SetScissorRect(&rect);

		//auto& rg = *reinterpret_cast<game::r_globals_t*>(0x13683F0);

		renderer::effect_drawsurf_count_ = viewInfo->emissiveInfo.drawSurfCount;

		game::R_DrawSurfs(source, state, nullptr, &viewInfo->emissiveInfo);
		game::R_ShowTris(source, state, &viewInfo->emissiveInfo);

		state->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
	}

	void R_DrawCall(void(__cdecl* callback)(game::GfxViewInfo*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*), game::GfxViewInfo* viewInfo, game::GfxCmdBufSourceState* source, game::GfxViewInfo* viewInfo2, [[maybe_unused]] game::GfxDrawSurfListInfo* listinfo, game::GfxViewInfo* viewinfo3, [[maybe_unused]] game::GfxCmdBuf* cmdbuf, [[maybe_unused]] int prepass)
	{
		game::GfxCmdBufState state1 = {};

		R_BeginView(source, &viewInfo2->sceneDef, viewinfo3);

		memcpy(&state1, game::gfxCmdBufState, sizeof(state1));
		memset(state1.vertexShaderConstState, 0, sizeof(state1.vertexShaderConstState));
		memset(state1.pixelShaderConstState, 0, sizeof(state1.pixelShaderConstState));

		callback(viewInfo, source, &state1, nullptr, nullptr);

		memcpy(game::gfxCmdBufState, &state1, sizeof(game::GfxCmdBufState));
	}

	void R_DepthPrepass(game::GfxCmdBuf* cmdbuf, game::GfxViewInfo* viewinfo)
	{
		game::GfxCmdBufSourceState source = {};
		game::R_InitCmdBufSourceState(&source, &viewinfo->input, 1);

		

		game::R_SetupRendertarget(&source, game::R_RENDERTARGET_FLOAT_Z);

		//game::R_ClearScreen(cmdbuf->device, 6, game::color_white, 1.0f, false, nullptr);

		// R_SetSceneViewport
		source.sceneViewport = viewinfo->sceneViewport;
		source.viewMode = game::VIEW_MODE_NONE;
		source.viewportIsDirty = true;

		R_DrawCall((void(__cdecl*)(game::GfxViewInfo*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*))R_DepthPrepassCallback, viewinfo, &source, viewinfo, nullptr, viewinfo, cmdbuf, 0);
	}

	void R_DrawLit(game::GfxCmdBuf* cmdbuf, game::GfxViewInfo* viewinfo)
	{
		game::GfxCmdBufSourceState source = {};
		game::R_InitCmdBufSourceState(&source, &viewinfo->input, 1);
		game::R_SetupRendertarget(&source, game::R_RENDERTARGET_FRAME_BUFFER); //dest_rendertarget);

		// R_SetSceneViewport
		source.sceneViewport = viewinfo->sceneViewport;
		source.viewMode = game::VIEW_MODE_NONE;
		source.viewportIsDirty = true;

		R_DrawCall((void(__cdecl*)(game::GfxViewInfo*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*))R_DrawLitCallback, viewinfo, &source, viewinfo, &viewinfo->litInfo, viewinfo, cmdbuf, 0);
	}

	void R_DrawDecal(game::GfxCmdBuf* cmdbuf, game::GfxViewInfo* viewinfo)
	{
		game::GfxCmdBufSourceState source = {};
		game::R_InitCmdBufSourceState(&source, &viewinfo->input, 1);
		game::R_SetupRendertarget(&source, game::R_RENDERTARGET_FRAME_BUFFER); //dest_rendertarget);

		// R_SetSceneViewport
		source.sceneViewport = viewinfo->sceneViewport;
		source.viewMode = game::VIEW_MODE_NONE;
		source.viewportIsDirty = true;

		R_DrawCall((void(__cdecl*)(game::GfxViewInfo*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*))R_DrawDecalCallback, viewinfo, &source, viewinfo, &viewinfo->decalInfo, viewinfo, cmdbuf, 0);
	}

	void R_DrawPointLitSurfs(game::GfxViewInfo* viewinfo /*esi*/, game::GfxCmdBufSourceState* source, game::GfxCmdBuf* cmdbuf)
	{
		const static uint32_t func_addr = 0x55BD40;
		__asm
		{
			pushad;
			push	cmdbuf;
			push	source;
			mov		esi, viewinfo;
			call	func_addr;
			add		esp, 8;
			popad;
		}
	}

	void R_DrawLights(game::GfxCmdBuf* cmdbuf, game::GfxViewInfo* viewinfo)
	{
		game::GfxCmdBufSourceState source = {};
		game::R_InitCmdBufSourceState(&source, &viewinfo->input, 1);
		game::R_SetupRendertarget(&source, game::R_RENDERTARGET_FRAME_BUFFER);

		// R_SetSceneViewport
		source.viewportBehavior = game::GFX_USE_VIEWPORT_FOR_VIEW;
		source.sceneViewport = viewinfo->sceneViewport;
		source.viewMode = game::VIEW_MODE_NONE;
		source.viewportIsDirty = true;

		//auto x = game::rg;
		//auto scene = game::scene;

		R_DrawPointLitSurfs(viewinfo, &source, cmdbuf);
	}

	void R_DrawEmissive(game::GfxCmdBuf* cmdbuf, game::GfxViewInfo* viewinfo)
	{
		game::GfxCmdBufSourceState source = {};
		game::R_InitCmdBufSourceState(&source, &viewinfo->input, 1);
		game::R_SetupRendertarget(&source, game::R_RENDERTARGET_FRAME_BUFFER); //dest_rendertarget);

		// R_SetSceneViewport
		source.sceneViewport = viewinfo->sceneViewport;
		source.viewMode = game::VIEW_MODE_NONE;
		source.viewportIsDirty = true;

		R_DrawCall((void(__cdecl*)(game::GfxViewInfo*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*, game::GfxCmdBufSourceState*, game::GfxCmdBufState*))R_DrawEmissiveCallback, viewinfo, &source, viewinfo, &viewinfo->emissiveInfo, viewinfo, cmdbuf, 0);
	}

	// render bsp and effects
	void RB_StandardDrawCommands(game::GfxViewInfo* viewInfo)
	{
		game::GfxCmdBuf cmdBuf = { game::dx->device };

		if (game::dx->device && (renderer::is_rendering_camerawnd() && 
			(effects::effect_is_playing() || fx_system::ed_is_paused && !effects::effect_is_playing() || dvars::r_draw_bsp && dvars::r_draw_bsp->current.enabled)
				|| renderer::is_rendering_effectswnd()))
		{
			// setup resolvedPostSun sampler (effect distortion)
			game::GfxRenderTarget* targets = reinterpret_cast<game::GfxRenderTarget*>(0x174F4A8);
			game::GfxRenderTarget* resolved_post_sun = &targets[game::R_RENDERTARGET_RESOLVED_POST_SUN];

			if (dvars::fx_browser_use_camera_for_distortion && dvars::fx_browser_use_camera_for_distortion->current.enabled)
			{
				renderer::copy_scene_to_texture(renderer::CCAMERAWND, reinterpret_cast<IDirect3DTexture9*&>(resolved_post_sun->image->texture.data));
			}
			else
			{
				renderer::copy_scene_to_texture(renderer::is_rendering_effectswnd() ? renderer::CFXWND : renderer::CCAMERAWND, reinterpret_cast<IDirect3DTexture9*&>(resolved_post_sun->image->texture.data));
			}
			
			game::gfxCmdBufSourceState->input.codeImages[10] = resolved_post_sun->image;

			// ------

			const auto backend = game::get_backenddata();
			const auto dyn_shadow_type = viewInfo->dynamicShadowType;

			const auto r_fullbright = game::Dvar_FindVar("r_fullbright");
			const auto r_debugshader = game::Dvar_FindVar("r_debugshader");

			if (!renderer::is_rendering_effectswnd())
			{
				if (!(r_fullbright && r_fullbright->current.enabled) && !(r_debugshader && r_debugshader->current.enabled))
				{
					if (dvars::r_draw_bsp->current.enabled)
					{
						if (dyn_shadow_type == game::SHADOW_MAP)
						{
							if (game::Com_BitCheckAssert(backend->shadowableLightHasShadowMap, game::rgp->world->sunPrimaryLightIndex, 32))
							{
								game::RB_SunShadowMaps(backend, viewInfo);
							}

							game::RB_SpotShadowMaps(backend, viewInfo);
						}
					}
				}
			}

			//R_DepthPrepass(&cmdBuf, viewInfo);	// no need to do a depth prepass, only causes issues upon resizing
													// needs depthbuffer resize logic

			if (viewInfo->litInfo.viewInfo && viewInfo->litInfo.drawSurfs)
			{
				R_DrawLit(&cmdBuf, viewInfo);
			}

			if (viewInfo->decalInfo.viewInfo && viewInfo->decalInfo.drawSurfs)
			{
				R_DrawDecal(&cmdBuf, viewInfo);
			}			

			// impl. RB_DrawSun? :p
			if (!renderer::is_rendering_effectswnd() && dvars::r_draw_bsp->current.enabled)
			{
				R_DrawLights(&cmdBuf, viewInfo); // fx lights
			}

			if (viewInfo->emissiveInfo.viewInfo && viewInfo->emissiveInfo.drawSurfs)
			{
				R_DrawEmissive(&cmdBuf, viewInfo);
			}
		}

		RB_EndSceneRendering(game::gfxCmdBufSourceState, game::gfxCmdBufState, &viewInfo->input, viewInfo);
	}

	void renderer::RB_Draw3D()
	{
		const auto backend = game::get_backenddata();

		if (backend->viewInfo->displayViewport.width == 0)
		{
			return;
		}

		if (backend->viewInfoCount)
		{
			RB_StandardDrawCommands(&backend->viewInfo[0]);
		}
	}

	// -------------------------------------------------------------------------------

	constexpr unsigned int TESS_INDICES_AMOUNT = 1048576;
	unsigned __int16 tess_indices_reloc[TESS_INDICES_AMOUNT] = {}; // og: 32704 // iw3: 1048576

#if 0 // working fine but no need when we can use the original (here to stay for debugging purposes)
	void RB_BeginSurface(game::Material* material, game::MaterialTechniqueType tech)
	{
		if (tess.indexCount || tess.vertexCount || !material)
		{
			Assert();
		}

		tess.firstVertex = 0;
		tess.lastVertex = 0;

		game::gfxCmdBufState->material = material;
		game::gfxCmdBufState->techType = tech;
		game::gfxCmdBufState->prim.vertDeclType = game::VERTDECL_GENERIC;
		game::gfxCmdBufState->origMaterial = material;
		game::gfxCmdBufState->origTechType = tech;

		if (pixelCostMode > 2)
		{
			//R_PixelCost_GetAccumulationMaterial(material);
			game::gfxCmdBufState->material = utils::hook::call<game::Material* (__cdecl)(game::Material*)>(0x542A10)(material);
			game::gfxCmdBufState->techType = game::TECHNIQUE_UNLIT;
		}

		// Material_GetTechnique(mtl, v2);
		game::gfxCmdBufState->technique = utils::hook::call<game::MaterialTechnique* (__cdecl)(game::Material*, game::MaterialTechniqueType)>(0x4FA720)(game::gfxCmdBufState->material, game::gfxCmdBufState->techType);
		if (!game::gfxCmdBufState->technique)
		{
			Assert();
		}
	}

	// this is how radiant is drawing meshes and models
	void RB_DrawEditorSkinnedCached(int index, int amount)
	{
		editorMesh_s* mesh; // esi
		game::Material* material; // ebx
		int index_count; // eax
		bool v9; // zf
		int v10; // eax
		game::GfxMatrix* v11; // edi
		__int16 v12; // cx
		unsigned __int16 mesh_vertcount; // ax
		int v14; // eax
		game::GfxMatrix* v15; // esi
		int v16; // [esp+Ch] [ebp-20h]
		int firstIndex; // [esp+10h] [ebp-1Ch] BYREF
		editorSurf_s* v19; // [esp+18h] [ebp-14h]
		game::MaterialTechniqueType tech_type; // [esp+1Ch] [ebp-10h]
		GfxModelSkinnedSurface* mSurf; // [esp+34h] [ebp+8h]
		editorSurf_sub* amounta; // [esp+38h] [ebp+Ch]

		editorSurf_sub* modelsurf = 0;
		if (tess.indexCount)
		{
			//RB_EndTessSurface();
			utils::hook::call<void(__cdecl)()>(0x53ADC0)();
			
		}

		R_Set3D(game::gfxCmdBufSourceState);

		editorSurf_s* surf = &edSceneGlobals_sceneSurfaces[index];

		int v21 = 0xFFFF;
		int vertcount = 0;
		IDirect3DVertexBuffer9* vb = nullptr;
		IDirect3DVertexBuffer9* vb_x = nullptr;
		v19 = surf;
		if (amount > 0)
		{
			v16 = amount;
			while (1)
			{
				if (surf->type == ED_SURF_MESH)
				{
					mesh = surf->mesh_or_surfSub;
					

					//editorVB_GetBufferAndIndex(mesh->low16_firstIndex__high16_vbOffset, &vb, &firstIndex);
					utils::hook::call<void(__cdecl)(unsigned int offset, DWORD* vb, int* firstIndex)>(0x51CCE0)(mesh->low16_firstIndex__high16_vbOffset, (DWORD*)&vb, &firstIndex);

					material = mesh->material;
					tech_type = (game::MaterialTechniqueType)mesh->techType;


					index_count = static_cast<int>(mesh->indexCount);
					mSurf = nullptr;
					amounta = nullptr;
				}
				else
				{
					if (surf->type != ED_SURF_MODEL)
					{
						Assert();
					}

					vb = nullptr;
					firstIndex = 0;
					mSurf = (GfxModelSkinnedSurface*)surf->mesh_or_surfSub->low16_firstIndex__high16_vbOffset; // prob a union 
					material = surf->mesh_or_surfSub->material;
					mesh = nullptr;
					amounta = (editorSurf_sub*)surf->mesh_or_surfSub;
					tech_type = (game::MaterialTechniqueType)surf->mesh_or_surfSub->techType;

					// RIGID_SKINNED_CACHE_OFFSET
					if (mSurf->skinnedCachedOffset == -2)
					{
						Assert();
					}

					// HIDDEN_SURFACE_OFFSET
					if (mSurf->skinnedCachedOffset == -3)
					{
						Assert();
					}

					modelsurf = amounta;
					index_count = 3 * mSurf->xsurf->triCount; //XSurfaceGetNumTris(v8->xsurf);
				}

				if (   vb != vb_x 
					|| material != game::gfxCmdBufState->material 
					|| tech_type != game::gfxCmdBufState->techType 
					|| index_count + tess.indexCount > 32704)
				{

					if (vb_x)
					{
						//RB_DrawTessSurface(v21, vertcount);
						utils::hook::call<void(__cdecl)(unsigned __int16 vertCount, unsigned __int16 index)>(0x4FE690)(v21, vertcount);
						
						v21 = 0xFFFF;
						vertcount = 0;
					}
					else
					{
						//v11 = R_GetActiveWorldMatrix(&game::gfxCmdBufSourceState);// v11 = first mtx in gfxCmdBufSourceState ?
						v11 = utils::hook::call<game::GfxMatrix* (__cdecl)(game::GfxCmdBufSourceState*)>(0x53CD70)(game::gfxCmdBufSourceState);

						//MatrixIdentity44(v11);
						utils::hook::call<void(__cdecl)(game::GfxMatrix*)>(0x4A5BC0)(v11);
						

						v11->m[3][0] = v11->m[3][0] - game::gfxCmdBufSourceState->eyeOffset[0];
						v11->m[3][1] = v11->m[3][1] - game::gfxCmdBufSourceState->eyeOffset[1];
						v11->m[3][2] = v11->m[3][2] - game::gfxCmdBufSourceState->eyeOffset[2];
						modelsurf = amounta;
					}

					RB_BeginSurface(material, tech_type);
					if (vb)
					{
						game::gfxCmdBufState->prim.vertDeclType = game::VERTDECL_WORLD;

						if (game::gfxCmdBufState->prim.streams[0].vb != vb || game::gfxCmdBufState->prim.streams[0].offset || game::gfxCmdBufState->prim.streams[0].stride != 44)
						{
							//R_ChangeStreamSource(&game::gfxCmdBufState->prim, 0, vb, 0, 44);
							utils::hook::call<void(__cdecl)(game::GfxCmdBufPrimState*, int, IDirect3DVertexBuffer9*, int, int)>(0x4FD780)(&game::gfxCmdBufState->prim, 0, vb, 0, 44);
						}

						if (game::gfxCmdBufState->prim.streams[1].vb || game::gfxCmdBufState->prim.streams[1].offset || game::gfxCmdBufState->prim.streams[1].stride)
						{
							//R_ChangeStreamSource(&game::gfxCmdBufState->prim, 1, 0, 0, 0);
							utils::hook::call<void(__cdecl)(game::GfxCmdBufPrimState*, int, IDirect3DVertexBuffer9*, int, int)>(0x4FD780)(&game::gfxCmdBufState->prim, 1, 0, 0, 0);
						}
					}
					else
					{
						game::gfxCmdBufState->prim.vertDeclType = game::VERTDECL_PACKED;
					}

					vb_x = vb;
				}

				if (mesh)
				{
					if (modelsurf)
					{
						Assert();
					}

					v12 = firstIndex;
					if ((unsigned __int16)v21 > (unsigned __int16)firstIndex)
					{
						v21 = (unsigned __int16)firstIndex;
					}

					mesh_vertcount = mesh->vertexCount;
					if ((unsigned __int16)vertcount < mesh_vertcount + (unsigned __int16)firstIndex - 1)
					{
						vertcount = (unsigned __int16)(mesh_vertcount + firstIndex - 1);
					}

					v14 = 0;
					if (mesh->indexCount)
					{
						do
						{
							tess_indices_reloc[v14 + tess.indexCount] = v12 + *(WORD*)(mesh->unk3 + 2 * v14);
							//tess.indices[v14 + tess.indexCount] = v12 + *(WORD*)(mesh->unk3 + 2 * v14);
							++v14;

						} while (v14 < (unsigned __int16)mesh->indexCount);
					}

					tess.indexCount += (unsigned __int16)mesh->indexCount;
				}
				else
				{
					if (!modelsurf)
					{
						Assert();
					}

					if (!modelsurf->skinnedSurf)
					{
						Assert();
					}

					if (tess.indexCount)
					{
						Assert();
					}

					if (tess.vertexCount)
					{
						Assert();
					}

					// RIGID_SKINNED_CACHE_OFFSET
					if (mSurf->skinnedCachedOffset == -2)
					{
						Assert();
					}

					// HIDDEN_SURFACE_OFFSET
					if (mSurf->skinnedCachedOffset == -3)
					{
						Assert();
					}

					if (game::gfxCmdBufSourceState->objectPlacement != modelsurf->placement)
					{
						//R_ChangeObjectPlacement(&game::gfxCmdBufSourceState, modelsurf->placement);
						utils::hook::call<void(__cdecl)(game::GfxCmdBufSourceState*, game::GfxScaledPlacement*)>(0x537C40)(game::gfxCmdBufSourceState, modelsurf->placement);
						
					}

					//R_DrawXModelSkinnedUncached_2(indexa->xsurf, indexa->___u3.skinnedVert);
					utils::hook::call<void(__cdecl)(game::XSurface*, game::GfxPackedVertex*)>(0x53AA30)(mSurf->xsurf, mSurf->___u3.skinnedVert);

					// ^ above function does not check amount of indices before memcpy so it can corrupt data past the indices array boundaries
					// dirty hack but works
					auto x = tess.indexCount; //  = 0;
					auto y = tess.vertexCount; // = 0;

					game::gfxCmdBufSourceState->objectPlacement = 0;
				}
				++v19;
				if (!--v16)
				{
					break;
				}

				surf = v19;
				modelsurf = nullptr;
			}

			if (vb)
			{
				//RB_DrawTessSurface(v21, vertcount);
				utils::hook::call<void(__cdecl)(unsigned __int16 vertCount, unsigned __int16 index)>(0x4FE690)(v21, vertcount);
			}
		}

		game::gfxCmdBufState->prim.vertDeclType = game::VERTDECL_GENERIC;

		//v15 = R_GetActiveWorldMatrix(&game::gfxCmdBufSourceState);
		v15 = utils::hook::call<game::GfxMatrix*(__cdecl)(game::GfxCmdBufSourceState*)>(0x53CD70)(game::gfxCmdBufSourceState);
		

		//MatrixIdentity44(v15);
		utils::hook::call<void(__cdecl)(game::GfxMatrix*)>(0x4A5BC0)(v15);

		//game::gfxCmdBufState->prim.device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);

		v15->m[3][0] = v15->m[3][0] - game::gfxCmdBufSourceState->eyeOffset[0];
		v15->m[3][1] = v15->m[3][1] - game::gfxCmdBufSourceState->eyeOffset[1];
		v15->m[3][2] = v15->m[3][2] - game::gfxCmdBufSourceState->eyeOffset[2];
	}
#endif

	void draw_additional_debug()
	{
		if (game::glob::debug_sundir)
		{
			if (const auto	world_ent = game::g_world_entity();
							world_ent && world_ent->firstActive)
			{
				for (auto epair = world_ent->firstActive->epairs; epair; epair = epair->next)
				{
					if (utils::string_equals(epair->key, "sundirection"))
					{
						const std::vector<std::string> value_str = utils::explode(epair->value, ' ');

						if (value_str.size() != 3)
						{
							break;
						}

						game::vec3_t sun_dir = {};
						sun_dir[0] = utils::try_stof(value_str[0], true);
						sun_dir[1] = utils::try_stof(value_str[1], true);
						sun_dir[2] = utils::try_stof(value_str[0], true);

						game::vec3_t end_pt = {};
						utils::vector::angle_vectors(sun_dir, end_pt, nullptr, nullptr);
						utils::vector::scale(end_pt, game::glob::debug_sundir_length, end_pt);

						game::vec4_t color_start = { 1.0f, 0.4f, 0.1f, 1.0f };
						game::vec4_t color_end = { 0.85f, 0.75f, 0.25f, 1.0f };

						game::GfxPointVertex pts[2] = {};

						game::Byte4PackPixelColor(color_start, &pts[0].color);
						utils::vector::copy(game::glob::debug_sundir_startpos, pts[0].xyz);						

						game::Byte4PackPixelColor(color_end, &pts[1].color);

						utils::vector::add(game::glob::debug_sundir_startpos, end_pt, end_pt);
						utils::vector::copy(end_pt, pts[1].xyz);

						renderer::R_AddLineCmd(1, 4, 3, pts);
					}
				}
			}
		}

		mesh_painter::on_frame();
		GET_GUI(ggui::entity_info)->render_hovered();
	}

	// *

	const int debug_box_edge_pairs[12][2] =
	{
		{0, 1}, {0, 2}, {0, 4}, {1, 3},
		{1, 5}, {2, 3}, {2, 6}, {3, 7},
		{4, 5}, {4, 6}, {5, 7}, {6, 7},
	};

	const int debug_box_edges[23] =
	{
		1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7,
	};

	void renderer::add_debug_box(const float* origin, const float* mins, const float* maxs, float yaw, float size_offset, bool depth_test_override, bool depth_test_value)
	{
		float v[8][3];

		const float fCos = cosf(yaw * 0.017453292f);
		const float fSin = sinf(yaw * 0.017453292f);

		for (auto i = 0u; i < 8; ++i)
		{
			for (auto j = 0u; j < 3; ++j)
			{
				float val;
				if ((i & (1 << j)) != 0)
				{
					val = maxs[j] + size_offset;
				}
				else
				{
					val = mins[j] - size_offset;
				}
				v[i][j] = val;
			}

			v[i][0] = (v[i][0] * fCos) - (v[i][1] * fSin) + origin[0];
			v[i][1] = (v[i][0] * fSin) + (v[i][1] * fCos) + origin[1];
			v[i][2] += origin[2];
		}

		const bool old_depth_value = components::renderer::get()->get_line_depthtesting();
		if (depth_test_override)
		{
			components::renderer::get()->set_line_depthtesting(depth_test_value);
		}

		for (auto ia = 0u; ia < 12; ++ia)
		{
			game::GfxPointVertex vert[2];
			vert[0].xyz[0] = v[debug_box_edge_pairs[ia][0]][0];
			vert[0].xyz[1] = v[debug_box_edge_pairs[ia][0]][1];
			vert[0].xyz[2] = v[debug_box_edge_pairs[ia][0]][2];
			vert[0].color.packed = (unsigned)physx::PxDebugColor::eARGB_RED;

			vert[1].xyz[0] = v[debug_box_edges[2 * ia]][0];
			vert[1].xyz[1] = v[debug_box_edges[2 * ia]][1];
			vert[1].xyz[2] = v[debug_box_edges[2 * ia]][2];
			vert[1].color.packed = (unsigned)physx::PxDebugColor::eARGB_RED;

			components::renderer::R_AddLineCmd(1, 2, 3, vert);
		}

		if (depth_test_override)
		{
			components::renderer::get()->set_line_depthtesting(old_depth_value);
		}
	}

	// *

	void set_line_depth_testing_helper_hack(bool val)
	{
		renderer::get()->set_line_depthtesting(val);

		// # HACK
		// constantly add a RC_DRAW_TRIANGLES cmd to fix depth-testing on selection outlines
		// (fails when static meshes + a light entity or trigger radius .. is in view)

		const float verts[4][4] =
		{
			{ 0.0f, 0.0f, 10000.0f, 1.0f },
			{ 0.0f, 1.0f, 10000.0f, 1.0f },
			{ 1.0f, 1.0f, 10000.0f, 1.0f },
			{ 1.0f, 0.0f, 10000.0f, 1.0f },
		};

		game::R_DrawSelectionbox(verts[0]);
	}

	// spot where a depthbuffer clear command would be added
	void __declspec(naked) set_line_depth_testing()
	{
		// disable depth testing for outlines
		const static uint32_t retn_addr = 0x4084D7;
		__asm
		{
			pushad;
			push	0;
			call	set_line_depth_testing_helper_hack;
			add		esp, 4;
			popad;

			jmp		retn_addr;
		}
	}

	void set_line_depth_testing_helper(bool val)
	{
		renderer::get()->set_line_depthtesting(val);
	}

	void __declspec(naked) set_line_depth_testing_2()
	{
		// enable depth testing for connection lines
		const static uint32_t draw_target_connection_lines_func = 0x46A2C0;
		const static uint32_t retn_addr = 0x40CC26;
		__asm
		{
			pushad;
			push	1;
			call	set_line_depth_testing_helper;
			add		esp, 4;
			popad;

			pushad;
			call	draw_target_connection_lines_func;
			call	draw_additional_debug;
			popad;

			jmp		retn_addr;
		}
	}

	// *

	void light_selection_tint()
	{
		game::R_SetMaterialColor(game::g_qeglobals->d_savedinfo.colors[11]);
	}

	void __declspec(naked) light_selection_tint_stub()
	{
		// enable depth testing for connection lines
		const static uint32_t func_addr = 0x4FD910;
		const static uint32_t retn_addr = 0x408302;
		__asm
		{
			call	func_addr;
			pushad;
			call	light_selection_tint;
			popad;
			jmp		retn_addr;
		}
	}

	void light_selection_tint_reset()
	{
		game::R_SetMaterialColor(nullptr);
	}

	void __declspec(naked) light_selection_tint_reset_stub()
	{
		const static uint32_t func_addr = 0x4FD910;
		const static uint32_t retn_addr = 0x4083BE;
		__asm
		{
			call	func_addr;
			pushad;
			call	light_selection_tint_reset;
			popad;
			jmp		retn_addr;
		}
	}

	// *

	// add depth_test
	struct GfxCmdDrawLines
	{
		game::GfxCmdHeader header;
		std::uint16_t lineCount;	// 0x4
		char width;					// 0x6
		char dimensions;			// 0x7
		bool depth_test;			// 0x8
		char pad[3];
		game::GfxPointVertex verts[2];	// 0xC (12)
	};

	// rewrite to add depth_test functionality (set 'g_line_depth_testing')
	void renderer::R_AddLineCmd(const std::uint16_t count, const char width, const char dimension, const game::GfxPointVertex* verts)
	{
		if (count <= 0)
		{
			Assert();
		}

		const game::GfxCmdArray* s_cmdList = reinterpret_cast<game::GfxCmdArray*>(*(DWORD*)0x73D4A0);
		const auto merged_cmd = reinterpret_cast<GfxCmdDrawLines*>(s_cmdList->lastCmd);

		if (merged_cmd && merged_cmd->header.id == game::RC_DRAW_LINES
			&& (count * sizeof(game::GfxPointVertex) * 2) + (unsigned int)merged_cmd->header.byteCount <= 0xFFFF 
			&& merged_cmd->width == width 
			&& merged_cmd->dimensions == dimension 
			&& count + merged_cmd->lineCount <= 0x7FFF)
		{
			// unsure about the name, lets call it R_AddMultipleRendercommands
			void* cmds = utils::hook::call<void* (__cdecl)(int)>(0x4FB0D0)(count * sizeof(game::GfxPointVertex) * 2);
			
			if (cmds)
			{
				memcpy(cmds, verts, count * sizeof(game::GfxPointVertex) * 2);
				merged_cmd->lineCount += count;
			}
		}
		else
		{
			const size_t vert_mem_size = count * sizeof(game::GfxPointVertex) * 2;
			const auto line = reinterpret_cast<GfxCmdDrawLines*>( game::R_GetCommandBuffer(vert_mem_size + offsetof(GfxCmdDrawLines, verts), game::RC_DRAW_LINES));

			if (line)
			{
				line->lineCount = count;
				line->width = width;
				line->dimensions = dimension;
				line->depth_test = renderer::get()->g_line_depth_testing;

				memcpy(line->verts, verts, vert_mem_size);
			}
		}
	}

	// *

	struct GfxCmdDrawPoints
	{
		game::GfxCmdHeader header;
		__int16 pointCount;
		char width;
		char dimensions;
		game::GfxPointVertex point; // + 0x8 
	};

	void renderer::R_AddPointCmd(const std::uint16_t count, const char width, const char dimension, const game::GfxPointVertex* points)
	{
		if (count <= 0)
		{
			Assert();
		}

		const game::GfxCmdArray* s_cmdList = reinterpret_cast<game::GfxCmdArray*>(*(DWORD*)0x73D4A0);
		const auto merged_cmd = reinterpret_cast<GfxCmdDrawPoints*>(s_cmdList->lastCmd);

		if (merged_cmd && merged_cmd->header.id == game::RC_DRAW_POINTS
			&& count * sizeof(game::GfxPointVertex) + (unsigned int)merged_cmd->header.byteCount <= 0xFFFF
			&& merged_cmd->width == width
			&& merged_cmd->dimensions == dimension
			&& count + merged_cmd->pointCount <= 0x7FFF)
		{
			// unsure about the name, lets call it R_AddMultipleRendercommands
			void* cmds = utils::hook::call<void* (__cdecl)(int)>(0x4FB0D0)(count * sizeof(game::GfxPointVertex));

			if (cmds)
			{
				memcpy(cmds, points, count * sizeof(game::GfxPointVertex));
				merged_cmd->pointCount += count;
			}
		}
		else
		{
			const size_t vert_mem_size = count * sizeof(game::GfxPointVertex);
			const auto line = reinterpret_cast<GfxCmdDrawPoints*>(game::R_GetCommandBuffer(vert_mem_size + offsetof(GfxCmdDrawPoints, point), game::RC_DRAW_POINTS));

			if (line)
			{
				line->pointCount = count;
				line->width = width;
				line->dimensions = dimension;
				memcpy(&line->point, points, vert_mem_size);
			}
		}
	}

	// *

	// use cmd's depth_test var instead of the hardcoded 1
	void __declspec(naked) rb_drawlinescmd_stub()
	{
		const static uint32_t retn_addr = 0x5336B5;
		__asm
		{
			movsx   eax, dword ptr[esi + 8]; // depth test offset
			push	eax;

			movsx   eax, word ptr[esi + 4]; // og
			jmp		retn_addr;
		}
	}

	void renderer_init()
	{
		// RB_InitBackendGlobalStructs (original func)
		utils::hook::call<void(__cdecl)()>(0x536040)();
	}

	void post_render_init()
	{
		// R_InitFonts - register fonts/smalldevfont
		utils::hook::call<void(__cdecl)()>(0x535F10)();
	}

	// called from QE_LoadProject
	//void renderer_init_internal()
	//{
	//	g_invalid_material = game::Material_RegisterHandle("invalid_material", 0);
	//}

	//void __declspec(naked) r_begin_registration_internal_stub()
	//{
	//	const static uint32_t func_addr = 0x5011D0; // overwritten call
	//	const static uint32_t retn_addr = 0x4166D4;
	//	__asm
	//	{
	//		call	func_addr;
	//		add		esp, 4;

	//		pushad;
	//		call	renderer_init_internal;
	//		popad;

	//		jmp		retn_addr;
	//	}
	//}


	// *
	// replace missing invisible materials with 'invalid_material' (custom material)
	// *

	game::Material* Material_Register_LoadObj(const char* name)
	{
		if (!name || !*name)
		{
			Assert();
		}

		bool exists;
		std::uint16_t index;
		game::Material_GetHashIndex(name, &index, &exists);

		if (exists)
		{
			return static_cast<game::Material*>(game::rg->Material_materialHashTable[index]);
		}

		game::Material* material = game::Material_Load(name, 0);
		if (!material)
		{
			if (!game::rgp->defaultMaterial)
			{
				if (strcmp(name, "$default"))
				{
					AssertS("!strcmp( name, MATERIAL_DEFAULT_NAME )");
				}

				game::Com_Error("couldn't load material '$default'");
			}

			game::printf_to_console("WARNING: Could not find material '%s'\n", name);

			// #
			// replace missing materials with the invalid material (fixed invisible brushes)

			std::uint16_t invalid_mtl_idx;
			bool invalid_mtl_exists;

			game::Material_GetHashIndex("invalid_material", &invalid_mtl_idx, &invalid_mtl_exists); // wc/case256
			if (invalid_mtl_exists)
			{
				const auto invalid_mtl = static_cast<game::Material*>(game::rg->Material_materialHashTable[invalid_mtl_idx]);
				return utils::hook::call<game::Material* (__cdecl)(game::Material*, const char*)>(0x511900)(invalid_mtl, name); // Copy_Material
			}

			// og
			return utils::hook::call<game::Material* (__cdecl)(game::Material*, const char*)>(0x511900)(game::rgp->defaultMaterial, name); // Copy_Material

		}

		game::Material_GetHashIndex(name, &index, &exists);
		if (exists)
		{
			Assert();
		}

		game::Material_Add(index, material);
		return material;
	}

	void __declspec(naked) Material_Register_LoadObj_stub01()
	{
		const static uint32_t retn_addr = 0x511BCF;
		__asm
		{
			push	eax; // name
			call	Material_Register_LoadObj;
			add		esp, 4;
			jmp		retn_addr;
		}
	}

	void __declspec(naked) Material_Register_LoadObj_stub02()
	{
		const static uint32_t retn_addr = 0x511C50;
		__asm
		{
			push	eax; // name
			call	Material_Register_LoadObj;
			add		esp, 4;
			jmp		retn_addr;
		}
	}

	// called on texture refresh and when chaning texture resolution
	void on_reload_images()
	{
		for (auto img = 0; img < 32768; img++)
		{
			if (game::imageGlobals[img])
			{
				if (game::imageGlobals[img]->category == 3)
				{
					game::R_ReloadImage(game::imageGlobals[img]);
				}
				else if (game::imageGlobals[img]->category == 66)
				{
					game::Image_Release(game::imageGlobals[img]);

					const auto jpg_string = "prefab_thumbs\\"s + game::imageGlobals[img]->name + ".jpg"s;
					game::R_LoadJpeg(game::imageGlobals[img], jpg_string.c_str());
				}
			}
		}
	}


	// *
	// * Fix asserts when playing effects that use xmodels with no bsp loaded 
	// *

	int R_CalcReflectionProbeIndex(const float* lightingOrigin)
	{
		if (game::rgp->world)
		{
			return utils::hook::call<int (__cdecl)(const float*)>(0x5235E0)(lightingOrigin);
		}

		return 0;
	}

	bool is_rgp_world_loaded()
	{
		if (game::rgp->world)
		{
			return true;
		}

		return false;
	}

	void __declspec(naked) GetPrimaryLightForBoxCallback_stub()
	{
		const static uint32_t retn_addr = 0x52A6FC;
		__asm
		{
			pushad;
			call    is_rgp_world_loaded;
			test    al, al;
			popad;

			je      SKIP;
			call    dword ptr[ebp + 0Ch];

		SKIP:
			push    1;
			jmp		retn_addr;
		}
	}
	
	void __declspec(naked) R_GetLightingAtPoint_stub()
	{
		const static uint32_t func_addr = 0x52A290;
		const static uint32_t retn_addr = 0x52A704;
		__asm
		{
			pushad;
			call    is_rgp_world_loaded;
			test    al, al;
			popad;

			je      SKIP;
			call    func_addr;
			jmp		retn_addr;

		SKIP:
			mov		eax, 0;
			jmp		retn_addr;
		}
	}

	void begin_registration_internal_additional()
	{
		g_invalid_material = game::Material_RegisterHandle("invalid_material", 0);

		new cfxwnd();
		game::R_InitRendererForWindow(renderer::get_window(renderer::CFXWND)->hwnd);
	}

	void __declspec(naked) begin_registration_internal_stub()
	{
		const static uint32_t retn_addr = 0x4166D6;
		__asm
		{
			add     esp, 4; // overwritten

			pushad;
			call	begin_registration_internal_additional;
			popad;

			push    7; // overwritten
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

		dvars::r_sunpreview_shadow_enable = dvars::register_bool(
			/* name		*/ "r_sunpreview_shadow_enable",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "draw shadows when using sunpreview (SLOW!)");

		dvars::r_sunpreview_shadow_dist = dvars::register_float(
			/* name		*/ "r_sunpreview_shadow_dist",
			/* default	*/ 500.0f,
			/* mins		*/ 0.0f,
			/* maxs		*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "shadow drawing distance (camera to center of brush)");
	}

	void relocate_struct_ref(const std::uintptr_t code_addr, const void* target_addr, const unsigned int offset)
	{
		const auto struct_final_addr = reinterpret_cast<std::uintptr_t>(target_addr) + offset;
		utils::hook::set<std::uintptr_t>(code_addr, struct_final_addr);
	}

	void relocate_struct_ref(const std::uintptr_t* code_addr, const void* target_addr, const int patch_amount)
	{
		const auto struct_final_addr = reinterpret_cast<std::uintptr_t>(target_addr);

		for (auto i = 0; i < patch_amount; i++)
		{
			utils::hook::set<std::uintptr_t>(code_addr[i], struct_final_addr);
		}
	}

	void relocate_struct_ref(const std::uintptr_t* code_addr, const void* target_addr, const int patch_amount, const unsigned int offset)
	{
		const auto struct_final_addr = reinterpret_cast<std::uintptr_t>(target_addr) + offset;

		for (auto i = 0; i < patch_amount; i++)
		{
			utils::hook::set<std::uintptr_t>(code_addr[i], struct_final_addr);
		}
	}

	renderer::renderer()
	{
		renderer::p_this = this;

		// realoc tess.indices[32704] (materialCommands_t) and increase its size to iw3's (1048576)
		// -> xmodels with more then 32704 indices no longer crash radiant (was writing out of bounds)

		// base address
		uintptr_t tess_indices_base_patches[] =
		{
			// 00 4C 7A 01
			0x4FE6E8 + 1, 0x4FEA03 + 4, 0x530E92 + 4, 0x531052 + 4,
			0x531233 + 4, 0x5319D0 + 4, 0x531C8C + 4, 0x531F5D + 4,
			0x532B16 + 4, 0x532DFD + 4, 0x5330F9 + 4, 0x5334B1 + 4,
			0x53373D + 4, 0x534F5B + 4, 0x53AAD0 + 1, 0x53AB56 + 1,
			0x53AD7B + 1, 0x541693 + 4, 0x56CECD + 4,

		}; relocate_struct_ref(tess_indices_base_patches, tess_indices_reloc, ARRAYSIZE(tess_indices_base_patches));


		// 2
		uintptr_t tess_indices_p2_patches[] =
		{
			// 02 4C 7A 01
			0x530EA0 + 4, 0x531060 + 4, 0x53123D + 4, 0x5319EB + 4,
			0x531CAA + 4, 0x531F65 + 4, 0x532B2B + 4, 0x532E18 + 4,
			0x53310E + 4, 0x5334CA + 4, 0x534F48 + 4, 0x5416A1 + 4,
			0x56CEE6 + 4,

		}; relocate_struct_ref(tess_indices_p2_patches, tess_indices_reloc, ARRAYSIZE(tess_indices_p2_patches), 2);

		
		//  4
		uintptr_t tess_indices_p4_patches[] =
		{
			// 04 4C 7A 01
			0x530EB1 + 4, 0x531071 + 4, 0x531255 + 4, 0x5319DB + 4,
			0x531C9A + 4, 0x531F78 + 4, 0x532B41 + 4, 0x532E34 + 4,
			0x533124 + 4, 0x5334E0 + 4, 0x534F66 + 4, 0x5416B2 + 4,
			0x56CEFE + 4,


		}; relocate_struct_ref(tess_indices_p4_patches, tess_indices_reloc, ARRAYSIZE(tess_indices_p4_patches), 4);


		// 6
		uintptr_t tess_indices_p6_patches[] =
		{
			// 06 4C 7A 01
			0x530EBF + 4, 0x53107F + 4, 0x531260 + 4, 0x5319E3 + 4,
			0x531CA2 + 4, 0x531F80 + 4, 0x532B57 + 4, 0x532E4B + 4,
			0x53313B + 4, 0x5334F6 + 4, 0x534F6E + 4, 0x5416C0 + 4,


		}; relocate_struct_ref(tess_indices_p6_patches, tess_indices_reloc, ARRAYSIZE(tess_indices_p6_patches), 6);


		// 8
		uintptr_t tess_indices_p8_patches[] =
		{
			// 08 4C 7A 01
			0x530ECD + 4, 0x53108D + 4, 0x531245 + 4, 0x5319F3 + 4,
			0x531CB2 + 4, 0x531F6D + 4, 0x532B6C + 4, 0x532E60 + 4,
			0x533150 + 4, 0x53350B + 4, 0x534F50 + 4, 0x5416CE + 4,


		}; relocate_struct_ref(tess_indices_p8_patches, tess_indices_reloc, ARRAYSIZE(tess_indices_p8_patches), 8);


		// 10
		uintptr_t tess_indices_p10_patches[] =
		{
			// 0A 4C 7A 01
			0x530EE1 + 4, 0x5310A1 + 4, 0x53126F + 4, 0x5319FE + 4,
			0x531CBA + 4, 0x531F8B + 4, 0x532B82 + 4, 0x532E76 + 4,
			0x533166 + 4, 0x533521 + 4, 0x534F79 + 4, 0x5416DF + 4,


		}; relocate_struct_ref(tess_indices_p10_patches, tess_indices_reloc, ARRAYSIZE(tess_indices_p10_patches), 10);

		// patch indices cmp
		utils::hook::set<DWORD>(0x4FE899 + 2, TESS_INDICES_AMOUNT); // RB_DrawEditorSkinnedCached_Sub
		utils::hook::set<DWORD>(0x530AD2 + 2, TESS_INDICES_AMOUNT); // RB_DrawTriangles_Internal and RB_DrawPolyInteriors
		utils::hook::set<DWORD>(0x530B0F + 2, TESS_INDICES_AMOUNT); // Everything else is 2D or lines
		utils::hook::set<DWORD>(0x530E72 + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x531032 + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x5311EA + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x53199D + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x531C6E + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x531F2A + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x532AE9 + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x532DC9 + 1, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x5330DE + 1, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x533484 + 2, TESS_INDICES_AMOUNT);
		utils::hook::set<DWORD>(0x534F1B + 2, TESS_INDICES_AMOUNT);

		// ------------------------------------------------------------------------------------------------

		// realoc dx.windows[5] to add more d3d windows

		// base address (hwnd)
		uintptr_t dxwnd_base_patches[] =
		{
			// FC 82 36 01
			0x4FFB88 + 2, 0x500692 + 2, 0x5012B3 + 1, 0x501414 + 2,
			0x501441 + 2, 0x5015F6 + 1, 0x501AE6 + 1,

		}; relocate_struct_ref(dxwnd_base_patches, renderer::windows, ARRAYSIZE(dxwnd_base_patches), 0x0);


		// + 0x4 address (swapchain)
		uintptr_t dxwnd_swapchain_patches[] =
		{
			// 00 83 36 01
			0x4FFB57 + 2, 0x500625 + 2, 0x500EA1 + 1, 0x5012F7 + 2,
			0x501740 + 2, 0x501748 + 2, 0x52BBE0 + 2, 0x52CA63 + 1,
			0x53578E + 2,

		}; relocate_struct_ref(dxwnd_swapchain_patches, renderer::windows, ARRAYSIZE(dxwnd_swapchain_patches), 0x4);


		// + 0x8 address (width)
		uintptr_t dxwnd_width_patches[] =
		{
			// 04 83 36 01
			0x4FFB99 + 2, 0x5013D4 + 2, 0x50160A + 1, 0x5017C6 + 1,
			0x501B2A + 2, 0x50648E + 2, 0x50653B + 2, 0x52BBA1 + 2,

		}; relocate_struct_ref(dxwnd_width_patches, renderer::windows, ARRAYSIZE(dxwnd_width_patches), 0x8);


		// + 0xC address (height)
		uintptr_t dxwnd_height_patches[] =
		{
			// 08 83 36 01
			0x4FFBAB + 2, 0x5013DA + 2, 0x5015FB + 2, 0x501B33 + 2,
			0x506488 + 2, 0x506522 + 2, 0x52BBA7 + 2,

		}; relocate_struct_ref(dxwnd_height_patches, renderer::windows, ARRAYSIZE(dxwnd_height_patches), 0xC);


		// patch dx.windowCount checks (inc. to GFX_TARGETWINDOW_COUNT)
		utils::hook::set<BYTE>(0x4FFB29 + 2, GFX_TARGETWINDOW_COUNT);
		utils::hook::set<BYTE>(0x50057F + 2, GFX_TARGETWINDOW_COUNT);
		utils::hook::set<BYTE>(0x5005F0 + 6, GFX_TARGETWINDOW_COUNT);
		utils::hook::set<BYTE>(0x5005F9 + 1, GFX_TARGETWINDOW_COUNT);

		// stub to register new gfxwindows
		utils::hook(0x4166D1, begin_registration_internal_stub, HOOK_JUMP).install()->quick();

		// ------------------------------------------------------------------------------------------------

		// rewrite, working fine but not needed (debug)
		// utils::hook::detour(0x4FE750, RB_DrawEditorSkinnedCached, HK_JUMP);

		// ------------------------------------------------------------------------------------------------

		// set default value for r_vsync to false
		utils::hook::set<BYTE>(0x51FB1A + 1, 0x0);

		// enable/disable drawing of boxes around the origin on entities
		utils::hook(0x478E33, render_origin_boxes_stub, HOOK_JUMP).install()->quick();

		// enable/disable drawing of backface wireframe on patches
		utils::hook::nop(0x441567, 16);
			 utils::hook(0x441567, patch_backface_wireframe_stub).install()->quick();

		// do not cull entities with the custom no-cull flag (used phys prefabs)
		utils::hook(0x407AD2, cubic_culling_overwrite_stub1, HOOK_JUMP).install()->quick();
		utils::hook(0x407BE0, cubic_culling_overwrite_stub2, HOOK_JUMP).install()->quick();

		// do not force spec and bump picmip
		utils::hook::nop(0x420A73, 30);
		utils::hook::nop(0x4208B0, 30);
		utils::hook::nop(0x41656E, 30);

		// custom shader constants
		utils::hook(0x53BC39, R_SetPassPixelShaderStableArguments_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x53B9E3, R_SetPassShaderObjectArguments_stub, HOOK_JUMP).install()->quick();

		// vertex and pixelshader loading (allows loading of shaders that are not included in the shader_names file)
		utils::hook(0x5188A6, r_create_pixelshader, HOOK_CALL).install()->quick();

		utils::hook::nop(0x51873F, 17);
		 	 utils::hook(0x51873F, r_create_vertexshader_stub, HOOK_JUMP).install()->quick();

		
		// fix sun preview (selecting brushes keeps sunpreview active; sun no longer casts shadows -> FPS ++)
		utils::hook(0x406706, sunpreview, HOOK_CALL).install()->quick();

		// check for nullptr (world_entity) in a sunlight preview function.
		utils::hook::nop(0x4067C7, 6);
		utils::hook(0x4067C7, sunlight_preview_arg_check, HOOK_JUMP).install()->quick();


		// disable world darkening when selecting light entities with light preview enabled
		utils::hook::nop(0x407099, 5);

		// hook R_SetupPass to set custom techniques etc
		utils::hook(0x53AC4F, r_setup_pass_xmodel, HOOK_CALL).install()->quick();
		utils::hook(0x4FE646, r_setup_pass_brush, HOOK_CALL).install()->quick();
		utils::hook(0x53A7BA, r_setup_pass_2d, HOOK_CALL).install()->quick(); // 2d and translucent
		utils::hook(0x532376, r_setup_pass_surflists, HOOK_CALL).install()->quick(); // effects
		utils::hook(0x53238F, r_setup_pass_surflists, HOOK_CALL).install()->quick(); // effects

		// * ------

		// hook RB_ExecuteRenderCommandsLoop to implement postfx, logging and effect logic
		utils::hook::nop(0x535B10, 6);
			utils::hook( 0x535B10, RB_ExecuteRenderCommandsLoop_stub, HOOK_JUMP).install()->quick();

		// hook R_AddClearCmd call (depthbuffer clearing)(last function in R_SetSceneParms) to implement new logic:
		// setup viewinfo and emissive draw lists (effects) (normally done within R_RenderScene)
		utils::hook(0x5064B1, setup_viewinfo_stub, HOOK_JUMP).install()->quick();

		// disable original RB_Draw3D call (gets called after RB_ExecuteRenderCommandsLoop, so after the actual scene was rendered)
		utils::hook::nop(0x4FD6B3, 5);

		// do not call RB_Draw3DCommon within RB_CallExecuteRenderCommands (not handled yet, needs to be called after effects where added)
		utils::hook::nop(0x535A6E, 5);

		// do not add a clearscreen command at the beginning of CCamWnd::OnPaint
		// !! spot used to implement effect controlling logic (effects::camera_onpaint_stub)
		//utils::hook::nop(0x40304D, 5);

		// disable depth testing for outlines on selected brushes
		// ^ spot would normally clear the depthbuffer to achieve that but we need depth info for effects later down the line
		utils::hook(0x4084D2, set_line_depth_testing, HOOK_JUMP).install()->quick(); 

		// enable depth testing for lines + stub that draws additional debug lines (sun direction, mesh painter ...)
		utils::hook(0x40CC21, set_line_depth_testing_2, HOOK_JUMP).install()->quick();

		// tint selected light entities
		utils::hook(0x4082FD, light_selection_tint_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x4083B9, light_selection_tint_reset_stub, HOOK_JUMP).install()->quick();

		// * ------

		// rewrite R_AddLineCmd to add depth_test functionality
		utils::hook::detour(0x4FD0A0, renderer::R_AddLineCmd, HK_JUMP);

		// make RB_DrawLinesCmd use the cmd's depth_test var
		utils::hook::nop(0x5336AF, 6);
			 utils::hook(0x5336AF, rb_drawlinescmd_stub, HOOK_JUMP).install()->quick();

		// change vertex offset in RB_DrawLinesCmd because depth_test var was added to GfxCmdDrawLines
		utils::hook::set<BYTE>(0x533669 + 2, 12);
		utils::hook::set<BYTE>(0x5336B5 + 2, 12);

		// hook R_InitBackendGlobalStructs somewhere within R_Init
		utils::hook(0x500742, renderer_init, HOOK_CALL).install()->quick();

		// register smalldevfont
		utils::hook(0x5011B8, post_render_init, HOOK_CALL).install()->quick();

		// stub within R_BeginRegistrationInternal (on call to init layermatwnd)
		//utils::hook(0x4166CC, r_begin_registration_internal_stub, HOOK_JUMP).install()->quick();

		// hk 'R_SortMaterials' call after 'R_IssueRenderCommands' in 'CCamWnd::OnPaint'
		utils::hook(0x40306B, on_cam_paint_post_rendercommands_stub, HOOK_JUMP).install()->quick();

		// replace missing invisible materials with 'invalid_material' (custom material)
		utils::hook(0x511BCA, Material_Register_LoadObj_stub01, HOOK_JUMP).install()->quick();
		utils::hook(0x511C4B, Material_Register_LoadObj_stub02, HOOK_JUMP).install()->quick();

		// rewrite R_ReloadImages (mainly for the prefab previewer -> handle jpg)
		utils::hook::detour(0x513D70, on_reload_images, HK_JUMP);

		// *
		// * Fix asserts when playing effects that use xmodels (gfx-scene-entities) with no bsp loaded 
		// *

		utils::hook(0x52A6E8, R_CalcReflectionProbeIndex, HOOK_CALL).install()->quick();
		utils::hook(0x52A6F7, GetPrimaryLightForBoxCallback_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x52A6FF, R_GetLightingAtPoint_stub, HOOK_JUMP).install()->quick();
		utils::hook::nop(0x500F4C, 5); // < on shutdown

		// silence "gfxCmdBufState.prim.vertDeclType == VERTDECL_PACKED" assert
		utils::hook::nop(0x53AB4A, 5);

		// silence assert 'localDrawSurf->fields.prepass == MTL_PREPASS_NONE'
		utils::hook::nop(0x52EE39, 5);
		utils::hook::nop(0x52F2FA, 5);

		// silence assert 'drawSurf.fields.primaryLightIndex doesn't index info->viewInfo->shadowableLightCount'
		utils::hook::nop(0x55A3A3, 5);

		// silence assert '((region == DRAW_SURF_FX_CAMERA_EMISSIVE) || (drawSurf == scene.drawSurfs[region]) || (drawSurf->fields.primarySortKey >= (drawSurf - 1)->fields.primarySortKey))'
		utils::hook::nop(0x52EE95, 5);

		// * ------

		// load depth prepass and build-floatz technique (Material_LoadTechniqueSet -> g_useTechnique)
		utils::hook::set<BYTE>(0x633FC4 + 0, 0x1);  // depth prepass
		utils::hook::set<BYTE>(0x633FC4 + 1, 0x1);  // build floatz
		utils::hook::set<BYTE>(0x633FC4 + 2, 0x1);  // build shadowmap depth
		utils::hook::set<BYTE>(0x633FC4 + 3, 0x1);  // build shadowmap color
		utils::hook::set<BYTE>(0x633FC4 + 4, 0x1);  // unlit
		utils::hook::set<BYTE>(0x633FC4 + 5, 0x1);  // emissive
		utils::hook::set<BYTE>(0x633FC4 + 6, 0x1);  // emissive shadow
		utils::hook::set<BYTE>(0x633FC4 + 7, 0x1);  // lit
		utils::hook::set<BYTE>(0x633FC4 + 8, 0x1);  // lit sun
		utils::hook::set<BYTE>(0x633FC4 + 9, 0x1);  // lit sun shadow
		utils::hook::set<BYTE>(0x633FC4 + 10, 0x1); // lit spot
		utils::hook::set<BYTE>(0x633FC4 + 11, 0x1); // lit spot shadow
		utils::hook::set<BYTE>(0x633FC4 + 12, 0x1); // lit omni
		utils::hook::set<BYTE>(0x633FC4 + 13, 0x1); // lit omni shadow
		utils::hook::set<BYTE>(0x633FC4 + 14, 0x1); // lit instanced
		utils::hook::set<BYTE>(0x633FC4 + 15, 0x1); // lit instanced sun
		utils::hook::set<BYTE>(0x633FC4 + 16, 0x1); // lit instanced sun shadow
		utils::hook::set<BYTE>(0x633FC4 + 17, 0x1); // lit instanced spot
		utils::hook::set<BYTE>(0x633FC4 + 18, 0x1); // lit instanced spot shadow
		utils::hook::set<BYTE>(0x633FC4 + 19, 0x1); // lit instanced omni
		utils::hook::set<BYTE>(0x633FC4 + 20, 0x1); // lit instanced omni shadow
		utils::hook::set<BYTE>(0x633FC4 + 21, 0x1); // light spot
		utils::hook::set<BYTE>(0x633FC4 + 22, 0x1); // light omni
		utils::hook::set<BYTE>(0x633FC4 + 23, 0x1); // light spot shadow
		utils::hook::set<BYTE>(0x633FC4 + 24, 0x1); // fakelight normal
		utils::hook::set<BYTE>(0x633FC4 + 25, 0x1); // fakelight view
		utils::hook::set<BYTE>(0x633FC4 + 26, 0x1); // sunlight preview
		utils::hook::set<BYTE>(0x633FC4 + 27, 0x1); // case texture
		utils::hook::set<BYTE>(0x633FC4 + 28, 0x1); // solid wireframe
		utils::hook::set<BYTE>(0x633FC4 + 29, 0x1); // shaded wireframe
		utils::hook::set<BYTE>(0x633FC4 + 30, 0x1); // shadowcookie caster
		utils::hook::set<BYTE>(0x633FC4 + 31, 0x1); // shadowcookie receiver
		utils::hook::set<BYTE>(0x633FC4 + 32, 0x1); // debug bumpmap
		utils::hook::set<BYTE>(0x633FC4 + 33, 0x1); // debug bumpmap instanced

		// * ------


		// write a logfile showing all rendercommands for a single frame
		command::register_command("log_rendercommands"s, [this](auto)
		{
			g_log_rendercommands = true;
		});


		// only affects materials registered / used post init
		command::register_command("reload_shaders"s, [this](auto)
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


		// dump vertex and pixelshader for given technique
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

		command::register_command_with_hotkey("fakesun_fog_toggle"s, [this](auto)
		{
			dvars::set_bool(dvars::r_fakesun_fog_enabled, !dvars::r_fakesun_fog_enabled->current.enabled);
		});

		command::register_command_with_hotkey("filmtweak_toggle"s, [this](auto)
		{
			const auto r_filmtweakenable = game::Dvar_FindVar("r_filmtweakenable");
			dvars::set_bool(r_filmtweakenable, !r_filmtweakenable->current.enabled);
		});
	}

	renderer::~renderer()
	{ }
}
