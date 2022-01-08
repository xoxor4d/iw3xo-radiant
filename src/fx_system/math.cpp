#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) \
						__debugbreak();		\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__)

namespace fx_system
{
	void AxisCopy(const float(*in)[3], float(*out)[3])
	{
		(*out)[0] = (*in)[0];
		(*out)[1] = (*in)[1];
		(*out)[2] = (*in)[2];
		(*out)[3] = (*in)[3];
		(*out)[4] = (*in)[4];
		(*out)[5] = (*in)[5];
		(*out)[6] = (*in)[6];
		(*out)[7] = (*in)[7];
		(*out)[8] = (*in)[8];
	}

	void PerpendicularVector(const float* src, float* dst)
	{
		if (!Vec3IsNormalized(src))
		{
			Assert();
		}

		float srcSq[3];
		srcSq[0] = src[0] * src[0];
		srcSq[1] = src[1] * src[1];
		srcSq[2] = src[2] * src[2];

		int pos = srcSq[0] > srcSq[1];
		if (srcSq[pos] > srcSq[2])
		{
			pos = 2;
		}

		dst[0] = -src[pos] * src[0];
		dst[1] = -src[pos] * src[1];
		dst[2] = -src[pos] * src[2];
		dst[pos] = dst[pos] + 1.0f;

		Vec3Normalize(dst);
	}

	void AxisToQuat(const float(*mat)[3], float* out)
	{
		int best;
		float test[4][4];

		test[0][0] = (*mat)[5] - (*mat)[7];
		test[0][1] = (*mat)[6] - (*mat)[2];
		test[0][2] = (*mat)[1] - (*mat)[3];
		test[0][3] = (*mat)[0] + (*mat)[4] + (*mat)[8] + 1.0f;

		float testSizeSq = Vec4LengthSq(test[0]);
		if (testSizeSq < 1.0f)
		{
			test[1][0] = (*mat)[6] + (*mat)[2];
			test[1][1] = (*mat)[7] + (*mat)[5];
			test[1][2] = (*mat)[8] - (*mat)[4] - (*mat)[0] + 1.0f;
			test[1][3] = test[0][2];

			testSizeSq = Vec4LengthSq(test[1]);
			if (testSizeSq < 1.0f)
			{
				test[2][0] = (*mat)[0] - (*mat)[4] - (*mat)[8] + 1.0f;
				test[2][1] = (*mat)[3] + (*mat)[1];
				test[2][2] = test[0][0];
				test[2][3] = test[1][0];

				testSizeSq = Vec4LengthSq(test[2]);
				if (testSizeSq < 1.0f)
				{
					test[3][0] = test[2][1];
					test[3][1] = (*mat)[4] - (*mat)[0] - (*mat)[8] + 1.0f;
					test[3][2] = test[0][1];
					test[3][3] = test[1][1];

					testSizeSq = Vec4LengthSq(test[3]);
					if (testSizeSq < 1.0f)
					{
						Assert();
					}

					best = 3;
				}
				else { best = 2; }
			}
			else { best = 1; }
		}
		else
		{
			best = 0;
		}
		if (testSizeSq == 0.0f)
		{
			Assert();
		}

		const float invLength = 1.0f / sqrtf(testSizeSq);
		out[0] = invLength * test[best][0];
		out[1] = invLength * test[best][1];
		out[2] = invLength * test[best][2];
		out[3] = invLength * test[best][3];
	}

	void UnitQuatToAxis(const float* quat, float(*axis)[3])
	{
		float xx, xy, xz, xw;
		float yy, yz, yw;
		float zz, zw;

		if (!Vec4IsNormalized(quat))
		{
			Assert();
		}

		const float scaledX = quat[0] + quat[0];
		xx = scaledX * quat[0];
		xy = scaledX * quat[1];
		xz = scaledX * quat[2];
		xw = scaledX * quat[3];

		const float scaledY = quat[1] + quat[1];
		yy = scaledY * quat[1];
		yz = scaledY * quat[2];
		yw = scaledY * quat[3];

		const float scaledZ = quat[2] + quat[2];
		zz = scaledZ * quat[2];
		zw = scaledZ * quat[3];

		(*axis)[0] = 1.0f - (yy + zz);
		(*axis)[1] = xy + zw;
		(*axis)[2] = xz - yw;
		(*axis)[3] = xy - zw;
		(*axis)[4] = 1.0f - (xx + zz);
		(*axis)[5] = yz + xw;
		(*axis)[6] = xz + yw;
		(*axis)[7] = yz - xw;
		(*axis)[8] = 1.0f - (xx + yy);
	}

	float Vec3LengthSq(const float* v)
	{
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	}

	bool Vec3IsNormalized(const float* v)
	{
		return fabs(Vec3LengthSq(v) - 1.0f) < 0.0020000001f;
	}

	bool Vec3Compare(const float* v, const float* f)
	{
		return v[0] == f[0] && v[1] == f[1] && v[2] == f[2];
	}

	float Vec3Normalize(float* v)
	{
		float length = sqrt(Vec3LengthSq(v));
		if (-length >= 0.0f)
		{
			length = 1.0f;
		}
		
		v[0] = v[0] * (1.0f / length);
		v[1] = v[1] * (1.0f / length);
		v[2] = v[2] * (1.0f / length);

		return length;
	}

	void Vec3Cross(const float* v0, const float* v1, float* cross)
	{
		if (v0 == cross || v1 == cross)
		{
			Assert();
		}

		cross[0] = (v0[1] * v1[2]) - (v0[2] * v1[1]);
		cross[1] = (v0[2] * v1[0]) - (v0[0] * v1[2]);
		cross[2] = (v0[0] * v1[1]) - (v0[1] * v1[0]);
	}

	void Vec3Basis_RightHanded(const float* forward, float* left, float* up)
	{
		PerpendicularVector(forward, up);
		Vec3Cross(up, forward, left);
	}

	float Vec4LengthSq(const float* v)
	{
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
	}

	bool Vec4IsNormalized(const float* v)
	{
		return fabs(Vec4LengthSq(v) - 1.0f) < 0.0020000001f;
	}

	float Vec4Normalize(float* v)
	{
		const float length = sqrtf(Vec4LengthSq(v));
		if (length != 0.0f)
		{
			v[0] = v[0] * (1.0f / length);
			v[1] = v[1] * (1.0f / length);
			v[2] = v[2] * (1.0f / length);
			v[3] = v[3] * (1.0f / length);
		}

		return length;
	}

	float Abs(const float* v)
	{
		return sqrtf((v[0] * v[0] + v[1] * v[1]) + v[2] * v[2]);
	}
}