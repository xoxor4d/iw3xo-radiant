#include "std_include.hpp"

namespace components
{
	d3dbsp::bspGlob_s d3dbsp::comBspGlob = {};
	game::clipMap_t d3dbsp::cm = {};

	game::cplane_s* d3dbsp::CM_GetPlanes()
	{
		return d3dbsp::cm.planes;
	}

	int d3dbsp::CM_GetPlaneCount()
	{
		return d3dbsp::cm.planeCount;
	}

	bool d3dbsp::Com_IsBspLoaded()
	{
		return comBspGlob.header != nullptr;
	}

	std::uint32_t d3dbsp::Com_GetBspVersion()
	{
		if (!d3dbsp::Com_IsBspLoaded())
		{
			return 0;
		}

		return comBspGlob.header->version;
	}

	char* d3dbsp::Com_ValidateBspLumpData(LumpType type, unsigned int offset, unsigned int length, unsigned int elemSize, unsigned int* count)
	{
		char* result = nullptr;

		ASSERT_MSG(count, "count");
		
		if (length)
		{
			if (length + offset > comBspGlob.fileSize)
			{
				game::Com_Error("LoadMap: lump %i extends past end of file", type);
			}

			*count = length / elemSize;

			if (elemSize * *count != length)
			{
				game::Com_Error("LoadMap: lump %i has funny size", type);
			}

			result = (char*)comBspGlob.header + offset;
		}
		else
		{
			*count = 0;
		}

		return result;
	}

	bool d3dbsp::Com_GetBspLumpBool(LumpType type)
	{
		auto t_count = 0u;
		if(d3dbsp::Com_GetBspLump(type, 1, &t_count))
		{
			return true;
		}

		return false;
	}

	const void* d3dbsp::Com_GetBspLump(LumpType type, unsigned int elemSize, unsigned int* count)
	{
		const void* result = nullptr;
		unsigned int offset;

		ASSERT_MSG(Com_IsBspLoaded(), "Com_IsBspLoaded()");
		
		if (comBspGlob.header->version > 18)
		{
			offset = 8 * comBspGlob.header->chunkCount + 12;
			for (auto chunkIter = 0u; chunkIter < comBspGlob.header->chunkCount; ++chunkIter)
			{
				if (comBspGlob.header->chunks[chunkIter].type == type)
				{
					return Com_ValidateBspLumpData(type, offset, comBspGlob.header->chunks[chunkIter].length, elemSize, count);
				}

				offset += (comBspGlob.header->chunks[chunkIter].length + 3) & 0xFFFFFFFC;
			}

			*count = 0;
		}
		/*else if (type < Com_GetBspLumpCountForVersion(comBspGlob.header->version))
		{
			result = Com_ValidateBspLumpData(type, comBspGlob.header->chunks[type].type, *(&comBspGlob.header->chunkCount + 2 * type), elemSize, count);
		}*/
		else
		{
			*count = 0;
		}

		return result;
	}

	char* Com_GetHunkStringCopy(const char* string)
	{
		const auto len = strlen(string);
		auto hunkCopy = reinterpret_cast<char*>(game::Hunk_Alloc(len + 1));

		memcpy(hunkCopy, string, len + 1);
		return hunkCopy;
	}

	char* Com_GetLightDefName(const char* defName, game::ComPrimaryLight* primaryLights, unsigned int primaryLightCount)
	{
		for (auto primaryLightIndex = 0u; primaryLightIndex < primaryLightCount; ++primaryLightIndex)
		{
			if (primaryLights[primaryLightIndex].defName && !strcmp(defName, primaryLights[primaryLightIndex].defName))
			{
				return (char*)primaryLights[primaryLightIndex].defName;
			}
		}

		return Com_GetHunkStringCopy(defName);
	}

	game::ComPrimaryLight* Com_LoadPrimaryLights()
	{
		game::ComPrimaryLight* result;

		unsigned int primary_light_count;
		auto bsp_lights = (game::DiskPrimaryLight*)(d3dbsp::Com_GetBspLump(d3dbsp::LUMP_PRIMARY_LIGHTS, 128, &primary_light_count));

		auto asd = &bsp_lights[1];

		if (primary_light_count <= 1)
		{
			//Com_Error(ERR_DROP, "no primary lights in bsp\n");
		}

		game::comworld->primaryLightCount = primary_light_count;
		game::comworld->primaryLights = reinterpret_cast<game::ComPrimaryLight*>(game::Hunk_Alloc(sizeof(game::ComPrimaryLight) * primary_light_count));

		game::ComPrimaryLight* out = game::comworld->primaryLights;
		auto lightIndex = 0u;

		while (lightIndex < primary_light_count)
		{
			out->type = bsp_lights->type;
			out->canUseShadowMap = bsp_lights->canUseShadowMap;
			out->exponent = bsp_lights->exponent;
			out->color[0] = bsp_lights->color[0];
			out->color[1] = bsp_lights->color[1];
			out->color[2] = bsp_lights->color[2];
			out->dir[0] = bsp_lights->dir[0];
			out->dir[1] = bsp_lights->dir[1];
			out->dir[2] = bsp_lights->dir[2];
			out->origin[0] = bsp_lights->origin[0];
			out->origin[1] = bsp_lights->origin[1];
			out->origin[2] = bsp_lights->origin[2];
			out->radius = bsp_lights->radius;
			out->cosHalfFovOuter = bsp_lights->cosHalfFovOuter;
			out->cosHalfFovInner = bsp_lights->cosHalfFovInner;
			out->rotationLimit = bsp_lights->rotationLimit;
			out->translationLimit = bsp_lights->translationLimit;

			if (bsp_lights->type && bsp_lights->type != 1)
			{
				out->defName = Com_GetLightDefName(bsp_lights->defName, game::comworld->primaryLights, lightIndex);
				if (out->cosHalfFovOuter >= out->cosHalfFovInner)
				{
					out->cosHalfFovInner = out->cosHalfFovOuter * 0.75f + 0.25f;
				}
				if (out->rotationLimit == 1.0f)
				{
					out->cosHalfFovExpanded = out->cosHalfFovOuter;
				}
				else if (-out->cosHalfFovOuter < out->rotationLimit)
				{
					out->cosHalfFovExpanded = utils::vector::cos_of_sum_of_arc_cos(out->cosHalfFovOuter, out->rotationLimit);
				}
				else
				{
					out->cosHalfFovExpanded = -1.0f;
				}
			}
			else
			{
				out->defName = nullptr;
				out->cosHalfFovExpanded = bsp_lights->cosHalfFovOuter;
			}

			++lightIndex;
			++bsp_lights;
			result = ++out;
		}

		return result;
	}

	bool d3dbsp::Com_LoadBsp(const char* filename)
	{
		int h;
		comBspGlob.fileSize = game::FS_FOpenFileRead(filename, &h);

		if(!h)
		{
			game::printf_to_console("[ERR][BSP] Couldn't open %s\n", filename);
			return false;
		}

		comBspGlob.header = reinterpret_cast<BspHeader*>(game::Z_Malloc(comBspGlob.fileSize));

		unsigned int bytesRead;
		bytesRead = game::FS_Read(comBspGlob.header, comBspGlob.fileSize, h);
		game::FS_FCloseFile(h);

		if (bytesRead != comBspGlob.fileSize)
		{
			free(comBspGlob.header);
			game::printf_to_console("[ERR][BSP] bytesRead != comBspGlob.fileSize");
			return false;
		}

		// skip checksum and version checks for now

		memcpy(comBspGlob.name, filename, strlen(filename) + 1);

		return d3dbsp::Com_IsBspLoaded();
	}

	void R_LoadPrimaryLights(unsigned int bspVersion)
	{
		if (bspVersion > 14)
		{
			game::s_world->primaryLightCount = game::comworld->primaryLightCount;//Com_GetPrimaryLightCount();
			if (game::s_world->primaryLightCount <= 1)
			{
				game::s_world->sunPrimaryLightIndex = 0;
			}
			else
			{
				game::s_world->sunPrimaryLightIndex = game::comworld->primaryLights[1].type == 1; //Com_GetPrimaryLight(1u)->type == 1;
			}
			for (auto lightIndex = 0u; lightIndex < game::s_world->primaryLightCount; ++lightIndex)
			{
				game::ComPrimaryLight* primaryLight = &game::comworld->primaryLights[lightIndex]; //Com_GetPrimaryLight(lightIndex);
				if (primaryLight->defName)
				{
					utils::hook::call<game::GfxLightDef*(__cdecl)(const char*)>(0x53D510)(primaryLight->defName);
				}
			}
		}
		else
		{
			game::s_world->sunPrimaryLightIndex = 1;
			game::s_world->primaryLightCount = 2;
		}
	}

	game::GfxLight d3dbsp::scene_lights[255] = {};

	void R_InitPrimaryLights(game::GfxLight* primaryLights)
	{
		game::GfxLight* out; // [esp+70h] [ebp-Ch]
		game::ComPrimaryLight* in; // [esp+74h] [ebp-8h]
		int savedregs; // [esp+7Ch] [ebp+0h] BYREF

		
		for (auto lightIndex = 0u; lightIndex < game::s_world->primaryLightCount; ++lightIndex)
		{
			in = &game::comworld->primaryLights[lightIndex]; //Com_GetPrimaryLight(lightIndex);
			out = &primaryLights[lightIndex];
			out->type = in->type;
			out->canUseShadowMap = in->canUseShadowMap;
			out->color[0] = in->color[0];
			out->color[1] = in->color[1];
			out->color[2] = in->color[2];
			out->dir[0] = in->dir[0];
			out->dir[1] = in->dir[1];
			out->dir[2] = in->dir[2];
			out->origin[0] = in->origin[0];
			out->origin[1] = in->origin[1];
			out->origin[2] = in->origin[2];
			out->radius = in->radius;
			out->cosHalfFovOuter = in->cosHalfFovOuter;
			out->cosHalfFovInner = in->cosHalfFovInner;
			out->exponent = (unsigned __int8)in->exponent;
			if (in->defName)
			{
				out->def = utils::hook::call<game::GfxLightDef* (__cdecl)(const char*)>(0x53D510)(in->defName);
			}
			else
			{
				out->def = nullptr;
			}

			if (out->type == 2)
			{
				//SpotLightViewMatrix((unsigned int)&savedregs, out->dir, out->angles[2], out->viewMatrix.m);
				//SpotLightProjectionMatrix(out->cosHalfFovOuter, out->falloff[0], out->falloff[1], out->projMatrix.m);
			}
		}

		if (game::s_world->sunPrimaryLightIndex)
		{
			//memcpy(&primaryLights[game::s_world->sunPrimaryLightIndex], game::rgp->world->sunLight, sizeof(game::GfxLight));
		}
	}

	void CMod_LoadPlanes()
	{
		unsigned int count;
		auto in = static_cast<const game::dplane_t*>(d3dbsp::Com_GetBspLump(d3dbsp::LUMP_PLANES, sizeof(game::dplane_t), &count));
		if (!count)
		{
			game::Com_Error("CMod_LoadPlanes: Map with no planes");
		}
		if (count > 0x10000)
		{
			game::Com_Error("CMod_LoadPlanes: Number of planes exceeds 65536");
		}

		d3dbsp::cm.planes = reinterpret_cast<game::cplane_s*>(game::Hunk_Alloc(sizeof(game::cplane_s) * count));
		d3dbsp::cm.planeCount = count;

		game::cplane_s* out = d3dbsp::cm.planes;

		/*for (auto i = out; count; --count)
		{
			char v5 = 0;
			i->normal[0] = in->normal[0];
			if (in->normal[0] < 0.0f)
			{
				v5 = 1;
			}

			i->normal[1] = in->normal[1];
			if (in->normal[1] < 0.0f)
			{
				v5 |= 2u;
			}

			i->normal[2] = in->normal[2];
			if (in->normal[2] < 0.0f)
			{
				v5 |= 4u;
			}

			i->dist = in->dist;

			char n0;

			if (1.0f == i->normal[0])
			{
				n0 = 0;
			}
			else if (1.0f == i->normal[1])
			{
				n0 = 1;
			}
			else
			{
				n0 = 2;
				if (1.0f != i->normal[2])
				{
					n0 = 3;
				}
			}
			i->type = n0;
			i->signbits = v5;
			++in;
			++i;
		}*/

		for (auto planeIter = 0u; planeIter < count; ++planeIter)
		{
			char bits = 0;

			for (auto axisIter = 0u; axisIter < 3; ++axisIter)
			{
				out->normal[axisIter] = in->normal[axisIter];
				if (out->normal[axisIter] < 0.0f)
				{
					bits |= 1 << axisIter;
				}
			}

			out->dist = in->dist;
			char n0;

			if (out->normal[0] == 1.0f)
			{
				n0 = 0;
			}
			else
			{
				char n1;
				if (out->normal[1] == 1.0f)
				{
					n1 = 1;
				}
				else
				{
					char n2;
					if (out->normal[2] == 1.0f)
					{
						n2 = 2;
					}
					else
					{
						n2 = 3;
					}

					n1 = n2;
				}

				n0 = n1;
			}

			out->type = n0;
			out->signbits = bits;
			++in;
			++out;
		}
	}

	void load_test_bsp()
	{
		if(d3dbsp::Com_LoadBsp("mp_shadertest.d3dbsp"))
		{
			// load cm
			// link cm

			// CM_LoadMapFromBsp
			d3dbsp::cm.name = Com_GetHunkStringCopy("mp_shadertest");
			CMod_LoadPlanes();
			d3dbsp::cm.isInUse = 1;


			// load world
			Com_LoadPrimaryLights();
			game::comworld->name = Com_GetHunkStringCopy("mp_shadertest");
			game::comworld->isInUse = true;
			//game::rgp->world = game::s_world;

			// R_LoadPrimaryLights is missing in R_LoadWorldInternal (s_world ...)
			R_LoadPrimaryLights(d3dbsp::Com_GetBspVersion());
			R_InitPrimaryLights(d3dbsp::scene_lights);

			unsigned int checksum;
			utils::hook::call<void(__cdecl)(const char* _name, unsigned int* _checksum, int _savegame)>(0x52E450)("mp_shadertest", &checksum, 0); // R_LoadWorld
		}
	}
	

	d3dbsp::d3dbsp()
	{
		// #
		// R_LoadWorldInternal patches

		// disable check that checks the "r_useLayeredMaterials" dvar (not implemented)
		//utils::hook::nop(0x5101BD, 10);

		// re-implement a LUMP_UNLAYERED_TRIANGLES check
		utils::hook::detour(0x4161B0, d3dbsp::Com_GetBspLumpBool, HK_JUMP);

		// re-implement Com_GetBspVersion
		utils::hook::detour(0x416150, d3dbsp::Com_GetBspVersion, HK_JUMP);

		// re-implement Com_GetBspLump
		utils::hook::detour(0x416180, d3dbsp::Com_GetBspLump, HK_JUMP);

		// re-implement CM_GetPlanes
		utils::hook::detour(0x4161E0, d3dbsp::CM_GetPlanes, HK_JUMP);

		// re-implement CM_GetPlaneCount
		utils::hook::detour(0x4161F0, d3dbsp::CM_GetPlaneCount, HK_JUMP);


		// dirty hacks

		utils::hook::nop(0x5525A2, 5);
		utils::hook::nop(0x5101A4, 5); // memset of s_world in R_LoadWorldInternal

		utils::hook::nop(0x41625A, 5); // CM_BoxSightTrace Assert
		utils::hook::nop(0x56C8DC, 5); // bspSurf->material->info.name) = %s", "(!lightmapSecondaryFlag) Assert

		// RB_FullbrightDrawCommands : R_RENDERTARGET_SCENE to FRAMEBUFFER
		//utils::hook::nop(0x55B695, 5); // R_SetAndClearSceneTarget

		//utils::hook::set<BYTE>(0x55B733 + 1, 1);
		//utils::hook::set<BYTE>(0x55B73E + 1, 1);
		//utils::hook::set<BYTE>(0x55B5D0 + 1, 1);

		// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players (disable all culling)
		utils::hook::nop(0x56998B, 3);

		// R_AddWorldSurfacesPortalWalk :: less culling :: 0x7C -> 0xEB (jl -> jmp)
		utils::hook::set<BYTE>(0x527370, 0xEB);

		// R_AddAabbTreeSurfacesInFrustum_r :: less culling :: 0x74 -> 0xEB (je to jmp)
		utils::hook::set<BYTE>(0x555C4C, 0xEB);

		// less culling: 0x527370 -> 0x7C -> 0xEB (jl -> jmp)
		// nop 0x00555C1B ?
		// 0x555C4C -> 0x74 -> 0xEB (je to jmp)
		

		// 

		command::register_command("offset"s, [](auto)
		{
			auto off1 = offsetof(game::GfxViewInfo, localClientNum);
			game::printf_to_console("GfxViewInfo::localClientNum %d", off1);
		});

		command::register_command("bsp"s, [](auto)
		{
			load_test_bsp();
		});
	}

	d3dbsp::~d3dbsp()
	{ }
}
