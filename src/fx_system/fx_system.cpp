#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace fx_system
{
	FxSystem fx_systemPool = {};
	FxSystemBuffers fx_systemBufferPool = {};

	bool ed_is_paused = false;
	bool ed_is_playing = false;
	bool ed_is_repeating = false;
	bool ed_is_filename_valid = false;
	bool ed_is_editor_effect_valid = false;

	fx_system::FxEffect* ed_active_effect = nullptr;
	fx_system::FxEditorEffectDef ed_editor_effect = {};

	float ed_timescale = 1.0f;
	float ed_looppause = 0.3f;

	int ed_playback_tick = 0;
	int ed_playback_tick_old = 0;
	int ed_repeat_tickcount = 0;

	// * --------------------------------

	FxSystem* FX_GetSystem([[maybe_unused]] int localClientNum)
	{
#ifdef FXEDITOR
		return &game::fx_systemPool;
#else
		return &fx_systemPool;
#endif
	}

	FxSystemBuffers* FX_GetSystemBuffers([[maybe_unused]] int localClientNum)
	{
#ifdef FXEDITOR
		return &game::fx_systemBufferPool;
#else
		return &fx_systemBufferPool;
#endif
	}

	bool FX_GetEffectStatus(FxEffect* effect)
	{
		if (!effect)
		{
			
			Assert();
		}

		return effect->status == 1;
	}

	FxElemDef* FX_GetEffectElemDef(const FxEffect* effect, int index)
	{
		return &effect->def->elemDefs[index];
	}

	void FX_GetTrailHandleList_Last(FxSystem* system, FxEffect* effect, unsigned __int16* outHandleList, int* outTrailCount)
	{
		FxTrail* trail = nullptr;
		int trailIndex = 0;

		for (std::uint16_t trailHandle = effect->firstTrailHandle; trailHandle != UINT16_MAX; trailHandle = trail->nextTrailHandle)
		{
			trail = FX_TrailFromHandle(system, trailHandle);
			if (trailIndex >= 2)
			{
				Assert();
			}

			outHandleList[trailIndex++] = trail->lastElemHandle;
		}

		*outTrailCount = trailIndex;
	}

	// checked :: utils::hook::detour(0x475A70, fx_system::FX_RetriggerEffect, HK_JUMP);
	void FX_RetriggerEffect(int localClientNum, FxEffect* effect, int msecBegin)
	{
		if ((effect->status & FX_STATUS_REF_COUNT_MASK) == 0)
		{
			Assert();
		}

		FxSystem* system = FX_GetSystem(localClientNum);
		++effect->status;

		if ((effect->status & FX_STATUS_HAS_PENDING_LOOP_ELEMS) != 0)
		{
			FX_SpawnAllFutureLooping(system, effect, 0, effect->def->elemDefCountLooping, &effect->framePrev, &effect->frameNow, effect->msecBegin, effect->msecLastUpdate);
			FX_StopEffect(system, effect);
		}

		std::uint16_t firstOldElemHandle[3];
		firstOldElemHandle[0] = effect->firstElemHandle[0];
		firstOldElemHandle[1] = effect->firstElemHandle[1];
		firstOldElemHandle[2] = effect->firstElemHandle[2];

		int trailCount = 0;
		std::uint16_t lastOldTrailElemHandle[8] = {};
		FX_GetTrailHandleList_Last(system, effect, lastOldTrailElemHandle, &trailCount);

		if (msecBegin > effect->msecLastUpdate)
		{
			std::uint16_t lastElemHandle[3];
			lastElemHandle[0] = UINT16_MAX; //-1;
			lastElemHandle[1] = UINT16_MAX;
			lastElemHandle[2] = UINT16_MAX;
			FX_UpdateEffectPartial(system, effect, effect->msecLastUpdate, msecBegin, 0.0f, 0.0f, firstOldElemHandle, lastElemHandle, nullptr, lastOldTrailElemHandle);
		}

		effect->distanceTraveled = 0.0f;
		effect->msecBegin = msecBegin;

		FX_BeginLooping(system, effect, 0, effect->def->elemDefCountLooping, &effect->frameNow, &effect->frameNow, msecBegin, msecBegin);
		FX_TriggerOneShot(system, effect, effect->def->elemDefCountLooping, effect->def->elemDefCountOneShot, &effect->frameNow, msecBegin);


		if (effect->def->msecLoopingLife != 0)
		{
			effect->status |= FX_STATUS_HAS_PENDING_LOOP_ELEMS;
		}

		if (msecBegin < effect->msecLastUpdate)
		{
			FX_UpdateEffectPartial(system, effect, effect->msecBegin, effect->msecLastUpdate, 0.0, 0.0, effect->firstElemHandle, firstOldElemHandle, lastOldTrailElemHandle, 0);
		}

		FX_SortNewElemsInEffect(system, effect);

		if (effect->def->msecLoopingLife == 0)
		{
			FX_DelRefToEffect(system, effect);
		}
	}

	// checked :: utils::hook::detour(0x487880, fx_system::FX_BeginLooping, HK_JUMP);
	void FX_BeginLooping(FxSystem* system, FxEffect* effect, int elemDefFirst, int elemDefCount, FxSpatialFrame* frameWhenPlayed, FxSpatialFrame* frameNow, int msecWhenPlayed, int msecNow)
	{
		if (!effect || !effect->def)
		{
			Assert();
		}

		const int elemDefStop = elemDefCount + elemDefFirst;
		for (int elemDefIndex = elemDefFirst; elemDefIndex != elemDefStop; ++elemDefIndex)
		{
			if (effect->def->elemDefs[elemDefIndex].elemType != FX_ELEM_TYPE_TRAIL)
			{
				FX_SpawnElem(system, effect, elemDefIndex, frameWhenPlayed, msecWhenPlayed, 0.0f, 0);
			}
		}

		FxTrail* trail = nullptr;

		for (std::uint16_t trailHandle = effect->firstTrailHandle; trailHandle != UINT16_MAX; trailHandle = trail->nextTrailHandle)
		{
			trail = FX_TrailFromHandle(system, trailHandle);
			int trailIndex = trail->defIndex;
			if (trailIndex >= elemDefFirst && trailIndex < elemDefStop)
			{
				FxElemDef* elemDef = &effect->def->elemDefs[trailIndex];
				if (elemDef->elemType != FX_ELEM_TYPE_TRAIL)
				{
					Assert();
				}

				FX_SpawnTrailElem_NoCull(system, effect, trail, frameWhenPlayed, msecWhenPlayed, 0.0f);
				if (msecNow <= msecWhenPlayed)
				{
					FX_SpawnTrailElem_NoCull(system, effect, trail, frameWhenPlayed, msecWhenPlayed, 0.0f);
				}
				else
				{
					FX_SpawnTrailLoopingElems(trail, effect, system, frameWhenPlayed, frameNow, msecWhenPlayed, msecWhenPlayed, msecNow, 0.0f, effect->distanceTraveled);
					FX_SpawnTrailElem_NoCull(system, effect, trail, frameNow, msecNow, 0.0f);
				}
			}
		}
	}

	// checked
	void FX_StartNewEffect(FxSystem* system, FxEffect* effect)
	{
		FxEffectDef* def = effect->def;
		if (!def)
		{
			Assert();
		}

		FX_BeginLooping(system, effect, 0, def->elemDefCountLooping, &effect->frameAtSpawn, &effect->frameNow, effect->msecBegin, system->msecNow);
		FX_TriggerOneShot(system, effect, def->elemDefCountLooping, def->elemDefCountOneShot, &effect->frameAtSpawn, effect->msecBegin);
		FX_SortNewElemsInEffect(system, effect);
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

	// checked
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

	// checked
	void FX_TrailElem_CompressBasis(const float(*inBasis)[3], char(*outBasis)[3])
	{
		for (int basisVecIter = 0; basisVecIter != 2; ++basisVecIter)
		{
			for (int dimIter = 0; dimIter != 3; ++dimIter)
			{
				int base = static_cast<int>(127.0f * (float)(*inBasis)[3 * basisVecIter + dimIter]);
				if (base >= -128)
				{
					if (base <= 127)
					{
						base = static_cast<int>(127.0f * (float)(*inBasis)[3 * basisVecIter + dimIter]);
					}
					else
					{
						base = 127;
					}
				}
				else
				{
					base = 128;
				}

				(*outBasis)[3 * basisVecIter + dimIter] = static_cast<char>(base);
			}
		}
	}

	bool FX_IsSpotLightEffect(FxEffectDef* def)
	{

		for (int elemDefIter = 0; elemDefIter != def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission; ++elemDefIter)
		{
			if (def->elemDefs[elemDefIter].elemType == FX_ELEM_TYPE_SPOT_LIGHT)
			{
				return true;
			}
		}

		return false;
	}

	bool FX_CanAllocSpotLightEffect(FxSystem* system)
	{
		return system->activeSpotLightEffectCount < 1;
	}

	std::int16_t FX_CalculatePackedLighting([[maybe_unused]] const float* origin)
	{
		// R_GetAverageLightingAtPoint

		return 255;
	}

	bool FX_EffectAffectsGameplay(FxEffectDef* remoteEffectDef)
	{
		if (!remoteEffectDef)
		{
			Assert();
		}

		unsigned int elemDefCount = remoteEffectDef->elemDefCountEmission + remoteEffectDef->elemDefCountOneShot + remoteEffectDef->elemDefCountLooping;
		bool result = false;

		if (elemDefCount)
		{
			FxElemDef* elemDefs = remoteEffectDef->elemDefs;

			for (unsigned int elemDefIndex = 0; elemDefIndex < elemDefCount; ++elemDefIndex)
			{
				FxElemDef* elemDef = &elemDefs[elemDefIndex];

				if ((elemDef->flags & FX_ELEM_BLOCK_SIGHT) != 0)
				{
					return true;
				}

				if (elemDef->effectOnDeath.handle && FX_EffectAffectsGameplay(elemDef->effectOnDeath.handle))
				{
					return true;
				}

				if (elemDef->effectOnImpact.handle && FX_EffectAffectsGameplay(elemDef->effectOnImpact.handle))
				{
					return true;
				}

				if (elemDef->effectEmitted.handle && FX_EffectAffectsGameplay(elemDef->effectEmitted.handle))
				{
					return true;
				}

				if (elemDef->elemType == FX_ELEM_TYPE_RUNNER)
				{
					if (elemDef->visualCount == 1)
					{
						if (FX_EffectAffectsGameplay(elemDef->visuals.instance.effectDef.handle))
						{
							return true;
						}
					}
					else
					{
						FxElemVisuals* visArray = elemDef->visuals.array;
						for (unsigned int visIndex = 0; visIndex < static_cast<unsigned int>(elemDef->visualCount); ++visIndex)
						{
							if (FX_EffectAffectsGameplay(visArray[visIndex].effectDef.handle))
							{
								result = true;
								break;
							}
						}
					}
				}
			}
		}

		return result;
	}

	void FX_SetEffectRandomSeed(FxEffectDef* remoteDef, FxEffect* effect)
	{
		if (FX_EffectAffectsGameplay(remoteDef))
		{
			// fast rand
			effect->randomSeed = static_cast<std::uint16_t>( (479 * ((214013 * effect->msecBegin + 2531011) >> 17)) >> 15 );
		}
		else
		{
			effect->randomSeed = static_cast<std::uint16_t>( 479 * rand() / 32768 );
		}
	}

	// *
	// ----------------------------

	// checked
	FxPool_FxElem* FX_AllocPool_Generic_FxElem_FxElemContainer_(volatile int* activeCount, volatile int* firstFreeIndex, FxPool_FxElem* pool)
	{
		volatile int itemIndex = *firstFreeIndex;
		if (*firstFreeIndex == -1)
		{
			return nullptr;
		}
		if (itemIndex < 0 || itemIndex >= FX_ELEM_LIMIT)
		{
			Assert();
		}

		FxPool_FxElem* elem = &pool[itemIndex];
		if (elem->___u0.nextFree != -1 && (elem->___u0.nextFree < 0 || elem->___u0.nextFree >= FX_ELEM_LIMIT))
		{
			Assert();
		}

		*firstFreeIndex = elem->___u0.nextFree;
		++ *activeCount;

		return elem;
	}

	// checked
	FxElem* FX_AllocElem(FxSystem* system)
	{
		return reinterpret_cast<FxElem*>(FX_AllocPool_Generic_FxElem_FxElemContainer_(&system->activeElemCount, &system->firstFreeElem, system->elems));
	}


	// *
	// ----------------------------

	// checked
	FxPool_FxTrailElem* FX_AllocPool_Generic_FxTrailElem_FxTrailElem_(volatile int* activeCount, volatile int* trailElems, FxPool_FxTrailElem* pool)
	{

		volatile int itemIndex = *trailElems;
		if (*trailElems == -1)
		{
			return nullptr;
		}

		if (itemIndex < 0 || itemIndex >= FX_TRAILELEM_LIMIT)
		{
			Assert();
		}

		FxPool_FxTrailElem* trailElem = &pool[itemIndex];
		const int nextFree = trailElem->___u0.nextFree;

		if (trailElem->___u0.nextFree != -1 && (nextFree < 0 || nextFree >= FX_TRAILELEM_LIMIT))
		{
			Assert();
		}

		*trailElems = trailElem->___u0.nextFree;
		++ *activeCount;

		return trailElem;
	}

	// checked
	FxTrailElem* FX_AllocTrailElem(FxSystem* system)
	{
		return reinterpret_cast<FxTrailElem*>(FX_AllocPool_Generic_FxTrailElem_FxTrailElem_(&system->activeTrailElemCount, &system->firstFreeTrailElem, system->trailElems));
	}


	// *
	// ----------------------------


	FxPool_FxTrail* FX_AllocPool_Generic_FxTrail_FxTrail_(volatile int* activeTrailCount, volatile int* firstFreeIndex, FxPool_FxTrail* pool)
	{
		volatile int itemIndex = *firstFreeIndex;
		if (*firstFreeIndex == -1)
		{
			return nullptr;
		}

		if (itemIndex < 0 || itemIndex >= FX_TRAIL_LIMIT)
		{
			Assert();
		}

		FxPool_FxTrail* trail = &pool[itemIndex];
		const int nextFree = trail->___u0.nextFree;

		if (trail->___u0.nextFree != -1 && (nextFree < 0 || nextFree >= FX_TRAIL_LIMIT))
		{
			Assert();
		}

		*firstFreeIndex = trail->___u0.nextFree;
		++ *activeTrailCount;

		return trail;
	}

	FxTrail* FX_AllocTrail(FxSystem* system)
	{
		return reinterpret_cast<FxTrail*>(FX_AllocPool_Generic_FxTrail_FxTrail_(&system->activeTrailCount, &system->firstFreeTrail, system->trails));
	}


	// *
	// ----------------------------

	void FX_SpawnRunner(FxSpatialFrame* effectFrameWhenPlayed, FxEffect* effect, FxElemDef* remoteElemDef, FxSystem* system, int randomSeed, int msecWhenPlayed)
	{
		FxEffect* spawnedEffect = nullptr;

		const float(*usedAxis)[3];
		float axis[3][3];
		float spawnOrigin[3];

		FX_GetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);
		FX_OffsetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);

		FxEffectDef* effectDef = FX_GetElemVisuals(remoteElemDef, randomSeed).effectDef.handle;
		UnitQuatToAxis(effectFrameWhenPlayed->quat, axis);

		if ((remoteElemDef->flags & FX_ELEM_RUNNER_USES_RAND_ROT) != 0)
		{
			float rotatedAxis[3][3] = {};
			FX_RandomlyRotateAxis(axis, randomSeed, rotatedAxis);
			usedAxis = rotatedAxis;
		}
		else
		{
			usedAxis = axis;
		}

		int sortOrder = 0;
		if (static_cast<std::uint8_t>(remoteElemDef->sortOrder) == 255)
		{
			sortOrder = 255;
		}
		else
		{
			sortOrder = static_cast<std::uint8_t>(remoteElemDef->sortOrder);
			if (!remoteElemDef->sortOrder)
			{
				sortOrder = 0;
			}
		}

		FxBoltAndSortOrder boltAndSortOrder = effect->boltAndSortOrder;
		if ((*(DWORD*)&boltAndSortOrder & 0xFFC000) == 0xFFC000)  
		{
			int v11 = *(WORD*)&boltAndSortOrder & 4095;
			if (v11 == 4095)
			{
				v11 = 2175;
			}

			spawnedEffect = FX_SpawnEffect(system, effectDef, msecWhenPlayed, spawnOrigin, usedAxis, 4095, 1023, sortOrder, effect->owner, v11);
			//spawnedEffect = utils::hook::call<FxEffect* (__cdecl)(FxSystem*, FxEffectDef*, int, const float*, const float(*)[3], int, int, int, unsigned __int16, unsigned int)>(0x4740E0)(system, effectDef, msecWhenPlayed, spawnOrigin, usedAxis, 4095, 1023, sortOrder, effect->owner, v11);
		}
		else
		{
			spawnedEffect = FX_SpawnEffect(system, effectDef, msecWhenPlayed, spawnOrigin, usedAxis, *(WORD*)&boltAndSortOrder & 4095, (*(unsigned int*)&boltAndSortOrder >> 14) & 1023, sortOrder, effect->owner, 2175u);
			//spawnedEffect = utils::hook::call<FxEffect* (__cdecl)(FxSystem*, FxEffectDef*, int, const float*, const float(*)[3], int, int, int, unsigned __int16, unsigned int)>(0x4740E0)(system, effectDef, msecWhenPlayed, spawnOrigin, usedAxis, *(WORD*)&boltAndSortOrder & 4095, (*(unsigned int*)&boltAndSortOrder >> 14) & 1023, sortOrder, effect->owner, 2175u);
		}

		if (spawnedEffect)
		{
			FX_DelRefToEffect(system, spawnedEffect);
		}
	}

	void FX_SpawnElem(FxSystem* system, FxEffect* effect, int elemDefIndex, FxSpatialFrame* effectFrameWhenPlayed, int msecWhenPlayed, float distanceWhenPlayed, int sequence)
	{
		if (!system || !effect || !effect->def)
		{
			Assert();
		}
		
		FxElemDef* elemDef = &effect->def->elemDefs[elemDefIndex];
		if (elemDef->elemType == FX_ELEM_TYPE_TRAIL)
		{
			Assert();
		}

		if (game::Dvar_FindVar("fx_cull_elem_spawn")->current.enabled)
		{
			if (FX_CullElemForSpawn(effectFrameWhenPlayed->origin, &system->cameraPrev, elemDef))
			{
				return;
			}
		}

		int msecBegin = msecWhenPlayed + elemDef->spawnDelayMsec.base;
		if (elemDef->spawnDelayMsec.amplitude)
		{
			//msecBegin += ((elemDef->spawnDelayMsec.amplitude + 1) * static_cast<std::uint16_t>(fx_randomTable[18 + FX_ElemRandomSeed(effect->randomSeed, msecBegin, sequence)])) >> 16;
			msecBegin += ((elemDef->spawnDelayMsec.amplitude + 1) * FX_RandomFloatAsUInt16(18, FX_ElemRandomSeed(effect->randomSeed, msecBegin, sequence))) >> 16;
		}

		const int randomSeed = FX_ElemRandomSeed(effect->randomSeed, msecBegin, sequence);
		if (elemDef->elemType == FX_ELEM_TYPE_RUNNER)
		{
			FX_SpawnRunner(effectFrameWhenPlayed, effect, elemDef, system, randomSeed, msecBegin);
		}
		else if (elemDef->elemType == FX_ELEM_TYPE_DECAL)
		{
			// #MARKS
			// FX_CreateImpactMarkInternal(system->localClientNum, (int)elemDef, (int)effectFrameWhenPlayed, (unsigned __int16)randomSeed);
		}
		else if (elemDef->elemType != FX_ELEM_TYPE_SOUND && 
			(elemDef->effectOnImpact.handle || elemDef->effectOnDeath.handle || elemDef->effectEmitted.handle ||
				msecBegin + FX_GetElemLifeSpanMsec(randomSeed, elemDef) > system->msecNow)) //msecBegin + elemDef->lifeSpanMsec.base + (((elemDef->lifeSpanMsec.amplitude + 1) * (unsigned __int16)fx_randomTable[17 + randomSeed]) >> 16) > system->msecNow))
		{
			FxElem* remoteElem = FX_AllocElem(system);
			if (remoteElem)
			{
				FX_AddRefToEffect(system, effect);
				remoteElem->defIndex = static_cast<char>(elemDefIndex);
				remoteElem->sequence = static_cast<char>(sequence);
				remoteElem->atRestFraction = -1;
				remoteElem->emitResidual = 0;
				remoteElem->msecBegin = msecBegin;

				// should be remoteElem->sequence? casting sequence "128" to char results in -128 tho .. can throw an assert here
				if (randomSeed != FX_ElemRandomSeed(effect->randomSeed, msecBegin, sequence))
				{
					Assert();
				}

				FX_GetOriginForElem(effect, remoteElem->___u8.origin, elemDef, effectFrameWhenPlayed, randomSeed);
				remoteElem->baseVel[0] = 0.0f;
				remoteElem->baseVel[1] = 0.0f;
				remoteElem->baseVel[2] = 0.0f;

				if (elemDef->elemType == FX_ELEM_TYPE_TRAIL)
				{
					remoteElem->u.trailTexCoord = distanceWhenPlayed / static_cast<float>(elemDef->trailDef->repeatDist);
				}

				remoteElem->prevElemHandleInEffect = UINT16_MAX; //-1;
				if (elemDef->elemType == FX_ELEM_TYPE_SPOT_LIGHT)
				{
					FX_SpawnSpotLightElem(system, remoteElem);
				}
				else
				{
					int elemClass = FX_ELEM_CLASS_SPRITE;
					if (elemDef->elemType > FX_ELEM_TYPE_TRAIL)
					{
						if (elemDef->elemType == FX_ELEM_TYPE_CLOUD)
						{
							elemClass = FX_ELEM_CLASS_CLOUD;
						}
						else
						{
							elemClass = FX_ELEM_CLASS_NONSPRITE;
						}
					}

					remoteElem->nextElemHandleInEffect = effect->firstElemHandle[elemClass];
					effect->firstElemHandle[elemClass] = FX_ElemToHandle(system->elems, remoteElem);

					if (remoteElem->nextElemHandleInEffect != 0xFFFF)
					{
						FX_ElemFromHandle(system, remoteElem->nextElemHandleInEffect)->prevElemHandleInEffect = effect->firstElemHandle[elemClass];
					}

					if (elemDef->elemType == FX_ELEM_TYPE_MODEL)
					{
						remoteElem->u.lightingHandle = 0;

						// #PHYS
						//if ((elemDef->flags & FX_ELEM_USE_MODEL_PHYSICS) != 0 && !FX_SpawnModelPhysics(elemDef, effect, randomSeed, remoteElem))
						if ((elemDef->flags & FX_ELEM_USE_MODEL_PHYSICS) != 0)
						{
							// remove physics flag (does not alter the fx project file)
							elemDef->flags &= ~FX_ELEM_USE_MODEL_PHYSICS;

							FX_FreeElem(system, FX_ElemToHandle(system->elems, remoteElem), effect, elemClass);
						}
					}
				}
			}
			else
			{
				//R_WarnOncePerFrame(33);
			}
		}
	}

	void FX_SpawnSpotLightElem(FxSystem* system, FxElem* elem)
	{
		if (system->activeSpotLightEffectCount != 0)
		{
			Assert();
		}
		
		++system->activeSpotLightElemCount;
		system->activeSpotLightElemHandle = FX_ElemToHandle(system->elems, elem);
	}

	void FX_SpawnTrailLoopingElems(FxTrail* trail, FxEffect* effect, FxSystem* system, FxSpatialFrame* frameBegin, FxSpatialFrame* frameEnd, int msecWhenPlayed, int msecUpdateBegin, int msecUpdateEnd, float distanceTravelledBegin, float distanceTravelledEnd)
	{
		if (!effect)
		{
			Assert();
		}

		FxEffectDef* effectDef = effect->def;
		if (!effectDef)
		{
			Assert();
		}

		if (trail->defIndex >= static_cast<std::uint8_t>(effectDef->elemDefCountEmission + effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping))
		{
			Assert();
		}

		if (trail->defIndex >= effectDef->elemDefCountLooping && trail->defIndex < effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping)
		{
			Assert();
		}

		if ((msecWhenPlayed > msecUpdateBegin || msecUpdateBegin > msecUpdateEnd))
		{
			Assert();
		}

		FxElemDef* elemDef = &effect->def->elemDefs[trail->defIndex];
		if (elemDef->elemType != FX_ELEM_TYPE_TRAIL || !elemDef->trailDef)
		{
			Assert();
		}

		FxSpatialFrame frameWhenPlayed = {};

		const float normalizedTotalDistance = (distanceTravelledEnd - distanceTravelledBegin) / static_cast<float>(elemDef->trailDef->splitDist);
		float normalizedDistanceBeforeSpawn = 1.0f - (distanceTravelledBegin / static_cast<float>(elemDef->trailDef->splitDist) - floorf(distanceTravelledBegin / static_cast<float>(elemDef->trailDef->splitDist)));
		float normalizedDistanceTraversed = 0.0f;
		float normalizedDistanceRemaining = normalizedTotalDistance;

		while (normalizedDistanceRemaining > normalizedDistanceBeforeSpawn)
		{
			normalizedDistanceTraversed = normalizedDistanceTraversed + normalizedDistanceBeforeSpawn;
			const float lerp = normalizedDistanceTraversed / normalizedTotalDistance;
			FX_FrameLerp(frameBegin, frameEnd, &frameWhenPlayed, lerp);

			const int msecSpawn = msecUpdateBegin + static_cast<int>(((float)(msecUpdateEnd - msecUpdateBegin) * (normalizedDistanceTraversed / normalizedTotalDistance)));
			const float distSpawn = (distanceTravelledEnd - distanceTravelledBegin) * (normalizedDistanceTraversed / normalizedTotalDistance) + distanceTravelledBegin;

			FX_SpawnTrailElem_Cull(system, effect, trail, &frameWhenPlayed, msecSpawn, distSpawn);

			normalizedDistanceRemaining = normalizedDistanceRemaining - normalizedDistanceBeforeSpawn;
			normalizedDistanceBeforeSpawn = 1.0f;
		}
	}

	void FX_SpawnTrailElem_Cull(FxSystem* system, FxEffect* effect, FxTrail* trail, FxSpatialFrame* effectFrameWhenPlayed, int msecWhenPlayed, float distanceWhenPlayed)
	{
		if (!system || !effect || !effect->def || !trail)
		{
			Assert();
		}
		
		FxElemDef* elemDef = &effect->def->elemDefs[trail->defIndex];
		if (elemDef->elemType != FX_ELEM_TYPE_TRAIL)
		{
			Assert();
		}

		if (FX_CullTrailElem(elemDef, trail->sequence, &system->cameraPrev, effectFrameWhenPlayed->origin))
		{
			trail->sequence = trail->sequence + 1;
		}
		else
		{
			FX_SpawnTrailElem_NoCull(system, effect, trail, effectFrameWhenPlayed, msecWhenPlayed, distanceWhenPlayed);
		}
	}

	// checked :: utils::hook::detour(0x474940, fx_system::FX_SpawnTrailElem_NoCull, HK_JUMP);
	void FX_SpawnTrailElem_NoCull(FxSystem* system, FxEffect* effect, FxTrail* trail, FxSpatialFrame* effectFrameWhenPlayed, int msecWhenPlayed, float distanceWhenPlayed)
	{
		if (!system || !effect || !effect->def || !trail)
		{
			Assert();
		}

		FxElemDef* elemDef = &effect->def->elemDefs[trail->defIndex];

		if (elemDef->elemType != FX_ELEM_TYPE_TRAIL)
		{
			Assert();
		}

		int msecBegin = msecWhenPlayed + elemDef->spawnDelayMsec.base;

		if (elemDef->spawnDelayMsec.amplitude)
		{
			msecBegin += ((elemDef->spawnDelayMsec.amplitude + 1) * static_cast<std::uint16_t>(fx_randomTable[18 + FX_ElemRandomSeed(effect->randomSeed, msecBegin, trail->sequence)])) >> 16;
		}

		const unsigned int randomSeed = FX_ElemRandomSeed(effect->randomSeed, msecBegin, trail->sequence);

		if (elemDef->effectOnImpact.handle || elemDef->effectOnDeath.handle || elemDef->effectEmitted.handle || 
			msecBegin + elemDef->lifeSpanMsec.base + (((elemDef->lifeSpanMsec.amplitude + 1) * static_cast<std::uint16_t>(fx_randomTable[17 + ((msecBegin + 296 * trail->sequence + (unsigned int)effect->randomSeed) % 479)])) >> 16) > system->msecNow)
		{
			FxTrailElem* remoteTrailElem = FX_AllocTrailElem(system);
			if (remoteTrailElem)
			{
				float basis[2][3] = {};

				++effect->status;

				FX_GetOriginForTrailElem(effect, elemDef, effectFrameWhenPlayed, randomSeed, remoteTrailElem->origin, (float*)basis, basis[1]);
				//utils::hook::call<void(__cdecl)(FxEffect*, FxElemDef*, FxSpatialFrame*, int, float*, float*, float*)>(0x489DE0)(effect, elemDef, effectFrameWhenPlayed, randomSeed, remoteTrailElem->origin, (float*)basis, basis[1]);

				const std::uint16_t trailElemHandle = FX_TrailElemToHandle(system->trailElems, remoteTrailElem);
				if (trail->lastElemHandle == FX_HANDLE_NONE)
				{
					if (trail->firstElemHandle != FX_HANDLE_NONE)
					{
						Assert();
					}

					trail->firstElemHandle = trailElemHandle;
				}
				else
				{
					FX_TrailElemFromHandle(system, trail->lastElemHandle)->nextTrailElemHandle = trailElemHandle;
				}

				trail->lastElemHandle = trailElemHandle;
				remoteTrailElem->nextTrailElemHandle = UINT16_MAX; //-1;
				remoteTrailElem->spawnDist = distanceWhenPlayed;
				remoteTrailElem->msecBegin = msecBegin;
				remoteTrailElem->baseVelZ = 0;
				remoteTrailElem->sequence = trail->sequence++;

				FX_TrailElem_CompressBasis(basis, remoteTrailElem->basis);
			}
		}
	}

	bool FX_CullTrailElem(FxElemDef* elemDef, char sequence, FxCamera* camera, const float* origin)
	{
		const float baseCutoffDist = elemDef->spawnRange.base + elemDef->spawnRange.amplitude;
		if (baseCutoffDist == 0.0f)
		{
			return false;
		}

		if (!sequence)
		{
			return false;
		}

		float cutoffMultiple = 1.0f;
		while ((sequence & 1) == 0)
		{
			cutoffMultiple = cutoffMultiple + 1.0f;
			sequence = static_cast<std::uint8_t>(sequence) >> 1;
		}

		const float distSq = Vec3DistanceSq(origin, camera->origin);
		if (distSq <= ((baseCutoffDist * cutoffMultiple) * (baseCutoffDist * cutoffMultiple)))
		{
			return false;
		}

		return true;
	}

	void FX_SpawnEffect_AllocSpotLightEffect(FxEffect* effect, FxSystem* system)
	{
		FxEffectDef* def = effect->def;
		if (!def)
		{
			Assert();
		}

		const int elemDefCount = def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
		for (int elemDefIter = 0; elemDefIter != elemDefCount; ++elemDefIter)
		{
			if (effect->def->elemDefs[elemDefIter].elemType == FX_ELEM_TYPE_SPOT_LIGHT)
			{
				if (system->activeSpotLightEffectCount >= FX_SPOT_LIGHT_LIMIT)
				{
					Assert();
				}

				++system->activeSpotLightEffectCount;
				system->activeSpotLightEffectHandle = FX_EffectToHandle(system, effect);
			}
		}
	}

	// checked
	void FX_SpawnEffect_AllocTrails(FxSystem* system, FxEffect* effect)
	{
		const FxEffectDef* def = effect->def;
		if (!def)
		{
			Assert();
		}

		FxTrail localTrail = {};

		const int elemDefCount = def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
		for (int elemDefIter = 0; elemDefIter != elemDefCount; ++elemDefIter)
		{
			if (effect->def->elemDefs[elemDefIter].elemType == FX_ELEM_TYPE_TRAIL)
			{
				FxTrail* remoteTrail = FX_AllocTrail(system);
				if (!remoteTrail)
				{
					return;
				}

				localTrail.nextTrailHandle = effect->firstTrailHandle;
				localTrail.defIndex = static_cast<char>(elemDefIter);

				//*(DWORD*)&localTrail.firstElemHandle = -1; // < sets first and lastElemHandle to 0xFFFF
				localTrail.firstElemHandle = UINT16_MAX;
				localTrail.lastElemHandle = UINT16_MAX;
				localTrail.sequence = 0;

				effect->firstTrailHandle = FX_TrailToHandle(system->trails, remoteTrail);
				*remoteTrail = localTrail;
			}
		}
	}
	
	// todo: clean me up (has issues)
	FxEffect* FX_SpawnEffect(FxSystem* system, FxEffectDef* remoteDef, int msecBegin, const float* origin, const float(*axis)[3], int dobjHandle, int boneIndex, int runnerSortOrder, unsigned __int16 owner, unsigned int markEntnum)
	{
		if (!system || system->isArchiving || !remoteDef || !origin || !axis)
		{
			Assert();
		}

		if (game::Dvar_FindVar("fx_cull_effect_spawn")->current.enabled && FX_CullEffectForSpawn(remoteDef, &system->cameraPrev, origin))
		{
			return nullptr;
		}

		const bool isSpotLightEffect = FX_IsSpotLightEffect(remoteDef);
		if (isSpotLightEffect && !FX_CanAllocSpotLightEffect(system))
		{
			//R_WarnOncePerFrame(32);
			return nullptr;
		}

		if (system->firstFreeEffect - system->firstActiveEffect == FX_EFFECT_LIMIT)
		{
			return nullptr;
		}

		auto old_firstFree = system->firstFreeEffect;
		system->firstFreeEffect = old_firstFree + 1;

		FxEffect* remoteEffect = FX_EffectFromHandle(system, system->allEffectHandles[old_firstFree & FX_EFFECT_HANDLE_NONE]);
		remoteEffect->def = remoteDef;
		remoteEffect->status = remoteDef->msecLoopingLife != 0 ? 65538 : 1;
		remoteEffect->firstElemHandle[0] = UINT16_MAX; //-1;
		remoteEffect->firstElemHandle[1] = UINT16_MAX; //-1;
		remoteEffect->firstElemHandle[2] = UINT16_MAX; //-1;
		remoteEffect->firstSortedElemHandle = UINT16_MAX; //-1;

		if ((remoteDef->flags & 1) != 0)
		{
			remoteEffect->packedLighting = FX_CalculatePackedLighting(origin);
		}
		else
		{
			remoteEffect->packedLighting = 255;
		}

		remoteEffect->msecBegin = msecBegin;
		remoteEffect->distanceTraveled = 0.0f;
		remoteEffect->msecLastUpdate = msecBegin;

		FX_SetEffectRandomSeed(remoteDef, remoteEffect);
		remoteEffect->firstTrailHandle = UINT16_MAX; // -1;
		FX_SpawnEffect_AllocTrails(system, remoteEffect);

		if (isSpotLightEffect)
		{
			FX_SpawnEffect_AllocSpotLightEffect(remoteEffect, system);
		}

		if ((remoteEffect->status & FX_STATUS_OWNED_EFFECTS_MASK) != 0)
		{
			Assert();
		}

		if (owner == FX_HANDLE_NONE)
		{
			remoteEffect->owner = system->allEffectHandles[old_firstFree & FX_EFFECT_HANDLE_NONE];
			remoteEffect->status |= FX_STATUS_SELF_OWNED;
		}
		else
		{
			remoteEffect->owner = owner;
			FxEffect* ownerEffect = FX_EffectFromHandle(system, owner);
			if (!ownerEffect)
			{
				Assert();
			}

			volatile int oldStatusValue = ++ownerEffect->status;
			ownerEffect->status = oldStatusValue + (1 << FX_STATUS_OWNED_EFFECTS_SHIFT); //131072

			//if ((oldStatusValue & 0xF801FFFF) != ((oldStatusValue + 0x20000) & 0xF801FFFF))
			if ((oldStatusValue & ~FX_STATUS_OWNED_EFFECTS_MASK) != ((oldStatusValue + (1 << FX_STATUS_OWNED_EFFECTS_SHIFT)) & ~FX_STATUS_OWNED_EFFECTS_MASK))
			{
				Assert();
			}

			if ((ownerEffect->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0)
			{
				Assert();
			}
		}

		if (dobjHandle < 0)
		{
			Assert();
		}

		*(DWORD*)&remoteEffect->boltAndSortOrder ^= (*(DWORD*)&remoteEffect->boltAndSortOrder ^ (boneIndex << 14)) & 0xFFC000;
		int v17 = (*(DWORD*)&remoteEffect->boltAndSortOrder >> 14) & FX_SPAWN_BONE_INDEX;

		// effect->boltAndSortOrder.boneIndex == static_cast<uint>( boneIndex )
		if (v17 != boneIndex)
		{
			Assert();
		}

		*(DWORD*)&remoteEffect->boltAndSortOrder &= 0xFFFFCFFF;
		if (markEntnum == FX_SPAWN_MARK_ENTNUM)
		{
			if (boneIndex == FX_SPAWN_BONE_INDEX)
			{
				if (dobjHandle != FX_SPAWN_DOBJ_HANDLES)
				{
					Assert();
				}
			}
			else
			{
				if (boneIndex < 0)
				{
					Assert();
				}
			}

			// effect->boltAndSortOrder.dobjHandle == static_cast<uint>( dobjHandle )
			*(DWORD*)&remoteEffect->boltAndSortOrder ^= ((unsigned __int16)dobjHandle ^ (unsigned __int16)*(DWORD*)&remoteEffect->boltAndSortOrder) & FX_SPAWN_DOBJ_HANDLES;
			int v19 = *(DWORD*)&remoteEffect->boltAndSortOrder & FX_SPAWN_DOBJ_HANDLES;
			if (v19 != dobjHandle)
			{
				Assert();
			}
		}
		else
		{
			if (boneIndex != FX_SPAWN_BONE_INDEX) // FX_BONE_INDEX_NONE
			{
				Assert();
			}
			if (dobjHandle != FX_SPAWN_DOBJ_HANDLES) // FX_DOBJ_HANDLE_NONE
			{
				Assert();
			}
			if (markEntnum >= FX_SPAWN_DOBJ_HANDLES) // markEntnum doesn't index FX_DOBJ_HANDLE_NONE
			{
				Assert();
			}

			// (effect->boltAndSortOrder.dobjHandle == markEntnum)
			*(DWORD*)&remoteEffect->boltAndSortOrder ^= ((unsigned __int16)markEntnum ^ (unsigned __int16)*(DWORD*)&remoteEffect->boltAndSortOrder) & FX_SPAWN_DOBJ_HANDLES;
			if ((*(DWORD*)&remoteEffect->boltAndSortOrder & FX_SPAWN_DOBJ_HANDLES) != markEntnum)
			{
				Assert();
			}
		}

		// effect->boltAndSortOrder.sortOrder == static_cast<uint>( runnerSortOrder )
		*((std::uint8_t*)&remoteEffect->boltAndSortOrder + 3) = static_cast<std::uint8_t>(runnerSortOrder);
		if (remoteEffect->boltAndSortOrder.sortOrder != static_cast<std::uint16_t>(runnerSortOrder))
		{
			Assert();
		}

		remoteEffect->frameAtSpawn.origin[0] = origin[0];
		remoteEffect->frameAtSpawn.origin[1] = origin[1];
		remoteEffect->frameAtSpawn.origin[2] = origin[2];

		AxisToQuat(axis, remoteEffect->frameAtSpawn.quat);
		memcpy(&remoteEffect->framePrev, &remoteEffect->frameAtSpawn, sizeof(remoteEffect->framePrev));
		memcpy(&remoteEffect->frameNow, &remoteEffect->frameAtSpawn, sizeof(remoteEffect->frameNow));

		system->firstNewEffect = system->firstFreeEffect;

		FX_StartNewEffect(system, remoteEffect); //utils::hook::call<void(__cdecl)(FxSystem*, FxEffect*)>(0x487A80)(system, remoteEffect);

		return remoteEffect;
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

	bool FX_CullEffectForSpawn(FxEffectDef* effectDef, FxCamera* camera, const float* origin)
	{
		if (effectDef->elemDefs->spawnRange.amplitude != 0.0f)
		{
			const float dist2 = Vec3DistanceSq(origin, camera->origin);
			if ((effectDef->elemDefs->spawnRange.base * effectDef->elemDefs->spawnRange.base) > dist2)
			{
				return true;
			}

			const float dist1 = effectDef->elemDefs->spawnRange.amplitude + effectDef->elemDefs->spawnRange.base;
			if (dist2 > (dist1 * dist1))
			{
				return true;
			}
		}
		return (effectDef->elemDefs->flags & FX_ELEM_SPAWN_FRUSTUM_CULL) != 0 && FX_CullSphere(camera, camera->frustumPlaneCount, origin, effectDef->elemDefs->spawnFrustumCullRadius);
	}

	bool FX_CullElemForSpawn(const float* origin, FxCamera* camera, FxElemDef* elemDef)
	{
		if (elemDef->spawnRange.amplitude != 0.0f)
		{
			const float dist = Vec3DistanceSq(origin, camera->origin);
			if ((elemDef->spawnRange.base * elemDef->spawnRange.base) > dist)
			{
				return true;
			}

			const float range = elemDef->spawnRange.amplitude + elemDef->spawnRange.base;
			if (dist > (range * range))
			{
				return true;
			}
		}

		return (elemDef->flags & FX_ELEM_SPAWN_FRUSTUM_CULL) != 0 && FX_CullSphere(camera, camera->frustumPlaneCount, origin, elemDef->spawnFrustumCullRadius);
	}

	// *
	// ----------------------------

	// checked
	std::uint16_t FX_EffectToHandle(FxSystem* system, FxEffect* effect)
	{
		if (!system)
		{
			Assert();
		}

		if (!effect || (effect < &system->effects[0]) || effect >= &system->effects[FX_EFFECT_LIMIT])
		{
			Assert();
		}

		return static_cast<std::uint16_t>(((char*)effect - (char*)system->effects) / 4);
	}

	// checked
	FxEffect* FX_EffectFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		if (!system)
		{
			Assert();
		}

		// if ( handle >= 32768u || (handle & 31) != 0 )
		// handle < FX_EFFECT_LIMIT * sizeof( FxEffect ) / FxEffect::HANDLE_SCALE && handle % (sizeof( FxEffect ) / FxEffect::HANDLE_SCALE) == 0

		if(handle >= (FX_EFFECT_LIMIT * sizeof(FxEffect) / FX_EFFECT_HANDLE_SCALE) || handle % (sizeof(FxEffect) / FX_EFFECT_HANDLE_SCALE) != 0)
		{
			Assert();
		}

		return reinterpret_cast<FxEffect*>(reinterpret_cast<char*>(system->effects) + 4 * handle);
	}

	// *
	// ----------------------------

	// checked :: utils::hook::detour(0x473300, fx_system::FX_ElemToHandle, HK_JUMP);
	std::uint16_t FX_ElemToHandle(FxPool_FxElem* pool, FxElem* item_slim)
	{
		// item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item
		if (!item_slim || item_slim < (FxElem*)pool || item_slim >= (FxElem*)&pool[FX_ELEM_LIMIT])
		{
			Assert();
		}

		return static_cast<std::uint16_t>(((char*)item_slim - (char*)pool) / 4);
	}

	// checked :: utils::hook::detour(0x473910, fx_system::FX_ElemFromHandle, HK_JUMP);
	FxElem* FX_ElemFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		// if ( handle < 20480u && !(handle % 10u) )
		// "handle < LIMIT * sizeof( ITEM_TYPE ) / ITEM_TYPE::HANDLE_SCALE && handle % (sizeof( ITEM_TYPE ) / ITEM_TYPE::HANDLE_SCALE) == 0

		if (handle >= (FX_ELEM_LIMIT * sizeof(FxElem) / FX_ELEM_HANDLE_SCALE) || handle % (sizeof(FxElem) / FX_ELEM_HANDLE_SCALE) != 0)
		{
			Assert();
		}

		return reinterpret_cast<FxElem*>(reinterpret_cast<char*>(system->elems) + 4 * handle);
	}

	// *
	// ----------------------------

	// checked
	std::uint16_t FX_TrailElemToHandle(FxPool_FxTrailElem* pool, FxTrailElem* item_slim)
	{
		// item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item
		if (!item_slim || item_slim < (FxTrailElem*)pool || item_slim >= (FxTrailElem*)&pool[FX_TRAILELEM_LIMIT])
		{
			Assert();
		}

		return static_cast<std::uint16_t>(((char*)item_slim - (char*)pool) / 4);
	}

	// checked
	FxTrailElem* FX_TrailElemFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		// if ( handle >= 16384u || (handle & 7) != 0 )
		if (handle >= (FX_TRAILELEM_LIMIT * sizeof(FxTrailElem) / FX_TRAILELEM_HANDLE_SCALE) || handle % (sizeof(FxTrailElem) / FX_TRAILELEM_HANDLE_SCALE) != 0)
		{
			Assert();
		}

		return reinterpret_cast<FxTrailElem*>(reinterpret_cast<char*>(system->trailElems) + 4 * handle);
	}

	// *
	// ----------------------------

	std::uint16_t FX_TrailToHandle(FxPool_FxTrail* pool, FxTrail* item_slim)
	{
		// item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item
		if (!item_slim || item_slim < (FxTrail*)pool || item_slim >= (FxTrail*)&pool[FX_TRAIL_LIMIT])
		{
			Assert();
		}

		return static_cast<std::uint16_t>(((char*)item_slim - (char*)pool) / 4);
	}

	// checked
	FxTrail* FX_TrailFromHandle(FxSystem* system, unsigned __int16 handle)
	{
		// if ( trailHandle >= 256u || (trailHandle & 1) != 0 )
		if (handle >= (FX_TRAIL_LIMIT * sizeof(FxTrail) / FX_TRAIL_HANDLE_SCALE) || handle % (sizeof(FxTrail) / FX_TRAIL_HANDLE_SCALE) != 0)
		{
			Assert();
		}

		return reinterpret_cast<FxTrail*>(reinterpret_cast<char*>(system->trails) + 4 * handle);
	}

	// *
	// ----------------------------

	// checked
	void FX_AddRefToEffect([[maybe_unused]] FxSystem* system, FxEffect* effect)
	{
		if (!effect)
		{
			Assert();
		}

		++effect->status;
	}

	// checked
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

	// checked :: utils::hook::detour(0x4728A0, fx_system::FX_EffectNoLongerReferenced, HK_JUMP);
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

	// checked
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

		*(DWORD*)&item_slim->nextTrailHandle = *firstFreeIndex;
		*firstFreeIndex = freedIndex;
	}

	// checked
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

	// checked :: utils::hook::detour(0x474D60, fx_system::FX_FreeElem, HK_JUMP);
	void FX_FreeElem(FxSystem* system, unsigned __int16 elemHandle, FxEffect* effect, unsigned int elemClass)
	{
		if (!system)
		{
			Assert();
		}

		// custom assert
		// effect->def = null on live effect edit (eg. velocity inc) when KillEffect detoured
		if(!effect->def)
		{
			Assert();
		}

		FxElem* remoteElem = FX_ElemFromHandle(system, elemHandle);

		if (!elemClass && effect->firstSortedElemHandle == elemHandle)
		{
			effect->firstSortedElemHandle = remoteElem->nextElemHandleInEffect;
		}

		
		const std::uint16_t nextElemHandle = remoteElem->nextElemHandleInEffect;

		if (nextElemHandle != 0xFFFF)
		{
			FX_ElemFromHandle(system, nextElemHandle)->prevElemHandleInEffect = remoteElem->prevElemHandleInEffect;
		}

		const std::uint16_t prevElemHandle = remoteElem->prevElemHandleInEffect;

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
		if (elemDef->elemType == FX_ELEM_TYPE_MODEL && (elemDef->flags & FX_ELEM_USE_MODEL_PHYSICS) != 0)
		{
			if (remoteElem->___u8.physObjId)
			{
				// #PHYS
				// Phys_ObjDestroy(1, (DWORD*)remoteElem->___u8.physObjId);
			}
		}

		memset(&remoteElem->defIndex, 0, sizeof(FxElem));

		FX_FreePool_Generic_FxElem_FxElemContainer_(&system->firstFreeElem, remoteElem, system->elems);
		FX_DelRefToEffect(system, effect);

		--system->activeElemCount;
	}

	// should be good
	void FX_FreeTrailElem(FxSystem* system, std::int16_t trailElemHandle, FxEffect* effect, FxTrail* trail)
	{
		FxTrailElem* remoteTrailElem;

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

			trail->lastElemHandle = UINT16_MAX; // -1
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


	// checked
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

	// checked :: utils::hook::detour(0x4753C0, fx_system::FX_KillEffect, HK_JUMP);
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
				FX_RunGarbageCollectionAndPrioritySort(system); //utils::hook::call<void(__cdecl)(FxSystem*)>(0x473E50)(system);
			}

			o_effect = effect;
		}

		FX_DelRefToEffect(system, o_effect);
	}


	// checked
	void FX_RunGarbageCollection_FreeTrails(FxSystem* system, FxEffect* effect)
	{
		for (; effect->firstTrailHandle != (unsigned __int16)FX_STATUS_REF_COUNT_MASK; --system->activeTrailCount)
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

	// checked :: utils::hook::detour(0x473E50, fx_system::FX_RunGarbageCollectionAndPrioritySort, HK_JUMP);
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

					if (((unsigned __int16)effect->status & FX_STATUS_REF_COUNT_MASK) != 0)
					{
						system->allEffectHandles[((WORD)freedCount + (WORD)activeIndex) & 0x3FF] = effectHandle;
					}
					else
					{
						FX_RunGarbageCollection_FreeTrails(system, effect);
						FX_RunGarbageCollection_FreeSpotLight(system, effectHandle);
						freedHandles[freedCount++] = effectHandle;
					}
				} 

				while (freedCount)
				{
					system->allEffectHandles[activeIndex++ & 0x3FF] = freedHandles[--freedCount];
					FxEffect* effect = FX_EffectFromHandle(system, freedHandles[freedCount]);
					memset(effect, 0, sizeof(FxEffect));
				}
			}

			system->firstActiveEffect = activeIndex;
		}
	}

	void FX_LinkSystemBuffers(FxSystem* system, FxSystemBuffers* systemBuffers)
	{
		system->elems = systemBuffers->elems;
		system->trails = systemBuffers->trails;
		system->trailElems = systemBuffers->trailElems;
		system->effects = systemBuffers->effects;
		system->visState = systemBuffers->visState;
		system->deferredElems = systemBuffers->deferredElems;
	}

	void FX_ResetSystem(FxSystem* system)
	{
		system->effects->def = nullptr;

		for (int effectIndex = 0; effectIndex < 1024; ++effectIndex)
		{
			system->allEffectHandles[effectIndex] = FX_EffectToHandle(system, &system->effects[effectIndex]);
		}

		system->firstActiveEffect = 0;
		system->firstNewEffect = 0;
		system->firstFreeEffect = 0;
		system->iteratorCount = 0;
		system->deferredElemCount = 0;
		system->firstFreeElem = 0;

		int i;
		for (i = 0; i < 2047; ++i)
		{
			system->elems[i].___u0.nextFree = i + 1;
		}

		system->elems[i].___u0.nextFree = -1;
		system->activeElemCount = 0;
		system->firstFreeTrailElem = 0;


		int j;
		for (j = 0; j < 2047; ++j)
		{
			system->trailElems[j].___u0.nextFree = j + 1;
		}

		system->trailElems[j].___u0.nextFree = -1;
		system->activeTrailElemCount = 0;
		system->firstFreeTrail = 0;

		int k;
		for (k = 0; k < 127; ++k)
		{
			system->trails[k].___u0.nextFree = k + 1;
		}

		system->trails[k].___u0.nextFree = -1;
		system->activeTrailCount = 0;
		system->activeSpotLightEffectCount = 0;
		system->activeSpotLightElemCount = 0;
		system->gfxCloudCount = 0;
		system->visState->blockerCount = 0;
		system->visStateBufferRead = system->visState;
		system->visStateBufferWrite = system->visState + 1;
	}

	void FX_InitSystem(int localClientNum)
	{
		FxSystem* system = FX_GetSystem(localClientNum);
		if (!system)
		{
			Assert();
		}
		memset(system, 0, sizeof(FxSystem));

		FxSystemBuffers* systemBuffers = FX_GetSystemBuffers(localClientNum);
		if (!systemBuffers)
		{
			Assert();
		}
		memset(systemBuffers, 0, sizeof(FxSystemBuffers));

		FX_LinkSystemBuffers(system, systemBuffers);
		//FX_RegisterDvars();
		FX_ResetSystem(system);

		system->msecNow = 0;
		system->msecDraw = -1;
		system->cameraPrev.isValid = 1;
		system->cameraPrev.frustumPlaneCount = 0;
		system->frameCount = 1;

		if (system->firstActiveEffect || system->firstNewEffect || system->firstFreeEffect)
		{
			Assert();
		}

		//FX_InitMarksSystem((FxMarksSystem*)&markSystem);
		system->localClientNum = static_cast<unsigned char>(localClientNum);
		system->isInitialized = true;
	}
	
}