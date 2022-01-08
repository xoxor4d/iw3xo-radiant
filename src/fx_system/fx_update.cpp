#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) \
						__debugbreak();		\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__)

#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword

namespace fx_system
{
	int FX_GetElemLifeSpanMsec(int elemRandomSeed, FxElemDef* elemDef)
	{
		return elemDef->lifeSpanMsec.base + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD((&fx_randomTable)[elemRandomSeed + 17])) >> 16);
	}

	FxElemDef* FX_GetUpdateElemDef(FxUpdateElem* update)
	{
		if (!update->effect)
		{
			Assert();
		}

		return &update->effect->def->elemDefs[update->elemIndex];
	}

	float FX_GetAtRestFraction(FxUpdateElem* update, float msec)
	{
		return ceilf((msec - static_cast<float>(update->msecElemBegin)) * 255.0f / update->msecLifeSpan - 0.25f);
	}

	void __cdecl FX_GetQuatForOrientation(FxElemDef* elemDef, FxSpatialFrame* frameNow, FxEffect* effect, float* quat, game::orientation_t* orient)
	{
		const int runFlags = elemDef->flags & FX_ELEM_RUN_RELATIVE_TO_OFFSET;

		if (runFlags)
		{
			if (runFlags == FX_ELEM_RUN_RELATIVE_TO_SPAWN)
			{
				quat[0] = effect->frameAtSpawn.quat[0];
				quat[1] = effect->frameAtSpawn.quat[1];
				quat[2] = effect->frameAtSpawn.quat[2];
				quat[3] = effect->frameAtSpawn.quat[3];
			}
			else if (runFlags == FX_ELEM_RUN_RELATIVE_TO_EFFECT)
			{
				quat[0] = frameNow->quat[0];
				quat[1] = frameNow->quat[1];
				quat[2] = frameNow->quat[2];
				quat[3] = frameNow->quat[3];
			}
			else
			{
				if (runFlags != FX_ELEM_RUN_RELATIVE_TO_OFFSET)
				{
					Assert();
				}

				AxisToQuat(orient->axis, quat);
			}
		}
		else
		{
			quat[0] = 0.0f;
			quat[1] = 0.0f;
			quat[2] = 0.0f;
			quat[3] = 1.0f;
		}
	}

	void FX_FrameLerp(FxSpatialFrame* begin, FxSpatialFrame* end, FxSpatialFrame* now, float fraction)
	{
		now->origin[0] = (end->origin[0] - begin->origin[0]) * fraction + begin->origin[0];
		now->origin[1] = (end->origin[1] - begin->origin[1]) * fraction + begin->origin[1];
		now->origin[2] = (end->origin[2] - begin->origin[2]) * fraction + begin->origin[2];
		now->quat[0] = (end->quat[0] - begin->quat[0]) * fraction + begin->quat[0];
		now->quat[1] = (end->quat[1] - begin->quat[1]) * fraction + begin->quat[1];
		now->quat[2] = (end->quat[2] - begin->quat[2]) * fraction + begin->quat[2];
		now->quat[3] = (end->quat[3] - begin->quat[3]) * fraction + begin->quat[3];

		Vec4Normalize(now->quat);
	}

	void FX_IntegrateVelocityInSegmentInFrame(FxElemVelStateInFrame* statePrev, FxElemVelStateInFrame* stateNext, const float* weight, const float* amplitudeScale, [[maybe_unused]] float integralScale, float* pos)
	{
		pos[0] = (((amplitudeScale[0] * statePrev->velocity.amplitude[0]) + statePrev->velocity.base[0]) * weight[0]) + pos[0];
		pos[0] = (((amplitudeScale[0] * stateNext->velocity.amplitude[0]) + stateNext->velocity.base[0]) * weight[1]) + pos[0];
		pos[1] = (((amplitudeScale[1] * statePrev->velocity.amplitude[1]) + statePrev->velocity.base[1]) * weight[0]) + pos[1];
		pos[1] = (((amplitudeScale[1] * stateNext->velocity.amplitude[1]) + stateNext->velocity.base[1]) * weight[1]) + pos[1];
		pos[2] = (((amplitudeScale[2] * statePrev->velocity.amplitude[2]) + statePrev->velocity.base[2]) * weight[0]) + pos[2];
		pos[2] = (((amplitudeScale[2] * stateNext->velocity.amplitude[2]) + stateNext->velocity.base[2]) * weight[1]) + pos[2];
	}

	void FX_IntegrateVelocityInSegment(int elemDefFlags, game::orientation_t* orient, FxElemVelStateSample* velState, float t0, float t1, const float* amplitudeScale, float integralScale, float* posLocal, float* posWorld)
	{
		if ((t0 < 0.0f || t0 > 1.0f) || (t1 < 0.0f || t1 > 1.0f))
		{
			Assert();
		}

		float weight[2];
		weight[1] = integralScale * 0.5f * (t1 * t1 - t0 * t0);
		weight[0] = (t1 - t0) * integralScale - weight[1];

		if ((elemDefFlags & FX_ELEM_HAS_VELOCITY_GRAPH_LOCAL) != 0)
		{
			FX_IntegrateVelocityInSegmentInFrame(&velState->local, &velState[1].local, weight, amplitudeScale, integralScale, posLocal);
		}

		FX_OrientationPosToWorldPos(orient, posLocal, posWorld);

		if ((elemDefFlags & FX_ELEM_HAS_VELOCITY_GRAPH_WORLD) != 0)
		{
			FX_IntegrateVelocityInSegmentInFrame(&velState->world, &velState[1].world, weight, amplitudeScale, integralScale, posWorld);
		}
	}

	void FX_IntegrateVelocityFromZeroInSegment(FxElemVelStateInFrame* statePrev, FxElemVelStateInFrame* stateNext, float* weight, const float* amplitudeScale, float integralScale, float* pos)
	{
		pos[0] = (((amplitudeScale[0] * statePrev->totalDelta.amplitude[0]) + statePrev->totalDelta.base[0]) * integralScale) + pos[0];
		pos[0] = (((amplitudeScale[0] * statePrev->velocity.amplitude[0])	+ statePrev->velocity.base[0])	 * weight[0])	  + pos[0];
		pos[0] = (((amplitudeScale[0] * stateNext->velocity.amplitude[0])	+ stateNext->velocity.base[0])	 * weight[1])	  + pos[0];
		pos[1] = (((amplitudeScale[1] * statePrev->totalDelta.amplitude[1])	+ statePrev->totalDelta.base[1]) * integralScale) + pos[1];
		pos[1] = (((amplitudeScale[1] * statePrev->velocity.amplitude[1])	+ statePrev->velocity.base[1])	 * weight[0])	  + pos[1];
		pos[1] = (((amplitudeScale[1] * stateNext->velocity.amplitude[1])	+ stateNext->velocity.base[1])	 * weight[1])	  + pos[1];
		pos[2] = (((amplitudeScale[2] * statePrev->totalDelta.amplitude[2])	+ statePrev->totalDelta.base[2]) * integralScale) + pos[2];
		pos[2] = (((amplitudeScale[2] * statePrev->velocity.amplitude[2])	+ statePrev->velocity.base[2])	 * weight[0])	  + pos[2];
		pos[2] = (((amplitudeScale[2] * stateNext->velocity.amplitude[2])	+ stateNext->velocity.base[2])	 * weight[1])	  + pos[2];
	}

	void FX_IntegrateVelocityAcrossSegments(int elemDefFlags, game::orientation_t* orient, FxElemVelStateSample* velState0, FxElemVelStateSample* velState1, float t0, float t1, const float* amplitudeScale, float integralScale, float* posLocal, float* posWorld)
	{
		if ((t0 < 0.0f || t0 > 1.0f) || (t1 < 0.0f || t1 > 1.0f))
		{
			Assert();
		}

		float w0[2]; float w1[2];

		const float t0_integral = -integralScale * t0;
		w0[1] = (0.5f * t0) * t0_integral;
		w0[0] = t0_integral - w0[1];
		w1[1] = (0.5f * t1) * (t1 * integralScale);
		w1[0] = (t1 * integralScale) - w1[1];

		FxElemVelStateSample* localVelState0 = velState0;

		if ((elemDefFlags & FX_ELEM_HAS_VELOCITY_GRAPH_LOCAL) != 0)
		{
			FX_IntegrateVelocityFromZeroInSegment(&velState1->local, &velState1[1].local, w1, amplitudeScale, integralScale, posLocal);
			FX_IntegrateVelocityFromZeroInSegment(&localVelState0->local, &localVelState0[1].local, w0, amplitudeScale, -integralScale, posLocal);
		}

		FX_OrientationPosToWorldPos(orient, posLocal, posWorld);

		if ((elemDefFlags & FX_ELEM_HAS_VELOCITY_GRAPH_WORLD) != 0)
		{
			FX_IntegrateVelocityFromZeroInSegment(&velState1->world, &velState1[1].world, w1, amplitudeScale, integralScale, posWorld);
			FX_IntegrateVelocityFromZeroInSegment(&localVelState0->world, &localVelState0[1].world, w0, amplitudeScale, -integralScale, posWorld);
		}
	}

	void FX_IntegrateVelocity(FxUpdateElem* update, float t0, float t1, float* posLocal, float* posWorld)
	{
		FxElemDef* elemDef = FX_GetUpdateElemDef(update);

		if (!update || !elemDef || !elemDef->velSamples || !elemDef->velIntervalCount)
		{
			Assert();
		}

		if (t0 < 0.0f || t1 <= t0 || t1 > 1.0f)
		{
			Assert();
		}

		const float rangeLerp[3] =
		{
			fx_randomTable[0 + update->randomSeed],
			fx_randomTable[1 + update->randomSeed],
			fx_randomTable[2 + update->randomSeed]
		};

		if (elemDef->velIntervalCount == 1)
		{
			FX_IntegrateVelocityInSegment(elemDef->flags, &update->orient, elemDef->velSamples, t0, t1, rangeLerp, update->msecLifeSpan, posLocal, posWorld);
		}
		else
		{

			const float startPoint = static_cast<float>(elemDef->velIntervalCount) * t0;
			const float endPoint = static_cast<float>(elemDef->velIntervalCount) * t1;

			const int startIndex = static_cast<int>(floorf(startPoint));
			const int endIndex = static_cast<int>(ceilf(endPoint)) - 1;

			const float startLerp = startPoint - static_cast<float>(startIndex);
			const float endLerp = endPoint - static_cast<float>(endIndex);

			if (startIndex > endIndex)
			{
				Assert();
			}

			if (startIndex < 0 || startIndex >= elemDef->velIntervalCount)
			{
				Assert();
			}

			if (endIndex < 0 || endIndex >= elemDef->velIntervalCount)
			{
				Assert();
			}

			if (startIndex == endIndex)
			{
				FX_IntegrateVelocityInSegment(elemDef->flags, &update->orient, &elemDef->velSamples[startIndex], startLerp, endLerp, rangeLerp, update->msecLifeSpan, posLocal, posWorld);
			}
			else
			{
				FX_IntegrateVelocityAcrossSegments(elemDef->flags, &update->orient, &elemDef->velSamples[startIndex], &elemDef->velSamples[endIndex], startLerp, endLerp, rangeLerp, update->msecLifeSpan, posLocal, posWorld);
			}
		}
	}

	void FX_NextElementPosition_NoExternalForces(int msecUpdateEnd, int msecUpdateBegin, FxUpdateElem* update, float* posLocal, float* posWorld)
	{
		if (msecUpdateEnd - msecUpdateBegin <= 0 || !posLocal)
		{
			Assert();
		}

		const float t0 = static_cast<float>(msecUpdateBegin - update->msecElemBegin) / update->msecLifeSpan;
		const float t1 = static_cast<float>(msecUpdateEnd - update->msecElemBegin) / update->msecLifeSpan;
		
		FX_IntegrateVelocity(update, t0, t1, posLocal, posWorld);
	}

	void FX_NextElementPosition(int msecUpdateEnd, FxUpdateElem* update, int msecUpdateBegin)
	{
		if ((msecUpdateEnd - msecUpdateBegin) <= 0)
		{
			Assert();
		}

		float posLocal[3];
		posLocal[0] = update->elemOrigin[0];
		posLocal[1] = update->elemOrigin[1];
		posLocal[2] = update->elemOrigin[2];

		FX_NextElementPosition_NoExternalForces(msecUpdateEnd, msecUpdateBegin, update, posLocal, update->posWorld);

		const float secDuration = static_cast<float>(msecUpdateEnd - msecUpdateBegin) * 0.001f;
		update->posWorld[0] = update->elemBaseVel[0] * secDuration + update->posWorld[0];
		update->posWorld[1] = update->elemBaseVel[1] * secDuration + update->posWorld[1];
		update->posWorld[2] = update->elemBaseVel[2] * secDuration + update->posWorld[2];

		const FxElemDef* elemDef = FX_GetUpdateElemDef(update);
		const float deltaVelFromGravity = (fx_randomTable[15 + update->randomSeed] * elemDef->gravity.amplitude + elemDef->gravity.base) * 800.0f * secDuration;

		update->elemBaseVel[2] = update->elemBaseVel[2] - deltaVelFromGravity;
		update->posWorld[2] = update->posWorld[2] - secDuration * deltaVelFromGravity * 0.5f;
	}

	char FX_ProcessEmitting(char emitResidual, FxUpdateElem* update, FxSystem* system, FxSpatialFrame* frameBegin, FxSpatialFrame* frameEnd)
	{
		FxSpatialFrame frameElemNow = {};
		float distLastEmit;

		FxElemDef* elemDef = FX_GetUpdateElemDef(update);

		float axisSpawn[3][3];
		axisSpawn[0][0] = frameEnd->origin[0] - frameBegin->origin[0];
		axisSpawn[0][1] = frameEnd->origin[1] - frameBegin->origin[1];
		axisSpawn[0][2] = frameEnd->origin[2] - frameBegin->origin[2];

		const float distInUpdate = Vec3Normalize((float*)axisSpawn);
		if (distInUpdate == 0.0f)
		{
			return emitResidual;
		}

		const float baseDistPerEmit = ((fx_randomTable[20 + update->randomSeed] * elemDef->emitDist.amplitude) + elemDef->emitDist.base) + elemDef->emitDistVariance.base;
		const float maxDistPerEmit = baseDistPerEmit + elemDef->emitDistVariance.amplitude;
		float distNextEmit = -((static_cast<float>(emitResidual) * maxDistPerEmit) * 0.00390625f);

		while (true)
		{
			distLastEmit = distNextEmit;
			distNextEmit = (((static_cast<float>(rand()) * 0.000030517578f) * elemDef->emitDistVariance.amplitude) + baseDistPerEmit) + distNextEmit;

			if (distNextEmit > distInUpdate)
			{
				break;
			}

			if ((distNextEmit - 0.0f) < 0.0f)
			{
				distNextEmit = 0.0f;
			}

			int msecAtSpawn = update->msecUpdateBegin + static_cast<int>(floor(static_cast<float>(update->msecUpdateEnd - update->msecUpdateBegin) * (distNextEmit / distInUpdate)));

			FX_FrameLerp(frameBegin, frameEnd, &frameElemNow, distNextEmit / distInUpdate);
			PerpendicularVector(axisSpawn[0], axisSpawn[1]);
			Vec3Cross(axisSpawn[0], axisSpawn[1], axisSpawn[2]);


			//handle = FX_SpawnEffect(system, elemDef->effectEmitted.handle, msecAtSpawn, frameElemNow.origin, axisSpawn, 2047, 511, 255, update->effect->owner, 0x3FFu, &orIdentity);
			FxEffect* effect = FX_SpawnEffect(system, elemDef->effectEmitted.handle, msecAtSpawn, frameElemNow.origin, (const float(*)[3])axisSpawn, FX_SPAWN_DOBJ_HANDLES, FX_SPAWN_BONE_INDEX, 255, update->effect->owner, FX_SPAWN_MARK_ENTNUM);

			if (effect)
			{
				FX_DelRefToEffect(system, effect);
			}

			elemDef = FX_GetUpdateElemDef(update);
		}

		const float residual = distInUpdate - distLastEmit;
		if ((distInUpdate - distLastEmit) < -0.001f || (maxDistPerEmit + 0.001f) < residual)
		{
			Assert();
		}

		return static_cast<char>(((256.0f * residual) / maxDistPerEmit) + 9.313225746154785e-10);
	}

	bool FX_UpdateElement_SetupUpdate(FxUpdateElem* update, int msecUpdateEnd, int msecUpdateBegin, FxEffect* effect, int elemDefIndex, int elemAtRestFraction, int elemMsecBegin, int elemSequence, float* elemOrigin)
	{
		memset(update, 208, sizeof(FxUpdateElem));

		update->effect = effect;
		update->msecUpdateBegin = msecUpdateBegin;
		update->msecUpdateEnd = msecUpdateEnd;
		update->msecElemBegin = elemMsecBegin;

		if (msecUpdateBegin > msecUpdateEnd)
		{
			Assert();
		}

		if (update->msecUpdateEnd < update->msecElemBegin)
		{
			return false;
		}

		if (elemDefIndex >= effect->def->elemDefCountLooping + effect->def->elemDefCountOneShot + effect->def->elemDefCountEmission)
		{
			return false;
		}

		update->atRestFraction = elemAtRestFraction;
		update->elemIndex = elemDefIndex;
		update->sequence = elemSequence;
		update->randomSeed = (0x128 * elemSequence + elemMsecBegin + (unsigned int)effect->randomSeed) % 0x1DF;

		FxElemDef* elemDef = FX_GetUpdateElemDef(update);
		const int random_lifespan = FX_GetElemLifeSpanMsec(update->randomSeed, elemDef);

		update->msecElemEnd = random_lifespan + update->msecElemBegin;
		update->msecLifeSpan = static_cast<float>(random_lifespan);
		update->elemOrigin = elemOrigin;


		return true;
	}

	bool FX_UpdateElement_TruncateToElemBegin(FxUpdateElem* update, FxUpdateResult* outUpdateResult)
	{
		if (update->msecUpdateBegin < update->msecElemBegin)
		{
			update->msecUpdateBegin = update->msecElemBegin;
		}

		if (update->msecUpdateBegin == update->msecUpdateEnd)
		{
			*outUpdateResult = (FxUpdateResult)(update->msecUpdateBegin < update->msecElemEnd);
			return false;
		}

		if (update->msecLifeSpan <= 0.0f)
		{
			Assert();
		}

		update->msecElapsed = static_cast<float>(update->msecUpdateEnd - update->msecElemBegin);
		update->normTimeUpdateEnd = update->msecElapsed / update->msecLifeSpan;

		if (update->normTimeUpdateEnd < 0.0f || update->normTimeUpdateEnd > 1.0f)
		{
			Assert();
		}

		FxElemDef* elemDef = FX_GetUpdateElemDef(update);
		FX_GetOrientation(elemDef, &update->effect->frameAtSpawn, &update->effect->frameNow, update->randomSeed, &update->orient);

		return true;
	}

	void FX_UpdateElement_TruncateToElemEnd(FxUpdateElem* update, FxUpdateResult* outUpdateResult)
	{
		if (update->msecUpdateEnd >= update->msecElemEnd)
		{
			if (FX_GetUpdateElemDef(update)->effectEmitted.handle)
			{
				update->msecUpdateEnd = update->msecElemEnd;
				if (update->msecUpdateBegin > update->msecUpdateEnd)
				{
					update->msecUpdateBegin = update->msecUpdateEnd;
				}
			}
			else
			{
				*outUpdateResult = FX_UPDATE_REMOVE;
			}
		}
	}

	void FX_UpdateElement_HandleEmitting(FxUpdateElem* update, FxSystem* system, FxElem* elem, float* elemOriginPrev, FxUpdateResult* outUpdateResult)
	{
		FxElemDef* elemDef = FX_GetUpdateElemDef(update);

		if (elemDef->effectEmitted.handle)
		{
			game::orientation_t orient = {};
			FxSpatialFrame frameBegin = {};
			FxSpatialFrame frameEnd = {};

			FxEffect* effect = update->effect;

			FX_GetOrientation(elemDef, &update->effect->frameAtSpawn, &update->effect->framePrev, update->randomSeed, &orient);
			FX_OrientationPosToWorldPos(&orient, elemOriginPrev, frameBegin.origin);
			FX_GetQuatForOrientation(elemDef, &effect->framePrev, effect, frameBegin.quat, &orient);

			frameEnd.origin[0] = update->posWorld[0];
			frameEnd.origin[1] = update->posWorld[1];
			frameEnd.origin[2] = update->posWorld[2];

			FX_GetQuatForOrientation(elemDef, &effect->frameNow, effect, frameEnd.quat, &update->orient);

			elem->emitResidual = FX_ProcessEmitting(elem->emitResidual, update, system, &frameBegin, &frameEnd);

			if (update->msecUpdateEnd == update->msecElemEnd)
			{
				*outUpdateResult = FX_UPDATE_REMOVE;
			}
		}
	}

	FxUpdateResult FX_UpdateElementPosition_CollidingStep(int msecUpdateBegin, FxUpdateElem* update, [[maybe_unused]] FxSystem* system, int msecUpdateEnd, float* xyzWorldOld)
	{
		// CM_BoxTrace does nothing but this:
		trace_t trace = {};
		trace.fraction = 1.0;

		do
		{
			update->onGround = false;
			FX_NextElementPosition(msecUpdateEnd, update, msecUpdateBegin);

			//CM_BoxTrace(&trace);
			if(!trace.startsolid && !trace.allsolid && 1.0f != trace.fraction) //if (!FX_TraceHitSomething(&trace))
			{
				// always breaks
				break;
			}

			/*if (trace.normal[2] > 0.69999999f)
			{
				update->onGround = true;
			}
			msecUpdateBegin = FX_CollisionResponse(update, (int)system, &trace.fraction, msecUpdateBegin, msecUpdateEnd, xyzWorldOld);
			if ((FX_GetUpdateElemDef(update)->flags & 0x200) != 0)
			{
				return FX_UPDATE_REMOVE;
			}*/
		} while (msecUpdateBegin == msecUpdateEnd);

		FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);

		xyzWorldOld[0] = update->posWorld[0];
		xyzWorldOld[1] = update->posWorld[1];
		xyzWorldOld[2] = update->posWorld[2];

		return FX_UPDATE_KEEP;
	}

	FxUpdateResult FX_UpdateElementPosition_Colliding(FxUpdateElem* update, FxSystem* system)
	{
		if (update->atRestFraction == 255)
		{
			float xyzWorldOld[3];
			FX_OrientationPosToWorldPos(&update->orient, update->elemOrigin, xyzWorldOld);

			int msecUpdateBegin = update->msecUpdateBegin;
			for (int msecUpdatePartial = msecUpdateBegin + 50; msecUpdatePartial < update->msecUpdateEnd; msecUpdatePartial += 50)
			{
				if (FX_UpdateElementPosition_CollidingStep(msecUpdateBegin, update, system, update->msecUpdateEnd, xyzWorldOld) == FX_UPDATE_REMOVE)
				{
					return FX_UPDATE_REMOVE;
				}

				msecUpdateBegin = msecUpdatePartial;
			}

			return FX_UpdateElementPosition_CollidingStep(msecUpdateBegin, update, system, update->msecUpdateEnd, xyzWorldOld);
		}

		FX_OrientationPosToWorldPos(&update->orient, update->elemOrigin, update->posWorld);

		return FX_UPDATE_KEEP;
	}

	FxUpdateResult FX_UpdateElementPosition(FxUpdateElem* update, FxSystem* system)
	{
		FxElemDef* elemDef = FX_GetUpdateElemDef(update);
		if (elemDef->elemType == FX_ELEM_TYPE_MODEL && (elemDef->flags & FX_ELEM_USE_MODEL_PHYSICS) != 0)
		{
			return FxUpdateResult::FX_UPDATE_KEEP;
		}

		if ((elemDef->flags & FX_ELEM_USE_COLLISION) != 0)
		{
			return FX_UpdateElementPosition_Colliding(update, system);
		}

		if((elemDef->flags & (FX_ELEM_HAS_VELOCITY_GRAPH_WORLD | FX_ELEM_HAS_GRAVITY)) != 0)
		{
			FX_NextElementPosition(update->msecUpdateEnd, update, update->msecUpdateBegin);
			FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);
			return FxUpdateResult::FX_UPDATE_KEEP;
		}

		// if ( 0.0 != *v2 || 0.0 != v2[1] || 0.0 != v2[2] )
		if(!Vec3Compare(update->elemBaseVel, game::vec3_origin))
		{
			Assert();
		}

		FX_NextElementPosition_NoExternalForces(update->msecUpdateEnd, update->msecUpdateBegin, update, update->elemOrigin, update->posWorld);

		return FxUpdateResult::FX_UPDATE_KEEP;
	}

	FxUpdateResult FX_UpdateElement(FxSystem* system, FxEffect* effect, FxElem* elem, int msecUpdateBegin, int msecUpdateEnd)
	{
		if (!elem)
		{
			Assert();
		}

		FxUpdateElem update = {};
		FxUpdateResult updateResult = FX_UPDATE_KEEP;

		if (!FX_UpdateElement_SetupUpdate(&update, msecUpdateEnd, msecUpdateBegin, effect, elem->defIndex, elem->atRestFraction, elem->msecBegin, elem->sequence, elem->___u8.origin))
		{
			return updateResult;
		}

		float elemOriginPrev[3] = {};

		FX_UpdateElement_TruncateToElemEnd(&update, &updateResult);
		if (updateResult)
		{
			if (!FX_UpdateElement_TruncateToElemBegin(&update, &updateResult))
			{
				return updateResult;
			}

			elemOriginPrev[0] = elem->___u8.origin[0];
			elemOriginPrev[1] = elem->___u8.origin[1];
			elemOriginPrev[2] = elem->___u8.origin[2];

			update.elemBaseVel = elem->baseVel;
			update.physObjId = elem->___u8.physObjId;
			update.onGround = false;

			updateResult = FX_UpdateElementPosition(&update, system);
			FX_UpdateElement_HandleEmitting(&update, system, elem, elemOriginPrev, &updateResult);
		}

		FxElemDef* elemDef = FX_GetUpdateElemDef(&update);
		if (updateResult)
		{
			if (update.atRestFraction == 255 && Vec3Compare(elem->___u8.origin, elemOriginPrev) && ((elemDef->flags & 0x100) == 0 || update.onGround))
			{
				elem->atRestFraction = static_cast<char>(FX_GetAtRestFraction(&update, static_cast<float>(update.msecUpdateEnd)));
				return updateResult;
			}

			elem->atRestFraction = static_cast<char>(update.atRestFraction);
		}
		else if (elemDef->effectOnDeath.handle)
		{
			FX_SpawnDeathEffect(&update, system);
			return updateResult;
		}

		return updateResult;
	}

	void FX_SetNextUpdateTime(int localClientNum, int time)
	{
		const auto system = FX_GetSystem(localClientNum);

		if (time < system->msecNow)
		{
			Assert();
		}

		system->camera.isValid = 0;
		system->msecDraw = time;
		system->msecNow = time;

		if (++system->frameCount <= 0)
		{
			system->frameCount = 1;
		}
	}

	void FX_SetupCamera(FxCamera* camera, const float* vieworg, const float(*viewaxis)[3], const float tanHalfFovX, const float tanHalfFovY, const float zfar)
	{
		const float cosHalfFovX = 1.0f / sqrtf((tanHalfFovX * tanHalfFovX) + 1.0f);
		const float cosHalfFovY = 1.0f / sqrtf((tanHalfFovY * tanHalfFovY) + 1.0f);

		camera->origin[0] = vieworg[0];
		camera->origin[1] = vieworg[1];
		camera->origin[2] = vieworg[2];

		AxisCopy(viewaxis, camera->axis);

		camera->frustum[0][0] = (*viewaxis)[0];
		camera->frustum[0][1] = (*viewaxis)[1];
		camera->frustum[0][2] = (*viewaxis)[2];
		
		camera->frustum[1][0] = ((tanHalfFovX * cosHalfFovX) * (*viewaxis)[0]) + (cosHalfFovX * (*viewaxis)[3]);
		camera->frustum[1][1] = ((tanHalfFovX * cosHalfFovX) * (*viewaxis)[1]) + (cosHalfFovX * (*viewaxis)[4]);
		camera->frustum[1][2] = ((tanHalfFovX * cosHalfFovX) * (*viewaxis)[2]) + (cosHalfFovX * (*viewaxis)[5]);
		camera->frustum[2][0] = ((tanHalfFovX * cosHalfFovX) * (*viewaxis)[0]) + (-cosHalfFovX * (*viewaxis)[3]);
		camera->frustum[2][1] = ((tanHalfFovX * cosHalfFovX) * (*viewaxis)[1]) + (-cosHalfFovX * (*viewaxis)[4]);
		camera->frustum[2][2] = ((tanHalfFovX * cosHalfFovX) * (*viewaxis)[2]) + (-cosHalfFovX * (*viewaxis)[5]);

		camera->frustum[3][0] = ((tanHalfFovY * cosHalfFovY) * (*viewaxis)[0]) + (cosHalfFovY * (*viewaxis)[6]);
		camera->frustum[3][1] = ((tanHalfFovY * cosHalfFovY) * (*viewaxis)[1]) + (cosHalfFovY * (*viewaxis)[7]);
		camera->frustum[3][2] = ((tanHalfFovY * cosHalfFovY) * (*viewaxis)[2]) + (cosHalfFovY * (*viewaxis)[8]);
		camera->frustum[4][0] = ((tanHalfFovY * cosHalfFovY) * (*viewaxis)[0]) + (-cosHalfFovY * (*viewaxis)[6]);
		camera->frustum[4][1] = ((tanHalfFovY * cosHalfFovY) * (*viewaxis)[1]) + (-cosHalfFovY * (*viewaxis)[7]);
		camera->frustum[4][2] = ((tanHalfFovY * cosHalfFovY) * (*viewaxis)[2]) + (-cosHalfFovY * (*viewaxis)[8]);

		camera->frustumPlaneCount = 5;

		for (unsigned int planeIndex = 0; planeIndex < camera->frustumPlaneCount; ++planeIndex)
		{
			if (!Vec3IsNormalized(camera->frustum[planeIndex]))
			{
				Assert();
			}

			camera->frustum[planeIndex][3] = 
				  (camera->origin[0] * camera->frustum[planeIndex][0])
				+ (camera->origin[1] * camera->frustum[planeIndex][1])
				+ (camera->origin[2] * camera->frustum[planeIndex][2]);
		}

		if (zfar > 0.0f)
		{
			camera->frustum[5][0] = -(*viewaxis)[0];
			camera->frustum[5][1] = -(*viewaxis)[1];
			camera->frustum[5][2] = -(*viewaxis)[2];
			camera->frustum[5][3] = -camera->frustum[0][3] - zfar;
			camera->frustumPlaneCount = 6;
		}
	}

	// radiant setup
	void FX_SetupCamera_Radiant()
	{
		const auto system = FX_GetSystem(0);
		const auto cam = &cmainframe::activewnd->m_pCamWnd->camera;

		float axis[3][3] = {};
		axis[0][0] = cam->vpn[0];
		axis[0][1] = cam->vpn[1];
		axis[0][2] = cam->vpn[2];
		axis[1][0] = -cam->vright[0];
		axis[1][1] = -cam->vright[1];
		axis[1][2] = -cam->vright[2];
		axis[2][0] = cam->vup[0];
		axis[2][1] = cam->vup[1];
		axis[2][2] = cam->vup[2];

		const float halfTanY = tanf(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f;
		const float halfTanX = halfTanY * (static_cast<float>(cam->width) / static_cast<float>(cam->height));

		FX_SetupCamera(&system->camera, cam->origin, axis, halfTanX, halfTanY, 0.0f);
	}

}