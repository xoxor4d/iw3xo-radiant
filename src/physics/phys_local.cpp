#include "std_include.hpp"

namespace physics
{
	int Phys_IndexFromODEWorld(dxWorld* world)
	{
		int id = 0;

		while (world != physGlob.world[id])
		{
			if (++id >= 3)
			{
				Assert(); 
				return 3;
			}
		}

		return id;
	}

	void Phys_OdeMatrix3ToAxis(const float* inMatrix, float(*outAxis)[3])
	{
		for (auto r = 0; r < 3; ++r)
		{
			for (auto c = 0; c < 3; ++c)
			{
				(*outAxis)[3 * c + r] = inMatrix[c + 4 * r];
			}
		}
	}

	void Phys_AxisToOdeMatrix3(const float(*inAxis)[3], float* outMatrix)
	{
		for (auto r = 0; r < 3; ++r)
		{
			for (auto c = 0; c < 3; ++c)
			{
				outMatrix[c + 4 * r] = (*inAxis)[3 * c + r];
			}
				
			outMatrix[4 * r + 3] = 0.0f;
		}
	}

	// checked
	void AxisTransformVec3(const float* v1, const float* v2, float* out)
	{
		out[0] = v1[3] * v2[1] + v1[0] * v2[0] + v1[6] * v2[2];
		out[1] = v1[1] * v2[0] + v1[4] * v2[1] + v1[7] * v2[2];
		out[2] = v1[2] * v2[0] + v1[5] * v2[1] + v1[8] * v2[2];
	}
}