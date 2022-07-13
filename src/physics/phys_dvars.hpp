#pragma once
#include <game/structs.hpp>

namespace physics
{
	extern game::dvar_s* phys_cfm;
	extern game::dvar_s* phys_erp;
	extern game::dvar_s* phys_mcv;
	extern game::dvar_s* phys_mcv_ragdoll;
	extern game::dvar_s* phys_csl;
	extern game::dvar_s* phys_gravity;
	extern game::dvar_s* phys_bulletUpBias;
	extern game::dvar_s* phys_bulletSpinScale;
	extern game::dvar_s* phys_dumpcontacts;
	extern game::dvar_s* phys_qsi;
	extern game::dvar_s* phys_drawcontacts;
	extern game::dvar_s* phys_drawCollisionWorld;
	extern game::dvar_s* phys_drawCollisionObj;
	extern game::dvar_s* phys_drawAwake;
	extern game::dvar_s* phys_drawAwakeTooLong;
	extern game::dvar_s* phys_drawDebugInfo;
	extern game::dvar_s* phys_visibleTris;
	extern game::dvar_s* phys_reorderConst;
	extern game::dvar_s* phys_noIslands;
	extern game::dvar_s* phys_interBodyCollision;
	extern game::dvar_s* phys_collUseEntities;
	extern game::dvar_s* phys_autoDisableLinear;
	extern game::dvar_s* phys_autoDisableAngular;
	extern game::dvar_s* phys_autoDisableTime;
	extern game::dvar_s* phys_contact_cfm;
	extern game::dvar_s* phys_contact_erp;
	extern game::dvar_s* phys_contact_cfm_ragdoll;
	extern game::dvar_s* phys_contact_erp_ragdoll;
	extern game::dvar_s* phys_joint_cfm;
	extern game::dvar_s* phys_joint_stop_cfm;
	extern game::dvar_s* phys_joint_stop_erp;
	extern game::dvar_s* phys_frictionScale;
	extern game::dvar_s* phys_dragLinear;
	extern game::dvar_s* phys_dragAngular;
	extern game::dvar_s* phys_minImpactMomentum;
	extern game::dvar_s* phys_jitterMaxMass;
	extern game::dvar_s* phys_gravityChangeWakeupRadius;
	extern game::dvar_s* phys_narrowObjMaxLength;

	void register_dvars();

}