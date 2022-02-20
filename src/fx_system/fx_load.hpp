#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	extern $145C5CACE7A579404A9D7C1B73F29F79 fx_load;
	extern const FxFlagDef s_allFlagDefs[41];

	// * ----------------------

	void*			FX_AllocMem(size_t size);
	void			FX_UnregisterAll();
	FxEffectDef*	FX_Register(const char* name);

	FxCurve*		FxCurve_AllocAndCreateWithKeys(const float* keyArray, int dimensionCount, int keyCount);
	bool			FX_LoadEditorEffect(const char* name, FxEditorEffectDef* edEffectDef);
	FxEffectDef*	FX_Load(const char* name);
}