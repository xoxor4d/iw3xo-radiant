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

#include "characterkinematic/PxBoxController.h"
#include "characterkinematic/PxCapsuleController.h"
#include "characterkinematic/PxControllerManager.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

physx_cct_controller_desc::physx_cct_controller_desc() :
	m_type					(PxControllerShapeType::eFORCE_DWORD),
	m_position				(PxExtendedVec3(0,0,0)),
	m_slope_limit			(0.0f),
	m_contact_offset		(0.0f),
	m_step_offset			(0.0f),
	m_invisible_wall_height	(0.0f),
	m_max_jump_height		(0.0f),
	m_radius				(0.0f),
	m_height				(0.0f),
	m_crouch_height			(0.0f),
	m_proxy_density			(10.0f),
	m_proxy_scale			(0.9f),
	m_volume_growth			(1.5f),
	m_report_callback		(nullptr),
	m_behavior_callback		(nullptr)
{
	
}

///////////////////////////////////////////////////////////////////////////////

physx_cct_controller::physx_cct_controller() :
	m_type					(PxControllerShapeType::eFORCE_DWORD),
	m_controller			(nullptr),
	m_standing_size			(0.0f),
	m_crouching_size		(0.0f),
	m_controller_radius		(0.0f),
	m_do_standup			(false),
	m_is_crouching			(false)
{
	m_initial_position = PxExtendedVec3(0,0,0);
	m_delta = PxVec3(0);
	m_transfer_momentum = false;
}

physx_cct_controller::~physx_cct_controller()
{

}

void physx_cct_controller::reset()
{
	m_controller->setPosition(m_initial_position);
}

void physx_cct_controller::teleport(const PxVec3& pos)
{
	m_controller->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
	m_transfer_momentum = false;
	m_delta = PxVec3(0);
}

PxExtendedVec3 physx_cct_controller::get_foot_position() const
{
	return m_controller->getFootPosition();
}

void physx_cct_controller::sync()
{
	if (m_do_standup)
	{
		try_standup();
	}
}

PxController* physx_cct_controller::init(const physx_cct_controller_desc& desc, PxControllerManager* manager)
{
	const float radius	= desc.m_radius;
	float height		= desc.m_height;
	float crouch_height	= desc.m_crouch_height;

	PxControllerDesc* c_desc;
	PxBoxControllerDesc box_desc;
	PxCapsuleControllerDesc capsule_desc;

	if (desc.m_type == PxControllerShapeType::eBOX)
	{
		height *= 0.5f;
		//height += radius;
		//height -= radius;

		crouch_height *= 0.5f;
		//crouch_height += radius;
		//crouch_height -= radius;

		box_desc.halfHeight			= height;
		box_desc.halfSideExtent		= radius;
		box_desc.halfForwardExtent	= radius;
		c_desc = &box_desc;
	}
	else 
	{
		PX_ASSERT(desc.mType==PxControllerShapeType::eCAPSULE);
		capsule_desc.height = height;
		capsule_desc.radius = radius;
		capsule_desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
		c_desc = &capsule_desc;
	}

	c_desc->density				= desc.m_proxy_density;
	c_desc->scaleCoeff			= 1.0f; //desc.m_proxy_scale;
	c_desc->material			= components::physx_impl::get()->m_cct_material;
	c_desc->position			= desc.m_position;
	c_desc->slopeLimit			= desc.m_slope_limit;
	c_desc->contactOffset		= desc.m_contact_offset;
	c_desc->stepOffset			= desc.m_step_offset;
	c_desc->invisibleWallHeight	= desc.m_invisible_wall_height;
	c_desc->maxJumpHeight		= desc.m_max_jump_height;
	c_desc->nonWalkableMode		= PxControllerNonWalkableMode::ePREVENT_CLIMBING;
	c_desc->reportCallback		= desc.m_report_callback;
	c_desc->behaviorCallback	= desc.m_behavior_callback;
	c_desc->volumeGrowth		= 1.0f; //desc.m_volume_growth;
	c_desc->upDirection			= PxVec3(0, 0, 1.0f);
	

	m_type						= desc.m_type;
	m_initial_position			= desc.m_position;
	m_standing_size				= height;
	m_crouching_size			= crouch_height;
	m_controller_radius			= radius;

	PxController* ctrl = static_cast<PxBoxController*>(manager->createController(*c_desc));
	PX_ASSERT(ctrl);

	// remove controller shape from scene query for standup overlap test
	const auto actor = ctrl->getActor();

	//actor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, true);

	if (actor)
	{
		if (actor->getNbShapes())
		{
			PxShape* ctrl_shape;
			actor->getShapes(&ctrl_shape,1);
			ctrl_shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		}
	}

	m_controller = ctrl;
	return ctrl;
}

void physx_cct_controller::try_standup()
{
	// overlap with upper part
	if (m_type == PxControllerShapeType::eBOX)
	{
	}
	else if (m_type == PxControllerShapeType::eCAPSULE)
	{
		PxScene* scene = m_controller->getScene();
		PxSceneReadLock scopedLock(*scene);
		
		const auto capsule_ctrl = static_cast<PxCapsuleController*>(m_controller);

		const auto r = capsule_ctrl->getRadius();
		const auto dh = m_standing_size - m_crouching_size - 2 * r;
		const PxCapsuleGeometry geom(r, dh * 0.5f);

		const auto position = physx::toVec3(m_controller->getPosition());
		const PxVec3 pos(position.x, position.y + m_standing_size * 0.5f + r, position.z);
		const PxQuat orientation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));

		PxOverlapBuffer hit;
		if (scene->overlap(geom, PxTransform(pos,orientation), hit, PxQueryFilterData(PxQueryFlag::eANY_HIT | PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC)))
			return;
	}

	// if no hit, we can stand up
	resize_standing();

	m_do_standup = false;
	m_is_crouching = false;
}

void physx_cct_controller::resize_controller(PxReal height)
{
	m_is_crouching = true;
	m_controller->resize(height);
}
