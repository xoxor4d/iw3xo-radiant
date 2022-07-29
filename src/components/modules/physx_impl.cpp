#include "std_include.hpp"

constexpr bool USE_PVD = false; // PhysX Visual Debugger
constexpr bool USE_CONTACT_CALLBACK = false; // can be used to implement effect spawing on impact

// fx_system impacts:
// - 'play new effect on impact' only works when 'enable model physics simulation' is turned off
// - 'play new effect on impact' does not kill the elem

namespace components
{
	physx_impl* physx_impl::p_this = nullptr;

	constexpr int g_phys_minMsecStep = 3;	// 11
	constexpr int g_phys_maxMsecStep = 11;	// 67

	void physx_impl::run_frame(float seconds)
	{
		mScene->simulate(seconds);
		mScene->fetchResults(true);
	}

	// -> renderer::setup_viewinfo
	void physx_impl::frame()
	{
		const auto fxs = fx_system::FX_GetSystem(0);
		const auto efx = fx_system::ed_active_effect;

		m_simulation_running = false;
		m_effect_is_using_physics = false;

		if (efx && fxs)
		{
			for (int elemDefIndex = 0; elemDefIndex != efx->def->elemDefCountLooping; ++elemDefIndex)
			{
				if (efx->def->elemDefs[elemDefIndex].elemType == fx_system::FX_ELEM_TYPE_MODEL && (efx->def->elemDefs[elemDefIndex].flags & fx_system::FX_ELEM_USE_MODEL_PHYSICS) != 0)
				{
					m_effect_is_using_physics = true;
					break;
				}
			}

			if (!m_effect_is_using_physics)
			{
				return;
			}

			const auto time_now = fxs->msecNow;

			if (m_time_last_update < time_now)
			{
				m_time_last_snapshot = m_time_last_update;

				auto maxIter = 2u;
				for (auto i = 2u; ; maxIter = i)
				{
					if (!maxIter)
					{
						Assert();
					}

					const auto delta = (time_now - m_time_last_update) / static_cast<int>(maxIter);
					auto step = m_phys_msec_step;

					if (step < delta)
					{
						step = delta;
					}

					--i;

					m_simulation_running = true;

					physx_impl::run_frame(static_cast<float>(step) * 0.001f);
					m_time_last_update += step;


					// #
					// dxPostProcessIslands(static_cast<PhysWorld>(worldIndex));

					mScene->getActiveActors(m_active_actor_count);


					constexpr float REDUCE_MSEC_BEGIN_AT_COUNT = 64.0f; // object count needed to start increasing m_phys_msec_step # og: 32
					constexpr float REDUCE_MSEC_RANGE_TO_MAX = 64.0f;   // range - how many objects are needed to hit g_phys_maxMsecStep # og: 18

					const auto step_for_count = (static_cast<float>(m_active_actor_count) - REDUCE_MSEC_BEGIN_AT_COUNT) / REDUCE_MSEC_RANGE_TO_MAX;
					const auto s0 = step_for_count - 1.0f < 0.0f ? step_for_count : 1.0f;
					const auto s1 = 0.0f - step_for_count < 0.0f ? s0 : 0.0f;

					m_phys_msec_step = g_phys_minMsecStep + static_cast<int>((static_cast<float>((g_phys_maxMsecStep - g_phys_minMsecStep)) * s1));

					// #

					if (m_time_last_update >= time_now)
					{
						break;
					}
				}
			}

			if (m_time_last_snapshot > time_now || time_now > m_time_last_update)
			{
				Assert();
			}

			if (m_time_last_update <= m_time_last_snapshot)
			{
				if (m_time_last_update != m_time_last_snapshot)
				{
					Assert();
				}

				m_time_now_lerp_frac = 1.0f;
			}
			else
			{
				auto delta = static_cast<float>((time_now - m_time_last_snapshot)) / static_cast<float>((m_time_last_update - m_time_last_snapshot));
				m_time_now_lerp_frac = delta;

				if (delta < 0.0f || delta > 1.0f)
				{
					Assert();
				}
			}
		}

		// keep drawing visualization if 'phys_force_frame_logic' is enabled
		const bool force_update = GET_GUI(ggui::camera_settings_dialog)->phys_force_frame_logic;
		if ((force_update && !efx) || m_simulation_running || (effects::effect_is_paused() && efx))
		{
			// update 
			if ((force_update && !efx && !m_simulation_running) 
				|| force_update && efx && effects::effect_is_paused())
			{
				physx_impl::run_frame(1);
			}

			const physx::PxRenderBuffer& rb = mScene->getRenderBuffer();
			for (physx::PxU32 i = 0; i < rb.getNbLines(); i++)
			{
				const auto& line = rb.getLines()[i];

				game::GfxPointVertex vert[2];
				vert[0].xyz[0] = line.pos0.x;
				vert[0].xyz[1] = line.pos0.y;
				vert[0].xyz[2] = line.pos0.z;
				vert[0].color.packed = line.color0;

				vert[1].xyz[0] = line.pos1.x;
				vert[1].xyz[1] = line.pos1.y;
				vert[1].xyz[2] = line.pos1.z;
				vert[1].color.packed = line.color1;

				renderer::R_AddLineCmd(1, 4, 3, vert);

				if (mScene->getVisualizationParameter(physx::PxVisualizationParameter::eCONTACT_POINT) > 0.0f)
				{
					renderer::R_AddPointCmd(1, 12, 3, vert);
				}
			}
		}
	}

	physx::PxMaterial* physx_impl::create_material(game::PhysPreset* preset)
	{
		return mPhysics->createMaterial(preset->friction, preset->friction, preset->bounce);
	}

	// saves ptr to shape in 'm_effect_shape.custom_shape'
	void physx_impl::create_shape_from_selection(game::selbrush_def_t* sb)
	{
		if (sb)
		{
			std::vector<physx::PxVec3> verts;
			verts.reserve(50);

			game::vec3_t brush_center;
			utils::vector::add(sb->def->mins, sb->def->maxs, brush_center);
			utils::vector::scale(brush_center, 0.5f, brush_center);

			for (auto f = 0; f < sb->def->facecount; f++)
			{
				const auto face = &sb->def->brush_faces[f];
				for (auto p = 0; face->w && p < face->w->numPoints; p++)
				{
					game::vec3_t tw_point = { face->w->points[p][0], face->w->points[p][1], face->w->points[p][2] };
					utils::vector::subtract(tw_point, brush_center, tw_point);

					const physx::PxVec3 t_point = { tw_point[0], tw_point[1], tw_point[2] };
					bool contains = false;

					for (auto x = 0; x < static_cast<int>(verts.size()); x++)
					{
						if (utils::vector::compare(&verts[x].x, &t_point[0]))
						{
							contains = true;
							break;
						}
					}

					if (!contains)
					{
						verts.push_back(t_point);
					}
				}
			}


			physx::PxConvexMeshDesc convexDesc;
			convexDesc.points.count = verts.size();
			convexDesc.points.stride = sizeof(physx::PxVec3);
			convexDesc.points.data = verts.data();
			convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxConvexMeshCookingResult::Enum result;

			if (mCooking->cookConvexMesh(convexDesc, buf, &result))
			{
				physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
				m_effect_shape.custom_shape = mPhysics->createConvexMesh(input);
			}
		}
	}

	// no nullptr checks besides the selected brush itself
	// does not exclude brushes
	void physx_impl::create_static_brush(game::selbrush_def_t* sb, bool is_prefab, const game::vec3_t position_offset, const float* quat)
	{
		if (sb)
		{
			std::vector<physx::PxVec3> verts;
			verts.reserve(50);

			game::vec3_t brush_center;
			utils::vector::add(sb->def->mins, sb->def->maxs, brush_center);
			utils::vector::scale(brush_center, 0.5f, brush_center);

			for (auto f = 0; f < sb->def->facecount; f++)
			{
				const auto face = &sb->def->brush_faces[f];
				for (auto p = 0; face->w && p < face->w->numPoints; p++)
				{
					game::vec3_t tw_point = { face->w->points[p][0], face->w->points[p][1], face->w->points[p][2] };

					if(!is_prefab)
					{
						utils::vector::subtract(tw_point, brush_center, tw_point);
					}
					
					const physx::PxVec3 t_point = { tw_point[0], tw_point[1], tw_point[2] };
					bool contains = false;

					for (auto x = 0; x < static_cast<int>(verts.size()); x++)
					{
						if (utils::vector::compare(&verts[x].x, &t_point[0]))
						{
							contains = true;
							break;
						}
					}

					if (!contains)
					{
						verts.push_back(t_point);
					}
				}
			}


			physx::PxConvexMeshDesc convexDesc;
			convexDesc.points.count = verts.size();
			convexDesc.points.stride = sizeof(physx::PxVec3);
			convexDesc.points.data = verts.data();
			convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxConvexMeshCookingResult::Enum result;

			if (mCooking->cookConvexMesh(convexDesc, buf, &result))
			{
				physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
				physx::PxConvexMesh* convexMesh = mPhysics->createConvexMesh(input);

				const physx::PxTransform t
				(
					brush_center[0], brush_center[1], brush_center[2]
				);

				// do not offset prefab brushes by their brush center
				auto* actor = mPhysics->createRigidStatic(is_prefab ? physx::PxTransform(0.0f, 0.0f, 0.0f) : t);
				actor->setActorFlags(physx::PxActorFlag::eVISUALIZATION);

				physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, physx::PxConvexMeshGeometry(convexMesh), *m_static_collision_material);

				physx::PxTransform local;
				local.p = physx::PxVec3(0.0f, 0.0f, 0.0f);
				local.q = physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f);

				// mainly for prefab brushes
				if (position_offset || quat)
				{
					if (position_offset)
					{
						local.p = physx::PxVec3(position_offset[0], position_offset[1], position_offset[2]);
					}

					if (quat)
					{
						local.q = physx::PxQuat(quat[0], quat[1], quat[2], quat[3]);
					}

					shape->setLocalPose(local);
				}

				if (USE_CONTACT_CALLBACK)
				{
					physx::PxFilterData filterData;
					filterData.word0 = 4; // 4 = static collision
					filterData.word1 = 2; // 2 = physics object
					shape->setSimulationFilterData(filterData);
				}

				actor->attachShape(*shape);
				shape->release();

				mScene->addActor(*actor);

				m_static_brushes.push_back(actor);
				m_static_brush_count++;
			}
		}
	}


	void physx_impl::create_static_terrain(game::selbrush_def_t* sb, const game::vec3_t position_offset, const float* quat)
	{
		// reference - Q3 - Terrain_GetTriangle

		const auto phys = components::physx_impl::get();
		const auto patch = sb->def->patch;

		std::vector<physx::PxVec3> verts;
		std::vector<uint32_t> inds;
		int tri_count = 0;


		if ((sb->def->patch->type & game::PATCH_TERRAIN) != 0)
		{
			for (auto x = 0; x < patch->width - 1; x++)
			{
				for (auto y = 0; y < patch->height; y++)
				{
					if (y != patch->height - 1)
					{
						const game::vec_t* v1;
						const game::vec_t* v2;
						const game::vec_t* v3;

						if (patch->ctrl[x][y].turned_edge)
						{
							v1 = patch->ctrl[x + 1][y + 0].xyz;
							v2 = patch->ctrl[x + 1][y + 1].xyz;
							v3 = patch->ctrl[x + 0][y + 0].xyz;
						}
						else
						{
							v1 = patch->ctrl[x + 0][y + 0].xyz;
							v2 = patch->ctrl[x + 1][y + 0].xyz;
							v3 = patch->ctrl[x + 0][y + 1].xyz;
						}

						verts.emplace_back(physx::PxVec3(v1[0], v1[1], v1[2]));
						verts.emplace_back(physx::PxVec3(v2[0], v2[1], v2[2]));
						verts.emplace_back(physx::PxVec3(v3[0], v3[1], v3[2]));

						inds.push_back(inds.size());
						inds.push_back(inds.size());
						inds.push_back(inds.size());

						tri_count++;
					}

					if (y != 0)
					{
						const game::vec_t* v1;
						const game::vec_t* v2;
						const game::vec_t* v3;

						if (patch->ctrl[x][y - 1].turned_edge)
						{
							v1 = patch->ctrl[x + 0][y + 0].xyz;
							v2 = patch->ctrl[x + 0][y - 1].xyz;
							v3 = patch->ctrl[x + 1][y + 0].xyz;
						}
						else
						{
							v1 = patch->ctrl[x + 1][y + 0].xyz;
							v2 = patch->ctrl[x + 0][y + 0].xyz;
							v3 = patch->ctrl[x + 1][y - 1].xyz;
						}

						verts.emplace_back(physx::PxVec3(v1[0], v1[1], v1[2]));
						verts.emplace_back(physx::PxVec3(v2[0], v2[1], v2[2]));
						verts.emplace_back(physx::PxVec3(v3[0], v3[1], v3[2]));

						inds.push_back(inds.size());
						inds.push_back(inds.size());
						inds.push_back(inds.size());

						tri_count++;
					}
				}
			}
		}

		// curve
		else
		{
			const auto width = patch->curveDef->width;
			const auto height = patch->curveDef->height;

			for (auto x = 0; x < width - 1; x++)
			{
				for (auto y = 0; y < height; y++)
				{
					if (y != height - 1)
					{
						const game::vec_t* v1;
						const game::vec_t* v2;
						const game::vec_t* v3;

						{
							v1 = patch->curveDef->verts[x + 0 + (width * (y + 0))].xyz;
							v2 = patch->curveDef->verts[x + 1 + (width * (y + 0))].xyz;
							v3 = patch->curveDef->verts[x + 0 + (width * (y + 1))].xyz;
						}

						verts.emplace_back(physx::PxVec3(v1[0], v1[1], v1[2]));
						verts.emplace_back(physx::PxVec3(v2[0], v2[1], v2[2]));
						verts.emplace_back(physx::PxVec3(v3[0], v3[1], v3[2]));

						inds.push_back(inds.size());
						inds.push_back(inds.size());
						inds.push_back(inds.size());

						tri_count++;
					}

					if (y != 0)
					{
						const game::vec_t* v1;
						const game::vec_t* v2;
						const game::vec_t* v3;

						{
							v1 = patch->curveDef->verts[x + 1 + (width * (y + 0))].xyz;
							v2 = patch->curveDef->verts[x + 0 + (width * (y + 0))].xyz;
							v3 = patch->curveDef->verts[x + 1 + (width * (y - 1))].xyz;
						}

						verts.emplace_back(physx::PxVec3(v1[0], v1[1], v1[2]));
						verts.emplace_back(physx::PxVec3(v2[0], v2[1], v2[2]));
						verts.emplace_back(physx::PxVec3(v3[0], v3[1], v3[2]));

						inds.push_back(inds.size());
						inds.push_back(inds.size());
						inds.push_back(inds.size());

						tri_count++;
					}
				}
			}
		}
		

		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = verts.size();
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.points.data = verts.data();

		meshDesc.triangles.count = tri_count;
		meshDesc.triangles.stride = 3 * sizeof(uint32_t);
		meshDesc.triangles.data = inds.data();

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxTriangleMeshCookingResult::Enum result;

		if (phys->mCooking->cookTriangleMesh(meshDesc, buf, &result))
		{
			physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
			physx::PxTriangleMesh* triangleMesh = phys->mPhysics->createTriangleMesh(input);

			auto* actor = phys->mPhysics->createRigidStatic(physx::PxTransform(0.0f, 0.0f, 0.0f));
			actor->setActorFlags(physx::PxActorFlag::eVISUALIZATION);

			physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, physx::PxTriangleMeshGeometry(triangleMesh), *phys->m_static_collision_material);

			physx::PxTransform local;
			local.p = physx::PxVec3(0.0f, 0.0f, 0.0f);
			local.q = physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f);

			// mainly for prefab brushes
			if (position_offset || quat)
			{
				if (position_offset)
				{
					local.p = physx::PxVec3(position_offset[0], position_offset[1], position_offset[2]);
				}

				if (quat)
				{
					local.q = physx::PxQuat(quat[0], quat[1], quat[2], quat[3]);
				}

				shape->setLocalPose(local);
			}

			if (USE_CONTACT_CALLBACK)
			{
				physx::PxFilterData filterData;
				filterData.word0 = 4; // 4 = static collision
				filterData.word1 = 2; // 2 = physics object
				shape->setSimulationFilterData(filterData);
			}

			actor->attachShape(*shape);
			shape->release();

			phys->mScene->addActor(*actor);
			phys->m_static_terrain.push_back(actor);
			m_static_terrain_count++;
		}
	}


	bool exclude_brushes_from_static_collision(game::selbrush_def_t* b)
	{
		// skip sky
		if (b->def->contents & game::BRUSHCONTENTS_SKY)
		{
			return true;
		}

		// skip fixed size objects
		if (b->brushflags & game::BRUSHFLAG_FIXED_SIZE)
		{
			return true;
		}

		// skip all nodes (reflection_probes, script_origins, lights etc)
		if (b->owner && b->owner->firstActive && b->owner->firstActive->eclass)
		{
			const auto class_type = b->owner->firstActive->eclass->classtype;

			if (class_type & game::ECLASS_RADIANT_NODE)
			{
				return true;
			}
		}

		// include all generic solid + detail and weaponclip brushes
		if (!(b->brushflags & game::BRUSHFLAG_SOLID) && (b->def->contents & game::BRUSHCONTENTS_DETAIL || b->def->contents & game::BRUSHCONTENTS_WEAPONCLIP))
		{
			return true;
		}

		// skip all tooling (spawns, lightgrid ...) but include (some) clip
		if ((b->brushflags & game::BRUSHFLAG_TOOL) && !(b->def->contents & 0x10000 || b->def->contents & 0x20000 || b->def->contents & 0x30000))
		{
			return true;
		}

		return false;
	}


	// only call via components::process
	void physx_impl::create_static_collision()
	{
		const auto phys = components::physx_impl::get();
		phys->m_static_brush_estimated_count = 0;
		phys->m_static_brush_count = 0;
		phys->m_static_terrain_estimated_count = 0;
		phys->m_static_terrain_count = 0;

		for (const auto brush : phys->m_static_brushes)
		{
			brush->release();
		}
		phys->m_static_brushes.clear();
		phys->m_static_brushes.reserve(1000);

		// #

		for (const auto terrain : phys->m_static_terrain) 
		{
			terrain->release();
		}
		phys->m_static_terrain.clear();
		phys->m_static_terrain.reserve(100);


		// #

		const auto process = process::get();

		// #
		// count all brushes (used for progressbar)

 		FOR_ALL_ACTIVE_BRUSHES(sb)
		{
			if (process->pending_termination())
			{
				return;
			}

			// prefabs
			if (sb && sb->owner && sb->owner->prefab && sb->owner->firstActive && sb->owner->firstActive->eclass && sb->owner->firstActive->eclass->classtype & game::ECLASS_PREFAB)
			{
				FOR_ALL_BRUSHES(prefab, sb->owner->prefab->active_brushlist, sb->owner->prefab->active_brushlist_next)
				{
					if (process->pending_termination())
					{
						return;
					}

					if (prefab && prefab->def)
					{
						// patches
						if (prefab->def->patch)
						{
							if (sb->def->contents & game::BRUSHCONTENTS_NONCOLLIDING)
							{
								continue;
							}

							phys->m_static_terrain_estimated_count++;
							continue;
						}

						// brushes
						if (exclude_brushes_from_static_collision(prefab))
						{
							continue;
						}

						phys->m_static_brush_estimated_count++;
					}
				}
			}

			// root map
			else if (sb && sb->def)
			{
				// patches
				if (sb->def->patch)
				{
					if (sb->def->contents & game::BRUSHCONTENTS_NONCOLLIDING)
					{
						continue;
					}

					phys->m_static_terrain_estimated_count++;
					continue;
				}

				// brushes
				if (exclude_brushes_from_static_collision(sb))
				{
					continue;
				}

				phys->m_static_brush_estimated_count++;
			}
		}


		// #
		// generate static collision

		FOR_ALL_ACTIVE_BRUSHES(sb)
		{
			if (process->pending_termination())
			{
				return;
			}

			// prefab brushes and terrain
			if (sb && sb->owner && sb->owner->prefab && sb->owner->firstActive && sb->owner->firstActive->eclass && sb->owner->firstActive->eclass->classtype & game::ECLASS_PREFAB)
			{
				FOR_ALL_BRUSHES(prefab, sb->owner->prefab->active_brushlist, sb->owner->prefab->active_brushlist_next)
				{
					if (process->pending_termination())
					{
						return;
					}

					if (prefab && prefab->def)
					{
						const auto is_patch = prefab->def->patch;
						if (is_patch)
						{
							if (sb->def->contents & game::BRUSHCONTENTS_NONCOLLIDING)
							{
								continue;
							}
						}
						else if (exclude_brushes_from_static_collision(prefab))
						{
							continue;
						}

						game::vec3_t prefab_angles = {};
						game::vec4_t quat = { 0.0f, 0.0f, 0.0f, 1.0f };

						// angles to quat - use identity if prefab has no angles kvp
						if (GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(sb->owner->firstActive, prefab_angles, "angles"))
						{
							game::orientation_t orientation = {};
							game::AnglesToAxis(prefab_angles, &orientation.axis[0][0]);
							fx_system::AxisToQuat(orientation.axis, quat);
						}

						if (is_patch)
						{
							phys->create_static_terrain(prefab, sb->owner->firstActive->origin, quat);
						}
						else
						{
							phys->create_static_brush(prefab, true, sb->owner->firstActive->origin, quat);
						}
					}
				}
			}

			// root map brushes and terrain
			else if (sb && sb->def)
			{
				// patches
				if (sb->def->patch)
				{
					if (sb->def->contents & game::BRUSHCONTENTS_NONCOLLIDING)
					{
						continue;
					}

					phys->create_static_terrain(sb);
				}

				// brushes
				else
				{
					// skip brushes that should not be part of the static collision
					if (exclude_brushes_from_static_collision(sb))
					{
						continue;
					}

					phys->create_static_brush(sb);
				}
			}
		}

		phys->m_static_brush_count = phys->m_static_brushes.size();
		phys->m_static_brush_estimated_count = 0;

		phys->m_static_terrain_count = phys->m_static_terrain.size();
		phys->m_static_terrain_estimated_count = 0;
	}


	void physx_impl::convert_phys_to_misc_models()
	{
		const auto system = fx_system::FX_GetSystem(0);
		const auto entity_gui = GET_GUI(ggui::entity_dialog);

		utils::show_external_console(true);

		m_converted_misc_model_count = 0;

		std::uint16_t elemHandle = fx_system::ed_active_effect->firstElemHandle[fx_system::FX_ELEM_CLASS_NONSPRITE];
		if (elemHandle != UINT16_MAX)
		{
			game::Select_Deselect(true);
			game::Undo_ClearRedo();
			game::Undo_GeneralStart("create entity from physics");
			ggui::entity_dialog::addprop_helper_s no_undo = {};

			while (elemHandle != UINT16_MAX)
			{
				const fx_system::FxElem* elem = fx_system::FX_ElemFromHandle(system, elemHandle);
				const fx_system::FxElemDef* elem_def = &fx_system::ed_active_effect->def->elemDefs[static_cast<std::uint8_t>(elem->defIndex)];

				if (elem_def->elemType <= fx_system::FX_ELEM_TYPE_LAST_SPRITE)
				{
					Assert();
				}

				if (elem_def->elemType == fx_system::FX_ELEM_TYPE_MODEL && elem_def->flags & fx_system::FX_ELEM_USE_MODEL_PHYSICS)
				{
					const auto actor = reinterpret_cast<physx::PxRigidDynamic*>(elem->___u8.physObjId);

					if (elem_def->visuals.instance.model)
					{
						const auto pos = actor->getGlobalPose().p;
						const auto quat = actor->getGlobalPose().q;

						float axis[3][3];
						fx_system::UnitQuatToAxis(&quat.x, axis);

						game::vec3_t angles = {};
						game::AxisToAngles(axis, angles);

						{
							if ((DWORD*)game::g_selected_brushes_next() == game::currSelectedBrushes)
							{
								game::CreateEntityBrush(0, 0, cmainframe::activewnd->m_pXYWnd);
							}

							//utils::benchmark timer;

							// do not open the original modeldialog for this use-case, see: create_entity_from_name_intercept()
							g_block_radiant_modeldialog = true;
							game::CreateEntityFromName("misc_model");
							g_block_radiant_modeldialog = false;

							// CreateEntityFromName takes ~ 24 ms
							//timer.now("CreateEntityBrush");

							entity_gui->add_prop("model", elem_def->visuals.instance.model->name, &no_undo);

							char str_buf[64] = {};
							if (sprintf_s(str_buf, "%.3f %.3f %.3f", pos[0], pos[1], pos[2]))
							{
								entity_gui->add_prop("origin", str_buf, &no_undo);
							}

							if (sprintf_s(str_buf, "%.3f %.3f %.3f", angles[0], angles[1], angles[2]))
							{
								entity_gui->add_prop("angles", str_buf, &no_undo);
							}

							if (elem_def->visSamples && elem_def->visSamples->base.scale != 1.0f)
							{
								entity_gui->add_prop("modelscale", std::to_string(elem_def->visSamples->base.scale).c_str(), &no_undo);
							}
						}
						

						game::printf_to_console("spawned model #%d\n", m_converted_misc_model_count);
						m_converted_misc_model_count++;

						game::Select_Deselect(true);
					}
				}

				elemHandle = elem->nextElemHandleInEffect;
			}

			game::Undo_End();
		}

		m_converted_misc_model_count = 0;

		utils::show_external_console(false);
	}


	void physx_impl::obj_destroy(int id)
	{
		const auto actor = reinterpret_cast<physx::PxRigidDynamic*>(id);

		if (actor->userData)
		{
			//const auto material = static_cast<physx::PxMaterial*>(actor->userData);
			const auto data = static_cast<userdata_s*>(actor->userData);
			data->material->release();

			delete(data);
		}

		actor->release();
	}

	void physx_impl::obj_get_interpolated_state(int id, float* out_pos, float* out_quat)
	{
		const auto actor = reinterpret_cast<physx::PxRigidDynamic*>(id);

		const physx::PxQuat quat = actor->getGlobalPose().q;
		const auto pos = actor->getGlobalPose().p;

		out_pos[0] = pos.x;
		out_pos[1] = pos.y;
		out_pos[2] = pos.z;

		out_quat[0] = quat.x;
		out_quat[1] = quat.y;
		out_quat[2] = quat.z;
		out_quat[3] = quat.w;
	}


	int physx_impl::create_physx_object(game::XModel* model, const float* world_pos, const float* quat, const float* velocity, const float* angular_velocity)
	{
		const auto material = create_material(model->physPreset);

		game::vec3_t half_bounds;
		utils::vector::subtract(model->maxs, model->mins, half_bounds);
		utils::vector::scale(half_bounds, 0.5f, half_bounds);

		physx::PxShape* shape = nullptr;

		if (m_effect_shape.index == EFFECT_PHYSX_SHAPE::CUSTOM && !m_effect_shape.custom_shape)
		{
			m_effect_shape.index = EFFECT_PHYSX_SHAPE::CUBE;
			game::printf_to_console("[!] No custom shape defined! Create a custom shape from a selected brush first.");
			imgui::Toast(ImGuiToastType_Warning, "PhysX custom shape", "No custom shape defined!\nCreate a custom shape from a selected brush first.");
		}

		switch(m_effect_shape.index)
		{
		default:
		case EFFECT_PHYSX_SHAPE::CUBE:
			shape = mPhysics->createShape(
				physx::PxBoxGeometry(half_bounds[0] * m_effect_shape.scalar, half_bounds[1] * m_effect_shape.scalar, half_bounds[2] * m_effect_shape.scalar), 
				*m_static_collision_material, true);
			break;

		case EFFECT_PHYSX_SHAPE::SPHERE:
			shape = mPhysics->createShape(physx::PxSphereGeometry(model->radius * m_effect_shape.scalar), 
				*m_static_collision_material, true);
			break;
		case EFFECT_PHYSX_SHAPE::CUSTOM:
			shape = mPhysics->createShape(physx::PxConvexMeshGeometry(m_effect_shape.custom_shape), *m_static_collision_material, true);
			break;
		}

		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);

		game::vec3_t origin_offset;
		utils::vector::subtract(model->maxs, half_bounds, origin_offset);
		shape->setLocalPose(physx::PxTransform(origin_offset[0], origin_offset[1], origin_offset[2]));

		const physx::PxTransform t
		(
			world_pos[0], world_pos[1], world_pos[2],
			physx::PxQuat(quat[0], quat[1], quat[2], quat[3])
		);

		physx::PxRigidDynamic* actor = mPhysics->createRigidDynamic(t);

		actor->setActorFlags(physx::PxActorFlag::eVISUALIZATION);

		userdata_s* data = new userdata_s();
		data->material = material;
		actor->userData = data;

		if (USE_CONTACT_CALLBACK)
		{
			physx::PxFilterData filterData;
			filterData.word0 = 2; // 2 = physics object
			filterData.word1 = 4; // 4 = static collision
			shape->setSimulationFilterData(filterData);
		}

		actor->attachShape(*shape);
		shape->release();

		/*const physx::PxVec3 center_of_mass =
		{
			(model->mins[0] + model->maxs[0]) * 0.5f,
			(model->mins[1] + model->maxs[1]) * 0.5f,
			(model->mins[2] + model->maxs[2]) * 0.5f,
		};*/

		physx::PxRigidBodyExt::updateMassAndInertia(*actor, model->physPreset->mass /*, &center_of_mass*/);

		mScene->addActor(*actor);
	
		if (velocity)
		{
			actor->setLinearVelocity(physx::PxVec3(velocity[0], velocity[1], velocity[2]));
		}

		if (angular_velocity)
		{
			actor->setAngularVelocity(physx::PxVec3(angular_velocity[0], angular_velocity[1], angular_velocity[2]));
		}

		return reinterpret_cast<int>(actor);
	}


	void physx_impl::update_static_collision_material()
	{
		const auto gui = GET_GUI(ggui::camera_settings_dialog);

		m_static_collision_material->setStaticFriction(gui->phys_material[0]);
		m_static_collision_material->setDynamicFriction(gui->phys_material[1]);
		m_static_collision_material->setRestitution(gui->phys_material[2]);

		/*if (m_static_collision_material)
		{
			m_static_collision_material->release();
		}
		
		m_static_collision_material = mPhysics->createMaterial(gui->phys_material[0], gui->phys_material[1], gui->phys_material[2]);*/
	}


	// active when 'USE_CONTACT_CALLBACK'
	void physx_impl::collision_feedback::onContact([[maybe_unused]] const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		for (physx::PxU32 i = 0; i < nbPairs; i++)
		{
			const physx::PxContactPair& cp = pairs[i];

			if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				const auto udata = pairHeader.actors[0]->userData ? pairHeader.actors[0]->userData : pairHeader.actors[1]->userData;
				if (udata)
				{
					//const auto userdata = static_cast<userdata_s*>(udata);
					//game::printf_to_console("dynamic actor");
				}

				//game::printf_to_console("contact!");
				break;
			}
		}
	}


	// active when 'USE_CONTACT_CALLBACK'
	// used to identify collisions between static collision and physic's enabled objects
	physx::PxFilterFlags FilterShader(	[[maybe_unused]] physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
										[[maybe_unused]] physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
										physx::PxPairFlags& pairFlags, [[maybe_unused]] const void* constantBlock, [[maybe_unused]] physx::PxU32 constantBlockSize)
	{
		//// let triggers through
		//if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		//{
		//	pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		//	return physx::PxFilterFlag::eDEFAULT;
		//}

		// generate contacts for all that were not filtered above
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
		}

		return physx::PxFilterFlag::eDEFAULT;
	}


	void physx_impl::register_dvars()
	{
		dvars::physx_debug_visualization_box_size = dvars::register_float(
			/* name		*/ "physx_debug_visualization_box_size",
			/* default	*/ 2000.0f,
			/* mins		*/ 0.0f,
			/* maxs		*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "size of culling box in which to draw debug visualizations. 0 disables the culling box");
	}


	physx_impl::physx_impl()
	{
		physx_impl::p_this = this;

		m_simulation_running = false;
		m_effect_is_using_physics = false;

		m_time_last_snapshot = 0;
		m_time_last_update = 0;
		m_time_now_lerp_frac = 0;

		m_phys_msec_step = 3;
		m_active_actor_count = 0;
		m_static_brush_count = 0;
		m_static_brush_estimated_count = 0;
		m_static_terrain_estimated_count = 0;
		m_static_terrain_count = 0;
		m_converted_misc_model_count = 0;

		// #

		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		if (!mFoundation)
		{
			AssertS("PxCreateFoundation failed!");
		}

		mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale);
		if (!mCooking)
		{
			AssertS("PxCreateCooking failed!");
		}

		if (USE_PVD)
		{
			physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
			mPvd = PxCreatePvd(*mFoundation);
			mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
		}

		mToleranceScale.length = 1;  // typical length of an object
		mToleranceScale.speed = 100; // typical speed of an object, gravity*1s is a reasonable choice
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, USE_PVD ? mPvd : nullptr);
		mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

		/*if (!PxInitExtensions(*mPhysics, mPvd))
		{
			AssertS("PxInitExtensions failed!");
		}*/

		physx::PxSceneDesc scene_desc(mPhysics->getTolerancesScale());
		scene_desc.gravity = physx::PxVec3(0.0f, 0.0f, -800.0f); // default: -9.81 // scale of 80
		scene_desc.bounceThresholdVelocity = 1400.0f; // default: 20
		scene_desc.cpuDispatcher = mDispatcher;

		if (USE_CONTACT_CALLBACK)
		{
			scene_desc.filterShader = FilterShader;
			scene_desc.simulationEventCallback = new collision_feedback();
		}
		else
		{
			scene_desc.filterShader = physx::PxDefaultSimulationFilterShader;
		}

		scene_desc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
		mScene = mPhysics->createScene(scene_desc);

		if (USE_PVD)
		{
			physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
			if (pvdClient)
			{
				pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
				pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
				pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
			}
		}

		m_static_collision_material = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	}

	physx_impl::~physx_impl()
	{
		//PxCloseExtensions();

		mCooking->release();
		mPhysics->release();
		mFoundation->release();
	}
}
