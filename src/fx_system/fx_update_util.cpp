#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace fx_system
{
	// checked
	int FX_GetElemLifeSpanMsec(int elemRandomSeed, FxElemDef* elemDef)
	{
		int rnd;

		// JTAG :*
		rnd = *reinterpret_cast<std::uint16_t*>(&fx_randomTable[17 + elemRandomSeed]);
		rnd *= (elemDef->lifeSpanMsec.amplitude + 1);
		rnd = rnd >> 16;
		rnd += elemDef->lifeSpanMsec.base;

		return rnd;
	}

	void FX_GetVelocityAtTimeInFrame(float* velocity, const float* rangeLerp, const float* weight, FxElemVelStateInFrame* stateNext, FxElemVelStateInFrame* statePrev)
	{
		velocity[0] = (rangeLerp[0] * statePrev->velocity.amplitude[0] + statePrev->velocity.base[0]) * weight[0];
		velocity[0] = (rangeLerp[0] * stateNext->velocity.amplitude[0] + stateNext->velocity.base[0]) * weight[1] + velocity[0];

		velocity[1] = (rangeLerp[1] * statePrev->velocity.amplitude[1] + statePrev->velocity.base[1]) * weight[0];
		velocity[1] = (rangeLerp[1] * stateNext->velocity.amplitude[1] + stateNext->velocity.base[1]) * weight[1] + velocity[1];

		velocity[2] = (rangeLerp[2] * statePrev->velocity.amplitude[2] + statePrev->velocity.base[2]) * weight[0];
		velocity[2] = (rangeLerp[2] * stateNext->velocity.amplitude[2] + stateNext->velocity.base[2]) * weight[1] + velocity[2];
	}

	// checked
	void FX_GetVelocityAtTime(FxElemDef* elemDef, int randomSeed, float msecLifeSpan, float msecElapsed, game::orientation_t* orient, const float* baseVel, float* velocity)
	{
		if (!elemDef || !elemDef->velSamples || !elemDef->velIntervalCount)
		{
			Assert();
		}
		

		const float sampleTime = msecElapsed / msecLifeSpan;
		if (sampleTime < 0.0f && sampleTime > 1.0f)
		{
			Assert();
		}

		const float rangeLerp[3] =
		{
			fx_randomTable[0 + randomSeed],
			fx_randomTable[1 + randomSeed],
			fx_randomTable[2 + randomSeed]
		};

		const auto intervalCount = static_cast<float>( static_cast<std::uint8_t>(elemDef->velIntervalCount) );
		const float samplePoint = sampleTime * intervalCount;

		const int sampleIndex = static_cast<int>( floor(samplePoint) );
		const float sampleLerp = samplePoint - static_cast<float>(sampleIndex);

		if (sampleIndex < 0 || sampleIndex >= static_cast<std::uint8_t>(elemDef->velIntervalCount))
		{
			Assert();
		}

		FxElemVelStateSample* samples = &elemDef->velSamples[sampleIndex];

		float weight[2] = {};
		weight[1] = intervalCount * sampleLerp;
		weight[0] = intervalCount - weight[1];

		velocity[0] = baseVel[0];
		velocity[1] = baseVel[1];
		velocity[2] = baseVel[2];

		float velocityWorld[3] = {};

		if ((elemDef->flags & FX_ELEM_HAS_VELOCITY_GRAPH_WORLD) != 0)
		{
			FX_GetVelocityAtTimeInFrame(velocityWorld, rangeLerp, weight, &samples[1].world, &samples->world);
			velocity[0] = velocityWorld[0] * 1000.0f + velocity[0];
			velocity[1] = velocityWorld[1] * 1000.0f + velocity[1];
			velocity[2] = velocityWorld[2] * 1000.0f + velocity[2];
		}

		if ((elemDef->flags & FX_ELEM_HAS_VELOCITY_GRAPH_LOCAL) != 0)
		{
			float velocityLocal[3] = {};
			FX_GetVelocityAtTimeInFrame(velocityLocal, rangeLerp, weight, &samples[1].local, &samples->local);
			FX_OrientationDirToWorldDir(orient, velocityLocal, velocityWorld);

			velocity[0] = velocityWorld[0] * 1000.0f + velocity[0];
			velocity[1] = velocityWorld[1] * 1000.0f + velocity[1];
			velocity[2] = velocityWorld[2] * 1000.0f + velocity[2];
		}
	}

	void FX_SpatialFrameToOrientation(FxSpatialFrame* frame, game::orientation_t* orient)
	{
		orient->origin[0] = frame->origin[0];
		orient->origin[1] = frame->origin[1];
		orient->origin[2] = frame->origin[2];

		UnitQuatToAxis(frame->quat, orient->axis);
	}

	void FX_TransformPosFromLocalToWorld(FxSpatialFrame* frame, const float* posLocal, float* posWorld)
	{
		float axis[3][3];
		UnitQuatToAxis(frame->quat, axis);

		posWorld[0] = (float)(posLocal[0] * axis[0][0]) + frame->origin[0];
		posWorld[1] = (float)(posLocal[0] * axis[0][1]) + frame->origin[1];
		posWorld[2] = (float)(posLocal[0] * axis[0][2]) + frame->origin[2];

		posWorld[0] = (float)(posLocal[1] * axis[1][0]) + posWorld[0];
		posWorld[1] = (float)(posLocal[1] * axis[1][1]) + posWorld[1];
		posWorld[2] = (float)(posLocal[1] * axis[1][2]) + posWorld[2];

		posWorld[0] = (float)(posLocal[2] * axis[2][0]) + posWorld[0];
		posWorld[1] = (float)(posLocal[2] * axis[2][1]) + posWorld[1];
		posWorld[2] = (float)(posLocal[2] * axis[2][2]) + posWorld[2];
	}

	void FX_GetOriginForElem(FxEffect* effect, float* outOrigin, FxElemDef* elemDef, FxSpatialFrame* effectFrameWhenPlayed, int randomSeed)
	{
		FxSpatialFrame* effectFrame = nullptr;

		const int runFlags = elemDef->flags & FX_ELEM_RUN_RELATIVE_TO_OFFSET;
		if (runFlags == FX_ELEM_RUN_RELATIVE_TO_SPAWN)
		{
			effectFrame = &effect->frameAtSpawn;
		}
		else
		{
			effectFrame = effectFrameWhenPlayed;
			if (runFlags == FX_ELEM_RUN_RELATIVE_TO_OFFSET)
			{
				outOrigin[0] = 0.0f;
				outOrigin[1] = 0.0f;
				outOrigin[2] = 0.0f;

				return;
			}
		}

		float effectFrameAxis[3][3] = {};

		UnitQuatToAxis(effectFrame->quat, effectFrameAxis);
		FX_GetSpawnOrigin(effectFrame, elemDef, randomSeed, outOrigin);
		FX_OffsetSpawnOrigin(effectFrame, elemDef, randomSeed, outOrigin);

		if (runFlags == FX_ELEM_RUN_RELATIVE_TO_EFFECT || runFlags == FX_ELEM_RUN_RELATIVE_TO_SPAWN)
		{
			const float delta[3] =
			{
				outOrigin[0] - effectFrame->origin[0],
				outOrigin[1] - effectFrame->origin[1],
				outOrigin[2] - effectFrame->origin[2]
			};

			outOrigin[0] = effectFrameAxis[0][0] * delta[0] + effectFrameAxis[0][1] * delta[1] + effectFrameAxis[0][2] * delta[2];
			outOrigin[1] = effectFrameAxis[1][0] * delta[0] + effectFrameAxis[1][1] * delta[1] + effectFrameAxis[1][2] * delta[2];
			outOrigin[2] = effectFrameAxis[2][0] * delta[0] + effectFrameAxis[2][1] * delta[1] + effectFrameAxis[2][2] * delta[2];
		}
	}

	// checked
	void FX_GetOriginForTrailElem([[maybe_unused]] FxEffect* effect, FxElemDef* elemDef, FxSpatialFrame* effectFrameWhenPlayed, int randomSeed, float* outOrigin, float* outRight, float* outUp)
	{

		if (!outRight || !outUp)
		{
			Assert();
		}

		// todo: limit selection to world only
		if ((elemDef->flags & FX_ELEM_RUN_MASK) != FX_ELEM_RUN_RELATIVE_TO_WORLD)
		{
			Assert();
		}

		float effectFrameAxis[3][3];
		UnitQuatToAxis(effectFrameWhenPlayed->quat, effectFrameAxis); //utils::hook::call<void(__cdecl)(const float *, float (*)[3])>(0x433A50)(effectFrameWhenPlayed->quat, effectFrameAxis);
		

		outRight[0] = effectFrameAxis[1][0];
		outRight[1] = effectFrameAxis[1][1];
		outRight[2] = effectFrameAxis[1][2];
		outUp[0] = effectFrameAxis[2][0];
		outUp[1] = effectFrameAxis[2][1];
		outUp[2] = effectFrameAxis[2][2];

		FX_GetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin); //utils::hook::call<void(__cdecl)(FxSpatialFrame*, FxElemDef*, int, float*)>(0x489280)(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin);
		FX_OffsetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin); //utils::hook::call<void(__cdecl)(FxSpatialFrame*, FxElemDef*, int, float*)>(0x489120)(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin);
	}

	// checked
	void FX_GetSpawnOrigin(FxSpatialFrame* frameAtSpawn, FxElemDef* elemDef, int randomSeed, float* spawnOrigin)
	{
		float offset[3];
		offset[0] = fx_randomTable[6 + randomSeed] * elemDef->spawnOrigin[0].amplitude + elemDef->spawnOrigin[0].base;
		offset[1] = fx_randomTable[7 + randomSeed] * elemDef->spawnOrigin[1].amplitude + elemDef->spawnOrigin[1].base;
		offset[2] = fx_randomTable[8 + randomSeed] * elemDef->spawnOrigin[2].amplitude + elemDef->spawnOrigin[2].base;

		if ((elemDef->flags & FX_ELEM_SPAWN_RELATIVE_TO_EFFECT) == 0)
		{
			spawnOrigin[0] = frameAtSpawn->origin[0] + offset[0];
			spawnOrigin[1] = frameAtSpawn->origin[1] + offset[1];
			spawnOrigin[2] = frameAtSpawn->origin[2] + offset[2];
		}
		else
		{
			FX_TransformPosFromLocalToWorld(frameAtSpawn, offset, spawnOrigin);
		}
	}

	// checked
	void FX_OffsetSpawnOrigin(FxSpatialFrame* effectFrame, FxElemDef* elemDef, int randomSeed, float* spawnOrigin)
	{
		if ((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) != FX_ELEM_SPAWN_OFFSET_NONE)
		{
			if ((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) == FX_ELEM_SPAWN_OFFSET_SPHERE)
			{
				float dir[3];
				FX_RandomDir(randomSeed, dir);

				const float radius = (fx_randomTable[11 + randomSeed] * elemDef->spawnOffsetRadius.amplitude) + elemDef->spawnOffsetRadius.base;
				spawnOrigin[0] = (radius * dir[0]) + spawnOrigin[0];
				spawnOrigin[1] = (radius * dir[1]) + spawnOrigin[1];
				spawnOrigin[2] = (radius * dir[2]) + spawnOrigin[2];
			}
			else
			{
				if ((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) != FX_ELEM_SPAWN_OFFSET_CYLINDER)
				{
					Assert();
				}

				float axis[3][3];
				UnitQuatToAxis(effectFrame->quat, axis);

				const float yaw = fx_randomTable[9 + randomSeed] * 6.2831855f;
				const float cosYaw = cos(yaw);
				const float sinYaw = sin(yaw);

				const float radius = (fx_randomTable[11 + randomSeed] * elemDef->spawnOffsetRadius.amplitude) + elemDef->spawnOffsetRadius.base;
				const float cosRadius = radius * cosYaw;
				const float sinRadius = radius * sinYaw;

				spawnOrigin[0] = ((cosRadius * axis[1][0]) + spawnOrigin[0]) + (sinRadius * axis[2][0]);
				spawnOrigin[1] = ((cosRadius * axis[1][1]) + spawnOrigin[1]) + (sinRadius * axis[2][1]);
				spawnOrigin[2] = ((cosRadius * axis[1][2]) + spawnOrigin[2]) + (sinRadius * axis[2][2]);

				const float height = (fx_randomTable[10 + randomSeed] * elemDef->spawnOffsetHeight.amplitude) + elemDef->spawnOffsetHeight.base;
				spawnOrigin[0] = (height * axis[0][0]) + spawnOrigin[0];
				spawnOrigin[1] = (height * axis[0][1]) + spawnOrigin[1];
				spawnOrigin[2] = (height * axis[0][2]) + spawnOrigin[2];
			}
		}
	}

	void FX_OrientationPosFromWorldPos(game::orientation_t* orient, const float* pos, float* out)
	{
		if (pos == out)
		{
			Assert();
		}

		const float p0 = pos[0] - orient->origin[0];
		const float p1 = pos[1] - orient->origin[1];
		const float p2 = pos[2] - orient->origin[2];

		out[0] = orient->axis[0][0] * p0 + orient->axis[0][1] * p1 + orient->axis[0][2] * p2;
		out[1] = orient->axis[1][0] * p0 + orient->axis[1][1] * p1 + orient->axis[1][2] * p2;
		out[2] = orient->axis[2][0] * p0 + orient->axis[2][1] * p1 + orient->axis[2][2] * p2;
	}

	void FX_OrientationPosToWorldPos(game::orientation_t* orient, const float* pos, float* out)
	{
		if (pos == out)
		{
			Assert();
		}

		out[0] = orient->axis[0][0] * pos[0] + orient->origin[0] + orient->axis[1][0] * pos[1] + orient->axis[2][0] * pos[2];
		out[1] = orient->axis[0][1] * pos[0] + orient->origin[1] + orient->axis[1][1] * pos[1] + orient->axis[2][1] * pos[2];
		out[2] = orient->axis[0][2] * pos[0] + orient->origin[2] + orient->axis[1][2] * pos[1] + orient->axis[2][2] * pos[2];
	}

	void FX_OrientationDirToWorldDir(game::orientation_t* orient, const float* dir, float* out)
	{
		if (dir == out)
		{
			Assert();
		}

		out[0] = orient->axis[1][0] * dir[1] + orient->axis[0][0] * dir[0] + orient->axis[2][0] * dir[2];
		out[1] = orient->axis[1][1] * dir[1] + orient->axis[0][1] * dir[0] + orient->axis[2][1] * dir[2];
		out[2] = orient->axis[1][2] * dir[1] + orient->axis[0][2] * dir[0] + orient->axis[2][2] * dir[2];
	}

	void FX_GetOrientation(FxElemDef* elemDef, FxSpatialFrame* frameAtSpawn, FxSpatialFrame* frameNow, int randomSeed, game::orientation_t* orient)
	{
		if (!Vec4IsNormalized(frameAtSpawn->quat) || !Vec4IsNormalized(frameNow->quat))
		{
			Assert();
		}

		int runFlags = elemDef->flags & FX_ELEM_RUN_RELATIVE_TO_OFFSET;
		if (runFlags)
		{
			if (runFlags == FX_ELEM_RUN_RELATIVE_TO_SPAWN)
			{
				FX_SpatialFrameToOrientation(frameAtSpawn, orient);
			}
			else if (runFlags == FX_ELEM_RUN_RELATIVE_TO_EFFECT)
			{
				FX_SpatialFrameToOrientation(frameNow, orient);
			}
			else
			{
				if (runFlags != FX_ELEM_RUN_RELATIVE_TO_OFFSET)
				{
					Assert();
				}

				if ((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) == FX_ELEM_SPAWN_OFFSET_NONE)
				{
					Assert();
				}

				FX_GetSpawnOrigin(frameAtSpawn, elemDef, randomSeed, orient->origin);
				orient->axis[0][0] = 0.0;
				orient->axis[0][1] = 0.0;
				orient->axis[0][2] = 0.0;

				FX_OffsetSpawnOrigin(frameAtSpawn, elemDef, randomSeed, (float*)orient->axis);
				orient->origin[0] = orient->axis[0][0] + orient->origin[0];
				orient->origin[1] = orient->axis[0][1] + orient->origin[1];
				orient->origin[2] = orient->axis[0][2] + orient->origin[2];

				if (Vec3Normalize((float*)orient->axis) == 0.0f)
				{
					orient->axis[0][0] = 1.0;
					orient->axis[0][1] = 0.0;
					orient->axis[0][2] = 0.0;
				}

				if ((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) == FX_ELEM_SPAWN_OFFSET_SPHERE)
				{
					Vec3Basis_RightHanded(orient->axis[0], orient->axis[1], orient->axis[2]);
				}
				else
				{
					if ((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) != FX_ELEM_SPAWN_OFFSET_CYLINDER)
					{
						Assert();
					}

					float up[3] = {};

					if (fabs(orient->axis[0][2]) < 0.9990000128746033f)
					{
						up[1] = 0.0f;
						up[2] = 1.0f;
					}
					else
					{
						up[1] = 1.0f;
						up[2] = 0.0f;
						
					}

					Vec3Cross(up, (float*)orient->axis, orient->axis[1]);
					Vec3Normalize(orient->axis[1]);
					Vec3Cross(orient->axis[0], orient->axis[1], orient->axis[2]);
				}
			}
		}
		else
		{
			orient->origin[0] = 0.0f;
			orient->origin[1] = 0.0f;
			orient->origin[2] = 0.0f;

			orient->axis[0][0] = 1.0f;
			orient->axis[0][1] = 0.0f;
			orient->axis[0][2] = 0.0f;
			orient->axis[1][0] = 0.0f;
			orient->axis[1][1] = 1.0f;
			orient->axis[1][2] = 0.0f;
			orient->axis[2][0] = 0.0f;
			orient->axis[2][1] = 0.0f;
			orient->axis[2][2] = 1.0f;
		}
	}
}