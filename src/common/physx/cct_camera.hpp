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

#pragma once

//#include "characterkinematic/PxController.h"
//#include "characterkinematic/PxControllerObstacles.h"

class	ccamwnd;
class	physx_cct_controller;

class physx_cct_camera
{
	public:
		physx_cct_camera();

		void			set_controlled(physx_cct_controller* controlled);
		static void		reset_enter_controller_parms();

		virtual	void	update(PxReal dtime);
		virtual	PxReal	get_camera_speed()
		{ 
			return mKeyShiftDown ? m_running_speed : m_walking_speed; 
		}

		enum GROUND_TYPE_
		{
			GROUND_TYPE_NONE,
			GROUND_TYPE_GROUND,
			GROUND_TYPE_SLOPE,
		};

		PX_FORCE_INLINE physx_cct_controller*		get_controller()						{ return m_ccts; }
		PX_FORCE_INLINE void						set_gravity(PxReal g)					{ m_gravity = g; }
		PX_FORCE_INLINE void						enable_cct(bool bState)					{ m_cct_enabled = bState; }
		PX_FORCE_INLINE bool						get_cct_state()							{ return m_cct_enabled; }

		PX_FORCE_INLINE void						setObstacleContext(PxObstacleContext* context)		{ m_obstacle_context = context;	}
		PX_FORCE_INLINE void						setFilterData(const PxFilterData* filterData)		{ m_filter_data = filterData; }
		PX_FORCE_INLINE void						setFilterCallback(PxQueryFilterCallback* cb)		{ m_filter_callback = cb; }
		PX_FORCE_INLINE void						setCCTFilterCallback(PxControllerFilterCallback* cb){ m_cct_filter_callback = cb; }

	private:
						void	key_inputs();
						void	mouse_input();
						void	ground_trace();
						void	do_slopes(PxReal dtime);

						physx_cct_camera& operator=(const physx_cct_camera&);
						PxObstacleContext*			m_obstacle_context;		// User-defined additional obstacles
						const PxFilterData*			m_filter_data;			// User-defined filter data for 'move' function
						PxQueryFilterCallback*		m_filter_callback;		// User-defined filter data for 'move' function
						PxControllerFilterCallback*	m_cct_filter_callback;	// User-defined filter data for 'move' function

						physx_cct_controller*		m_ccts;

						bool						mFwd, mBwd, mLeft, mRight, mKeyShiftDown;
						bool						m_cct_enabled;

						GROUND_TYPE_				m_ground_type;
						bool						m_jumping;
						bool						m_fly;
						int							m_jumping_timer;
						PxReal						direction_up;
						PxVec3						m_abs_force;
						PxVec3						m_surface_normal;

						PxReal						m_running_speed;
						PxReal						m_walking_speed;
						PxReal						m_gravity;
};
