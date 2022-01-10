#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	extern FxSystem		fx_systemPool;

	FxSystem*		FX_GetSystem(int localClientNum);
	bool			FX_GetEffectStatus(FxEffect* effect);

	void			FX_BeginLooping(FxSystem* system, FxEffect* effect, int elemDefFirst, int elemDefCount, FxSpatialFrame* frameWhenPlayed, FxSpatialFrame* frameNow, int msecWhenPlayed, int msecNow);
	void			FX_StartNewEffect(FxSystem* system, FxEffect* effect);

	void			FX_StopEffectNonRecursive(FxEffect* effect, FxSystem* system);
	void			FX_StopEffect(FxSystem* system, FxEffect* effect);

	void			FX_TrailElem_CompressBasis(const float(*inBasis)[3], char(*outBasis)[3]);

	bool			FX_IsSpotLightEffect(FxEffectDef* def);
	bool			FX_CanAllocSpotLightEffect(FxSystem* system);
	std::int16_t	FX_CalculatePackedLighting(const float* origin);
	bool			FX_EffectAffectsGameplay(FxEffectDef* remoteEffectDef);
	void			FX_SetEffectRandomSeed(FxEffectDef* remoteDef, FxEffect* effect);


	FxPool_FxElem*  FX_AllocPool_Generic_FxElem_FxElemContainer_(volatile int* activeCount, volatile int* firstFreeIndex, FxPool_FxElem* pool);
	FxElem*			FX_AllocElem(FxSystem* system);

	FxPool_FxTrailElem* FX_AllocPool_Generic_FxTrailElem_FxTrailElem_(volatile int* activeCount, volatile int* trailElems, FxPool_FxTrailElem* pool);
	FxTrailElem*		FX_AllocTrailElem(FxSystem* system);

	FxPool_FxTrail* FX_AllocPool_Generic_FxTrail_FxTrail_(volatile int* activeTrailCount, volatile int* firstFreeIndex, FxPool_FxTrail* pool);
	FxTrail*		FX_AllocTrail(FxSystem* system);



	void			FX_SpawnRunner(FxSpatialFrame* effectFrameWhenPlayed, FxEffect* effect, FxElemDef* remoteElemDef, FxSystem* system, int randomSeed, int msecWhenPlayed);
	void			FX_SpawnElem(FxSystem* system, FxEffect* effect, int elemDefIndex, FxSpatialFrame* effectFrameWhenPlayed, int msecWhenPlayed, float distanceWhenPlayed, int sequence);
	void			FX_SpawnSpotLightElem(FxSystem* system, FxElem* elem);

	void			FX_SpawnTrailLoopingElems(FxTrail* trail, FxEffect* effect, FxSystem* system, FxSpatialFrame* frameBegin, FxSpatialFrame* frameEnd, int msecWhenPlayed, int msecUpdateBegin, int msecUpdateEnd, float distanceTravelledBegin, float distanceTravelledEnd);
	void			FX_SpawnTrailElem_Cull(FxSystem* system, FxEffect* effect, FxTrail* trail, FxSpatialFrame* effectFrameWhenPlayed, int msecWhenPlayed, float distanceWhenPlayed);
	void			FX_SpawnTrailElem_NoCull(FxSystem* system, FxEffect* effect, FxTrail* trail, FxSpatialFrame* effectFrameWhenPlayed, int msecWhenPlayed, float distanceWhenPlayed);
	bool			FX_CullTrailElem(FxElemDef* elemDef, char sequence, FxCamera* camera, const float* origin);

	void			FX_SpawnEffect_AllocSpotLightEffect(FxEffect* effect, FxSystem* system);
	void			FX_SpawnEffect_AllocTrails(FxSystem* system, FxEffect* effect);
	FxEffect*		FX_SpawnEffect(FxSystem* system, FxEffectDef* remoteDef, int msecBegin, const float* origin, const float(*axis)[3], int dobjHandle, int boneIndex, int runnerSortOrder, unsigned __int16 owner, unsigned int markEntnum);
	void			FX_SpawnDeathEffect(FxUpdateElem* update, FxSystem* system);
	bool			FX_CullEffectForSpawn(FxEffectDef* effectDef, FxCamera* camera, const float* origin);
	bool			FX_CullElemForSpawn(const float* origin, FxCamera* camera, FxElemDef* elemDef);

	std::uint16_t	FX_EffectToHandle(FxSystem* system, FxEffect* effect);
	FxEffect*		FX_EffectFromHandle(FxSystem* system, unsigned __int16 handle);

	std::uint16_t	FX_ElemToHandle(FxPool_FxElem* pool, FxElem* item_slim);
	FxElem*			FX_ElemFromHandle(FxSystem* system, unsigned __int16 handle);

	std::uint16_t	FX_TrailElemToHandle(FxPool_FxTrailElem* pool, FxTrailElem* item_slim);
	FxTrailElem*	FX_TrailElemFromHandle(FxSystem* system, unsigned __int16 handle);

	std::uint16_t	FX_TrailToHandle(FxPool_FxTrail* pool, FxTrail* item_slim);
	FxTrail*		FX_TrailFromHandle(FxSystem* system, unsigned __int16 handle);



	void			FX_AddRefToEffect(FxSystem* system, FxEffect* effect);
	void			FX_DelRefToEffect(FxSystem* system, FxEffect* effect);
	void			FX_EffectNoLongerReferenced(FxSystem* system, FxEffect* remoteEffect);

	void			FX_FreePool_Generic_FxElem_FxElemContainer_(volatile int* firstFreeIndex, FxElem* item_slim, FxPool_FxElem* pool);
	void			FX_FreePool_Generic_FxTrail_FxTrail_(FxTrail* item_slim, volatile int* firstFreeIndex, FxPool_FxTrail* pool);
	void			FX_FreePool_Generic_FxTrailElem_FxTrailElem_(FxTrailElem* item_slim, volatile int* firstFreeIndex, FxPool_FxTrailElem* pool);
	void			FX_FreeElem(FxSystem* system, unsigned __int16 elemHandle, FxEffect* effect, unsigned int elemClass);
	void			FX_FreeTrailElem(FxSystem* system, std::int16_t trailElemHandle, FxEffect* effect, FxTrail* trail);
	void			FX_FreeSpotLightElem(FxSystem* system, unsigned __int16 elemHandle, FxEffect* effect);

	void			FX_RemoveAllEffectElems(FxEffect* effect, FxSystem* system);
	void			FX_KillEffect(FxSystem* system, FxEffect* effect);

	void			FX_RunGarbageCollection_FreeTrails(FxSystem* system, FxEffect* effect);
	void			FX_RunGarbageCollection_FreeSpotLight(FxSystem* system, unsigned __int16 effectHandle);
	void			FX_RunGarbageCollectionAndPrioritySort(FxSystem* system);
}