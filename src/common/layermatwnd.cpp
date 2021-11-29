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

namespace layermatwnd
{
	int rendermethod_axis = 4;
	int rendermethod_preview = 24;
	float fov = 60.0f;
	bool rotation_pause = false;
}

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
	m_selector->camera_fov = layermatwnd::fov;
	
	// stop yaw animation if rotated manually
	if(!m_selector->user_rotation && !layermatwnd::rotation_pause)
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
					if(	  !model->surfs 
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
					origin[0] = 0.0f; //m_selector->camera_offset[0]; //cmainframe::activewnd->m_pCamWnd->camera.origin[0];
					origin[1] = 0.0f; //m_selector->camera_offset[1]; //cmainframe::activewnd->m_pCamWnd->camera.origin[1];
					origin[2] = m_selector->camera_offset[2]; //cmainframe::activewnd->m_pCamWnd->camera.origin[2];

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

					game::vec3_t model_origin;
					utils::vector::copy(origin, model_origin);
					model_origin[1] = m_selector->camera_offset[1];

					// calculate model origin including "zoom"
					utils::vector::ma(model_origin, model->radius * 1.75f + m_selector->camera_distance, direction, model_orientation.origin);
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

				// sorts surfaces and adds RC_DRAW_EDITOR_SKINNEDCACHED rendercmd
				utils::hook::call<void(__cdecl)()>(0x4FDA10)();

				game::R_EndFrame();
				game::R_IssueRenderCommands(-1);
				game::R_SortMaterials(); // not needed because only a single model
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

void clayermatwnd::hooks()
{
	utils::hook(0x422694, clayermatwnd::create_layermatwnd, HOOK_CALL).install()->quick();
}