#include "std_include.hpp"

namespace physics
{
	void Phys_CreateBasisFromNormal(float* o2, float* normal, float* o1)
	{
		float dir[3] = {};

		if (fabs(normal[2]) < 0.5f)
		{
			dir[2] = 1.0f;
		}
		else
		{
			dir[1] = 1.0f;
		}

		utils::vector::cross_product(normal, dir, o1);
		utils::vector::normalize(o1);
		utils::vector::cross_product(normal, o1, o2);
		utils::vector::normalize(o2);
	}

	float ClampMin(float val, float min)
	{
		return min - val < 0.0f ? min : val;
	}

	void Vec3Lerp(const float* from, const float* to, float frac, float* out)
	{
		out[0] = ((to[0] - from[0]) * frac) + from[0];
		out[1] = ((to[1] - from[1]) * frac) + from[1];
		out[2] = ((to[2] - from[2]) * frac) + from[2];
	}

	void Vec4Lerp(const float* from, const float* to, float frac, float* out)
	{
		out[0] = ((to[0] - from[0]) * frac) + from[0];
		out[1] = ((to[1] - from[1]) * frac) + from[1];
		out[2] = ((to[2] - from[2]) * frac) + from[2];
		out[3] = ((to[3] - from[3]) * frac) + from[3];
	}

	// checked
	void QuatLerp(const float* qa, const float* qb, float frac, float* out)
	{
		if (qb[0] * qa[0] + qa[1] * qb[1] + qa[2] * qb[2] + qb[3] * qa[3] < 0.0f)
		{
			out[0] = -qb[0];
			out[1] = -qb[1];
			out[2] = -qb[2];
			out[3] = -qb[3];

			Vec4Lerp(qa, out, frac, out);
		}
		else
		{
			Vec4Lerp(qa, qb, frac, out);
		}
	}

	// checked
	void QuatToAxis(const float* quat, float* axis)
	{
		const float qa[4] =
		{
			quat[0] * quat[0], quat[1] * quat[1], quat[2] * quat[2], quat[3] * quat[3]
		};

		const auto mag_sqr = qa[0] + qa[1] + qa[2] + qa[3];
		const auto mag = 2.0f / mag_sqr;

		if (mag_sqr <= 0.0f)
		{
			Assert();
		}
		
		const float aaa[3] =
		{
			quat[0] * mag* quat[1],
			quat[0] * mag* quat[2],
			quat[0] * mag* quat[3]
		};

		const float bb[2] =
		{
			quat[1] * mag* quat[2],
			quat[1] * mag* quat[3]
		};

		const auto c = quat[2] * mag * quat[3];

		axis[0] = 1.0f - (mag * qa[2] + mag * qa[1]);
		axis[1] = c + aaa[0];
		axis[2] = aaa[1] - bb[1];
		axis[3] = aaa[0] - c;
		axis[4] = 1.0f - (mag * qa[2] + mag * qa[0]);
		axis[5] = bb[0] + aaa[2];
		axis[6] = aaa[1] + bb[1];
		axis[7] = bb[0] - aaa[2];
		axis[8] = 1.0f - (mag * qa[0] + mag * qa[1]);
	}
}