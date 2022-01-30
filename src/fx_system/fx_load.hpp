#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	extern $145C5CACE7A579404A9D7C1B73F29F79 fx_load;
	extern const FxFlagDef s_allFlagDefs[];

	// * ----------------------

	FxEffectDef*	FX_Register(const char* name);
	FxEffectDef*	FX_Load(const char* name);
}