#pragma once

namespace physics
{
	void Phys_CreateBasisFromNormal(float* o2, float* normal, float* o1);
	float ClampMin(float val, float min);

	void Vec3Lerp(const float* from, const float* to, float frac, float* out);
	void Vec4Lerp(const float* from, const float* to, float frac, float* out);

	void QuatLerp(const float* qa, const float* qb, float frac, float* out);
	void QuatToAxis(const float* quat, float* axis);
}