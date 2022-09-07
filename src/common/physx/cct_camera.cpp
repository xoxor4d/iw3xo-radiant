//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "std_include.hpp"


constexpr auto JUMP_TIMEOUT = 40;

physx_cct_camera::physx_cct_camera() :
	m_obstacle_context		(nullptr),
	m_filter_data			(nullptr),
	m_filter_callback		(nullptr),
	m_cct_filter_callback	(nullptr),
	m_ccts					(nullptr),
	mFwd					(false),
	mBwd					(false),
	mLeft					(false),
	mRight					(false),
	mKeyShiftDown			(false),
	m_cct_enabled			(true),
	m_ground_type			(GROUND_TYPE_NONE),
	m_jumping				(false),
	m_fly					(false),
	m_jumping_timer			(0),
	direction_up			(0.0f),
	m_abs_force				(PxVec3(0.0f, 0.0f, 0.0f)),
	m_surface_normal		(PxVec3(0.0f, 0.0f, 0.0f)),
	m_running_speed			(285.0f),
	m_walking_speed			(190.0f),

	gravity					(800.0f),
	friction				(5.5f),
	moveSpeed				(190.0f),
	runAcceleration			(9.0f),
	runDeacceleration		(9.0f),
	airAcceleration			(1.0f),
	airDecceleration		(1.0f),
	airControl				(0.3f),
	sideStrafeAcceleration	(9.0f),
	sideStrafeSpeed			(0.8f),
	jumpSpeed				(250.0f),
	moveDirectionNorm		(PxVec3(0.0f, 0.0f, 0.0f)),
	playerVelocity			(PxVec3(0.0f, 0.0f, 0.0f)),
	playerMaxVelocity		(0.0f),
	playerFriction			(0.0f),
	playerSpeed				(0.0f),
	wishJump				(false),
	isGrounded				(false),
	_cmd					({ 0.0f, 0.0f, 0.0f })
	//pm						(),
	//pml						()
{
	
}

void physx_cct_camera::set_controlled(physx_cct_controller* controlled)
{
	m_ccts = controlled;
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
			_cmd.forwardMove = 1.0f;
		}
		else if (GetKeyState(0x53) < 0) // S
		{
			_cmd.forwardMove = -1.0f;
		}
		else
		{
			_cmd.forwardMove = 0.0f;
		}

		if (GetKeyState(0x41) < 0) // A
		{
			_cmd.rightMove = -1.0f;
		}
		else if (GetKeyState(0x44) < 0) // D
		{
			_cmd.rightMove = 1.0f;
		}
		else
		{
			_cmd.rightMove = 0.0f;
		}


		mKeyShiftDown = io.KeyMods == (ImGuiKeyModFlags_Shift);



		if (GetKeyState(VK_SPACE) < 0 && !wishJump)
		{
			wishJump = true;
		}
		else
		{
			wishJump = false;
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

#if 0
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
		mFwd = GetKeyState(0x57) < 0;	// W
		mBwd = GetKeyState(0x53) < 0;	// S
		mLeft = GetKeyState(0x41) < 0;	// A
		mRight = GetKeyState(0x44) < 0;	// D

		mKeyShiftDown = io.KeyMods == (ImGuiKeyModFlags_Shift);

		const auto ctrl = get_controller();

		if (m_jumping_timer < 10000)
		{
			m_jumping_timer++;
		}

		if (GetKeyState(VK_SPACE) < 0)
		{
			if (m_ground_type == GROUND_TYPE_GROUND && !m_jumping && m_jumping_timer >= JUMP_TIMEOUT)
			{
				// hack: box collider height + stepOffset can prevent the player from moving under an object
				// the player can pass if he has some upward velocity tho (eg. is jumping ...)
				// so we make the collider smaller then it should be but increase its size when jumping

				//		 .__. 
				// .__.	 |  |  - object above player, should be able to pass but can't due to stepOffset
				// |  |  |__|
				// |__|	- stepOffset
				// |  |
				// |__| - box collider height

				ctrl->m_controller->resize(
					(ctrl->m_is_crouching ? ctrl->m_crouching_size : ctrl->m_standing_size) + 15.0f); // radius

				direction_up = 2.49f;
				m_ground_type = GROUND_TYPE_NONE;
				m_jumping = true;
				m_jumping_timer = 0;
				m_fly = false;
			}
		}
		else
		{
			// used as released space bool
			m_fly = true;
		}

		if (m_fly && m_jumping && GetKeyState(VK_SPACE) < 0)
		{
			direction_up = 2.49f;
		}

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
#endif

void physx_cct_camera::mouse_input()
{
	const auto cam = cmainframe::activewnd->m_pCamWnd;
	const auto guicam = GET_GUI(ggui::camera_dialog);

	if (guicam->rtt_is_hovered() && guicam->m_rtt_focused)
	{
		if (get_cct_state())
		{
			bool& skip_first_mouse_frame = components::physx_impl::get()->m_cctrl_skip_first_mouse_frame;
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
	}
}

#if 0
void physx_cct_camera::ground_trace()
{
	//PxRaycastHit hitInfo;

	const PxVec3 center (toVec3(m_ccts->get_foot_position()));
	//PxVec3 to	(from.x, from.y, from.z - (m_ccts->m_standing_size * 2.0f));

	const auto px = components::physx_impl::get();

	PxRaycastBuffer hit;

	const float rad = m_ccts->m_controller_radius;

	const PxVec3 from[4] =
	{
		{ center.x - rad, center.y + rad, center.z },
		{ center.x + rad, center.y + rad, center.z },
		{ center.x + rad, center.y - rad, center.z },
		{ center.x - rad, center.y - rad, center.z }
	};

	bool hit_slope = false;
	bool hit_something = false;

	for (auto edge = 0; edge < 4; edge++)
	{
		if (px->mScene->raycast(
			from[edge],
			PxVec3(0.0f, 0.0f, -1.0f),
			4.0f, //m_ccts->m_standing_size,
			hit,
			PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
		{
			hit_something = true;

			if (hit.block.normal.z < 0.7f && hit.block.normal.z > 0.0f)
			{
				m_surface_normal = hit.block.normal;
				m_ground_type = GROUND_TYPE_SLOPE;

				px->m_dbgline_ground_trace[0].xyz[0] = from[edge].x;
				px->m_dbgline_ground_trace[0].xyz[1] = from[edge].y;
				px->m_dbgline_ground_trace[0].xyz[2] = from[edge].z;

				px->m_dbgline_ground_trace[1].xyz[0] = hit.block.position.x;
				px->m_dbgline_ground_trace[1].xyz[1] = hit.block.position.y;
				px->m_dbgline_ground_trace[1].xyz[2] = hit.block.position.z;

				px->m_dbgline_ground_trace[0].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_GREEN);
				px->m_dbgline_ground_trace[1].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_RED);

				hit_slope = true;

				break;
			}
		}
	}

	if (hit_something && !hit_slope)
	{
		m_ground_type = GROUND_TYPE_GROUND;
	}
	else if (!hit_something)
	{
		m_ground_type = GROUND_TYPE_NONE;
	}

	if (m_ground_type != GROUND_TYPE_SLOPE)
	{
		px->m_dbgline_ground_trace[0].xyz[0] = center.x;
		px->m_dbgline_ground_trace[0].xyz[1] = center.y;
		px->m_dbgline_ground_trace[0].xyz[2] = center.z;
		px->m_dbgline_ground_trace[0].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_GREY);

		px->m_dbgline_ground_trace[1].xyz[0] = center.x;
		px->m_dbgline_ground_trace[1].xyz[1] = center.y;
		px->m_dbgline_ground_trace[1].xyz[2] = center.z - m_ccts->m_standing_size;
		px->m_dbgline_ground_trace[1].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_GREY);
	}
}
#endif

void physx_cct_camera::reset_enter_controller_parms()
{
	const auto physx = components::physx_impl::get();

	// all resets here
	physx->m_cctrl_skip_first_mouse_frame = false;
	cmainframe::activewnd->m_pCamWnd->cursor_visible = true;

	int  sw_cur;
	do { sw_cur = ShowCursor(1); } while (sw_cur < 0);
}

#if 0
void physx_cct_camera::do_slopes(PxReal dtime)
{
	const auto cct_gravity = 190.0f;
	const auto slope_limit = 0.7f;
	const auto cct_slope_slide_slowdown_factor = 0.99f;

	if (m_ground_type == GROUND_TYPE_SLOPE)
	{
		game::vec3_t normal = { m_surface_normal.x, m_surface_normal.y, m_surface_normal.z };
		//game::printf_to_console("nrml %.1f, %.1f, %.1f\n", normal[0], normal[1], normal[2]);

		game::vec3_t ang = {};
		game::vectoangles(normal, ang);
		//game::printf_to_console("angl %.1f, %.1f, %.1f\n", ang[0], ang[1], ang[2]);

		game::vec3_t fwd = {};
		game::vec3_t rt = {};
		game::vec3_t up = {};
		utils::vector::angle_vectors(ang, fwd, rt, up); 

		// gravity draws him down the slope
		m_abs_force.x = fwd[0] * cct_gravity * slope_limit;
		m_abs_force.y = fwd[1] * cct_gravity * slope_limit;
		m_abs_force.z = 0.0f;

		//game::printf_to_console("do slope %.1f, %.1f\n", m_abs_force.x, m_abs_force.y);
	}
	else
	{
		// not of much use with current (infinite) friction
		// reset absolute forces slowly (don't stop right after the slope ends)
		m_abs_force.x -= (m_abs_force.x * cct_slope_slide_slowdown_factor * dtime);
		m_abs_force.y -= (m_abs_force.y * cct_slope_slide_slowdown_factor * dtime);
		m_abs_force.z = 0;

		/*if (m_abs_force.x > 0.0f || m_abs_force.y > 0.0f)
		{
			game::printf_to_console("do slope slowdown %.3f, %.3f\n", m_abs_force.x, m_abs_force.y);
		}*/
	}
}
#endif

void physx_cct_camera::AirMove(PxReal dtime)
{
	float accel;

	const auto cam = cmainframe::activewnd->m_pCamWnd->camera;

	game::vec3_t dir = {};
	game::vec3_t wishdir = {};

	dir[0] = cam.forward[0] * _cmd.forwardMove + cam.right[0] * _cmd.rightMove;
	dir[1] = cam.forward[1] * _cmd.forwardMove + cam.right[1] * _cmd.rightMove;
	dir[2] = 0.0f;

	float wishspeed = utils::vector::normalize2(dir, wishdir) * moveSpeed;
	moveDirectionNorm = PxVec3(wishdir[0], wishdir[1], wishdir[2]);


	// CPM: Aircontrol
	float wishspeed2 = wishspeed;
	if (playerVelocity.dot(moveDirectionNorm) < 0)
		accel = airDecceleration;
	else
		accel = airAcceleration;

	// If the player is ONLY strafing left or right
	if (_cmd.forwardMove == 0.0f && _cmd.rightMove != 0.0f)
	{
		if (wishspeed > sideStrafeSpeed)
			wishspeed = sideStrafeSpeed;

		accel = sideStrafeAcceleration;
	}

	Accelerate(moveDirectionNorm, wishspeed, accel, dtime);

	if (airControl > 0)
		AirControl(moveDirectionNorm, wishspeed2, dtime);

	// !CPM: Aircontrol

	// Apply gravity
	playerVelocity.z -= gravity * dtime;
}

void physx_cct_camera::AirControl(PxVec3 wishdir, float wishspeed, PxReal dtime)
{
	float zspeed;
	float speed;
	float dot;
	float k;

	// Can't control movement if not moving forward or backward
	if (abs(_cmd.forwardMove) < 0.001f || abs(wishspeed) < 0.001f)
		return;

	zspeed = playerVelocity.z;
	playerVelocity.z = 0.0f;

	/* Next two lines are equivalent to idTech's VectorNormalize() */
	speed = playerVelocity.magnitude();
	playerVelocity.normalize();

	dot = playerVelocity.dot(wishdir);
	k = 32;
	k *= airControl * dot * dot * dtime;

	// Change direction while slowing down
	if (dot > 0)
	{
		playerVelocity.x = playerVelocity.x * speed + wishdir.x * k;
		playerVelocity.y = playerVelocity.y * speed + wishdir.y * k;
		playerVelocity.z = playerVelocity.z * speed + wishdir.z * k;

		playerVelocity.normalize();
		moveDirectionNorm = playerVelocity;
	}

	playerVelocity.x *= speed;
	playerVelocity.y *= speed;
	playerVelocity.z = zspeed; // Note this line
}

bool physx_cct_camera::is_sprinting()
{
	if (mKeyShiftDown && isGrounded)
	{
		return true;
	}

	return false;
}

bool physx_cct_camera::jump_check()
{
	const auto physx = components::physx_impl::get();

	if (wishJump && isGrounded && !physx->m_cct_controller->m_is_crouching)
	{
		return true;
	}

	return false;
}

void physx_cct_camera::walk_move(PxReal dtime)
{
	if (is_sprinting())
	{
		//_cmd.rightMove *= 0.667f; // player_sprintStrafeSpeedScale
	}

	if (jump_check())
	{

	}

	// Do not apply friction if the player is queueing up the next jump
	if (!wishJump)
		ApplyFriction(1.0f, dtime);
	else
		ApplyFriction(0.0f, dtime);


	

	const auto cam = cmainframe::activewnd->m_pCamWnd->camera;

	game::vec3_t dir = {};
	game::vec3_t wishdir = {};

	dir[0] = cam.forward[0] * _cmd.forwardMove + cam.right[0] * _cmd.rightMove;
	dir[1] = cam.forward[1] * _cmd.forwardMove + cam.right[1] * _cmd.rightMove;
	dir[2] = 0.0f;

	const float wishspeed = utils::vector::normalize2(dir, wishdir) * moveSpeed;
	moveDirectionNorm = PxVec3(wishdir[0], wishdir[1], wishdir[2]);

	//float wishspeed = wishdir.magnitude();
	//wishspeed *= moveSpeed;

	Accelerate(moveDirectionNorm, wishspeed, runAcceleration, dtime);

	// Reset the gravity velocity
	playerVelocity.z = -gravity * dtime;

	if (wishJump)
	{
		playerVelocity.z = jumpSpeed;
		wishJump = false;
	}
}

void physx_cct_camera::ApplyFriction(float t, PxReal dtime)
{
	PxVec3 vec = playerVelocity; // Equivalent to: VectorCopy();
	float speed;
	float newspeed;
	float control;
	float drop;

	vec.z = 0.0f;
	speed = vec.magnitude();
	drop = 0.0f;

	/* Only if the player is on the ground then apply friction */
	if (isGrounded)
	{
		control = speed < runDeacceleration ? runDeacceleration : speed;
		drop = control * friction * dtime * t;
	}

	newspeed = speed - drop;
	playerFriction = newspeed;

	if (newspeed < 0)
		newspeed = 0;

	if (speed > 0)
		newspeed /= speed;

	playerVelocity.x *= newspeed;
	playerVelocity.y *= newspeed;
}

void physx_cct_camera::Accelerate(PxVec3 wishdir, float wishspeed, float accel, PxReal dtime)
{
	float addspeed;
	float accelspeed;
	float currentspeed;

	currentspeed = playerVelocity.dot(wishdir);
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0)
		return;

	accelspeed = accel * dtime * wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	playerVelocity.x += accelspeed * wishdir.x;
	playerVelocity.y += accelspeed * wishdir.y;
}

void physx_cct_camera::update(PxReal dtime)
{
	const auto camera = cmainframe::activewnd->m_pCamWnd;
	const auto physx = components::physx_impl::get();

	if (!m_ccts)
	{
		return;
	}

	physx_cct_camera::mouse_input();
	physx_cct_camera::key_inputs();

	//QueueJump();

	if (isGrounded)
		walk_move(dtime);

	else if (!isGrounded)
		AirMove(dtime);


	// Move the controller
	const PxControllerFilters filters(m_filter_data, m_filter_callback, m_cct_filter_callback);
	const PxU32 flags = m_ccts->m_controller->move(playerVelocity * dtime, 0.0f, 0.008f, filters, m_obstacle_context);

	if (flags & PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		isGrounded = true;
	}
	else
	{
		isGrounded = false;
	}

	if (flags & PxControllerCollisionFlag::eCOLLISION_UP)
	{
		playerVelocity.z = 0.0f;
	}

	/* Calculate max velocity */
	PxVec3 udp = playerVelocity;
	udp.z = 0.0f;

	if (udp.magnitude() > playerMaxVelocity)
		playerMaxVelocity = udp.magnitude();

	// calc speed
	playerSpeed = sqrtf(playerVelocity.x * playerVelocity.x + playerVelocity.y * playerVelocity.y);


	// update camera
	if (const auto cct = m_ccts->m_controller; cct)
	{
		PxExtendedVec3 cam_target;
		cam_target = cct->getFootPosition();

		//const PxVec3 delta = cct->getPosition() - cct->getFootPosition();
		//const PxVec3 physicsPos = cct->getActor()->getGlobalPose().p - delta;
		//cam_target = PxExtendedVec3 (physicsPos.x, physicsPos.y, physicsPos.z);

		const PxVec3 target = toVec3(cam_target);

		camera->camera.origin[0] = target.x;
		camera->camera.origin[1] = target.y;
		camera->camera.origin[2] = target.z + (physx->m_cct_controller->m_is_crouching ? 40.0f : 60.0f);

		physx->m_cct_controller->sync();
	}
}

#if 0
void physx_cct_camera::update(PxReal dtime)
{
	const auto camera = cmainframe::activewnd->m_pCamWnd;
	const auto physx = components::physx_impl::get();

	if (!m_ccts)
	{
		return;
	}

	physx_cct_camera::mouse_input();
	physx_cct_camera::key_inputs();

	// update CCT
	if (m_cct_enabled)
	{
		physx_cct_camera::ground_trace();
		physx_cct_camera::do_slopes(dtime);

		const PxControllerFilters filters (m_filter_data, m_filter_callback, m_cct_filter_callback);

		PxVec3 disp;
		PxVec3 targetKeyDisplacement(0);

		const PxVec3 forward (camera->camera.forward[0], camera->camera.forward[1], camera->camera.forward[2]);
		const PxVec3 right   (camera->camera.right[0],   camera->camera.right[1],   camera->camera.right[2]);

		{
			if (mFwd)	targetKeyDisplacement += forward;
			if (mBwd)	targetKeyDisplacement -= forward;

			if (mRight)	targetKeyDisplacement += right;
			if (mLeft)	targetKeyDisplacement -= right;

			targetKeyDisplacement *= mKeyShiftDown ? m_running_speed : m_walking_speed;
			targetKeyDisplacement *= dtime;
		}

		disp = targetKeyDisplacement;
		disp.z = direction_up;
		

		const PxU32 flags = m_ccts->m_controller->move(disp, 0.0f, dtime, filters, m_obstacle_context);

		if (flags & PxControllerCollisionFlag::eCOLLISION_DOWN && m_ground_type != GROUND_TYPE_SLOPE)
		{
			if (m_jumping && m_jumping_timer >= JUMP_TIMEOUT)
			{
				const auto ctrl = get_controller();

				// hint : physx_cct_camera::key_inputs()
				ctrl->m_controller->resize(
					(ctrl->m_is_crouching ? ctrl->m_crouching_size : ctrl->m_standing_size));

				m_jumping = false;
				m_fly = false;
			}

			direction_up = 0;
			m_ground_type = GROUND_TYPE_GROUND;
		}
		else
		{
			if (m_ground_type == GROUND_TYPE_SLOPE)
			{
				direction_up += -9.8f * dtime;
			}
			else
			{
				direction_up += -9.8f * dtime;
				m_ground_type = GROUND_TYPE_NONE;
			}
		}


		m_abs_force *= dtime;
		m_ccts->m_controller->move(m_abs_force, 0.0f, dtime, filters, m_obstacle_context);



		// update camera
		if (const auto cct = m_ccts->m_controller; cct)
		{
			PxExtendedVec3 cam_target;
			cam_target = cct->getFootPosition();

			//const PxVec3 delta = cct->getPosition() - cct->getFootPosition();
			//const PxVec3 physicsPos = cct->getActor()->getGlobalPose().p - delta;
			//cam_target = PxExtendedVec3 (physicsPos.x, physicsPos.y, physicsPos.z);

			const PxVec3 target = toVec3(cam_target);

			camera->camera.origin[0] = target.x;
			camera->camera.origin[1] = target.y;
			camera->camera.origin[2] = target.z + (physx->m_cct_controller->m_is_crouching ? 40.0f : 60.0f);

			physx->m_cct_controller->sync();
		}
	}
}
#endif

