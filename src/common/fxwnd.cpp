#include "std_include.hpp"


void cfxwnd::stop_effect()
{
	m_effect_initial_trigger = false;
	m_effect_is_playing = false;
	m_raw_effect.name[0] = 0;
	m_raw_effect.elemCount = 0;

	if (m_active_effect)
	{
		const auto system = fx_system::FX_GetSystem(fx_system::FX_SYSTEM_BROWSER);

		{
			const auto stat = fx_system::FX_GetEffectStatus(m_active_effect);
			fx_system::FX_DelRefToEffect(system, m_active_effect);

			if (!stat)
			{
				fx_system::FX_KillEffect(system, m_active_effect);
			}

			m_active_effect = nullptr;
		}
	}
}

void cfxwnd::retrigger_effect(int msecBegin)
{
	if (m_active_effect)
	{
		fx_system::FX_RetriggerEffect(fx_system::FX_SYSTEM_BROWSER, m_active_effect, msecBegin);
	}
}

bool cfxwnd::load_effect(const char* effect_name)
{
	if (effect_name && *effect_name)
	{
		std::string fxname = effect_name;
		utils::replace(fxname, ".efx", "");

		//cfxwnd::stop();
		fx_system::FX_UnregisterAll();

		if (fx_system::FX_LoadEditorEffect(fxname.c_str(), &m_raw_effect))
		{
			//fx_system::ed_is_editor_effect_valid = true;
			game::printf_to_console("[FX-BROWSER] loaded editor effect [%s]\n", effect_name);

			return true;
		}

		//fx_system::ed_is_editor_effect_valid = false;
		game::printf_to_console("[FX-BROWSER] failed to load editor effect [%s]\n", effect_name);

		m_raw_effect.name[0] = 0;
		m_raw_effect.elemCount = 0;
	}
	
	return false;
}

float cfx_spawn_axis[3][3] =
{
	{  0.0f,  0.0f,  1.0f },
	{  0.0f, -1.0f,  0.0f },
	{  1.0f,  0.0f,  0.0f }
};

void cfxwnd::setup_and_spawn_fx()
{
	if (strlen(m_raw_effect.name))
	{
		fx_system::FxEffectDef* def = fx_system::FX_Convert(&m_raw_effect, fx_system::FX_AllocMem);

		if (!def)
		{
			// can happen if all elemdefs are disabled
			return;
		}

		utils::vector::angle_vectors(m_angles, cfx_spawn_axis[2], cfx_spawn_axis[1], cfx_spawn_axis[0]);

		m_effect_is_playing = true;
		const auto effect = components::effects::Editor_SpawnEffect(fx_system::FX_SYSTEM_BROWSER, def, m_tickcount_playback, game::vec3_origin, cfx_spawn_axis, fx_system::FX_SPAWN_MARK_ENTNUM);
		m_active_effect = effect;
	}
}



void cfxwnd::tick_playback()
{
	const auto saved_tick = static_cast<int>(GetTickCount());
	auto tick_cmp = static_cast<int>(GetTickCount()) - m_tickcount_repeat;

	if (tick_cmp > 200)
	{
		tick_cmp = 200;
	}

	m_tickcount_repeat = saved_tick;

	//const bool force_update = GET_GUI(ggui::camera_settings_dialog)->phys_force_frame_logic;
	//if (effects::effect_is_playing() || (force_update && !effects::effect_is_paused()))
	{
		auto tick_inc = static_cast<int>(static_cast<double>(tick_cmp) * 1.0 /*timescale*/ + 9.313225746154785e-10);
		if (tick_inc <= 1)
		{
			tick_inc = 1;
		}

		m_tickcount_playback += tick_inc;
	}
}

void cfxwnd::update_fx()
{
	cfxwnd::tick_playback();
	fx_system::FX_SetNextUpdateTime(fx_system::FX_SYSTEM_BROWSER, m_tickcount_playback);

	// FX_SetupCamera_Radiant
	const auto system = fx_system::FX_GetSystem(fx_system::FX_SYSTEM_BROWSER);

	float axis[3][3] = {};
	axis[0][0] = m_vpn[0];
	axis[0][1] = m_vpn[1];
	axis[0][2] = m_vpn[2];
	axis[1][0] = -m_vright[0];
	axis[1][1] = -m_vright[1];
	axis[1][2] = -m_vright[2];
	axis[2][0] = m_vup[0];
	axis[2][1] = m_vup[1];
	axis[2][2] = m_vup[2];

	const float halfTanY = tanf(60.0f * 0.01745329238474369f * 0.5f) * 0.75f;
	const float halfTanX = halfTanY * (static_cast<float>(m_width) / static_cast<float>(m_height));

	FX_SetupCamera(&system->camera, m_origin, axis, halfTanX, halfTanY, 0.0f);

	// ----

	if (m_active_effect && !m_effect_initial_trigger)
	{
		m_effect_initial_trigger = true;
		m_effect_is_playing = true;
		m_tickcount_repeat = GetTickCount();

		fx_system::FX_RetriggerEffect(fx_system::FX_SYSTEM_BROWSER, m_active_effect, m_tickcount_playback);
	}

	if (!m_active_effect)
	{
		cfxwnd::setup_and_spawn_fx();
		if (!m_active_effect)
		{
			m_effect_is_playing = false;
		}
	}

	if (m_active_effect)
	{
		//if (!effects::effect_is_repeating())
		{
			if (fx_system::FX_GetEffectStatus(m_active_effect))
			{
				cfxwnd::stop_effect();
			}
		}

		// retrigger active effect after completion
		if ((m_active_effect->status & fx_system::FX_STATUS_HAS_PENDING_LOOP_ELEMS) == 0)
		{
			if (m_active_effect->firstSortedElemHandle == UINT16_MAX)
			{
				const auto system = fx_system::FX_GetSystem(fx_system::FX_SYSTEM_BROWSER);

				if (!system->activeElemCount)
				{
					//game::printf_to_console("retrigger, status %d\n", m_active_effect->status);
					fx_system::FX_RetriggerEffect(fx_system::FX_SYSTEM_BROWSER, m_active_effect, m_tickcount_playback);
				}
			}
		}

		fx_system::FxCmd cmd = {};
		FX_FillUpdateCmd(fx_system::FX_SYSTEM_BROWSER, &cmd);
		Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_NON_DEPENDENT, &cmd);
		Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_SPOT_LIGHT, &cmd);
		Sys_DoWorkerCmd(fx_system::WRKCMD_UPDATE_FX_REMAINING, &cmd);
	}
}

struct fx_grid_line
{
	game::GfxPointVertex pts[2];
};

struct fx_pt
{
	float xyz[3];
};

std::vector<fx_pt> fx_grid_pts(512);
std::vector<fx_grid_line> fx_grid(512);

void cfxwnd::draw_grid()
{
	fx_grid.clear();
	fx_grid_pts.clear();

	game::vec4_t grid_color = { 0.25f, 0.25f, 0.25f, 1.0f };
	const auto grid_sections = /*sections:*/ 17 /**/ - 1;
	const auto grid_scale = 64.0f;

	for (int j = 0; j <= grid_sections; ++j)
	{
		for (int i = 0; i <= grid_sections; ++i)
		{
			fx_grid_pts.emplace_back(fx_pt
			{
				static_cast<float>(i) * grid_scale - grid_scale * static_cast<float>(grid_sections) * 0.5f,
				static_cast<float>(j) * grid_scale - grid_scale * static_cast<float>(grid_sections) * 0.5f,
				0.0f
			});
		}
	}

	game::GfxColor col = {};
	game::Byte4PackPixelColor(grid_color, &col);

	fx_grid_line line = {};
	line.pts[0].color = col;
	line.pts[1].color = col;

	for (int j = 0; j < grid_sections; ++j)
	{
		for (int i = 0; i < grid_sections; ++i)
		{
			const int row1 = j * (grid_sections + 1);
			const int row2 = (j + 1) * (grid_sections + 1);

			utils::vector::copy(fx_grid_pts[row1 + i].xyz, line.pts[0].xyz);
			utils::vector::copy(fx_grid_pts[row1 + i + 1].xyz, line.pts[1].xyz);
			fx_grid.emplace_back(line);

			utils::vector::copy(fx_grid_pts[row1 + i + 1].xyz, line.pts[0].xyz);
			utils::vector::copy(fx_grid_pts[row2 + i + 1].xyz, line.pts[1].xyz);
			fx_grid.emplace_back(line);

			utils::vector::copy(fx_grid_pts[row2 + i + 1].xyz, line.pts[0].xyz);
			utils::vector::copy(fx_grid_pts[row2 + i].xyz, line.pts[1].xyz);
			fx_grid.emplace_back(line);

			utils::vector::copy(fx_grid_pts[row2 + i].xyz, line.pts[0].xyz);
			utils::vector::copy(fx_grid_pts[row1 + i].xyz, line.pts[1].xyz);
			fx_grid.emplace_back(line);
		}
	}

	components::renderer::R_AddLineCmd(static_cast<std::uint16_t>(fx_grid.size()), 3, 3, fx_grid[0].pts);

	game::vec3_t pxs_x = { 0.5f, 0.0f, 0.0f };
	game::vec3_t pxs_y = { 0.0f, -0.5f, 0.0f };
	game::vec4_t txt_col = { 0.35f, 0.35f, 0.35f, 1.0f };

	
	if (const auto font = game::R_RegisterFont("fonts/smalldevfont", 1); font)
	{
		for (auto s = 0u; s < fx_grid.size(); s++)
		{
			if (!(s % 16))
			{
				const char* txt = utils::va("%.1f %.1f", fx_grid[s].pts[0].xyz[0], fx_grid[s].pts[0].xyz[1]);
				components::renderer::R_AddCmdDrawTextAtPosition(txt, font /*game::g_qeglobals->d_font_list*/, fx_grid[s].pts[0].xyz, pxs_x, pxs_y, txt_col);
			}
		}
	}
}

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

	if (clear)
	{
		float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::R_Clear(6, white, 1.0f, 0);
	}



	// a bit of R_RenderScene

	const auto frontEndDataOut = game::get_frontenddata();
	const auto viewInfo = &frontEndDataOut->viewInfo[0];

	frontEndDataOut->viewInfoIndex = 0;
	frontEndDataOut->viewInfoCount = 1;

	memcpy(&viewInfo->input, game::gfxCmdBufInput, sizeof(viewInfo->input));
	viewInfo->input.data = frontEndDataOut;
	viewInfo->sceneDef = game::scene->def;

	memcpy(&viewInfo->viewParms, view_parms, sizeof(game::GfxViewParms));
	viewInfo->viewParms.zNear = game::Dvar_FindVar("r_zNear")->current.value;

	const auto gfx_window = components::renderer::get_window(components::renderer::CFXWND);
	const game::GfxViewport viewport = { 0, 0, gfx_window->width, gfx_window->height };

	viewInfo->sceneViewport = viewport;
	viewInfo->displayViewport = viewport;

	// needed for debug plumes (3D text in space)
	game::rg->debugViewParms = view_parms;

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

	components::renderer::R_InitDrawSurfListInfo(&viewInfo->litInfo);

	viewInfo->litInfo.baseTechType = game::TECHNIQUE_FAKELIGHT_NORMAL;
	viewInfo->litInfo.viewInfo = viewInfo;
	viewInfo->litInfo.viewOrigin[0] = view_parms->origin[0];
	viewInfo->litInfo.viewOrigin[1] = view_parms->origin[1];
	viewInfo->litInfo.viewOrigin[2] = view_parms->origin[2];
	viewInfo->litInfo.viewOrigin[3] = view_parms->origin[3];
	viewInfo->litInfo.cameraView = 1;

	const int initial_lit_drawSurfCount = frontEndDataOut->drawSurfCount;

	// R_MergeAndEmitDrawSurfLists
	utils::hook::call<void(__cdecl)(int, int)>(0x549F50)(0, 3);

	viewInfo->litInfo.drawSurfs = &frontEndDataOut->drawSurfs[initial_lit_drawSurfCount];
	viewInfo->litInfo.drawSurfCount = frontEndDataOut->drawSurfCount - initial_lit_drawSurfCount;



	// R_SortDrawSurfs
	utils::hook::call<void(__cdecl)(game::GfxDrawSurf*, signed int)>(0x54D750)(game::scene->drawSurfs[6], game::scene->drawSurfCount[6]);

	components::renderer::R_InitDrawSurfListInfo(&viewInfo->decalInfo);
	viewInfo->decalInfo.baseTechType = game::TECHNIQUE_FAKELIGHT_NORMAL;
	viewInfo->decalInfo.viewInfo = viewInfo;
	viewInfo->decalInfo.viewOrigin[0] = view_parms->origin[0];
	viewInfo->decalInfo.viewOrigin[1] = view_parms->origin[1];
	viewInfo->decalInfo.viewOrigin[2] = view_parms->origin[2];
	viewInfo->decalInfo.viewOrigin[3] = view_parms->origin[3];
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
	components::renderer::R_InitDrawSurfListInfo(&viewInfo->emissiveInfo);

	viewInfo->emissiveInfo.baseTechType = game::TECHNIQUE_EMISSIVE;
	viewInfo->emissiveInfo.viewInfo = viewInfo;
	viewInfo->emissiveInfo.viewOrigin[0] = view_parms->origin[0];
	viewInfo->emissiveInfo.viewOrigin[1] = view_parms->origin[1];
	viewInfo->emissiveInfo.viewOrigin[2] = view_parms->origin[2];
	viewInfo->emissiveInfo.viewOrigin[3] = view_parms->origin[3];
	viewInfo->emissiveInfo.cameraView = 1;

	const int initial_emissive_drawSurfCount = frontEndDataOut->drawSurfCount;

	// R_MergeAndEmitDrawSurfLists
	utils::hook::call<void(__cdecl)(int, int)>(0x549F50)(9, 6);

	emissiveList->drawSurfs = &frontEndDataOut->drawSurfs[initial_emissive_drawSurfCount];
	viewInfo->emissiveInfo.drawSurfCount = frontEndDataOut->drawSurfCount - initial_emissive_drawSurfCount;
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

	const auto gui = GET_GUI(ggui::effects_browser);
	
	build_matrix();

	fxwnd->m_width = components::renderer::get_window(components::renderer::CFXWND)->width;
	fxwnd->m_height = components::renderer::get_window(components::renderer::CFXWND)->height;

	if (fxwnd->m_width != 0 && fxwnd->m_height != 0)
	{
		{
			if (gui->is_active() && !gui->is_inactive_tab())
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
					
					cfxwnd::get()->update_fx();

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
					origin[0] = fxwnd->m_origin[0];//gui->m_camera_distance; //m_selector->camera_offset[0]; //cmainframe::activewnd->m_pCamWnd->camera.origin[0];
					origin[1] = fxwnd->m_origin[1]; //m_selector->camera_offset[1]; //cmainframe::activewnd->m_pCamWnd->camera.origin[1];
					origin[2] = fxwnd->m_origin[2];

					float temp_angles[3];
					temp_angles[0] = 0.0f;
					temp_angles[1] = -70.0f;
					temp_angles[2] = 40.0f;

					const auto gfx_window = components::renderer::get_window(static_cast<components::renderer::GFXWND_>(game::dx->targetWindowIndex));
					setup_scene(origin, axis, 0, 0, gfx_window->width, gfx_window->height);

					cfxwnd::get()->draw_grid();
#if 0
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
					utils::vector::ma(model_origin, model->radius * 1.75f + gui->m_camera_distance, direction, model_orientation.origin);
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
#endif
				}

				// sorts surfaces and adds RC_DRAW_EDITOR_SKINNEDCACHED rendercmd
				//utils::hook::call<void(__cdecl)()>(0x4FDA10)();

				game::R_EndFrame();
				game::R_IssueRenderCommands(-1);
				game::R_SortMaterials(); // !!! clear scene and buffers or camera will preview flickering effect browser effects

				// big fat mem leak was caused by renderer::copy_scene_to_texture in RB_StandardDrawCommands



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
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD /*| WS_POPUP | WS_VISIBLE*/, // WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_POPUP; // WS_VISIBLE
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