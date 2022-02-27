#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace fx_system
{
	std::ofstream effect_file_buffer;

	const char* s_elemTypes[11] =
	{
		"billboardSprite",
		"orientedSprite",
		"tail",
		"trail",
		"cloud",
		"model",
		"light",
		"spotLight",
		"sound",
		"decal",
		"runner"
	};

	// * -----------------------------------------------

	void FX_WriteElemField(std::ofstream& buff, _In_z_ _Printf_format_string_ char const* const _format, ...)
	{
		va_list _arglist;
		char text_out[512];

		__crt_va_start(_arglist, _format);
		vsprintf(text_out, _format, _arglist);
		__crt_va_end(_arglist);

		buff << text_out;
	}

	void FX_WriteElemFieldStringArray(const char* name, std::ofstream& buff, int flags, int flag_type)
	{
		FX_WriteElemField(buff, "\t%s", name);

		for(const auto& flag : s_allFlagDefs)
		{
			if (flag.flagType == flag_type && (flag.mask & flags) == flag.value)
			{
				FX_WriteElemField(buff, " %s", flag.name);
				flags &= ~flag.mask;
			}
		}
		

		FX_WriteElemField(buff, ";\n");
		if (flags)
		{
			Assert();
		}
	}

	void FX_WriteElemField_Int(const char* args, int integer, std::ofstream& buff)
	{
		FX_WriteElemField(buff, "\t%s %i;\n", args, integer);
	}

	void FX_WriteElemField_IntRange(FxIntRange* ramge, std::ofstream& buff, const char* args)
	{
		FX_WriteElemField(buff, "\t%s %i %i;\n", args, ramge->base, ramge->amplitude);
	}

	void FX_WriteElemField_IntRange2(FxIntRange* range, std::ofstream& buff, const char* args)
	{
		FX_WriteElemField(buff, "\t%s %i %i;\n", args, range->base, range->amplitude);
	}

	void FX_WriteElemField_Float(const char* name, std::ofstream& buff, float value)
	{
		FX_WriteElemField(buff, "\t%s %g;\n", name, value);
	}

	void FX_WriteElemField_FloatRange(const FxFloatRange* range, std::ofstream& buff, const char* args)
	{
		FX_WriteElemField(buff, "\t%s %g %g;\n", args, range->base, range->amplitude);
	}

	void FX_WriteElemField_Vec3(float* vec3, std::ofstream& buff, const char* args)
	{
		FX_WriteElemField(buff, "\t%s %g %g %g;\n", args, vec3[0], vec3[1], vec3[2]);
	}

	void FX_WriteElemField_Curve(std::ofstream& buff, FxCurve* curve)
	{
		FX_WriteElemField(buff, "\t\t{\n");

		int valIndex = 0;
		for (int i = 0; i < curve->keyCount; ++i)
		{
			FX_WriteElemField(buff, "\t\t\t%g", curve->keys[valIndex]);
			int next = valIndex + 1;

			if (curve->dimensionCount > 0)
			{
				int dims = 0;
				float* key = &curve->keys[next];

				do
				{
					FX_WriteElemField(buff, " %g", *key);
					++next;
					++key;
					++dims;

				} while (dims < curve->dimensionCount);
			}

			FX_WriteElemField(buff, "\n");
			valIndex = next;
		}

		FX_WriteElemField(buff, "\t\t}\n");
		if (valIndex != curve->keyCount * (curve->dimensionCount + 1))
		{
			Assert();
		}
	}

	void FX_WriteElemField_Graph(const char* name, std::ofstream& buff, FxCurve** curve, float scale)
	{
		FX_WriteElemField(buff, "\t%s %g\n", name, scale);
		FX_WriteElemField(buff, "\t{\n");
		FX_WriteElemField_Curve(buff, curve[0]);
		FX_WriteElemField_Curve(buff, curve[1]);
		FX_WriteElemField(buff, "\t};\n");
	}

	void FX_WriteElemField_Effect(FxEffectDef* effectDef, std::ofstream& buff, const char* name)
	{
		if (effectDef)
		{
			FX_WriteElemField(buff, "\t%s \"%s\";\n", name, effectDef->name);
		}
		else
		{
			FX_WriteElemField(buff, "\t%s \"%s\";\n", name, "");
		}
	}

	void FX_WriteElemField_Traildef(std::ofstream& buff, FxEditorTrailDef* trail)
	{
		FX_WriteElemField(buff, "\ttrailDef\n\t{\n");

		if (trail->vertCount > 0)
		{
			for(int vert = 0; vert < trail->vertCount; vert++)
			{
				FX_WriteElemField(buff, "\t\t%g %g %g\n", trail->verts[vert].pos[0], trail->verts[vert].pos[1], trail->verts[vert].texCoord);
			}
		}

		FX_WriteElemField(buff, "\t} {\n");

		if (trail->indCount > 0)
		{
			for(int i = 0; i < trail->indCount; i++)
			{
				FX_WriteElemField(buff, "\t\t%i\n", trail->inds[i]);
			}
		}

		FX_WriteElemField(buff, "\t};\n");
	}

	const char* FX_GetDecalNameForElem(std::uint8_t type, FxElemMarkVisuals* visuals)
	{
		if (type != FX_ELEM_TYPE_DECAL)
		{
			Assert();
		}

		return Material_GetName(visuals->materials[0]);
	}

	const char* FX_GetVisualsNameForElem(FxElemVisuals* visuals, std::uint8_t type)
	{
		const char* name = "";

		if (!visuals->anonymous)
		{
			Assert();
		}

		switch (type)
		{
		case FX_ELEM_TYPE_SPRITE_BILLBOARD:
		case FX_ELEM_TYPE_SPRITE_ORIENTED:
		case FX_ELEM_TYPE_TAIL:
		case FX_ELEM_TYPE_TRAIL:
		case FX_ELEM_TYPE_CLOUD:
			name = Material_GetName(visuals->material);
			break;

		case FX_ELEM_TYPE_MODEL:
			name = visuals->model->name;
			break;

		case FX_ELEM_TYPE_SOUND:
			name = visuals->soundName;
			break;

		case FX_ELEM_TYPE_RUNNER:
			name = visuals->material->info.name;
			break;

		default:
			Assert();
			break;
		}

		return name;
	}

	void FX_WriteElemField_Type(FxEditorElemDef* elem, std::ofstream& buff)
	{
		if (elem->elemType >= FX_ELEM_TYPE_COUNT)
		{
			Assert();
		}

		if (elem->elemType == FX_ELEM_TYPE_OMNI_LIGHT || elem->elemType == FX_ELEM_TYPE_SPOT_LIGHT)
		{
			FX_WriteElemField(buff, "\t%s;\n", s_elemTypes[static_cast<std::uint8_t>(elem->elemType)]);
		}
		else
		{

			FX_WriteElemField(buff, "\t%s\n", s_elemTypes[elem->elemType]);
			FX_WriteElemField(buff, "\t{\n");

			for(auto vis = 0; vis < elem->visualCount; vis++)
			{
				if (elem->elemType == FX_ELEM_TYPE_DECAL)
				{
					FX_WriteElemField(buff, "\t\t\"%s\"\n", FX_GetDecalNameForElem(FX_ELEM_TYPE_DECAL, &elem->u.markVisuals[vis]));
				}
				else
				{
					FX_WriteElemField(buff, "\t\t\"%s\"\n", FX_GetVisualsNameForElem(&elem->u.visuals[vis], elem->elemType));
				}
			}

			FX_WriteElemField(buff, "\t};\n");
		}
	}

	void FX_EffectDef(std::ofstream& buff, FxEditorElemDef* elem, int flags)
	{
		FX_WriteElemField(buff, "{\n");
		FX_WriteElemField(buff, "\tname \"%s\";\n", elem->name);

		FX_WriteElemFieldStringArray("editorFlags", buff, flags & elem->editorFlags, 0);
		FX_WriteElemFieldStringArray("flags", buff, elem->flags, 1);

		FX_WriteElemField_FloatRange(&elem->spawnRange, buff, "spawnRange");
		FX_WriteElemField_FloatRange(&elem->fadeInRange, buff, "fadeInRange");
		FX_WriteElemField_FloatRange(&elem->fadeOutRange, buff, "fadeOutRange");

		FX_WriteElemField_Float("spawnFrustumCullRadius", buff, elem->spawnFrustumCullRadius);

		FX_WriteElemField_IntRange(reinterpret_cast<FxIntRange*>(&elem->spawnLooping), buff, "spawnLooping");

		FX_WriteElemField_IntRange2(&elem->spawnOneShot.count, buff, "spawnOneShot");
		FX_WriteElemField_IntRange2(&elem->spawnDelayMsec, buff, "spawnDelayMsec");
		FX_WriteElemField_IntRange2(&elem->lifeSpanMsec, buff, "lifeSpanMsec");

		FX_WriteElemField_FloatRange(elem->spawnOrigin, buff, "spawnOrgX");
		FX_WriteElemField_FloatRange(&elem->spawnOrigin[1], buff, "spawnOrgY");
		FX_WriteElemField_FloatRange(&elem->spawnOrigin[2], buff, "spawnOrgZ");
		FX_WriteElemField_FloatRange(&elem->spawnOffsetRadius, buff, "spawnOffsetRadius");
		FX_WriteElemField_FloatRange(&elem->spawnOffsetHeight, buff, "spawnOffsetHeight");
		FX_WriteElemField_FloatRange(elem->spawnAngles, buff, "spawnAnglePitch");
		FX_WriteElemField_FloatRange(&elem->spawnAngles[1], buff, "spawnAngleYaw");
		FX_WriteElemField_FloatRange(&elem->spawnAngles[2], buff, "spawnAngleRoll");
		FX_WriteElemField_FloatRange(elem->angularVelocity, buff, "angleVelPitch");
		FX_WriteElemField_FloatRange(&elem->angularVelocity[1], buff, "angleVelYaw");
		FX_WriteElemField_FloatRange(&elem->angularVelocity[2], buff, "angleVelRoll");
		FX_WriteElemField_FloatRange(&elem->initialRotation, buff, "initialRot");
		FX_WriteElemField_FloatRange(&elem->gravity, buff, "gravity");
		FX_WriteElemField_FloatRange(&elem->elasticity, buff, "elasticity");

		FX_WriteElemFieldStringArray("atlasBehavior", buff, elem->atlas.behavior, 2);

		FX_WriteElemField_Int("atlasIndex", elem->atlas.index, buff);
		FX_WriteElemField_Int("atlasFps", elem->atlas.fps, buff);
		FX_WriteElemField_Int("atlasLoopCount", elem->atlas.loopCount, buff);
		FX_WriteElemField_Int("atlasColIndexBits", elem->atlas.colIndexBits, buff);
		FX_WriteElemField_Int("atlasRowIndexBits", elem->atlas.rowIndexBits, buff);
		FX_WriteElemField_Int("atlasEntryCount", elem->atlas.entryCount, buff);

		FX_WriteElemField_Graph("velGraph0X", buff, elem->velShape[0][0], elem->velScale[0][0]);
		FX_WriteElemField_Graph("velGraph0Y", buff, elem->velShape[0][1], elem->velScale[0][1]);
		FX_WriteElemField_Graph("velGraph0Z", buff, elem->velShape[0][2], elem->velScale[0][2]);
		FX_WriteElemField_Graph("velGraph1X", buff, elem->velShape[1][0], elem->velScale[1][0]);
		FX_WriteElemField_Graph("velGraph1Y", buff, elem->velShape[1][1], elem->velScale[1][1]);
		FX_WriteElemField_Graph("velGraph1Z", buff, elem->velShape[1][2], elem->velScale[1][2]);
		FX_WriteElemField_Graph("rotGraph",   buff, elem->rotationShape, elem->rotationScale);
		FX_WriteElemField_Graph("sizeGraph0", buff, elem->sizeShape[0], elem->sizeScale[0]);
		FX_WriteElemField_Graph("sizeGraph1", buff, elem->sizeShape[1], elem->sizeScale[1]);
		FX_WriteElemField_Graph("scaleGraph", buff, elem->scaleShape, elem->scaleScale);
		FX_WriteElemField_Graph("colorGraph", buff, elem->color, 1.0f);
		FX_WriteElemField_Graph("alphaGraph", buff, elem->alpha, 1.0f);

		FX_WriteElemField_Float("lightingFrac", buff, elem->lightingFrac);

		FX_WriteElemField_Vec3(elem->collOffset, buff, "collOffset");
		FX_WriteElemField_Float("collRadius", buff, elem->collRadius);

		FX_WriteElemField_Effect(elem->effectOnImpact, buff, "fxOnImpact");
		FX_WriteElemField_Effect(elem->effectOnDeath, buff, "fxOnDeath");

		FX_WriteElemField_Int("sortOrder", elem->sortOrder, buff);

		FX_WriteElemField_Effect(elem->emission, buff, "emission");
		FX_WriteElemField_FloatRange(&elem->emitDist, buff, "emitDist");
		FX_WriteElemField_FloatRange(&elem->emitDistVariance, buff, "emitDistVariance");

		FX_WriteElemField_Int("trailSplitDist", elem->trailSplitDist, buff);
		FX_WriteElemField_Float("trailScrollTime", buff, elem->trailScrollTime);
		FX_WriteElemField_Int("trailRepeatDist", elem->trailRepeatDist, buff);

		if (elem->trailDef.indCount || elem->trailDef.vertCount)
		{
			FX_WriteElemField_Traildef(buff, &elem->trailDef);
		}

		FX_WriteElemField_Type(elem, buff);
		FX_WriteElemField(buff, "}\n");
	}

	int FX_BuildEditorEffectString(FxEditorEffectDef* edEffectDef, int flags, std::ofstream& buff)
	{
		if (!edEffectDef)
		{
			Assert();
		}

		FX_WriteElemField(buff, "%s %i\n\n", "iwfx", 2);
		int elem = 0;

		if (edEffectDef->elemCount > 0)
		{
			FxEditorElemDef* elems = edEffectDef->elems;
			do
			{
				FX_EffectDef(buff, elems, flags);
				++elem;
				++elems;

			} while (elem < edEffectDef->elemCount);
		}

		return 0;
	}

	bool FX_SaveEditorEffect()
	{
		const auto editor_effect = get_editor_effect();
		const std::string loaded_effect_string = get_loaded_effect_string();

		if (loaded_effect_string.empty())
		{
			return false;
		}

		effect_file_buffer.open(loaded_effect_string);

		if (effect_file_buffer.is_open())
		{
			FX_BuildEditorEffectString(editor_effect, 0x7FFFFFFF, effect_file_buffer);
			effect_file_buffer.close();

			return true;
		}

		return false;
	}

}