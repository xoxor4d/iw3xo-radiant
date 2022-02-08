#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	bool	FX_ElemUsesMaterial(FxEditorElemDef* elemDef);

	bool	FX_ValidateAtlasSettings(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef);
	bool	FX_ValidateFlags(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef);
	bool	FX_ValidateColor(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef);
	bool	FX_ValidateVisuals(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef);
	bool	FX_ValidatePhysics(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef);
	bool	FX_Validate(FxEditorElemDef* edElemDef, FxEditorEffectDef* editorEffect);

	void	FX_InterpolateSamples(int dimensions, float time0, const float* samples0, float time1, const float* samples1, float timeEval, float* result);
	float	FX_MaxErrorForIntervalCount(int dimensions, int sampleCount, const float* samples, int intervalCount, float errorCutoff);

	void	FX_ReserveElemDefMemory(FxElemDef* elemDef, char** memPool);

	int		FX_AdditionalBytesNeededForElemDef(int visStateSampleCount, int velStateSampleCount, int visualCount, char elemType);
	int		FX_AdditionalBytesNeededForGeomTrail(FxEditorElemDef* edElemDef);
	int		FX_AdditionalBytesNeededForEmission(FxEffectDef* emission);
	int		FX_FindEmission(FxEditorEffectDef* editorEffect, FxEffectDef* emission);

	FxSampleChannel* FX_GetVisualSampleRouting(FxEditorElemDef* edElem, FxSampleChannel* routing);
	int		FX_DecideIntervalLimit(FxEditorElemDef* edElemDef);
	int		FX_DecideSampleCount(int curveCount, FxCurve** curves, int intervalLimit);
	int		FX_DecideVelocitySampleCount(FxEditorElemDef* edElem, int intervalLimit);
	int		FX_DecideVisualSampleCount(FxEditorElemDef* edElem, FxSampleChannel* routing, int intervalLimit);

	void	FX_SampleVelocityInFrame(FxElemVelStateInFrame* velState, int velStateStride, FxEditorElemDef* edElemDef, FxElemDef* elemDef, const float(*velScale)[3], int useGraphBit);
	void	FX_SampleVelocity(FxEditorElemDef* edElemDef, FxElemDef* elemDef);
	void	FX_SampleVisualStateScalar(FxSampleChannel routing, float* amplitude, float* base, FxEditorElemDef* edElemDef, float sampleTime, float scaleFactor);
	void	FX_SampleVisualState(FxElemDef* elemDef, FxEditorElemDef* edElemDef);

	void	FX_BoundFloatRange(FxFloatRange* range, float lower, float upper);	
	void	FX_ScaleFloatRange(FxFloatRange* from, FxFloatRange* to, float scale);

	int		FX_GetLoopingLife(FxEffectDef* effectDef);

	int		FX_CopyEmittedElemDefs(FxElemDef* elemDefArray, FxEditorEffectDef* editorEffect, char** memPool);
	void	FX_CopyMarkVisuals(FxEditorElemDef* edElemDef, FxElemMarkVisuals* markVisualsArray);
	void	FX_CopyVisuals(FxEditorElemDef* edElemDef, FxElemVisuals* visualsArray);
	void	FX_CopyCanonicalRange_FxIntRange_FxIntRange_(FxIntRange* from, FxIntRange* to);
	void	FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(FxFloatRange* from, FxFloatRange* to);

	void	FX_ConvertAtlas(FxEditorElemDef* edElemDef, FxElemDef* elemDef);
	void	FX_ConvertTrail_CalcNormForSegment(const float* vert0, const float* vert1, float* outNormal);
	void	FX_ConvertTrail_CompileVertices(FxEditorElemDef* edElemDef, FxTrailDef* outTrailDef, char** mempool);
	void	FX_ConvertTrail(char** mempool, FxTrailDef** outTrailDef, FxEditorElemDef* edElemDef);
	void	FX_ConvertEffectDefRef(FxEffectDef* result, FxEffectDefRef* ref);
	void	FX_ConvertElemDef(FxElemDef* elemDef, FxEditorElemDef* edElemDef, int velStateCount, int visStateCount, char** memPool);
	int		FX_ConvertElemDefsOfType(const int* velStateCount, FxElemDef* elemDefArray, FxEditorEffectDef* editorEffect, int loopingFlagState, const int* visStateCount, char** memPool);

	FxEffectDef* FX_Convert(FxEditorEffectDef* editorEffect, void* (__cdecl* Alloc)(size_t));
}