#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	void		FX_GetInsertSortElem(FxElem* elem, FxInsertSortElem* sortElem, FxSystem* system, FxEffect* effect);
	bool		FX_ExistingElemSortsBeforeNewElem(FxElem* remoteElem, FxEffect* effect, FxSystem* system, FxInsertSortElem* sortElemNew);
	void		FX_SortSpriteElemIntoEffect(FxSystem* system, FxEffect* effect, FxElem* elem);
	void		FX_SortNewElemsInEffect(FxSystem* system, FxEffect* effect);

	int			FX_CalcRunnerParentSortOrder(FxEffect* effect);
	bool		FX_FirstEffectIsFurther(FxEffect* firstEffect, FxEffect* secondEffect);
	void		FX_SortEffects(FxSystem* system);
}