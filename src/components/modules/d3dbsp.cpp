#include "std_include.hpp"

namespace components
{
	// *
	// * Entity Handling
	// *

	int						d3dbsp::radiant_dobj_count = 0;
	d3dbsp::radiant_dobj_s	d3dbsp::radiant_dobj[512] = {};

	void d3dbsp::dobj_clear_list()
	{
		memset(d3dbsp::radiant_dobj, 0, sizeof(d3dbsp::radiant_dobj));
		d3dbsp::radiant_dobj_count = 0;
	}

	// hack but works
	unsigned short lightinghandle = 0;

	// R_AddDObjToScene
	void dobj_add_to_scene(game::DObj_s* obj, game::cpose_t* pose, unsigned int ent_num, float* lighting_origin)
	{
		if (!obj)
		{
			Assert();
		}

		if (!pose)
		{
			Assert();
		}

		unsigned int& gfxCfg_entCount = *reinterpret_cast<unsigned int*>(0x13683DC);
		//DWORD& scene_dpvs_sceneDObjIndex = *reinterpret_cast<DWORD*>(0x14C564C);
		DWORD& scene_dpvs_sceneXModelIndex = *reinterpret_cast<DWORD*>(0x14C5648);

		if (ent_num >= gfxCfg_entCount)
		{
			Assert();
		}

		if (game::Dvar_FindVar("r_drawEntities")->current.enabled)
		{
			const unsigned int scene_ent_index = utils::hook::call<unsigned int(__cdecl)()>(0x501D80)(); // R_AllocSceneModel()
			if (scene_ent_index < 1024)
			{
				game::GfxSceneModel* sceneModel = &game::scene->sceneModel[scene_ent_index];
				sceneModel->model = obj->models[0];
				sceneModel->DObj_s_obj = obj;
				sceneModel->entnum = static_cast<unsigned short>(ent_num);
				sceneModel->cachedLightingHandle = &lightinghandle;
				sceneModel->placement.base.origin[0] = pose->origin[0];
				sceneModel->placement.base.origin[1] = pose->origin[1];
				sceneModel->placement.base.origin[2] = pose->origin[2];

				game::AnglesToQuat(pose->angles, sceneModel->placement.base.quat);

				sceneModel->placement.scale = 1.0f;
				sceneModel->radius = sceneModel->model->radius;
				sceneModel->lightingOrigin[0] = *lighting_origin;
				sceneModel->lightingOrigin[1] = lighting_origin[1];
				sceneModel->lightingOrigin[2] = lighting_origin[2];
				sceneModel->gfxEntIndex = 0;

				*((WORD*)scene_dpvs_sceneXModelIndex + ent_num) = static_cast<unsigned short>(scene_ent_index);
			}
		}
	}

	void dobj_get_lighting_origin(game::cpose_t* pose, float* lighting_origin)
	{
		lighting_origin[0] = pose->origin[0];
		lighting_origin[1] = pose->origin[1];
		lighting_origin[2] = pose->origin[2] + 4.0f;
	}
	
	void d3dbsp::add_entities_to_scene()
	{
		for (auto i = 0; i < radiant_dobj_count; i++)
		{
			game::vec3_t lighting_origin = {};
			dobj_get_lighting_origin(&radiant_dobj[i].pose, lighting_origin);
			dobj_add_to_scene(&radiant_dobj[i].obj, &radiant_dobj[i].pose, 0, lighting_origin);
		}
	}


	// *
	// * BSP Loading
	// *
	
	std::string			d3dbsp::loaded_bsp_path;
	d3dbsp::bspGlob_s	d3dbsp::comBspGlob = {};
	game::clipMap_t		d3dbsp::cm = {};
	game::GfxLight		d3dbsp::scene_lights[255] = {};


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
		if (d3dbsp::Com_GetBspLump(type, 1, &t_count))
		{
			return true;
		}

		return false;
	}

#pragma warning(push)
#pragma warning(disable: 4146)
	void d3dbsp::Com_SaveLump(LumpType type, const void* newLump, unsigned int size)
	{
		const void* chunkData[100];

		BspHeader new_header = {};
		new_header.ident = 1347633737;
		new_header.version = 22;
		new_header.chunkCount = 0;

		bool isNewChunk = true;
		unsigned int offset = 8 * comBspGlob.header->chunkCount + 12;

		for (auto chunkIter = 0u; chunkIter < comBspGlob.header->chunkCount; ++chunkIter)
		{
			const BspChunk* chunk = &comBspGlob.header->chunks[chunkIter];
			if (chunk->type == type)
			{
				isNewChunk = false;
				if (size)
				{
					new_header.chunks[new_header.chunkCount].type = type;
					new_header.chunks[new_header.chunkCount].length = size;
					chunkData[new_header.chunkCount++] = newLump;
				}
			}
			else
			{
				new_header.chunks[new_header.chunkCount].type = chunk->type;
				new_header.chunks[new_header.chunkCount].length = chunk->length;
				chunkData[new_header.chunkCount++] = (char*)comBspGlob.header + offset;
			}

			offset += (chunk->length + 3) & 0xFFFFFFFC;
		}
		if (isNewChunk && size)
		{
			new_header.chunks[new_header.chunkCount].type = type;
			new_header.chunks[new_header.chunkCount].length = size;
			chunkData[new_header.chunkCount++] = newLump;
		}

		if (const auto h = game::FS_OpenFileOverwrite(comBspGlob.name); h)
		{
			game::FS_Write(&new_header, 8 * new_header.chunkCount + 12, h);
			for (auto chunkIter = 0u; chunkIter < new_header.chunkCount; ++chunkIter)
			{
				game::FS_Write(chunkData[chunkIter], new_header.chunks[chunkIter].length, h);
				const unsigned int zeroCount = -new_header.chunks[chunkIter].length & 3;

				if (zeroCount)
				{
					unsigned int zero;
					game::FS_Write(&zero, zeroCount, h);
				}
					
			}
			
			game::FS_FCloseFile(h);
			d3dbsp::radiant_load_bsp(d3dbsp::loaded_bsp_path.c_str(), true);
		}
		else
		{
			game::Com_Error("Failed to open file %s for writing", comBspGlob.name);
		}
	}
#pragma warning(pop)
	
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
		else
		{
			*count = 0;
		}

		return result;
	}

	const char* Com_GetHunkStringCopy(const char* string)
	{
		const auto len = strlen(string);
		const auto hunk_copy = reinterpret_cast<char*>(game::Hunk_Alloc(len + 1));

		memcpy(hunk_copy, string, len + 1);
		return hunk_copy;
	}

	const char* Com_GetLightDefName(const char* defName, game::ComPrimaryLight* primaryLights, unsigned int primaryLightCount)
	{
		for (auto primaryLightIndex = 0u; primaryLightIndex < primaryLightCount; ++primaryLightIndex)
		{
			if (primaryLights[primaryLightIndex].defName && !strcmp(defName, primaryLights[primaryLightIndex].defName))
			{
				return primaryLights[primaryLightIndex].defName;
			}
		}

		return Com_GetHunkStringCopy(defName);
	}

	void Com_LoadPrimaryLights()
	{
		unsigned int primary_light_count;
		auto bsp_lights = (game::DiskPrimaryLight*)(d3dbsp::Com_GetBspLump(d3dbsp::LUMP_PRIMARY_LIGHTS, 128, &primary_light_count));

		if (primary_light_count <= 1)
		{
			game::Com_Error("Com_LoadPrimaryLights: no primary lights in bsp\n");
		}

		game::comworld->primaryLightCount = primary_light_count;
		game::comworld->primaryLights = reinterpret_cast<game::ComPrimaryLight*>(game::Hunk_Alloc(sizeof(game::ComPrimaryLight) * primary_light_count));

		game::ComPrimaryLight* out = game::comworld->primaryLights;
		auto lightIndex = 0u;

		while (lightIndex < primary_light_count)
		{
			out->type = bsp_lights->type;
			out->canUseShadowMap = bsp_lights->canUseShadowMap;
			out->exponent = static_cast<char>(bsp_lights->exponent);
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
			++out;
		}
	}
	
	void R_LoadPrimaryLights(unsigned int bsp_version)
	{
		if (bsp_version > 14)
		{
			game::s_world->primaryLightCount = game::comworld->primaryLightCount; // Com_GetPrimaryLightCount();

			if (game::s_world->primaryLightCount <= 1)
			{
				game::s_world->sunPrimaryLightIndex = 0;
			}
			else
			{
				game::s_world->sunPrimaryLightIndex = game::comworld->primaryLights[1].type == 1; // Com_GetPrimaryLight(1u)->type == 1;
			}

			for (auto light_index = 0u; light_index < game::s_world->primaryLightCount; ++light_index)
			{
				const auto primary_light = &game::comworld->primaryLights[light_index]; // Com_GetPrimaryLight(lightIndex);
				if (primary_light->defName)
				{
					game::R_RegisterLightDef(primary_light->defName);
				}
			}
		}
		else
		{
			game::s_world->sunPrimaryLightIndex = 1;
			game::s_world->primaryLightCount = 2;
		}
	}

	void R_InitPrimaryLights(game::GfxLight* primaryLights)
	{
		for (auto lightIndex = 0u; lightIndex < game::s_world->primaryLightCount; ++lightIndex)
		{
			const auto in = &game::comworld->primaryLights[lightIndex]; //Com_GetPrimaryLight(lightIndex);
			const auto out = &primaryLights[lightIndex];

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
			out->exponent = static_cast<std::uint8_t>(in->exponent);
			out->def = nullptr;

			if (in->defName)
			{
				out->def = game::R_RegisterLightDef(in->defName);
			}
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
		d3dbsp::cm.planeCount = static_cast<int>(count);

		game::cplane_s* out = d3dbsp::cm.planes;

		for (auto plane = 0u; plane < count; ++plane)
		{
			char bits = 0;

			for (auto axis = 0u; axis < 3; ++axis)
			{
				out->normal[axis] = in->normal[axis];
				if (out->normal[axis] < 0.0f)
				{
					bits |= 1 << axis;
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

	void CMod_LoadCollisionVertsAndTris()
	{
		unsigned int vert_count;
		const auto verts = d3dbsp::Com_GetBspLump(d3dbsp::LUMP_COLLISIONVERTS, sizeof(float[3]), &vert_count);

		d3dbsp::cm.verts = reinterpret_cast<float(*)[3]>(game::Hunk_Alloc(sizeof(float[3]) * vert_count));
		d3dbsp::cm.vertCount = static_cast<int>(vert_count);
		memcpy(d3dbsp::cm.verts, verts, sizeof(float[3]) * vert_count);


		// #
		// CMod_LoadCollisionTriangles();

		unsigned int tri_count;
		const auto tris = d3dbsp::Com_GetBspLump(d3dbsp::LUMP_COLLISIONTRIS, sizeof(uint16_t), &tri_count);

		d3dbsp::cm.triIndices = reinterpret_cast<uint16_t*>(game::Hunk_Alloc(sizeof(uint16_t) * tri_count));
		d3dbsp::cm.triCount = static_cast<int>(tri_count) / 3;
		memcpy(d3dbsp::cm.triIndices, tris, sizeof(uint16_t) * tri_count);
	}

	const char* Com_EntityString(unsigned int* num_entity_chars)
	{
		if (!d3dbsp::Com_IsBspLoaded())
		{
			Assert();
		}

		unsigned int count;
		const char* entity_string = static_cast<const char*>(d3dbsp::Com_GetBspLump(d3dbsp::LUMP_ENTITIES, 1u, &count));

		if (num_entity_chars)
		{
			*num_entity_chars = count;
		}

		return entity_string;
	}

	game::MapEnts* MapEnts_GetFromString(const char* name, const char* entity_string, int num_entity_chars)
	{
		const auto ents = reinterpret_cast<game::MapEnts*>(game::Hunk_Alloc(sizeof(game::MapEnts)));
		ents->name = Com_GetHunkStringCopy(name);

		ents->entityString = reinterpret_cast<char*>(game::Hunk_Alloc(num_entity_chars));
		memcpy(ents->entityString, entity_string, num_entity_chars);
		ents->numEntityChars = num_entity_chars;

		return ents;
	}

	void CMod_LoadEntityString()
	{
		unsigned int numEntityChars;

		const char* entityString = Com_EntityString(&numEntityChars);
		d3dbsp::cm.mapEnts = MapEnts_GetFromString(d3dbsp::cm.name, entityString, numEntityChars);
	}

	bool d3dbsp::Com_LoadBsp(const char* filename)
	{
		int h;
		comBspGlob.fileSize = game::FS_FOpenFileRead(filename, &h);

		if (!h)
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
			game::Z_Free(comBspGlob.header);
			game::printf_to_console("[ERR][BSP] bytesRead != comBspGlob.fileSize");
			return false;
		}

		// skip checksum and version checks for now

		memcpy(comBspGlob.name, filename, strlen(filename) + 1);

		return d3dbsp::Com_IsBspLoaded();
	}

	void d3dbsp::shutdown_bsp()
	{
		d3dbsp::dobj_clear_list();

		comBspGlob.loadedLumpData = nullptr;
		
		game::Z_Free(comBspGlob.header);

		comBspGlob.header = nullptr;
		comBspGlob.name[0] = 0;

		memset(&d3dbsp::cm, 0, sizeof(d3dbsp::cm));
		memset(&d3dbsp::scene_lights, 0, sizeof(d3dbsp::scene_lights));

		game::comworld->isInUse = false;

		cdeclcall(void, 0x52E7D0); // R_ShutdownWorld
		game::rgp->world = nullptr;

		memset(game::s_world, 0, 0x2D0 /*sizeof(game::GfxWorld)*/);

		if (game::rg->registered)
		{
			cdeclcall(void, 0x529D50); // R_ResetModelLighting
		}
	}

	bool d3dbsp::radiant_load_bsp(const char* bsppath, bool reload)
	{
		const auto con = GetConsoleWindow();

		if (d3dbsp::Com_IsBspLoaded())
		{
			d3dbsp::shutdown_bsp();
		}

		if (d3dbsp::Com_LoadBsp(bsppath))
		{
			if (!reload) ShowWindow(con, SW_SHOW);
			game::printf_to_console("[BSP] loading bsp: %s\n", bsppath);

			// *
			// clipmap

			if(!reload) game::printf_to_console("[BSP] clipmap ..\n");

			// CM_LoadMapFromBsp
			d3dbsp::cm.name = Com_GetHunkStringCopy(bsppath);
			CMod_LoadPlanes();
			CMod_LoadCollisionVertsAndTris();
			CMod_LoadEntityString();
			d3dbsp::cm.isInUse = 1;


			// *
			// entities (script_models only for now)

			// spawn:
			/* "classname" "script_model"
			 * "model" "vehicle_80s_sedan1_red_destructible_mp"
			 * "origin" "-235.1 1394.7 230.2"
			 * "angles" "358.94 101.765 3.47606" */

			 // exclude:
			 /* "script_exploder"
			  * "script_gameobjectname" */

			if (dvars::bsp_load_entities->current.enabled && d3dbsp::cm.isInUse && d3dbsp::cm.mapEnts && d3dbsp::cm.mapEnts->numEntityChars)
			{
				if (!reload) game::printf_to_console("[BSP] entities ..\n");

				// create a spawnvar for each map entity
				utils::spawnvars script_models(d3dbsp::cm.mapEnts->entityString);

				// get name, origin and angles for script_models (see spawn/exclude note)
				std::vector<utils::spawnvars::script_model_for_dobj_s> dobjs;
				script_models.get_script_models_for_dobj(dobjs);

				if (!dobjs.empty())
				{
					for(radiant_dobj_count = 0; radiant_dobj_count < static_cast<int>(dobjs.size()) && radiant_dobj_count < 512; radiant_dobj_count++)
					{
						game::DObjModel_s dobjModels = {};
						dobjModels.model = game::R_RegisterModel(dobjs[radiant_dobj_count].model_name.c_str());
						dobjModels.boneName = 0;
						dobjModels.ignoreCollision = true;

						game::DObjCreate(&dobjModels, &radiant_dobj[radiant_dobj_count].obj, 1, nullptr, 0);

						utils::vector::copy(dobjs[radiant_dobj_count].origin, radiant_dobj[radiant_dobj_count].pose.origin);
						utils::vector::copy(dobjs[radiant_dobj_count].angles, radiant_dobj[radiant_dobj_count].pose.angles);
					}
				}

				if (!reload) game::printf_to_console("[BSP] loaded %d entities\n", radiant_dobj_count);
			}


			// *
			// load com world

			if (!reload) game::printf_to_console("[BSP] primary lights ..\n");

			Com_LoadPrimaryLights();
			game::comworld->name = Com_GetHunkStringCopy(bsppath);
			game::comworld->isInUse = true;


			// *
			// R_LoadPrimaryLights is missing in R_LoadWorldInternal (s_world ...)
			// so load it here and nop s_world memset at the beginning of R_LoadWorld->R_LoadWorldInternal

			R_LoadPrimaryLights(d3dbsp::Com_GetBspVersion());
			R_InitPrimaryLights(d3dbsp::scene_lights);

			unsigned int checksum;

			if (!reload) game::printf_to_console("[BSP] the world ..\n");

			// R_LoadWorld
			utils::hook::call<void(__cdecl)(const char* _name, unsigned int* _checksum, int _savegame)>(0x52E450)(bsppath, &checksum, 0);

			if (game::s_world->sunPrimaryLightIndex)
			{
				memcpy(&d3dbsp::scene_lights[game::s_world->sunPrimaryLightIndex], game::s_world->sunLight, sizeof(game::GfxLight));
			}

			game::R_SortWorldSurfaces();
			d3dbsp::loaded_bsp_path = bsppath;

			game::printf_to_console("[BSP] finished loading bsp\n");
			if (!reload) ShowWindow(con, SW_HIDE);

			return true;
		}

		return false;
	}

	/**
	 * @brief	reload currently loaded bsp\n
	 *			tries to load the bsp for the currently loaded .map otherwise
	 */
	void d3dbsp::reload_bsp()
	{
		if (d3dbsp::Com_IsBspLoaded() && !d3dbsp::loaded_bsp_path.empty())
		{
			d3dbsp::radiant_load_bsp(d3dbsp::loaded_bsp_path.c_str(), true);
		}
		else
		{
			std::string mapname;
			if (game::current_map_filepath && game::current_map_filepath != "unnamed.map"s)
			{
				mapname = std::string(game::current_map_filepath).substr(std::string(game::current_map_filepath).find_last_of("\\") + 1);
				utils::replace(mapname, ".map", ".d3dbsp");

				const bool is_mp = utils::starts_with(mapname, "mp_");
				const std::string bsp_path = (is_mp ? R"(maps\mp\)"s : R"(maps\)"s) + mapname;

				d3dbsp::radiant_load_bsp(bsp_path.c_str());
			}
			else
			{
				game::printf_to_console("Load a .map first!");
			}
		}
	}

	void d3dbsp::toggle_radiant_bsp_view(bool show_bsp)
	{
		if (d3dbsp::Com_IsBspLoaded())
		{
			const bool gameview_was_enabled = dvars::radiant_gameview->current.enabled;

			dvars::set_bool(dvars::r_draw_bsp, show_bsp);
			gameview::get()->toggle_all_filters(show_bsp);

			if (gameview_was_enabled)
			{
				components::gameview::p_this->set_state(gameview_was_enabled);
			}
		}
	}

	void d3dbsp::compile_fastfile(const std::string& bsp_name)
	{
		game::printf_to_console("[BSP] Compiling fastfile for map: %s ... \n", bsp_name.c_str());

		const auto egui = GET_GUI(ggui::entity_dialog);

		const char* base_path = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");

		std::string args;

		// batch
		args += R"(")"s + base_path + R"(\bin\IW3xRadiant\batch\compile_ff.bat")"s + " ";

		// args
		args += R"(english )"s + bsp_name;

		const auto process = components::process::get();

		process->set_process_type(process::PROC_TYPE_BATCH);
		process->set_indicator(process::INDICATOR_TYPE_SPINNER);
		process->set_output(true);
		process->set_arguments(args);

		if (game::glob::live_connected)
		{
			process->set_post_process_callback([bsp_name]
				{
					game::printf_to_console("[!] Sending devmap cmd to game\n");
					remote_net::Cmd_SendDvar(utils::va("{\n\"dvarname\" \"%s\"\n\"value\" \"%s\"\n}",
						"devmap",
						bsp_name.c_str()));

				}, true);
		}

		process->create_process();
	}

	/**
	 * @brief					run batch to compile bsp
	 * @param bsp_name			plain map name with no extension or pathing
	 * @param generate_createfx	automatically generate createfx files + place into raw instead of bin/IW3xRadiant
	 */
	void d3dbsp::compile_bsp(const std::string& bsp_name, bool generate_createfx)
	{
		//game::printf_to_console("^1generate_createfx is %s", generate_createfx ? "true" : "false");
		game::printf_to_console("[BSP] Compiling bsp for map: %s ...", bsp_name.c_str());
		
		const auto egui = GET_GUI(ggui::entity_dialog);

		const char* base_path = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
		const char* map_path = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");

		const bool is_mp = utils::starts_with(bsp_name, "mp_");
		const std::string bsp_path = (is_mp ? R"(maps\mp\)"s : R"(maps\)"s) + bsp_name + ".d3dbsp";

		std::string bps_args;
		bps_args += (dvars::bsp_compile_onlyents->current.enabled ? "-onlyents" : "") + " "s;
		bps_args += (dvars::bsp_compile_samplescale_enabled->current.enabled ? ("-samplescale " + std::to_string(dvars::bsp_compile_samplescale->current.value)) : "") + " "s;
		bps_args += (dvars::bsp_compile_custom_cmd_enabled->current.enabled ? dvars::bsp_compile_custom_cmd->current.string : "") + " "s;
		utils::rtrim(bps_args);

		std::string light_args;
		light_args += (dvars::bsp_compile_light_fast->current.enabled ? "-fast" : "") + " "s;
		light_args += (dvars::bsp_compile_light_extra->current.enabled ? "-extra" : "") + " "s;
		light_args += (dvars::bsp_compile_light_modelshadow->current.enabled ? "-modelshadow" : "") + " "s;
		light_args += (dvars::bsp_compile_light_dump->current.enabled ? "-dumpoptions" : "") + " "s;
		light_args += (dvars::bsp_compile_light_traces_enabled->current.enabled ? ("-traces " + std::to_string(dvars::bsp_compile_light_traces->current.integer)) : "") + " "s;
		light_args += (dvars::bsp_compile_light_custom_cmd_enabled->current.enabled ? dvars::bsp_compile_light_custom_cmd->current.string : "") + " "s;
		utils::rtrim(light_args);

		std::string args;

		// launch arg
		args += R"(")"s + base_path + R"(\bin\IW3xRadiant\batch\compile_bsp.bat")"s + " ";

		// bsppath
		args += R"(")"s + base_path + (is_mp ? R"(\raw\maps\mp\")"s : R"(\raw\maps\")"s) + " "s;

		// mapsourcepath
		args += R"(")"s + map_path + R"(\")"s + " "s;

		// treepath
		args += R"(")"s + base_path + R"(\")"s + " "s;

		// mapname
		args += bsp_name + " "s;

		// parmBSPOptions
		args += (!bps_args.empty() ? R"(")" + bps_args + R"(" )" : "- ");

		// parmLightOptions
		args += (!light_args.empty() ? R"(")" + light_args + R"(" )" : "- ");

		// compileBSP
		args += (dvars::bsp_compile_bsp->current.enabled ? "1 "s : "0 "s);

		// compileLight
		args += (dvars::bsp_compile_light->current.enabled ? "1 "s : "0 "s);

		const auto process = components::process::get();
		process->set_process_type(process::PROC_TYPE_BATCH);
		process->set_indicator(process::INDICATOR_TYPE_SPINNER);
		process->set_output(true);
		process->set_arguments(args);
		process->set_post_process_callback([bsp_name, bsp_path, generate_createfx]
		{
			game::printf_to_console("^2[PROCESS] Post-Process Callback");

			if (d3dbsp::radiant_load_bsp(bsp_path.c_str(), true))
			{
				if (dvars::bsp_gen_reflections_on_compile->current.enabled)
				{
					//components::renderer::on_cam_paint_post_rendercommands
					dvars::set_bool(dvars::r_reflectionprobe_generate, true);
				}
				else
				{
					if (dvars::bsp_show_bsp_after_compile->current.enabled)
					{
						toggle_radiant_bsp_view(true);
					}

					// slightly delay process generation
					exec::on_gui_once([bsp_name]
					{
						if (dvars::bsp_gen_fastfile_on_compile->current.enabled)
						{
							d3dbsp::compile_fastfile(bsp_name);
						}
					});
				}
			}

			if (generate_createfx)
			{
				game::printf_to_console("Generating CreateFX files ...");
				components::effects::generate_createfx(true);
			}
		}, true );

		process->create_process();
	}

	/**
	 * @brief	run batch to compile bsp for currently loaded .map
	 */
	void d3dbsp::compile_current_map()
	{
		std::string d3dbsp_name = std::string(game::current_map_filepath).substr(std::string(game::current_map_filepath).find_last_of("\\") + 1);
		utils::erase_substring(d3dbsp_name, ".map");
		d3dbsp::last_compiled_map = d3dbsp_name;

		components::d3dbsp::compile_bsp(d3dbsp_name, dvars::bsp_gen_createfx_on_compile->current.enabled);
	}


	// *
	// *


	float* CG_GetEntityOrigin(int scene_model_index)
	{
		return game::scene->sceneModel[scene_model_index].placement.base.origin;
	}

	void __declspec(naked) CG_GetEntityOrigin_stub()
	{
		const static uint32_t retn_addr = 0x5274D1;
		__asm
		{
			push	edi; // curr index in do-while loop (i < sceneModelCount)
			call	CG_GetEntityOrigin;
			add		esp, 4;
			jmp		retn_addr;
		}
	}


	// *
	// *


	void d3dbsp::force_dvars()
	{
		/*if (const auto& sm_enable = game::Dvar_FindVar("sm_enable"); sm_enable && sm_enable->current.enabled) {
			dvars::set_bool(sm_enable, false);
		}*/

		if (const auto& r_floatz = game::Dvar_FindVar("r_floatz"); r_floatz && !r_floatz->current.enabled) {
			dvars::set_bool(r_floatz, true);
		}

		if (const auto& r_distortion = game::Dvar_FindVar("r_distortion"); r_distortion && !r_distortion->current.enabled) {
			dvars::set_bool(r_distortion, true);
		}

		if (const auto& r_zFeather = game::Dvar_FindVar("r_zFeather"); r_zFeather && !r_zFeather->current.enabled) {
			dvars::set_bool(r_zFeather, true);
		}

		if (const auto& r_useLayeredMaterials = game::Dvar_FindVar("r_useLayeredMaterials"); r_useLayeredMaterials && r_useLayeredMaterials->current.enabled) {
			dvars::set_bool(r_useLayeredMaterials, false);
		}

		if (const auto& r_polygonOffsetBias = game::Dvar_FindVar("r_polygonOffsetBias"); r_polygonOffsetBias) {
			dvars::set_float(r_polygonOffsetBias, 0.0f);
		}

		if (const auto& r_polygonOffsetScale = game::Dvar_FindVar("r_polygonOffsetScale"); r_polygonOffsetScale) {
			dvars::set_float(r_polygonOffsetScale, -0.05f); // or radiant brushes/decals start to flicker
		}

		if (const auto& r_zNear = game::Dvar_FindVar("r_zNear"); r_zNear) {
			dvars::set_float(r_zNear, 4.0f);
		}
	}

	void d3dbsp::register_dvars()
	{
		dvars::bsp_load_entities = dvars::register_bool(
			/* name		*/ "bsp_load_entities",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "enable to load entities when loading a bsp (static_models only)");

		dvars::bsp_show_bsp_after_compile = dvars::register_bool(
			/* name		*/ "bsp_show_bsp_after_compile",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "automatically turn on bsp view after compiling");

		dvars::bsp_gen_reflections_on_compile = dvars::register_bool(
			/* name		*/ "bsp_gen_reflections_on_compile",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "automatically build reflections when compiling the bsp");

		dvars::bsp_gen_fastfile_on_compile = dvars::register_bool(
			/* name		*/ "bsp_gen_fastfile_on_compile",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "automatically build the maps fastfile after compiling the bsp\n+ reload map (devmap) in-game if livelink is established");

		dvars::bsp_gen_createfx_on_compile = dvars::register_bool(
			/* name		*/ "bsp_gen_createfx_on_compile",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "automatically generate CreateFX files when compiling the bsp");

		dvars::r_draw_bsp = dvars::register_bool(
			/* name		*/ "r_draw_bsp",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "enable to render bsp (if loaded)");

		dvars::r_draw_bsp_overwrite_sunlight = dvars::register_bool(
			/* name		*/ "r_draw_bsp_overwrite_sunlight",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "enable to overwrite bsp sunlight with fakesun settings");

		dvars::r_draw_bsp_overwrite_sundir = dvars::register_bool(
			/* name		*/ "r_draw_bsp_overwrite_sundir",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "enable to overwrite bsp sundir with fakesun settings");

		dvars::r_draw_bsp_overwrite_sunspecular = dvars::register_bool(
			/* name		*/ "r_draw_bsp_overwrite_sunspecular",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "enable to overwrite bsp sunspecular with fakesun settings");
	}

	

	d3dbsp::d3dbsp()
	{
		// #
		// * NOTES
		// * DONE:	load "dynamic" entities like exploding cars (scene.sceneModelCount :: R_AddXModelSurfacesCamera :: R_AddAllSceneEntSurfacesCamera)
		// * TODO:	^ CG_ProcessEntity :: CG_ScriptMover :: R_AddDObjToScene
		// *		^ G_InitGame :: G_SpawnEntitiesFromString .... G_ModelIndex -> XModelPrecache_LoadObj

		// * TODO:  draw sun / load sun dvars from sun file

		// #
		// R_LoadWorldInternal patches
		
		// re-implement a LUMP_UNLAYERED_TRIANGLES check (r_useLayeredMaterials)
		utils::hook::detour(0x4161B0, d3dbsp::Com_GetBspLumpBool, HK_JUMP);

		// re-implement Com_GetBspVersion
		utils::hook::detour(0x416150, d3dbsp::Com_GetBspVersion, HK_JUMP);

		// re-implement Com_GetBspLump
		utils::hook::detour(0x416180, d3dbsp::Com_GetBspLump, HK_JUMP);

		// re-implement CM_GetPlanes
		utils::hook::detour(0x4161E0, d3dbsp::CM_GetPlanes, HK_JUMP);

		// re-implement CM_GetPlaneCount
		utils::hook::detour(0x4161F0, d3dbsp::CM_GetPlaneCount, HK_JUMP);

		// re-implement CG_GetEntityOrigin in R_ShowCull
		utils::hook(0x5274CC, CG_GetEntityOrigin_stub, HOOK_JUMP).install()->quick();


		// #
		// dirty hacks

		utils::hook::nop(0x5525A2, 5); // Outdoor_ComputeTexels call
		utils::hook::nop(0x5101A4, 5); // memset of s_world in R_LoadWorldInternal

		utils::hook::nop(0x41625A, 5); // CM_BoxSightTrace Assert
		utils::hook::nop(0x56C8DC, 5); // bspSurf->material->info.name) = %s", "(!lightmapSecondaryFlag) Assert

		utils::hook::nop(0x41637A, 5); // Ragdoll_HandleBody Assert
		


		// Material_Add :: set rgp.needSortMaterials = 1 to 0 (do not re-sort materials when loading new ones; fixes bsp texture issues)
		utils::hook::set<BYTE>(0x510CF1 + 6, 0x0);

		// RENDERTARGET_SCENE to FRAMEBUFFER in R_DrawPointLitSurfsCallback
		utils::hook::set<BYTE>(0x55BC8F + 1, 0x1);

		// R_LoadSun (not needed and prints irrelevant info)
		utils::hook::nop(0x51047D, 5); 

		// R_LoadSunThroughDvars :: no longer called because above nop ^
		utils::hook::nop(0x52DAFA, 5); // Com_LoadDvarsFromBuffer (not implemented)



		// #
		// no bsp culling

		// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players (disable all culling)
		//utils::hook::nop(0x56998B, 3);

		// R_AddWorldSurfacesPortalWalk :: less culling :: 0x7C -> 0xEB (jl -> jmp)
		//utils::hook::set<BYTE>(0x527370, 0xEB);

		// R_AddAabbTreeSurfacesInFrustum_r :: less culling :: 0x74 -> 0xEB (je to jmp)
		//utils::hook::set<BYTE>(0x555C4C, 0xEB);


		// #
		// --------------------------


		// reload the currently loaded bsp
		// tries to automatically load a bsp based of the .map name if no bsp is loaded
		command::register_command_with_hotkey("bsp_reload"s, [](auto)
		{
			components::d3dbsp::reload_bsp();
		});

		// toggle bsp rendering on/off
		command::register_command_with_hotkey("toggle_bsp"s, [this](auto)
		{
			dvars::set_bool(dvars::r_draw_bsp, !dvars::r_draw_bsp->current.enabled);
		});

		// toggle between bsp and radiant rendering 
		command::register_command_with_hotkey("toggle_bsp_radiant"s, [this](auto)
		{
			if (d3dbsp::Com_IsBspLoaded())
			{
				const bool tstate = gameview::p_this->get_all_geo_state() || gameview::p_this->get_all_ents_state() || gameview::p_this->get_all_triggers_state() || gameview::p_this->get_all_others_state();

				const bool gameview_was_enabled = dvars::radiant_gameview->current.enabled;

				dvars::set_bool(dvars::r_draw_bsp, !tstate);
				command::execute("toggle_filter_all");

				if (gameview_was_enabled)
				{
					components::gameview::p_this->set_state(gameview_was_enabled);
				}
			}
		});
		
		command::register_command_with_hotkey("bsp_compile"s, [this](auto)
		{
			components::d3dbsp::compile_current_map();
		});
	}

	d3dbsp::~d3dbsp()
	{
		d3dbsp::shutdown_bsp();
	}
}
