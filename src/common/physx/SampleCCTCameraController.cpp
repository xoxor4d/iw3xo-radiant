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

// PT: TODO: move this inside the CCT?
static const bool gTransferPlatformMomentum = true;

static bool gDumpCCTState = false;
static bool gDumpCCTStats = false;

static bool canJump(ControlledActor* actor)
{
	PxControllerState cctState;
	actor->getController()->getState(cctState);
	return (cctState.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) != 0;
}

#if defined(WIN32)
static bool copyToClipboard(const char* text)
{
	if(!text)
		return false;

	if(OpenClipboard(NULL))
	{
		HGLOBAL hMem = GlobalAlloc(GHND|GMEM_DDESHARE, strlen(text)+1);
		if(hMem)
		{
			char* pMem = (char*)GlobalLock(hMem);
			strcpy(pMem, text);
			GlobalUnlock(hMem);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hMem);
		}
		CloseClipboard();
	}
	return true;
}
#endif

SampleCCTCameraController::SampleCCTCameraController() :
	mObstacleContext	(NULL),
	mFilterData			(NULL),
	mFilterCallback		(NULL),
	mCCTFilterCallback	(NULL),
	mControlledIndex	(0),
	mNbCCTs				(0),
	mCCTs				(NULL),
	mTargetYaw			(0.0f-PxPi/2),
	mTargetPitch		(0.0f),
	mPitchMin			(-PxHalfPi),
	mPitchMax			(PxHalfPi),
	mGamepadPitchInc	(0.0f),
	mGamepadYawInc		(0.0f),
	mGamepadForwardInc	(0.0f),
	mGamepadLateralInc	(0.0f),
	mSensibility		(0.001f),
	mFwd				(false),
	mBwd				(false),
	mLeft				(false),
	mRight				(false),
	mKeyShiftDown		(false),
	mCCTEnabled			(true),
	mRunningSpeed		(285.0f),
	mWalkingSpeed		(190.0f),
//	mWalkingSpeed		(0.5f),
	mGamepadWalkingSpeed(190.0f),
	mCameraMaxSpeed		(FLT_MAX),
	mJumpForce			(500.0f), // sqrtf ( gravity * (jump_height + jump_height) ) :: 250
	mGravity			(-800.0f),
	mLinkCameraToPhysics(false),
	direction_up		(0.0f),
	m_ground_plane		(false)
{
	
}

void SampleCCTCameraController::setControlled(ControlledActor** controlled, PxU32 controlledIndex, PxU32 nbCCTs)
{
	mControlledIndex	= controlledIndex;
	mNbCCTs				= nbCCTs;
	mCCTs				= controlled;
}

void SampleCCTCameraController::handle_inputs()
{
	const auto& io = imgui::GetIO();

	if (imgui::IsKeyDown(ImGuiKey_Escape))
	{
		components::physx_impl::get()->m_character_controller_enabled = false;
	}

	//mFwd = imgui::IsKeyPressed(ImGuiKey_W);
	mFwd = GetKeyState(0x57) < 0;

	//mBwd = imgui::IsKeyPressed(ImGuiKey_S);
	mBwd = GetKeyState(0x53) < 0;

	//mLeft = imgui::IsKeyPressed(ImGuiKey_A);
	mLeft = GetKeyState(0x41) < 0;

	//mRight = imgui::IsKeyPressed(ImGuiKey_D);
	mRight = GetKeyState(0x44) < 0;

	mKeyShiftDown = io.KeyMods == (ImGuiKeyModFlags_Shift);//imgui::IsKeyPressed(ImGuiKey_LeftShift);

	if (GetKeyState(VK_SPACE) < 0) //(imgui::IsKeyPressed(ImGuiKey_Space))
	{
		//if (canJump(mCCTs[0]))
		if (m_ground_plane)
		{
			direction_up = 2.49f;
			m_ground_plane = false;
		}
	}

	// const bool is_alt_key_pressed = GetKeyState(VK_MENU) < 0;

	static bool was_down = false;
	if (io.KeyMods == (ImGuiKeyModFlags_Ctrl))
	{
		was_down = true;
		getControlledActor()->resizeCrouching();
		game::printf_to_console("crouch\n");
	}
	else if (was_down)
	{
		getControlledActor()->mDoStandup = true;
		was_down = false;
		game::printf_to_console("un-crouch\n");
	}
}

void SampleCCTCameraController::input_frame()
{
	const auto cam = cmainframe::activewnd->m_pCamWnd;
	const auto guicam = GET_GUI(ggui::camera_dialog);
	const auto prefs = game::g_PrefsDlg();

	if (guicam->rtt_is_hovered() && guicam->m_rtt_focused)
	{
		bool& skip_first_mouse_frame = components::physx_impl::get()->m_cctrl_skip_first_mouse_frame;
		if (skip_first_mouse_frame)
		{
			CPoint point;
			GetCursorPos(&point);

			//game::printf_to_console("x %d -- y %d", point.x, point.y);

			// use anglespeed
			cam->camera.angles[1] = cam->camera.angles[1] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
			cam->camera.angles[0] = cam->camera.angles[0] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.y - cam->m_ptCursor.y);

			// org
			//cam->camera.angles[1] = cam->camera.angles[1] - prefs->m_nMoveSpeed / 500.0 * (point.x - cam->m_ptCursor.x);
			//cam->camera.angles[0] = cam->camera.angles[0] - prefs->m_nMoveSpeed / 500.0 * (point.y - cam->m_ptCursor.y);


			//SetCursorPos(cam->m_ptCursor.x, cam->m_ptCursor.y);

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
		SampleCCTCameraController::handle_inputs();
		
	}
}

void SampleCCTCameraController::setView(PxReal pitch, PxReal yaw)
{
	mTargetPitch = pitch;
	mTargetYaw   = yaw;
}

void SampleCCTCameraController::update(PxReal dtime)
{
//	shdfnd::printFormatted("SampleCCTCameraController::update\n");

	const auto camera = cmainframe::activewnd->m_pCamWnd;
	const auto physx = components::physx_impl::get();

	//dtime = 0.01f;

	if(!mCCTs)
		return;

	// Update CCT
	if(mCCTEnabled)
	{
		SampleCCTCameraController::input_frame();

		const PxControllerFilters filters(mFilterData, mFilterCallback, mCCTFilterCallback);

		for(PxU32 i=0;i<mNbCCTs;i++)
		{
			PxVec3 disp;

			/*const PxF32 heightDelta = mCCTs[i]->mJump.getHeight(dtime);
			float dy;

			if(heightDelta != 0.0f)
				dy = heightDelta;
			else
				dy = mGravity * dtime;*/

//			shdfnd::printFormatted("%f\n", dy);

			if(i==mControlledIndex)
			{
				PxVec3 targetKeyDisplacement(0);
				PxVec3 targetPadDisplacement(0);

				//game::vec3_t dir = {};
				//camera->calculate_ray_direction(static_cast<int>(camera->camera.width * 0.5f), static_cast<int>(camera->camera.height * 0.5f), dir);

				//PxVec3 forward = camera.getViewDir();
				//forward.y = 0;
				//forward.normalize();

				//PxVec3 forward (dir[0], dir[1], dir[2]);
				//PxVec3 up (0.0f, 0.0f, 1.0f);
				//PxVec3 right = forward.cross(up);

				PxVec3 forward (camera->camera.forward[0], camera->camera.forward[1], camera->camera.forward[2]);//(camera->camera.vpn[0], camera->camera.vpn[1], camera->camera.vpn[2]);
				PxVec3 right(camera->camera.right[0], camera->camera.right[1], camera->camera.right[2]);//(camera->camera.vright[0], camera->camera.vright[1], camera->camera.vright[2]);

//				if(canJump(mCCTs[i]))	// PT: prevent displacement in mid-air
				{
					if (mFwd)	targetKeyDisplacement += forward;
					if (mBwd)	targetKeyDisplacement -= forward;

					if (mRight)	targetKeyDisplacement += right;
					if (mLeft)	targetKeyDisplacement -= right;

					targetKeyDisplacement *= mKeyShiftDown ? mRunningSpeed : mWalkingSpeed;
					targetKeyDisplacement *= dtime;

					targetPadDisplacement += forward * mGamepadForwardInc * mGamepadWalkingSpeed;
					targetPadDisplacement += right * mGamepadLateralInc * mGamepadWalkingSpeed;
					targetPadDisplacement *= dtime;
				}

				disp = targetKeyDisplacement + targetPadDisplacement;
				disp.z = direction_up;
			}
			else
			{
				disp = PxVec3(0, 0, direction_up);
			}

//			const PxU32 flags = mCCTs[i]->mController->move(disp, 0.001f, dtime, filters, mObstacleContext);
			const PxU32 flags = mCCTs[i]->mController->move(disp, 0.0f, dtime, filters, mObstacleContext);



			if (flags & PxControllerCollisionFlag::eCOLLISION_DOWN)
			{
//				shdfnd::printFormatted("Stop jump\n");
				//game::printf_to_console("Stop jump\n");
				//mCCTs[i]->mJump.stopJump();

				direction_up = 0;
				m_ground_plane = true;
			}
			else
			{
				direction_up += -9.8f * dtime;
				m_ground_plane = false;
			}

			//game::printf_to_console("dtime %.4f", dtime);

		}
	}

	// Update camera
	PxController* cct = mCCTs[mControlledIndex]->mController;
	if (cct)
	{
		PxExtendedVec3 camTarget;
		if (!mLinkCameraToPhysics)
		{
			camTarget = cct->getFootPosition();
		}
		else
		{
			const PxVec3 delta = cct->getPosition() - cct->getFootPosition();
			const PxVec3 physicsPos = cct->getActor()->getGlobalPose().p - delta;
			camTarget = PxExtendedVec3(physicsPos.x, physicsPos.y, physicsPos.z);
		}

		const PxVec3 target = toVec3(camTarget);

		camera->camera.origin[0] = target.x;
		camera->camera.origin[1] = target.y;
		camera->camera.origin[2] = target.z + (physx->m_actor->mIsCrouching ? 40.0f : 60.0f);

		physx->m_actor->sync();
	}
}

