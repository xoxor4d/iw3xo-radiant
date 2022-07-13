#pragma once

namespace physics
{
	int Phys_IndexFromODEWorld(dxWorld* world);

	void Phys_OdeMatrix3ToAxis(const float* inMatrix, float(*outAxis)[3]);
	void Phys_AxisToOdeMatrix3(const float(*inAxis)[3], float* outMatrix);

	void AxisTransformVec3(const float* v1, const float* v2, float* out);

}