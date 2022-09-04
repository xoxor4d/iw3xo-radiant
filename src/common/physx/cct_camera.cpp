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
	m_running_speed			(285.0f),
	m_walking_speed			(190.0f),
	m_gravity				(-800.0f)
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

void physx_cct_camera::ground_trace()
{
	//PxRaycastHit hitInfo;

	PxVec3 from (toVec3(m_ccts->get_foot_position()));
	//PxVec3 to	(from.x, from.y, from.z - (m_ccts->m_standing_size * 2.0f));

	const auto px = components::physx_impl::get();

	PxRaycastBuffer hit;
	if (px->mScene->raycast(
		from, 
		PxVec3(0.0f, 0.0f, -1.0f), 
		m_ccts->m_standing_size * 2.0f, 
		hit,
		PxHitFlag::ePOSITION | PxHitFlag::eNORMAL))
	{
		px->m_dbgline_ground_trace[0].xyz[0] = from.x;
		px->m_dbgline_ground_trace[0].xyz[1] = from.y;
		px->m_dbgline_ground_trace[0].xyz[2] = from.z;
		px->m_dbgline_ground_trace[0].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_BLUE);

		px->m_dbgline_ground_trace[1].xyz[0] = hit.block.position.x;
		px->m_dbgline_ground_trace[1].xyz[1] = hit.block.position.y;
		px->m_dbgline_ground_trace[1].xyz[2] = hit.block.position.z;
		px->m_dbgline_ground_trace[1].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_BLUE);

		if (hit.block.normal.z < 0.7f && hit.block.normal.z > 0.0f)
		{
			m_ground_type = GROUND_TYPE_SLOPE;
			px->m_dbgline_ground_trace[1].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_RED);
		}
		else
		{
			m_ground_type = GROUND_TYPE_GROUND;
		}
	}
	else
	{
		px->m_dbgline_ground_trace[0].xyz[0] = from.x;
		px->m_dbgline_ground_trace[0].xyz[1] = from.y;
		px->m_dbgline_ground_trace[0].xyz[2] = from.z;
		px->m_dbgline_ground_trace[0].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_GREY);

		px->m_dbgline_ground_trace[1].xyz[0] = from.x;
		px->m_dbgline_ground_trace[1].xyz[1] = from.y;
		px->m_dbgline_ground_trace[1].xyz[2] = from.z + m_ccts->m_standing_size * 2.0f;
		px->m_dbgline_ground_trace[1].color.packed = static_cast<unsigned>(PxDebugColor::eARGB_GREY);
	}
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
		ground_trace();

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
