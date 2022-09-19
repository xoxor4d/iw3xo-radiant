#include "std_include.hpp"

//cfxwnd* cfxwnd::p_this = nullptr;

// rewrite of R_SetSceneParms (not of much use anymore)
void set_scene_params(const float* origin, float* axis, game::GfxMatrix* projection, int x, int y, int width, int height, bool clear)
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
	game::R_AddCmdSetViewportValues(x, y, width, height);

	if(clear)
	{
		float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::R_Clear(6, white, 1.0f, 0);
	}
}

void setup_scene(float* origin, float* axis, int x, int y, int width, int height)
{
	const float fov_y = tan(60.0f * 0.01745329238474369f * 0.5f) * 0.75f;
	const float fov_x = fov_y * (static_cast<float>(cfxwnd::get()->m_width) / static_cast<float>(cfxwnd::get()->m_height));
	
	game::GfxMatrix projection = {};
	
	game::R_SetupProjection(&projection, fov_x, fov_y, 4.0f);
	set_scene_params(origin, axis, &projection, x, y, width, height, false);
}

void calc_raydir(int x, int y, float* dir)
{
	const auto fxwnd = cfxwnd::get();

	const float tan_half_y = tan(60.0f * 0.01745329238474369f * 0.5f);
	const float tan_half_x = (tan_half_y * 0.75f + tan_half_y * 0.75f) / static_cast<float>(fxwnd->m_height);

	const float xa = tan_half_x * (static_cast<float>(y) - (static_cast<float>(fxwnd->m_height) / 2.0f));
	const float xb = tan_half_x * (static_cast<float>(x) - (static_cast<float>(fxwnd->m_width) / 2.0f));

	dir[0] = fxwnd->m_vup[0] * xa + fxwnd->m_vright[0] * xb + fxwnd->m_vpn[0];
	dir[1] = fxwnd->m_vup[1] * xa + fxwnd->m_vright[1] * xb + fxwnd->m_vpn[1];
	dir[2] = fxwnd->m_vup[2] * xa + fxwnd->m_vright[2] * xb + fxwnd->m_vpn[2];

	utils::vector::normalize(dir);
}

void build_matrix()
{
	const auto fxwnd = cfxwnd::get();

	float angles[3];

	angles[0] = fxwnd->m_angles[0];
	angles[1] = fxwnd->m_angles[1];
	angles[2] = fxwnd->m_angles[2];
	angles[0] = -angles[0];

	game::AngleVectors(angles, fxwnd->m_vpn, fxwnd->m_vright, fxwnd->m_vup);

	const float xa = fxwnd->m_angles[1] * sin(1.0f);
	const float xb = cos(xa);
	const float xc = sin(xa);

	fxwnd->m_forward[0] = xb;
	fxwnd->m_forward[1] = xc;
	fxwnd->m_forward[2] = 0.0f;

	fxwnd->m_right[0] = xc;
	fxwnd->m_right[1] = -xb;
	fxwnd->m_right[2] = 0.0f;
}

// *
// ggui::modelselector::menu()
// drag drop target => cxywnd::rtt_grid_window()
// drag drop target => ccamwnd::rtt_camera_window()

// rendering happens in 0x533880
// model inst rendering in 0x53AA30

void cfxwnd::on_paint()
{
	const auto fxwnd = cfxwnd::get();
	const auto& hwnd = components::renderer::get_window(components::renderer::CFXWND)->hwnd;

	PAINTSTRUCT Paint;
	BeginPaint(hwnd, &Paint);

	if (!game::R_SetupRendertarget_CheckDevice(hwnd))
	{
		EndPaint(hwnd, &Paint);
		return;
	}

	// *
	// 
	
	build_matrix();
	
	if (fxwnd->m_width != 0 && fxwnd->m_height != 0)
	{
		{
			//if (gui->is_active() && !gui->is_inactive_tab() && !gui->m_preview_model_name.empty())
			{
				//gui->m_bad_model = false;

				// get model handle
				if(auto model = game::R_RegisterModel("fx");
						model)
				{
					// use the fx model for invalid or vertex heavy models
					if (  !model->surfs 
						|| model->bad
						|| model->surfs->vertCount > 15000)
					{
						model = game::R_RegisterModel("fx");
						//gui->m_bad_model = true;
					}

					// begin a new frame, clear the scene
					game::R_BeginFrame();
					game::R_Clear(7, dvars::gui_window_bg_color->current.vector, 1.0f, false);

					// setup scene
					float axis[9];
					axis[0] = fxwnd->m_vpn[0];
					axis[1] = fxwnd->m_vpn[1];
					axis[2] = fxwnd->m_vpn[2];
					axis[3] = -fxwnd->m_vright[0];
					axis[4] = -fxwnd->m_vright[1];
					axis[5] = -fxwnd->m_vright[2];
					axis[6] = fxwnd->m_vup[0];
					axis[7] = fxwnd->m_vup[1];
					axis[8] = fxwnd->m_vup[2];

					float origin[3];
					origin[0] = 0.0f; //m_selector->camera_offset[0]; //cmainframe::activewnd->m_pCamWnd->camera.origin[0];
					origin[1] = 15.0f; //m_selector->camera_offset[1]; //cmainframe::activewnd->m_pCamWnd->camera.origin[1];
					origin[2] = 0.0f;

					float temp_angles[3];
					temp_angles[0] = 0.0f;
					temp_angles[1] = -70.0f;
					temp_angles[2] = 40.0f;

					const auto gfx_window = components::renderer::get_window(static_cast<components::renderer::GFXWND_>(game::dx->targetWindowIndex));
					setup_scene(origin, axis, 0, 0, gfx_window->width, gfx_window->height);

#if 0
					float dir_out[3] = {};
					
					// AngleVectors(float* angles, float* vpn, float* right, float* up)
					utils::hook::call<void(__cdecl)(float* angles, float* vpn, float* right, float* up)>(0x4ABD70)(ggui::preferences::modelpreview_sun_dir, dir_out, nullptr, nullptr);

					components::renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_SUN_POSITION, dir_out[0], dir_out[1], dir_out[2], 1.0f);
					components::renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_SUN_DIFFUSE, ggui::preferences::modelpreview_sun_diffuse[0], ggui::preferences::modelpreview_sun_diffuse[1], ggui::preferences::modelpreview_sun_diffuse[2], 1.0f);
					components::renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_SUN_SPECULAR, ggui::preferences::modelpreview_sun_specular[0], ggui::preferences::modelpreview_sun_specular[1], ggui::preferences::modelpreview_sun_specular[2], ggui::preferences::modelpreview_sun_specular[3]);
					components::renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_LIGHT_SPOTDIR, ggui::preferences::modelpreview_material_specular[0], ggui::preferences::modelpreview_material_specular[1], ggui::preferences::modelpreview_material_specular[2], ggui::preferences::modelpreview_material_specular[3]);
					components::renderer::R_AddCmdSetCustomShaderConstant(game::CONST_SRC_CODE_LIGHT_SPOTFACTORS, ggui::preferences::modelpreview_ambient[0], ggui::preferences::modelpreview_ambient[1], ggui::preferences::modelpreview_ambient[2], ggui::preferences::modelpreview_ambient[3]);
#endif
					
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
					calc_raydir(static_cast<int>(gfx_window->width * 0.5f), static_cast<int>(gfx_window->height * 0.5f), direction);

					game::vec3_t model_origin;
					utils::vector::copy(origin, model_origin);
					model_origin[1] = 10.0f;

					// calculate model origin including "zoom"
					utils::vector::ma(model_origin, model->radius * 1.75f + 40.0f, direction, model_orientation.origin);
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
						calc_raydir(static_cast<int>(gfx_window->width) - 35, 25, direction);

						// push the model into the scene so it does not clip with the camera
						utils::vector::ma(origin, axis_dist, direction, axis_orientation.origin);

						//if (!gui->m_axis_model_initiated)
						//{
						//	// get model handle
						//	if (const auto	axis_model = game::R_RegisterModel("axis");
						//					axis_model)
						//	{
						//		// AddModelToModelInstBuff - add the model to the instance "buffer"
						//		if (const int model_inst_num = utils::hook::call<int(__cdecl)(game::XModel * _model, float* _axis, float _scale)>(0x4FDBE0)(axis_model, &axis_orientation.origin[0], axis_scale);
						//					  model_inst_num)
						//		{
						//			gui->m_axis_model_inst_handle = model_inst_num;
						//		}

						//		gui->m_axis_model_initiated = true;
						//	}
						//}
						//else
						//{
						//	// ModelInstUpdate
						//	utils::hook::call<void(__cdecl)(int _inst, float* _axis, float scale)>(0x4FDD80)
						//		(gui->m_axis_model_inst_handle, &axis_orientation.origin[0], axis_scale);

						//	// SkinModelInst - add model surfs to the skinnedCached buffer
						//	utils::hook::call<int(__cdecl)(int inst_handle, int checkhandle, int techflags, game::GfxColor* color, int drawflags)>(0x4FE2E0)(
						//		gui->m_axis_model_inst_handle, 0, layermatwnd::rendermethod_axis, nullptr, 2);
						//}
					}

					
					// *
					// draw preview model
					{
						// is the last model still the selected one? -> update the model instance
						if (fxwnd->m_xmodel_ptr_test && fxwnd->m_xmodel_ptr_test == model)
						{
							// ModelInstUpdate
							utils::hook::call<void(__cdecl)(int _inst, float* _axis, float scale)>(0x4FDD80)
								(fxwnd->m_xmodel_inst, &model_orientation.origin[0], 1.0f);
						}
						// new model selected
						else
						{
							// reset camera zoom and angles
							/*gui->m_camera_angles[0] = 0.0f;
							gui->m_camera_angles[1] = 0.0f;
							gui->m_camera_angles[2] = 0.0f;
							gui->m_camera_distance = 0.0f;
							gui->m_camera_offset[0] = 0.0f;
							gui->m_camera_offset[1] = 0.0f;
							gui->m_camera_offset[2] = 0.0f;*/
							
							// RemoveModelInstFromBuf - clear old model instance
							if (fxwnd->m_xmodel_ptr_test && fxwnd->m_xmodel_inst)
							{
								utils::hook::call<void(__cdecl)(int)>(0x4FDCE0)(fxwnd->m_xmodel_inst);
							}

							// AddModelToModelInstBuff - add the model to the instance "buffer"
							if (const int model_inst_num = utils::hook::call<int(__cdecl)(game::XModel * _model, float* _axis, float _scale)>(0x4FDBE0)(model, &model_orientation.origin[0], 1.0f);
								model_inst_num)
							{
								fxwnd->m_xmodel_ptr_test = model;
								fxwnd->m_xmodel_inst = model_inst_num;
							}
						}

						if (fxwnd->m_xmodel_ptr_test)
						{
							// SkinModelInst - add model surfs to the skinnedCached buffer
							utils::hook::call<int(__cdecl)(int inst_handle, int checkhandle, int techflags, game::GfxColor* color, int drawflags)>(0x4FE2E0)(
								fxwnd->m_xmodel_inst, 0, layermatwnd::FAKELIGHT_NORMAL, nullptr, 2);
						}
					}
				}

				// sorts surfaces and adds RC_DRAW_EDITOR_SKINNEDCACHED rendercmd
				utils::hook::call<void(__cdecl)()>(0x4FDA10)();

				game::R_EndFrame();
				game::R_IssueRenderCommands(-1);

				//game::R_SortMaterials(); // not needed because only a single model
				//components::renderer::copy_scene_to_texture(components::renderer::CFXWND, gui->rtt_get_texture());
			}
		}
	}

	game::R_CheckTargetWindow(hwnd);
	
	// hunk related
	int& random_dword01 = *reinterpret_cast<int*>(0x25D5B88);
	int& random_dword02 = *reinterpret_cast<int*>(0x242293C);
	int& random_dword03 = *reinterpret_cast<int*>(0x2422940);
	
	if(!random_dword01) {
		game::Com_Error("cfxwnd::on_paint() :: s_hunkData");
	}

	random_dword03 = random_dword02;

	// nice meme iw
	EndPaint(hwnd, &Paint);
}


// window proc for the complete window
LRESULT __stdcall cfxwnd::windowproc_frame(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
LRESULT __stdcall cfxwnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
			cfxwnd::on_paint();
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
void cfxwnd::create_content_window()
{
	const auto fxwnd = cfxwnd::get();
	auto& hwnd = components::renderer::windows[components::renderer::CFXWND].hwnd;

	tagRECT rect;
	GetClientRect(fxwnd->m_frame_hwnd, &rect);

	hwnd = CreateWindowExA(
		0, // WS_EX_COMPOSITED
		"FxWindowContent",
		nullptr,
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD | WS_POPUP | WS_VISIBLE, // WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_POPUP; // WS_VISIBLE
		0, 0, rect.right - rect.left,rect.bottom - rect.top,
		fxwnd->m_frame_hwnd,
		nullptr, nullptr, nullptr);

	if (!hwnd)
	{
		game::Com_Error("Couldn't create fx content window");
	}
}

// register window classes (frame + content)
void cfxwnd::precreate_window()
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
	wc.lpszClassName = "FxWindow"; // frame
	wc.lpfnWndProc = cfxwnd::windowproc_frame;
	RegisterClassExA(&wc);

	wc.lpszClassName = "FxWindowContent"; // content
	wc.lpfnWndProc = cfxwnd::windowproc;
	RegisterClassExA(&wc);
}

void cfxwnd::create_fxwnd()
{
	cfxwnd::precreate_window();

	cfxwnd::get()->m_frame_hwnd = CreateWindowExA(WS_EX_PALETTEWINDOW, "FxWindow", "", WS_THICKFRAME | WS_SYSMENU | WS_DLGFRAME | WS_BORDER | 0x80000000, 0, 0, 400, 400, 0, 0, 0, 0);

	const auto fxwnd = cfxwnd::get();
	fxwnd->m_width = 400;
	fxwnd->m_height = 400;

	cfxwnd::create_content_window();
}

// *
// *

cfxwnd::cfxwnd()
{
	p_this = this;
	create_fxwnd();
}