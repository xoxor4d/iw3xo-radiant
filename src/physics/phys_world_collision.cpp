#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace physics
{
	dGetColliderFnFn* Phys_GetColliderNull()
	{
		return nullptr;
	}

	void Phys_GetBrushmodelAABB([[maybe_unused]] dxGeom* geom, float* aabb)
	{
		aabb[0] = -3.4028235e38f;
		aabb[1] =  3.4028235e38f;
		aabb[2] = -3.4028235e38f;
		aabb[3] =  3.4028235e38f;
		aabb[4] = -3.4028235e38f;
		aabb[5] =  3.4028235e38f;
	}

	void Phys_GetBrushAABB(dxGeom* geom, float* aabb)
	{
		if (!geom || dGeomGetClass(geom) != GEOM_CLASS_BRUSH)
		{
			Assert();
		}

		aabb[0] = -3.4028235e38f;
		aabb[1] = -3.4028235e38f;
		aabb[2] = -3.4028235e38f;
		aabb[3] =  3.4028235e38f;
		aabb[4] =  3.4028235e38f;
		aabb[5] =  3.4028235e38f;
	}

	void Phys_GetCylinderAABB(dxGeom* geom, float* aabb)
	{
		if (!geom || dGeomGetClass(geom) != GEOM_CLASS_CYLINDER)
		{
			Assert();
		}

		const auto class_data = static_cast<GeomStateCylinder*>(dGeomGetClassData(geom));

		float axis[12];
		Phys_OdeMatrix3ToAxis(dGeomGetRotation(geom), (float(*)[3])axis);
		const auto geom_pos = dGeomGetPosition(geom);

		int axisIdx = class_data->direction - 1;
		
		if (axisIdx < 0 || axisIdx > 2)
		{
			Assert();
		}
		
		float dims[3];
		dims[axisIdx] = class_data->halfHeight;
		dims[(axisIdx + 1) % 3] = class_data->radius;
		dims[(axisIdx + 2) % 3] = class_data->radius;

		const auto d1 = fabs(fabs(axis[axisIdx] * dims[0]) + axis[axisIdx + 3] * dims[0]) + fabs(axis[axisIdx + 6] * dims[0]);
		aabb[0] = geom_pos[0] - d1;
		aabb[1] = geom_pos[0] + d1;

		axisIdx = (axisIdx + 1) % 3;
		const auto d2 = fabs(axis[axisIdx] * dims[1]) + fabs(axis[axisIdx + 3] * dims[1]) + fabs(axis[axisIdx + 6] * dims[1]);
		aabb[2] = geom_pos[1] - d2;
		aabb[3] = geom_pos[1] + d2;

		axisIdx = (axisIdx + 1) % 3;
		const auto d3 = fabs(axis[axisIdx] * dims[2]) + fabs(axis[axisIdx + 3] * dims[2]) + fabs(axis[axisIdx + 6] * dims[2]);
		aabb[4] = geom_pos[2] - d3;
		aabb[5] = geom_pos[2] + d3;
	}

	void Phys_GetCapsuleAABB(dxGeom* geom, float* aabb)
	{
		if (!geom || dGeomGetClass(geom) != GEOM_CLASS_CAPSULE)
		{
			Assert();
		}

		const auto class_data = static_cast<GeomStateCylinder*>(dGeomGetClassData(geom));

		float axis[12];
		Phys_OdeMatrix3ToAxis(dGeomGetRotation(geom), (float(*)[3])axis);
		const auto geom_pos = dGeomGetPosition(geom);

		const int axisIdx = class_data->direction - 1;
		if (axisIdx < 0 || axisIdx > 2)
		{
			Assert();
		}

		float dims[3];
		dims[0] = fabs(axis[axisIdx]);
		dims[1] = fabs(axis[axisIdx + 3]);
		dims[2] = fabs(axis[axisIdx + 6]);

		const auto d1 = dims[0] * class_data->halfHeight + class_data->radius;
		aabb[0] = geom_pos[0] - d1;
		aabb[1] = geom_pos[0] + d1;

		const auto d2 = dims[1] * class_data->halfHeight + class_data->radius;
		aabb[2] = geom_pos[1] - d2;
		aabb[3] = geom_pos[1] + d2;

		const auto d3 = dims[2] * class_data->halfHeight + class_data->radius;
		aabb[4] = geom_pos[2] - d3;
		aabb[5] = geom_pos[2] + d3;
	}

	void Phys_InitBrushmodelGeomClass()
	{
		dGeomClass geom = {};
		geom.collider = (int(__cdecl * (__cdecl*)(int))(dxGeom*, dxGeom*, int, dContactGeom*, int))Phys_GetColliderNull;
		geom.aabb = (void(__cdecl*)(dxGeom*, float*))Phys_GetBrushmodelAABB;
		geom.aabb_test = nullptr;
		geom.bytes = 16;

		if (dCreateGeomClass(&geom) != GEOM_CLASS_BRUSHMODEL)
		{
			Assert();
		}
	}

	void Phys_InitBrushGeomClass()
	{
		dGeomClass geom = {};
		geom.collider = (int(__cdecl * (__cdecl*)(int))(dxGeom*, dxGeom*, int, dContactGeom*, int))Phys_GetColliderNull;
		geom.aabb = (void(__cdecl*)(dxGeom*, float*))Phys_GetBrushAABB;
		geom.aabb_test = nullptr;
		geom.bytes = 16;

		if (dCreateGeomClass(&geom) != GEOM_CLASS_BRUSH)
		{
			Assert();
		}
	}

	void Phys_InitCylinderGeomClass()
	{
		dGeomClass geom = {};
		geom.collider = (int(__cdecl * (__cdecl*)(int))(dxGeom*, dxGeom*, int, dContactGeom*, int))Phys_GetColliderNull;
		geom.aabb = (void(__cdecl*)(dxGeom*, float*))Phys_GetCylinderAABB;
		geom.aabb_test = nullptr;
		geom.bytes = 12;

		if (dCreateGeomClass(&geom) != GEOM_CLASS_CYLINDER)
		{
			Assert();
		}
	}

	void Phys_InitCapsuleGeomClass()
	{
		dGeomClass geom = {};
		geom.collider = (int(__cdecl * (__cdecl*)(int))(dxGeom*, dxGeom*, int, dContactGeom*, int))Phys_GetColliderNull;
		geom.aabb = (void(__cdecl*)(dxGeom*, float*))Phys_GetCapsuleAABB;
		geom.aabb_test = nullptr;
		geom.bytes = 12;

		if (dCreateGeomClass(&geom) != GEOM_CLASS_CAPSULE)
		{
			Assert();
		}
	}
}
