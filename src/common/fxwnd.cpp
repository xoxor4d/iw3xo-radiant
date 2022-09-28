#include "std_include.hpp"

void cfxwnd::stop_effect()
{
	m_effect_is_using_physx = false;
	m_effect_is_playing = false;

	memset(&m_raw_effect, 0, sizeof(fx_system::FxEditorEffectDef));

	//m_raw_effect.name[0] = 0;
	//m_raw_effect.elemCount = 0;

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

		cfxwnd::stop_effect();
		fx_system::FX_UnregisterAll();

		if (fx_system::FX_LoadEditorEffect(fxname.c_str(), &m_raw_effect))
		{
			//fx_system::ed_is_editor_effect_valid = true;
			game::printf_to_console("[FX-BROWSER] loaded editor effect [%s]\n", effect_name);

			return true;
		}

		//fx_system::ed_is_editor_effect_valid = false;
		game::printf_to_console("[FX-BROWSER] failed to load editor effect [%s]\n", effect_name);

		//m_raw_effect.name[0] = 0;
		//m_raw_effect.elemCount = 0;

		memset(&m_raw_effect, 0, sizeof(fx_system::FxEditorEffectDef));
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
		m_effect_is_using_physx = false;

		fx_system::FxEffectDef* def = fx_system::FX_Convert(&m_raw_effect, fx_system::FX_AllocMem);

		if (!def)
		{
			// can happen if all elemdefs are disabled
			return;
		}

		game::vec3_t spawn_origin = {};

		for (int elemDefIndex = 0; elemDefIndex != def->elemDefCountLooping + def->elemDefCountOneShot + def->elemDefCountEmission; ++elemDefIndex)
		{
			if (def->elemDefs[elemDefIndex].elemType == fx_system::FX_ELEM_TYPE_MODEL && (def->elemDefs[elemDefIndex].flags & fx_system::FX_ELEM_USE_MODEL_PHYSICS) != 0)
			{
				spawn_origin[2] = 80.0f;
				m_effect_is_using_physx = true;
				break;
			}
		}

		m_effect_is_playing = true;

		const auto effect = components::effects::Editor_SpawnEffect(fx_system::FX_SYSTEM_BROWSER, def, m_tickcount_playback, spawn_origin, cfx_spawn_axis, fx_system::FX_SPAWN_MARK_ENTNUM);
		m_active_effect = effect;
	}
}


void cfxwnd::tick_playback()
{
	const auto saved_tick = static_cast<int>(GetTickCount());
	auto tick_cmp = static_cast<int>(GetTickCount()) - m_saved_tick_old;

	if (tick_cmp > 200)
	{
		tick_cmp = 200;
	}

	m_saved_tick_old = saved_tick;

	{
		auto tick_inc = static_cast<int>(static_cast<double>(tick_cmp) * 1.0 /*timescale*/ + 9.313225746154785e-10);
		if (tick_inc <= 1)
		{
			tick_inc = 1;
		}

		m_tickcount_playback += tick_inc;
	}
}

/**
 * @brief	general fx logic
 */
void cfxwnd::update_fx()
{
	const auto system = fx_system::FX_GetSystem(fx_system::FX_SYSTEM_BROWSER);

	cfxwnd::tick_playback();

	if (m_effect_is_using_physx)
	{
		components::physx_impl::get()->fx_browser_frame(m_tickcount_playback);
	}

	fx_system::FX_SetNextUpdateTime(fx_system::FX_SYSTEM_BROWSER, m_tickcount_playback);

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
				return;
			}
		}

		// retrigger active effect after completion
		if ((m_active_effect->status & fx_system::FX_STATUS_HAS_PENDING_LOOP_ELEMS) == 0)
		{
			if (m_active_effect->firstSortedElemHandle == UINT16_MAX)
			{
				if (!system->activeElemCount && !system->activeSpotLightElemCount)
				{
					//game::printf_to_console("retrigger, status %d\n", m_active_effect->status);
					cfxwnd::retrigger_effect(m_tickcount_playback);
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

std::vector<fx_pt> fx_grid_pts;
std::vector<fx_grid_line> fx_grid;

void create_grid()
{
	fx_grid.clear();
	fx_grid_pts.clear();

	//game::vec4_t grid_color = { 0.25f, 0.25f, 0.25f, 1.0f };
	const auto grid_sections = /*sections:*/ dvars::fx_browser_grid_sections->current.integer /**/ - 1; // 17
	const auto grid_scale = static_cast<float>(dvars::fx_browser_grid_scale->current.integer); //64.0f;

	for (int j = 0; j <= grid_sections; ++j)
	{
		for (int i = 0; i <= grid_sections; ++i)
		{
			fx_grid_pts.emplace_back(fx_pt
				{
					static_cast<float>(i) * grid_scale - grid_scale * static_cast<float>(grid_sections) * 0.5f,
					static_cast<float>(j) * grid_scale - grid_scale * static_cast<float>(grid_sections) * 0.5f,
					-1.0f
				});
		}
	}

	game::GfxColor col = {};
	game::Byte4PackPixelColor(dvars::fx_browser_grid_color->current.vector, &col);

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
}

/**
 * @brief	draw a grid and position as text
 */
void cfxwnd::draw_grid()
{
	if (!m_grid_generated)
	{
		create_grid();
		m_grid_generated = true;
	}

	for (auto p = 0u; p < fx_grid.size(); p += 200)
	{
		const uint16_t count = fx_grid.size() - p < 200 ? static_cast<uint16_t>(fx_grid.size() - p) : 200;
		components::renderer::R_AddLineCmd(count, static_cast<char>(dvars::fx_browser_grid_line_width->current.integer), 3, fx_grid[p].pts);
	}

	//components::renderer::R_AddLineCmd(static_cast<std::uint16_t>(fx_grid.size()), 3, 3, fx_grid[0].pts);

	game::vec3_t pxs_x = { dvars::fx_browser_grid_font_scale->current.value, 0.0f, 0.0f };
	game::vec3_t pxs_y = { 0.0f, -dvars::fx_browser_grid_font_scale->current.value, 0.0f };
	//game::vec4_t txt_col = { 0.35f, 0.35f, 0.35f, 1.0f };

	if (const auto font = game::R_RegisterFont("fonts/smalldevfont", 1); font)
	{
		for (auto s = 0u; s < fx_grid.size(); s++)
		{
			if (!(s % 16))
			{
				const char* txt = utils::va("%.1f %.1f", fx_grid[s].pts[0].xyz[0], fx_grid[s].pts[0].xyz[1]);
				components::renderer::R_AddCmdDrawTextAtPosition(txt, font /*game::g_qeglobals->d_font_list*/, fx_grid[s].pts[0].xyz, pxs_x, pxs_y, dvars::fx_browser_grid_font_color->current.vector);
			}
		}
	}
}

/**
 * @brief	scene parameters and drawlists (R_RenderScene)
 */
void set_scene_params(const float* origin, float* axis, game::GfxMatrix* projection, int x, int y, int width, int height, bool clear)
{
	game::GfxViewParms* view_parms = game::R_SetupViewParms();
	memset(view_parms, 0, sizeof(game::GfxViewParms));

	game::scene->def.time = timeGetTime();
	game::scene->def.floatTime = static_cast<float>(game::scene->def.time) * 0.001000000047497451f;
	
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
		const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		game::R_Clear(6, white, 1.0f, false);
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

	/*if (!viewInfo->needsFloatZ)
	{

		if (viewInfo->emissiveInfo.drawSurfCount)
		{
			bool needs_floatz = true;
			auto surf = 0;
			while (true)
			{
				const auto material = game::rgp->sortedMaterials[(viewInfo->emissiveInfo.drawSurfs[surf].packed >> 29) & 0xFFF];
				game::MaterialTechnique* technique = (game::MaterialTechnique*)*((DWORD*)&material->techniqueSet->techniques[0]->passArray[0].vertexShader + viewInfo->emissiveInfo.baseTechType);

				if (technique)
				{
					if ((technique->flags & 0x20) != 0)
					{
						break;
					}
				}

				if (++surf == viewInfo->emissiveInfo.drawSurfCount)
				{
					needs_floatz = false;
					break;
				}
			}

			viewInfo->needsFloatZ = needs_floatz;
		}
	}*/
}

/**
 * @brief	setup matrices and general scene
 */
void setup_scene(float* origin, float* axis, int x, int y, int width, int height)
{
	const float fov_y = tan(60.0f * 0.01745329238474369f * 0.5f) * 0.75f;
	const float fov_x = fov_y * (static_cast<float>(cfxwnd::get()->m_width) / static_cast<float>(cfxwnd::get()->m_height));
	
	game::GfxMatrix projection = {};
	
	game::R_SetupProjection(&projection, fov_x, fov_y, 4.0f);
	set_scene_params(origin, axis, &projection, x, y, width, height, false);
}

/**
 * @brief	build camera matrix
 */
void camera_vectors()
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

/**
 * @brief	effect window rendering
 */
void cfxwnd::on_paint()
{
	const auto gui = GET_GUI(ggui::effects_browser);

	if (!gui->is_active() || gui->is_inactive_tab())
	{
		return;
	}

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

	camera_vectors();

	fxwnd->m_width = components::renderer::get_window(components::renderer::CFXWND)->width;
	fxwnd->m_height = components::renderer::get_window(components::renderer::CFXWND)->height;

	if (fxwnd->m_width != 0 && fxwnd->m_height != 0)
	{
		if (gui->is_active() && !gui->is_inactive_tab())
		{
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

			const auto gfx_window = components::renderer::get_window(static_cast<components::renderer::GFXWND_>(game::dx->targetWindowIndex));
			setup_scene(origin, axis, 0, 0, gfx_window->width, gfx_window->height);

			cfxwnd::get()->draw_grid();

			game::R_EndFrame();
			game::R_IssueRenderCommands(-1);
			game::R_SortMaterials(); // !!! clear scene and buffers or camera will preview flickering effect browser effects
		}
	}

	game::R_CheckTargetWindow(hwnd);

	int&		hunk_low_temp = *reinterpret_cast<int*>(0x2422940);
	const int&	hunk_low_permanent = *reinterpret_cast<int*>(0x242293C);
	const int&	s_hunkData = *reinterpret_cast<int*>(0x25D5B88);

	if (!s_hunkData)
	{
		game::Com_Error("cfxwnd::on_paint() :: s_hunkData");
	}

	hunk_low_temp = hunk_low_permanent;

	EndPaint(hwnd, &Paint);
}


/**
 * @brief	window proc for the complete window
 */
LRESULT __stdcall cfxwnd::windowproc_frame(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg <= WM_NOTIFY)
	{
		/*if (Msg == WM_SIZE)
		{
			return DefWindowProcA(hWnd, 5u, wParam, lParam);
		}*/

		if (Msg == WM_CLOSE)
		{
			ShowWindow(cfxwnd::get()->m_frame_hwnd, SW_HIDE);
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
		ShowWindow(cfxwnd::get()->m_frame_hwnd, SW_HIDE);
		return 0;
	}

	return 0;
}

/**
 * @brief	window proc for content area
 */
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

/**
 * @brief	create the "content" area
 */
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

/**
 * @brief	register window classes (frame + content)
 */
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

/**
 * @brief	create frame and content window
 */
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

void cfxwnd::register_dvars()
{
	dvars::fx_browser_grid_sections = dvars::register_int(
		/* name		*/ "fx_browser_grid_sections",
		/* val		*/ 17,
		/* minVal	*/ 8,
		/* maxVal	*/ 64,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "fx browser grid : total grid sections");

	dvars::fx_browser_grid_scale = dvars::register_int(
		/* name		*/ "fx_browser_grid_scale",
		/* val		*/ 64,
		/* minVal	*/ 16,
		/* maxVal	*/ 1024,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "fx browser grid : grid scale (section squared)");

	dvars::fx_browser_grid_color = dvars::register_vec4(
		/* name		*/ "fx_browser_grid_color",
		/* x		*/ 0.1f,
		/* y		*/ 0.1f,
		/* z		*/ 0.1f,
		/* w		*/ 1.0f,
		/* minVal	*/ 0.0f,
		/* maxVal	*/ 1.0f,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "fx browser grid : color");

	dvars::fx_browser_grid_line_width = dvars::register_int(
		/* name		*/ "fx_browser_grid_line_width",
		/* val		*/ 1,
		/* minVal	*/ 1,
		/* maxVal	*/ 8,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "fx browser grid : grid line width");

	dvars::fx_browser_grid_font_scale = dvars::register_float(
		/* name		*/ "fx_browser_grid_font_scale",
		/* default	*/ 0.5f,
		/* mins		*/ 0.1f,
		/* maxs		*/ 4.0f,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "fx browser grid : font scale");

	dvars::fx_browser_grid_font_color = dvars::register_vec4(
		/* name		*/ "fx_browser_grid_font_color",
		/* x		*/ 0.35f,
		/* y		*/ 0.35f,
		/* z		*/ 0.35f,
		/* w		*/ 1.0f,
		/* minVal	*/ 0.0f,
		/* maxVal	*/ 1.0f,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "fx browser grid : font color");

	dvars::fx_browser_use_camera_for_distortion = dvars::register_bool(
		/* name		*/ "fx_browser_use_camera_for_distortion",
		/* default	*/ false,
		/* flags	*/ game::dvar_flags::saved,
		/* desc		*/ "fx browser : use radiants camera image for distortion effects; uses the fx browser image otherwise (hard to see)");
}

// *
// *

cfxwnd::cfxwnd()
{
	p_this = this;
	create_fxwnd();
}