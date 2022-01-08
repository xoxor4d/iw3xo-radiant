#pragma once
#include "fx_structs.hpp"

constexpr auto FX_EFFECT_HANDLE_SCALE = 4;
constexpr auto FX_EFFECT_LIMIT = 1024; // size 0x80

constexpr auto FX_ELEM_HANDLE_SCALE = 4;
constexpr auto FX_ELEM_LIMIT = 2048; // size 0x28

constexpr auto FX_TRAIL_HANDLE_SCALE = 4;
constexpr auto FX_TRAIL_LIMIT = 128; // size 0x8

constexpr auto FX_TRAILELEM_HANDLE_SCALE = 4;
constexpr auto FX_TRAILELEM_LIMIT = 2048; // size 0x20

namespace fx_system
{
	extern FxSystem		fx_systemPool;

	FxSystem*		FX_GetSystem(int localClientNum);
	bool			FX_GetEffectStatus(FxEffect* effect);

	void			FX_StopEffectNonRecursive(FxEffect* effect, FxSystem* system);
	void			FX_StopEffect(FxSystem* system, FxEffect* effect);

	FxEffect*		FX_SpawnEffect(FxSystem* system, FxEffectDef* remoteDef, int msecBegin, const float* origin, const float(*axis)[3], int dobjHandle, int boneIndex, int runnerSortOrder, unsigned __int16 owner, unsigned int markEntnum);
	void			FX_SpawnDeathEffect(FxUpdateElem* update, FxSystem* system);

	std::uint16_t	FX_EffectToHandle(FxSystem* system, FxEffect* effect);
	FxEffect*		FX_EffectFromHandle(FxSystem* system, unsigned __int16 handle);
	FxElem*			FX_ElemFromHandle(FxSystem* system, unsigned __int16 handle);
	FxTrailElem*	FX_TrailElemFromHandle(FxSystem* system, unsigned __int16 handle);
	FxTrail*		FX_TrailFromHandle(FxSystem* system, unsigned __int16 handle);

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