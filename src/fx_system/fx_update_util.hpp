#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	



	// *
	// ------------------------------------------

	void	FX_SpatialFrameToOrientation(FxSpatialFrame* frame, game::orientation_t* orient);
	void	FX_TransformPosFromLocalToWorld(FxSpatialFrame* frame, const float* posLocal, float* posWorld);

	void	FX_GetSpawnOrigin(FxSpatialFrame* frameAtSpawn, FxElemDef* elemDef, int randomSeed, float* spawnOrigin);
	void	FX_OffsetSpawnOrigin(FxSpatialFrame* effectFrame, FxElemDef* elemDef, int randomSeed, float* spawnOrigin);

	void	FX_OrientationPosFromWorldPos(game::orientation_t* orient, const float* pos, float* out);
	void	FX_OrientationPosToWorldPos(game::orientation_t* orient, const float* pos, float* out);
	void	FX_GetOrientation(FxElemDef* elemDef, FxSpatialFrame* frameAtSpawn, FxSpatialFrame* frameNow, int randomSeed, game::orientation_t* orient);



}