#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace fx_system
{
	int Clamp(int val, int min, int max)
	{
		if (min >= max)
		{
			Assert();
		}

		int result = val;
		if (val < min)
		{
			return min;
		}

		if (val > max)
		{
			result = max;
		}

		return result;
	}

	void SetIdentityAxis(float* axis)
	{
		axis[0] = 1.0;
		axis[1] = 0.0;
		axis[2] = 0.0;

		axis[3] = 0.0;
		axis[4] = 1.0;
		axis[5] = 0.0;

		axis[6] = 0.0;
		axis[7] = 0.0;
		axis[8] = 1.0;
	}

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

	void RotatePointAroundVector(float* dst, const float* dir, const float* point, const float degrees)
	{
		if(Vec3Compare(dir, game::vec3_origin))
		{
			Assert();
		}

		const float vf[3] =
		{
			dir[0], dir[1], dir[2]
		};

		float vup[3];
		float vr[3];

		PerpendicularVector(dir, vr);
		Vec3Cross(vr, vf, vup);

		
		float m[3][3] = {};
		m[0][0] = vr[0];
		m[1][0] = vr[1];
		m[2][0] = vr[2];
		m[0][1] = vup[0];
		m[1][1] = vup[1];
		m[2][1] = vup[2];
		m[0][2] = vf[0];
		m[1][2] = vf[1];
		m[2][2] = vf[2];

		float zrot[3][3] = {};
		zrot[0][0] = 1.0;
		zrot[0][1] = 0.0f;
		zrot[0][2] = 0.0f;
		zrot[1][0] = 0.0;
		zrot[1][1] = 1.0;
		zrot[1][2] = 0.0;
		zrot[2][0] = 0.0f;
		zrot[2][1] = 0.0f;
		zrot[2][2] = 1.0;

		float im[3][3];
		memcpy(im, m, sizeof(im));
		im[0][1] = vr[1];
		im[0][2] = vr[2];
		im[1][0] = vup[0];
		im[1][2] = vup[2];
		im[2][0] = vf[0];
		im[2][1] = vf[1];

		const float rad = degrees * 0.01745329238474369f;

		if (std::isnan(rad))
		{
			Assert();
		}

		zrot[0][0] = cos(rad);
		zrot[0][1] = sin(rad);

		if (std::isnan(zrot[0][1]))
		{
			Assert();
		}

		if (std::isnan(zrot[0][0]))
		{
			Assert();
		}

		zrot[1][0] = -zrot[0][1];
		zrot[1][1] =  zrot[0][0];

		MatrixMultiply(m, zrot, zrot);
		MatrixMultiply(zrot, im, m);

		dst[0] = m[0][0] * point[0] + m[0][1] * point[1] + point[2] * m[0][2];
		dst[1] = m[1][0] * point[0] + m[1][1] * point[1] + point[2] * m[1][2];
		dst[2] = m[2][0] * point[0] + m[2][1] * point[1] + point[2] * m[2][2];
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

#pragma warning( push )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4245 )
	game::PackedTexCoords Vec2PackTexCoords(float ucord, float texcoord)
	{
		std::uint16_t s0, t0;
		int s_temp, t_temp;

		if ((int)((2 * LODWORD(ucord)) ^ 0x80000000) >> 14 < 0x3FFF)
		{
			s_temp = (int)((2 * LODWORD(ucord)) ^ 0x80000000) >> 14;
		}
		else
		{
			s_temp = 0x3FFF;
		}

		if (s_temp > -16384)
		{
			s0 = s_temp;
		}
		else
		{
			s0 = -16384;
		}

		if ((int)((2 * LODWORD(texcoord)) ^ 0x80000000) >> 14 < 0x3FFF)
		{
			t_temp = (int)((2 * LODWORD(texcoord)) ^ 0x80000000) >> 14;
		}
		else
		{
			t_temp = 0x3FFF;
		}

		if (t_temp > -16384)
		{
			t0 = t_temp;
		}
		else
		{
			t0 = -16384;
		}

		return (game::PackedTexCoords)(( (t0 & 0x3FFF) | ((LODWORD(texcoord) >> 16) & 0xC000)) + (( (s0 & 0x3FFF) | ((LODWORD(ucord) >> 16) & 0xC000)) << 16));
	}
#pragma warning( pop )

	float Vec2Length(const float* v)
	{
		return sqrtf(v[0] * v[0] + v[1] * v[1]);
	}

	float Vec2Normalize(float* v)
	{
		float length = Vec2Length(v);
		if (-length >= 0.0f)
		{
			length = 1.0f;
		}

		v[0] = v[0] * (1.0f / length);
		v[1] = v[1] * (1.0f / length);

		return length;
	}

	float Vec3Distance(const float* p1, const float* p2)
	{
		float dir[3];
		dir[0] = p2[0] - p1[0];
		dir[1] = p2[1] - p1[1];
		dir[2] = p2[2] - p1[2];

		return Abs(dir);
	}

	float Vec3DistanceSq(const float* p1, const float* p2)
	{
		const float d1 = p2[1] - p1[1];
		const float d2 = p2[2] - p1[2];

		return d2 * d2 + d1 * d1 + (p2[0] - p1[0]) * (p2[0] - p1[0]);
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

	char Byte1PackClamp(const float from)
	{
		const int pack = static_cast<int>( static_cast<double>( from * 255.0f ) + 9.313225746154785e-10 );

		if(pack > 255)
		{
			return -1;
		}

		if(pack < 0)
		{
			return 0;
		}

		return static_cast<char>(pack);
	}

	void Byte4PackVertexColor(const float* from, char* to)
	{
		to[2] = Byte1PackClamp(from[0]);
		to[1] = Byte1PackClamp(from[1]);
		to[0] = Byte1PackClamp(from[2]);
		to[3] = Byte1PackClamp(from[3]);
	}

	float Abs(const float* v)
	{
		return sqrtf((v[0] * v[0] + v[1] * v[1]) + v[2] * v[2]);
	}

#pragma warning( push )
#pragma warning( disable : 6385 )
#pragma warning( disable : 6386 )
	void MatrixMultiply(const float(*in1)[3], const float(*in2)[3], float(*out)[3])
	{
		(*out)[0] = (*in1)[0] * (*in2)[0] + (*in1)[1] * (*in2)[3] + (*in1)[2] * (*in2)[6];
		(*out)[1] = (*in2)[4] * (*in1)[1] + (*in2)[1] * (*in1)[0] + (*in1)[2] * (*in2)[7];
		(*out)[2] = (*in2)[5] * (*in1)[1] + (*in2)[2] * (*in1)[0] + (*in1)[2] * (*in2)[8];
		(*out)[3] = (*in1)[4] * (*in2)[3] + (*in1)[3] * (*in2)[0] + (*in2)[6] * (*in1)[5];
		(*out)[4] = (*in1)[4] * (*in2)[4] + (*in1)[3] * (*in2)[1] + (*in1)[5] * (*in2)[7];
		(*out)[5] = (*in1)[4] * (*in2)[5] + (*in1)[3] * (*in2)[2] + (*in1)[5] * (*in2)[8];
		(*out)[6] = (*in1)[7] * (*in2)[3] + (*in1)[6] * (*in2)[0] + (*in2)[6] * (*in1)[8];
		(*out)[7] = (*in1)[7] * (*in2)[4] + (*in1)[6] * (*in2)[1] + (*in1)[8] * (*in2)[7];
		(*out)[8] = (*in1)[7] * (*in2)[5] + (*in1)[6] * (*in2)[2] + (*in1)[8] * (*in2)[8];
	}
#pragma warning( pop )
}