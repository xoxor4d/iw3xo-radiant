#pragma once

namespace fx_system
{
	int		Clamp(int val, int min, int max);
	void	SetIdentityAxis(float* axis);
	void	AxisCopy(const float(*in)[3], float(*out)[3]);
	void	PerpendicularVector(const float* src, float* dst);
	void	RotatePointAroundVector(float* dst, const float* dir, const float* point, const float degrees);
	void	AxisToQuat(const float(*mat)[3], float* out);
	void	UnitQuatToAxis(const float* quat, float(*axis)[3]);

	game::PackedTexCoords Vec2PackTexCoords(float ucord, float texcoord);
	float	Vec2Length(const float* v);
	float	Vec2Normalize(float* v);

	float	Vec3Distance(const float* p1, const float* p2);
	float	Vec3DistanceSq(const float* p1, const float* p2);
	float	Vec3LengthSq(const float* v);
	bool	Vec3IsNormalized(const float* v);
	bool	Vec3Compare(const float* v, const float* f);
	float	Vec3Normalize(float* v);
	void	Vec3Cross(const float* v0, const float* v1, float* cross);
	void	Vec3Basis_RightHanded(const float* forward, float* left, float* up);

	float	Vec4LengthSq(const float* v);
	bool	Vec4IsNormalized(const float* v);
	float	Vec4Normalize(float* v);

	void	Byte4PackVertexColor(const float* from, char* to);

	float	Abs(const float* v);
	void	MatrixMultiply(const float(*in1)[3], const float(*in2)[3], float(*out)[3]);


}