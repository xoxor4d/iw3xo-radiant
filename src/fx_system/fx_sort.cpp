#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword

namespace fx_system
{
	void FX_GetInsertSortElem(FxElem* elem, FxInsertSortElem* sortElem, FxSystem* system, FxEffect* effect)
	{
		game::orientation_t orient = {};
		float posWorld[3];

		sortElem->msecBegin = elem->msecBegin;
		sortElem->defIndex = (unsigned __int8)elem->defIndex;

		FxElemDef* elemDef = &effect->def->elemDefs[(unsigned __int8)elem->defIndex];
		sortElem->elemType = elemDef->elemType;

		if (elemDef->elemType > FX_ELEM_TYPE_LAST_SPRITE)
		{
			Assert();
		}

		sortElem->defSortOrder = (unsigned __int8)elemDef->sortOrder;
		FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, (elem->msecBegin + effect->randomSeed + 296 * (unsigned int)elem->sequence) % 0x1DF, &orient);
		FX_OrientationPosToWorldPos(&orient, elem->___u8.origin, posWorld);
		sortElem->distToCamSq = Vec3DistanceSq(posWorld, system->cameraPrev.origin);
	}

	bool FX_ExistingElemSortsBeforeNewElem(FxElem* remoteElem, FxEffect* effect, FxSystem* system, FxInsertSortElem* sortElemNew)
	{
		FxElemDef* elemDef = &effect->def->elemDefs[ static_cast<std::uint8_t>(remoteElem->defIndex) ];

		if (elemDef->elemType == FX_ELEM_TYPE_TRAIL)
		{
			Assert();
		}

		if (!(elemDef->elemType <= FX_ELEM_TYPE_TRAIL))
		{
			return true;
		}

		if (!elemDef->visualCount)
		{
			return false;
		}

		if (elemDef->sortOrder < sortElemNew->defSortOrder)
		{
			return true;
		}
		if (elemDef->sortOrder > sortElemNew->defSortOrder)
		{
			return false;
		}

		game::orientation_t orient = {};
		float posWorld[3];

		FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, (remoteElem->msecBegin + effect->randomSeed + 296 * (unsigned int)(unsigned __int8)remoteElem->sequence) % 0x1DF, &orient);
		FX_OrientationPosToWorldPos(&orient, remoteElem->___u8.origin, posWorld);

		return sortElemNew->distToCamSq < Vec3DistanceSq(posWorld, system->cameraPrev.origin);
	}

	// todo: clean me up
	void FX_SortSpriteElemIntoEffect(FxSystem* system, FxEffect* effect, FxElem* elem)
	{
		FxInsertSortElem sortElem = {};

		FxElem* nextElem = nullptr;

		int prevElemHandle = UINT16_MAX;
		std::uint16_t* nextElemHandle = effect->firstElemHandle;
		std::uint16_t* v15 = effect->firstElemHandle;

		if (!(effect->firstElemHandle[0] == UINT16_MAX))
		{
			FX_GetInsertSortElem(elem, &sortElem, system, effect);
			if (sortElem.defSortOrder < 0)
			{
				Assert();
			}

			while (true)
			{
				nextElem = FX_ElemFromHandle(system, *nextElemHandle);
				if (!FX_ExistingElemSortsBeforeNewElem(nextElem, effect, system, &sortElem))
				{
					break;
				}

				prevElemHandle = *v15;
				v15 = &nextElem->nextElemHandleInEffect;

				if (nextElem->nextElemHandleInEffect == UINT16_MAX)
				{
					break;
				}

				nextElemHandle = v15;
			}

			nextElemHandle = v15;
		}

		elem->nextElemHandleInEffect = *nextElemHandle;
		elem->prevElemHandleInEffect = static_cast<std::uint16_t>(prevElemHandle);

		const std::uint16_t elemHandle = FX_ElemToHandle(system->elems, elem);
		*nextElemHandle = elemHandle;

		if (elem->nextElemHandleInEffect != UINT16_MAX)
		{
			nextElem->prevElemHandleInEffect = elemHandle;
		}
	}

	// checked
	void FX_SortNewElemsInEffect(FxSystem* system, FxEffect* effect)
	{
		if (!system)
		{
			Assert();
		}

		std::uint16_t elemHandle = effect->firstElemHandle[0];
		const std::uint16_t stopElemHandle = effect->firstSortedElemHandle;

		if (elemHandle != effect->firstSortedElemHandle)
		{
			effect->firstElemHandle[0] = effect->firstSortedElemHandle;
			if (effect->firstSortedElemHandle != UINT16_MAX)
			{
				FX_ElemFromHandle(system, effect->firstSortedElemHandle)->prevElemHandleInEffect = UINT16_MAX;
			}

			while (true)
			{
				const auto elem = FX_ElemFromHandle(system, elemHandle);
				elemHandle = elem->nextElemHandleInEffect;

				FX_SortSpriteElemIntoEffect(system, effect, elem);

				if (elemHandle == stopElemHandle)
				{
					break;
				}
			}

			effect->firstSortedElemHandle = effect->firstElemHandle[0];
			const FxElem* elem = nullptr;

			for (std::uint16_t i = effect->firstElemHandle[0]; i != UINT16_MAX; i = elem->nextElemHandleInEffect)
			{
				elem = FX_ElemFromHandle(system, i);
				if(FX_GetEffectElemDef(effect, elem->defIndex)->elemType > FX_ELEM_TYPE_LAST_SPRITE)
				
				{
					Assert();
				}
			}
		}
	}


	int FX_CalcRunnerParentSortOrder(FxEffect* effect)
	{
		if (!effect || !effect->def)
		{
			Assert();
		}

		int totalSortOrder = 0;
		int totalNonRunnerElemDefs = 0;
		for (int elemDefIndex = 0; elemDefIndex < effect->def->elemDefCountLooping + effect->def->elemDefCountOneShot + effect->def->elemDefCountEmission; ++elemDefIndex)
		{
			FxElemDef* elemDef = &effect->def->elemDefs[elemDefIndex];
			if (elemDef->elemType != FX_ELEM_TYPE_RUNNER)
			{
				totalSortOrder += static_cast<std::uint8_t>(elemDef->sortOrder);
				++totalNonRunnerElemDefs;
			}
		}

		if (totalNonRunnerElemDefs <= 0)
		{
			return 0;
		}

		int order = 254;

		if (totalSortOrder / totalNonRunnerElemDefs < 254)
		{
			order = totalSortOrder / totalNonRunnerElemDefs;
		}

		return order > 0 ? order : 0;
	}

	bool FX_FirstEffectIsFurther(FxEffect* firstEffect, FxEffect* secondEffect)
	{
		
		//if (*((BYTE*)&firstEffect->boltAndSortOrder + 3) == 255)
		if(firstEffect->boltAndSortOrder.sortOrder == 255u)
		{
			if (secondEffect->boltAndSortOrder.sortOrder == 255u)
			{
				return false;
			}

			firstEffect->boltAndSortOrder.sortOrder = FX_CalcRunnerParentSortOrder(firstEffect);
		}

		if (secondEffect->boltAndSortOrder.sortOrder == 255u)
		{
			secondEffect->boltAndSortOrder.sortOrder = FX_CalcRunnerParentSortOrder(secondEffect);
		}

		return firstEffect->boltAndSortOrder.sortOrder < secondEffect->boltAndSortOrder.sortOrder;
	}

	void FX_SortEffects(FxSystem* system)
	{
		if (!system)
		{
			Assert();
		}

		int idx, idy;
		volatile int i, j;
		float sort_buffer[1024] = {};

		for (i = system->firstActiveEffect; i != system->firstNewEffect; ++i)
		{
			const std::uint16_t firstHandle = system->allEffectHandles[i & 0x3FF];
			FxEffect* firstEffect = FX_EffectFromHandle(system, firstHandle);

			float dist = Vec3DistanceSq(firstEffect->frameNow.origin, system->camera.origin);

			for (j = i; j != system->firstActiveEffect; --j)
			{
				idy = ((WORD)j - 1) & 0x3FF;
				if (sort_buffer[idy] >= dist * 0.99989998f)
				{
					if (sort_buffer[idy] >= dist * 1.0001f)
					{
						break;
					}

					FxEffect* secondEffect = FX_EffectFromHandle(system, system->allEffectHandles[idy]);
					if (secondEffect->owner != firstEffect->owner || !FX_FirstEffectIsFurther(firstEffect, secondEffect))
					{
						break;
					}
				}

				idx = j & 0x3FF;
				sort_buffer[idx] = sort_buffer[idy];
				system->allEffectHandles[idx] = system->allEffectHandles[idy];
			}

			idx = j & 0x3FF;
			sort_buffer[idx] = dist;
			system->allEffectHandles[idx] = firstHandle;
		}

		system->iteratorCount = 0;
	}
}