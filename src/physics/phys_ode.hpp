#pragma once
#include "physics_structs.hpp"

namespace physics
{
	extern bool physInited;
	extern int g_phys_msecStep[3];
	extern int g_phys_minMsecStep[3];
	extern int g_phys_maxMsecStep[3];

	extern odeGlob_t odeGlob;
	extern PhysGlob physGlob;

	void Phys_ObjAddForce(PhysWorld worldIndex, dxBody* id, const float* worldPos, const float* impulse);
	void Phys_ObjGetInterpolatedState(int worldIndex, int id, float* outPos, float* out_quat);
	dxBody* Phys_ObjCreate(int worldIndex, const float* position, const float* quat, const float* velocity, game::PhysPreset* physPreset);
	dxBody* Phys_ObjCreateAxis(PhysWorld worldIndex, const float* position, float* axis, const float* velocity, game::PhysPreset* physPreset);
	void Phys_ObjSetCollisionFromXModel(const game::XModel* model, int worldIndex, dxBody* body);
	void Phys_ObjSetAngularVelocity(int id, const float* velocity);

	void Phys_RunToTime(int worldIndex, int timeNow);
	void Pool_Init(void* pool, pooldata_t* pooldata, unsigned int itemSize, unsigned int itemCount);
	void Phys_ObjDestroy(int worldIndex, int id);
	void Phys_Init();
}
