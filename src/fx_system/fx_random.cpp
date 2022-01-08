#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) \
						__debugbreak();		\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__)

#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword

namespace fx_system
{
	void FX_RandomDir(int seed, float* dir)
	{
		const float height = (fx_randomTable[10 + seed] * 2.0f) - 1.0f;
		if (height < -1.0f || height > 1.0f)
		{
			Assert();
		}

		const float random2 = fx_randomTable[9 + seed] * 6.2831855f;
		const float cosYaw = cosf(random2);
		const float sinYaw = sinf(random2);

		const float radiusAtHeight = sqrt(1.0f - (height * height));
		dir[0] = radiusAtHeight * cosYaw;
		dir[1] = radiusAtHeight * sinYaw;
		dir[2] = height;

		if (!Vec3IsNormalized(dir))
		{
			Assert();
		}
	}
}