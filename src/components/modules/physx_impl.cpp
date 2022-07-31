#include "std_include.hpp"

constexpr bool USE_PVD = false; // PhysX Visual Debugger
constexpr bool USE_CONTACT_CALLBACK = false; // can be used to implement effect spawing on impact

// fx_system impacts:
// - 'play new effect on impact' only works when 'enable model physics simulation' is turned off
// - 'play new effect on impact' does not kill the elem

namespace components
{
	physx_impl* physx_impl::p_this = nullptr;

	constexpr int m_phys_min_msec_step = 3;	// 11
	constexpr int m_phys_max_msec_step = 11;	// 67

	// components::effects::camera_onpaint_intercept()
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

	void physx_impl::run_frame(float seconds)
	{
		mScene->simulate(seconds);
		mScene->fetchResults(true);
	}

	// separate frame for physics only (not FX related - eg. brushes)
	// -> renderer::setup_viewinfo
	void physx_impl::phys_frame()
	{
		if (game::glob::is_loading_map)
		{
			/*if (!m_dynamic_prefabs.empty())
			{
				clear_dynamic_prefabs();
			}*/

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
						Assert();
					}

					const auto delta = (m_phys_sim_tick - m_phys_time_last_update) / static_cast<int>(max_iter);
					auto step = m_phys_msec_step;

					if (step < delta)
					{
						step = delta;
					}

					--i;

					m_phys_sim_running = true;

					physx_impl::run_frame(static_cast<float>(step) * 0.001f);
					m_phys_time_last_update += step;

					// #
					// dxPostProcessIslands(static_cast<PhysWorld>(worldIndex));

					mScene->getActiveActors(m_phys_active_actor_count);


					constexpr float REDUCE_MSEC_BEGIN_AT_COUNT = 64.0f; // object count needed to start increasing m_phys_msec_step # og: 32
					constexpr float REDUCE_MSEC_RANGE_TO_MAX = 64.0f;   // range - how many objects are needed to hit g_phys_maxMsecStep # og: 18

					const auto step_for_count = (static_cast<float>(m_active_actor_count) - REDUCE_MSEC_BEGIN_AT_COUNT) / REDUCE_MSEC_RANGE_TO_MAX;
					const auto s0 = step_for_count - 1.0f < 0.0f ? step_for_count : 1.0f;
					const auto s1 = 0.0f - step_for_count < 0.0f ? s0 : 0.0f;

					m_phys_msec_step = m_phys_min_msec_step + static_cast<int>((static_cast<float>((m_phys_max_msec_step - m_phys_min_msec_step)) * s1));

					// #

					if (m_phys_time_last_update >= m_phys_sim_tick)
					{
						break;
					}
				}
			}

			if (m_phys_time_last_snapshot > m_phys_sim_tick || m_phys_sim_tick > m_phys_time_last_update)
			{
				Assert();
			}

			if (m_phys_time_last_update <= m_phys_time_last_snapshot)
			{
				if (m_phys_time_last_update != m_phys_time_last_snapshot)
				{
					Assert();
				}

				m_time_now_lerp_frac = 1.0f;
			}
			else
			{
				const auto delta = static_cast<float>((m_phys_sim_tick - m_phys_time_last_snapshot)) / static_cast<float>((m_phys_time_last_update - m_phys_time_last_snapshot));
				m_time_now_lerp_frac = delta;

				if (delta < 0.0f || delta > 1.0f)
				{
					Assert();
				}
			}

			if (!m_dynamic_prefabs.empty())
			{
				for (const auto& p : m_dynamic_prefabs)
				{
					const auto user_data = static_cast<userdata_s*>(p->userData);
					if (user_data && user_data->entity)
					{
						const auto pose = p->getGlobalPose();

						const physx::PxU32 numShapes = p->getNbShapes();
						const auto shapes = (physx::PxShape**)game::Z_Malloc(sizeof(physx::PxShape*) * numShapes);
						p->getShapes(shapes, numShapes);

						const auto local_pose = shapes[0]->getLocalPose();

						/*for (physx::PxU32 i = 0; i < numShapes; i++)
						{
							physx::PxShape* shape = shapes[i];
						}*/
						game::Z_Free(shapes);

						//float angle = 0.0f;
						//physx::PxVec3 axis_test;
						//pose.q.toRadiansAndUnitAxis(angle, axis_test);
						
						float axis[3][3];

						//auto qq = local_pose.q * pose.q;
						//fx_system::UnitQuatToAxis(&qq.x, axis);

						fx_system::UnitQuatToAxis(&pose.q.x, axis);

						game::vec3_t angles = {};
						game::AxisToAngles(axis, angles);

						if (pose.p != user_data->last_transform.p || pose.q != user_data->last_transform.q) /*!utils::vector::compare(angles, user_data->last_angles))*/
						{
							//auto sb = game::g_selected_brushes(); // BAD

							//game::vec3_t brush_center;
							//utils::vector::add(sb->def->mins, sb->def->maxs, brush_center);
							//utils::vector::scale(brush_center, 0.5f, brush_center);

							////// rotation

							//auto curr_quat = p->getGlobalPose().q;
							//auto q_delta = curr_quat * user_data->last_quat.getConjugate();

							//float rotate_axis[4][3];
							//const auto x_axis = q_delta.getBasisVector0();
							//const auto y_axis = q_delta.getBasisVector1();
							//const auto z_axis = q_delta.getBasisVector2();

							//rotate_axis[0][0] = pose.p.x; //brush_center[0];
							//rotate_axis[0][1] = pose.p.y; //brush_center[1];
							//rotate_axis[0][2] = pose.p.z; //brush_center[2];

							//rotate_axis[1][0] = x_axis.x;
							//rotate_axis[1][1] = x_axis.y;
							//rotate_axis[1][2] = x_axis.z;

							//rotate_axis[2][0] = y_axis.x;
							//rotate_axis[2][1] = y_axis.y;
							//rotate_axis[2][2] = y_axis.z;

							//rotate_axis[3][0] = z_axis.x;
							//rotate_axis[3][1] = z_axis.y;
							//rotate_axis[3][2] = z_axis.z;

							////
							//game::Select_ApplyMatrix(&rotate_axis[0][0], sb, false, 1.0f, false);
							////user_data->last_quat = curr_quat;

							//user_data->entity->firstActive->version0++;

							game::vec3_t offset_pos;
							//utils::vector::subtract(&pos.x, user_data->initial_origin, offset_pos);

							//offset_pos[0] = user_data->initial_ent_origin[0]; /*user_data->initial_ent_origin[0] - local_pose.p.x */ /*+ pose.p.x*/;
							//offset_pos[1] = user_data->initial_ent_origin[1]; /*user_data->initial_ent_origin[1] - local_pose.p.y */ /*+ pose.p.y*/;
							//offset_pos[2] = user_data->initial_ent_origin[2]; /*user_data->initial_ent_origin[2] - local_pose.p.z */ /*+ pose.p.z*/;

							////utils::vector::rotate_point(offset_pos, axis[0]);
							//utils::vector::rotate_point(offset_pos, &pose.q.x, offset_pos);

							//offset_pos[0] += pose.p.x;
							//offset_pos[1] += pose.p.y;
							//offset_pos[2] += pose.p.z;

							//offset_pos[0] += (pose.p.x /*+ local_pose.p.x*/); //user_data->initial_ent_origin[0];
							//offset_pos[1] += (pose.p.y /*+ local_pose.p.y*/); //user_data->initial_ent_origin[1];
							//offset_pos[2] += (pose.p.z /*+ local_pose.p.z*/); //user_data->initial_ent_origin[2];

	/*						auto tt = physx::PxTransform(physx::PxVec3(offset_pos[0], offset_pos[1], offset_pos[2])).rotate(local_pose.p);
	*/
							offset_pos[0] = pose.p.x - user_data->last_transform.p.x;
							offset_pos[1] = pose.p.y - user_data->last_transform.p.y;
							offset_pos[2] = pose.p.z - user_data->last_transform.p.z;

							utils::vector::add(offset_pos, user_data->entity->firstActive->origin, offset_pos);

							char str_buf[64] = {};
							if (sprintf_s(str_buf, "%.3f %.3f %.3f", 
								offset_pos[0], offset_pos[1], offset_pos[2]))
							{
								game::SetKeyValue(user_data->entity->firstActive, "origin", str_buf);
							}

							utils::vector::copy(offset_pos, user_data->entity->firstActive->origin);






							auto q_delta = pose.q * user_data->last_transform.q.getConjugate();

							float rotate_axis[4][3];
							const auto x_axis = q_delta.getBasisVector0();
							const auto y_axis = q_delta.getBasisVector1();
							const auto z_axis = q_delta.getBasisVector2();

							rotate_axis[0][0] = pose.p.x;
							rotate_axis[0][1] = pose.p.y;
							rotate_axis[0][2] = pose.p.z;

							rotate_axis[1][0] = x_axis.x; //axis[0][0];
							rotate_axis[1][1] = x_axis.y; //axis[0][1];
							rotate_axis[1][2] = x_axis.z; //axis[0][2];

							rotate_axis[2][0] = y_axis.x; //axis[1][0];
							rotate_axis[2][1] = y_axis.y; //axis[1][1];
							rotate_axis[2][2] = y_axis.z; //axis[1][2];

							rotate_axis[3][0] = z_axis.x; //axis[2][0];
							rotate_axis[3][1] = z_axis.y; //axis[2][1];
							rotate_axis[3][2] = z_axis.z; //axis[2][2];

							game::Select_RotateFixedSize(user_data->entity, user_data->def, rotate_axis);













							user_data->last_transform = pose;
							//user_data->last_pos = pose.p;
							//user_data->last_quat = pose.q;

							game::vec3_t offset_angles;
							offset_angles[0] = /*user_data->initial_ent_angles[0] +*/ angles[0];
							offset_angles[1] = /*user_data->initial_ent_angles[1] +*/ angles[1];
							offset_angles[2] = /*user_data->initial_ent_angles[2] +*/ angles[2];

							/*if (sprintf_s(str_buf, "%.3f %.3f %.3f", offset_angles[0], offset_angles[1], offset_angles[2]))
							{
								game::SetKeyValue(user_data->entity->firstActive, "angles", str_buf);
							}*/

							/*utils::vector::copy(offset_pos, user_data->entity->firstActive->origin);
							utils::vector::copy(angles, user_data->last_angles);
							user_data->last_pos = pos;

							user_data->entity->firstActive->version0++;*/
						}
					}
				}
			}

			// modifying actual brushes
#if 0
			if (!m_dynamic_brushes.empty())
			{
				for (const auto& b : m_dynamic_brushes)
				{
					const auto user_data = static_cast<userdata_s*>(b->userData);
					if (user_data && user_data->brush)
					{
						// works but brush will loose its uv's (will move through the texture)
						game::vec3_t half_bounds;
						utils::vector::subtract(user_data->brush->maxs, user_data->brush->mins, half_bounds);
						utils::vector::scale(half_bounds, 0.5f, half_bounds);

						game::vec3_t brush_center;
						utils::vector::add(user_data->brush->mins, user_data->brush->maxs, brush_center);
						utils::vector::scale(brush_center, 0.5f, brush_center);

						// rotation

						auto curr_quat = b->getGlobalPose().q;
						auto q_delta = curr_quat * user_data->old_quat.getConjugate();

						float rotate_axis[4][3];
						const auto x_axis = q_delta.getBasisVector0();
						const auto y_axis = q_delta.getBasisVector1();
						const auto z_axis = q_delta.getBasisVector2();

						rotate_axis[0][0] = brush_center[0];
						rotate_axis[0][1] = brush_center[1];
						rotate_axis[0][2] = brush_center[2];

						rotate_axis[1][0] = x_axis.x;
						rotate_axis[1][1] = x_axis.y;
						rotate_axis[1][2] = x_axis.z;

						rotate_axis[2][0] = y_axis.x;
						rotate_axis[2][1] = y_axis.y;
						rotate_axis[2][2] = y_axis.z;

						rotate_axis[3][0] = z_axis.x;
						rotate_axis[3][1] = z_axis.y;
						rotate_axis[3][2] = z_axis.z;

						auto sb = game::g_selected_brushes(); // BAD
						game::Select_ApplyMatrix(&rotate_axis[0][0], sb, false, 1.0f, false);

						user_data->old_quat = curr_quat;


						// pos

						game::vec3_t pos_delta;
						const auto pos = b->getGlobalPose().p;
						
						utils::vector::subtract(&pos.x, user_data->old_origin, pos_delta);
						game::Brush_Move(pos_delta, user_data->brush, false);

						utils::vector::copy(&pos.x, user_data->old_origin, 3);
					}
				}
			}
#endif
		}

		if (m_phys_sim_run || (m_phys_sim_run && m_phys_sim_pause))
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
		}
	}

	// -> renderer::setup_viewinfo
	void physx_impl::fx_frame()
	{
		const auto fxs = fx_system::FX_GetSystem(0);
		const auto efx = fx_system::ed_active_effect;

		m_fx_sim_running = false;
		m_effect_is_using_physics = false;

		const bool force_update = GET_GUI(ggui::camera_settings_dialog)->phys_force_frame_logic;

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
						Assert();
					}

					const auto delta = (time_now - m_fx_time_last_update) / static_cast<int>(max_iter);
					auto step = m_phys_msec_step;

					if (step < delta)
					{
						step = delta;
					}

					--i;

					m_fx_sim_running = true;

					physx_impl::run_frame(static_cast<float>(step) * 0.001f);
					m_fx_time_last_update += step;


					// #
					// dxPostProcessIslands(static_cast<PhysWorld>(worldIndex));

					mScene->getActiveActors(m_active_actor_count);


					constexpr float REDUCE_MSEC_BEGIN_AT_COUNT = 64.0f; // object count needed to start increasing m_phys_msec_step # og: 32
					constexpr float REDUCE_MSEC_RANGE_TO_MAX = 64.0f;   // range - how many objects are needed to hit g_phys_maxMsecStep # og: 18

					const auto step_for_count = (static_cast<float>(m_active_actor_count) - REDUCE_MSEC_BEGIN_AT_COUNT) / REDUCE_MSEC_RANGE_TO_MAX;
					const auto s0 = step_for_count - 1.0f < 0.0f ? step_for_count : 1.0f;
					const auto s1 = 0.0f - step_for_count < 0.0f ? s0 : 0.0f;

					m_phys_msec_step = m_phys_min_msec_step + static_cast<int>((static_cast<float>((m_phys_max_msec_step - m_phys_min_msec_step)) * s1));

					// #

					if (m_fx_time_last_update >= time_now)
					{
						break;
					}
				}
			}

			if (m_fx_time_last_snapshot > time_now || time_now > m_fx_time_last_update)
			{
				Assert();
			}

			if (m_fx_time_last_update <= m_fx_time_last_snapshot)
			{
				if (m_fx_time_last_update != m_fx_time_last_snapshot)
				{
					Assert();
				}

				m_time_now_lerp_frac = 1.0f;
			}
			else
			{
				const auto delta = static_cast<float>((time_now - m_fx_time_last_snapshot)) / static_cast<float>((m_fx_time_last_update - m_fx_time_last_snapshot));
				m_time_now_lerp_frac = delta;

				if (delta < 0.0f || delta > 1.0f)
				{
					Assert();
				}
			}
		}

		if (effects::effect_is_playing() || effects::effect_is_paused() || force_update)
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
		}
	}

	physx::PxMaterial* physx_impl::create_material(game::PhysPreset* preset)
	{
		return mPhysics->createMaterial(preset->friction, preset->friction, preset->bounce);
	}

	physx::PxConvexMesh* physx_impl::create_convex_mesh_from_brush(game::selbrush_def_t* sb)
	{
		physx::PxConvexMesh* mesh = nullptr;

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
				mesh = mPhysics->createConvexMesh(input);
			}
		}

		return mesh;
	}

	// saves ptr to shape in 'm_effect_shape.custom_shape'
	void physx_impl::create_custom_shape_from_selection(game::selbrush_def_t* sb)
	{
		m_effect_shape.custom_shape = create_convex_mesh_from_brush(sb);
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
		if (m_active_actor_count && fx_system::ed_active_effect)
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

	// clear m_dynamic_brushes
	void physx_impl::clear_dynamic_prefabs()
	{
		for (const auto p : m_dynamic_prefabs)
		{
			p->release();
		}
		m_dynamic_prefabs.clear();
	}


	// clear m_dynamic_brushes
	void physx_impl::reset_dynamic_prefabs()
	{
		ggui::entity_dialog::addprop_helper_s no_undo = {};

		for (const auto& p : m_dynamic_prefabs)
		{
			const auto user_data = static_cast<userdata_s*>(p->userData);
			if (user_data && user_data->entity && user_data->entity->firstActive)
			{
				/*p->setGlobalPose(physx::PxTransform(
					physx::PxVec3(user_data->initial_origin[0], user_data->initial_origin[1], user_data->initial_origin[2]),
					user_data->initial_quat));*/

				/*p->setGlobalPose(physx::PxTransform(
					physx::PxVec3(user_data->initial_origin[0], user_data->initial_origin[1], user_data->initial_origin[2]),
					user_data->initial_quat));*/

				p->setGlobalPose(user_data->initial_transform); //physx::PxTransform(
					//user_data->initial_pos,
					//user_data->initial_quat));

				user_data->last_transform = user_data->initial_transform;

				const auto null_vec = physx::PxVec3(0.0f);
				p->setLinearVelocity(null_vec);
				p->setAngularVelocity(null_vec);

				//utils::vector::copy(user_data->initial_ent_angles, user_data->last_angles);
				//user_data->last_pos = physx::PxVec3(user_data->initial_origin[0], user_data->initial_origin[1], user_data->initial_origin[2]);
				//user_data->last_pos = physx::PxVec3(0.0f, 0.0f, 0.0f);

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
				//user_data->entity->firstActive->version++;
				user_data->entity->firstActive->version0++;
			}
		}
	}

#if 0
	// creates a physics actor from a prefab
	void physx_impl::create_physx_object(game::selbrush_def_t* sb)
	{
		if (const auto mesh = create_convex_mesh_from_brush(sb); mesh)
		{
			const auto shape = mPhysics->createShape(physx::PxConvexMeshGeometry(mesh), *m_static_collision_material, true);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);

			//game::vec3_t origin_offset;
			//utils::vector::subtract(model->maxs, half_bounds, origin_offset);
			//shape->setLocalPose(physx::PxTransform(origin_offset[0], origin_offset[1], origin_offset[2]));

			game::vec3_t brush_center;
			utils::vector::add(sb->def->mins, sb->def->maxs, brush_center);
			utils::vector::scale(brush_center, 0.5f, brush_center);

			const physx::PxTransform t
			(
				brush_center[0], brush_center[1], brush_center[2]
			);

			physx::PxRigidDynamic* actor = mPhysics->createRigidDynamic(t);
			actor->setActorFlags(physx::PxActorFlag::eVISUALIZATION);

			userdata_s* data = new userdata_s();
			data->brush = sb->def;
			data->old_origin[0] = brush_center[0];
			data->old_origin[1] = brush_center[1];
			data->old_origin[2] = brush_center[2];
			data->old_quat = actor->getGlobalPose().q;
			actor->userData = data;

			actor->attachShape(*shape);
			shape->release();

			physx::PxRigidBodyExt::updateMassAndInertia(*actor, 10.0f);
			mScene->addActor(*actor);

			m_dynamic_prefabs.push_back(actor);
		}
	}
#endif

	// creates a physics actor from a prefab
	void physx_impl::create_physx_object(game::selbrush_def_t* sb)
	{
		// prefab brushes and terrain
		if (sb && sb->owner && sb->owner->prefab && sb->owner->firstActive && sb->owner->firstActive->eclass && sb->owner->firstActive->eclass->classtype & game::ECLASS_PREFAB)
		{
			FOR_ALL_BRUSHES(prefab, sb->owner->prefab->active_brushlist, sb->owner->prefab->active_brushlist_next)
			{
				if (prefab && prefab->def)
				{
					if (exclude_brushes_from_static_collision(prefab))
					{
						continue;
					}

					game::vec3_t prefab_angles = {};
					game::vec4_t prefab_quat = { 0.0f, 0.0f, 0.0f, 1.0f };
					const auto prefab_origin = sb->owner->firstActive->origin;

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

					// 
					// create_static_brush

					std::vector<physx::PxVec3> verts;
					verts.reserve(50);

					game::vec3_t brush_center;
					utils::vector::add(prefab->def->mins, prefab->def->maxs, brush_center);
					utils::vector::scale(brush_center, 0.5f, brush_center);

					for (auto f = 0; f < prefab->def->facecount; f++)
					{
						const auto face = &prefab->def->brush_faces[f];
						for (auto p = 0; face->w && p < face->w->numPoints; p++)
						{
							game::vec3_t tw_point = { face->w->points[p][0], face->w->points[p][1], face->w->points[p][2] };
							utils::vector::subtract(tw_point, brush_center, tw_point);
							//utils::vector::subtract(tw_point, prefab_origin, tw_point);

							const physx::PxVec3 t_point = { tw_point[0], tw_point[1], tw_point[2] };
							//const physx::PxVec3 t_point = { face->w->points[p][0], face->w->points[p][1], face->w->points[p][2] };
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

						auto* actor = mPhysics->createRigidDynamic(physx::PxTransform(0.0f, 0.0f, 0.0f));
						actor->setActorFlags(physx::PxActorFlag::eVISUALIZATION);

						userdata_s* data = new userdata_s();
						data->entity = sb->owner;
						data->def = sb->def;

						utils::vector::copy(prefab_origin, data->initial_ent_origin);
						utils::vector::copy(prefab_angles, data->initial_ent_angles);
						//utils::vector::copy(real_origin, data->actual_ent_origin);

						// do not save local quat?
						//data->initial_quat = physx::PxQuat(prefab_quat[0], prefab_quat[1], prefab_quat[2], prefab_quat[3]);
						//data->last_quat = data->initial_quat;

						//utils::vector::copy(prefab_angles, data->last_angles);
						//data->last_pos = physx::PxVec3(data->initial_origin[0], data->initial_origin[1], data->initial_origin[2]);
						actor->userData = data;

						physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, physx::PxConvexMeshGeometry(convexMesh), *m_static_collision_material);

						//physx::PxTransform local;
						//local.p = physx::PxVec3(0.0f, 0.0f, 0.0f);
						//local.q = physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f);

						//if (prefab_origin || prefab_quat)
						//{
						//	/*game::vec3_t half_offset;
						//	utils::vector::subtract(prefab->def->maxs, prefab->def->mins, half_offset);
						//	utils::vector::scale(half_offset, 0.5f, half_offset);

						//	if (prefab_origin)
						//	{
						//		local.p = physx::PxVec3(-prefab_origin[0], -prefab_origin[1], -prefab_origin[2]);
						//	}*/

						//	if (prefab_origin)
						//	{
						//		local.p = physx::PxVec3(prefab_origin[0], prefab_origin[1], prefab_origin[2]);
						//	}

						//	if (prefab_quat)
						//	{
						//		//local.q = physx::PxQuat(prefab_quat[0], prefab_quat[1], prefab_quat[2], prefab_quat[3]);
						//		//local.q = local.q.getConjugate();
						//	}

						//	//shape->setLocalPose(local);
						//}

						actor->attachShape(*shape);
						shape->release();

						physx::PxRigidBodyExt::updateMassAndInertia(*actor, 10.0f);
						mScene->addActor(*actor);

						

						/*actor->setGlobalPose(physx::PxTransform(
							prefab_origin[0], prefab_origin[1], prefab_origin[2],
							data->initial_quat));*/

						game::vec3_t world_pos;

						utils::vector::copy(real_origin, world_pos);

						actor->setGlobalPose(physx::PxTransform(
							physx::PxVec3(world_pos[0], world_pos[1], world_pos[2]),
							physx::PxQuat(prefab_quat[0], prefab_quat[1], prefab_quat[2], prefab_quat[3])));


						const auto pose = actor->getGlobalPose();
						//data->initial_pos = physx::PxVec3(pose.p.x, pose.p.y, pose.p.z);
						//data->initial_quat = physx::PxQuat(pose.q.x, pose.q.y, pose.q.z, pose.q.w);

						data->initial_transform = physx::PxTransform(pose.p, pose.q);
						data->last_transform = data->initial_transform;

						//data->last_pos = pose.p;
						//data->last_quat = pose.q;


						m_dynamic_prefabs.push_back(actor);

						// cull checks at 0x407AF4, 0x407BEC & 0x408000 result in flickering prefab entities
						// skips culling function if custom_no_cull is set
						data->entity->firstActive->custom_no_cull = true;
					}
				}
			}
		}
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
