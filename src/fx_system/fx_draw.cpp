#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword
#define SLODWORD(x)  (*((int*)&(x)))

namespace fx_system
{
	void (* const s_drawElemHandler[8])(FxDrawState*) =
	{
		FX_DrawElem_BillboardSprite,
		FX_DrawElem_OrientedSprite,
		FX_DrawElem_Tail,
		nullptr,
		FX_DrawElem_Cloud,
		FX_DrawElem_Model,
		FX_DrawElem_Light,
		FX_DrawElem_SpotLight
	};

	// ----------------------

	// checked
	void FX_DrawElem_BillboardSprite(FxDrawState* draw)
	{
		//utils::hook::call<void(__cdecl)(FxDrawState*)>(0x48CBB0)(draw);

		if (!FX_CullElementForDraw_Sprite(draw))
		{
			const float normal[3] =
			{
				-draw->camera->axis[0][0],
				-draw->camera->axis[0][1],
				-draw->camera->axis[0][2]
			};

			const float tangent[3] =
			{
				-draw->camera->axis[1][0],
				-draw->camera->axis[1][1],
				-draw->camera->axis[1][2]
			};

			const float binormal[3] =
			{
				draw->camera->axis[2][0],
				draw->camera->axis[2][1],
				draw->camera->axis[2][2]
			};

			FX_GenSpriteVerts(draw, normal, tangent, binormal);
		}
	}

	// checked
	void FX_DrawElem_OrientedSprite(FxDrawState* draw)
	{
		//utils::hook::call<void(__cdecl)(FxDrawState*)>(0x48CC40)(draw);

		if (!FX_CullElementForDraw_Sprite(draw))
		{
			FX_GenSpriteVerts(draw, draw->orient.axis[0], draw->orient.axis[1], draw->orient.axis[2]);
		}
	}

	// checked
	void FX_DrawElem_Tail(FxDrawState* draw)
	{
		//utils::hook::call<void(__cdecl)(FxDrawState*)>(0x48CF90)(draw);

		FX_GetVelocityAtTime(draw->elemDef, draw->randomSeed, draw->msecLifeSpan, draw->msecElapsed, &draw->orient, draw->elem->baseVel, draw->velDirWorld);
		//utils::hook::call<void(__cdecl)(FxElemDef*, int, float, float, game::orientation_t*, const float*, float*)>(0x4896C0) (draw->elemDef, draw->randomSeed, draw->msecLifeSpan, draw->msecElapsed, &draw->orient, draw->elem->baseVel, draw->velDirWorld);


		Vec3Normalize(draw->velDirWorld);

		if (!FX_CullElementForDraw_Tail(draw))
		{
			draw->posWorld[0] = (draw->velDirWorld[0] * -draw->visState.size[1]) + draw->posWorld[0];
			draw->posWorld[1] = (draw->velDirWorld[1] * -draw->visState.size[1]) + draw->posWorld[1];
			draw->posWorld[2] = (draw->velDirWorld[2] * -draw->visState.size[1]) + draw->posWorld[2];

			const float deltaCamera[3] =
			{
				draw->camera->origin[0] - draw->posWorld[0],
				draw->camera->origin[1] - draw->posWorld[1],
				draw->camera->origin[2] - draw->posWorld[2]
			};

			float tangent[3] = {};
			Vec3Cross(draw->velDirWorld, deltaCamera, tangent);
			Vec3Normalize(tangent);

			float normal[3] = {};
			Vec3Cross(tangent, draw->velDirWorld, normal);
			FX_GenSpriteVerts(draw, normal, tangent, draw->velDirWorld);

			//auto binormal = draw->velDirWorld;
			// FxDrawState *draw@<edx>, float *normal@<ecx>, float *tangent, float *binormal
			/*const static uint32_t func_addr = 0x48BE80;
			__asm
			{
				pushad;

				push	binormal;

				lea		eax, tangent;
				push	eax;

				lea		ecx, normal

				mov		edx, draw;
				call	func_addr;
				add		esp, 8;

				popad;
			}*/
		}
	}

	// checked
	void FX_DrawElem_Cloud(FxDrawState* draw)
	{
		//utils::hook::call<void(__cdecl)(FxDrawState*)>(0x48D2D0)(draw);

		if (game::Dvar_FindVar("fx_drawClouds")->current.enabled)
		{
			FX_GetVelocityAtTime(draw->elemDef, draw->randomSeed, draw->msecLifeSpan, draw->msecElapsed, &draw->orient, draw->elem->baseVel, draw->velDirWorld);
			Vec3Normalize(draw->velDirWorld);

			const float rnd = fx_randomTable[27 + draw->preVisState.randomSeed];
			const float graph_a = rnd * draw->preVisState.refState->amplitude.scale + draw->preVisState.refState->base.scale;
			const float graph_b = rnd * draw->preVisState.refState[1].amplitude.scale + draw->preVisState.refState[1].base.scale;

			draw->visState.scale = draw->preVisState.sampleLerpInv * graph_a + draw->preVisState.sampleLerp * graph_b;
			if (draw->visState.scale != 0.0f && !FX_CullElementForDraw_Cloud(draw))
			{
				++draw->system->gfxCloudCount;

				FxElemVisuals visuals;
				visuals.material = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).material;

				if (!visuals.material)
				{
					Assert();
				}

#ifdef FXEDITOR // #ENV_DEPENDENT
				// R_AddParticleCloudToScene(visuals);
				game::GfxParticleCloud* cloud = utils::hook::call<game::GfxParticleCloud* (__cdecl)(game::Material*)>(0x49FDC0)(visuals.material);
#else
				game::GfxParticleCloud* cloud = utils::hook::call<game::GfxParticleCloud* (__cdecl)(game::Material*)>(0x5022D0)(visuals.material);
#endif

				if (cloud)
				{
					FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);
					FX_SetPlacement(&cloud->placement, draw);

					cloud->color.array[0] = draw->visState.color[0];
					cloud->color.array[1] = draw->visState.color[1];
					cloud->color.array[2] = draw->visState.color[2];
					cloud->color.array[3] = draw->visState.color[3];

					cloud->radius[0] = draw->visState.size[0];
					cloud->radius[1] = draw->visState.size[1];

					cloud->endpos[0] = draw->posWorld[0] - draw->velDirWorld[0];
					cloud->endpos[1] = draw->posWorld[1] - draw->velDirWorld[1];
					cloud->endpos[2] = draw->posWorld[2] - draw->velDirWorld[2];
				}
			}
		}
	}

	void FX_DrawElem_Model(FxDrawState* draw)
	{
		//utils::hook::call<void(__cdecl)(FxDrawState*)>(0x48D460)(draw);
		const float rnd = fx_randomTable[27 + draw->preVisState.randomSeed];
		const float graph_b = rnd * draw->preVisState.refState[1].amplitude.scale + draw->preVisState.refState[1].base.scale;
		const float graph_a = rnd * draw->preVisState.refState->amplitude.scale + draw->preVisState.refState->base.scale;

		draw->visState.scale = draw->preVisState.sampleLerpInv * graph_a + graph_b * draw->preVisState.sampleLerp;
		if (draw->visState.scale != 0.0f)
		{
			game::GfxScaledPlacement placement = {};
			FX_SetPlacement(&placement, draw);

			if (draw->elemDef->elemType != FX_ELEM_TYPE_MODEL)
			{
				Assert();
			}

			if ((draw->elemDef->flags & FX_ELEM_USE_MODEL_PHYSICS) != 0)
			{
				// #PHYS
				// FX_SetPlacementFromPhysics(&placement.base, draw);
			}

			FxElemVisuals visuals;
			visuals.model = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).model;

			if (!visuals.model)
			{
				Assert();
			}

#ifdef FXEDITOR // #ENV_DEPENDENT
			// R_FilterXModelIntoScene(visuals.model, &placement, 0, (unsigned __int16*)&draw->elem->u);
			utils::hook::call<void(__cdecl)(game::XModel*, game::GfxScaledPlacement*, unsigned __int16, unsigned __int16*)>(0x4BF910)(visuals.model, &placement, 0, (unsigned __int16*)&draw->elem->u);
#else
			utils::hook::call<void(__cdecl)(game::XModel*, game::GfxScaledPlacement*, unsigned __int16, unsigned __int16*)>(0x524FD0)(visuals.model, &placement, 0, (unsigned __int16*)&draw->elem->u);
#endif
		}
	}

	void FX_DrawElem_Light(FxDrawState* draw)
	{
		//utils::hook::call<void(__cdecl)(FxDrawState*)>(0x48D5B0)(draw);

		if (!FX_CullElementForDraw_Light(draw))
		{
			FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);

			const float b = static_cast<float>( static_cast<std::uint8_t>(draw->visState.color[0]) ) * 0.003921568859368563f;
			const float g = static_cast<float>( static_cast<std::uint8_t>(draw->visState.color[1]) ) * 0.003921568859368563f;
			const float r = static_cast<float>( static_cast<std::uint8_t>(draw->visState.color[2]) ) * 0.003921568859368563f;

#ifdef FXEDITOR // #ENV_DEPENDENT
			// R_AddOmniLightToScene(draw->posWorld, draw->visState.size[0], r, g, b);
			utils::hook::call<void(__cdecl)(const float*, float, float, float, float)>(0x49FE30)(draw->posWorld, draw->visState.size[0], r, g, b);
#else
			// not going to work because rgp.world
			utils::hook::call<void(__cdecl)(const float*, float, float, float, float)>(0x502340)(draw->posWorld, draw->visState.size[0], r, g, b);
#endif
		}
	}

	void FX_DrawElem_SpotLight(FxDrawState* draw)
	{
		// utils::hook::call<void(__cdecl)(FxDrawState*)>(0x48D640)(draw);

		if (!FX_CullElementForDraw_Light(draw))
		{
			FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);

			const float b = static_cast<float>(static_cast<std::uint8_t>(draw->visState.color[0])) * 0.003921568859368563f;
			const float g = static_cast<float>(static_cast<std::uint8_t>(draw->visState.color[1])) * 0.003921568859368563f;
			const float r = static_cast<float>(static_cast<std::uint8_t>(draw->visState.color[2])) * 0.003921568859368563f;

#ifdef FXEDITOR // #ENV_DEPENDENT
			// R_AddSpotLightToScene(draw->posWorld, draw->orient.axis, draw->visState.size[0], r, g, b);
			utils::hook::call<void(__cdecl)(const float*, const float(*)[3], float, float, float, float)>(0x49FF00)(draw->posWorld, draw->orient.axis, draw->visState.size[0], r, g, b);
#else
			// not going to work because rgp.world
			utils::hook::call<void(__cdecl)(const float*, const float(*)[3], float, float, float, float)>(0x502410)(draw->posWorld, draw->orient.axis, draw->visState.size[0], r, g, b);
#endif
		}
	}

	void FX_DrawSpotLightEffect(FxSystem* system, FxEffect* effect, int msecDraw)
	{
		if (system->activeSpotLightEffectCount <= 0 || system->activeSpotLightElemCount <= 0)
		{
			Assert();
		}

		FxDrawState drawState = {};
		drawState.effect = effect;
		drawState.system = system;
		drawState.msecDraw = msecDraw;

		FxElem* elem = FX_ElemFromHandle(system, system->activeSpotLightElemHandle);
		FxElemDef* elemDef = &effect->def->elemDefs[static_cast<std::uint8_t>(elem->defIndex)];

		if (elemDef->elemType != FX_ELEM_TYPE_SPOT_LIGHT)
		{
			Assert();
		}

		FX_DrawElement(elemDef, elem, &drawState);
	}

	void FX_DrawSpotLight(FxSystem* system)
	{
		if (!system || !system->camera.isValid || system->isArchiving)
		{
			Assert();
		}

		if (system->activeSpotLightElemCount > 0)
		{
			if (system->activeSpotLightEffectCount != 1 || system->activeSpotLightElemCount != 1)
			{
				Assert();
			}

			auto effect = FX_EffectFromHandle(system, system->activeSpotLightEffectHandle);
			FX_DrawSpotLightEffect(system, effect, system->msecDraw);
		}

		if (system->needsGarbageCollection)
		{
			FX_RunGarbageCollectionAndPrioritySort(system);
		}
	}

	void FX_SetPlacement(game::GfxScaledPlacement* placement, FxDrawState* draw)
	{
		float axis[3][3];

		const float msecElapsed = FX_GetMsecForSamplingAxis(draw->msecElapsed, draw->msecLifeSpan, static_cast<std::uint8_t>(draw->elem->atRestFraction));
		FX_GetElemAxis(draw->elemDef, draw->randomSeed, &draw->orient, msecElapsed, axis);
		AxisToQuat(axis, placement->base.quat);

		placement->base.origin[0] = draw->posWorld[0];
		placement->base.origin[1] = draw->posWorld[1];
		placement->base.origin[2] = draw->posWorld[2];
		placement->scale = draw->visState.scale;
	}

	void FX_GetElemAxis(FxElemDef* elemDef, int randomSeed, game::orientation_t* orient, float msecElapsed, float(*axis)[3])
	{
		float angles[3];

		angles[0] = fx_randomTable[12 + randomSeed] * elemDef->spawnAngles[0].amplitude + elemDef->spawnAngles[0].base;
		angles[1] = fx_randomTable[13 + randomSeed] * elemDef->spawnAngles[1].amplitude + elemDef->spawnAngles[1].base;
		angles[2] = fx_randomTable[14 + randomSeed] * elemDef->spawnAngles[2].amplitude + elemDef->spawnAngles[2].base;

		angles[0] = (fx_randomTable[3 + randomSeed] * elemDef->angularVelocity[0].amplitude + elemDef->angularVelocity[0].base) * msecElapsed + angles[0];
		angles[1] = (fx_randomTable[4 + randomSeed] * elemDef->angularVelocity[1].amplitude + elemDef->angularVelocity[1].base) * msecElapsed + angles[1];
		angles[2] = (fx_randomTable[5 + randomSeed] * elemDef->angularVelocity[2].amplitude + elemDef->angularVelocity[2].base) * msecElapsed + angles[2];

		FX_AnglesToOrientedAxis(axis, angles, orient);
	}

	void FX_AnglesToOrientedAxis(float(*axisOut)[3], const float* anglesInRad, game::orientation_t* orient)
	{
		float localDir[3];

		float cy = cos(anglesInRad[1]);
		float sy = sin(anglesInRad[1]);

		float cp = cos(anglesInRad[0]);
		float sp = sin(anglesInRad[0]);

		float cr = cos(anglesInRad[2]);
		float sr = sin(anglesInRad[2]);

		localDir[0] = cp * cy;
		localDir[1] = cp * sy;
		localDir[2] = -sp;

		FX_OrientationDirToWorldDir(orient, localDir, (float*)axisOut);
		localDir[0] = ((sr * sp) * cy) - (cr * sy);
		localDir[1] = ((sr * sp) * sy) + (cr * cy);
		localDir[2] = sr * cp;

		FX_OrientationDirToWorldDir(orient, localDir, &(*axisOut)[3]);
		localDir[0] = ((cr * sp) * cy) + (sr * sy);
		localDir[1] = ((cr * sp) * sy) - (sr * cy);
		localDir[2] = cr * cp;

		FX_OrientationDirToWorldDir(orient, localDir, &(*axisOut)[6]);
	}

	// checked
	void FX_GetSpriteTexCoords(FxDrawState* draw, float* s0, float* ds, float* t0, float* dt)
	{
		if (!draw)
		{
			Assert();
		}

		FxElemDef* elemDef = draw->elemDef;
		if (!elemDef)
		{
			Assert();
		}

		const int atlasCount = elemDef->atlas.entryCount;
		if (atlasCount == 1)
		{
			*s0 = 0.0f;
			*ds = 1.0f;
			*t0 = 0.0f;
			*dt = 1.0f;
			return;
		}

		if ((unsigned int)(atlasCount - 2) > 254 || ((atlasCount - 1) & atlasCount) != 0)
		{
			Assert();
		}

		int atlasIndex = 0;

		if ((elemDef->atlas.behavior & FX_ATLAS_START_MASK) != FX_ATLAS_START_FIXED)
		{
			if ((elemDef->atlas.behavior & FX_ATLAS_START_MASK) == FX_ATLAS_START_RANDOM)
			{
				atlasIndex = FX_RandomFloatAsUInt16(22, draw->randomSeed); // 2 * draw->randomSeed ?
			}
			else
			{
				if ((elemDef->atlas.behavior & FX_ATLAS_START_MASK) != FX_ATLAS_START_INDEXED)
				{
					Assert();
				}

				atlasIndex = (atlasCount - 1) & static_cast<std::uint8_t>(draw->elem->sequence);
			}
		}
		else
		{
			atlasIndex = static_cast<std::uint8_t>(elemDef->atlas.index);
		}

		if ((elemDef->atlas.behavior & FX_ATLAS_PLAY_OVER_LIFE) != FX_ATLAS_START_FIXED)
		{
			atlasIndex += (static_cast<int>( draw->normTimeUpdateEnd * static_cast<float>(atlasCount) ));
		}
		else if (elemDef->atlas.fps)
		{
			atlasIndex += (static_cast<std::uint8_t>(elemDef->atlas.fps) * static_cast<int>(draw->msecElapsed) / 1000);
		}

		if ((elemDef->atlas.behavior & FX_ATLAS_LOOP_ONLY_N_TIMES) != FX_ATLAS_START_FIXED && atlasIndex >= atlasCount * static_cast<std::uint8_t>(elemDef->atlas.loopCount))
		{
			atlasIndex = atlasCount - 1;
		}

		atlasIndex = (atlasCount - 1) & atlasIndex;

		const auto col_idx = static_cast<float>((1 << elemDef->atlas.colIndexBits));
		*ds = (1.0f / col_idx);
		*s0 = (1.0f / col_idx) * static_cast<float>((atlasIndex & (1 << elemDef->atlas.colIndexBits) - 1));

		const auto row_idx = static_cast<float>((1 << elemDef->atlas.rowIndexBits));
		*dt = (1.0f / row_idx);
		*t0 = (1.0f / row_idx) * (float)(atlasIndex >> elemDef->atlas.colIndexBits);
	}

	float FX_GetMsecForSamplingAxis(float msecElapsed, float msecLifeSpan, int atRestFraction)
	{
		const float msecAtRest = static_cast<float>(atRestFraction) * msecLifeSpan * 0.0039215689f;
		const float msecSinceAtRest = msecElapsed - msecAtRest;

		if (msecElapsed - msecAtRest <= 0.0f)
		{
			return msecElapsed;
		}

		if (msecSinceAtRest < 300.0f)
		{
			return msecElapsed - msecSinceAtRest * msecSinceAtRest * 0.0016666667f;
		}

		return msecAtRest + 150.0f;
	}


	FxElemVisuals FX_GetElemVisuals(FxElemDef* elemDef, int randomSeed)
	{
		FxElemVisuals result = {};

		if (!elemDef->visualCount)
		{
			Assert();
		}

		if (elemDef->visualCount == 1)
		{
			result.anonymous = elemDef->visuals.instance.anonymous;
		}
		else
		{
			//result.anonymous = elemDef->visuals.markArray->materials[(elemDef->visualCount * static_cast<std::uint16_t>(fx_randomTable[21 + randomSeed])) >> 16];
			result.anonymous = elemDef->visuals.markArray->materials[(elemDef->visualCount * FX_RandomFloatAsUInt16(21, randomSeed)) >> 16];
			
		}
		return result;
	}

	void FX_SetupVisualState(FxElemDef* elemDef, FxEffect* effect, int randomSeed, float normTimeUpdateEnd, FxElemPreVisualState* preVisState)
	{
		const float samplePoint = static_cast<float>( static_cast<std::uint8_t>(elemDef->visStateIntervalCount) ) * normTimeUpdateEnd;
		const int point = static_cast<int>(floorf(samplePoint));

		preVisState->sampleLerp = samplePoint - static_cast<float>(point);
		preVisState->sampleLerpInv = 1.0f - preVisState->sampleLerp;
		preVisState->elemDef = elemDef;
		preVisState->effect = effect;
		preVisState->refState = &elemDef->visSamples[point];
		preVisState->randomSeed = randomSeed;
		preVisState->distanceFade = 255;
	}

	// checked
	char FX_InterpolateColor(int channel, FxElemVisStateSample* refState, float valueLerp, float valueLerpInv, float sampleLerp, float sampleLerpInv)
	{
		const float a1b1 =	static_cast<float>( static_cast<std::uint8_t>( refState[1].base.color[channel] ) ) * valueLerpInv + 
							static_cast<float>( static_cast<std::uint8_t>( refState[1].amplitude.color[channel] ) ) * valueLerp;

		const float a0b0 =	static_cast<float>( static_cast<std::uint8_t>( refState->base.color[channel] ) ) * valueLerpInv +
							static_cast<float>( static_cast<std::uint8_t>( refState->amplitude.color[channel] ) ) * valueLerp;

		return static_cast<char>( (static_cast<double>( a1b1 * sampleLerp + a0b0 * sampleLerpInv ) + 9.313225746154785e-10) );
	}

	// checked
	float FX_IntegrateRotationFromZero(FxElemVisStateSample* refState, int randomSeed, int randomKey, float sampleLerp, float msecLifeSpan)
	{
		const float random = fx_randomTable[randomKey + randomSeed];
		const float squared_half = sampleLerp * sampleLerp * 0.5f;

		const float sampleLerp_a = (refState->amplitude.rotationTotal	* random + refState->base.rotationTotal);
		const float sampleLerp_b = (refState->amplitude.rotationDelta	* random + refState->base.rotationDelta)   * (sampleLerp - squared_half) + sampleLerp_a;
		const float sampleLerp_c = (refState[1].amplitude.rotationDelta * random + refState[1].base.rotationDelta) * squared_half + sampleLerp_b;

		return sampleLerp_c * msecLifeSpan;
	}

	void FX_EvaluateSize(FxElemPreVisualState* preVisState, FxElemVisualState* visState)
	{
		const float rnd_width = fx_randomTable[25 + preVisState->randomSeed];
		const float graphA = rnd_width * preVisState->refState->amplitude.size[0] + preVisState->refState->base.size[0];
		const float graphB = rnd_width * preVisState->refState[1].amplitude.size[0] + preVisState->refState[1].base.size[0];

		visState->size[0] = graphB * preVisState->sampleLerp + graphA * preVisState->sampleLerpInv;

		if ((preVisState->elemDef->flags & FX_ELEM_NONUNIFORM_SCALE) != 0)
		{
			const float rnd_height = fx_randomTable[26 + preVisState->randomSeed];
			const float graphX = rnd_height * preVisState->refState->amplitude.size[1] + preVisState->refState->base.size[1];
			const float graphY = rnd_height * preVisState->refState[1].amplitude.size[1] + preVisState->refState[1].base.size[1];

			visState->size[1] = graphY * preVisState->sampleLerp + graphX * preVisState->sampleLerpInv;
		}
		else
		{
			visState->size[1] = visState->size[0];
		}
	}

	// checked
	void FX_EvaluateVisualState(FxElemPreVisualState* preVisState, float msecLifeSpan, FxElemVisualState* visState)
	{
		FxElemDef* elemDef = preVisState->elemDef;
		if (!elemDef)
		{
			Assert();
		}

		const float valueLerp = fx_randomTable[23 + preVisState->randomSeed];

		visState->color[0] = FX_InterpolateColor(0, preVisState->refState, valueLerp, 1.0f - valueLerp, preVisState->sampleLerp, preVisState->sampleLerpInv);
		visState->color[1] = FX_InterpolateColor(1, preVisState->refState, valueLerp, 1.0f - valueLerp, preVisState->sampleLerp, preVisState->sampleLerpInv);
		visState->color[2] = FX_InterpolateColor(2, preVisState->refState, valueLerp, 1.0f - valueLerp, preVisState->sampleLerp, preVisState->sampleLerpInv);
		visState->color[3] = FX_InterpolateColor(3, preVisState->refState, valueLerp, 1.0f - valueLerp, preVisState->sampleLerp, preVisState->sampleLerpInv);

		// #NOT_IMPL
		/*if (elemDef->lightingFrac)
		{
			FX_LerpFloat(preVisState, visState, elemDef);
		}*/

		visState->rotationTotal = fx_randomTable[24 + preVisState->randomSeed] * elemDef->initialRotation.amplitude + elemDef->initialRotation.base;
		visState->rotationTotal = FX_IntegrateRotationFromZero(preVisState->refState, preVisState->randomSeed, FXRAND_ROTATION_DELTA, preVisState->sampleLerp, msecLifeSpan) + visState->rotationTotal;

		visState->color[3] = static_cast<std::uint16_t>(LOWORD(preVisState->distanceFade) * static_cast<std::uint8_t>(visState->color[3])) >> 8;
	}

	void FX_EvaluateDistanceFade(FxDrawState* draw)
	{
		if (draw->elemDef->fadeInRange.amplitude != 0.0f || draw->elemDef->fadeOutRange.amplitude != 0.0f)
		{
			float fade_in = 1.0f;
			float fade_out = 1.0f;

			const float dist = Vec3Distance(draw->camera->origin, draw->posWorld);

			if (draw->elemDef->fadeInRange.amplitude != 0.0f)
			{
				fade_in = FX_CalculateFade(dist, &draw->elemDef->fadeInRange);
			}
			if (draw->elemDef->fadeOutRange.amplitude != 0.0f)
			{
				fade_out = 1.0f - FX_CalculateFade(dist, &draw->elemDef->fadeInRange);
			}

			float end = fade_out;
			if (fade_out - fade_in >= 0.0f)
			{
				end = fade_in;
			}

			draw->preVisState.distanceFade = static_cast<std::int64_t>(end * 255.0f + 0.5f);
		}
	}

	float FX_CalculateFade(float dist, FxFloatRange* range)
	{
		const float prog = dist - range->base;
		float fade = 0.0f;

		if (prog < 0.0f)
		{
			return 1.0f;
		}

		if (range->amplitude > prog)
		{
			fade = 1.0f - prog / range->amplitude;
		}

		return fade;
	}

	// checked
	void FX_GenTrail_IndsForSegment(FxDrawState* draw, unsigned __int16 reservedBaseVertex, r_double_index_t* outIndices)
	{
		FxTrailDef* trailDef = draw->elemDef->trailDef;
		if (!trailDef)
		{
			Assert();
		}

		const int indCount = trailDef->indCount;
		if (2 * (indCount / 2) != indCount)
		{
			Assert();
		}

		const std::uint16_t farBase = LOWORD(trailDef->vertCount) + reservedBaseVertex;
		for (int indPairIter = 0; indPairIter != indCount / 2; ++indPairIter)
		{

			r_double_index_t index = {};
			index.value[0] = trailDef->inds[2 * indPairIter] + reservedBaseVertex;
			index.value[1] = trailDef->inds[2 * indPairIter + 1] + reservedBaseVertex;

			outIndices[0] = index;

			r_double_index_t quadInds = {};
			quadInds.value[0] = trailDef->inds[2 * indPairIter] + farBase;
			quadInds.value[1] = trailDef->inds[2 * indPairIter + 1] + farBase;

			index.value[0] = quadInds.value[0];
			// keep value[1]

			outIndices[1] = quadInds;
			outIndices[2] = index;
			outIndices = &outIndices[3];
		}
	}

	// checked
	void FX_GenTrail_VertsForSegment(FxTrailSegmentDrawState* segmentDrawState, game::GfxPackedVertex* remoteVerts)
	{
		FxTrailDef* trailDef = segmentDrawState->trailDef;
		if (!segmentDrawState->trailDef || trailDef->vertCount <= 0 || trailDef->indCount <= 0)
		{
			Assert();
		}

		const float cosRot = cosf(segmentDrawState->rotation);
		const float sinRot = sinf(segmentDrawState->rotation);

		const float left[3] =
		{
			(cosRot * segmentDrawState->basis[0][0]) + (sinRot * segmentDrawState->basis[1][0]),
			(cosRot * segmentDrawState->basis[0][1]) + (sinRot * segmentDrawState->basis[1][1]),
			(cosRot * segmentDrawState->basis[0][2]) + (sinRot * segmentDrawState->basis[1][2])
		};

		const float up[3] =
		{
			(sinRot * segmentDrawState->basis[0][0]) + -cosRot * segmentDrawState->basis[1][0],
			(sinRot * segmentDrawState->basis[0][1]) + -cosRot * segmentDrawState->basis[1][1],
			(sinRot * segmentDrawState->basis[0][2]) + -cosRot * segmentDrawState->basis[1][2]
		};
		
		float uCoord = segmentDrawState->uCoord;

		for (int vertIter = 0; vertIter != trailDef->vertCount; ++vertIter)
		{
			const float width = trailDef->verts[vertIter].pos[0] * segmentDrawState->size[0];
			const float height = trailDef->verts[vertIter].pos[1] * segmentDrawState->size[1];
			remoteVerts->xyz[0] = ((width * left[0]) + segmentDrawState->posWorld[0]) + (height * up[0]);
			remoteVerts->xyz[1] = ((width * left[1]) + segmentDrawState->posWorld[1]) + (height * up[1]);
			remoteVerts->xyz[2] = ((width * left[2]) + segmentDrawState->posWorld[2]) + (height * up[2]);

			remoteVerts->color.array[0] = segmentDrawState->color[0];
			remoteVerts->color.array[1] = segmentDrawState->color[1];
			remoteVerts->color.array[2] = segmentDrawState->color[2];
			remoteVerts->color.array[3] = segmentDrawState->color[3];

			remoteVerts->texCoord = Vec2PackTexCoords(uCoord, trailDef->verts[vertIter].texCoord);

			game::PackedUnitVec packed_normal = {};
			packed_normal.array[0] = static_cast<char>(((trailDef->verts[vertIter].normal[1] * up[0] + trailDef->verts[vertIter].normal[0] * left[0]) * 127.0f + 127.5f));
			packed_normal.array[1] = static_cast<char>(((trailDef->verts[vertIter].normal[1] * up[1] + trailDef->verts[vertIter].normal[0] * left[1]) * 127.0f + 127.5f));
			packed_normal.array[2] = static_cast<char>(((trailDef->verts[vertIter].normal[1] * up[2] + trailDef->verts[vertIter].normal[0] * left[2]) * 127.0f + 127.5f));
			packed_normal.array[3] = 63;

			remoteVerts->normal = packed_normal;
			remoteVerts->tangent.packed = 0x3F7F7FFE; // 0.998047f
			++remoteVerts;
		}
	}

	// checked
	void FX_GenTrail_PopulateSegmentDrawState(FxTrailSegmentDrawState* outState, const float(*basis)[3], FxDrawState* draw, float spawnDist, float uCoordOffset)
	{
		outState->trailDef = draw->elemDef->trailDef;

		outState->posWorld[0] = draw->posWorld[0];
		outState->posWorld[1] = draw->posWorld[1];
		outState->posWorld[2] = draw->posWorld[2];

		outState->basis[0][0] = (*basis)[0];
		outState->basis[0][1] = (*basis)[1];
		outState->basis[0][2] = (*basis)[2];
		outState->basis[1][0] = (*basis)[3];
		outState->basis[1][1] = (*basis)[4];
		outState->basis[1][2] = (*basis)[5];

		outState->rotation = draw->visState.rotationTotal;

		outState->size[0] = draw->visState.size[0];
		outState->size[1] = draw->visState.size[1];

		outState->color[0] = draw->visState.color[0];
		outState->color[1] = draw->visState.color[1];
		outState->color[2] = draw->visState.color[2];
		outState->color[3] = draw->visState.color[3];

		outState->uCoord = spawnDist / static_cast<float>(draw->elemDef->trailDef->repeatDist) + uCoordOffset;
	}

	// checked
	void FX_TrailElem_UncompressBasis(float(*basis)[3], const char(*inBasis)[3])
	{
		for (int basisVecIter = 0; basisVecIter != 2; ++basisVecIter)
		{
			for (int dimIter = 0; dimIter != 3; ++dimIter)
			{
				(*basis)[3 * basisVecIter + dimIter] = (float)(*inBasis)[3 * basisVecIter + dimIter] * 0.0078740157f;
			}
		}
	}

	unsigned int FX_CullElementForDraw_FrustumPlaneCount(FxDrawState* draw)
	{
		if ((!draw || !draw->camera || draw->camera->frustumPlaneCount < 5))
		{
			Assert();
		}

		if ((draw->elemDef->flags & FX_ELEM_DRAW_PAST_FOG) != 0)
		{
			return 5;
		}

		return draw->camera->frustumPlaneCount;
	}

	bool FX_CullElementForDraw_Sprite(FxDrawState* draw)
	{
		if (!game::Dvar_FindVar("fx_cull_elem_draw")->current.enabled)
		{
			return false;
		}

		const unsigned int frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
		float radius = draw->visState.size[0];

		if (draw->visState.size[0] - draw->visState.size[1] < 0.0f)
		{
			radius = draw->visState.size[1];
		}

		if (FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, radius))
		{
			return true;
		}
		
		return false;
	}

	bool FX_CullElementForDraw_Tail(FxDrawState* draw)
	{
		if (game::Dvar_FindVar("fx_cull_elem_draw")->current.enabled)
		{
			unsigned int frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);

			float endpoint[3] =
			{
				draw->velDirWorld[0] * (draw->visState.size[1] * -2.0f) + draw->posWorld[0],
				draw->velDirWorld[1] * (draw->visState.size[1] * -2.0f) + draw->posWorld[1],
				draw->velDirWorld[2] * (draw->visState.size[1] * -2.0f) + draw->posWorld[2]
			};

			if (FX_CullCylinder(draw->camera, frustumPlaneCount, draw->posWorld, endpoint, draw->visState.size[0]))
			{
				return true;
			}
		}

		return false;
	}

	bool FX_CullElementForDraw_Cloud(FxDrawState* draw)
	{
		if (game::Dvar_FindVar("fx_cull_elem_draw")->current.enabled)
		{
			const unsigned int frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
			if (FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, draw->visState.scale + (draw->visState.size[0] - draw->visState.size[1]) < 0.0f ? draw->visState.size[1] : draw->visState.size[0]))
			{
				return true;
			}
		}

		return false;
	}

	bool FX_CullElementForDraw_Light(FxDrawState* draw)
	{
		if (game::Dvar_FindVar("fx_cull_elem_draw")->current.enabled)
		{
			const unsigned int frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
			if (FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, draw->visState.size[0]))
			{
				return true;
			}
		}

		return false;
	}

	bool FX_CullSphere(FxCamera* camera, unsigned int frustumPlaneCount, const float* posWorld, float radius)
	{
		if (!camera->isValid || frustumPlaneCount != camera->frustumPlaneCount && frustumPlaneCount != 5)
		{
			Assert();
		}

		for (unsigned int planeIndex = 0; planeIndex < frustumPlaneCount; ++planeIndex)
		{
			if (!Vec3IsNormalized(camera->frustum[planeIndex]))
			{
				Assert();
			}

			if (-radius >= 
				(camera->frustum[planeIndex][0] * posWorld[0]) +
				(camera->frustum[planeIndex][1] * posWorld[1]) +
				(camera->frustum[planeIndex][2] * posWorld[2]) - camera->frustum[planeIndex][3])
			{
				return true;
			}
		}

		return false;
	}

	bool FX_CullCylinder(FxCamera* camera, unsigned int frustumPlaneCount, const float* posWorld0, const float* posWorld1, float radius)
	{
		if (!camera->isValid || frustumPlaneCount != camera->frustumPlaneCount && frustumPlaneCount != 5)
		{
			Assert();
		}

		for (unsigned int planeIndex = 0; planeIndex < frustumPlaneCount; ++planeIndex)
		{
			if (!Vec3IsNormalized(camera->frustum[planeIndex]))
			{
				Assert();
			}

			if (( camera->frustum[planeIndex][0] * posWorld0[0]
				+ camera->frustum[planeIndex][1] * posWorld0[1]
				+ camera->frustum[planeIndex][2] * posWorld0[2] - camera->frustum[planeIndex][3]) <= -radius

				&& (( camera->frustum[planeIndex][0] * posWorld1[0]
					+ camera->frustum[planeIndex][1] * posWorld1[1]
					+ camera->frustum[planeIndex][2] * posWorld1[2]) - camera->frustum[planeIndex][3]) <= -radius)
			{
				return true;
			}
		}

		return false;
	}

	void FX_DrawSpriteEffect(FxSystem* system, FxEffect* effect, int drawTime)
	{
		FxDrawState drawState = {};
		drawState.effect = effect;
		drawState.msecDraw = drawTime;

		std::uint16_t elemHandle = effect->firstElemHandle[0];
		if (elemHandle != UINT16_MAX)
		{
			drawState.system = system;
			FxElemDef* elemDefs = effect->def->elemDefs;

			do
			{
				FxElem* elem = FX_ElemFromHandle(system, elemHandle);
				FxElemDef* elemDef = &elemDefs[static_cast<std::uint8_t>(elem->defIndex)];

				if (elemDef->elemType > FX_ELEM_TYPE_LAST_SPRITE)
				{
					Assert();
				}

				if (elemDef->visualCount)
				{
					FX_DrawElement(elemDef, elem, &drawState);
				}

				elemHandle = elem->nextElemHandleInEffect;

			} while (elemHandle != UINT16_MAX);
		}
	}

	// checked
	void FX_DrawTrail(FxSystem* system, FxTrail* trail, FxDrawState* draw)
	{
		FxTrailSegmentDrawState tailSegmentDrawState = {};
		FxTrailSegmentDrawState lastSegmentDrawState = {};
		FxTrailSegmentDrawState segmentDrawState = {};

		draw->elemDef = &draw->effect->def->elemDefs[static_cast<std::uint8_t>(trail->defIndex)];
		if (draw->elemDef->visualCount)
		{
			std::uint16_t trailElemHandle = trail->firstElemHandle;
			if (trailElemHandle != UINT16_MAX)
			{
				FxTrailElem* trailElem = FX_TrailElemFromHandle(system, trailElemHandle);
				FxTrailDef* trailDef = draw->elemDef->trailDef;

				float uCoordOffset = -floorf(trailElem->spawnDist / static_cast<float>(trailDef->repeatDist));

				if (trailDef->scrollTimeMsec)
				{
					if (trailDef->scrollTimeMsec <= 0)
					{
						// t5
						//uCoordOffset = (float)(1.0 - (float)((float)(draw->msecDraw % trailDef->scrollTimeMsec) / (float)trailDef->scrollTimeMsec)) + uCoordOffset;

						uCoordOffset = 1.0f - (static_cast<float>((draw->msecDraw % trailDef->scrollTimeMsec)) /
							static_cast<float>(trailDef->scrollTimeMsec)) + uCoordOffset;
					}
					else
					{
						// T5 does + uCoordOffset
						//uCoordOffset = (float)((float)(draw->msecDraw % -trailDef->scrollTimeMsec) / (float)-trailDef->scrollTimeMsec) + uCoordOffset;

						uCoordOffset = uCoordOffset - static_cast<float>((draw->msecDraw % trailDef->scrollTimeMsec)) /
							static_cast<float>(trailDef->scrollTimeMsec);
					}

				}

				int upperBoundSegmentCount = 0;
				for (trailElemHandle = trail->firstElemHandle; trailElemHandle != UINT16_MAX; trailElemHandle = trailElem->nextTrailElemHandle)
				{
					trailElem = FX_TrailElemFromHandle(system, trailElemHandle);
					if (trailElem->msecBegin <= draw->msecDraw)
					{
						++upperBoundSegmentCount;
					}
				}

				const int trailDefVertCount = trailDef->vertCount;
				const int trailDefIndCount = trailDef->indCount;
				std::uint16_t reservedBaseVertex = 0;

				if (R_ReserveCodeMeshVerts(trailDefVertCount * upperBoundSegmentCount, &reservedBaseVertex))
				{
					game::GfxPackedVertex* reservedVerts = R_GetCodeMeshVerts(reservedBaseVertex);
					int exactSegmentCount = 0;
					float lastSegmentNormTime = 1.0f;

					memset(&lastSegmentDrawState, 0, sizeof(lastSegmentDrawState));

					for (trailElemHandle = trail->firstElemHandle; trailElemHandle != UINT16_MAX; trailElemHandle = trailElem->nextTrailElemHandle)
					{
						trailElem = FX_TrailElemFromHandle(system, trailElemHandle);
						if (trailElem->msecBegin <= draw->msecDraw)
						{
							float segmentNormTime = 0.0f;
							float basis[2][3] = {};

							
							FX_DrawElement_Setup_1_(draw, trailElem->msecBegin, static_cast<std::uint8_t>(trailElem->sequence), trailElem->origin, &segmentNormTime);
							/*auto* norm = &segmentNormTime;
							auto msec = trailElem->msecBegin;
							auto org = trailElem->origin;
							int seq = static_cast<int>(trailElem->sequence);

							const static uint32_t func_addr = 0x48D740;
							__asm
							{
								pushad;

								push	norm;
								push	org;
								push	seq;
								mov		ebx, msec;
								mov		eax, draw;
								call	func_addr;
								add		esp, 12;

								popad;
							}*/

							FX_TrailElem_UncompressBasis(basis, trailElem->basis);
							//utils::hook::call<void(__fastcall)(float(*_basis)[3], const char(*_inBasis)[3])>(0x48B6E0)(basis, trailElem->basis);

							FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);
							//utils::hook::call<void(__cdecl)(FxElemPreVisualState*, float, FxElemVisualState*)>(0x48BB10)(&draw->preVisState, draw->msecLifeSpan, &draw->visState);

							FX_GenTrail_PopulateSegmentDrawState(&segmentDrawState, basis, draw, trailElem->spawnDist, uCoordOffset);

							if (segmentNormTime < 1.0f)
							{
								if (trailElemHandle == trail->firstElemHandle)
								{
									if (!trailElem->sequence)
									{
										segmentDrawState.color[3] = 0;
									}
								}
								else
								{
									if (lastSegmentNormTime >= 1.0f)
									{
										memcpy(&tailSegmentDrawState, &lastSegmentDrawState, sizeof(tailSegmentDrawState));

										lastSegmentNormTime = (1.0f - lastSegmentNormTime) / (segmentNormTime - lastSegmentNormTime);

										tailSegmentDrawState.uCoord = lastSegmentDrawState.uCoord + (segmentDrawState.uCoord - lastSegmentDrawState.uCoord) * lastSegmentNormTime;
										tailSegmentDrawState.posWorld[0] = (segmentDrawState.posWorld[0] - lastSegmentDrawState.posWorld[0]) * lastSegmentNormTime + lastSegmentDrawState.posWorld[0];
										tailSegmentDrawState.posWorld[1] = (segmentDrawState.posWorld[1] - lastSegmentDrawState.posWorld[1]) * lastSegmentNormTime + lastSegmentDrawState.posWorld[1];
										tailSegmentDrawState.posWorld[2] = (segmentDrawState.posWorld[2] - lastSegmentDrawState.posWorld[2]) * lastSegmentNormTime + lastSegmentDrawState.posWorld[2];
										tailSegmentDrawState.basis[0][0] = (segmentDrawState.basis[0][0] - lastSegmentDrawState.basis[0][0]) * lastSegmentNormTime + lastSegmentDrawState.basis[0][0];
										tailSegmentDrawState.basis[0][1] = (segmentDrawState.basis[0][1] - lastSegmentDrawState.basis[0][1]) * lastSegmentNormTime + lastSegmentDrawState.basis[0][1];
										tailSegmentDrawState.basis[0][2] = (segmentDrawState.basis[0][2] - lastSegmentDrawState.basis[0][2]) * lastSegmentNormTime + lastSegmentDrawState.basis[0][2];
										tailSegmentDrawState.basis[1][0] = (segmentDrawState.basis[1][0] - lastSegmentDrawState.basis[1][0]) * lastSegmentNormTime + lastSegmentDrawState.basis[1][0];
										tailSegmentDrawState.basis[1][1] = (segmentDrawState.basis[1][1] - lastSegmentDrawState.basis[1][1]) * lastSegmentNormTime + lastSegmentDrawState.basis[1][1];
										tailSegmentDrawState.basis[1][2] = (segmentDrawState.basis[1][2] - lastSegmentDrawState.basis[1][2]) * lastSegmentNormTime + lastSegmentDrawState.basis[1][2];

										FX_GenTrail_VertsForSegment(&tailSegmentDrawState,  &reservedVerts[trailDefVertCount * exactSegmentCount++]);
										/*auto xx = &reservedVerts[trailDefVertCount * exactSegmentCount++];
										const static uint32_t func_adr = 0x48C780;
										__asm
										{
											pushad;
											push	xx;
											lea		edi, tailSegmentDrawState;
											call	func_adr;
											add     esp, 4;
											popad;
										}*/
									}

									if (trailElemHandle == trail->lastElemHandle)
									{
										segmentDrawState.color[3] = 0;
									}
								}

								FX_GenTrail_VertsForSegment(&segmentDrawState, &reservedVerts[trailDefVertCount * exactSegmentCount++]);
								/*auto yy = &reservedVerts[trailDefVertCount * exactSegmentCount++];
								const static uint32_t func_adr = 0x48C780;
								__asm
								{
									pushad;
									push	yy;
									lea		edi, segmentDrawState;
									call	func_adr;
									add     esp, 4;
									popad;
								}*/
							}

							lastSegmentNormTime = segmentNormTime;
							memcpy(&lastSegmentDrawState, &segmentDrawState, sizeof(lastSegmentDrawState));
						}
					}

					if (exactSegmentCount > upperBoundSegmentCount)
					{
						Assert();
					}

					if (exactSegmentCount > 1)
					{

						const int indicesToReserve = 3 * trailDefIndCount;
						const int vertsPerSegment = trailDef->vertCount;

						FxElemVisuals visuals;
						visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;

						if (system->sprite.material != visuals.anonymous && system->sprite.indexCount)
						{
							if (!system->sprite.name || !system->sprite.material || !system->sprite.indices)
							{
								Assert();
							}
							
							R_AddCodeMeshDrawSurf(system->sprite.material, system->sprite.indices, system->sprite.indexCount, 0, 0, system->sprite.name);
							system->sprite.indexCount = 0;
						}

						int curSegment = 0;
						r_double_index_t* reservedIndices = nullptr;

						while (curSegment < exactSegmentCount - 1 && R_ReserveCodeMeshIndices(indicesToReserve, &reservedIndices))
						{
							if (system->sprite.material != visuals.anonymous)
							{
								system->sprite.name = draw->effect->def->name;
								system->sprite.material = visuals.material;
								system->sprite.indices = reservedIndices;
							}

							system->sprite.indexCount += indicesToReserve;
							FX_GenTrail_IndsForSegment(draw, reservedBaseVertex, reservedIndices);

							++curSegment;
							reservedBaseVertex += static_cast<std::uint16_t>(vertsPerSegment);
						}
					}
				}
			}
		}
	}

	void FX_DrawTrailsForEffect(FxSystem* system, FxEffect* effect, int drawTime)
	{
		FxDrawState drawState = {};
		drawState.system = system;
		drawState.effect = effect;
		drawState.msecDraw = drawTime;

		FxTrail* trail = nullptr;

		for (std::uint16_t trailHandle = drawState.effect->firstTrailHandle; trailHandle != UINT16_MAX; trailHandle = trail->nextTrailHandle)
		{
			trail = FX_TrailFromHandle(system, trailHandle);
			FX_DrawTrail(system, trail, &drawState);
		}
	}

	void FX_DrawSpriteElems(FxSystem* system, int drawTime)
	{
		if (!system || !system->camera.isValid || system->isArchiving)
		{
			Assert();
		}

		system->gfxCloudCount = 0;
		system->sprite.indices = nullptr;
		system->sprite.indexCount = 0;
		system->sprite.name = nullptr;
		system->sprite.material = nullptr;

		int numTrailEffects = 0;
		std::uint16_t trailEffects[1024];

		for (int activeIndex = system->firstActiveEffect; activeIndex != system->firstNewEffect; ++activeIndex)
		{
			std::uint16_t effectHandle = system->allEffectHandles[activeIndex & 1023];
			FxEffect* effect = FX_EffectFromHandle(system, effectHandle);

			FX_DrawSpriteEffect(system, effect, drawTime);
			FX_DrawNonSpriteEffect(2, drawTime, system, effect);

			if (effect->firstTrailHandle != UINT16_MAX)
			{
				trailEffects[numTrailEffects++] = effectHandle;
			}
		}

		if (numTrailEffects > 0)
		{
			for (int i = 0; i < numTrailEffects; ++i)
			{
				FxEffect* effect = FX_EffectFromHandle(system, trailEffects[i]);
				FX_DrawTrailsForEffect(system, effect, drawTime);
			}
		}

		if (system->needsGarbageCollection)
		{
			FX_RunGarbageCollectionAndPrioritySort(system);
		}

		if (system->sprite.indexCount)
		{
			if (!system->sprite.name || !system->sprite.material || !system->sprite.indices)
			{
				Assert();
			}

			R_AddCodeMeshDrawSurf(system->sprite.material, system->sprite.indices, system->sprite.indexCount, 0, 0, system->sprite.name);
			system->sprite.indexCount = 0;
			system->sprite.indices = nullptr;
		}
	}

	void FX_DrawElement_Setup_1_(FxDrawState* draw, int msecBegin, int sequence, float* origin, float* outRealNormTime)
	{
		const int msecElapsed = draw->msecDraw - msecBegin;
		draw->randomSeed = FX_ElemRandomSeed(draw->effect->randomSeed, msecBegin, sequence);
		draw->msecLifeSpan = static_cast<float>(FX_GetElemLifeSpanMsec(draw->randomSeed, draw->elemDef));

		const float msecElapsedFloat = static_cast<float>(msecElapsed);
		const float normTime = msecElapsedFloat / draw->msecLifeSpan;

		if (outRealNormTime)
		{
			*outRealNormTime = normTime;
		}

		if (msecElapsedFloat < draw->msecLifeSpan)
		{
			draw->msecElapsed = msecElapsedFloat;
			draw->normTimeUpdateEnd = normTime;
		}
		else
		{
			draw->msecElapsed = draw->msecLifeSpan;
			draw->normTimeUpdateEnd = 1.0f;
		}

		FX_GetOrientation(draw->elemDef, &draw->effect->frameAtSpawn, &draw->effect->frameNow, draw->randomSeed, &draw->orient);
		FX_OrientationPosToWorldPos(&draw->orient, origin, draw->posWorld);
		FX_SetupVisualState(draw->elemDef, draw->effect, draw->randomSeed, draw->normTimeUpdateEnd, &draw->preVisState);
		FX_EvaluateSize(&draw->preVisState, &draw->visState);

		draw->camera = &draw->system->camera;
		FX_EvaluateDistanceFade(draw);
	}

	void FX_DrawElement(FxElemDef* elemDef, FxElem* elem, FxDrawState* state)
	{
		const std::uint8_t elem_type = static_cast<uint8_t>(elemDef->elemType);
		if (elem_type >= FX_ELEM_TYPE_LAST_DRAWN + 1)
		{
			Assert();
		}

		if (elemDef->visualCount)
		{
			if (elem->msecBegin <= state->msecDraw)
			{
				state->elem = elem;
				state->elemDef = elemDef;
				FX_DrawElement_Setup_1_(state, elem->msecBegin, static_cast<std::uint8_t>(elem->sequence), elem->___u8.origin, nullptr);

				if (!s_drawElemHandler[elem_type])
				{
					Assert();
				}

				s_drawElemHandler[elem_type](state);
			}
		}
	}

	void FX_DrawNonSpriteEffect(int elemClass, int drawTime, FxSystem* system, FxEffect* effect)
	{
		if (!system)
		{
			Assert();
		}

		FxDrawState drawState = {};
		drawState.effect = effect;
		drawState.msecDraw = drawTime;
		drawState.system = system;

		std::uint16_t elemHandle = effect->firstElemHandle[elemClass];

		if (elemHandle != UINT16_MAX)
		{
			while (elemHandle != UINT16_MAX)
			{
				FxElem* elem = FX_ElemFromHandle(system, elemHandle);
				FxElemDef* elemDef = &effect->def->elemDefs[static_cast<std::uint8_t>(elem->defIndex)];

				if (elemDef->elemType <= FX_ELEM_TYPE_LAST_SPRITE)
				{
					Assert();
				}

				FX_DrawElement(elemDef, elem, &drawState);
				elemHandle = elem->nextElemHandleInEffect;
			}
		}
	}

	void FX_DrawNonSpriteElems(FxSystem* system)
	{
		if (!system || !system->camera.isValid || system->isArchiving)
		{
			Assert();
		}
		
		for (auto i = system->firstActiveEffect; i != system->firstNewEffect; ++i)
		{
			FxEffect* effect = FX_EffectFromHandle(system, system->allEffectHandles[i & 1023]);
			FX_DrawNonSpriteEffect(1, system->msecDraw, system, effect);
		}

		if (system->needsGarbageCollection)
		{
			FX_RunGarbageCollectionAndPrioritySort(system);
		}
	}

	// checked
	void FX_GenSpriteVerts(FxDrawState* draw, const float* normal, const float* tangent, const float* binormal)
	{
		FxElemVisuals visuals;
		visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;

		if (!visuals.material)
		{
			Assert();
		}

		FxSpriteInfo* sprite = &draw->system->sprite;

		if (sprite->material != visuals.anonymous && sprite->indexCount)
		{
			if (!sprite->name || !sprite->material || !sprite->indices)
			{
				Assert();
			}
			
			R_AddCodeMeshDrawSurf(sprite->material, sprite->indices, sprite->indexCount, 0, 0, sprite->name);
			sprite->indexCount = 0;
		}

		r_double_index_t* baseIndices = nullptr;
		std::uint16_t baseVertex = 0;

		if (R_ReserveCodeMeshVerts(4, &baseVertex) && R_ReserveCodeMeshIndices(6, &baseIndices))
		{
			if (sprite->material != visuals.anonymous)
			{
				sprite->name = draw->effect->def->name;
				sprite->material = visuals.material;
				sprite->indices = baseIndices;
			}

			sprite->indexCount += 6;
			FX_EvaluateVisualState(&draw->preVisState, draw->msecLifeSpan, &draw->visState);

			const float rotTotal = draw->visState.rotationTotal;
			const float cosRot = cosf(rotTotal);
			const float sinRot = sinf(rotTotal);

			const float rotatedTangent[3] =
			{
				(cosRot * tangent[0]) + (sinRot * binormal[0]),
				(cosRot * tangent[1]) + (sinRot * binormal[1]),
				(cosRot * tangent[2]) + (sinRot * binormal[2])
			};

			const float rotatedBinormal[3] = 
			{
				(sinRot * tangent[0]) + (-cosRot * binormal[0]),
				(sinRot * tangent[1]) + (-cosRot * binormal[1]),
				(sinRot * tangent[2]) + (-cosRot * binormal[2])
			};
			
			const float left[3] =
			{
				draw->visState.size[0] * rotatedTangent[0],
				draw->visState.size[0] * rotatedTangent[1],
				draw->visState.size[0] * rotatedTangent[2]
			};

			const float up[3] =
			{
				draw->visState.size[1] * rotatedBinormal[0],
				draw->visState.size[1] * rotatedBinormal[1],
				draw->visState.size[1] * rotatedBinormal[2]
			};

			const float leftSide[3] =
			{
				draw->posWorld[0] - left[0],
				draw->posWorld[1] - left[1],
				draw->posWorld[2] - left[2]
			};

			const float rightSide[3] =
			{
				draw->posWorld[0] + left[0],
				draw->posWorld[1] + left[1],
				draw->posWorld[2] + left[2]
			};

			float s0, ds, t0, dt;
			FX_GetSpriteTexCoords(draw, &s0, &ds, &t0, &dt);

			game::PackedUnitVec p_nrm = {};
			p_nrm.array[0] = static_cast<char>(((normal[0] * 127.0f) + 127.5f));
			p_nrm.array[1] = static_cast<char>(((normal[1] * 127.0f) + 127.5f));
			p_nrm.array[2] = static_cast<char>(((normal[2] * 127.0f) + 127.5f));
			p_nrm.array[3] = 63;

			game::PackedUnitVec p_tan = {};
			p_tan.array[0] = static_cast<char>(((rotatedTangent[0] * 127.0f) + 127.5f));
			p_tan.array[1] = static_cast<char>(((rotatedTangent[1] * 127.0f) + 127.5f));
			p_tan.array[2] = static_cast<char>(((rotatedTangent[2] * 127.0f) + 127.5f));
			p_tan.array[3] = 63;

			r_double_index_t index;
			r_double_index_t* indices = baseIndices;

			index.value[0] = baseVertex;
			index.value[1] = baseVertex + 1;
			*baseIndices = index;
			++indices;

			index.value[0] = baseVertex + 2;
			index.value[1] = baseVertex + 2;
			*indices++ = index;

			index.value[0] = baseVertex + 3;
			index.value[1] = baseVertex;
			*indices++ = index;

			float testBinormal[3] = {};
			Vec3Cross(normal, rotatedTangent, testBinormal);

			if (testBinormal[0] * rotatedBinormal[0] + testBinormal[1] * rotatedBinormal[1] + testBinormal[2] * rotatedBinormal[2] > 0.0f)
			{
				Assert();
			}

			game::GfxPackedVertex* verts = R_GetCodeMeshVerts(baseVertex);

			for (int i = 0; i < 4; ++i)
			{
				verts[i].binormalSign = -1.0f;

				verts[i].color.array[0] = draw->visState.color[0];
				verts[i].color.array[1] = draw->visState.color[1];
				verts[i].color.array[2] = draw->visState.color[2];
				verts[i].color.array[3] = draw->visState.color[3];

				verts[i].normal = p_nrm;
				verts[i].tangent = p_tan;
			}


			// ---
			verts->xyz[0] = leftSide[0] + up[0];
			verts->xyz[1] = leftSide[1] + up[1];
			verts->xyz[2] = leftSide[2] + up[2];
			verts->texCoord = Vec2PackTexCoords(s0, t0 + dt);


			// ---
			verts[1].xyz[0] = leftSide[0] - up[0];
			verts[1].xyz[1] = leftSide[1] - up[1];
			verts[1].xyz[2] = leftSide[2] - up[2];
			verts[1].texCoord = Vec2PackTexCoords(s0, t0);


			// ---
			verts[2].xyz[0] = rightSide[0] - up[0];
			verts[2].xyz[1] = rightSide[1] - up[1];
			verts[2].xyz[2] = rightSide[2] - up[2];
			verts[2].texCoord = Vec2PackTexCoords(ds + s0, t0);


			// ---
			verts[3].xyz[0] = rightSide[0] + up[0];
			verts[3].xyz[1] = rightSide[1] + up[1];
			verts[3].xyz[2] = rightSide[2] + up[2];
			verts[3].texCoord = Vec2PackTexCoords(ds + s0, t0 + dt);
		}
	}

	void FX_FillGenerateVertsCmd(int localClientNum, FxGenerateVertsCmd* cmd)
	{
		if (!cmd)
		{
			Assert();
		}

		cmd->system = FX_GetSystem(localClientNum);
		cmd->beamInfo = nullptr;
		cmd->postLightInfo = nullptr;
		cmd->spriteInfo = nullptr;
		cmd->localClientNum = localClientNum;
		cmd->vieworg[0] = 0.0f;
		cmd->vieworg[1] = 0.0f;
		cmd->vieworg[2] = 0.0f;

		SetIdentityAxis(cmd->viewaxis[0]);
	}


	// #ENV_DEPENDENT
	void R_BeginCodeMeshVerts()
	{
		if (game::g_processCodeMesh)
		{
			Assert();
		}

		const auto frontend_data = game::get_frontenddata();
		game::g_processCodeMesh = 1;

#ifdef FXEDITOR
		// R_BeginMeshVerts(&game::get_frontenddata()->codeMesh);
		utils::hook::call<void(__cdecl)(game::GfxMeshData* mesh)>(0x4E4250)(&frontend_data->codeMesh);
#else
		utils::hook::call<void(__cdecl)(game::GfxMeshData* mesh)>(0x52B580)(&frontend_data->codeMesh);
#endif
	}


	// #ENV_DEPENDENT
	void R_EndCodeMeshVerts()
	{
		const auto frontend_data = game::get_frontenddata();
		game::g_processCodeMesh = 0;

#ifdef FXEDITOR
		// R_EndMeshVerts(&game::get_frontenddata()->codeMesh);
		utils::hook::call<void(__cdecl)(game::GfxMeshData* mesh)>(0x4E42B0)(&frontend_data->codeMesh);
#else
		utils::hook::call<void(__cdecl)(game::GfxMeshData* mesh)>(0x52B5E0)(&frontend_data->codeMesh);
#endif
	}


	void FX_GenerateVerts(FxGenerateVertsCmd* cmd)
	{
		R_BeginCodeMeshVerts();

		if (cmd->system->msecDraw >= 0 && game::Dvar_FindVar("fx_enable")->current.enabled && game::Dvar_FindVar("fx_draw")->current.enabled)
		{
			FX_DrawSpriteElems(cmd->system, cmd->system->msecDraw);
		}

		R_EndCodeMeshVerts();
	}


	// #ENV_DEPENDENT - Wrapper
	game::GfxPackedVertex* R_GetCodeMeshVerts(unsigned __int16 baseVertex)
	{
#ifdef FXEDITOR
		return utils::hook::call<game::GfxPackedVertex* (__cdecl)(__int16)>(0x4B6470)(baseVertex);
#else
		return utils::hook::call<game::GfxPackedVertex* (__cdecl)(__int16)>(0x52F6C0)(baseVertex);
#endif
	}


	bool R_ReserveCodeMeshIndices(int indexCount, r_double_index_t** indicesOut)
	{
#ifdef FXEDITOR
		return utils::hook::call<bool(__cdecl)(int, r_double_index_t**)>(0x4B61F0)(indexCount, indicesOut);
#else
		return utils::hook::call<bool(__cdecl)(int, r_double_index_t**)>(0x52F440)(indexCount, indicesOut);
#endif
	}


	// #ENV_DEPENDENT - Wrapper
	bool R_ReserveCodeMeshVerts(int vertCount, unsigned __int16* baseVertex)
	{
#ifdef FXEDITOR
		return utils::hook::call<bool(__cdecl)(int, unsigned __int16*)>(0x4B6250)(vertCount, baseVertex);
#else
		return utils::hook::call<bool(__cdecl)(int, unsigned __int16*)>(0x52F4A0)(vertCount, baseVertex);
#endif
	}


	// #ENV_DEPENDENT - Wrapper
	void R_AddCodeMeshDrawSurf(game::Material* material, r_double_index_t* indices, unsigned int indexCount, unsigned int argOffset, unsigned int argCount, const char* fxName)
	{
#ifdef FXEDITOR
		utils::hook::call<void(__cdecl)(game::Material*, r_double_index_t*, unsigned int, unsigned int, unsigned int, const char*)>(0x4B5990)(
			material, indices, indexCount, argOffset, argCount, fxName);
#else
		utils::hook::call<void(__cdecl)(game::Material*, r_double_index_t*, unsigned int, unsigned int, unsigned int, const char*)>(0x52EBE0)(
			material, indices, indexCount, argOffset, argCount, fxName);
#endif
	}
}