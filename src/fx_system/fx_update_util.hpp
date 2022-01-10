#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	int		FX_GetElemLifeSpanMsec(int elemRandomSeed, FxElemDef* elemDef);

	void	FX_SpatialFrameToOrientation(FxSpatialFrame* frame, game::orientation_t* orient);
	void	FX_TransformPosFromLocalToWorld(FxSpatialFrame* frame, const float* posLocal, float* posWorld);

	void	FX_GetOriginForElem(FxEffect* effect, float* outOrigin, FxElemDef* elemDef, FxSpatialFrame* effectFrameWhenPlayed, int randomSeed);
	void	FX_GetOriginForTrailElem(FxEffect* effect, FxElemDef* elemDef, FxSpatialFrame* effectFrameWhenPlayed, int randomSeed, float* outOrigin, float* outRight, float* outUp);

	void	FX_GetSpawnOrigin(FxSpatialFrame* frameAtSpawn, FxElemDef* elemDef, int randomSeed, float* spawnOrigin);
	void	FX_OffsetSpawnOrigin(FxSpatialFrame* effectFrame, FxElemDef* elemDef, int randomSeed, float* spawnOrigin);

	bool	FX_CullSphere(FxCamera* camera, unsigned int frustumPlaneCount, const float* posWorld, float radius);

	void	FX_OrientationPosFromWorldPos(game::orientation_t* orient, const float* pos, float* out);
	void	FX_OrientationPosToWorldPos(game::orientation_t* orient, const float* pos, float* out);
	void	FX_GetOrientation(FxElemDef* elemDef, FxSpatialFrame* frameAtSpawn, FxSpatialFrame* frameNow, int randomSeed, game::orientation_t* orient);



}