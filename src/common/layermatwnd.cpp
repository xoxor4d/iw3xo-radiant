/*
 * The layered material window is using d3d to draw the materials.
 * Since its not in-use (I think?), we use the fully implemented drawing routine to draw xmodels for the modelselector
 * to a texture that is then shown by ImGui (similar to how the grid/camera window works)
 *
 * this file handles:
 * - layered window creation
 * - scene setup
 * - axis model + preview model rendering
 * - resetting camera distance and angles if preview model switches
 * - copy scene to a texture
 */

#include "std_include.hpp"

layermatwnd_s* layermatwnd_struct = reinterpret_cast<layermatwnd_s*>(0x181F500);
camera_s layercam = {};
float _model_anim_yaw = 0.0f;

static game::orientation_t editor_instmodel_mtx =
{
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f
};

void R_AddCmdSetViewportValues(int x, int y, int width, int height)
{
	if (width <= 0) {
		game::Com_Error("R_AddCmdSetViewportValues :: width");
	}
	
	if (height <= 0) {
		game::Com_Error("R_AddCmdSetViewportValues :: height");
	}
	
	// RC_SET_VIEWPORT
	if (auto cmd = reinterpret_cast<game::GfxCmdSetViewport*>(game::R_GetCommandBuffer(20, 7));
			 cmd)
	{
		cmd->viewport.height = height;
		cmd->viewport.x = x;
		cmd->viewport.y = y;
		cmd->viewport.width = width;
	}
	else
	{
		game::Com_Error("R_AddCmdSetViewportValues :: cmd");
	}
}

// rewrite of R_SetSceneParms (not of much use anymore)
void set_scene_params_modelselector(const float* origin, float* axis, game::GfxMatrix* projection, int x, int y, int width, int height, bool clear)
{
	game::GfxViewParms* view_parms = game::R_SetupViewParms();
	memset(view_parms, 0, sizeof(game::GfxViewParms));

	game::scene->def.time = timeGetTime();
	game::scene->def.floatTime = (float)game::scene->def.time * 0.001000000047497451f;
	
	game::scene->def.viewOffset[0] = 0.0f;
	game::scene->def.viewOffset[1] = 0.0f;
	game::scene->def.viewOffset[2] = 0.0f;

	view_parms->origin[0] = origin[0];
	view_parms->origin[1] = origin[1];
	view_parms->origin[2] = origin[2];
	view_parms->origin[3] = 1.0f;

	game::CopyAxis(axis, &view_parms->axis[0][0]);
	game::MatrixForViewer(view_parms->viewMatrix.m, origin, axis);
	
	game::rg->lodParms.origin[0] = origin[0];
	game::rg->lodParms.origin[1] = origin[1];
	game::rg->lodParms.origin[2] = origin[2];

	game::rg->lodParms.ramp[0].scale = game::Dvar_FindVar("r_lodScaleRigid")->current.value;
	game::rg->lodParms.ramp[0].bias = game::Dvar_FindVar("r_lodBiasRigid")->current.value;

	game::rg->lodParms.ramp[1].scale = game::Dvar_FindVar("r_lodScaleSkinned")->current.value;
	game::rg->lodParms.ramp[1].bias = game::Dvar_FindVar("r_lodBiasSkinned")->current.value;

	memcpy(&view_parms->projectionMatrix, projection, sizeof(view_parms->projectionMatrix));
	view_parms->depthHackNearClip = projection->m[3][2];
	
	game::MatrixMultiply44(view_parms, &view_parms->projectionMatrix, &view_parms->viewProjectionMatrix);
	game::MatrixInverse44(&view_parms->viewProjectionMatrix, &view_parms->inverseViewProjectionMatrix);

	game::rg->viewOrg[0] = origin[0];
	game::rg->viewOrg[1] = origin[1];
	game::rg->viewOrg[2] = origin[2];
	game::rg->viewDir[0] = axis[0];
	game::rg->viewDir[1] = axis[1];
	game::rg->viewDir[2] = axis[2];

	game::R_SetupRenderCmd(&game::scene->def, view_parms);
	R_AddCmdSetViewportValues(x, y, width, height);

	if(clear)
	{
		float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::R_Clear(6, white, 1.0f, 0);
	}
}

void setup_scene_modelselector(float* origin, float* axis, int x, int y, int width, int height)
{
	float fov_y = tan(ggui::get_rtt_modelselector()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f;
	float fov_x = fov_y * ((float)layercam.width / (float)layercam.height);
	
	game::GfxMatrix projection = {};
	
	game::R_SetupProjection(&projection, fov_x, fov_y, 0.0099999998f);
	set_scene_params_modelselector(origin, axis, &projection, x, y, width, height, false);
}

void calc_raydir_modelselector(int x, int y, float* dir)
{
	const float tan_half_y = tan(ggui::get_rtt_modelselector()->camera_fov * 0.01745329238474369f * 0.5f);
	const float tan_half_x = (tan_half_y * 0.75f + tan_half_y * 0.75f) / (float)layercam.height;

	const float xa = tan_half_x * (float)(y - layercam.height / 2);
	const float xb = tan_half_x * (float)(x - layercam.width / 2);

	dir[0] = layercam.vup[0] * xa + layercam.vright[0] * xb + layercam.vpn[0];
	dir[1] = layercam.vup[1] * xa + layercam.vright[1] * xb + layercam.vpn[1];
	dir[2] = layercam.vup[2] * xa + layercam.vright[2] * xb + layercam.vpn[2];

	utils::vector::normalize(dir);
}

void build_matrix_modelselector()
{
	float angles[3];

	angles[0] = layercam.angles[0];
	angles[1] = layercam.angles[1];
	angles[2] = layercam.angles[2];
	angles[0] = -angles[0];

	game::AngleVectors(angles, layercam.vpn, layercam.vright, layercam.vup);

	const float xa = layercam.angles[1] * sin(1.0f);
	const float xb = cos(xa);
	const float xc = sin(xa);

	layercam.forward[0] = xb;
	layercam.forward[1] = xc;
	layercam.forward[2] = 0.0f;

	layercam.right[0] = xc;
	layercam.right[1] = -xb;
	layercam.right[2] = 0.0f;
}

// *
// ggui::modelselector::menu()
// drag drop target => cxywnd::rtt_grid_window()
// drag drop target => ccamwnd::rtt_camera_window()

// rendering happens in 0x533880
// model inst rendering in 0x53AA30

namespace layermatwnd
{
	int rendermethod_axis = 4;
	int rendermethod_preview = 24;
}

void clayermatwnd::on_paint()
{
	PAINTSTRUCT Paint;
	BeginPaint(layermatwnd_struct->m_content_hwnd, &Paint);

	if(!game::R_SetupRendertarget_CheckDevice(layermatwnd_struct->m_content_hwnd))
	{
		EndPaint(layermatwnd_struct->m_content_hwnd, &Paint);
		return;
	}

	// *
	// 
	
	build_matrix_modelselector();
	
	const auto m_selector = ggui::get_rtt_modelselector();
	m_selector->camera_fov = 60.0f;
	
	// stop yaw animation if rotated manually
	if(!m_selector->user_rotation)
	{
		_model_anim_yaw += 0.1f;
	}
	
	if (_model_anim_yaw > 360.0f) {
		_model_anim_yaw -= 360.0f;
	}

	/*  + render methods +
	 *	fullbright		= 4,
	 *	normal-based	= 24,
	 *	view-based		= 25,
	 *	case-textures	= 27,
	 *	wireframe		= 29,
	 */
	
	//const int rendermethod_axis = 4;
	//const int rendermethod_preview = 24;

	layercam.width = game::dx->windows[ggui::LAYERED].width;
	layercam.height = game::dx->windows[ggui::LAYERED].height;
	
	if (layercam.width != 0 && layercam.height != 0)
	{
		{
			if (m_selector->menustate && !m_selector->preview_model_name.empty())
			{
                m_selector->bad_model = false;

				// get model handle
				if(auto model = game::R_RegisterModel(m_selector->preview_model_name.c_str());
						model)
				{
                    // use the fx model for invalid or vertex heavy models
                    if(   !model->surfs 
                        || model->bad
                        || model->surfs->vertCount > 15000)
                    {
                        model = game::R_RegisterModel("fx");
                        m_selector->bad_model = true;
                    }

					// begin a new frame, clear the scene
					game::R_BeginFrame();
					game::R_Clear(7, game::g_qeglobals->d_savedinfo.colors[0], 1.0f, 0);

					// setup scene
					float axis[9];
					axis[0] = layercam.vpn[0];
					axis[1] = layercam.vpn[1];
					axis[2] = layercam.vpn[2];
					axis[3] = -layercam.vright[0];
					axis[4] = -layercam.vright[1];
					axis[5] = -layercam.vright[2];
					axis[6] = layercam.vup[0];
					axis[7] = layercam.vup[1];
					axis[8] = layercam.vup[2];

					float origin[3];
					origin[0] = m_selector->camera_offset[0]; //cmainframe::activewnd->m_pCamWnd->camera.origin[0];
					origin[1] = m_selector->camera_offset[1]; //cmainframe::activewnd->m_pCamWnd->camera.origin[1];
					origin[2] = m_selector->camera_offset[1]; //cmainframe::activewnd->m_pCamWnd->camera.origin[2];

					float temp_angles[3];
					temp_angles[0] = -m_selector->camera_angles[0];
					temp_angles[1] = _model_anim_yaw - m_selector->camera_angles[1];
					temp_angles[2] = m_selector->camera_angles[2];

					setup_scene_modelselector(origin, axis, 0, 0, game::dx->windows[game::dx->targetWindowIndex].width, game::dx->windows[game::dx->targetWindowIndex].height);

					// setup model orientation
					game::orientation_t model_orientation = {};
					game::AnglesToAxis(temp_angles, &model_orientation.axis[0][0]);

					
					// center model (old way of doing it)
					const float dist = (fabsf(model->maxs[2]) + fabsf(model->mins[2])) * 0.5f;
					// model_orientation.origin[0] = model->radius * 1.75f;
					// model_orientation.origin[1] = 0.0f;
					// model_orientation.origin[2] = -(model->maxs[2] - dist);

					// get direction vector from center point into the scene
					game::vec3_t direction;
					calc_raydir_modelselector(static_cast<int>(m_selector->scene_size_imgui.x * 0.5f), static_cast<int>(m_selector->scene_size_imgui.y * 0.5f), direction);

					// calculate model origin including "zoom"
					utils::vector::ma(origin, model->radius * 1.75f + m_selector->camera_distance, direction, model_orientation.origin);
					model_orientation.origin[2] = -(model->maxs[2] - dist);
					
					// transform by identity matrix (not needed)
					// game::orientation_t model_orientation = {};
					// game::OrientationConcatenate(&temp_orientation, &editor_instmodel_mtx, &model_orientation);

					
					// *
					// draw axis model
					{
						const float axis_scale = 0.05f;
						const float axis_dist  = 25.0f;
						
						game::orientation_t axis_orientation = {};
						game::AnglesToAxis(temp_angles, &axis_orientation.axis[0][0]);

						// get direction vector from bottom right point into the scene
						calc_raydir_modelselector(static_cast<int>(m_selector->scene_size_imgui.x) - 35, 25, direction);

						// push the model into the scene so it does not clip with the camera
						utils::vector::ma(origin, axis_dist, direction, axis_orientation.origin);

						if (!m_selector->axis_model_initiated)
						{
							// get model handle
							if (auto axis_model = game::R_RegisterModel("axis");
									 axis_model)
							{
								// AddModelToModelInstBuff - add the model to the instance "buffer"
								if (const int model_inst_num = utils::hook::call<int(__cdecl)(game::XModel * _model, float* _axis, float _scale)>(0x4FDBE0)(axis_model, &axis_orientation.origin[0], axis_scale);
											  model_inst_num)
								{
									m_selector->axis_model_inst_handle = model_inst_num;
								}

								m_selector->axis_model_initiated = true;
							}
						}
						else
						{
							// ModelInstUpdate
							utils::hook::call<void(__cdecl)(int _inst, float* _axis, float scale)>(0x4FDD80)
								(m_selector->axis_model_inst_handle, &axis_orientation.origin[0], axis_scale);

							// SkinModelInst - add model surfs to the skinnedCached buffer
							utils::hook::call<int(__cdecl)(int inst_handle, int checkhandle, int techflags, game::GfxColor* color, int drawflags)>(0x4FE2E0)(
								m_selector->axis_model_inst_handle, 0, layermatwnd::rendermethod_axis, nullptr, 2);
						}
					}

					
					// *
					// draw preview model
					{
						// is the last model still the selected one? -> update the model instance
						if (m_selector->preview_model_ptr && m_selector->preview_model_ptr == model)
						{
							// ModelInstUpdate
							utils::hook::call<void(__cdecl)(int _inst, float* _axis, float scale)>(0x4FDD80)
								(m_selector->preview_model_inst_handle, &model_orientation.origin[0], 1.0f);
						}
						// new model selected
						else
						{
							// reset camera zoom and angles
							m_selector->camera_angles[0] = 0.0f;
							m_selector->camera_angles[1] = 0.0f;
							m_selector->camera_angles[2] = 0.0f;
							m_selector->camera_distance = 0.0f;
							m_selector->camera_offset[0] = 0.0f;
							m_selector->camera_offset[1] = 0.0f;
							m_selector->camera_offset[2] = 0.0f;
							
							// RemoveModelInstFromBuf - clear old model instance
							if (m_selector->preview_model_ptr && m_selector->preview_model_inst_handle)
							{
								utils::hook::call<void(__cdecl)(int)>(0x4FDCE0)(m_selector->preview_model_inst_handle);
							}

							// AddModelToModelInstBuff - add the model to the instance "buffer"
							if (const int model_inst_num = utils::hook::call<int(__cdecl)(game::XModel * _model, float* _axis, float _scale)>(0x4FDBE0)(model, &model_orientation.origin[0], 1.0f);
								model_inst_num)
							{
								m_selector->preview_model_ptr = model;
								m_selector->preview_model_inst_handle = model_inst_num;
							}
						}

						if (m_selector->preview_model_ptr)
						{
							// SkinModelInst - add model surfs to the skinnedCached buffer
							utils::hook::call<int(__cdecl)(int inst_handle, int checkhandle, int techflags, game::GfxColor* color, int drawflags)>(0x4FE2E0)(
								m_selector->preview_model_inst_handle, 0, layermatwnd::rendermethod_preview, nullptr, 2);
						}
					}
				}

				// sorts surfaces and adds draw triangle rendercmd
				utils::hook::call<void(__cdecl)()>(0x4FDA10)();

				game::R_EndFrame();
				game::R_IssueRenderCommands(-1);
				game::R_SortMaterials();
				components::renderer::copy_scene_to_texture(ggui::LAYERED, m_selector->scene_texture);
			}
		}
	}

	game::R_CheckTargetWindow(layermatwnd_struct->m_content_hwnd);
	
	// hunk related
	int& random_dword01 = *reinterpret_cast<int*>(0x25D5B88);
	int& random_dword02 = *reinterpret_cast<int*>(0x242293C);
	int& random_dword03 = *reinterpret_cast<int*>(0x2422940);
	
	if(!random_dword01) {
		game::Com_Error("clayermatwnd::on_paint() :: s_hunkData");
	}

	random_dword03 = random_dword02;

	// nice meme iw
	EndPaint(layermatwnd_struct->m_content_hwnd, &Paint);
}


// window proc for the complete window
LRESULT __stdcall clayermatwnd::windowproc_frame(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg <= WM_NOTIFY)
	{
		if (Msg == WM_SIZE)
		{
			int x, y, cx, cy;

			utils::hook::call<void(__cdecl)(int*, int*, int*, int*)>(0x417B40)(&x, &y, &cx, &cy);
			SetWindowPos(layermatwnd_struct->m_content_hwnd, 0, x, y, cx, cy, SWP_NOZORDER);
			
			return DefWindowProcA(hWnd, 5u, wParam, lParam);
		}

		if (Msg == WM_CLOSE)
		{
			ShowWindow(layermatwnd_struct->m_frame_hwnd, SW_HIDE);
			return 0;
		}

		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}

	if (Msg != WM_KEYDOWN)
	{
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	
	if(wParam == VK_ESCAPE)
	{
		ShowWindow(layermatwnd_struct->m_frame_hwnd, SW_HIDE);
		return 0;
	}

	return 0;
}

// window proc for content area
LRESULT __stdcall clayermatwnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg <= WM_VSCROLL)
	{
		if (Msg == WM_SIZE)
		{
			game::R_Hwnd_Resize(hWnd, LOWORD(lParam), HIWORD(lParam));
			return 0;
		}

		if (Msg == WM_PAINT)
		{
			clayermatwnd::on_paint();
			return 0;
		}

		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}

	if (Msg != WM_LBUTTONDOWN)
	{
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
	
	return 0;
}

// create the "content" area
void clayermatwnd::create_layerlist()
{
	tagRECT rect;
	GetClientRect(layermatwnd_struct->m_frame_hwnd, &rect);

	layermatwnd_struct->m_content_hwnd = CreateWindowExA(
		0, // WS_EX_COMPOSITED
		"LayeredMaterialList",
		0,
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD,
		0, 0, rect.right - rect.left,rect.bottom - rect.top,
		layermatwnd_struct->m_frame_hwnd,
		0, 0, 0);

	if (!layermatwnd_struct->m_content_hwnd)
	{
		game::Com_Error("Couldn't create the material layer list");
	}
}

// register window classes (frame + content)
void clayermatwnd::precreate_window()
{
	WNDCLASSEXA wc;
	memset(&wc, 0, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandleA(0);
	wc.hIcon = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.style = 8; //512;
	wc.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
	wc.lpszClassName = "LayeredMaterialWindow"; // frame
	wc.lpfnWndProc = clayermatwnd::windowproc_frame;
	RegisterClassExA(&wc);

	wc.lpszClassName = "LayeredMaterialList"; // content
	wc.lpfnWndProc = clayermatwnd::windowproc;
	RegisterClassExA(&wc);
}

void clayermatwnd::create_layermatwnd()
{
	memset(layermatwnd_struct, 0, sizeof(layermatwnd_s));
	clayermatwnd::precreate_window();

	layermatwnd_struct->m_frame_hwnd = CreateWindowExA(WS_EX_PALETTEWINDOW, "LayeredMaterialWindow", "(no layered material)", WS_THICKFRAME | WS_SYSMENU | WS_DLGFRAME | WS_BORDER | 0x80000000, 0, 0, 400, 400, 0, 0, 0, 0);
	clayermatwnd::create_layerlist();
}


// *
// *

/*
 * custom shader constants within model previewer:
 * hook call to "R_DrawXModelSkinnedUncached_2" @ 0x4FEB29 to flip bool on
 * ^ call, then flip bool off
 * hooks for vertex and shader constants - check if bool is true -> only affects model previewer
 */

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
							game::vec4_t get_temp;
							game::dx->device->GetVertexShaderConstantF(arg_def->dest, get_temp, 1);
							
							game::vec4_t temp = { 1.0f, 0.4f, 0.4f, 1.0f };
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


/*
 *  vertexshader + pixelshader loading (allows to load shaders that are not included in the shader_names file; TODO: mod linker to do the same -> no shader_tool)
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


// *
// *

void clayermatwnd::hooks()
{
	utils::hook(0x422694, clayermatwnd::create_layermatwnd, HOOK_CALL).install()->quick();

	// custom shader constants for model previewer
	utils::hook(0x4FEB29, r_draw_xmodel_skinned_uncached_stub, HOOK_JUMP).install()->quick();
	utils::hook(0x53BC39, R_SetPassPixelShaderStableArguments_stub, HOOK_JUMP).install()->quick();
	utils::hook(0x53B9E3, R_SetPassShaderObjectArguments_stub, HOOK_JUMP).install()->quick();

	utils::hook(0x5188A6, r_create_pixelshader, HOOK_CALL).install()->quick();

	utils::hook::nop(0x51873F, 17);
		 utils::hook(0x51873F, r_create_vertexshader_stub, HOOK_JUMP).install()->quick();

	//components::command::register_command("reload_techset", [](auto)
	//	{
	//		//utils::hook::call<void(__cdecl)(const char*, int)>(0x5199E0)("fakelight_view_dtex", 1);
	//	});
}