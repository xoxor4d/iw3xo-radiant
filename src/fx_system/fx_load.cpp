#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
						game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

#define AssertS(str)	if(IsDebuggerPresent()) __debugbreak();	else {	\
							game::Com_Error("%s\nLine %d :: %s\n%s ", str, __LINE__, __func__, __FILE__); }

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

namespace fx_system
{
	$145C5CACE7A579404A9D7C1B73F29F79 fx_load = {};

	const FxFlagDef s_allFlagDefs[41] =
	{
		{ "looping", 0, 1, 1 },
		{ "useRandColor", 0, 2, 2 },
		{ "useRandAlpha", 0, 4, 4 },
		{ "useRandSize0", 0, 8, 8 },
		{ "useRandSize1", 0, 0x10, 0x10 },
		{ "useRandScale", 0, 0x20, 0x20 },
		{ "useRandRotDelta", 0, 0x40, 0x40 },
		{ "modColorByAlpha", 0, 0x80, 0x80 },
		{ "useRandVel0", 0, 0x100, 0x100 },
		{ "useRandVel1", 0, 0x200, 0x200 },
		{ "useBackCompatVel", 0, 0x400, 0x400 },
		{ "absVel0", 0, 0x800, 0x800 },
		{ "absVel1", 0, 0x1000, 0x1000 },
		{ "playOnTouch", 0, 0x2000, 0x2000 },
		{ "playOnDeath", 0, 0x4000, 0x4000 },
		{ "playOnRun", 0, 0x8000, 0x8000 },
		{ "boundingSphere", 0, 0x10000, 0x10000 },
		{ "useItemClip", 0, 0x20000, 0x20000 },
		{ "disabled", 0, 0x80000000, 0x80000000 },
		{ "spawnRelative", 1, 2, 2 },
		{ "spawnFrustumCull", 1, 4, 4 },
		{ "runnerUsesRandRot", 1, 8, 8 },
		{ "spawnOffsetNone", 1, 0x30, 0 },
		{ "spawnOffsetSphere", 1, 0x30, 0x10 },
		{ "spawnOffsetCylinder", 1, 0x30, 0x20 },
		{ "runRelToWorld", 1, 0x0C0, 0 },
		{ "runRelToSpawn", 1, 0x0C0, 0x40 },
		{ "runRelToEffect", 1, 0x0C0, 0x80 },
		{ "runRelToOffset", 1, 0x0C0, 0x0C0 },
		{ "useCollision", 1, 0x100, 0x100 },
		{ "dieOnTouch", 1,0x200, 0x200 },
		{ "drawPastFog", 1, 0x400, 0x400 },
		{ "drawWithViewModel", 1, 0x800, 0x800 },
		{ "blocksSight", 1, 0x1000, 0x1000 },
		{ "modelUsesPhysics", 1, 0x8000000, 0x8000000 },
		{ "nonUniformScale", 1, 0x10000000, 0x10000000 },
		{ "startFixed", 2, 3, 0 },
		{ "startRandom", 2, 3, 1 },
		{ "startIndexed", 2, 3, 2 },
		{ "playOverLife", 2, 4, 4 },
		{ "loopOnlyNTimes", 2, 8, 8 }
	};

	// * --------------------------------

	void FX_UnregisterAll()
	{
		memset(&fx_load, 0, sizeof(fx_load));
	}

	game::XModel* FX_RegisterModel(const char* modelName)
	{
		return game::R_RegisterModel(modelName);
	}

	bool FX_RegisterAsset_Model(const char* name, FxElemVisuals* visuals)
	{
		visuals->anonymous = FX_RegisterModel(name);
		return visuals->anonymous != nullptr;
	}

	game::Material* FX_RegisterMaterial(const char* material)
	{
		const char* mat = material;

		if (!strcmp(material, "$default"))
		{
			mat = "$default3d";
		}

		return game::Material_RegisterHandle(mat, 6);
	}

	bool FX_RegisterMarkMaterials(const char* materialName, game::Material** materials)
	{
		bool success;
		const char* typePrefixes[2] =
		{
			"mc", "wc"
		};

		if (!strcmp(materialName, "$default"))
		{
			materialName = "$default3d";
		}

		success = true;
		for (int typeIndex = 0; typeIndex != 2; ++typeIndex)
		{
			char materialNameWithPrefix[256];
			sprintf_s(materialNameWithPrefix, 256, "%s/%s", typePrefixes[typeIndex], materialName);

			materials[typeIndex] = FX_RegisterMaterial(materialNameWithPrefix);
			if (!materials[typeIndex])
			{
				success = false;
			}
		}

		return success;
	}

	bool FX_RegisterAsset_DecalMaterials(const char* name, FxElemMarkVisuals* visuals)
	{
		return FX_RegisterMarkMaterials(name, visuals->materials);
	}

	bool FX_RegisterAsset_Material(const char* name, FxElemVisuals* visuals)
	{
		visuals->anonymous = FX_RegisterMaterial(name);
		return visuals->anonymous != nullptr;
	}

	bool FX_RegisterAsset_EffectDef(const char* name, FxElemVisuals* visuals)
	{
		visuals->anonymous = FX_Register(name);
		return visuals->anonymous != nullptr;
	}

	bool FX_RegisterAsset_SoundAliasName([[maybe_unused]] const char* name, [[maybe_unused]] FxElemVisuals* visuals)
	{
		// #UNFINISHED
		Assert();

		//ReplaceString(visuals, name);
		return true;
	}

	// * ------------------

	bool FX_ParseInt(const char** parse, int* value)
	{
		*value = Com_ParseInt(parse);
		return true;
	}

	bool FX_ParseIntRange(const char** parse, FxIntRange* range)
	{
		bool result = false;

		if (FX_ParseInt(parse, &range->base))
		{
			result = FX_ParseInt(parse, &range->amplitude) != 0;
		}

		return result;
	}

	bool FX_ParseFloat(const char** parse, float* value)
	{
		*value = Com_ParseFloat(parse);
		return true;
	}

	bool FX_ParseVector2(const char** parse, float* value)
	{
		bool result = false;

		if (FX_ParseFloat(parse, value))
		{
			result = FX_ParseFloat(parse, value + 1) != 0;
		}

		return result;
	}

	bool FX_ParseVector(const char** parse, float* value)
	{
		if (!FX_ParseFloat(parse, value))
		{
			return false;
		}

		if (FX_ParseFloat(parse, value + 1))
		{
			return FX_ParseFloat(parse, value + 2) != 0;
		}

		return false;
	}

	bool FX_ParseEffectRef(const char** parse, FxEffectDef** fx)
	{
		char* token = game::Com_Parse(parse);
		if (*token)
		{
			*fx = FX_Register(token);
		}
		else
		{
			*fx = nullptr;
		}

		return true;
	}

	bool FX_SetEditorElemType(FxEditorElemDef* edElemDef, char type)
	{
		if (edElemDef->elemType == FX_ELEM_TYPE_COUNT)
		{
			edElemDef->elemType = type;
			return true;
		}

		AssertS("More than one type of visuals present in effect element");
		return false;
	}

	FxCurve* FxCurve_AllocAndCreateWithKeys(const float* keyArray, int dimensionCount, int keyCount)
	{
		if (!keyArray || keyCount <= 0 || dimensionCount <= 0)
		{
			Assert();
		}
		
		const int	keySize = dimensionCount + 1;
		const bool	dims = keyArray[(dimensionCount + 1) * (keyCount - 1)] != 1.0f;
		const int	createdKeyCount = dims + keyCount + (keyArray[0] != 0.0f);

		if (createdKeyCount < 2)
		{
			Assert();
		}

		FxCurve* newCurve = (FxCurve*)malloc(4 * createdKeyCount * keySize + 12);
		if (!newCurve)
		{
			Assert();
		}

		newCurve->refCount = 1;
		newCurve->dimensionCount = dimensionCount;
		int keyIndex = 0;

		if (keyArray[0] != 0.0f)
		{
			newCurve->keys[0] = 0.0f;
			for (int elementIndex = 0; elementIndex != dimensionCount; ++elementIndex)
			{
				*((float*)&newCurve[1].dimensionCount + elementIndex) = keyArray[elementIndex + 1];
			}

			keyIndex = 1;
		}

		memcpy(&newCurve->keys[keySize * keyIndex], keyArray, 4 * keySize * keyCount);
		keyIndex = keyCount + keyIndex;

		if (dims)
		{
			newCurve->keys[keySize * keyIndex] = 1.0f;
			for (int elementIndex = 0; elementIndex != dimensionCount; ++elementIndex)
			{
				*((float*)&newCurve[1].dimensionCount + keySize * keyIndex + elementIndex) = keyArray[elementIndex + 1];
			}

			++keyIndex;
		}

		if (keyIndex != createdKeyCount)
		{
			Assert();
		}

		if (newCurve->keys[0] != 0.0f)
		{
			Assert();
		}

		if (newCurve->keys[keySize * (createdKeyCount - 1)] != 1.0f)
		{
			Assert();
		}

		newCurve->keyCount = keyCount;
		return newCurve;
	}

	bool FX_ParseCurve(const char** parse, int dimCount, float minValue, float maxValue, FxCurve** shape)
	{
		if (!game::Com_MatchToken(parse, "{", 1))
		{
			return false;
		}

		bool result;
		int valCount;
		float keys[512];

		for (valCount = 0;; ++valCount)
		{
			const char* token = game::Com_Parse(parse);
			if (*token == '}')
			{
				break;
			}

			if (valCount == 512)
			{
				Assert();
				return false;
			}

			keys[valCount] = static_cast<float>(atof(token));

			if (valCount % (dimCount + 1))
			{
				float val = maxValue;

				if (keys[valCount] - maxValue < 0.0f)
				{
					val = keys[valCount];
				}

				if (minValue - keys[valCount] >= 0.0f)
				{
					val = minValue;
				}

				keys[valCount] = val;
			}
		}

		if (valCount % (dimCount + 1))
		{
			Assert();
			result = false;
		}
		else
		{
			int keyCount = valCount / (dimCount + 1);
			if (keyCount >= 2)
			{
				if (keys[0] == 0.0f && keys[(dimCount + 1) * (keyCount - 1)] == 1.0f)
				{
					for (int keyIndex = 1; keyIndex < keyCount; ++keyIndex)
					{
						if (keys[(dimCount + 1) * (keyIndex - 1)] >= keys[keyIndex * (dimCount + 1)])
						{
							Assert();
							return false;
						}
					}

					*shape = FxCurve_AllocAndCreateWithKeys(keys, dimCount, keyCount);
					result = true;
				}
				else
				{
					Assert();
					return false;
				}
			}
			else
			{
				Assert();
				return false;
			}
		}

		return result;
	}

	bool FX_ParseSingleFlag(FxFlagOutputSet* flagOutputSet, const char* token)
	{
		for (auto flagDef = s_allFlagDefs; flagDef->name; ++flagDef)
		{
			int* outputFlag = flagOutputSet->flags[flagDef->flagType];
			if (outputFlag && !game::I_strcmp(token, flagDef->name))
			{
				*outputFlag &= ~flagDef->mask;
				*outputFlag |= flagDef->value;
				return true;
			}
		}

		AssertS(token);
		return false;
	}

	bool FX_ParseFlagsField(FxFlagOutputSet* flagOutputSet, const char** parse)
	{
		char* token;

		do
		{
			token = game::Com_Parse(parse);
			if (*token == ';')
			{
				game::Com_UngetToken();
				return true;
			}

		} while (FX_ParseSingleFlag(flagOutputSet, token));

		return false;
	}

	bool FX_ParseAssetArray_FxElemMarkVisuals_16_(const char** parse, char elemType, FxEditorElemDef* edElemDef, FxElemMarkVisuals(*visualsArray)[16], bool(__cdecl* RegisterAsset)(const char*, FxElemMarkVisuals*))
	{
		if (!FX_SetEditorElemType(edElemDef, elemType))
		{
			return false;
		}

		if (!game::Com_MatchToken(parse, "{", 1))
		{
			return false;
		}

		for (edElemDef->visualCount = 0;; ++edElemDef->visualCount)
		{
			const char* token = game::Com_Parse(parse);
			if (*token == '}')
			{
				return true;
			}

			if (edElemDef->visualCount == 16)
			{
				AssertS("More than 16 visuals in array");
				return false;
			}

			char name[260];
			game::I_strncpyz(name, token, 260);

			if (!RegisterAsset(name, &(*visualsArray)[edElemDef->visualCount]))
			{
				break;
			}
		}

		return false;
	}

	bool FX_ParseAssetArray_FxElemVisuals_32_(const char** parse, char elemType, FxEditorElemDef* edElemDef, FxElemVisuals(*visualsArray)[32], bool(__cdecl* RegisterAsset)(const char*, FxElemVisuals*))
	{
		if (!FX_SetEditorElemType(edElemDef, elemType))
		{
			return false;
		}
		if (!game::Com_MatchToken(parse, "{", 1))
		{
			return false;
		}
		for (edElemDef->visualCount = 0;; ++edElemDef->visualCount)
		{
			const char* token = game::Com_Parse(parse);
			if (*token == '}')
			{
				return true;
			}

			if (edElemDef->visualCount == 32)
			{
				AssertS("More than 32 visuals in array");
				return false;
			}

			char name[260];
			game::I_strncpyz(name, token, 260);

			if (!RegisterAsset(name, &(*visualsArray)[edElemDef->visualCount]))
			{
				break;
			}
		}

		return false;
	}


	bool FX_ParseName(const char** parse, FxEditorElemDef* edElemDef)
	{
		game::I_strncpyz(edElemDef->name, game::Com_Parse(parse), 48);
		return true;
	}

	bool FX_ParseNonAtlasFlags(const char** parse, FxEditorElemDef* edElemDef)
	{
		FxFlagOutputSet flagOutputSet = {};
		flagOutputSet.flags[0] = &edElemDef->editorFlags;
		flagOutputSet.flags[1] = &edElemDef->flags;
		flagOutputSet.flags[2] = nullptr;
		
		return FX_ParseFlagsField(&flagOutputSet, parse);
	}

	bool FX_ParseSpawnRange(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnRange.base);
	}

	bool FX_ParseFadeInRange(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->fadeInRange.base);
	}

	bool FX_ParseFadeOutRange(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->fadeOutRange.base);
	}

	bool FX_ParseSpawnFrustumCullRadius(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseFloat(parse, &edElemDef->spawnFrustumCullRadius);
	}

	bool FX_ParseSpawnDefLooping(const char** parse, FxEditorElemDef* edElemDef)
	{
		bool result = false;

		if (FX_ParseInt(parse, &edElemDef->spawnLooping.intervalMsec))
		{
			result = FX_ParseInt(parse, &edElemDef->spawnLooping.count) != 0;
		}
		
		return result;
	}

	bool FX_ParseSpawnDefOneShot(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseIntRange(parse, &edElemDef->spawnOneShot.count);
	}

	bool FX_ParseSpawnDelayMsec(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseIntRange(parse, &edElemDef->spawnDelayMsec);
	}

	bool FX_ParseLifeSpanMsec(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseIntRange(parse, &edElemDef->lifeSpanMsec);
	}

	bool FX_ParseSpawnOrgX(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnOrigin[0].base);
	}

	bool FX_ParseSpawnOrgY(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnOrigin[1].base);
	}

	bool FX_ParseSpawnOrgZ(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnOrigin[2].base);
	}

	bool FX_ParseSpawnOffsetRadius(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnOffsetRadius.base);
	}

	bool FX_ParseSpawnOffsetHeight(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnOffsetHeight.base);
	}

	bool FX_ParseSpawnAnglePitch(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnAngles[0].base);
	}

	bool FX_ParseSpawnAngleYaw(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnAngles[1].base);
	}

	bool FX_ParseSpawnAngleRoll(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->spawnAngles[2].base);
	}

	bool FX_ParseAngleVelPitch(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, (float*)edElemDef->angularVelocity);
	}

	bool FX_ParseAngleVelYaw(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->angularVelocity[1].base);
	}

	bool FX_ParseAngleVelRoll(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->angularVelocity[2].base);
	}

	bool FX_ParseInitialRot(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->initialRotation.base);
	}

	bool FX_ParseGravity(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->gravity.base);
	}

	bool FX_ParseElasticity(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->elasticity.base);
	}

	bool FX_ParseAtlasBehavior(const char** parse, FxEditorElemDef* edElemDef)
	{
		FxFlagOutputSet flagOutputSet = {};
		flagOutputSet.flags[0] = nullptr;
		flagOutputSet.flags[1] = nullptr;
		flagOutputSet.flags[2] = &edElemDef->atlas.behavior;
		
		return FX_ParseFlagsField(&flagOutputSet, parse);
	}

	bool FX_ParseAtlasIndex(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->atlas.index);
	}

	bool FX_ParseAtlasFps(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->atlas.fps);
	}

	bool FX_ParseAtlasLoopCount(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->atlas.loopCount);
	}

	bool FX_ParseAtlasColIndexBits(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->atlas.colIndexBits);
	}

	bool FX_ParseAtlasRowIndexBits(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->atlas.rowIndexBits);
	}

	bool FX_ParseAtlasEntryCount(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->atlas.entryCount);
	}

	bool FX_ParseGraphRange(const char** parse, int dimCount, float minValue, float maxValue, float* scale, FxCurve** shape)
	{
		if (!FX_ParseFloat(parse, scale))
		{
			return false;
		}

		if (!game::Com_MatchToken(parse, "{", 1))
		{
			return false;
		}

		if (!FX_ParseCurve(parse, dimCount, minValue, maxValue, shape))
		{
			return false;
		}

		if (FX_ParseCurve(parse, dimCount, minValue, maxValue, shape + 1))
		{
			return game::Com_MatchToken(parse, "}", 1) != 0;
		}

		return false;
	}

	bool FX_ParseVelGraph0X(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, -0.5f, 0.5f, &edElemDef->velScale[0][0], edElemDef->velShape[0][0]);
	}

	bool FX_ParseVelGraph0Y(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, -0.5f, 0.5f, &edElemDef->velScale[0][1], edElemDef->velShape[0][1]);
	}

	bool FX_ParseVelGraph0Z(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, -0.5f, 0.5f, &edElemDef->velScale[0][2], edElemDef->velShape[0][2]);
	}

	bool FX_ParseVelGraph1X(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, -0.5f, 0.5f, edElemDef->velScale[1], (FxCurve**)edElemDef->velShape[1]);
	}

	bool FX_ParseVelGraph1Y(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, -0.5f, 0.5f, &edElemDef->velScale[1][1], edElemDef->velShape[1][1]);
	}

	bool FX_ParseVelGraph1Z(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, -0.5f, 0.5f, &edElemDef->velScale[1][2], edElemDef->velShape[1][2]);
	}

	bool FX_ParseRotGraph(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, -0.5f, 0.5f, &edElemDef->rotationScale, edElemDef->rotationShape);
	}

	bool FX_ParseSizeGraph0(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, 0.0f, 1.0f, edElemDef->sizeScale, (FxCurve**)edElemDef->sizeShape);
	}

	bool FX_ParseSizeGraph1(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, 0.0f, 1.0f, &edElemDef->sizeScale[1], edElemDef->sizeShape[1]);
	}

	bool FX_ParseScaleGraph(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, 0.0f, 1.0f, &edElemDef->scaleScale, edElemDef->scaleShape);
	}

	bool FX_ParseColorGraph(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 3, 0.0f, 1.0f, (float*)&edElemDef, edElemDef->color);
	}

	bool FX_ParseLightingFrac(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseFloat(parse, &edElemDef->lightingFrac);
	}

	bool FX_ParseAlphaGraph(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseGraphRange(parse, 1, 0.0f, 1.0f, (float*)&edElemDef, edElemDef->alpha);
	}

	bool FX_ParseCollOffset(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector(parse, edElemDef->collOffset);
	}

	bool FX_ParseCollRadius(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseFloat(parse, &edElemDef->collRadius);
	}

	bool FX_ParseFxOnImpact(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseEffectRef(parse, &edElemDef->effectOnImpact);
	}

	bool FX_ParseFxOnDeath(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseEffectRef(parse, &edElemDef->effectOnDeath);
	}

	bool FX_ParseSortOrder(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->sortOrder);
	}

	bool FX_ParseEmission(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseEffectRef(parse, &edElemDef->emission);
	}

	bool FX_ParseEmitDist(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->emitDist.base);
	}

	bool FX_ParseEmitDistVariance(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseVector2(parse, &edElemDef->emitDistVariance.base);
	}

	bool FX_ParseTrailRepeatTime(const char** parse, [[maybe_unused]] FxEditorElemDef* edElemDef)
	{
		float deprecated;
		return FX_ParseFloat(parse, &deprecated);
	}

	bool FX_ParseTrailSplitDist(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->trailSplitDist);
	}

	bool FX_ParseTrailScrollTime(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseFloat(parse, &edElemDef->trailScrollTime);
	}

	bool FX_ParseTrailRepeatDist(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseInt(parse, &edElemDef->trailRepeatDist);
	}

	bool FX_ParseTrailDef(const char** parse, FxEditorElemDef* edElemDef)
	{
		if (!game::Com_MatchToken(parse, "{", 1))
		{
			return false;
		}

		const char* token;

		for (edElemDef->trailDef.vertCount = 0;; ++edElemDef->trailDef.vertCount)
		{
			token = (const char*)game::Com_Parse(parse);
			if (*token == '}')
			{
				break;
			}

			game::Com_UngetToken();
			if (edElemDef->trailDef.vertCount == 64)
			{
				AssertS("ran out of trail verts in edElemDef->trailDef");
				return false;
			}

			FxTrailVertex* trailVertex = &edElemDef->trailDef.verts[edElemDef->trailDef.vertCount];
			if (!FX_ParseFloat(parse, trailVertex->pos))
			{
				return false;
			}

			if (!FX_ParseFloat(parse, &trailVertex->pos[1]))
			{
				return false;
			}

			if (!FX_ParseFloat(parse, &trailVertex->texCoord))
			{
				return false;
			}
		}

		if (!game::Com_MatchToken(parse, "{", 1))
		{
			return false;
		}

		for (edElemDef->trailDef.indCount = 0;;++edElemDef->trailDef.indCount)
		{
			if (edElemDef->trailDef.indCount == 128)
			{
				AssertS("ran out of trail inds in edElemDef->trailDef");
				return false;
			}

			token = (const char*)game::Com_Parse(parse);
			if (*token == '}')
			{
				break;
			}

			game::Com_UngetToken();
			int index;

			if (!FX_ParseInt(parse, &index))
			{
				return false;
			}

			edElemDef->trailDef.inds[edElemDef->trailDef.indCount] = static_cast<std::uint16_t>(index);
			if (index != edElemDef->trailDef.inds[edElemDef->trailDef.indCount])
			{
				Assert();
			}
		}
		return true;
	}

	bool FX_ParseBillboardSprite(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_SPRITE_BILLBOARD, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_Material);
	}

	bool FX_ParseOrientedSprite(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_SPRITE_ORIENTED, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_Material);
	}

	bool FX_ParseCloud(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_CLOUD, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_Material);
	}

	bool FX_ParseTail(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_TAIL, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_Material);
	}

	bool FX_ParseTrail(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_TRAIL, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_Material);
	}

	bool FX_ParseDecal(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemMarkVisuals_16_(parse, FX_ELEM_TYPE_DECAL, edElemDef, (FxElemMarkVisuals(*)[16])&edElemDef->u, FX_RegisterAsset_DecalMaterials);
	}

	bool FX_ParseModel(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_MODEL, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_Model);
	}

	bool FX_ParseLight([[maybe_unused]] const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_SetEditorElemType(edElemDef, FX_ELEM_TYPE_OMNI_LIGHT);
	}

	bool FX_ParseSpotLight([[maybe_unused]] const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_SetEditorElemType(edElemDef, FX_ELEM_TYPE_SPOT_LIGHT);
	}

	bool FX_ParseRunner(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_RUNNER, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_EffectDef);
	}

	bool FX_ParseSound(const char** parse, FxEditorElemDef* edElemDef)
	{
		return FX_ParseAssetArray_FxElemVisuals_32_(parse, FX_ELEM_TYPE_SOUND, edElemDef, (FxElemVisuals(*)[32])&edElemDef->u, (bool(__cdecl*)(const char*, FxElemVisuals*))FX_RegisterAsset_SoundAliasName);
	}

	const FxElemField s_elemFields[69] =
	{
		{ "name", FX_ParseName },
		{ "editorFlags", FX_ParseNonAtlasFlags },
		{ "flags", FX_ParseNonAtlasFlags },
		{ "spawnRange",  FX_ParseSpawnRange },
		{ "fadeInRange",  FX_ParseFadeInRange },
		{ "fadeOutRange",  FX_ParseFadeOutRange },
		{ "spawnFrustumCullRadius",  FX_ParseSpawnFrustumCullRadius },
		{ "spawnLooping",  FX_ParseSpawnDefLooping },
		{ "spawnOneShot",  FX_ParseSpawnDefOneShot },
		{ "spawnDelayMsec",  FX_ParseSpawnDelayMsec },
		{ "lifeSpanMsec",  FX_ParseLifeSpanMsec },
		{ "spawnOrgX",  FX_ParseSpawnOrgX },
		{ "spawnOrgY",  FX_ParseSpawnOrgY },
		{ "spawnOrgZ",  FX_ParseSpawnOrgZ },
		{ "spawnOffsetRadius",  FX_ParseSpawnOffsetRadius },
		{ "spawnOffsetHeight",  FX_ParseSpawnOffsetHeight },
		{ "spawnAnglePitch",  FX_ParseSpawnAnglePitch },
		{ "spawnAngleYaw",  FX_ParseSpawnAngleYaw },
		{ "spawnAngleRoll",  FX_ParseSpawnAngleRoll },
		{ "angleVelPitch",  FX_ParseAngleVelPitch },
		{ "angleVelYaw",  FX_ParseAngleVelYaw },
		{ "angleVelRoll",  FX_ParseAngleVelRoll },
		{ "initialRot",  FX_ParseInitialRot },
		{ "gravity",  FX_ParseGravity },
		{ "elasticity",  FX_ParseElasticity },
		{ "atlasBehavior",  FX_ParseAtlasBehavior },
		{ "atlasIndex",  FX_ParseAtlasIndex },
		{ "atlasFps",  FX_ParseAtlasFps },
		{ "atlasLoopCount",  FX_ParseAtlasLoopCount },
		{ "atlasColIndexBits",  FX_ParseAtlasColIndexBits },
		{ "atlasRowIndexBits",  FX_ParseAtlasRowIndexBits },
		{ "atlasEntryCount",  FX_ParseAtlasEntryCount },
		{ "velGraph0X",  FX_ParseVelGraph0X },
		{ "velGraph0Y",  FX_ParseVelGraph0Y },
		{ "velGraph0Z",  FX_ParseVelGraph0Z },
		{ "velGraph1X",  FX_ParseVelGraph1X },
		{ "velGraph1Y",  FX_ParseVelGraph1Y },
		{ "velGraph1Z",  FX_ParseVelGraph1Z },
		{ "rotGraph",  FX_ParseRotGraph },
		{ "sizeGraph0",  FX_ParseSizeGraph0 },
		{ "sizeGraph1",  FX_ParseSizeGraph1 },
		{ "scaleGraph",  FX_ParseScaleGraph },
		{ "colorGraph",  FX_ParseColorGraph },
		{ "alphaGraph",  FX_ParseAlphaGraph },
		{ "lightingFrac",  FX_ParseLightingFrac },
		{ "collOffset",  FX_ParseCollOffset },
		{ "collRadius",  FX_ParseCollRadius },
		{ "fxOnImpact",  FX_ParseFxOnImpact },
		{ "fxOnDeath",  FX_ParseFxOnDeath },
		{ "sortOrder",  FX_ParseSortOrder },
		{ "emission",  FX_ParseEmission },
		{ "emitDist",  FX_ParseEmitDist },
		{ "emitDistVariance",  FX_ParseEmitDistVariance },
		{ "trailRepeatTime",  FX_ParseTrailRepeatTime },
		{ "trailSplitDist",  FX_ParseTrailSplitDist },
		{ "trailScrollTime",  FX_ParseTrailScrollTime },
		{ "trailRepeatDist",  FX_ParseTrailRepeatDist },
		{ "trailDef",  FX_ParseTrailDef },
		{ "billboardSprite",  FX_ParseBillboardSprite },
		{ "orientedSprite",  FX_ParseOrientedSprite },
		{ "cloud",  FX_ParseCloud },
		{ "tail",  FX_ParseTail },
		{ "trail",  FX_ParseTrail },
		{ "decal",  FX_ParseDecal },
		{ "model",  FX_ParseModel },
		{ "light",  FX_ParseLight },
		{ "spotLight",  FX_ParseSpotLight },
		{ "runner",  FX_ParseRunner },
		{ "sound",  FX_ParseSound }
	};

	// * --------------------------------

	void* FX_AllocMem(size_t size)
	{
		auto buff = malloc(size);
		if(buff)
		{
			memset(buff, 0, size);
		}
		else
		{
			Assert();
		}
		

		return buff;
	}

	int FX_HashName(const char* name)
	{
		char letter; 

		std::int16_t hash = 0;
		int scale = 119;

		while (*name)
		{
			letter = *name;

			if (*name == '\\')
			{
				letter = '/';
			}
			else if (letter >= 'A' && letter <= 'Z')
			{
				letter += 32;
			}

			hash += static_cast<std::int16_t>(scale * letter);
			++name;
			++scale;
		}
		return hash & 0x1FF;
	}

	int FX_GetHashIndex(const char* name, bool* exists)
	{
		if (!name || !exists)
		{
			Assert();
		}

		int hashIndex;

		for (hashIndex = FX_HashName(name); fx_load.effectDefs[hashIndex]; hashIndex = ((WORD)hashIndex + 1) & 0x1FF)
		{
			if (!game::I_stricmp(name, fx_load.effectDefs[hashIndex]->name))
			{
				*exists = true;
				return hashIndex;
			}
		}

		*exists = false;
		return hashIndex;
	}

	FxEffectDef* FX_Register_LoadObj(const char* name)
	{
		bool exists = false;
		const int hash_idx = FX_GetHashIndex(name, &exists);

		if (!exists)
		{
			fx_load.effectDefs[hash_idx] = FX_Load(name);
		}

		if (!fx_load.effectDefs[hash_idx])
		{
			Assert();
		}

		return fx_load.effectDefs[hash_idx];
	}

	FxEffectDef* FX_Register(const char* name)
	{
		return FX_Register_LoadObj(name);
	}

	void FX_RegisterDefaultEffect()
	{
		fx_load.defaultEffect = FX_Register("misc/missing_fx");
	}

	FxEffectDef* FX_LoadFailed(const char* name)
	{
		if (!fx_load.defaultEffect)
		{
			if (!game::I_stricmp(name, "misc/missing_fx"))
			{
				AssertS("Couldn't load default effect");
			}

			FX_RegisterDefaultEffect();
			if (!fx_load.defaultEffect || !fx_load.defaultEffect->name)
			{
				Assert();
			}

			if (game::I_strcmp(fx_load.defaultEffect->name, "misc/missing_fx"))
			{
				Assert();
			}
		}

		const char* def_name = &fx_load.defaultEffect->name[strlen(fx_load.defaultEffect->name) + 1];
		const size_t baseBytesNeeded = fx_load.defaultEffect->totalSize - (def_name - fx_load.defaultEffect->name);

		FxEffectDef* effectDef = (FxEffectDef*)FX_AllocMem(fx_load.defaultEffect->totalSize - (def_name - (fx_load.defaultEffect->name + 1)) + strlen(name));

		memcpy(effectDef, fx_load.defaultEffect, baseBytesNeeded);
		effectDef->name = (char*)effectDef + baseBytesNeeded;
		strcpy((char*)effectDef + baseBytesNeeded, name);

		const int relocationDistance = (char*)effectDef - (char*)fx_load.defaultEffect;
		effectDef->elemDefs = (FxElemDef*) ((char*)effectDef->elemDefs + relocationDistance);

		for (int elemIndex = 0; elemIndex < effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping; ++elemIndex)
		{
			effectDef->elemDefs[elemIndex].velSamples = (FxElemVelStateSample*)((char*)effectDef->elemDefs[elemIndex].velSamples + relocationDistance);
			effectDef->elemDefs[elemIndex].visSamples = (FxElemVisStateSample*)((char*)effectDef->elemDefs[elemIndex].visSamples + relocationDistance);
		}

		return effectDef;
	}

	bool FX_ParseEditorElemField(const char** parse, FxEditorElemDef* edElemDef, const char* token)
	{
		for (unsigned int fieldIndex = 0; fieldIndex < 69; ++fieldIndex)
		{
			if (!strcmp(token, s_elemFields[fieldIndex].keyName))
			{
				bool result = false;
				if (s_elemFields[fieldIndex].handler(parse, edElemDef))
				{
					result = game::Com_MatchToken(parse, ";", 1) != 0;
				}
				
				return result;
			}
		}

		Warning(0, "unkown field '%s'\n", token);
		return false;
	}

	bool FX_ParseEditorElem(int version, const char** parse, FxEditorElemDef* edElemDef)
	{
		memset(edElemDef->name, 0, sizeof(FxEditorElemDef));

		if (edElemDef->flags || edElemDef->editorFlags || edElemDef->atlas.behavior || edElemDef->lightingFrac != 0.0f)
		{
			Assert();
		}

		if (version < 2)
		{
			edElemDef->editorFlags = FX_ED_FLAG_BACKCOMPAT_VELOCITY;
		}

		edElemDef->elemType = FX_ELEM_TYPE_COUNT;
		edElemDef->sortOrder = 5;
		
		while (true)
		{
			char* token = game::Com_Parse(parse);
			if (*token == '}')
			{
				break;
			}

			if (!FX_ParseEditorElemField(parse, edElemDef, token))
			{
				return false;
			}
		}

		if (edElemDef->elemType != FX_ELEM_TYPE_COUNT)
		{
			return true;
		}

		Warning(0, "no visual type specified\n");
		return false;
	}

	bool FX_ParseEditorEffect(const char** parse, FxEditorEffectDef* edEffectDef)
	{
		bool result;
		char* token;

		int version = Com_ParseInt(parse);
		if (version <= 2)
		{
			edEffectDef->elemCount = 0;
			token = game::Com_Parse(parse);

			while (*parse)
			{
				if (*token != '{')
				{
					Warning(0, "Expected '{' to start a new segment, found '%s' instead.\n", token);
					return false;
				}

				if (edEffectDef->elemCount == 32)
				{
					Warning(0, "Cannot have more than %i segments.\n", edEffectDef->elemCount);
					return false;
				}

				if (!FX_ParseEditorElem(version, parse, &edEffectDef->elems[edEffectDef->elemCount]))
				{
					return false;
				}

				++edEffectDef->elemCount;
				token = game::Com_Parse(parse);
			}

			result = true;
		}
		else
		{
			Warning(0, "Version %i is too high. I can only handle up to %i.\n", version, 2);
			result = false;
		}

		return result;
	}

	bool FX_LoadEditorEffectFromBuffer(const char* buffer, const char* parseSessionName, FxEditorEffectDef* edEffectDef)
	{
		bool result = false;

		game::Com_BeginParseSession(parseSessionName);
		game::Com_SetSpaceDelimited(0);

#ifdef FXEDITOR
		game::Com_SetParseNegativeNumbers(1);
#else
		game::Com_SetParseNegativeNumbers();
#endif

		const char* parse = buffer;
		const char* token = game::Com_Parse(&parse);

		if (game::I_stricmp(token, "iwfx"))
		{
			AssertS("Effect needs to be updated from the legacy format.\n");
		}
		else
		{
			result = FX_ParseEditorEffect(&parse, edEffectDef);
		}

		game::Com_EndParseSession();
		return result;
	}

	bool FX_LoadEditorEffect(const char* name, FxEditorEffectDef* edEffectDef)
	{
		if (!name || !edEffectDef)
		{
			Assert();
		}

		char filename[64];
		sprintf_s(filename, 64, "fx/%s.efx", name);

		void* fileData;
		const int fileSize = game::FS_ReadFile(filename, &fileData);
		if (fileSize >= 0)
		{
			strcpy(edEffectDef->name, name);

			const bool result = FX_LoadEditorEffectFromBuffer((const char*)fileData, filename, edEffectDef);
			game::FS_FreeFile(fileData);

			return result;
		}

		Warning(21, "ERROR: effect '%s' not found\n", filename);
		return false;
	}

	FxEffectDef* FX_Load(const char* name)
	{
		auto* editorEffect = new FxEditorEffectDef();

		strcpy(editorEffect->name, name);
		
		if (FX_LoadEditorEffect(name, editorEffect))
		//if (utils::hook::call<bool(__cdecl)(const char*, FxEditorEffectDef*)>(0x477030)(name, editorEffect))
		{
			FxEffectDef* effectDef = FX_Convert(editorEffect, FX_AllocMem);
			delete editorEffect;

			if(effectDef)
			{
				return effectDef;
			}
		}
		else
		{
			delete editorEffect;
		}

		return FX_LoadFailed(name);
	}

}