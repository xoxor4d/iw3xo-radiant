#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	void			FX_DrawElem_BillboardSprite(FxDrawState* draw);
	void			FX_DrawElem_OrientedSprite(FxDrawState* draw);
	void			FX_DrawElem_Tail(FxDrawState* draw);
	void			FX_DrawElem_Cloud(FxDrawState* draw);
	void			FX_DrawElem_Model(FxDrawState* draw);
	void			FX_DrawElem_Light(FxDrawState* draw);
	void			FX_DrawElem_SpotLight(FxDrawState* draw);

	void			FX_DrawSpotLightEffect(FxSystem* system, FxEffect* effect, int msecDraw);
	void			FX_DrawSpotLight(FxSystem* system);

	void			FX_SetPlacement(game::GfxScaledPlacement* placement, FxDrawState* draw);
	void			FX_GetElemAxis(FxElemDef* elemDef, int randomSeed, game::orientation_t* orient, float msecElapsed, float(*axis)[3]);
	void			FX_AnglesToOrientedAxis(float(*axisOut)[3], const float* anglesInRad, game::orientation_t* orient);

	void			FX_GetSpriteTexCoords(FxDrawState* draw, float* s0, float* ds, float* t0, float* dt);
	float			FX_GetMsecForSamplingAxis(float msecElapsed, float msecLifeSpan, int atRestFraction);

	FxElemVisuals	FX_GetElemVisuals(FxElemDef* elemDef, int randomSeed);
	void			FX_SetupVisualState(FxElemDef* elemDef, FxEffect* effect, int randomSeed, float normTimeUpdateEnd, FxElemPreVisualState* preVisState);

	char			FX_InterpolateColor(int channel, FxElemVisStateSample* refState, float valueLerp, float valueLerpInv, float sampleLerp, float sampleLerpInv);
	float			FX_IntegrateRotationFromZero(FxElemVisStateSample* refState, int randomSeed, int randomKey, float sampleLerp, float msecLifeSpan);
	void			FX_EvaluateSize(FxElemPreVisualState* preVisState, FxElemVisualState* visState);
	void			FX_EvaluateVisualState(FxElemPreVisualState* preVisState, float msecLifeSpan, FxElemVisualState* visState);
	void			FX_EvaluateDistanceFade(FxDrawState* draw);
	float			FX_CalculateFade(float dist, FxFloatRange* range);

	void			FX_GenTrail_IndsForSegment(FxDrawState* draw, unsigned __int16 reservedBaseVertex, r_double_index_t* outIndices);
	void			FX_GenTrail_VertsForSegment(FxTrailSegmentDrawState* segmentDrawState, game::GfxPackedVertex* remoteVerts);
	void			FX_GenTrail_PopulateSegmentDrawState(FxTrailSegmentDrawState* outState, const float(*basis)[3], FxDrawState* draw, float spawnDist, float uCoordOffset);
	void			FX_TrailElem_UncompressBasis(float(*basis)[3], const char(*inBasis)[3]);

	unsigned int	FX_CullElementForDraw_FrustumPlaneCount(FxDrawState* draw);
	bool			FX_CullElementForDraw_Sprite(FxDrawState* draw);
	bool			FX_CullElementForDraw_Tail(FxDrawState* draw);
	bool			FX_CullElementForDraw_Cloud(FxDrawState* draw);
	bool			FX_CullElementForDraw_Light(FxDrawState* draw);

	bool			FX_CullSphere(FxCamera* camera, unsigned int frustumPlaneCount, const float* posWorld, float radius);
	bool			FX_CullCylinder(FxCamera* camera, unsigned int frustumPlaneCount, const float* posWorld0, const float* posWorld1, float radius);
	

	void			FX_DrawSpriteEffect(FxSystem* system, FxEffect* effect, int drawTime);
	void			FX_DrawTrailsForEffect(FxSystem* system, FxEffect* effect, int drawTime);
	void			FX_DrawSpriteElems(FxSystem* system, int drawTime);
	void			FX_DrawElement_Setup_1_(FxDrawState* draw, int msecBegin, int sequence, float* origin, float* outRealNormTime);
	void			FX_DrawElement(FxElemDef* elemDef, FxElem* elem, FxDrawState* state);
	void			FX_DrawNonSpriteEffect(int elemClass, int drawTime, FxSystem* system, FxEffect* effect);
	void			FX_DrawNonSpriteElems(FxSystem* system);

	void			FX_GenSpriteVerts(FxDrawState* draw, const float* normal, const float* tangent, const float* binormal);

	void			FX_FillGenerateVertsCmd(int localClientNum, FxGenerateVertsCmd* cmd);
	void			FX_GenerateVerts(FxGenerateVertsCmd* cmd);

	
	game::GfxPackedVertex*	R_GetCodeMeshVerts(unsigned __int16 baseVertex);
	bool					R_ReserveCodeMeshIndices(int indexCount, r_double_index_t** indicesOut);
	bool					R_ReserveCodeMeshVerts(int vertCount, unsigned __int16* baseVertex);
	void					R_AddCodeMeshDrawSurf(game::Material* material, r_double_index_t* indices, unsigned int indexCount, unsigned int argOffset, unsigned int argCount, const char* fxName);
}