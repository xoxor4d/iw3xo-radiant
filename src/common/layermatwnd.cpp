#include "std_include.hpp"

layermatwnd_s* layermatwnd_struct = reinterpret_cast<layermatwnd_s*>(0x181F500);
camera_s layercam = {};

void build_matrix()
{
	float angles[3];

	angles[0] = layercam.angles[0];
	angles[1] = layercam.angles[1];
	angles[2] = layercam.angles[2];
	angles[0] = -angles[0];

	//AngleVectors(angles, layercam.vpn, layercam.vright, layercam.vup);
	utils::hook::call<void(__cdecl)(float* _angles, float* _vpn, float* _right, float* _up)>(0x4ABD70)(angles, layercam.vpn, layercam.vright, layercam.vup);

	float xa = layercam.angles[1] * sin(1.0f);
	float xb = cos(xa);
	float xc = sin(xa);

	if (layercam.forward)
	{
		layercam.forward[0] = xb;
		layercam.forward[1] = xc;
		layercam.forward[2] = 0.0f;
	}
	if (layercam.right)
	{
		layercam.right[0] = xc;
		layercam.right[1] = -xb;
		layercam.right[2] = 0.0f;
	}
}

void R_AddCmdSetViewportValues(int x, int y, int width, int height)
{
	game::GfxCmdSetViewport* cmd;

	if (width <= 0)
	{
		game::Com_Error("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_rendercmds.cpp", 1677, 0, "%s\n\t(width) = %i", "(width > 0)", width);
	}
	if (height <= 0)
	{
		game::Com_Error("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_rendercmds.cpp", 1678, 0, "%s\n\t(height) = %i", "(height > 0)", height);
	}
	
	cmd = reinterpret_cast<game::GfxCmdSetViewport*>(game::R_GetCommandBuffer(20, 7)); // RC_SET_VIEWPORT
	if (!cmd)
	{
		game::Com_Error("C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\gfx_d3d\\r_rendercmds.cpp", 1682, 0, "%s", "cmd");
	}

	cmd->viewport.height = height;
	cmd->viewport.x = x;
	cmd->viewport.y = y;
	cmd->viewport.width = width;

}

// rewrite of R_SetSceneParms
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
	R_AddCmdSetViewportValues(x, y, width, height);

	if(clear)
	{
		float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::R_Clear(6, white, 1.0f, 0);
	}
}

void setup_scene(float* origin, float* axis, int x, int y, int width, int height)
{
	//float fov_y = tan(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f;
	float fov_y = tan(ggui::rtt_model_preview.fov * 0.01745329238474369f * 0.5f) * 0.75f;
	float fov_x = fov_y * ((float)layercam.width / (float)layercam.height);
	
	game::GfxMatrix projection = {};
	
	// R_SetupProjection
	//utils::hook::call<void(__cdecl)(game::GfxMatrix*, float halfx, float halfy, float znear)>(0x4A78E0)(&projection, fov_x, fov_y, 0.0099999998f);
	game::R_SetupProjection(&projection, fov_x, fov_y, 0.0099999998f);
	
	// R_SetSceneParms(org, axis, &projection);
	//utils::hook::call<void(__cdecl)(float* _origin, float* _axis, game::GfxMatrix* mtx)>(0x5062B0)(origin, axis, &projection);

	set_scene_params(origin, axis, &projection, x, y, width, height, false);
}

//typedef void(__stdcall* draw_crosshair_t)(cxywnd* wnd);
//draw_crosshair_t draw_crosshair = reinterpret_cast<draw_crosshair_t>(0x4655D0);
float yaw_angle = 0.0f;

static float editor_instmodel_mtx[] =
{
	0.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 0.0f
};

void clayermatwnd::on_paint()
{
	PAINTSTRUCT Paint;
	BeginPaint(layermatwnd_struct->m_content_hwnd, &Paint);

	// R_CheckHwnd_or_Device
	if (!utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(layermatwnd_struct->m_content_hwnd))
	{
		EndPaint(layermatwnd_struct->m_content_hwnd, &Paint);
		return;
	}

	// *
	// setup scene
	
	build_matrix();

	yaw_angle += 0.1f;
	if (yaw_angle > 360.0f) {
		yaw_angle -= 360.0f;
	}

	layercam.width = game::dx->windows[ggui::LAYERED].width;
	layercam.height = game::dx->windows[ggui::LAYERED].height;
	
	if (layercam.width != 0 && layercam.height != 0)
	{
		{
			if (ggui::rtt_model_preview.menustate && !ggui::rtt_model_preview.model_name.empty())
			{
				// get xmodel handle
				auto model = utils::hook::call<game::XModel* (__cdecl)(const char*)>(0x51D450)(ggui::rtt_model_preview.model_name.c_str());
				if  (model)
				{
					if (model->surfs->vertCount > 15000) {
						model = utils::hook::call<game::XModel* (__cdecl)(const char*)>(0x51D450)("fx");
					}
					
					game::R_BeginFrame();
					game::R_Clear(7, game::g_qeglobals->d_savedinfo.colors[0], 1.0f, 0);


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
					origin[0] = 0.0f; //cmainframe::activewnd->m_pCamWnd->camera.origin[0];
					origin[1] = 0.0f; //cmainframe::activewnd->m_pCamWnd->camera.origin[1];
					origin[2] = 0.0f; //cmainframe::activewnd->m_pCamWnd->camera.origin[2];
					

					
					setup_scene(origin, axis, 0, 0, game::dx->windows[game::dx->targetWindowIndex].width, game::dx->windows[game::dx->targetWindowIndex].height);

					//game::R_SortMaterials();

					float angles[3] = { 0.0f, yaw_angle, 0.0f };
					float model_axis[4][3] = {};
					float temp_axis[4][3] = {};

					game::AnglesToAxis(angles, &(*temp_axis)[3]);

					
					

					float dist = (fabsf(model->maxs[2]) + fabsf(model->mins[2])) * 0.5f;

					temp_axis[0][0] = model->radius * 1.75f;
					temp_axis[0][1] = 0.0f;
					temp_axis[0][2] = -(model->maxs[2] - dist); //model->mins[2] - (model->maxs[2] * 0.5f);

					// calc axis for model inst
					utils::hook::call<void(__cdecl)(float*, float*, float*)>(0x4BA7D0)(&temp_axis[0][0], editor_instmodel_mtx, &model_axis[0][0]);

					// is the last model still the selected one? -> update the model instance
					if (ggui::rtt_model_preview.model_ptr && ggui::rtt_model_preview.model_ptr == model)
					{
						// ModelInstUpdate
						utils::hook::call<void(__cdecl)(int _inst, float* _axis, float scale)>(0x4FDD80)
								(ggui::rtt_model_preview.model_inst_handle, &model_axis[0][0], 1.0f);
					}
					else
					{
						// clear old model inst
						if (ggui::rtt_model_preview.model_ptr && ggui::rtt_model_preview.model_inst_handle)
						{
							utils::hook::call<void(__cdecl)(int)>(0x4FDCE0)(ggui::rtt_model_preview.model_inst_handle);
						}

						// AddModelToModelInstBuff
						int model_inst_num = utils::hook::call<int(__cdecl)(game::XModel* _model, float* _axis, float _scale)>(0x4FDBE0)(
							model, &model_axis[0][0], 1.0f);

						if (model_inst_num)
						{
							ggui::rtt_model_preview.model_ptr = model;
							ggui::rtt_model_preview.model_inst_handle = model_inst_num;
						
						}
					}
				}

				if (ggui::rtt_model_preview.model_ptr)
				{
					// SkinModelInst
					utils::hook::call<int(__cdecl)(int inst_handle, int checkhandle, int techflags, game::GfxColor* color, int drawflags)>(0x4FE2E0)(
						ggui::rtt_model_preview.model_inst_handle, 0, 4, nullptr, 2);
				}

#if 0 // draw world brushes/models etc
				char zero[4];
				zero[0] = '\0';
				zero[1] = '\0';
				zero[2] = '\0';
				zero[3] = '\0';

				game::GfxColor col = {};
				float ccol[4];
				ccol[0] = 0.2f;
				ccol[1] = 0.89999998f;
				ccol[2] = 0.2f;
				ccol[3] = 0.80000001f;

				game::Byte4PackPixelColor(ccol, &col);

				//// draw a brush
				for (auto	sb = game::g_active_brushes_next();
					(DWORD*)sb != game::active_brushes_ptr;
					sb = sb->next)
				{
					// draw triangles
					utils::hook::call<void(__cdecl)()>(0x4FDA10)();

					game::R_SortMaterials();

					utils::hook::call<void(__cdecl)(game::selbrush_t*, float* mtx, int, int, int, int, int, game::GfxColor*, int, int, char*)>(0x47AFC0)(
						sb, editor_instmodel_mtx, -1, 0, 4, 0, 4, &col, 4, 2, zero);
				}
#endif

				// draw triangles
				utils::hook::call<void(__cdecl)()>(0x4FDA10)();

				game::R_EndFrame();
				game::R_IssueRenderCommands(-1);
				game::R_SortMaterials();
				components::renderer::copy_scene_to_texture(ggui::LAYERED, ggui::rtt_model_preview.scene_texture);
			}
		}

#if 0
		// Rendering a second time is slow as fuck
		{
			game::GfxColor col = {};
			float ccol[4];
			ccol[0] = 0.2f;
			ccol[1] = 0.89999998f;
			ccol[2] = 0.2f;
			ccol[3] = 0.80000001f;

			game::Byte4PackPixelColor(ccol, &col);

			char zero[4];
			zero[0] = '\0';
			zero[1] = '\0';
			zero[2] = '\0';
			zero[3] = '\0';

			//// draw a brush
			if (auto b = game::g_selected_brushes();
				b && b->currSelection)
			{
				utils::hook::call<void(__cdecl)()>(0x4FDA10)();
				utils::hook::call<void(__cdecl)()>(0x4FD910)();

				utils::hook::call<void(__cdecl)(game::selbrush_t*, float* mtx, int, int, int, int, int, game::GfxColor*, int, int, char*)>(0x47AFC0)(
					b, editor_instmodel_mtx, -1, 0, 29, 0, 29, &col, 4, 2, zero);
			}

			utils::hook::call<void(__cdecl)()>(0x4FDA10)();

			GfxPointVertex verts[2] = {};

			verts[0].xyz[0] = 0.0f;
			verts[0].xyz[1] = 0.0f;
			verts[0].xyz[2] = 0.0f;
			game::Byte4PackPixelColor(ccol, &verts[0].color);

			verts[1].xyz[0] = -131072.0f;
			verts[1].xyz[1] = 131072.0f;
			verts[1].xyz[2] = 0.0f;
			game::Byte4PackPixelColor(ccol, &verts[1].color);

			// render a line
			utils::hook::call<void(__cdecl)(std::int16_t _lines, std::int16_t, GfxPointVertex* buff)>(0x4FD1A0)(1, 2, &verts[0]);
			//draw_crosshair(cmainframe::activewnd->m_pXYWnd);

			game::R_EndFrame();
			game::R_IssueRenderCommands(-1);
			game::R_SortMaterials();
			components::renderer::copy_scene_to_texture(ggui::LAYERED, ggui::rtt_model02);
		}
#endif
	}
	
	// R_CheckTargetWindow
	utils::hook::call<void(__cdecl)(HWND)>(0x500660)(layermatwnd_struct->m_content_hwnd);

	int& random_dword01 = *reinterpret_cast<int*>(0x25D5B88);
	int& random_dword02 = *reinterpret_cast<int*>(0x242293C);
	int& random_dword03 = *reinterpret_cast<int*>(0x2422940);
	
	if(!random_dword01)
	{
		__debugbreak();
	}

	random_dword03 = random_dword02;

	// nice meme IW
	EndPaint(layermatwnd_struct->m_content_hwnd, &Paint);
}


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


void clayermatwnd::create_layerlist()
{
	//const auto prefs = game::g_PrefsDlg();
	//DWORD window_style;

	//if(prefs->m_nView == 1 && prefs->detatch_windows)
	//{
	//	window_style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD | WS_POPUP;
	//}
	//else
	//{
	//	window_style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD;
	//}
	//
	//layermatwnd_struct->m_hWnd = CreateWindowExA(
	//	0, // WS_EX_COMPOSITED
	//	"LayeredMaterialList",
	//	0,
	//	window_style,
	//	0, 0,
	//	400, 400,
	//	cmainframe::activewnd->GetWindow(), 
	//	0, 0, 0);

	//window_style = WS_VISIBLE | WS_CHILD;

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

	// disable layer render stuff in on_paint
	//utils::hook::nop(0x417FC6, 5);
}