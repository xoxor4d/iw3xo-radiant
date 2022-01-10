#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) \
						__debugbreak();		\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__)

#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword

namespace fx_system
{
	FxElemVisuals FX_GetElemVisuals(FxElemDef* elemDef, int randomSeed)
	{
		FxElemVisuals result = {};

		if (!elemDef->visualCount)
		{
			Assert();
		}

		if (elemDef->visualCount == 1)
		{
			result.anonymous = elemDef->visuals.instance.anonymous;
		}
		else
		{
			result.anonymous = elemDef->visuals.markArray->materials[(elemDef->visualCount * static_cast<std::uint16_t>(fx_randomTable[21 + randomSeed])) >> 16];
		}
		return result;
	}
}