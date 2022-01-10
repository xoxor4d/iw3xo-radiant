#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	FxElemDef*			FX_GetUpdateElemDef(FxUpdateElem* update);
	float				FX_GetAtRestFraction(FxUpdateElem* update, float msec);

	void				FX_FrameLerp(FxSpatialFrame* begin, FxSpatialFrame* end, FxSpatialFrame* now, float fraction);

	void				FX_IntegrateVelocityInSegmentInFrame(FxElemVelStateInFrame* statePrev, FxElemVelStateInFrame* stateNext, const float* weight, const float* amplitudeScale, float integralScale, float* pos);
	void				FX_IntegrateVelocityInSegment(int elemDefFlags, game::orientation_t* orient, FxElemVelStateSample* velState, float t0, float t1, const float* amplitudeScale, float integralScale, float* posLocal, float* posWorld);
	void				FX_IntegrateVelocityFromZeroInSegment(FxElemVelStateInFrame* statePrev, FxElemVelStateInFrame* stateNext, float* weight, const float* amplitudeScale, float integralScale, float* pos);
	void				FX_IntegrateVelocityAcrossSegments(int elemDefFlags, game::orientation_t* orient, FxElemVelStateSample* velState0, FxElemVelStateSample* velState1, float t0, float t1, const float* amplitudeScale, float integralScale, float* posLocal, float* posWorld);
	void				FX_IntegrateVelocity(FxUpdateElem* update, float t0, float t1, float* posLocal, float* posWorld);

	void				FX_NextElementPosition_NoExternalForces(int msecUpdateEnd, int msecUpdateBegin, FxUpdateElem* update, float* posLocal, float* posWorld);
	void				FX_NextElementPosition(int msecUpdateEnd, FxUpdateElem* update, int msecUpdateBegin);

	char				FX_ProcessEmitting(char emitResidual, FxUpdateElem* update, FxSystem* system, FxSpatialFrame* frameBegin, FxSpatialFrame* frameEnd);

	bool				FX_UpdateElement_SetupUpdate(FxUpdateElem* update, int msecUpdateEnd, int msecUpdateBegin, FxEffect* effect, int elemDefIndex, int elemAtRestFraction, int elemMsecBegin, int elemSequence, float* elemOrigin);
	bool				FX_UpdateElement_TruncateToElemBegin(FxUpdateElem* update, FxUpdateResult* outUpdateResult);
	void				FX_UpdateElement_TruncateToElemEnd(FxUpdateElem* update, FxUpdateResult* outUpdateResult);
	void				FX_UpdateElement_HandleEmitting(FxUpdateElem* update, FxSystem* system, FxElem* elem, float* elemOriginPrev, FxUpdateResult* outUpdateResult);

	FxUpdateResult		FX_UpdateElementPosition_CollidingStep(int msecUpdateBegin, FxUpdateElem* update, FxSystem* system, int msecUpdateEnd, float* xyzWorldOld);
	FxUpdateResult		FX_UpdateElementPosition_Colliding(FxUpdateElem* update, FxSystem* system);
	FxUpdateResult		FX_UpdateElementPosition(FxUpdateElem* update, FxSystem* system);
	FxUpdateResult		FX_UpdateElement(FxSystem* system, FxEffect* effect, FxElem* elem, int msecUpdateBegin, int msecUpdateEnd);

	void				FX_SpawnOneShotElems(FxSystem* system, FxEffect* effect, int elemDefIndex, FxSpatialFrame* frameWhenPlayed, int msecWhenPlayed);
	void				FX_TriggerOneShot(FxSystem* system, FxEffect* effect, int elemDefFirst, int elemDefCount, FxSpatialFrame* frameWhenPlayed, int msecWhenPlayed);

	void				FX_SetNextUpdateTime(int localClientNum, int time);
	void				FX_SetupCamera_Radiant();

}