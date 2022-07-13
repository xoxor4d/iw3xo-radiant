#include "std_include.hpp"

namespace physics
{
	game::dvar_s* phys_cfm = nullptr;
	game::dvar_s* phys_erp = nullptr;
	game::dvar_s* phys_mcv = nullptr;
	game::dvar_s* phys_mcv_ragdoll = nullptr;
	game::dvar_s* phys_csl = nullptr;
	game::dvar_s* phys_gravity = nullptr;
	game::dvar_s* phys_bulletUpBias = nullptr;
	game::dvar_s* phys_bulletSpinScale = nullptr;
	game::dvar_s* phys_dumpcontacts = nullptr;
	game::dvar_s* phys_qsi = nullptr;
	game::dvar_s* phys_drawcontacts = nullptr;
	game::dvar_s* phys_drawCollisionWorld = nullptr;
	game::dvar_s* phys_drawCollisionObj = nullptr;
	game::dvar_s* phys_drawAwake = nullptr;
	game::dvar_s* phys_drawAwakeTooLong = nullptr;
	game::dvar_s* phys_drawDebugInfo = nullptr;
	game::dvar_s* phys_visibleTris = nullptr;
	game::dvar_s* phys_reorderConst = nullptr;
	game::dvar_s* phys_noIslands = nullptr;
	game::dvar_s* phys_interBodyCollision = nullptr;
	game::dvar_s* phys_collUseEntities = nullptr;
	game::dvar_s* phys_autoDisableLinear = nullptr;
	game::dvar_s* phys_autoDisableAngular = nullptr;
	game::dvar_s* phys_autoDisableTime = nullptr;
	game::dvar_s* phys_contact_cfm = nullptr;
	game::dvar_s* phys_contact_erp = nullptr;
	game::dvar_s* phys_contact_cfm_ragdoll = nullptr;
	game::dvar_s* phys_contact_erp_ragdoll = nullptr;
	game::dvar_s* phys_joint_cfm = nullptr;
	game::dvar_s* phys_joint_stop_cfm = nullptr;
	game::dvar_s* phys_joint_stop_erp = nullptr;
	game::dvar_s* phys_frictionScale = nullptr;
	game::dvar_s* phys_dragLinear = nullptr;
	game::dvar_s* phys_dragAngular = nullptr;
	game::dvar_s* phys_minImpactMomentum = nullptr;
	game::dvar_s* phys_jitterMaxMass = nullptr;
	game::dvar_s* phys_gravityChangeWakeupRadius = nullptr;
	game::dvar_s* phys_narrowObjMaxLength = nullptr;

	void register_dvars()
	{
		phys_cfm = game::Dvar_RegisterFloat("phys_cfm", 0.000099f, 0.0f, 1.0f, 0, "Physics constraint force mixing magic parameter.");
		phys_erp = game::Dvar_RegisterFloat("phys_erp", 0.8f, 0.0f, 1.0f, 0, "Physics error reduction magic parameter.");
		phys_mcv = game::Dvar_RegisterFloat("phys_mcv", 20.0f, -FLT_MAX, FLT_MAX, 0, "Physics maximum correcting velocity magic parameter.");
		phys_mcv_ragdoll = game::Dvar_RegisterFloat("phys_mcv_ragdoll", 1000.0f, -FLT_MAX, FLT_MAX, 0, "Physics maximum correcting velocity magic parameter (for ragdoll).");
		phys_csl = game::Dvar_RegisterFloat("phys_csl", 1.0f, -FLT_MAX, FLT_MAX, 0, "Physics contact surface level magic parameter.");
		phys_gravity = game::Dvar_RegisterFloat("phys_gravity", -800.0f, -FLT_MAX, FLT_MAX, 0, "Physics gravity in units/sec^2.");
		phys_bulletUpBias = game::Dvar_RegisterFloat("phys_bulletUpBias", 0.5f, 0.0f, 2.0f, 0, "Up Bias for the direction of the bullet impact.");
		phys_bulletSpinScale = game::Dvar_RegisterFloat("phys_bulletSpinScale", 3.0f, -1.0f, 100.0f, 4096, "Scale of the effective offset from the center of mass for the bullet impacts.");
		phys_dumpcontacts = game::Dvar_RegisterBool("phys_dumpcontacts", 0, 0, "Set to true to dump all constraints in next physics frame.");
		phys_qsi = game::Dvar_RegisterInt("phys_qsi", 15, 1, 0x7FFFFFFF, 0, "Number of iterations that QuickStep performs per step.");
		phys_drawcontacts = game::Dvar_RegisterBool("phys_drawcontacts", 0, 0, "Debug draw contact points");
		phys_drawCollisionWorld = game::Dvar_RegisterBool("phys_drawCollisionWorld", 0, 0, "Debug draw collision brushes and terrain triangles");
		phys_drawCollisionObj = game::Dvar_RegisterBool("phys_drawCollisionObj", 0, 0, "Debug draw collision geometry for each physics object");
		phys_drawAwake = game::Dvar_RegisterBool("phys_drawAwake", 0, 0, "Debug draw a box indicating which bodies are disabled");
		phys_drawAwakeTooLong = game::Dvar_RegisterBool("phys_drawAwakeTooLong", 0, 0, "Draw an indicator showing where the objects are that have been awake too long.");
		phys_drawDebugInfo = game::Dvar_RegisterBool("phys_drawDebugInfo", 0, 0, "Print info about the physics objects");
		phys_visibleTris = game::Dvar_RegisterBool("phys_visibleTris", 0, 0, "Visible triangles are used for collision");
		phys_reorderConst = game::Dvar_RegisterBool("phys_reorderConst", 1, 0, "ODE solver reorder constraints");
		phys_noIslands = game::Dvar_RegisterBool("phys_noIslands", 0, 0, "Make all contacts joints between an object and the world: no object-object contacts");
		phys_interBodyCollision = game::Dvar_RegisterBool("phys_interBodyCollision", 0, 0, "Disable to turn off all inter-body collisions");
		phys_collUseEntities = game::Dvar_RegisterBool("phys_collUseEntities", 0, 0, "Disable to turn off testing for collision against entities");
		phys_autoDisableLinear = game::Dvar_RegisterFloat("phys_autoDisableLinear", 20.0f, 0.0f, FLT_MAX, 0, "A body must have linear velocity less than this to be considered idle.");
		phys_autoDisableAngular = game::Dvar_RegisterFloat("phys_autoDisableAngular", 1.0f, 0.0f, FLT_MAX, 0, "A body must have angular velocity less than this to be considered idle.");
		phys_autoDisableTime = game::Dvar_RegisterFloat("phys_autoDisableTime", 0.899f, 0.0f, FLT_MAX, 0, "The amount of time a body must be idle for it to go to sleep.");
		phys_contact_cfm = game::Dvar_RegisterFloat("phys_contact_cfm", 0.0000099f, 0.0f, 1.0f, 0, "Physics constraint force mixing magic parameter for contacts.");
		phys_contact_erp = game::Dvar_RegisterFloat("phys_contact_erp", 0.8f, 0.0f, 1.0f, 0, "Physics error reduction magic parameter for contacts.");
		phys_contact_cfm_ragdoll = game::Dvar_RegisterFloat("phys_contact_cfm_ragdoll", 0.001f, 0.0f, 1.0f, 0, "Physics constraint force mixing magic parameter for contacts.");
		phys_contact_erp_ragdoll = game::Dvar_RegisterFloat("phys_contact_erp_ragdoll", 0.3f, 0.0f, 1.0f, 0, "Physics error reduction magic parameter for contacts.");
		phys_joint_cfm = game::Dvar_RegisterFloat("phys_joint_cfm", 0.000099f, 0.0f, 1.0f, 0, "Physics constraint force mixing magic parameter for joints.");
		phys_joint_stop_cfm = game::Dvar_RegisterFloat("phys_joint_stop_cfm", 0.000099f, 0.0f, 1.0f, 0, "Physics constraint force mixing magic parameter for joints at their limits.");
		phys_joint_stop_erp = game::Dvar_RegisterFloat("phys_joint_stop_erp", 0.8f, 0.0f, 1.0f, 0, "Physics error reduction magic parameter for joints at their limits.");
		phys_frictionScale = game::Dvar_RegisterFloat("phys_frictionScale", 1.0f, 0.0f, FLT_MAX, 0, "Scales the amount of physics friction globally.");
		phys_dragLinear = game::Dvar_RegisterFloat("phys_dragLinear", 0.0299f, 0.0f, FLT_MAX, 0, "The amount of linear drag, applied globally");
		phys_dragAngular = game::Dvar_RegisterFloat("phys_dragAngular", 0.5f, 0.0f, FLT_MAX, 0, "The amount of angular drag, applied globally");
		phys_minImpactMomentum = game::Dvar_RegisterFloat("phys_minImpactMomentum", 250.0f, 0.0f, FLT_MAX, 0, "The minimum momentum required to trigger impact sounds");
		phys_jitterMaxMass = game::Dvar_RegisterFloat("phys_jitterMaxMass", 200.0f, 0.1f, FLT_MAX, 0, "Maximum mass to jitter - jitter will fall off up to this mass");
		phys_gravityChangeWakeupRadius = game::Dvar_RegisterFloat("phys_gravityChangeWakeupRadius", 120.0f, 0.0f, FLT_MAX, 4096, "The radius around the player within which objects get awakened when gravity changes");
		phys_narrowObjMaxLength = game::Dvar_RegisterFloat("phys_narrowObjMaxLength", 4.0f, 0.0f, FLT_MAX, 0, "If a geom has a dimension less than this, then extra work will be done to prevent it from falling into cracks (like between the wall and the floor)");
		
	}
}