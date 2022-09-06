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

#ifndef SAMPLE_CCT_ACTOR_H
#define SAMPLE_CCT_ACTOR_H

#include "characterkinematic/PxExtended.h"
#include "characterkinematic/PxController.h"

namespace physx
{
	class PxController;
	class PxUserControllerHitReport;
	class PxControllerBehaviorCallback;
	class PxControllerManager;
	class PxPhysics;
	class PxScene;
}

using namespace physx;

	struct physx_cct_controller_desc
	{
										physx_cct_controller_desc();

		PxControllerShapeType::Enum		m_type;
		PxExtendedVec3					m_position;
		float							m_slope_limit;
		float							m_contact_offset;
		float							m_step_offset;
		float							m_invisible_wall_height;
		float							m_max_jump_height;
		float							m_radius;
		float							m_height;
		float							m_crouch_height;
		float							m_proxy_density;
		float							m_proxy_scale;
		float							m_volume_growth;
		PxUserControllerHitReport*		m_report_callback;
		PxControllerBehaviorCallback*	m_behavior_callback;
	};

	class physx_cct_controller
	{
		public:
													physx_cct_controller();
		virtual										~physx_cct_controller();

						PxController*				init(const physx_cct_controller_desc& desc, PxControllerManager* manager);
						PxExtendedVec3				get_foot_position()	const;
						void						reset();
						void						teleport(const PxVec3& pos);
						void						sync();
						void						try_standup();
						void						resize_controller(PxReal height);
						void						resize_standing()			{ resize_controller(m_standing_size);	}
						void						resize_crouching()			{ resize_controller(m_crouching_size);	}
		PX_FORCE_INLINE	PxController*				get_controller()			{ return m_controller; }


		protected:
						PxControllerShapeType::Enum	m_type;

						PxExtendedVec3				m_initial_position;
						PxVec3						m_delta;
						bool						m_transfer_momentum;

						PxController*				m_controller;
						PxReal						m_standing_size;
						PxReal						m_crouching_size;
						PxReal						m_controller_radius;
						bool						m_do_standup;
						bool						m_is_crouching;

						friend class physx_cct_camera;

	private:
		physx_cct_controller& operator=(const physx_cct_controller&);
	};

#endif
