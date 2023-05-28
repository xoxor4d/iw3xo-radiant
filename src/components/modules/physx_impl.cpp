#include "std_include.hpp"
//#include "characterkinematic/PxControllerManager.h"

constexpr bool USE_PVD = false; // PhysX Visual Debugger
constexpr bool USE_CONTACT_CALLBACK = false; // can be used to implement effect spawning on impact

// fx_system impacts:
// - 'play new effect on impact' only works when 'enable model physics simulation' is turned off
// - 'play new effect on impact' does not kill the elem

namespace components
{
	PxControllerBehaviorFlags physx_impl::behavior_feedback::getBehaviorFlags([[maybe_unused]] const PxController& controller)
	{
		return PxControllerBehaviorFlag::eCCT_SLIDE;
	}

	PxControllerBehaviorFlags physx_impl::behavior_feedback::getBehaviorFlags([[maybe_unused]] const PxObstacle& obstacle)
	{
		return PxControllerBehaviorFlag::eCCT_SLIDE;
	}

	PxControllerBehaviorFlags physx_impl::behavior_feedback::getBehaviorFlags([[maybe_unused]] const PxShape& shape, [[maybe_unused]] const PxActor& actor)
	{
		return PxControllerBehaviorFlag::eCCT_SLIDE;
	}

#if 0
	void physx_impl::cct_hit_feedback::onShapeHit(const PxControllerShapeHit& hit)
	{

	}
#endif

	physx_impl* physx_impl::p_this = nullptr;

	constexpr int m_phys_min_msec_step = 3;	// 11
	constexpr int m_phys_max_msec_step = 11;	// 67

	/**
	 * @brief	PhysX tick logic (not related to effects)
	 * @note	- called from 'components::effects::camera_onpaint_intercept()'
	 *			- same logic as effects::tick_playback()
	 */
	void physx_impl::tick_playback()
	{
		const auto saved_tick = static_cast<int>(GetTickCount());
		auto tick_cmp = saved_tick - m_phys_sim_tick_old;

		if (tick_cmp > 200)
		{
			tick_cmp = 200;
		}

		m_phys_sim_tick_old = saved_tick;

		if (m_phys_sim_run && !m_phys_sim_pause)
		{
			auto tick_inc = static_cast<int>(static_cast<double>(tick_cmp) * static_cast<double>(fx_system::ed_timescale) + 9.313225746154785e-10);
			if (tick_inc <= 1)
			{
				tick_inc = 1;
			}

			m_phys_sim_tick += tick_inc;
		}
	}

	/**
	 * @brief				simulate and fetch results
	 * @param	seconds		amount to advance simulation by 
	 */
	void physx_impl::run_frame(float seconds)
	{
		const auto proc = process::get()->is_active();
		if (proc)
		{
			return;
		}

		mScene->simulate(seconds);
		mScene->fetchResults(true);
	}

	/**
	 * @brief			effect browser scene sim
	 * @param tick		current phys tick
	 */
	void physx_impl::fx_browser_frame([[maybe_unused]] int tick)
	{
		if (game::glob::is_loading_map)
		{
			return;
		}


		const auto time_now = static_cast<uint32_t>(tick);
		if (m_phys_time_last_update_fx_browser < time_now)
		{
			auto max_iter = 2u;
			for (auto i = 2u; ; max_iter = i)
			{
				if (!max_iter)
				{
					AssertS("!max_iter");
				}

				const auto delta = (time_now - m_phys_time_last_update_fx_browser) / static_cast<int>(max_iter);
				auto step = m_phys_fx_browser_msec_step;

				if (step < delta)
				{
					step = delta;
				}

				--i;

				mSceneEffectBrowser->simulate(static_cast<float>(step) * 0.001f);
				m_phys_time_last_update_fx_browser += step;

				mSceneEffectBrowser->fetchResults(true);
				mSceneEffectBrowser->getActiveActors(m_phys_active_actor_fx_browser_count);

				constexpr float REDUCE_MSEC_BEGIN_AT_COUNT = 64.0f; // object count needed to start increasing m_phys_msec_step # og: 32
				constexpr float REDUCE_MSEC_RANGE_TO_MAX = 64.0f;   // range - how many objects are needed to hit g_phys_maxMsecStep (begin + range) # og: 18

				const auto step_for_count = (static_cast<float>(m_phys_active_actor_fx_browser_count) - REDUCE_MSEC_BEGIN_AT_COUNT) / REDUCE_MSEC_RANGE_TO_MAX;
				const auto s0 = step_for_count - 1.0f < 0.0f ? step_for_count : 1.0f;
				const auto s1 = 0.0f - step_for_count < 0.0f ? s0 : 0.0f;

				m_phys_fx_browser_msec_step = m_phys_min_msec_step + static_cast<int>((static_cast<float>((m_phys_max_msec_step - m_phys_min_msec_step)) * s1));

				if (m_phys_time_last_update_fx_browser >= time_now)
				{
					break;
				}
			}
		}

		/*auto step = (tick - m_phys_time_last_update_fx_browser);

		step = step < 3 ? 3 : step > 11 ? 11 : step;

		m_phys_fx_browser_msec_step = step;

		mSceneEffectBrowser->simulate(static_cast<float>(step) * 0.001f);
		m_phys_time_last_update_fx_browser += step;

		mSceneEffectBrowser->fetchResults(true);
		mSceneEffectBrowser->getActiveActors(m_phys_active_actor_fx_browser_count);*/
	}

	/**
	 * @brief	draw debug data gathered by the last simulation
	 */
	void physx_impl::draw_debug_visualization()
	{
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

#if DEBUG
		if (!m_cct_camera->m_groundtrace.normal.isZero())
		{
			game::GfxPointVertex vert = {};
			vert.xyz[0] = m_cct_camera->m_groundtrace.position.x;
			vert.xyz[1] = m_cct_camera->m_groundtrace.position.y;
			vert.xyz[2] = m_cct_camera->m_groundtrace.position.z;
			vert.color.packed = static_cast<unsigned>(PxDebugColor::eARGB_RED);

			renderer::R_AddPointCmd(1, 12, 3, &vert);
		}
#endif
	}

	/**
	 * @brief	PhysX simulation frame for effect unrelated PhysX actors
	 * @note	called from renderer::setup_viewinfo()
	 */
	void physx_impl::phys_frame()
	{
		// its not save to access brush data while loading a map (obv.)
		// do not re-run phys if fx frame is active
		if (game::glob::is_loading_map || m_fx_sim_running)
		{
			return;
		}

		m_phys_sim_running = false;

		if (m_phys_sim_run || (m_phys_sim_run && m_phys_sim_pause))
		{
			if (m_phys_time_last_update < m_phys_sim_tick)
			{
				m_phys_time_last_snapshot = m_phys_time_last_update;

				auto max_iter = 2u;
				for (auto i = 2u; ; max_iter = i)
				{
					if (!max_iter)
					{
						AssertS("!max_iter");
					}

					const auto delta = (m_phys_sim_tick - m_phys_time_last_update) / static_cast<int>(max_iter);
					auto step = m_phys_msec_step;

					if (step < delta)
					{
						step = delta;
					}

					--i;

					m_phys_sim_running = true;

					// stop any running fx
					if (effects::effect_is_playing() || effects::effect_is_paused())
					{
						effects::stop_all();
					}

					physx_impl::run_frame(static_cast<float>(step) * 0.001f);
					m_phys_time_last_update += step;


					// part of 'dxPostProcessIslands'
					mScene->getActiveActors(m_phys_active_actor_count);

					constexpr float REDUCE_MSEC_BEGIN_AT_COUNT = 64.0f; // object count needed to start increasing m_phys_msec_step # og: 32
					constexpr float REDUCE_MSEC_RANGE_TO_MAX = 64.0f;   // range - how many objects are needed to hit g_phys_maxMsecStep (begin + range) # og: 18

					const auto step_for_count = (static_cast<float>(m_fx_active_actor_count) - REDUCE_MSEC_BEGIN_AT_COUNT) / REDUCE_MSEC_RANGE_TO_MAX;
					const auto s0 = step_for_count - 1.0f < 0.0f ? step_for_count : 1.0f;
					const auto s1 = 0.0f - step_for_count < 0.0f ? s0 : 0.0f;

					m_phys_msec_step = m_phys_min_msec_step + static_cast<int>((static_cast<float>((m_phys_max_msec_step - m_phys_min_msec_step)) * s1));

					if (m_phys_time_last_update >= m_phys_sim_tick)
					{
						break;
					}
				}
			}

			if (m_phys_time_last_snapshot > m_phys_sim_tick || m_phys_sim_tick > m_phys_time_last_update)
			{
				AssertS("m_phys_time_last_snapshot > m_phys_sim_tick || m_phys_sim_tick > m_phys_time_last_update");
			}

			if (m_phys_time_last_update <= m_phys_time_last_snapshot)
			{
				if (m_phys_time_last_update != m_phys_time_last_snapshot)
				{
					AssertS("m_phys_time_last_update != m_phys_time_last_snapshot");
				}
			}
			else
			{
				const auto delta = static_cast<float>((m_phys_sim_tick - m_phys_time_last_snapshot)) / static_cast<float>((m_phys_time_last_update - m_phys_time_last_snapshot));

				if (delta < 0.0f || delta > 1.0f)
				{
					AssertS("delta < 0.0f || delta > 1.0f");
				}
			}

			// *
			// update all dynamic prefabs

			if (!m_dynamic_prefabs.empty())
			{
				for (const auto& p : m_dynamic_prefabs)
				{
					const auto user_data = static_cast<userdata_prefab_s*>(p->userData);
					if (user_data && user_data->entity)
					{
						const auto pose = p->getGlobalPose();
						if (pose.p != user_data->last_transform.p || pose.q != user_data->last_transform.q)
						{
							// calculate delta pos
							game::vec3_t offset_pos;
							offset_pos[0] = pose.p.x - user_data->last_transform.p.x;
							offset_pos[1] = pose.p.y - user_data->last_transform.p.y;
							offset_pos[2] = pose.p.z - user_data->last_transform.p.z;

							// calculate total pos
							utils::vector::add(offset_pos, user_data->entity->firstActive->origin, offset_pos);


							// update entity origin (before rotation!)
							char str_buf[64] = {};

							if (sprintf_s(str_buf, "%.3f %.3f %.3f", offset_pos[0], offset_pos[1], offset_pos[2]))
							{
								game::SetKeyValue(user_data->entity->firstActive, "origin", str_buf);
							}

							utils::vector::copy(offset_pos, user_data->entity->firstActive->origin);



							// calculate delta rotation (quat) = to * from.inverse()
							auto q_delta = pose.q * user_data->last_transform.q.getConjugate();

							const auto x_axis = q_delta.getBasisVector0();
							const auto y_axis = q_delta.getBasisVector1();
							const auto z_axis = q_delta.getBasisVector2();

							float rotate_axis[4][3];
							rotate_axis[0][0] = pose.p.x;
							rotate_axis[0][1] = pose.p.y;
							rotate_axis[0][2] = pose.p.z;

							rotate_axis[1][0] = x_axis.x;
							rotate_axis[1][1] = x_axis.y;
							rotate_axis[1][2] = x_axis.z;

							rotate_axis[2][0] = y_axis.x;
							rotate_axis[2][1] = y_axis.y;
							rotate_axis[2][2] = y_axis.z;

							rotate_axis[3][0] = z_axis.x;
							rotate_axis[3][1] = z_axis.y;
							rotate_axis[3][2] = z_axis.z;

							// do the actual rotation - handles rotation around an arbitrary point + offsets the origin accordingly
							game::Select_RotateFixedSize(user_data->entity, user_data->def, rotate_axis);

							// update last_transform
							user_data->last_transform = pose;
						}
					}
				}
			}
		}

		if (m_phys_sim_run || (m_phys_sim_run && m_phys_sim_pause))
		{
			physx_impl::draw_debug_visualization();
		}
	}

	/**
	 * @brief	PhysX simulation frame for physic-enabled effect-objects
	 * @note	called from renderer::setup_viewinfo()
	 */
	void physx_impl::fx_frame()
	{
		const auto fxs = fx_system::FX_GetSystem(0);
		const auto efx = fx_system::ed_active_effect;

		m_fx_sim_running = false;
		m_effect_is_using_physics = false;

		const bool force_update = GET_GUI(ggui::camera_settings_dialog)->phys_force_frame_logic;

		if (process::get()->is_active())
		{
			return;
		}

		if ((efx && fxs) || (fxs && force_update))
		{
			if (!force_update)
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
			}

			const auto time_now = static_cast<uint32_t>(fxs->msecNow);
			if (m_fx_time_last_update < time_now)
			{
				m_fx_time_last_snapshot = m_fx_time_last_update;

				auto max_iter = 2u;
				for (auto i = 2u; ; max_iter = i)
				{
					if (!max_iter)
					{
						AssertS("!max_iter");
					}

					const auto delta = (time_now - m_fx_time_last_update) / static_cast<int>(max_iter);
					auto step = m_phys_msec_step;

					if (step < delta)
					{
						step = delta;
					}

					--i;

					m_fx_sim_running = true;

					// clear any dynamic prefabs when playing fx
					if (!m_dynamic_prefabs.empty())
					{
						reset_dynamic_prefabs();
						clear_dynamic_prefabs();
					}

					physx_impl::run_frame(static_cast<float>(step) * 0.001f);
					m_fx_time_last_update += step;


					// part of 'dxPostProcessIslands'
					mScene->getActiveActors(m_fx_active_actor_count);

					constexpr float REDUCE_MSEC_BEGIN_AT_COUNT = 64.0f; // object count needed to start increasing m_phys_msec_step # og: 32
					constexpr float REDUCE_MSEC_RANGE_TO_MAX = 64.0f;   // range - how many objects are needed to hit g_phys_maxMsecStep (begin + range) # og: 18

					const auto step_for_count = (static_cast<float>(m_fx_active_actor_count) - REDUCE_MSEC_BEGIN_AT_COUNT) / REDUCE_MSEC_RANGE_TO_MAX;
					const auto s0 = step_for_count - 1.0f < 0.0f ? step_for_count : 1.0f;
					const auto s1 = 0.0f - step_for_count < 0.0f ? s0 : 0.0f;

					m_phys_msec_step = m_phys_min_msec_step + static_cast<int>((static_cast<float>((m_phys_max_msec_step - m_phys_min_msec_step)) * s1));

					if (m_fx_time_last_update >= time_now)
					{
						break;
					}
				}
			}

			if (m_fx_time_last_snapshot > time_now || time_now > m_fx_time_last_update)
			{
				AssertS("m_fx_time_last_snapshot > time_now || time_now > m_fx_time_last_update");
			}

			if (m_fx_time_last_update <= m_fx_time_last_snapshot)
			{
				if (m_fx_time_last_update != m_fx_time_last_snapshot)
				{
					AssertS("m_fx_time_last_update != m_fx_time_last_snapshot");
				}
			}
			else
			{
				const auto delta = static_cast<float>((time_now - m_fx_time_last_snapshot)) / static_cast<float>((m_fx_time_last_update - m_fx_time_last_snapshot));

				if (delta < 0.0f || delta > 1.0f)
				{
					AssertS("delta < 0.0f || delta > 1.0f");
				}
			}
		}

		if (effects::effect_is_playing() || effects::effect_is_paused() || force_update)
		{
			physx_impl::draw_debug_visualization();
		}
	}

	/**
	 * @brief	create a new PhysX material using settings from a PhysPreset
	 * @param	preset the PhysPreset to use
	 * @return	pointer to the new PxMaterial
	 */
	physx::PxMaterial* physx_impl::create_material(game::PhysPreset* preset)
	{
		return mPhysics->createMaterial(preset->friction, preset->friction, preset->bounce);
	}

	/**
	 * @brief	create a convex mesh from a selected brush (sb->def)
	 * @param	sb brush used to create the convex mesh from
	 * @return	pointer to the new PxConvexMesh
	 */
	physx::PxConvexMesh* physx_impl::create_convex_mesh_from_brush(game::selbrush_def_t* sb)
	{
		physx::PxConvexMesh* mesh = nullptr;

		if (sb && sb->def)
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
				mesh = mPhysics->createConvexMesh(input);
			}
		}

		return mesh;
	}

	/**
	 * @brief	create a convex mesh from a selected brush (sb->def) which will be used \n
	 *			as the collision shape by all physic-enabled effect-objects (newly spawned)
	 * @param	sb brush used to create shape from
	 * @note	saves pointer to shape in 'm_effect_shape.custom_shape'
	 */
	void physx_impl::create_custom_shape_from_selection(game::selbrush_def_t* sb)
	{
		if (sb && sb->def)
		{
			m_effect_shape.custom_shape = create_convex_mesh_from_brush(sb);
		}
	}

	/**
	 * @brief	create a static PhysX actor from a selected brush (used for static collision by the simulation)
	 * @param	sb brush used to create the mesh from (sb->def)
	 * @param	is_prefab if selection is a prefab
	 * @param	position_offset	<prefab> local-space position offset
	 * @param	quat <prefab> local-space rotation
	 * @note	adds a pointer to the PhysX actor to 'm_static_brushes'
	 */
	void physx_impl::create_static_brush(game::selbrush_def_t* sb, bool is_prefab, const game::vec3_t position_offset, const float* quat)
	{
		if (sb && sb->def)
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
					bool contains_point = false;

					for (const auto& v : verts)
					{
						if (utils::vector::compare(&v.x, &t_point.x))
						{
							contains_point = true;
							break;
						}
					}

					if (!contains_point)
					{
						verts.emplace_back(t_point);
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

	/**
	 * @brief	create a static PhysX actor from a selected patch (used for static collision by the simulation)
	 * @param	sb patch used to create the mesh from (sb->def->patch)
	 * @param	position_offset	<prefab> local-space position offset
	 * @param	quat <prefab> local-space rotation
	 * @note	adds a pointer to the PhysX actor to 'm_static_terrain'
	 */
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

	/**
	 * @brief	check if the selection should be considered as static collision
	 * @param	b the selection to be checked
	 * @return	true if does not meet criteria
	 */
	bool physx_impl::exclude_brushes_from_static_collision(game::selbrush_def_t* b)
	{
		bool is_brushmodel = false;

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
				// allow brushmodels
				if (b->owner->firstActive->eclass->name != "script_brushmodel"s)
				{
					return true;
				}

				is_brushmodel = true;
			}
		}

		// include all generic solid + detail and weaponclip brushes
		if (!(b->brushflags & game::BRUSHFLAG_SOLID) && (b->def->contents & game::BRUSHCONTENTS_DETAIL || b->def->contents & game::BRUSHCONTENTS_WEAPONCLIP))
		{
			return true;
		}

		// skip all tooling (spawns, lightgrid ...) but include (some) clip
		if ((!is_brushmodel && b->brushflags & game::BRUSHFLAG_TOOL) && !(b->def->contents & 0x10000 || b->def->contents & 0x20000 || b->def->contents & 0x30000))
		{
			return true;
		}

		return false;
	}

	/**
	 * @brief	removes all static brushes and terrain actors from the scene\n
	 *			'm_static_brushes' & 'm_static_terrain' + counts
	 */
	void physx_impl::clear_static_collision()
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

		// #

		for (const auto terrain : phys->m_static_terrain)
		{
			terrain->release();
		}
		phys->m_static_terrain.clear();
	}


	/**
	 * @brief	thread-able static function generating static collision from \n
	 *			all non-selected brushes / patches / prefabs
	 * @note	called via components::process
	 */
	void physx_impl::create_static_collision()
	{
		game::printf_to_console("[PhysX] Building Static Collision ...");

		const auto phys = components::physx_impl::get();
		phys->clear_static_collision();

		phys->m_static_brushes.reserve(1000);
		phys->m_static_terrain.reserve(1000);

		//phys->clear_dynamic_prefabs();

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
						if (phys->exclude_brushes_from_static_collision(prefab))
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
				if (phys->exclude_brushes_from_static_collision(sb))
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
						else if (phys->exclude_brushes_from_static_collision(prefab))
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
					if (phys->exclude_brushes_from_static_collision(sb))
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

	/**
	 * @brief	create misc_models from all physics-enabled effect-objects present in the world \n
	 *			( at current position / rotation and scale when calling this function )
	 * @note	shows external console until done - pretty slow because of 'CreateEntityFromName'
	 */
	void physx_impl::convert_phys_to_misc_models()
	{
		if (m_fx_active_actor_count && fx_system::ed_active_effect)
		{
			const auto system = fx_system::FX_GetSystem(0);
			const auto entity_gui = GET_GUI(ggui::entity_dialog);

			utils::show_external_console(true);

			m_converted_misc_model_count = 0;

			std::uint16_t elemHandle = fx_system::ed_active_effect->firstElemHandle[fx_system::FX_ELEM_CLASS_NONSPRITE];
			if (elemHandle != UINT16_MAX)
			{
				game::Select_Deselect(true);

				ggui::entity_dialog::addprop_helper_s no_undo = {};

				while (elemHandle != UINT16_MAX)
				{
					fx_system::FxElem* elem = fx_system::FX_ElemFromHandle(system, elemHandle);
					fx_system::FxElemDef* elem_def = &fx_system::ed_active_effect->def->elemDefs[static_cast<std::uint8_t>(elem->defIndex)];

					if (elem_def->elemType <= fx_system::FX_ELEM_TYPE_LAST_SPRITE)
					{
						Assert();
					}

					if (elem_def->elemType == fx_system::FX_ELEM_TYPE_MODEL && elem_def->flags & fx_system::FX_ELEM_USE_MODEL_PHYSICS)
					{
						const auto actor = reinterpret_cast<physx::PxRigidDynamic*>(elem->___u8.physObjId);
						if (actor->userData)
						{
							const auto user_data = static_cast<userdata_s*>(actor->userData);

							const auto pos = actor->getGlobalPose().p;
							const auto quat = actor->getGlobalPose().q;

							float axis[3][3];
							fx_system::UnitQuatToAxis(&quat.x, axis);

							game::vec3_t angles = {};
							game::AxisToAngles(axis, angles);

							{
								game::Undo_ClearRedo();
								game::Undo_GeneralStart("create entity from physics");
#if 0
								// ------------------------------
								// generate a new brush

								game::vec3_t mins, maxs;
								const float half_width = 16.0f;
								utils::vector::set_vec3(mins, -half_width);
								utils::vector::set_vec3(maxs, half_width);


								// Brush_Alloc
								const auto new_b = utils::hook::call<game::brush_t_with_custom_def* (__cdecl)(void*, void*)>(0x4751E0)(game::g_qeglobals->random_texture_stuff, 0);
								game::Brush_Create(maxs, mins, new_b, 0);

								if (!new_b)
								{
									return;
								}

								game::Brush_BuildWindings(new_b, 1);
								++new_b->version;

								game::Entity_LinkBrush(new_b, game::g_world_entity()->firstActive);
								const auto b_linked = game::Brush_AddToList(new_b, game::g_world_entity());
								if (b_linked->onext || b_linked->oprev)
								{
									__debugbreak();
								}

								game::Brush_AddToList2(b_linked);
#endif
								if ((DWORD*)game::g_selected_brushes_next() == game::currSelectedBrushes)
								{

									game::CreateEntityBrush(cmainframe::activewnd->m_pXYWnd->m_nHeight / 2, cmainframe::activewnd->m_pXYWnd->m_nWidth / 2, cmainframe::activewnd->m_pXYWnd);
								}
								
								game::Undo_EndBrushList_Selected();

								// do not open the original modeldialog for this use-case, see: create_entity_from_name_intercept()
								g_block_radiant_modeldialog = true;
								game::CreateEntityFromName("misc_model"); // CreateEntityFromName takes ~ 24 ms
								g_block_radiant_modeldialog = false;

								// use visuals.array if visual count != 1
								//entity_gui->add_prop("model", elem_def->visuals.instance.model->name, &no_undo);

								entity_gui->add_prop("model", user_data->model_name.c_str(), &no_undo);

								char str_buf[64] = {};
								if (sprintf_s(str_buf, "%.3f %.3f %.3f", pos[0], pos[1], pos[2]))
								{
									entity_gui->add_prop("origin", str_buf, &no_undo);
								}

								if (sprintf_s(str_buf, "%.3f %.3f %.3f", angles[0], angles[1], angles[2]))
								{
									entity_gui->add_prop("angles", str_buf, &no_undo);
								}

								// * this is not a per model scale
								//if (elem_def->visSamples && elem_def->visSamples->base.scale != 1.0f)
								//{
									//entity_gui->add_prop("modelscale", std::to_string(elem_def->visSamples->base.scale).c_str(), &no_undo);
								//}


								// #
								// get scale for the current model

								fx_system::FxDrawState draw = {};
								draw.effect = fx_system::ed_active_effect;
								draw.msecDraw = system->msecDraw;
								draw.elem = elem;
								draw.elemDef = elem_def;

								fx_system::FX_DrawElement_Setup_1_(&draw, elem->msecBegin, static_cast<std::uint8_t>(elem->sequence), elem->___u8.origin, nullptr);

								const float rnd = fx_system::fx_randomTable[27 + draw.preVisState.randomSeed];
								const float graph_b = rnd * draw.preVisState.refState[1].amplitude.scale + draw.preVisState.refState[1].base.scale;
								const float graph_a = rnd * draw.preVisState.refState->amplitude.scale + draw.preVisState.refState->base.scale;
								draw.visState.scale = draw.preVisState.sampleLerpInv * graph_a + graph_b * draw.preVisState.sampleLerp;

								if (draw.visState.scale >= 0.0f)
								{
									entity_gui->add_prop("modelscale", std::to_string(draw.visState.scale).c_str(), &no_undo);
								}

								game::Undo_End(); // works #1
							}


							game::printf_to_console("spawned model #%d\n", m_converted_misc_model_count);
							m_converted_misc_model_count++;

							game::Select_Deselect(true);
						}
					}

					elemHandle = elem->nextElemHandleInEffect;
				}
			}

			m_converted_misc_model_count = 0;

			utils::show_external_console(false);
		}
	}

	/**
	 * @brief	release given actor and its material and free userData
	 * @param	id	handle to a PxRigidDynamic actor
	 */
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

	/**
	 * @brief		get position and rotation data for a given actor
	 * @param		id handle to a PxRigidDynamic actor
	 * @param[out]	out_pos	 current actor position
	 * @param[out]	out_quat current actor rotation
	 */
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

	/**
	 * @brief	clear dynamic prefab actors within 'm_dynamic_prefabs' incl. their userData
	 * @param	clear_state reset run and pause states
	 */
	void physx_impl::clear_dynamic_prefabs(bool clear_state)
	{
		for (const auto p : m_dynamic_prefabs)
		{
			if (p->userData)
			{
				const auto data = static_cast<userdata_prefab_s*>(p->userData);
				delete(data);
			}

			p->release();
		}

		m_dynamic_prefabs.clear();

		if (clear_state)
		{
			m_phys_sim_run = false;
			m_phys_sim_pause = false;
			m_phys_sim_running = false;
		}
	}

	/**
	 * @brief	reset all dynamic prefab actors to values upon creation (position & rotation)
	 */
	void physx_impl::reset_dynamic_prefabs()
	{
		ggui::entity_dialog::addprop_helper_s no_undo = {};

		for (const auto& p : m_dynamic_prefabs)
		{
			const auto user_data = static_cast<userdata_prefab_s*>(p->userData);
			if (user_data && user_data->entity && user_data->entity->firstActive)
			{
				// reset actor world-space position and rotation
				p->setGlobalPose(user_data->initial_transform);

				// same for the last transform
				user_data->last_transform = user_data->initial_transform;

				// clear velocity
				const auto null_vec = physx::PxVec3(0.0f);
				p->setLinearVelocity(null_vec);
				p->setAngularVelocity(null_vec);

				// check
				if (user_data->entity->firstActive->brushes.oprev->owner && user_data->entity->firstActive->brushes.oprev->owner->eclass)
				{
					// reset prefab entity

					char str_buf[64] = {};
					if (sprintf_s(str_buf, "%.3f %.3f %.3f", user_data->initial_ent_origin[0], user_data->initial_ent_origin[1], user_data->initial_ent_origin[2]))
					{
						game::SetKeyValue(user_data->entity->firstActive, "origin", str_buf);
					}

					if (sprintf_s(str_buf, "%.3f %.3f %.3f", user_data->initial_ent_angles[0], user_data->initial_ent_angles[1], user_data->initial_ent_angles[2]))
					{
						game::SetKeyValue(user_data->entity->firstActive, "angles", str_buf);
					}

					utils::vector::copy(user_data->initial_ent_origin, user_data->entity->firstActive->origin);
					user_data->entity->firstActive->version0++;
				}
				else
				{
					clear_dynamic_prefabs();
					return;
				}
			}
		}
	}


	// creates a physics actor from a prefab

	/**
	 * @brief	turns selected prefab into a dynamic PhysX actor
	 * @param	sb the selected prefab
	 * @note	creates a convex hull around all brushes within the prefab 
	 */
	void physx_impl::create_dynamic_prefab(game::selbrush_def_t* sb)
	{
		// prefab brushes and terrain
		if (sb && sb->owner && sb->owner->prefab && sb->owner->firstActive && sb->owner->firstActive->eclass && sb->owner->firstActive->eclass->classtype & game::ECLASS_PREFAB)
		{
			std::vector<physx::PxVec3> hull_verts;
			hull_verts.reserve(50);

			std::vector<physx::PxVec3> local_bounds;
			local_bounds.reserve(10);

			game::vec3_t prefab_angles = {};
			game::vec4_t prefab_quat = { 0.0f, 0.0f, 0.0f, 1.0f };

			// thats the origin of the placed prefab which might have an origin of 0 0 0
			// even tho it's contents are not at 0 0 0 in the parent world
			const auto prefab_origin = sb->owner->firstActive->origin;

			// calculate the real center point of the brushes contained in the prefab
			// in relation to the root world
			game::vec3_t real_origin;
			utils::vector::add(sb->def->mins, sb->def->maxs, real_origin);
			utils::vector::scale(real_origin, 0.5f, real_origin);

			// angles to quat - use identity if prefab has no angles kvp
			if (GET_GUI(ggui::entity_dialog)->get_vec3_for_key_from_entity(sb->owner->firstActive, prefab_angles, "angles"))
			{
				game::orientation_t orientation = {};
				game::AnglesToAxis(prefab_angles, &orientation.axis[0][0]);
				fx_system::AxisToQuat(orientation.axis, prefab_quat);
			}

			FOR_ALL_BRUSHES(prefab, sb->owner->prefab->active_brushlist, sb->owner->prefab->active_brushlist_next)
			{
				if (prefab && prefab->def)
				{
					if (exclude_brushes_from_static_collision(prefab))
					{
						continue;
					}

					for (auto f = 0; f < prefab->def->facecount; f++)
					{
						const auto face = &prefab->def->brush_faces[f];
						for (auto p = 0; face->w && p < face->w->numPoints; p++)
						{
							// calculate 'local-space' vertices by subtracting the bounding box center (world) from the brush points (world)
							game::vec3_t hull_point = { face->w->points[p][0], face->w->points[p][1], face->w->points[p][2] };

							// bbox_center (correct center but wrong hull) --- real_origin (actually creates outer hull but origin is wrong -> see 'local_bounds' usage)
							utils::vector::subtract(hull_point, real_origin, hull_point); 

							bool contains_point = false;
							for (const auto& v : hull_verts)
							{
								if (utils::vector::compare(&v.x, hull_point))
								{
									contains_point = true;
									break;
								}
							}

							if (!contains_point)
							{
								hull_verts.emplace_back(physx::PxVec3(hull_point[0], hull_point[1], hull_point[2]));
							}
						}
					}

					// save bounding box mid-point of the local brush
					game::vec3_t bbox_center;
					utils::vector::add(prefab->def->mins, prefab->def->maxs, bbox_center);
					utils::vector::scale(bbox_center, 0.5f, bbox_center);

					local_bounds.emplace_back(physx::PxVec3(bbox_center[0], bbox_center[1], bbox_center[2]));
				}
			}

			if (hull_verts.size() > 2)
			{
				{
					// get outer shell (bounds) of all local brush bounds
					game::vec3_t mins, maxs;
					utils::vector::set_vec3(mins, FLT_MAX);
					utils::vector::set_vec3(maxs, -FLT_MAX);

					for (const auto& local : local_bounds)
					{
						for (auto v = 0; v < 3; v++)
						{
							// mins :: find the closest point on each axis
							if (mins[v] > local[v])
								mins[v] = local[v];

							// maxs :: find the furthest point on each axis
							if (maxs[v] < local[v])
								maxs[v] = local[v];
						}
					}

					game::vec3_t local_center, origin_offset;

					// get mid-point
					utils::vector::add(mins, maxs, local_center);
					utils::vector::scale(local_center, 0.5f, local_center);

					// offset the actual origin by the total local mid-point
					utils::vector::subtract(real_origin, local_center, origin_offset);

					// add offset to all hull vertices
					const auto px_vec3 = physx::PxVec3(origin_offset[0], origin_offset[1], origin_offset[2]);
					for (auto& hull : hull_verts)
					{
						hull += px_vec3;
					}
				}

				// build a convex mesh
				physx::PxConvexMeshDesc convex_desc;
				convex_desc.points.count = hull_verts.size();
				convex_desc.points.stride = sizeof(physx::PxVec3);
				convex_desc.points.data = hull_verts.data();
				convex_desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

				physx::PxDefaultMemoryOutputStream buf;
				physx::PxConvexMeshCookingResult::Enum result;

				if (mCooking->cookConvexMesh(convex_desc, buf, &result))
				{
					physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
					physx::PxConvexMesh* convexMesh = mPhysics->createConvexMesh(input);

					auto* actor = mPhysics->createRigidDynamic(physx::PxTransform(0.0f, 0.0f, 0.0f));
					actor->setActorFlags(physx::PxActorFlag::eVISUALIZATION);

					const auto data = new userdata_prefab_s();
					actor->userData = data;

					// save entity and brush data
					data->entity = sb->owner;
					data->def = sb->def;
					utils::vector::copy(prefab_origin, data->initial_ent_origin);
					utils::vector::copy(prefab_angles, data->initial_ent_angles);

					physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, physx::PxConvexMeshGeometry(convexMesh), *m_static_collision_material);
					actor->attachShape(*shape);
					shape->release();

					physx::PxRigidBodyExt::updateMassAndInertia(*actor, 10.0f);
					mScene->addActor(*actor);

					actor->setGlobalPose(physx::PxTransform(
						physx::PxVec3(real_origin[0], real_origin[1], real_origin[2]),
						physx::PxQuat(prefab_quat[0], prefab_quat[1], prefab_quat[2], prefab_quat[3])));


					// save initial pose
					const auto pose = actor->getGlobalPose();
					data->initial_transform = physx::PxTransform(pose.p, pose.q);
					data->last_transform = data->initial_transform;

					m_dynamic_prefabs.push_back(actor);

					// cull checks at 0x407AF4, 0x407BEC & 0x408000 result in flickering prefab entities
					// renderer::cubic_culling_overwrite_check() skips culling function if custom_no_cull is set
					data->entity->firstActive->custom_no_cull = true;
				}
			}
		}
	}

	/**
	 * @brief					create a dynamic PhysX actor from a xmodel using its bounds 
	 * @param model				the xmodel to use
	 * @param world_pos			the current model position in world-space
	 * @param quat				the current rotation
	 * @param velocity			initial linear velocity
	 * @param angular_velocity	initial angular velocity
	 * @return					a handle to the actor (PxRigidDynamic)
	 */
	int physx_impl::create_physx_object(game::XModel* model, const float* world_pos, const float* quat, const float* velocity, const float* angular_velocity, fx_system::FX_SYSTEM_ scene)
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

		const auto data = new userdata_s();
		data->material = material;
		data->model_name = model->name;
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

		switch (scene)
		{
		default:
		case fx_system::FX_SYSTEM_CAMERA:
			mScene->addActor(*actor);
			break;

		case fx_system::FX_SYSTEM_BROWSER:
			mSceneEffectBrowser->addActor(*actor);
			break;
		}

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

	/**
	 * @brief	updates the PhysX material used by static collision and dynamic prefabs
	 */
	void physx_impl::update_static_collision_material()
	{
		const auto gui = GET_GUI(ggui::camera_settings_dialog);

		m_static_collision_material->setStaticFriction(gui->phys_material[0]);
		m_static_collision_material->setDynamicFriction(gui->phys_material[1]);
		m_static_collision_material->setRestitution(gui->phys_material[2]);
	}

	/**
	 * @brief				callback that fires when two actor contact each other\n
	 *						only active when 'USE_CONTACT_CALLBACK' is true
	 */
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

	/**
	 * @brief	used to identify collisions between static collision and physic's enabled objects //n
	 *			only active when 'USE_CONTACT_CALLBACK' is true
	 */
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

	void physx_impl::spawn_character()
	{
		// *
		// generate static collision and activate movement

		const auto process = components::process::get();

		process->set_indicator(components::process::INDICATOR_TYPE_PROGRESS);
		process->set_indicator_string("Building Static Collision");
		process->set_process_type(components::process::PROC_TYPE_GENERIC);
		process->set_success_toast_string(ICON_FA_RUNNING);

		process->set_thread_callback([]
			{
				components::physx_impl::create_static_collision();
			});

		process->set_progress_callback([]
			{
				const auto current = static_cast<float>(components::physx_impl::get()->m_static_brush_count + components::physx_impl::get()->m_static_terrain_count);
				const auto total = static_cast<float>(components::physx_impl::get()->m_static_brush_estimated_count + components::physx_impl::get()->m_static_terrain_estimated_count);
				components::process::get()->m_indicator_progress = current / total;
			});

		// activate movement on process end
		process->set_post_process_callback([]
			{
				const auto camera = GET_GUI(ggui::camera_dialog);
				const auto px = components::physx_impl::get();
				px->m_character_controller_enabled = !px->m_character_controller_enabled;

				if (px->m_character_controller_enabled)
				{
					camera->rtt_set_focus_state(true);
					camera->rtt_set_hovered_state(true);

					const auto cam_pos = cmainframe::activewnd->m_pCamWnd->camera.origin;
					px->m_cct_controller->get_controller()->setPosition({ cam_pos[0], cam_pos[1], cam_pos[2] - 24.5 }); // 24.5 ??

					px->m_cct_camera->m_player_velocity.x = 0.0f;
					px->m_cct_camera->m_player_velocity.y = 0.0f;
					px->m_cct_camera->m_player_velocity.z = 0.0f;

					px->m_cct_camera->enable_cct(true);
				}
			});

		process->create_process();
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

		dvars::physx_camera_sensitivity = dvars::register_float(
			/* name		*/ "physx_camera_sensitivity",
			/* default	*/ 80.0f,
			/* mins		*/ 0.0f,
			/* maxs		*/ FLT_MAX,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "camera sensitivity when using physx movement mode");
	}

	physx_impl::physx_impl()
	{
		physx_impl::p_this = this;
		
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

		m_static_collision_material = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);

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
		mSceneEffectBrowser = mPhysics->createScene(scene_desc);

		// add ground plane for effects browser
		PxRigidStatic* effects_browser_plane = PxCreatePlane(*mPhysics, PxPlane(PxVec3(0.0f, 0.0f, 1.0f), 0), *m_static_collision_material);
		mSceneEffectBrowser->addActor(*effects_browser_plane);

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

		// *
		// character controller

		m_manager = PxCreateControllerManager(*mScene);
		//m_manager->setDebugRenderingFlags(PxControllerDebugRenderFlag::eALL);
		//m_manager->setPreventVerticalSlidingAgainstCeiling(true);

		const bool is_capsule = false;

		m_cct_material = mPhysics->createMaterial(0.5f, 0.5f, 0.5f); // not related to movement

		physx_cct_controller_desc desc;
		desc.m_type = is_capsule ? PxControllerShapeType::eCAPSULE : PxControllerShapeType::eBOX;
		desc.m_position = PxExtendedVec3(0.0, 0.0, 0.0);
		desc.m_slope_limit = 0.7f;
		desc.m_contact_offset = 1.0f;
		desc.m_step_offset = 0.0f; // we set this dynamically when we trace a step nearby
		desc.m_invisible_wall_height = 0.0f;
		desc.m_max_jump_height = 64.0f;
		desc.m_radius = 15.0f;
		desc.m_height = 69.0f * (is_capsule ? 0.5f : 1.0f);
		desc.m_crouch_height = 48.0f * (is_capsule ? 0.5f : 1.0f);
		//desc.m_behavior_callback = new behavior_feedback(); // kinda useless

		m_cct_controller = new (physx_cct_controller)();
		m_cct_controller->init(desc, m_manager);

		m_cct_camera = new (physx_cct_camera)();
		m_cct_camera->set_controlled(m_cct_controller);


		command::register_command_with_hotkey("physx_movement"s, [this](auto)
		{
			physx_impl::spawn_character();
		});

#if DEBUG
		command::register_command("camtest"s, [this](const std::vector<std::string>&)
		{
			this->m_character_controller_enabled = !this->m_character_controller_enabled;
		});
#endif

		/*
		command::register_command("camreset"s, [this](const std::vector<std::string>&)
		{
			this->m_cct_controller->reset();
		});*/
	}

	physx_impl::~physx_impl()
	{
		//PxCloseExtensions();

		mCooking->release();
		mPhysics->release();
		mFoundation->release();
	}
}
