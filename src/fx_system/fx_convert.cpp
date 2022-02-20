#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

// #ENV_DEPENDENT
#ifdef FXEDITOR
#define Warning(unused, fmt, ...)	if(IsDebuggerPresent()) __debugbreak();	else {\
											game::allow_warnings = true; \
											game::Com_PrintError(unused, fmt, __VA_ARGS__); \
											game::allow_warnings = false; }
#else
#define Warning(unused, fmt, ...)	if(IsDebuggerPresent()) __debugbreak(); \
											game::printf_to_console(fmt, __VA_ARGS__);
#endif

#define SLODWORD(x)  (*((int*)&(x)))

namespace fx_system
{
	bool FX_ElemUsesMaterial(FxEditorElemDef* elemDef)
	{
		return static_cast<int>(elemDef->elemType) < FX_ELEM_TYPE_MODEL || static_cast<int>(elemDef->elemType) > FX_ELEM_TYPE_SOUND && static_cast<int>(elemDef->elemType) != FX_ELEM_TYPE_RUNNER;
	}

	bool FX_ValidateAtlasSettings(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef)
	{
		if (!edElemDef)
		{
			Assert();
		}
	
		if (!edElemDef->visualCount || !FX_ElemUsesMaterial(edElemDef))
		{
			return true;
		}

		game::MaterialInfo mtlInfoRef = {};
		game::MaterialInfo mtlInfo = {};

		Material_GetInfo(edElemDef->u.visuals[0].material, &mtlInfoRef);
		if (static_cast<std::uint8_t>(mtlInfoRef.textureAtlasColumnCount) * static_cast<std::uint8_t>(mtlInfoRef.textureAtlasRowCount) <= 256)
		{
			if ((static_cast<std::uint8_t>(mtlInfoRef.textureAtlasRowCount - 1) & static_cast<std::uint8_t>(mtlInfoRef.textureAtlasRowCount)) != 0 || 
				(static_cast<std::uint8_t>(mtlInfoRef.textureAtlasColumnCount - 1) & static_cast<std::uint8_t>(mtlInfoRef.textureAtlasColumnCount)) != 0)
			{
				Warning(21, "effect '%s' segment '%s':\nmaterial %s is a %i x %i atlas, which is not a power of 2 on both axes\n", 
					editorEffect->name, 
					edElemDef->name, 
					mtlInfoRef.name, 
					static_cast<std::uint8_t>(mtlInfoRef.textureAtlasColumnCount), 
					static_cast<std::uint8_t>(mtlInfoRef.textureAtlasRowCount));

				return false;
			}

			if (edElemDef->visualCount <= 1)
			{
				return true;
			}
			
			for (int visualIndex = 1; visualIndex < edElemDef->visualCount; ++visualIndex)
			{
				Material_GetInfo(edElemDef->u.visuals[visualIndex].material, &mtlInfo);
				if (mtlInfo.textureAtlasRowCount != mtlInfoRef.textureAtlasRowCount || mtlInfo.textureAtlasColumnCount != mtlInfoRef.textureAtlasColumnCount)
				{
					Warning(21, "effect '%s' segment '%s':\nmaterial %s is a %i x %i atlas, but material %s is a %i x %i atlas\n", 
						editorEffect->name, 
						edElemDef->name, 
						mtlInfoRef.name, 
						static_cast<std::uint8_t>(mtlInfoRef.textureAtlasColumnCount), 
						static_cast<std::uint8_t>(mtlInfoRef.textureAtlasRowCount), 
						mtlInfo.name, 
						static_cast<std::uint8_t>(mtlInfo.textureAtlasColumnCount), 
						static_cast<std::uint8_t>(mtlInfo.textureAtlasRowCount));

					return false;
				}
			}
	
			return true;
			
		}
	
		Warning(21, "effect '%s' segment '%s':\nmaterial %s is a %i x %i atlas, which exceeds 256 max frames per atlas\n", 
			editorEffect->name, 
			edElemDef->name, 
			mtlInfoRef.name, 
			static_cast<std::uint8_t>(mtlInfoRef.textureAtlasColumnCount),
			static_cast<std::uint8_t>(mtlInfoRef.textureAtlasRowCount));

		return false;
	}

	bool FX_ValidateFlags(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef)
	{
		if ((edElemDef->flags & 0xF0) != FX_ELEM_RUN_RELATIVE_TO_OFFSET)
		{
			return true;
		}

		Warning(21, "effect '%s' segment '%s'\nVelocity is 'relative to offset', but generation offset is 'none'\n", editorEffect->name, edElemDef->name);
		return false;
	}

	bool FX_ValidateColor(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef)
	{
		if (edElemDef->elemType == FX_ELEM_TYPE_DECAL)
		{
			if (edElemDef->lightingFrac != 0.0f)
			{
				Warning(21, "effect '%s' segment '%s'\nDecals cannot have a non-zero lighting fraction.\n", editorEffect->name, edElemDef->name);
				return false;
			}
		}
		else
		{
			if (edElemDef->lightingFrac < 0.0f)
			{
				Warning(21, "effect '%s' segment '%s'\nNegative lighting fraction.\n", editorEffect->name, edElemDef->name);
				return false;
			}

			if (edElemDef->lightingFrac > 1.0f)
			{
				Warning(21, "effect '%s' segment '%s'\nLighting fraction larger than 1.0.\n", editorEffect->name, edElemDef->name);
				return false;
			}
		}

		return true;
	}

	bool FX_ValidateVisuals(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef)
	{
		if ((edElemDef->elemType == FX_ELEM_TYPE_DECAL || edElemDef->elemType == FX_ELEM_TYPE_RUNNER) && !edElemDef->visualCount)
		{
			Warning(21, "effect '%s' segment '%s'\nThis type of segment must have at least one visual specified.\n", editorEffect->name, edElemDef->name);
			return false;
		}

		if (edElemDef->elemType == FX_ELEM_TYPE_TRAIL)
		{
			if (!edElemDef->trailDef.indCount || !edElemDef->trailDef.vertCount)
			{
				Warning(21, "effect '%s' segment '%s'\nTrail cross-section cannot be empty.\n", editorEffect->name, edElemDef->name);
				return false;
			}
			for (int indIter = 0; indIter != edElemDef->trailDef.indCount; ++indIter)
			{
				if (edElemDef->trailDef.inds[indIter] >= edElemDef->trailDef.vertCount)
				{
					Warning(21, "effect '%s' segment '%s'\nIndex references out of range vertex '%i'.\n", editorEffect->name, edElemDef->name, edElemDef->trailDef.inds[indIter]);
					return false;
				}
			}

			if (edElemDef->trailRepeatDist <= 0)
			{
				Warning(21, "effect '%s' segment '%s'\nTrail repeat dist <= 0.\n", editorEffect->name, edElemDef->name);
				return false;
			}

			if (edElemDef->trailSplitDist <= 0)
			{
				Warning(21, "effect '%s' segment '%s'\nTrail split dist <= 0.\n", editorEffect->name, edElemDef->name);
				return false;
			}

			if (static_cast<int>( static_cast<float>(edElemDef->trailRepeatDist) * 1000.0f ) <= 0)
			{
				Warning(21, "effect '%s' segment '%s'\nTrail texture repeat dist too close to, or below 0.\n", editorEffect->name, edElemDef->name);
				return false;
			}
		}

		return true;
	}

	bool FX_ValidatePhysics(FxEditorEffectDef* editorEffect, FxEditorElemDef* edElemDef)
	{
		if ((edElemDef->flags & FX_ELEM_USE_COLLISION) == 0)
		{
			return true;
		}

		const float elasticityMin = edElemDef->elasticity.base + (edElemDef->elasticity.amplitude < 0.0f ? edElemDef->elasticity.amplitude : 0.0f);
		const float elasticityMax = edElemDef->elasticity.base + (edElemDef->elasticity.amplitude < 0.0f ? edElemDef->elasticity.amplitude : 0.0f);

		if (elasticityMin >= -0.001f && elasticityMax <= 1.001f)
		{
			return true;
		}

		Warning(21, "effect '%s' segment '%s'\nElasticity %.3f to %.3f can go outside the range 0 to 1.\n", editorEffect->name, edElemDef->name, elasticityMin, elasticityMax);
		return false;
	}

	bool FX_Validate(FxEditorElemDef* edElemDef, FxEditorEffectDef* editorEffect)
	{
		if (!FX_ValidateAtlasSettings(editorEffect, edElemDef))
		{
			return false;
		}

		if (!FX_ValidateFlags(editorEffect, edElemDef))
		{
			return false;
		}

		if (!FX_ValidateColor(editorEffect, edElemDef))
		{
			return false;
		}

		if (FX_ValidateVisuals(editorEffect, edElemDef))
		{
			return FX_ValidatePhysics(editorEffect, edElemDef);
		}

		return false;
	}

	void FX_InterpolateSamples(int dimensions, float time0, const float* samples0, float time1, const float* samples1, float timeEval, float* result)
	{
		if (dimensions <= 0 || time1 <= time0)
		{
			Assert();
		}

		if (timeEval < time0 || time1 < timeEval)
		{
			Assert();
		}

		if (!samples0 || !samples1 || !result)
		{
			Assert();
		}

		for (int dimIndex = 0; dimIndex < dimensions; ++dimIndex)
		{
			result[dimIndex] = ((samples1[dimIndex] - samples0[dimIndex]) * ((timeEval - time0) / (time1 - time0))) + samples0[dimIndex];
		}
	}

	float FX_MaxErrorForIntervalCount(int dimensions, int sampleCount, const float* samples, int intervalCount, float errorCutoff)
	{
		if (dimensions <= 0 || sampleCount <= 1 || !samples)
		{
			Assert();
		}

		if (intervalCount < 1)
		{
			return 0.0f;
		}

		const int componentCount = dimensions + 1;
		float errorMax = 0.0f;
		float timePrev = 0.0f;

		int sampleIndexNext = 0;
		int sampleIndexPrev = 1;

		float timeNext;
		float lerpedValuePrev[3];

		if (dimensions)
		{
			memcpy(lerpedValuePrev, samples + 1, 4 * dimensions);
		}

		for (int intervalIndex = 1; intervalIndex <= intervalCount; ++intervalIndex)
		{
			const auto flt_intervalIndex = static_cast<float>(intervalIndex);
			const auto flt_invervalCount = static_cast<float>(intervalCount);

			for (sampleIndexNext = sampleIndexPrev; flt_intervalIndex > (flt_invervalCount * samples[componentCount * sampleIndexNext]); 
				++sampleIndexNext)
			{ }

			timeNext = flt_intervalIndex / flt_invervalCount;
			const float* samplesTo = &samples[componentCount * sampleIndexNext];
			const float* samplesFrom = &samplesTo[-componentCount];

			float lerpedValueNext[3] = {};
			FX_InterpolateSamples(dimensions, *samplesFrom, samplesFrom + 1, *samplesTo, samplesTo + 1, timeNext, lerpedValueNext);

			for (int sampleIndexIter = sampleIndexPrev; sampleIndexIter < sampleIndexNext; ++sampleIndexIter)
			{
				float lerpedValueIter[3] = {};
				FX_InterpolateSamples(dimensions, timePrev, lerpedValuePrev, timeNext, lerpedValueNext, samples[componentCount * sampleIndexIter], lerpedValueIter);

				for (int componentIndex = 1; componentIndex < componentCount; ++componentIndex)
				{
					const float error = fabs(samples[componentIndex + componentCount * sampleIndexIter] - lerpedValueIter[componentIndex - 1]); // lerpedValueIter[componentIndex] :: -1 !
					if (error > errorMax)
					{
						errorMax = error;

						if (error > errorCutoff)
						{
							return errorMax;
						}
					}
				}
			}

			if (dimensions)
			{
				memcpy(lerpedValuePrev, lerpedValueNext, 4 * dimensions);
			}

			sampleIndexPrev = sampleIndexNext;
			timePrev = timeNext;
		}

		return errorMax;
	}

	void FX_ReserveElemDefMemory(FxElemDef* elemDef, char** memPool)
	{
		if (!elemDef || !memPool || !*memPool || !elemDef->velIntervalCount)
		{
			Assert();
		}

		elemDef->velSamples = (FxElemVelStateSample*)*memPool;
		*memPool += sizeof(FxElemVelStateSample) * static_cast<std::uint8_t>(elemDef->velIntervalCount) + sizeof(FxElemVelStateSample);

		if (elemDef->visStateIntervalCount)
		{
			elemDef->visSamples = (FxElemVisStateSample*)*memPool;
			*memPool += sizeof(FxElemVisStateSample) * elemDef->visStateIntervalCount + sizeof(FxElemVisStateSample);
		}
		else
		{
			elemDef->visSamples = nullptr;
		}

		if (elemDef->elemType == FX_ELEM_TYPE_DECAL)
		{
			elemDef->visuals.markArray = (FxElemMarkVisuals*)*memPool;
			*memPool += sizeof(FxElemMarkVisuals) * static_cast<std::uint8_t>(elemDef->visualCount);
		}
		else
		{
			if (static_cast<std::uint8_t>(elemDef->visualCount) > 1u)
			{
				elemDef->visuals.markArray = (FxElemMarkVisuals*)*memPool;
				*memPool += 4 * elemDef->visualCount;
			}
		}
	}

	int FX_AdditionalBytesNeededForElemDef(int visStateSampleCount, int velStateSampleCount, int visualCount, char elemType)
	{
		int bytesNeeded = 0x30 * (visStateSampleCount + 2 * velStateSampleCount);

		if (elemType == FX_ELEM_TYPE_DECAL)
		{
			bytesNeeded += 8 * visualCount;
		}
		else if (visualCount > 1)
		{
			bytesNeeded += 4 * visualCount;
		}

		return bytesNeeded;
	}

	int FX_AdditionalBytesNeededForGeomTrail(FxEditorElemDef* edElemDef)
	{
		if (edElemDef->elemType == FX_ELEM_TYPE_TRAIL)
		{
			return 0x16 * edElemDef->trailDef.indCount + 0x1C;
		}
		
		return 0;
	}

	int FX_AdditionalBytesNeededForEmission(FxEffectDef* emission)
	{
		if (!emission)
		{
			__debugbreak();
		}

		int bytesNeeded = emission->elemDefCountOneShot * sizeof(FxElemDef);
		int elemDefStop = emission->elemDefCountOneShot + emission->elemDefCountLooping;

		for (int elemDefIndex = emission->elemDefCountLooping; elemDefIndex != elemDefStop; ++elemDefIndex)
		{
			FxElemDef* elemDef = &emission->elemDefs[elemDefIndex];

			bytesNeeded += FX_AdditionalBytesNeededForElemDef(
				elemDef->visStateIntervalCount ? static_cast<std::uint8_t>(elemDef->visStateIntervalCount) + 1 : 0,
				static_cast<std::uint8_t>(elemDef->velIntervalCount) + 1,
				static_cast<std::uint8_t>(elemDef->visualCount),
				elemDef->elemType);
		}

		return bytesNeeded;
	}

	int FX_FindEmission(FxEditorEffectDef* editorEffect, FxEffectDef* emission)
	{
		if (!editorEffect || !editorEffect->elemCount)
		{
			Assert();
		}


		for (int elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex)
		{
			if (emission == editorEffect->elems[elemIndex].emission || (editorEffect->elems[elemIndex].editorFlags & FX_ED_FLAG_DISABLED) == 0) // correct flag check?
			{
				return elemIndex;
			}
		}

		Assert();

		return -1;
	}

	FxSampleChannel* FX_GetVisualSampleRouting(FxEditorElemDef* edElem, FxSampleChannel* routing)
	{
		switch (edElem->elemType)
		{
		case FX_ELEM_TYPE_SPRITE_BILLBOARD:
		case FX_ELEM_TYPE_SPRITE_ORIENTED:
		case FX_ELEM_TYPE_TAIL:
		case FX_ELEM_TYPE_TRAIL:
			routing[0] = FX_CHAN_RGBA;
			routing[1] = FX_CHAN_SIZE_0;
			routing[2] = FX_CHAN_SIZE_0;

			if (edElem->elemType == FX_ELEM_TYPE_TAIL || (edElem->flags & FX_ELEM_NONUNIFORM_SCALE) != 0)
			{
				routing[2] = FX_CHAN_SIZE_1;
			}

			routing[3] = FX_CHAN_NONE;
			routing[4] = FX_CHAN_ROTATION;

			break;

		case FX_ELEM_TYPE_CLOUD:
			routing[0] = FX_CHAN_RGBA;
			routing[1] = FX_CHAN_SIZE_0;
			routing[2] = FX_CHAN_SIZE_1;
			routing[3] = FX_CHAN_SCALE;
			routing[4] = FX_CHAN_NONE;
			break;

		case FX_ELEM_TYPE_MODEL:
		case FX_ELEM_TYPE_SOUND:
			routing[0] = FX_CHAN_NONE;
			routing[1] = FX_CHAN_NONE;
			routing[2] = FX_CHAN_NONE;
			routing[3] = FX_CHAN_SCALE;
			routing[4] = FX_CHAN_NONE;
			break;

		case FX_ELEM_TYPE_OMNI_LIGHT:
			routing[0] = FX_CHAN_RGBA;
			routing[1] = FX_CHAN_SIZE_0;
			routing[2] = FX_CHAN_NONE;
			routing[3] = FX_CHAN_NONE;
			routing[4] = FX_CHAN_NONE;
			break;

		case FX_ELEM_TYPE_SPOT_LIGHT:
		case FX_ELEM_TYPE_DECAL:
			routing[0] = FX_CHAN_RGBA;
			routing[1] = FX_CHAN_SIZE_0;
			routing[2] = FX_CHAN_NONE;
			routing[3] = FX_CHAN_NONE;
			routing[4] = FX_CHAN_ROTATION;
			break;

		case FX_ELEM_TYPE_RUNNER:
			routing[0] = FX_CHAN_NONE;
			routing[1] = FX_CHAN_NONE;
			routing[2] = FX_CHAN_NONE;
			routing[3] = FX_CHAN_NONE;
			routing[4] = FX_CHAN_NONE;
			break;

		default:
			return routing;
		}
		return routing;
	}

	int FX_DecideIntervalLimit(FxEditorElemDef* edElemDef)
	{
		int intervalLimit = (edElemDef->lifeSpanMsec.base + edElemDef->lifeSpanMsec.amplitude / 2) / 100;
		if (intervalLimit > 80)
		{
			intervalLimit = 80;
		}

		return intervalLimit;
	}

	// should be good
	int FX_DecideSampleCount(int curveCount, FxCurve** curves, int intervalLimit)
	{
		if (intervalLimit < 1)
		{
			return 2;
		}

		float errorBest = 3.4028235e38f;
		int intervalCountBest = 1;

		for (int intervalCount = 1; intervalCount <= intervalLimit; ++intervalCount)
		{
			float errorCumulative = 0.0f;
			for (int curveIndex = 0; curveIndex < curveCount; ++curveIndex)
			{
				const float error = FX_MaxErrorForIntervalCount(curves[curveIndex]->dimensionCount, curves[curveIndex]->keyCount, curves[curveIndex]->keys, intervalCount, errorBest);
				if (error > errorCumulative)
				{
					errorCumulative = error;
					if (error > errorBest)
					{
						break;
					}
				}
			}

			if (errorBest > errorCumulative)
			{
				intervalCountBest = intervalCount;
				errorBest = errorCumulative - 0.02f;

				if (errorCumulative - 0.02f <= 0.0f)
				{
					break;
				}
			}
		}

		return intervalCountBest + 1;
	}

	int FX_DecideVelocitySampleCount(FxEditorElemDef* edElem, int intervalLimit)
	{
		FxCurve* curves[12];

		curves[0] = edElem->velShape[0][0][0];
		curves[1] = edElem->velShape[0][0][1];
		curves[2] = edElem->velShape[0][1][0];
		curves[3] = edElem->velShape[0][1][1];
		curves[4] = edElem->velShape[0][2][0];
		curves[5] = edElem->velShape[0][2][1];
		curves[6] = edElem->velShape[1][0][0];
		curves[7] = edElem->velShape[1][0][1];
		curves[8] = edElem->velShape[1][1][0];
		curves[9] = edElem->velShape[1][1][1];
		curves[10] = edElem->velShape[1][2][0];
		curves[11] = edElem->velShape[1][2][1];

		return FX_DecideSampleCount(12, curves, intervalLimit);
	}

	int FX_DecideVisualSampleCount(FxEditorElemDef* edElem, FxSampleChannel* routing, int intervalLimit)
	{
		FxCurve* curves[12]; 
		int curveCount = 0;
		int cc = 0;

		for (unsigned int chanIndex = 0; chanIndex < 5; ++chanIndex)
		{
			switch (routing[chanIndex])
			{
			case FX_CHAN_RGBA:
				curves[curveCount] = edElem->color[0];
				cc = curveCount + 1;
				if ((edElem->editorFlags & FX_ED_FLAG_USE_RANDOM_COLOR) != 0)
				{
					curves[cc++] = edElem->color[1];
				}
				curves[cc] = edElem->alpha[0];
				curveCount = cc + 1;

				if ((edElem->editorFlags & FX_ED_FLAG_USE_RANDOM_ALPHA) != 0)
				{
					curves[curveCount++] = edElem->alpha[1];
				}
				break;

			case FX_CHAN_SIZE_0:
				curves[curveCount++] = edElem->sizeShape[0][0];
				if ((edElem->editorFlags & FX_ED_FLAG_USE_RANDOM_SIZE_0) != 0)
				{
					curves[curveCount++] = edElem->sizeShape[0][1];
				}
				break;

			case FX_CHAN_SIZE_1:
				curves[curveCount++] = edElem->sizeShape[1][0];
				if ((edElem->editorFlags & FX_ED_FLAG_USE_RANDOM_SIZE_1) != 0)
				{
					curves[curveCount++] = edElem->sizeShape[1][1];
				}
				break;

			case FX_CHAN_SCALE:
				curves[curveCount++] = edElem->scaleShape[0];
				if ((edElem->editorFlags & FX_ED_FLAG_USE_RANDOM_SCALE) != 0)
				{
					curves[curveCount++] = edElem->scaleShape[1];
				}
				break;

			case FX_CHAN_ROTATION:
				curves[curveCount++] = edElem->rotationShape[0];
				if ((edElem->editorFlags & FX_ED_FLAG_USE_RANDOM_ROTATION_DELTA) != 0)
				{
					curves[curveCount++] = edElem->rotationShape[1];
				}
				break;

			default:
				if (routing[chanIndex] != FX_CHAN_NONE)
				{
					Assert();
				}
				break;
			}
		}

		if (curveCount > 12)
		{
			Assert();
		}

		if (curveCount)
		{
			return FX_DecideSampleCount(curveCount, curves, intervalLimit);
		}

		return 0;
	}

	void FX_SampleVelocityInFrame(FxElemVelStateInFrame* velState, int velStateStride, FxEditorElemDef* edElemDef, FxElemDef* elemDef, const float(*velScale)[3], int useGraphBit)
	{
		const bool useVelocity[2] =
		{
			(edElemDef->editorFlags & FX_ED_FLAG_ABSOLUTE_VELOCITY_0) == (useGraphBit != FX_ELEM_HAS_VELOCITY_GRAPH_WORLD ? 0 : FX_ED_FLAG_ABSOLUTE_VELOCITY_0),
			(edElemDef->editorFlags & FX_ED_FLAG_ABSOLUTE_VELOCITY_1) == (useGraphBit != FX_ELEM_HAS_VELOCITY_GRAPH_WORLD ? 0 : FX_ED_FLAG_ABSOLUTE_VELOCITY_1)
		};

		const bool useVelocityRand[2] =
		{
			useVelocity[0] && (edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_VELOCITY_0) != 0,
			useVelocity[1] && (edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_VELOCITY_1) != 0
		};

		const bool brokenCompatibilityMode = (edElemDef->editorFlags & FX_ED_FLAG_BACKCOMPAT_VELOCITY) != 0;
		float velEpsilonSq = 0.0f;

		if (useVelocity[0])
		{
			velEpsilonSq = Vec3LengthSq(&(*velScale)[0]);
		}

		if (useVelocity[1])
		{
			velEpsilonSq = Vec3LengthSq(&(*velScale)[3]) + velEpsilonSq;
		}

		bool anyNonZero = false;
		FxElemVelStateInFrame* velStatePrev = nullptr;

		for (int sampleIndex = 0; sampleIndex <= static_cast<std::uint8_t>( elemDef->velIntervalCount ); ++sampleIndex)
		{
			float sampleTime = static_cast<float>(sampleIndex) / static_cast<float>( static_cast<std::uint8_t>(elemDef->velIntervalCount) );

			velState->velocity.base[0] = 0.0f;
			velState->velocity.base[1] = 0.0f;
			velState->velocity.base[2] = 0.0f;
			velState->velocity.amplitude[0] = 0.0f;
			velState->velocity.amplitude[1] = 0.0f;
			velState->velocity.amplitude[2] = 0.0f;

			if (useVelocity[0])
			{
				float velocitySample[3];
				velocitySample[0] = FX_SampleCurve1D(edElemDef->velShape[0][0][0], (*velScale)[0], sampleTime);
				velocitySample[1] = FX_SampleCurve1D(edElemDef->velShape[0][1][0], (*velScale)[1], sampleTime);
				velocitySample[2] = FX_SampleCurve1D(edElemDef->velShape[0][2][0], (*velScale)[2], sampleTime);

				velState->velocity.base[0] = velState->velocity.base[0] + velocitySample[0];
				velState->velocity.base[1] = velState->velocity.base[1] + velocitySample[1];
				velState->velocity.base[2] = velState->velocity.base[2] + velocitySample[2];

				if (useVelocityRand[0])
				{
					velState->velocity.amplitude[0] = velState->velocity.amplitude[0] - velocitySample[0];
					velState->velocity.amplitude[1] = velState->velocity.amplitude[1] - velocitySample[1];
					velState->velocity.amplitude[2] = velState->velocity.amplitude[2] - velocitySample[2];
				}
			}

			if (useVelocity[1])
			{
				float velocitySample[3];
				velocitySample[0] = FX_SampleCurve1D(edElemDef->velShape[1][0][0], (*velScale)[3], sampleTime);
				velocitySample[1] = FX_SampleCurve1D(edElemDef->velShape[1][1][0], (*velScale)[4], sampleTime);
				velocitySample[2] = FX_SampleCurve1D(edElemDef->velShape[1][2][0], (*velScale)[5], sampleTime);
				velState->velocity.base[0] = velState->velocity.base[0] + velocitySample[0];
				velState->velocity.base[1] = velState->velocity.base[1] + velocitySample[1];
				velState->velocity.base[2] = velState->velocity.base[2] + velocitySample[2];

				if (useVelocityRand[!brokenCompatibilityMode])
				{
					velState->velocity.amplitude[0] = velState->velocity.amplitude[0] - velocitySample[0];
					velState->velocity.amplitude[1] = velState->velocity.amplitude[1] - velocitySample[1];
					velState->velocity.amplitude[2] = velState->velocity.amplitude[2] - velocitySample[2];
				}
			}

			if (velStatePrev)
			{
				float deltaInSegment[3];
				deltaInSegment[0] = (velStatePrev->velocity.base[0] + velState->velocity.base[0]) * 0.5f;
				deltaInSegment[1] = (velStatePrev->velocity.base[1] + velState->velocity.base[1]) * 0.5f;
				deltaInSegment[2] = (velStatePrev->velocity.base[2] + velState->velocity.base[2]) * 0.5f;

				velState->totalDelta.base[0] = deltaInSegment[0] + velStatePrev->totalDelta.base[0];
				velState->totalDelta.base[1] = deltaInSegment[1] +velStatePrev->totalDelta.base[1];
				velState->totalDelta.base[2] = deltaInSegment[2] +velStatePrev->totalDelta.base[2];

				if (!anyNonZero)
				{
					anyNonZero = Vec3LengthSq(velState->totalDelta.base) > velEpsilonSq * 0.000001f;
				}
			}
			else
			{
				velState->totalDelta.base[0] = 0.0f;
				velState->totalDelta.base[1] = 0.0f;
				velState->totalDelta.base[2] = 0.0f;
			}

			if (useVelocityRand[0])
			{
				velState->velocity.amplitude[0] = FX_SampleCurve1D(edElemDef->velShape[0][0][1], (*velScale)[0], sampleTime) + velState->velocity.amplitude[0];
				velState->velocity.amplitude[1] = FX_SampleCurve1D(edElemDef->velShape[0][1][1], (*velScale)[1], sampleTime) + velState->velocity.amplitude[1];
				velState->velocity.amplitude[2] = FX_SampleCurve1D(edElemDef->velShape[0][2][1], (*velScale)[2], sampleTime) + velState->velocity.amplitude[2];
			}

			if (useVelocityRand[1])
			{
				velState->velocity.amplitude[0] = FX_SampleCurve1D(edElemDef->velShape[1][0][1], (*velScale)[3], sampleTime) + velState->velocity.amplitude[0];
				velState->velocity.amplitude[1] = FX_SampleCurve1D(edElemDef->velShape[1][1][1], (*velScale)[4], sampleTime) + velState->velocity.amplitude[1];
				velState->velocity.amplitude[2] = FX_SampleCurve1D(edElemDef->velShape[1][2][1], (*velScale)[5], sampleTime) + velState->velocity.amplitude[2];
			}

			if (velStatePrev)
			{
				float deltaInSegment[3];
				deltaInSegment[0] = (velStatePrev->velocity.amplitude[0] + velState->velocity.amplitude[0]) * 0.5f;
				deltaInSegment[1] = (velStatePrev->velocity.amplitude[1] + velState->velocity.amplitude[1]) * 0.5f;
				deltaInSegment[2] = (velStatePrev->velocity.amplitude[2] + velState->velocity.amplitude[2]) * 0.5f;
				velState->totalDelta.amplitude[0] = deltaInSegment[0] + velStatePrev->totalDelta.amplitude[0];
				velState->totalDelta.amplitude[1] = deltaInSegment[1] + velStatePrev->totalDelta.amplitude[1];
				velState->totalDelta.amplitude[2] = deltaInSegment[2] + velStatePrev->totalDelta.amplitude[2];

				if (!anyNonZero)
				{
					anyNonZero = Vec3LengthSq(velState->totalDelta.amplitude) > velEpsilonSq * 0.000001f;
				}
			}
			else
			{
				velState->totalDelta.amplitude[0] = 0.0f;
				velState->totalDelta.amplitude[1] = 0.0f;
				velState->totalDelta.amplitude[2] = 0.0f;
			}

			velStatePrev = velState;
			velState += velStateStride;
		}

		if (anyNonZero)
		{
			elemDef->flags |= useGraphBit;
		}
	}

	void FX_SampleVelocity(FxEditorElemDef* edElemDef, FxElemDef* elemDef)
	{
		float velScale[2][3];
		const float interval = 1.0f / (static_cast<float>( static_cast<std::uint8_t>(elemDef->velIntervalCount) ) * 1000.0f);

		velScale[0][0] = interval * edElemDef->velScale[0][0];
		velScale[0][1] = interval * edElemDef->velScale[0][1];
		velScale[0][2] = interval * edElemDef->velScale[0][2];
		
		velScale[1][0] = interval * edElemDef->velScale[1][0];
		velScale[1][1] = interval * edElemDef->velScale[1][1];
		velScale[1][2] = interval * edElemDef->velScale[1][2];

		FX_SampleVelocityInFrame(&elemDef->velSamples->local, 2, edElemDef, elemDef, velScale, FX_ELEM_HAS_VELOCITY_GRAPH_LOCAL);
		FX_SampleVelocityInFrame(&elemDef->velSamples->world, 2, edElemDef, elemDef, velScale, FX_ELEM_HAS_VELOCITY_GRAPH_WORLD);
	}

	void FX_SampleVisualStateScalar(FxSampleChannel routing, float* amplitude, float* base, FxEditorElemDef* edElemDef, float sampleTime, float scaleFactor)
	{
		switch (routing)
		{
		case FX_CHAN_SIZE_0:
			*base = FX_SampleCurve1D(edElemDef->sizeShape[0][0], edElemDef->sizeScale[0] * scaleFactor, sampleTime);

			if ((edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_SIZE_0) != 0)
			{
				*amplitude = FX_SampleCurve1D(edElemDef->sizeShape[0][1], edElemDef->sizeScale[0] * scaleFactor, sampleTime) - *base;
				return;
			}

			*amplitude = 0.0;
			return;


		case FX_CHAN_SIZE_1:
			*base = FX_SampleCurve1D(edElemDef->sizeShape[1][0], edElemDef->sizeScale[1] * scaleFactor, sampleTime);

			if ((edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_SIZE_1) != 0)
			{
				*amplitude = FX_SampleCurve1D(edElemDef->sizeShape[1][1], edElemDef->sizeScale[1] * scaleFactor, sampleTime) - *base;
				return;
			}

			*amplitude = 0.0;
			return;


		case FX_CHAN_SCALE:
			*base = FX_SampleCurve1D(edElemDef->scaleShape[0], edElemDef->scaleScale * scaleFactor, sampleTime);

			if ((edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_SCALE) != 0)
			{
				*amplitude = FX_SampleCurve1D(edElemDef->scaleShape[1], edElemDef->scaleScale * scaleFactor, sampleTime) - *base;
				return;
			}

			*amplitude = 0.0;
			return;
		}

		if (routing != FX_CHAN_NONE)
		{
			Assert();
		}

		*base = 0.0;
		*amplitude = 0.0;
	}

	void FX_SampleVisualState(FxElemDef* elemDef, FxEditorElemDef* edElemDef)
	{
		FxSampleChannel routing[5];
		FX_GetVisualSampleRouting(edElemDef, routing);

		const float rotationScale = (edElemDef->rotationScale * 0.017453292f) / (static_cast<float>( static_cast<std::uint8_t>(elemDef->visStateIntervalCount) ) * 1000.0f);
		const int secondColorSrc = (edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_COLOR) != 0;
		const int secondAlphaSrc = (edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_ALPHA) != 0;

		for (int sampleIndex = 0; sampleIndex <= static_cast<std::uint8_t>(elemDef->visStateIntervalCount); ++sampleIndex)
		{
			const float sampleTime = static_cast<float>(sampleIndex) / static_cast<float>( static_cast<std::uint8_t>(elemDef->visStateIntervalCount) );
			FxElemVisStateSample* visStateRange = &elemDef->visSamples[sampleIndex];

			if (routing[0])
			{
				if (routing[0] != FX_CHAN_NONE)
				{
					Assert();
				}

				memset(&visStateRange->base.color, -1, 4);
				memset(&visStateRange->amplitude.color, -1, 4);

				//*(DWORD*)visStateRange->base.color = -1;
				//*(DWORD*)visStateRange->amplitude.color = -1;
			}
			else
			{
				float rgba[4];

				FX_SampleCurve3D(edElemDef->color[0], rgba, 1.0, sampleTime);
				rgba[3] = FX_SampleCurve1D(edElemDef->alpha[0], 1.0, sampleTime);

				if ((edElemDef->editorFlags & FX_ED_FLAG_MODULATE_COLOR_BY_ALPHA) != 0)
				{
					rgba[0] = rgba[3] * rgba[0];
					rgba[1] = rgba[3] * rgba[1];
					rgba[2] = rgba[3] * rgba[2];
					rgba[3] = 1.0f;
				}

				Byte4PackVertexColor(rgba, visStateRange->base.color);

				FX_SampleCurve3D(edElemDef->color[secondColorSrc], rgba, 1.0f, sampleTime);
				rgba[3] = FX_SampleCurve1D(edElemDef->alpha[secondAlphaSrc], 1.0f, sampleTime);

				if ((edElemDef->editorFlags & FX_ED_FLAG_MODULATE_COLOR_BY_ALPHA) != 0)
				{
					rgba[0] = rgba[3] * rgba[0];
					rgba[1] = rgba[3] * rgba[1];
					rgba[2] = rgba[3] * rgba[2];
					rgba[3] = 1.0f;
				}
				Byte4PackVertexColor(rgba, visStateRange->amplitude.color);
			}
			if (routing[4] == FX_CHAN_ROTATION)
			{
				visStateRange->base.rotationDelta = FX_SampleCurve1D(edElemDef->rotationShape[0], rotationScale, sampleTime);;


				if (sampleIndex)
				{
					visStateRange->base.rotationTotal = (visStateRange[-1].base.rotationDelta + visStateRange->base.rotationDelta) * 0.5f + visStateRange[-1].base.rotationTotal;
				}
				else
				{
					visStateRange->base.rotationTotal = 0.0f;
				}

				if ((edElemDef->editorFlags & FX_ED_FLAG_USE_RANDOM_ROTATION_DELTA) != 0)
				{
					visStateRange->amplitude.rotationDelta = FX_SampleCurve1D(edElemDef->rotationShape[1], rotationScale, sampleTime) - visStateRange->base.rotationDelta;

					if (sampleIndex)
					{
						visStateRange->amplitude.rotationTotal = (visStateRange[-1].amplitude.rotationDelta + visStateRange->amplitude.rotationDelta) * 0.5f + visStateRange[-1].amplitude.rotationTotal;
					}
					else
					{
						visStateRange->amplitude.rotationTotal = 0.0f;
					}
				}
				else
				{
					visStateRange->amplitude.rotationDelta = 0.0f;
					visStateRange->amplitude.rotationTotal = 0.0f;
				}
			}
			else
			{
				if (routing[4] != FX_CHAN_NONE)
				{
					Assert();
				}

				visStateRange->base.rotationDelta = 0.0f;
				visStateRange->base.rotationTotal = 0.0f;
				visStateRange->amplitude.rotationDelta = 0.0f;
				visStateRange->amplitude.rotationTotal = 0.0f;
			}

			FX_SampleVisualStateScalar(routing[1], visStateRange->amplitude.size, visStateRange->base.size, edElemDef, sampleTime, 0.5f);
			FX_SampleVisualStateScalar(routing[2], &visStateRange->amplitude.size[1], &visStateRange->base.size[1], edElemDef, sampleTime, 0.5f);
			FX_SampleVisualStateScalar(routing[3], &visStateRange->amplitude.scale, &visStateRange->base.scale, edElemDef, sampleTime, 1.0f);
		}
	}

	void FX_BoundFloatRange(FxFloatRange* range, float lower, float upper)
	{
		if (range->amplitude < 0.0f)
		{
			Assert();
		}

		if (lower > range->base || range->base + range->amplitude > upper)
		{
			range->base = range->base + 0.000099999997f;
			range->amplitude = range->amplitude - 0.0001999999f;

			if (range->base < lower)
			{
				Assert();
			}

			if (range->base + range->amplitude > upper)
			{
				Assert();
			}
		}
	}

	void FX_ScaleFloatRange(FxFloatRange* from, FxFloatRange* to, float scale)
	{
		to->base = from->base * scale;
		to->amplitude = from->amplitude * scale;
	}

	int FX_GetLoopingLife(FxEffectDef* effectDef)
	{
		int msecLoopingLifeMax = 0;
		for (int elemIndex = 0; elemIndex < effectDef->elemDefCountLooping; ++elemIndex)
		{
			FxElemDef* elemDef = &effectDef->elemDefs[elemIndex];
			if (elemDef->spawn.looping.count == 0x7FFFFFFF)
			{
				return 0x7FFFFFFF;
			}

			if (msecLoopingLifeMax < elemDef->spawn.looping.intervalMsec * (elemDef->spawn.looping.count - 1))
			{
				msecLoopingLifeMax = elemDef->spawn.looping.intervalMsec * (elemDef->spawn.looping.count - 1);
			}
		}

		return msecLoopingLifeMax;
	}

	int FX_CopyEmittedElemDefs(FxElemDef* elemDefArray, FxEditorEffectDef* editorEffect, char** memPool)
	{
		if (!editorEffect)
		{
			Assert();
		}

		int elemCount = 0;
		for (int elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex)
		{
			// skip disabled elemDefs
			if ((editorEffect->elems[elemIndex].editorFlags & FX_ED_FLAG_DISABLED) != 0)
			{
				continue;
			}

			FxEffectDef* emission = editorEffect->elems[elemIndex].emission;

			if (emission && emission->elemDefCountOneShot && FX_FindEmission(editorEffect, emission) == elemIndex)
			{
				const int elemIndexStop = emission->elemDefCountOneShot + emission->elemDefCountLooping;
				for (int elemIndexEmit = emission->elemDefCountLooping; elemIndexEmit != elemIndexStop; ++elemIndexEmit)
				{
					FxElemDef* elemDef = &elemDefArray[elemCount++];
					FxElemDef* elemDefEmit = &emission->elemDefs[elemIndexEmit];

					memcpy(elemDef, elemDefEmit, sizeof(FxElemDef));

					if ((elemDef->flags & FX_ELEM_RUN_MASK) == FX_ELEM_RUN_RELATIVE_TO_SPAWN)
					{
						elemDef->flags &= ~FX_ELEM_RUN_MASK;
					}

					FX_ReserveElemDefMemory(elemDef, memPool);
					memcpy(elemDef->velSamples, elemDefEmit->velSamples, sizeof(FxElemVelStateSample) * ((unsigned __int8)elemDef->velIntervalCount + 1));

					if (elemDef->visSamples)
					{
						memcpy(elemDef->visSamples->base.color, elemDefEmit->visSamples->base.color, sizeof(FxElemVisStateSample) * ((unsigned __int8)elemDef->visStateIntervalCount + 1));
					}

					if (static_cast<std::uint8_t>(elemDef->visualCount) > 1u)
					{
						memcpy(elemDef->visuals.array, elemDefEmit->visuals.array, sizeof(FxElemVisuals) * (unsigned __int8)elemDef->visualCount);
					}
				}
			}
		}

		return elemCount;
	}

	void FX_CopyMarkVisuals(FxEditorElemDef* edElemDef, FxElemMarkVisuals* markVisualsArray)
	{
		memcpy(markVisualsArray, &edElemDef->u.markVisuals, sizeof(FxElemMarkVisuals) * edElemDef->visualCount);
	}

	void FX_CopyVisuals(FxEditorElemDef* edElemDef, FxElemVisuals* visualsArray)
	{
		memcpy(visualsArray, &edElemDef->u.visuals, sizeof(FxElemVisuals) * edElemDef->visualCount);
	}

	void FX_CopyCanonicalRange_FxIntRange_FxIntRange_(FxIntRange* from, FxIntRange* to)
	{
		int amplitude;

		if (from->amplitude >= 0)
		{
			to->base = from->base;
			amplitude = from->amplitude;
		}
		else
		{
			to->base = from->amplitude + from->base;
			amplitude = -from->amplitude;
		}

		to->amplitude = amplitude;
	}

	void FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(FxFloatRange* from, FxFloatRange* to)
	{
		if (from->amplitude >= 0.0f)
		{
			*to = *from;
		}
		else
		{
			to->base = from->base + from->amplitude;
			to->amplitude = -from->amplitude;
		}
	}

	void FX_ConvertAtlas(FxEditorElemDef* edElemDef, FxElemDef* elemDef)
	{
		if (!elemDef || !edElemDef)
		{
			Assert();
		}

		if (edElemDef->visualCount && FX_ElemUsesMaterial(edElemDef))
		{
			elemDef->atlas.behavior = static_cast<char>(edElemDef->atlas.behavior);
			elemDef->atlas.index = static_cast<char>(edElemDef->atlas.index);
			elemDef->atlas.fps = static_cast<char>(edElemDef->atlas.fps);
			elemDef->atlas.loopCount = static_cast<char>(edElemDef->atlas.loopCount + 1);
			elemDef->atlas.colIndexBits = static_cast<char>(edElemDef->atlas.colIndexBits);
			elemDef->atlas.rowIndexBits = static_cast<char>(edElemDef->atlas.rowIndexBits);
			elemDef->atlas.entryCount = static_cast<std::int16_t>(edElemDef->atlas.entryCount);
			elemDef->atlas.rowIndexBits = 0;
			elemDef->atlas.colIndexBits = 0;

			game::MaterialInfo matInfo = {};
			Material_GetInfo(edElemDef->u.visuals[0].material, &matInfo);

			if (static_cast<std::uint8_t>(matInfo.textureAtlasRowCount) > 1u)
			{
				do
				{
					++elemDef->atlas.rowIndexBits;

				} while (1 << elemDef->atlas.rowIndexBits < static_cast<std::uint8_t>(matInfo.textureAtlasRowCount));
			}

			if (static_cast<std::uint8_t>(matInfo.textureAtlasColumnCount) > 1u)
			{
				do
				{
					++elemDef->atlas.colIndexBits;

				} while (1 << elemDef->atlas.colIndexBits < static_cast<std::uint8_t>(matInfo.textureAtlasColumnCount));
			}

			elemDef->atlas.entryCount = static_cast<std::int16_t>( 1 << (elemDef->atlas.colIndexBits + elemDef->atlas.rowIndexBits) );
		}

		else
		{
			elemDef->atlas.behavior = 0;
			elemDef->atlas.index = 0;

			elemDef->atlas.colIndexBits = 0;
			elemDef->atlas.rowIndexBits = 0;
		}
	}

	void FX_ConvertTrail_CalcNormForSegment(const float* vert0, const float* vert1, float* outNormal)
	{
		outNormal[0] = vert0[1] - vert1[1];
		outNormal[1] = vert1[0] - vert0[0];
		Vec2Normalize(outNormal);
	}

	void FX_ConvertTrail_CompileVertices(FxEditorElemDef* edElemDef, FxTrailDef* outTrailDef, char** mempool)
	{
		const float SNAP_TOLERANCE_POS = 0.0099999998f;
		const float SNAP_TOLERANCE_TEXCOORD = 0.0099999998f;
		const float SNAP_TOLERANCE_NORM = 0.94999999f;
		const float SMOOTH_THRESHOLD = 0.0f;

		FxEditorTrailDef* trailDef = &edElemDef->trailDef;
		if (edElemDef != (FxEditorElemDef*)0xFFFFFCC0)
		{
			if (!trailDef)
			{
				Assert();
			}

			int indCount = trailDef->indCount;
			if ((indCount & 1) != 0)
			{
				Assert();
			}

			FxTrailVertex* outVertPtrBegin = (FxTrailVertex*)*mempool;
			*mempool += 20 * indCount;

			FxTrailVertex* outVertPtrEnd = &outVertPtrBegin[indCount];
			FxTrailVertex* outVertPtrIter = outVertPtrBegin;

			const std::uint16_t* indPtrEnd = &trailDef->inds[indCount];
			for (const std::uint16_t* primaryEdgeIndPtr = trailDef->inds; primaryEdgeIndPtr != indPtrEnd; primaryEdgeIndPtr += 2)
			{
				float primaryEdgeNorm[2] = {};
				FX_ConvertTrail_CalcNormForSegment(trailDef->verts[primaryEdgeIndPtr[0]].pos, trailDef->verts[primaryEdgeIndPtr[1]].pos, primaryEdgeNorm);

				for (int edgeIter = 0; edgeIter != 2; ++edgeIter)
				{
					float accumNorm[2] = {};

					for (const std::uint16_t* secondaryEdgeIndPtr = trailDef->inds; secondaryEdgeIndPtr != indPtrEnd; secondaryEdgeIndPtr += 2)
					{
						float v[2];
						v[0] = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos[0] - trailDef->verts[*secondaryEdgeIndPtr].pos[0];
						v[1] = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos[1] - trailDef->verts[*secondaryEdgeIndPtr].pos[1];

						if (Vec2Length(v) > SNAP_TOLERANCE_POS)
						{
							v[0] = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos[0] - trailDef->verts[secondaryEdgeIndPtr[1]].pos[0];
							v[1] = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos[1] - trailDef->verts[secondaryEdgeIndPtr[1]].pos[1];

							if (Vec2Length(v) > SNAP_TOLERANCE_POS)
							{
								continue;
							}
						}

						float secondaryEdgeNorm[2] = {};
						FX_ConvertTrail_CalcNormForSegment(trailDef->verts[*secondaryEdgeIndPtr].pos, trailDef->verts[secondaryEdgeIndPtr[1]].pos, secondaryEdgeNorm);

						if (primaryEdgeNorm[0] * secondaryEdgeNorm[0] + primaryEdgeNorm[1] * secondaryEdgeNorm[1] > SMOOTH_THRESHOLD)
						{
							accumNorm[0] = secondaryEdgeNorm[0] + accumNorm[0];
							accumNorm[1] = secondaryEdgeNorm[1] + accumNorm[1];
						}
					}

					Vec2Normalize(accumNorm);

					outVertPtrIter[edgeIter].pos[0] = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos[0];
					outVertPtrIter[edgeIter].pos[1] = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos[1];

					outVertPtrIter[edgeIter].normal[0] = accumNorm[0];
					outVertPtrIter[edgeIter].normal[1] = accumNorm[1];
					outVertPtrIter[edgeIter].texCoord = trailDef->verts[primaryEdgeIndPtr[edgeIter]].texCoord;
				}

				outVertPtrIter += 2;
			}

			if (outVertPtrIter != outVertPtrEnd)
			{
				Assert();
			}

			FxTrailVertex* emittedVertPtrBegin = outVertPtrBegin;
			FxTrailVertex* emittedVertPtrEnd = outVertPtrBegin;
			std::uint16_t*  emittedIndPtrBegin = reinterpret_cast<std::uint16_t*>(*mempool); // (unsigned __int16*)*mempool;

			*mempool += 2 * indCount;
			std::uint16_t* emittedIndPtrEnd = emittedIndPtrBegin;

			for (outVertPtrIter = outVertPtrBegin; outVertPtrIter != outVertPtrEnd; ++outVertPtrIter)
			{
				if (emittedVertPtrEnd > outVertPtrIter)
				{
					Assert();
				}

				FxTrailVertex* emittedVertPtrIter = nullptr;

				for (emittedVertPtrIter = emittedVertPtrBegin; emittedVertPtrIter != emittedVertPtrEnd; ++emittedVertPtrIter)
				{
					float v[2];
					v[0] = emittedVertPtrIter->pos[0] - outVertPtrIter->pos[0];
					v[1] = emittedVertPtrIter->pos[1] - outVertPtrIter->pos[1];

					if (Vec2Length(v) <= SNAP_TOLERANCE_POS 
						&& fabs(outVertPtrIter->texCoord - emittedVertPtrIter->texCoord) <= SNAP_TOLERANCE_TEXCOORD 
						&& SNAP_TOLERANCE_NORM <= outVertPtrIter->normal[0] * emittedVertPtrIter->normal[0] + outVertPtrIter->normal[1] * emittedVertPtrIter->normal[1])
					{
						break;
					}
				}

				if (emittedVertPtrIter == emittedVertPtrEnd)
				{
					*emittedVertPtrIter = *outVertPtrIter;
					++emittedVertPtrEnd;
				}

				*emittedIndPtrEnd = static_cast<unsigned short>(emittedVertPtrIter - emittedVertPtrBegin);

				if (*emittedIndPtrEnd != emittedVertPtrIter - emittedVertPtrBegin)
				{
					Assert();
				}

				++emittedIndPtrEnd;
			}

			outTrailDef->verts = emittedVertPtrBegin;
			outTrailDef->vertCount = emittedVertPtrEnd - emittedVertPtrBegin;

			if (outTrailDef->vertCount * sizeof(FxTrailVertex) > sizeof(FxTrailVertex) * indCount)
			{
				Assert();
			}

			outTrailDef->inds = emittedIndPtrBegin;
			outTrailDef->indCount = emittedIndPtrEnd - emittedIndPtrBegin;

			if (outTrailDef->indCount * 2 > 2 * indCount)
			{
				Assert();
			}
		}
	}

	void FX_ConvertTrail(char** mempool, FxTrailDef** outTrailDef, FxEditorElemDef* edElemDef)
	{
		if (edElemDef->elemType == FX_ELEM_TYPE_TRAIL)
		{
			*outTrailDef = (FxTrailDef*)*mempool;
			*mempool += sizeof(FxTrailDef);

			FX_ConvertTrail_CompileVertices(edElemDef, *outTrailDef, mempool);

			if (edElemDef->trailSplitDist <= 0 || edElemDef->trailRepeatDist <= 0)
			{
				Assert();
			}

			(*outTrailDef)->splitDist = edElemDef->trailSplitDist;
			(*outTrailDef)->scrollTimeMsec = static_cast<int>(edElemDef->trailScrollTime * 1000.0f);
			(*outTrailDef)->repeatDist = edElemDef->trailRepeatDist;
		}
		else
		{
			*outTrailDef = nullptr;
		}
	}

	void FX_ConvertEffectDefRef(FxEffectDef* result, FxEffectDefRef* ref)
	{
		ref->handle = result;
	}

	void FX_ConvertElemDef(FxElemDef* elemDef, FxEditorElemDef* edElemDef, int velStateCount, int visStateCount, char** memPool)
	{
		if (!elemDef || !edElemDef || velStateCount < 2 || (visStateCount && visStateCount < 2))
		{
			Assert();
		}

		elemDef->flags = edElemDef->flags;

		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->spawnRange, &elemDef->spawnRange);
		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->fadeInRange, &elemDef->fadeInRange);
		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->fadeOutRange, &elemDef->fadeOutRange);

		elemDef->spawnFrustumCullRadius = edElemDef->spawnFrustumCullRadius;

		if ((edElemDef->editorFlags & FX_ED_FLAG_LOOPING) != 0)
		{
			elemDef->spawn.looping.intervalMsec = edElemDef->spawnLooping.intervalMsec;
			if (elemDef->spawn.looping.intervalMsec <= 0)
			{
				elemDef->spawn.looping.intervalMsec = 1;
			}

			elemDef->spawn.looping.count = edElemDef->spawnLooping.count;
			if (!elemDef->spawn.looping.count)
			{
				elemDef->spawn.looping.count = ~FX_ED_FLAG_DISABLED;
			}
		}
		else
		{
			elemDef->spawn = reinterpret_cast<FxSpawnDef&>(edElemDef->spawnOneShot); // elemDef->spawn = (FxSpawnDef)edElemDef->spawnOneShot;
		}

		FX_CopyCanonicalRange_FxIntRange_FxIntRange_(&edElemDef->spawnDelayMsec, &elemDef->spawnDelayMsec);
		FX_CopyCanonicalRange_FxIntRange_FxIntRange_(&edElemDef->lifeSpanMsec, &elemDef->lifeSpanMsec);

		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->spawnOrigin[0], &elemDef->spawnOrigin[0]);
		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->spawnOrigin[1], &elemDef->spawnOrigin[1]);
		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->spawnOrigin[2], &elemDef->spawnOrigin[2]);

		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->spawnOffsetRadius, &elemDef->spawnOffsetRadius);
		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->spawnOffsetHeight, &elemDef->spawnOffsetHeight);

		FX_ScaleFloatRange(&edElemDef->spawnAngles[0], &elemDef->spawnAngles[0], 0.017453292f);
		FX_ScaleFloatRange(&edElemDef->spawnAngles[1], &elemDef->spawnAngles[1], 0.017453292f);
		FX_ScaleFloatRange(&edElemDef->spawnAngles[2], &elemDef->spawnAngles[2], 0.017453292f);

		FX_ScaleFloatRange(&edElemDef->angularVelocity[0], &elemDef->angularVelocity[0], 0.000017453292f);
		FX_ScaleFloatRange(&edElemDef->angularVelocity[1], &elemDef->angularVelocity[1], 0.000017453292f);
		FX_ScaleFloatRange(&edElemDef->angularVelocity[2], &elemDef->angularVelocity[2], 0.000017453292f);

		FX_ScaleFloatRange(&edElemDef->initialRotation, &elemDef->initialRotation, 0.017453292f);
		FX_ScaleFloatRange(&edElemDef->gravity, &elemDef->gravity, 0.0099999998f);

		if (elemDef->gravity.base != 0.0f || elemDef->gravity.amplitude != 0.0f) 
		{
			elemDef->flags |= FX_ELEM_HAS_GRAVITY;
		}

		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->elasticity, &elemDef->reflectionFactor);
		FX_BoundFloatRange(&elemDef->reflectionFactor, 0.0f, 1.0f);
		FX_ConvertAtlas(edElemDef, elemDef);

		elemDef->elemType = edElemDef->elemType;
		elemDef->visualCount = static_cast<char>(edElemDef->visualCount);
		elemDef->velIntervalCount = static_cast<char>(velStateCount - 1);

		elemDef->visStateIntervalCount = 0;
		if (visStateCount)
		{
			elemDef->visStateIntervalCount = static_cast<char>(visStateCount - 1);
		}
		
		FX_ReserveElemDefMemory(elemDef, memPool);
		FX_SampleVelocity(edElemDef, elemDef);

		// FxEditorElemDef *edElemDef@<eax>, FxElemDef *elemDef@<esi>
		/*const static uint32_t FX_SampleVelocity = 0x47DB50;
		__asm
		{
			pushad;

			mov		esi, elemDef;
			mov		eax, edElemDef;
			call	FX_SampleVelocity;

			popad;
		}*/

		if (visStateCount)
		{
			FX_SampleVisualState(elemDef, edElemDef); //utils::hook::call<void(__cdecl)(FxElemDef*, FxEditorElemDef*)>(0x47DDA0)(elemDef, edElemDef);
		}

		if (elemDef->elemType == FX_ELEM_TYPE_DECAL)
		{
			FX_CopyMarkVisuals(edElemDef, elemDef->visuals.markArray);
		}
		else
		{
			if (elemDef->visualCount)
			{
				FxElemDefVisuals* vis = &elemDef->visuals;
				if (elemDef->visualCount != 1)
				{
					vis = (FxElemDefVisuals*)elemDef->visuals.array;
				}
				FX_CopyVisuals(edElemDef, (FxElemVisuals*)vis);
			}
			else
			{
				elemDef->visuals.markArray = nullptr;

				if (elemDef->elemType == FX_ELEM_TYPE_OMNI_LIGHT || elemDef->elemType == FX_ELEM_TYPE_SPOT_LIGHT)
				{
					elemDef->visualCount = 1;
				}
				else if (elemDef->elemType == FX_ELEM_TYPE_MODEL)
				{
					elemDef->flags &= ~FX_ELEM_USE_MODEL_PHYSICS;
				}
			}
		}

		if (edElemDef->lightingFrac > 255.0f)
		{
			Assert();
		}

		if (static_cast<int>(edElemDef->lightingFrac * 255.0f) >= 256)
		{
			Assert();
		}

		elemDef->lightingFrac = static_cast<char>((edElemDef->lightingFrac * 255.0f));
		elemDef->useItemClip = (edElemDef->editorFlags & FX_ED_FLAG_USE_ITEM_CLIP) != 0;

		if ((edElemDef->editorFlags & FX_ED_FLAG_BOUNDING_SPHERE) != 0)
		{
			elemDef->collMins[0] = edElemDef->collOffset[0] - edElemDef->collRadius;
			elemDef->collMins[1] = edElemDef->collOffset[1] - edElemDef->collRadius;
			elemDef->collMins[2] = edElemDef->collOffset[2] - edElemDef->collRadius;

			elemDef->collMaxs[0] = edElemDef->collRadius + edElemDef->collOffset[0];
			elemDef->collMaxs[1] = edElemDef->collRadius + edElemDef->collOffset[1];
			elemDef->collMaxs[2] = edElemDef->collRadius + edElemDef->collOffset[2];
		}
		else
		{
			elemDef->collMins[0] = 0.0f;
			elemDef->collMins[1] = 0.0f;
			elemDef->collMins[2] = 0.0f;
			elemDef->collMaxs[0] = 0.0f;
			elemDef->collMaxs[1] = 0.0f;
			elemDef->collMaxs[2] = 0.0f;
		}


		FX_ConvertEffectDefRef(
			(edElemDef->editorFlags& FX_ED_FLAG_PLAY_ON_TOUCH) != 0 ? edElemDef->effectOnImpact : nullptr, &elemDef->effectOnImpact);

		FX_ConvertEffectDefRef(
			(edElemDef->editorFlags & FX_ED_FLAG_PLAY_ON_DEATH) != 0 ? edElemDef->effectOnDeath : nullptr, &elemDef->effectOnDeath);

		FX_ConvertEffectDefRef(
			(edElemDef->editorFlags & FX_ED_FLAG_PLAY_ON_RUN) != 0 ? edElemDef->emission : nullptr, &elemDef->effectEmitted);

		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->emitDist, &elemDef->emitDist);
		FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&edElemDef->emitDistVariance, &elemDef->emitDistVariance);

		FX_ConvertTrail(memPool, &elemDef->trailDef, edElemDef);

		if (edElemDef->sortOrder < 255)
		{
			if (edElemDef->sortOrder <= 0)
			{
				elemDef->sortOrder = 0;
			}
			elemDef->sortOrder = static_cast<char>(edElemDef->sortOrder);
		}
		else
		{
			elemDef->sortOrder = -1;
		}
	}

	int FX_ConvertElemDefsOfType(const int* velStateCount, FxElemDef* elemDefArray, FxEditorEffectDef* editorEffect, int loopingFlagState, const int* visStateCount, char** memPool)
	{
		if (!elemDefArray || !editorEffect || !velStateCount || !visStateCount)
		{
			Assert();
		}

		if (loopingFlagState != 1 && loopingFlagState )
		{
			Assert();
		}

		int elemCount = 0;
		for (int elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex)
		{
			if ((editorEffect->elems[elemIndex].editorFlags & (FX_ED_FLAG_DISABLED | FX_ED_FLAG_LOOPING)) == loopingFlagState)
			{
				FxElemDef* elemDef = &elemDefArray[elemCount++];
				FX_ConvertElemDef(elemDef, &editorEffect->elems[elemIndex], velStateCount[elemIndex], visStateCount[elemIndex], memPool);
			}
		}

		return elemCount;
	}

	FxEffectDef* FX_Convert(FxEditorEffectDef* editorEffect, void* (__cdecl* Alloc)(size_t))
	{
		if (!editorEffect)
		{
			Assert();
		}

		int elemIndex;
		FxEditorElemDef* edElemDef;

		int emitIndex[32];
		memset(emitIndex, -1, sizeof(emitIndex));

		int totalBytesNeeded =  editorEffect->elemCount * sizeof(FxElemDef) + sizeof(FxEffectDef); // corr
		int elemCountTotal = editorEffect->elemCount;

		for (elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex)
		{
			edElemDef = &editorEffect->elems[elemIndex];

			// decrease total bytes and elemcount if elem is disabled
			if (editorEffect->elems[elemIndex].editorFlags < 0)
			{
				totalBytesNeeded -= sizeof(FxElemDef);
				--elemCountTotal;
			}

			if (editorEffect->elems[elemIndex].elemType == FX_ELEM_TYPE_MODEL && (edElemDef->flags & FX_ELEM_USE_MODEL_PHYSICS) != 0)
			{
				for (int visualIndex = 0; visualIndex < edElemDef->visualCount; ++visualIndex)
				{
					FxElemVisuals* elemVisual = &edElemDef->u.visuals[visualIndex];
					if (elemVisual->model)
					{
						// #NOT_IMPL

						edElemDef->flags &= ~FX_ELEM_USE_MODEL_PHYSICS;
						Warning(0, "Physics not yet implemented!");

						//if (!elemVisual->model->physPreset)
						//{
						//	//elemVisual->model->physPreset = FX_RegisterPhysPreset("default");
						//	Warning(20, "ERROR: no physics preset specified for the FX model [%s]\n", elemVisual->model->name);
						//}
					}
				}
			}
		}

		if(!elemCountTotal)
		{
			return nullptr;
		}

		int velStateCount[32];
		int visStateCount[32];

		int elemdef_bytes = 0;

		for (elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex)
		{
			edElemDef = &editorEffect->elems[elemIndex];

			// skip disabled elemDefs
			if((edElemDef->editorFlags & FX_ED_FLAG_DISABLED) != 0)
			{
				continue;
			}

			if (!FX_Validate(edElemDef, editorEffect))
			{
				return nullptr;
			}

			FxSampleChannel routing[5];
			FX_GetVisualSampleRouting(edElemDef, routing);

			const int intervalLimit = FX_DecideIntervalLimit(edElemDef);

			velStateCount[elemIndex] = FX_DecideVelocitySampleCount(edElemDef, intervalLimit);
			visStateCount[elemIndex] = FX_DecideVisualSampleCount(edElemDef, routing, intervalLimit);

			elemdef_bytes += FX_AdditionalBytesNeededForElemDef(visStateCount[elemIndex], velStateCount[elemIndex], edElemDef->visualCount, edElemDef->elemType);
			elemdef_bytes += FX_AdditionalBytesNeededForGeomTrail(edElemDef);

			if (edElemDef->emission && edElemDef->emission->elemDefCountOneShot)
			{
				const int firstEmitted = FX_FindEmission(editorEffect, edElemDef->emission);
				if (firstEmitted == elemIndex)
				{
					emitIndex[elemIndex] = elemCountTotal;

					elemCountTotal += edElemDef->emission->elemDefCountOneShot;
					elemdef_bytes += FX_AdditionalBytesNeededForEmission(edElemDef->emission);
				}
				else
				{
					emitIndex[elemIndex] = emitIndex[firstEmitted];
				}
			}
		}

		totalBytesNeeded += elemdef_bytes;
		totalBytesNeeded += static_cast<int>( strlen(editorEffect->name) + 1 );

		FxEffectDef* effect = (FxEffectDef*)Alloc(totalBytesNeeded);
		effect->elemDefs = (FxElemDef*)&effect[1];

		char* memPool;
		memPool = (char*)&effect[1];
		memPool += elemCountTotal * sizeof(FxElemDef);
		
		effect->elemDefCountLooping = FX_ConvertElemDefsOfType(
			velStateCount, 
			effect->elemDefs, 
			editorEffect,
			1,
			visStateCount, 
			&memPool);

		effect->elemDefCountOneShot = FX_ConvertElemDefsOfType(
			velStateCount, 
			&effect->elemDefs[effect->elemDefCountLooping], 
			editorEffect, 
			0, 
			visStateCount, 
			&memPool);

		effect->elemDefCountEmission = FX_CopyEmittedElemDefs(
			&effect->elemDefs[effect->elemDefCountOneShot + effect->elemDefCountLooping], 
			editorEffect, 
			&memPool);

		if (effect->elemDefCountEmission + effect->elemDefCountOneShot + effect->elemDefCountLooping != elemCountTotal)
		{
			Assert();
		}

		int effectFlags = 0;
		for (elemIndex = 0; elemIndex != elemCountTotal; ++elemIndex)
		{
			if (static_cast<float>( static_cast<std::uint8_t>( effect->elemDefs[elemIndex].lightingFrac )) != 0.0f)
			{
				effectFlags |= 1u;
				break;
			}
		}

		effect->flags = effectFlags;
		effect->msecLoopingLife = FX_GetLoopingLife(effect);
		effect->totalSize = totalBytesNeeded;
		effect->name = memPool;

		strcpy((char*)effect->name, editorEffect->name);
		memPool += &effect->name[strlen(effect->name) + 1] - effect->name;

		if (memPool - reinterpret_cast<char*>(effect) != effect->totalSize)
		{
			Assert();
		}

		return effect;
	}
}