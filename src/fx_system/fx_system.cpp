#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) \
						__debugbreak();		\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__)

#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword

namespace fx_system
{
	FxSystem fx_systemPool = {};

	// *

	FxSystem* FX_GetSystem([[maybe_unused]] int localClientNum)
	{
		return &fx_systemPool;
	}

	bool FX_GetEffectStatus(FxEffect* effect)
	{
		if (!effect)
		{
			
			Assert();
		}

		return effect->status == 1;
	}

	void FX_StopEffectNonRecursive(FxEffect* effect, FxSystem* system)
	{
		if (!effect)
		{
			Assert();
		}

		if ((effect->status & FX_STATUS_HAS_PENDING_LOOP_ELEMS) != 0)
		{
			effect->status &= ~FX_STATUS_HAS_PENDING_LOOP_ELEMS;
			FX_DelRefToEffect(system, effect);
		}
	}

	void FX_StopEffect(FxSystem* system, FxEffect* effect)
	{
		FxEffect* o_effect = effect;
		if (!effect)
		{
			Assert();
		}

		if ((effect->status & FX_STATUS_REF_COUNT_MASK) != 0)
		{
			++effect->status;
			FX_StopEffectNonRecursive(effect, system);

			if ((effect->status & FX_STATUS_OWNED_EFFECTS_MASK) != 0)
			{
				const std::uint16_t stoppedEffectHandle = FX_EffectToHandle(system, effect);

				if (system->isArchiving)
				{
					Assert();
				}

				for (volatile int activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
				{
					if (system->allEffectHandles[activeIndex & 0x3FF] != stoppedEffectHandle)
					{
						FxEffect* otherEffect = FX_EffectFromHandle(system, system->allEffectHandles[activeIndex & 0x3FF]);
						if (otherEffect->owner == stoppedEffectHandle)
						{
							FX_StopEffect(system, otherEffect);
						}
					}
				}

				if (system->needsGarbageCollection)
				{
					FX_RunGarbageCollectionAndPrioritySort(system);
				}

				o_effect = effect;
			}

			FX_DelRefToEffect(system, o_effect);
		}
		else if ((effect->status & FX_STATUS_HAS_PENDING_LOOP_ELEMS) != 0)
		{
			Assert();
		}
	}

	// FxEffect* FX_SpawnEffect(FxSystem* system, FxEffectDef* remoteDef, int msecBegin, const float* origin, const float(*axis)[3], int dobjHandle, int boneIndex, int runnerSortOrder, unsigned __int16 owner, unsigned int markEntnum)
	FxEffect* FX_SpawnEffect([[maybe_unused]] FxSystem* system, [[maybe_unused]] FxEffectDef* remoteDef, [[maybe_unused]] int msecBegin, [[maybe_unused]] const float* origin, [[maybe_unused]] const float(*axis)[3], [[maybe_unused]] int dobjHandle, [[maybe_unused]] int boneIndex, [[maybe_unused]] int runnerSortOrder, [[maybe_unused]] unsigned __int16 owner, [[maybe_unused]] unsigned int markEntnum)
	{
		// todo: not implemented

		Assert();
		return nullptr;
	}

	void FX_SpawnDeathEffect(FxUpdateElem* update, FxSystem* system)
	{
		game::orientation_t orient = {};
		float origin[3];

		FxElemDef* elemDef = FX_GetUpdateElemDef(update);
		FX_GetOrientation(elemDef, &update->effect->frameAtSpawn, &update->effect->framePrev, update->randomSeed, &orient);
		FX_OrientationPosToWorldPos(&orient, update->elemOrigin, origin);

		FxEffect* effect = FX_SpawnEffect(system, elemDef->effectOnDeath.handle, update->msecUpdateBegin, origin, orient.axis, FX_SPAWN_DOBJ_HANDLES, FX_SPAWN_BONE_INDEX, 255, update->effect->owner, FX_SPAWN_MARK_ENTNUM);
		if (effect)
		{
			FX_DelRefToEffect(system, effect);
		}
	}

	std::uint16_t FX_EffectToHandle(FxSystem* system, FxEffect* effect)
	{
		if (!system)
		{
			Assert();
		}

		if (!effect || (effect <= &system->effects[0]) || effect >= &system->effects[FX_EFFECT_LIMIT])
		{
			Assert();
		}

		return static_cast<std::uint16_t>(((char*)effect - (char*)system->effects) / 4);
	}

	FxEffect* FX_EffectFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		if (!system)
		{
			Assert();
		}

		// if ( handle >= 32768u || (handle & 31) != 0 )
		// handle < FX_EFFECT_LIMIT * sizeof( FxEffect ) / FxEffect::HANDLE_SCALE && handle % (sizeof( FxEffect ) / FxEffect::HANDLE_SCALE) == 0

		if(handle >= (FX_EFFECT_LIMIT * sizeof(FxEffect) / FX_EFFECT_HANDLE_SCALE) || handle % (sizeof(FxEffect) / FX_EFFECT_HANDLE_SCALE) == 0)
		{
			Assert();
		}

		return (system->effects + 4 * handle);
	}

	FxElem* FX_ElemFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		// if ( handle < 20480u && !(handle % 10u) )
		// "handle < LIMIT * sizeof( ITEM_TYPE ) / ITEM_TYPE::HANDLE_SCALE && handle % (sizeof( ITEM_TYPE ) / ITEM_TYPE::HANDLE_SCALE) == 0

		if (handle >= (FX_ELEM_LIMIT * sizeof(FxElem) / FX_ELEM_HANDLE_SCALE) || handle % (sizeof(FxElem) / FX_ELEM_HANDLE_SCALE) == 0)
		{
			Assert();
		}

		return reinterpret_cast<FxElem*>(system->elems + 4 * handle);
	}

	FxTrailElem* FX_TrailElemFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		// if ( handle >= 16384u || (handle & 7) != 0 )
		if (handle >= (FX_TRAILELEM_LIMIT * sizeof(FxTrailElem) / FX_TRAILELEM_HANDLE_SCALE) || handle % (sizeof(FxTrailElem) / FX_TRAILELEM_HANDLE_SCALE) == 0)
		{
			Assert();
		}

		return reinterpret_cast<FxTrailElem*>(system->trailElems + 4 * handle);
	}

	FxTrail* FX_TrailFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		// if ( trailHandle >= 256u || (trailHandle & 1) != 0 )
		if (handle >= (FX_TRAIL_LIMIT * sizeof(FxTrail) / FX_TRAIL_HANDLE_SCALE) || handle % (sizeof(FxTrail) / FX_TRAIL_HANDLE_SCALE) == 0)
		{
			Assert();
		}

		return reinterpret_cast<FxTrail*>(system->trails + 4 * handle);
	}



	void FX_DelRefToEffect(FxSystem* system, FxEffect* effect)
	{
		if (!effect)
		{
			Assert();
		}

		if(LOWORD(effect->status) == 1)
		{
			FX_EffectNoLongerReferenced(system, effect);
		}

		--effect->status;
	}

	void FX_EffectNoLongerReferenced(FxSystem* system, FxEffect* remoteEffect)
	{
		if (!remoteEffect)
		{
			Assert();
		}

		if (!remoteEffect->status)
		{
			Assert();
		}

		if ((remoteEffect->status & FX_STATUS_OWNED_EFFECTS_MASK) != 0)
		{
			Assert();
		}

		FxEffect* owner = FX_EffectFromHandle(system, remoteEffect->owner);

		if ((remoteEffect->status & FX_STATUS_SELF_OWNED) == 0)
		{
			if ((owner->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0)
			{
				Assert();
			}

			int oldStatusValue = owner->status;
			owner->status = oldStatusValue - 0x20000;

			//if (((oldStatusValue ^ (oldStatusValue - 0x20000)) & 0xF801FFFF) != 0)
			if(((oldStatusValue & ~FX_STATUS_OWNED_EFFECTS_MASK) != ((oldStatusValue - (1 << FX_STATUS_OWNED_EFFECTS_SHIFT)) & ~FX_STATUS_OWNED_EFFECTS_MASK)))
			{
				Assert();
			}

			FX_DelRefToEffect(system, owner);
		}
		system->needsGarbageCollection = true;
	}




	void FX_FreePool_Generic_FxElem_FxElemContainer_(volatile int* firstFreeIndex, FxElem* item_slim, FxPool_FxElem* pool)
	{
		const int freedIndex = ((char*)item_slim - (char*)pool) / 40;
		if (freedIndex < 0 || freedIndex >= FX_ELEM_LIMIT)
		{
			Assert();
		}

		if (*firstFreeIndex != -1 && (*firstFreeIndex < 0 || *firstFreeIndex >= FX_ELEM_LIMIT))
		{
			Assert();
		}

		*(DWORD*)&item_slim->defIndex = *firstFreeIndex;
		*firstFreeIndex = freedIndex;
	}

	void FX_FreePool_Generic_FxTrail_FxTrail_(FxTrail* item_slim, volatile int* firstFreeIndex, FxPool_FxTrail* pool)
	{
		const int freedIndex = ((char*)item_slim - (char*)pool) >> 3;
		if (freedIndex < 0 || freedIndex >= FX_TRAIL_LIMIT)
		{
			Assert();
		}

		if (*firstFreeIndex != -1 && (*firstFreeIndex < 0 || *firstFreeIndex >= FX_TRAIL_LIMIT))
		{
			Assert();
		}

		*(DWORD*)&item_slim->defIndex = *firstFreeIndex;
		*firstFreeIndex = freedIndex;
	}

	void FX_FreePool_Generic_FxTrailElem_FxTrailElem_(FxTrailElem* item_slim, volatile int* firstFreeIndex, FxPool_FxTrailElem* pool)
	{
		const int freedIndex = ((char*)item_slim - (char*)pool) >> 5;
		if (freedIndex < 0 || freedIndex >= FX_TRAILELEM_LIMIT)
		{
			Assert();
		}

		if (*firstFreeIndex != -1 && (*firstFreeIndex < 0 || *firstFreeIndex >= FX_TRAILELEM_LIMIT))
		{
			Assert();
		}

		LODWORD(item_slim->origin[0]) = *firstFreeIndex;
		*firstFreeIndex = freedIndex;
	}

	void FX_FreeElem(FxSystem* system, unsigned __int16 elemHandle, FxEffect* effect, unsigned int elemClass)
	{
		if (!system)
		{
			Assert();
		}

		FxElem* remoteElem = FX_ElemFromHandle(system, elemHandle);

		if (!elemClass && effect->firstSortedElemHandle == elemHandle)
		{
			effect->firstSortedElemHandle = remoteElem->nextElemHandleInEffect;
		}

		const std::uint16_t prevElemHandle = remoteElem->prevElemHandleInEffect;
		const std::uint16_t nextElemHandle = remoteElem->nextElemHandleInEffect;

		if (nextElemHandle != 0xFFFF)
		{
			FX_ElemFromHandle(system, nextElemHandle)->prevElemHandleInEffect = remoteElem->prevElemHandleInEffect;
		}
		
		if (prevElemHandle == 0xFFFF)
		{
			if (effect->firstElemHandle[elemClass] != elemHandle)
			{
				Assert();
			}

			effect->firstElemHandle[elemClass] = remoteElem->nextElemHandleInEffect;
		}
		else
		{
			FX_ElemFromHandle(system, prevElemHandle)->nextElemHandleInEffect = remoteElem->nextElemHandleInEffect;
		}

		FxElemDef* elemDef = &effect->def->elemDefs[static_cast<std::uint8_t>(remoteElem->defIndex)];
		if (elemDef->elemType == 5 && (elemDef->flags & 0x8000000) != 0)
		{
			if (remoteElem->___u8.physObjId)
			{
				// #PHYS
				//Phys_ObjDestroy(1, (DWORD*)remoteElem->___u8.physObjId);
			}
		}

		memset(&remoteElem->defIndex, 0, sizeof(FxElem));

		FX_FreePool_Generic_FxElem_FxElemContainer_(&system->firstFreeElem, remoteElem, system->elems);
		FX_DelRefToEffect(system, effect);

		--system->activeElemCount;
	}

	void FX_FreeTrailElem(FxSystem* system, std::int16_t trailElemHandle, FxEffect* effect, FxTrail* trail)
	{
		FxTrailElem* remoteTrailElem; // ebx

		if (!system || !effect)
		{
			Assert();
		}

		remoteTrailElem = FX_TrailElemFromHandle(system, trailElemHandle);

		if (trail->firstElemHandle != trailElemHandle)
		{
			Assert();
		}

		if (trail->lastElemHandle == trailElemHandle)
		{
			if (trail->firstElemHandle != trailElemHandle)
			{
				Assert();
			}

			trail->lastElemHandle = INT16_MAX; // -1
		}

		trail->firstElemHandle = remoteTrailElem->nextTrailElemHandle;
		remoteTrailElem->origin[0] = 0.0;
		remoteTrailElem->origin[1] = 0.0;
		remoteTrailElem->origin[2] = 0.0;
		remoteTrailElem->spawnDist = 0.0;
		remoteTrailElem->msecBegin = 0;
		*(DWORD*)&remoteTrailElem->nextTrailElemHandle = 0;
		*(DWORD*)&remoteTrailElem->basis[0][0] = 0;
		*(DWORD*)&remoteTrailElem->basis[1][1] = 0;

		FX_FreePool_Generic_FxTrailElem_FxTrailElem_(remoteTrailElem, &system->firstFreeTrailElem, system->trailElems);
		FX_DelRefToEffect(system, effect);

		--system->activeTrailElemCount;
	}

	void FX_FreeSpotLightElem(FxSystem* system, [[maybe_unused]] unsigned __int16 elemHandle, FxEffect* effect)
	{
		if (system->activeSpotLightEffectCount <= 0 || system->activeSpotLightElemCount <= 0)
		{
			Assert();
		}

		FxElem* elem = FX_ElemFromHandle(system, system->activeSpotLightElemHandle);
		memset(&elem->defIndex, 0, sizeof(FxElem));

		FX_FreePool_Generic_FxElem_FxElemContainer_((int*)&system->firstFreeElem, elem, system->elems);
		FX_DelRefToEffect(system, effect);
		
		--system->activeElemCount;
		--system->activeSpotLightElemCount;
	}



	void FX_RemoveAllEffectElems(FxEffect* effect, FxSystem* system)
	{
		if (!effect || !effect->status)
		{
			Assert();
		}

		++effect->status;
		FX_StopEffect(system, effect);

		for (unsigned int elemClass = 0; elemClass < 3; ++elemClass)
		{
			while (effect->firstElemHandle[elemClass] != 0xFFFF)
			{
				const std::uint16_t elemHandle = effect->firstElemHandle[elemClass];
				FX_FreeElem(system, elemHandle, effect, elemClass);

				if (effect->firstElemHandle[elemClass] == elemHandle)
				{
					Assert();
				}
			}
		}

		FxTrail* trail = nullptr;
		for (std::uint16_t trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->nextTrailHandle)
		{
			for (trail = FX_TrailFromHandle(system, trailHandle); trail->firstElemHandle != 0xFFFF; )
			{
				FX_FreeTrailElem(system, trail->firstElemHandle, effect, trail);
			}
		}

		if (system->activeSpotLightElemCount > 0 && effect == FX_EffectFromHandle(system, system->activeSpotLightEffectHandle))
		{
			FX_FreeSpotLightElem(system, system->activeSpotLightElemHandle, effect);
		}

		FX_DelRefToEffect(system, effect);
	}

	void FX_KillEffect(FxSystem* system, FxEffect* effect)
	{
		if (!effect || !effect->status)
		{
			Assert();
		}

		FxEffect* o_effect = effect;

		++effect->status;
		FX_RemoveAllEffectElems(effect, system);

		if ((effect->status & FX_STATUS_OWNED_EFFECTS_MASK) != 0)
		{
			const std::uint16_t handle = FX_EffectToHandle(system, effect);
			if (system->isArchiving)
			{
				Assert();
			}

			for (volatile int activeIndex = system->firstActiveEffect; (effect->status & FX_STATUS_OWNED_EFFECTS_MASK) != 0; ++activeIndex)
			{
				if (activeIndex == system->firstNewEffect)
				{
					Assert();
				}

				const int ref = activeIndex & 0x3FF;

				if (system->allEffectHandles[ref] != handle)
				{
					FxEffect* otherEffect = FX_EffectFromHandle(system, system->allEffectHandles[ref]);
					if (otherEffect->owner == handle)
					{
						if ((otherEffect->status & FX_STATUS_OWNED_EFFECTS_MASK) != 0)
						{
							Assert();
						}

						if (otherEffect->status)
						{
							FX_RemoveAllEffectElems(otherEffect, system);
						}
					}
				}
			}

			if (system->needsGarbageCollection)
			{
				FX_RunGarbageCollectionAndPrioritySort(system);
			}

			o_effect = effect;
		}

		FX_DelRefToEffect(system, o_effect);
	}



	void FX_RunGarbageCollection_FreeTrails(FxSystem* system, FxEffect* effect)
	{
		for (; effect->firstTrailHandle != FX_STATUS_REF_COUNT_MASK; --system->activeTrailCount)
		{
			if (!system)
			{
				Assert();
			}

			FxTrail* remoteTrail = FX_TrailFromHandle(system, effect->firstTrailHandle);
			effect->firstTrailHandle = remoteTrail->nextTrailHandle;

			*(DWORD*)&remoteTrail->nextTrailHandle = 0;
			*(DWORD*)&remoteTrail->lastElemHandle = 0;

			FX_FreePool_Generic_FxTrail_FxTrail_(remoteTrail, &system->firstFreeTrail, system->trails);
		}
	}

	void FX_RunGarbageCollection_FreeSpotLight(FxSystem* system, unsigned __int16 effectHandle)
	{
		if (system->activeSpotLightEffectCount)
		{
			if (system->activeSpotLightEffectHandle == effectHandle)
			{
				if (system->activeSpotLightEffectCount != 1)
				{
					Assert();
				}

				--system->activeSpotLightEffectCount;
			}
		}
	}

	void FX_RunGarbageCollectionAndPrioritySort(FxSystem* system)
	{
		if (!system)
		{
			Assert();
		}

		if (system->needsGarbageCollection)
		{
			if (system->isArchiving)
			{
				Assert();
			}

			volatile int activeIndex = system->firstNewEffect;
			system->needsGarbageCollection = false;

			if (activeIndex != system->firstActiveEffect)
			{
				int freedCount = 0;
				unsigned __int16 freedHandles[1024];

				while (activeIndex != system->firstActiveEffect)
				{
					const std::uint16_t effectHandle = system->allEffectHandles[--activeIndex & 0x3FF];
					FxEffect* effect = FX_EffectFromHandle(system, effectHandle);

					if ((effect->status & FX_STATUS_OWNED_EFFECTS_MASK) != 0) //if ((unsigned __int16)effect->status)
					{
						system->allEffectHandles[((WORD)freedCount + (WORD)activeIndex) & 0x3FF] = effectHandle;
					}
					else
					{
						FX_RunGarbageCollection_FreeTrails(system, effect);
						FX_RunGarbageCollection_FreeSpotLight(system, effectHandle);
						freedHandles[freedCount++] = effectHandle; //*((WORD*)&effectHandle[1] + freedCount++) = effectHandle[0];
					}
				} 

				while (freedCount)
				{
					system->allEffectHandles[activeIndex++ & 0x3FF] = freedHandles[--freedCount]; // *((WORD*)effectHandle + freedCount-- + 1);
					FxEffect* effect = FX_EffectFromHandle(system, freedHandles[freedCount]);
					memset(effect, 0, sizeof(FxEffect));
				}
			}

			system->firstActiveEffect = activeIndex;
		}
	}
	
}