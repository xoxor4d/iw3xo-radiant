#include "std_include.hpp"

constexpr auto JUMP_TIMEOUT = 60;

physx_cct_camera::physx_cct_camera() :
	m_obstacle_context		(nullptr),
	m_filter_data			(nullptr),
	m_filter_callback		(nullptr),
	m_cct_filter_callback	(nullptr),
	m_ccts					(nullptr),
	m_cct_enabled			(true),
	m_msec					(0.0f),
	m_wants_sprint			(false),
	m_wants_jump			(false),
	m_is_jumping			(false),
	m_jump_origin_z			(0.0f),
	m_jump_timer			(0),
	m_jump_held				(false),
	m_is_flying				(false),
	m_ground_type			(GROUND_TYPE_NONE),
	m_on_ground				(false),
	m_bounce	(false),
	m_walking				(false),

	m_gravity				(800.0f),
	m_friction				(5.5f),
	m_jump_velocity			(250.0f),

	m_player_velocity		(PxVec3(0.0f, 0.0f, 0.0f)),
	m_player_speed			(0.0f),
	m_cmd					({ 0.0f, 0.0f, 0.0f })
{
	
}

void physx_cct_camera::set_controlled(physx_cct_controller* controlled)
{
	m_ccts = controlled;
}

void physx_cct_camera::reset_enter_controller_parms()
{
	const auto physx = components::physx_impl::get();

	// all resets here
	physx->m_cctrl_skip_first_mouse_frame = false;
	cmainframe::activewnd->m_pCamWnd->cursor_visible = true;

	int  sw_cur;
	do { sw_cur = ShowCursor(1); } while (sw_cur < 0);
}

void physx_cct_camera::key_inputs()
{
	const auto& io = imgui::GetIO();

	// free mouse
	if (imgui::IsKeyDown(ImGuiKey_Escape))
	{
		components::physx_impl::get()->m_character_controller_enabled = false;
		enable_cct(false);
		physx_cct_camera::reset_enter_controller_parms();
	}

	if (m_cct_enabled)
	{
		if (GetKeyState(0x57) < 0) // W
		{
			m_cmd.forwardMove = 1.0f;
		}
		else if (GetKeyState(0x53) < 0) // S
		{
			m_cmd.forwardMove = -1.0f;
		}
		else
		{
			m_cmd.forwardMove = 0.0f;
		}

		if (GetKeyState(0x41) < 0) // A
		{
			m_cmd.rightMove = -1.0f;
		}
		else if (GetKeyState(0x44) < 0) // D
		{
			m_cmd.rightMove = 1.0f;
		}
		else
		{
			m_cmd.rightMove = 0.0f;
		}


		m_wants_sprint = io.KeyMods == (ImGuiKeyModFlags_Shift);


		if (m_jump_timer < 10000)
		{
			m_jump_timer++;
		}

		if (GetKeyState(VK_SPACE) < 0)
		{
			if (!m_jump_held && !m_is_jumping /*&& m_ground_type == GROUND_TYPE_GROUND*/ )
			{
				m_jump_held = true;
				m_wants_jump = true;
				m_jump_timer = 0;
			}
		}
		else
		{
			// used as released space bool
			m_jump_held = false;
		}

		if (!m_jump_held && m_is_jumping && m_jump_timer >= JUMP_TIMEOUT && GetKeyState(VK_SPACE) < 0)
		{
			m_is_flying = true;
			m_player_velocity.z += 10.0f;
		}
		else
		{
			m_is_flying = false;
		}


		const auto ctrl = get_controller();
		static bool was_down = false;

		if (io.KeyMods == (ImGuiKeyModFlags_Ctrl))
		{
			//game::printf_to_console("crouch\n");
			ctrl->resize_crouching();
			was_down = true;
		}
		else if (was_down)
		{
			//game::printf_to_console("un-crouch\n");
			ctrl->m_do_standup = true;
			was_down = false;
		}
	}
}

void physx_cct_camera::mouse_input()
{
	const auto cam = cmainframe::activewnd->m_pCamWnd;
	const auto guicam = GET_GUI(ggui::camera_dialog);
	const auto px = components::physx_impl::get();

	if (guicam->rtt_is_hovered() && guicam->m_rtt_focused)
	{
		if (get_cct_state())
		{
			bool& skip_first_mouse_frame = px->m_cctrl_skip_first_mouse_frame;
			if (skip_first_mouse_frame)
			{
				CPoint point;
				GetCursorPos(&point);

				//game::printf_to_console("x %d -- y %d", point.x, point.y);

				cam->camera.angles[1] = cam->camera.angles[1] - dvars::physx_camera_sensitivity->current.value / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
				cam->camera.angles[0] = cam->camera.angles[0] - dvars::physx_camera_sensitivity->current.value / 500.0f * static_cast<float>(point.y - cam->m_ptCursor.y);

				if (cam->camera.angles[0] < -85.0f)
				{
					cam->camera.angles[0] = -85.0f;
				}
				else if (cam->camera.angles[0] > 85.0f)
				{
					cam->camera.angles[0] = 85.0f;
				}

				// hide cursor
				cam->cursor_visible = false;

				int  sw_cur;
				do { sw_cur = ShowCursor(0); } while (sw_cur > 0);
			}

			RECT rect;
			GetWindowRect(cam->GetWindow(), &rect);

			ImVec2 cursor_pos;
			cursor_pos.x = guicam->rtt_get_size().x * 0.5f + rect.left;
			cursor_pos.y = guicam->rtt_get_size().y * 0.5f + rect.top;

			SetCursorPos((int)cursor_pos.x, (int)cursor_pos.y);
			cam->m_ptCursor.x = (int)cursor_pos.x;
			cam->m_ptCursor.y = (int)cursor_pos.y;

			//game::printf_to_console("x %d -- y %d", cam->m_ptCursor.x, cam->m_ptCursor.y);

			skip_first_mouse_frame = true;
		}
		else
		{
			if (imgui::IsMouseDown(ImGuiMouseButton_Left))
			{
				enable_cct(true);
			}
		}

		game::AngleVectors(cam->camera.angles, cam->camera.forward, cam->camera.right, cam->camera.up);
	}

	// cancel movement on right mouse button -> smooth transition into radiant cam
	if (px->m_character_controller_enabled && imgui::IsMouseDown(ImGuiMouseButton_Right))
	{
		enable_cct(false);
		guicam->rtt_set_focus_state(false);
		px->m_character_controller_enabled = false;
		reset_enter_controller_parms();
	}
}

#if 1
#define	MAX_CLIP_PLANES	5
bool physx_cct_camera::slide_move(bool do_gravity)
{
	int				bumpcount = 0, numbumps;
	PxVec3			dir;
	float			d;
	int				numplanes;
	game::vec3_t	planes[MAX_CLIP_PLANES];
	PxVec3			primal_velocity;
	PxVec3			clipVelocity;
	int				i, j, k;
	trace_t			trace;
	PxVec3			end;
	float			time_left;
	float			into;
	PxVec3			endVelocity;
	PxVec3			endClipVelocity;

	numbumps = 4;

	//VectorCopy(playerVelocity, primal_velocity);
	primal_velocity = m_player_velocity;

	if (do_gravity)
	{
		//VectorCopy(playerVelocity, endVelocity);
		endVelocity = m_player_velocity;
		endVelocity.z -= m_gravity * m_msec;

		m_player_velocity.z = (m_player_velocity.z + endVelocity.z) * 0.5f;
		primal_velocity.z = endVelocity.z;

		if (m_on_ground) 
		{
			// slide along the ground plane
			clip_velocity(m_groundtrace.normal, m_player_velocity, m_player_velocity);
		}
	}

#if 1
	time_left = m_msec;

	// never turn against the ground plane
	if (m_on_ground) 
	{
		numplanes = 1;
		//VectorCopy(groundtrace.normal, planes[0]);
		planes[0][0] = m_groundtrace.normal.x;
		planes[0][1] = m_groundtrace.normal.y;
		planes[0][2] = m_groundtrace.normal.z;
	}
	else 
	{
		numplanes = 0;
	}

	// never turn against original velocity
	const game::vec3_t v3_vel = { m_player_velocity.x, m_player_velocity.y, m_player_velocity.z };

	// VectorNormalize2(pm->ps->velocity, planes[numplanes]);
	utils::vector::normalize2(v3_vel, planes[numplanes]); 
	 
	numplanes++;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++) 
	{
		const PxVec3 origin (toVec3(m_ccts->get_foot_position()));

		// calculate position we are trying to move to
		//VectorMA(origin, time_left, playerVelocity, end);
		end.x = origin.x + m_player_velocity.x * time_left;
		end.y = origin.y + m_player_velocity.y * time_left;
		end.z = origin.z + m_player_velocity.z * time_left;

		// see if we can make it there
		// pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);

		trace_box(trace,
			origin, end);

		if (trace.allsolid) 
		{
			// entity is completely trapped in another solid
			// pm->ps->velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
			m_player_velocity.z = 0.0f;

			game::printf_to_console("entity is completely trapped in another solid\n");
			return true;
		}

		if (trace.fraction > 0.0f)
		{
			// actually covered some distance
			// VectorCopy(trace.endpos, pm->ps->origin);
			m_ccts->get_controller()->setFootPosition(PxExtendedVec3(trace.endpos.x, trace.endpos.y, trace.endpos.z));
		}

		/*if (trace.fraction == 0.0f)
		{
			int x = 1;
		}*/

		if (trace.fraction == 1.0f) 
		{
			break;		// moved the entire distance
		}

		// save entity for contact
		// PM_AddTouchEnt(trace.entityNum);

		time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES) 
		{
			// this shouldn't really happen
			// VectorClear(pm->ps->velocity);
			m_player_velocity.x = 0.0f;
			m_player_velocity.y = 0.0f;
			m_player_velocity.z = 0.0f;

			game::printf_to_console("this shouldn't really happen\n");
			return true;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for (i = 0; i < numplanes; i++) 
		{
			// if (DotProduct(trace.normal, planes[i]) > 0.99f)
			if (trace.hit.normal.dot(PxVec3(planes[i][0], planes[i][1], planes[i][2])) > 0.99f)
			{
				//VectorAdd(trace.hit.normal, playerVelocity, playerVelocity);
				m_player_velocity += trace.hit.normal;
				break;
			}
		}

		if (i < numplanes) 
		{
			continue;
		}

		//VectorCopy(trace.hit.normal, planes[numplanes]);
		planes[numplanes][0] = trace.hit.normal.x;
		planes[numplanes][1] = trace.hit.normal.y;
		planes[numplanes][2] = trace.hit.normal.z;

		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for (i = 0; i < numplanes; i++) 
		{
			//into = DotProduct(playerVelocity, planes[i]);

			into = m_player_velocity.dot(PxVec3(planes[i][0], planes[i][1], planes[i][2]));
			if (into >= 0.1f) 
			{
				continue;		// move doesn't interact with the plane
			}

			// see how hard we are hitting things
			/*if (-into > pml.impactSpeed) 
			{
				pml.impactSpeed = -into;
			}*/

			// slide along the plane
			// PM_ClipVelocity(pm->ps->velocity, planes[i], clipVelocity, OVERCLIP);
			clip_velocity(PxVec3(planes[i][0], planes[i][1], planes[i][2]), 
				m_player_velocity, clipVelocity);

			// slide along the plane
			// PM_ClipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

			clip_velocity(PxVec3(planes[i][0], planes[i][1], planes[i][2]), 
				endVelocity, endClipVelocity);

			// see if there is a second plane that the new move enters
			for (j = 0; j < numplanes; j++) 
			{
				if (j == i) 
				{
					continue;
				}

				//if (DotProduct(clipVelocity, planes[j]) >= 0.1f)
				if (clipVelocity.dot(PxVec3(planes[j][0], planes[j][1], planes[j][2])) >= 0.1f)
				{
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				// PM_ClipVelocity(clipVelocity, planes[j], clipVelocity, OVERCLIP);
				clip_velocity(PxVec3(planes[j][0], planes[j][1], planes[j][2]),
					clipVelocity, clipVelocity);


				// PM_ClipVelocity(endClipVelocity, planes[j], endClipVelocity, OVERCLIP);
				clip_velocity(PxVec3(planes[j][0], planes[j][1], planes[j][2]),
					endClipVelocity, endClipVelocity);


				// see if it goes back into the first clip plane
				//if (DotProduct(clipVelocity, planes[i]) >= 0.0f)
				if (clipVelocity.dot(PxVec3(planes[i][0], planes[i][1], planes[i][2])) >= 0.0f)
				{
					continue;
				}

				// slide the original velocity along the crease
				// CrossProduct(planes[i], planes[j], dir);
				dir = PxVec3(planes[i][0], planes[i][1], planes[i][2]).cross(
					  PxVec3(planes[j][0], planes[j][1], planes[j][2]));

				// VectorNormalize(dir);
				dir.normalize();

				//DotProduct(dir, playerVelocity);
				d = dir.dot(m_player_velocity); 
				//VectorScale(dir, d, clipVelocity);
				clipVelocity.x = dir.x * d;
				clipVelocity.y = dir.y * d;
				clipVelocity.z = dir.z * d;


				// CrossProduct(planes[i], planes[j], dir);
				dir = PxVec3(planes[i][0], planes[i][1], planes[i][2]).cross(
					  PxVec3(planes[j][0], planes[j][1], planes[j][2]));

				// VectorNormalize(dir);
				dir.normalize();

				//DotProduct(dir, endVelocity);
				d = dir.dot(endVelocity); 
				//VectorScale(dir, d, endClipVelocity);
				endClipVelocity.x = dir.x * d;
				endClipVelocity.y = dir.y * d;
				endClipVelocity.z = dir.z * d;



				// see if there is a third plane the the new move enters
				for (k = 0; k < numplanes; k++) 
				{
					if (k == i || k == j) 
					{
						continue;
					}

					//if (DotProduct(clipVelocity, planes[k]) >= 0.1f)
					if (clipVelocity.dot(PxVec3(planes[k][0], planes[k][1], planes[k][2])) >= 0.1f)
					{
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					// VectorClear(pm->ps->velocity);
					m_player_velocity.x = 0.0f;
					m_player_velocity.y = 0.0f;
					m_player_velocity.z = 0.0f;

					return true;
				}
			}

			// if we have fixed all interactions, try another move
			//VectorCopy(clipVelocity, playerVelocity);
			m_player_velocity = clipVelocity;

			//VectorCopy(endClipVelocity, endVelocity);
			endVelocity = endClipVelocity;

			break;
		}
	}

	if (do_gravity)
	{
		//VectorCopy(endVelocity, playerVelocity);
		m_player_velocity = endVelocity;
	}
#endif

	// don't change velocity if in a timer (FIXME: is this correct?)
	/*if (pm->ps->pm_time) 
	{
		VectorCopy(primal_velocity, pm->ps->velocity);
	}*/

	return (bumpcount != 0);
}
#endif

void physx_cct_camera::stepslide_move(bool do_gravity)
{
	PxVec3 start_o = {};
	PxVec3 start_v = {};
	//PxVec3 down_o = {};
	//PxVec3 down_v = {};
	PxVec3 up = { 0.0f, 0.0f, 1.0f };
	PxVec3 down = {};

	/*game::vec3_t		start_o, start_v;
	game::vec3_t		down_o, down_v;
	game::vec3_t		up, down;*/

	trace_t				trace = {};
	float				stepSize;

	// VectorCopy(pm->ps->origin, start_o);
	const PxVec3 center (toVec3(m_ccts->get_foot_position()));
	start_o = center;

	// VectorCopy(pm->ps->velocity, start_v);
	start_v = m_player_velocity;

	if (slide_move(do_gravity) == 0) 
	{
		game::printf_to_console("we got exactly where we wanted to go first try\n");
		return;		// we got exactly where we wanted to go first try	
	}


	//VectorCopy(start_o, down);
	down = start_o;
	down.y -= 18.0f; // Stepsize

	// pm->trace(&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);

	//trace_box(trace, PxVec3(start_o[0], start_o[1], start_o[2]), 18.0f, TRACE_DIR_DOWN);
	trace_box(trace,
		start_o, down);

	//VectorSet(up, 0.0f, 0.0f, 1.0f);

	// never step up when you still have up velocity
	// if (playerVelocity.z > 0.0f && (trace.fraction == 1.0f || DotProduct(trace.hit.normal, up) < 0.7f)) 

	if (m_player_velocity.z > 0.0f && (trace.fraction == 1.0f || trace.hit.normal.dot(up) < 0.7f))
	{
		game::printf_to_console("never step up when you still have up velocity\n");
		return;
	}

	//game::printf_to_console("stepslide #1 down fract: %.4f\n", trace.fraction);

#if 1
	// VectorCopy(pm->ps->origin, down_o);
	/*down_o[0] = center.x;
	down_o[1] = center.y;
	down_o[2] = center.z;*/ // this would be incorrect but down_o is not used anyway
	//down_o = (toVec3(m_ccts->get_foot_position())); // correct way

	// VectorCopy(pm->ps->velocity, down_v);
	/*down_v[0] = playerVelocity.x;
	down_v[1] = playerVelocity.y;
	down_v[2] = playerVelocity.z;*/ // not used

	//VectorCopy(start_o, up);
	up = start_o;
	up.z += 18.0f; //STEPSIZE;
#endif

	// test the player position if they were a stepheight higher
	// pm->trace(&trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);

	//trace_box(trace, PxVec3(start_o[0], start_o[1], start_o[2]), 18.0f, TRACE_DIR_UP);
	trace_box(trace,
		start_o, up);

	if (trace.allsolid) 
	{
	//	if (pm->debugLevel) {
	//		Com_Printf("%i:bend can't step\n", c_pmove);
	//	}

		game::printf_to_console("can't step up\n");
		return;		// can't step up
	}


	//game::printf_to_console("stepslide #2 up fract: %.4f\n", trace.fraction);

	stepSize = trace.endpos.z - start_o.z; // trace.endpos[2] - start_o[2];

	/*if (stepSize > 0.0f)
	{
		int y = 0;
	}*/

	// try slidemove from this position
	// VectorCopy(trace.endpos, pm->ps->origin);
	m_ccts->get_controller()->setFootPosition(PxExtendedVec3(trace.endpos.x, trace.endpos.y, trace.endpos.z));

	// VectorCopy(start_v, pm->ps->velocity);
	m_player_velocity = start_v;


	// PM_SlideMove(do_gravity);
	slide_move(do_gravity);


	// push down the final amount
	// VectorCopy(pm->ps->origin, down);
	const auto new_org = toVec3(m_ccts->get_foot_position());
	down = new_org;
	down.z -= stepSize;

	// pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
	// trace_box(trace, PxVec3(down[0], down[1], down[2]), 18.0f, TRACE_DIR_DOWN);
	trace_box(trace,
		new_org, down);


	if (!trace.allsolid) 
	{
		// VectorCopy(trace.endpos, pm->ps->origin);
		trace.endpos.z += 0.001f;
		m_ccts->get_controller()->setFootPosition(PxExtendedVec3(trace.endpos.x, trace.endpos.y, trace.endpos.z));
	}

	if (trace.fraction < 1.0f) 
	{
		clip_velocity(trace.hit.normal, m_player_velocity, m_player_velocity);
	}
}

void physx_cct_camera::trace_box(trace_t& trace, const PxVec3& from, const PxVec3& to)
{
	PxRaycastBuffer hit;
	trace.fraction = 1.0f;
	trace.allsolid = false;

	//fraction(percent) of the line that was traced, before
	// an obstacle was hit. Equal to 1 if no obstacle were found.

	const auto px = components::physx_impl::get();

	const float rad = m_ccts->m_controller_radius;
	const PxVec3 start[4] =
	{
		{ from.x - rad, from.y + rad, from.z },
		{ from.x + rad, from.y + rad, from.z },
		{ from.x + rad, from.y - rad, from.z },
		{ from.x - rad, from.y - rad, from.z }
	};

	const PxVec3 end[4] =
	{
		{ to.x - rad, to.y + rad, to.z },
		{ to.x + rad, to.y + rad, to.z },
		{ to.x + rad, to.y - rad, to.z },
		{ to.x - rad, to.y - rad, to.z }
	};

	const PxVec3 directions[4] =
	{
		{ (end[0] - start[0]).getNormalized() },
		{ (end[1] - start[1]).getNormalized() },
		{ (end[2] - start[2]).getNormalized() },
		{ (end[3] - start[3]).getNormalized() }
	};

	const PxReal distances[4] =
	{
		{ VectorLength(end[0] - start[0]) },
		{ VectorLength(end[1] - start[1]) },
		{ VectorLength(end[2] - start[2]) },
		{ VectorLength(end[3] - start[3]) }
	};

	for (auto edge = 0; edge < 4; edge++)
	{
		// test for all solid (overlap) .. how?
		/*if (px->mScene->raycast(
			start[edge],
			directions[edge],
			0.0f,
			hit,
			PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
		{
			trace.allsolid = true;
			trace.fraction = 0.0f;

			game::printf_to_console("trace started in solid\n");
			return;
		}*/

		if (px->mScene->raycast(
			start[edge],
			directions[edge],
			distances[edge],
			hit,
			PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
		{
			if (hit.block.distance > distances[edge])
			{
				__debugbreak();
			}

			trace.fraction = hit.block.distance / distances[edge];

			trace.hit.distance = hit.block.distance;
			trace.hit.position = hit.block.position;
			trace.hit.normal = hit.block.normal;

			trace.endpos = hit.block.position;
			return;
		}
	}

	trace.endpos = to;
}

void physx_cct_camera::trace_box(trace_t& trace, const PxVec3& from, const PxReal& dist, TRACE_DIR_ dir)
{
	PxRaycastBuffer hit;
	trace.fraction = 1.0f;

	//fraction(percent) of the line that was traced, before
	// an obstacle was hit. Equal to 1 if no obstacle were found.

	const auto px = components::physx_impl::get();

	const float rad = m_ccts->m_controller_radius;
	const PxVec3 start[4] =
	{
		{ from.x - rad, from.y + rad, from.z },
		{ from.x + rad, from.y + rad, from.z },
		{ from.x + rad, from.y - rad, from.z },
		{ from.x - rad, from.y - rad, from.z }
	};

	for (auto edge = 0; edge < 4; edge++)
	{
		// test for all solid (overlap)
		if (px->mScene->raycast(
			start[edge],
			PxVec3(0.0f, 0.0f, dir == TRACE_DIR_DOWN ? -1.0f : 1.0f),
			0.0f,
			hit,
			PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
		{
			trace.allsolid = true;
			trace.fraction = 0.0f;

			game::printf_to_console("trace started in solid\n");
			return;
		}

		if (px->mScene->raycast(
			start[edge],
			PxVec3(0.0f, 0.0f, dir == TRACE_DIR_DOWN ? -1.0f : 1.0f),
			dist,
			hit,
			PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
		{
			if (hit.block.distance > dist)
			{
				__debugbreak();
			}

			trace.fraction		= hit.block.distance / dist;

			trace.hit.distance	= hit.block.distance;
			trace.hit.position	= hit.block.position;
			trace.hit.normal	= hit.block.normal;

			trace.endpos		= hit.block.position;

			return;
		}
	}

	trace.endpos = from;

	if (dir == TRACE_DIR_DOWN)
	{
		trace.endpos.z -= dist;
	}
	else if (dir == TRACE_DIR_UP)
	{
		trace.endpos.z += dist;
	}
}

void physx_cct_camera::ground_trace()
{
	const auto px = components::physx_impl::get();
	const PxVec3 center (toVec3(m_ccts->get_foot_position()));

	PxRaycastBuffer stepsize_hit;
	PxRaycastBuffer hit;

	const float rad = m_ccts->m_controller_radius;

	const PxVec3 from[4] =
	{
		{ center.x - rad, center.y + rad, center.z }, // top left
		{ center.x + rad, center.y + rad, center.z }, // top right
		{ center.x + rad, center.y - rad, center.z }, // bottom right
		{ center.x - rad, center.y - rad, center.z }  // bottom left
	};

	const PxVec3 stepoffset_dirs[8] =
	{
		{ -1,  0,  0 }, {  0,  1,  0 }, // t left : left/up
		{  0,  1,  0 }, {  1,  0,  0 }, // t right: up/right
		{  1,  0,  0 }, {  0, -1,  0 }, // b right: right/back
		{  0, -1,  0 }, { -1,  0,  0 }  // b left : back/left
	};

	bool hit_slope = false;
	bool hit_something = false;
	bool enable_step = false;

	for (auto edge = 0; edge < 4; edge++)
	{
		// manual stepoffset logic because physx has its issues with the built-in stepoffset (for box colliders)
		// see @ https://forum.unity.com/threads/character-controller-getting-stuck-when-going-through-a-door-that-should-easily-be-fit-into-able.335783/
		// trace in all directions, slightly above the ground -> set physx stepoffset for 1 move if it hits something
		// (top down view, tracing from top left corner clockwise (see from/stepoffset_dirs))

		for (auto step = 0; step < 2; step++)
		{
			if (enable_step)
			{
				break;
			}

			if (px->mScene->raycast(
				PxVec3(from[edge].x, from[edge].y, from[edge].z + 1.0f),
				stepoffset_dirs[2 * edge + step],
				10.0f,
				stepsize_hit,
				PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
			{
				m_ccts->get_controller()->setStepOffset(18.0f);
				//game::printf_to_console("enable stepoffset from dir %d\n", 2 * edge + step);

				enable_step = true;
				break;
			}
		}


		// test for slopes (similar to stepoffset but less dist)
		for (auto step = 0; step < 2; step++)
		{
			if (hit_slope)
			{
				break;
			}

			if (px->mScene->raycast(
				PxVec3(from[edge].x, from[edge].y, from[edge].z + 1.0f),
				stepoffset_dirs[2 * edge + step],
				1.0f,
				hit,
				PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
			{
				hit_something = true;
				m_groundtrace.normal = hit.block.normal;
				m_groundtrace.distance = hit.block.distance;
				m_groundtrace.position = hit.block.position;

				if (hit.block.normal.z < 0.7f && hit.block.normal.z > 0.0f)
				{
					m_ground_type = GROUND_TYPE_SLOPE;
					m_walking = false;
					m_on_ground = false; // true needed for clip_velocity in air_move
					m_is_jumping = false;
					
					hit_slope = true;
					break;
				}
			}
		}

		if (!hit_slope)
		{
			// test ground
			if (px->mScene->raycast(
				from[edge],
				PxVec3(0.0f, 0.0f, -1.0f),
				1.0f,
				hit,
				PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
			{
				hit_something = true;
				m_groundtrace.normal = hit.block.normal;
				m_groundtrace.distance = hit.block.distance;
				m_groundtrace.position = hit.block.position;
			}
		}
	}

	if (hit_something && !hit_slope)
	{
		m_ground_type = GROUND_TYPE_GROUND;
		m_on_ground = true;
		m_walking = true;
		m_bounce = true;
	}
	else if (!hit_something)
	{
		// PM_GroundTraceMissed
		// re-trace with - 64 Z and set almostGroundPlane if hit

		//m_almost_ground_plane = false;
		m_on_ground = false;
		m_walking = false;

		// ..
		m_ground_type = GROUND_TYPE_NONE;
		m_groundtrace.normal = PxVec3(0.0f, 0.0f, 0.0f);
	}

	/*if (hit_something)
	{
		m_almost_ground_plane = true;
		m_on_ground = true;
		m_walking = true;
	}*/
}

bool physx_cct_camera::is_sprinting()
{
	if (m_wants_sprint && m_on_ground)
	{
		return true;
	}

	return false;
}

bool physx_cct_camera::jump_check()
{
	const auto physx = components::physx_impl::get();

	if (m_wants_jump && m_on_ground && !physx->m_cct_controller->m_is_crouching)
	{
		return true;
	}

	return false;
}

void physx_cct_camera::clip_velocity(const PxVec3& normal, const PxVec3& in, PxVec3& out)
{
	float	backoff;
	float	change;
	int		i;

	backoff = in.dot(normal);

	if (backoff < 0.0f) 
	{
		backoff *= 1.001f;
	}
	else 
	{
		backoff /= 1.001f;
	}

	for (i = 0; i < 3; i++) 
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}
}

void physx_cct_camera::project_velocity(const PxVec3& normal, float* v_in, float* v_out)
{
	const float speed_xy = v_in[1] * v_in[1] + v_in[0] * v_in[0];

	if (normal.z < 0.001f || speed_xy == 0.0f)
	{
		utils::vector::copy(v_in, v_out, 3);
	}
	else
	{
		const float normalized = -(normal.y * v_in[1] + v_in[0] * normal.x) / normal.z;
		const float projection = sqrtf(((v_in[2] * v_in[2] + speed_xy) / (speed_xy + normalized * normalized)));

		if (projection < 1.0f || normalized < 0.0f || v_in[2] > 0.0f)
		{
			v_out[0] = projection * v_in[0];
			v_out[1] = projection * v_in[1];
			v_out[2] = projection * normalized;
		}
	}
}

void physx_cct_camera::accelerate(const PxVec3& wishdir, float wishspeed, float accel)
{
	const float addspeed = wishspeed - m_player_velocity.dot(wishdir);

	//game::printf_to_console("accel - whishdir %.2f %.2f %.2f\n", wishdir.x, wishdir.y, wishdir.z);
	//game::printf_to_console("accel - wishspeed %.2f\n", wishspeed);

	if (addspeed > 0.0f)
	{
		// stopspeed
		if (wishspeed < 100.0f)
		{
			wishspeed = 100.0f;
		}

		float accelspeed = accel * m_msec * wishspeed;
		if (accelspeed > addspeed)
		{
			accelspeed = addspeed;
		}

		m_player_velocity.x += accelspeed * wishdir.x;
		m_player_velocity.y += accelspeed * wishdir.y;
		m_player_velocity.z += accelspeed * wishdir.z;
	}
}

void physx_cct_camera::pm_friction()
{
	float up_vel = m_player_velocity.z;
	if (m_walking)
	{
		up_vel = 0.0f; // ignore slope movement
	}

	const float vel = sqrtf(m_player_velocity.x * m_player_velocity.x + m_player_velocity.y * m_player_velocity.y + up_vel * up_vel);
	const float speed = vel;

	if (vel < 1.0f)
	{
		m_player_velocity.x = 0.0f;
		m_player_velocity.y = 0.0f;
		m_player_velocity.z = 0.0f;
		return;
	}

	float drop = 0.0f;

	//if (m_walking)
	{
		float calc_friction;

		// stopspeed->current.value
		if (100.0f <= speed)
		{
			calc_friction = vel;
		}
		else
		{
			calc_friction = 100.0f;
		}

		// (slowdown on land here ...)

		drop = m_friction * calc_friction * m_msec;
	}

	float newspeed = speed - drop;

	if (newspeed < 0)
	{
		newspeed = 0;
	}

	if (speed > 0)
	{
		newspeed /= speed;
	}

	m_player_velocity.x *= newspeed;
	m_player_velocity.y *= newspeed;
	m_player_velocity.z *= newspeed;
}

void physx_cct_camera::air_move()
{
	//pm_friction();

	const float forwardmove = (m_cmd.forwardMove * 127.0f);
	const float rightmove = (m_cmd.rightMove * 127.0f);

	// PM_CmdScale
	float cmd_scale = 0.0f;
	{
		const auto abs_forwardmove = fabs(forwardmove);
		const auto abs_rightmove = fabs(rightmove);

		float max_move = abs_forwardmove;

		if (abs_rightmove > abs_forwardmove)
		{
			max_move = abs_rightmove;
		}

		if (max_move)
		{
			float scale = (190.0f * max_move) / (127.0f * sqrtf(forwardmove * forwardmove + rightmove * rightmove));

			/*if ((ps->pm_flags & PMF_WALKING) != 0 || 0.0 != ps->leanf)
			{
				scale = scale * 0.4000000059604645;
			}*/

			cmd_scale = scale;
		}
	}

	const auto cam = cmainframe::activewnd->m_pCamWnd->camera;

	PxVec3 v_fwd (cam.forward[0], cam.forward[1], 0.0f);
	PxVec3 v_rt  (cam.right[0], cam.right[1], 0.0f);

	v_fwd.normalize();
	v_rt.normalize();

	game::vec3_t dir = {};
	dir[0] = v_fwd.x * forwardmove + v_rt.x * rightmove;
	dir[1] = v_fwd.y * forwardmove + v_rt.y * rightmove;
	dir[2] = 0.0f;

	game::vec3_t wishdir = {};
	const float wishspeed = utils::vector::normalize2(dir, wishdir) * cmd_scale;

	accelerate(PxVec3(wishdir[0], wishdir[1], wishdir[2]), wishspeed, 1.0f);

	//if (m_on_ground) // hack
	if (!m_is_jumping && m_groundtrace.normal.z < 0.7f && m_groundtrace.normal.z > 0.0f)
	{
		if (!m_bounce)
		{
			m_bounce = true;
			float new_vel[3] = { m_player_velocity.x, m_player_velocity.y, m_player_velocity.z };
			project_velocity(m_groundtrace.normal, new_vel, new_vel);

			m_player_velocity.x = new_vel[0];
			m_player_velocity.y = new_vel[1];
			m_player_velocity.z = new_vel[2];
		}
		else
		{
			clip_velocity(m_groundtrace.normal, m_player_velocity, m_player_velocity);
		}
	}

	// PM_StepSlideMove(pm, pml, 1);
	//stepslide_move(true); // broken

	// --- cct
	// gravity

	m_player_velocity.z -= m_gravity * m_msec;
}

void physx_cct_camera::walk_move()
{
	if (is_sprinting())
	{
		m_cmd.rightMove *= 0.667f; // player_sprintStrafeSpeedScale
	}

	// playerVelocity.z = -gravity * dtime;

	if (jump_check())
	{
		m_player_velocity.z = m_jump_velocity;
		m_wants_jump = false;

		m_on_ground = false;
		m_bounce = false;
		m_walking = false;
		m_is_jumping = true; // PMF_JUMPING

		air_move();
	}
	else
	{
		//playerVelocity.z -= gravity * msec;

		pm_friction();

		// pm_cmdscale
		// scale forwardmove to match quake (range of char +- 127/8)
		const float forwardmove = m_cmd.forwardMove * 127.0f;
		const float rightmove = m_cmd.rightMove * 127.0f;

		float cmdscale = 0.0f;
		{
			float axis = 0.0f;
			if (forwardmove >= 0.0f)
			{
				axis = fabs(forwardmove);
			}
			else
			{
				// player_backSpeedScale = 0.7f
				axis = fabs(forwardmove * 0.7f);
			}

			// player_strafeSpeedScale = 0.8f
			if (axis - fabs(rightmove * 0.8f) < 0.0f)
			{
				axis = fabs(rightmove * 0.8f);
			}

			if (axis != 0.0f)
			{
				// ps->speed = g_speed
				float scale = (190.0f * axis) / (127.0f * sqrtf(forwardmove * forwardmove + rightmove * rightmove));

				if (is_sprinting())
				{
					// player_sprintSpeedScale = 1.5f
					scale = 1.5f * scale;
				}

				// PM_CmdScaleForStance (scale * stance)
				// standing = 1.0f
				// crouch = 0.645f

				if (m_ccts->m_is_crouching)
				{
					scale *= 0.645f;
				}

				cmdscale = scale;
			}
		}

		const auto cam = cmainframe::activewnd->m_pCamWnd->camera;

		PxVec3 v_fwd (cam.forward[0], cam.forward[1], 0.0f);
		PxVec3 v_rt  (cam.right[0], cam.right[1], 0.0f);

		v_fwd.normalize();
		v_rt.normalize();

		game::vec3_t dir = {};
		dir[0] = v_fwd.x * forwardmove + v_rt.x * rightmove;
		dir[1] = v_fwd.y * forwardmove + v_rt.y * rightmove;
		dir[2] = 0.0f;

		game::vec3_t wishdir = {};
		const float wishspeed = utils::vector::normalize2(dir, wishdir) * cmdscale;

		project_velocity(m_groundtrace.normal, wishdir, wishdir);

		float accel = 9.0f;
		if (m_ccts->m_is_crouching)
		{
			accel = 19.0f;
		}

		accelerate(PxVec3(wishdir[0], wishdir[1], wishdir[2]), wishspeed, accel);

		float new_vel[3] = { m_player_velocity.x, m_player_velocity.y, m_player_velocity.z };
		project_velocity(m_groundtrace.normal, new_vel, new_vel);

		m_player_velocity.x = new_vel[0];
		m_player_velocity.y = new_vel[1];
		m_player_velocity.z = new_vel[2];

		if (m_player_velocity.x != 0.0f || m_player_velocity.y != 0.0f)
		{
			//PM_StepSlideMove(pm, pml, 0);
			//stepslide_move(false); // broken
		}
	}
}





void snap_vector(PxVec3& v)
{
	/*bool do_print = false;
	if (v.z != 0.0f)
	{
		do_print = true;
		game::printf_to_console("snapvec before: %.4f\n", v.z);
	}*/

	// actual rounding
	//v.x = static_cast<float>( static_cast<int>(v.x + 0.5f) );
	//v.y = static_cast<float>( static_cast<int>(v.y + 0.5f) );
	//v.z = static_cast<float>( static_cast<int>(v.z + 0.5f) );

	/*v.x = static_cast<float>(static_cast<int>(v.x));
	v.y = static_cast<float>(static_cast<int>(v.y));
	v.z = static_cast<float>(static_cast<int>(v.z));*/

	/*if (do_print)
	{
		game::printf_to_console("snapvec after: %.4f\n-------\n", v.z);
	}*/

	v.x = rint(v.x);
	v.y = rint(v.y);
	v.z = rint(v.z);
}

void physx_cct_camera::update(PxReal dtime)
{
	const auto camera = cmainframe::activewnd->m_pCamWnd;
	const auto physx = components::physx_impl::get();

	m_msec = dtime;

	if (!m_ccts)
	{
		return;
	}

	physx_cct_camera::mouse_input();
	physx_cct_camera::key_inputs();

	if (m_on_ground)
	{
		walk_move();
	}
	else
	{
		air_move();
	}


	// ground info and stepsize
	m_ccts->get_controller()->setStepOffset(0.0f);
	physx_cct_camera::ground_trace();


	// move controller
	const PxControllerFilters filters(m_filter_data, m_filter_callback, m_cct_filter_callback);
	const PxU32 flags = m_ccts->m_controller->move(m_player_velocity * dtime, 0.0f, dtime, filters, m_obstacle_context);

	if (flags & PxControllerCollisionFlag::eCOLLISION_DOWN && m_ground_type != GROUND_TYPE_SLOPE)
	{
		m_on_ground = true;
		m_is_jumping = false;
		m_bounce = false;
		m_jump_origin_z = 0.0f;
	}
	else if ((m_player_velocity.z != 0.0f || m_is_jumping) && m_ground_type != GROUND_TYPE_SLOPE)
	{
		m_on_ground = false;
	}

	if (flags & PxControllerCollisionFlag::eCOLLISION_UP && m_is_jumping)
	{
		m_player_velocity.z = 0.0f;
	}

	// calc speed
	m_player_speed = sqrtf(m_player_velocity.x * m_player_velocity.x + m_player_velocity.y * m_player_velocity.y);

	// update camera
	if (const auto cct = m_ccts->m_controller; cct)
	{
		const PxVec3 target = toVec3(cct->getFootPosition());

		camera->camera.origin[0] = target.x;
		camera->camera.origin[1] = target.y;
		camera->camera.origin[2] = target.z + (physx->m_cct_controller->m_is_crouching ? 40.0f : 60.0f);

		physx->m_cct_controller->sync();
	}

	snap_vector(m_player_velocity);
}