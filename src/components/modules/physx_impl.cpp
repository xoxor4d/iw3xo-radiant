#include "std_include.hpp"
//#include <PxPhysicsAPI.h>
//using namespace physx;

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

					mScene->getActiveActors(m_active_body_count);


					constexpr float REDUCE_MSEC_BEGIN_AT_COUNT = 64.0f; // object count needed to start increasing m_phys_msec_step # og: 32
					constexpr float REDUCE_MSEC_RANGE_TO_MAX = 64.0f;   // range - how many objects are needed to hit g_phys_maxMsecStep # og: 18

					const auto step_for_count = (static_cast<float>(m_active_body_count) - REDUCE_MSEC_BEGIN_AT_COUNT) / REDUCE_MSEC_RANGE_TO_MAX;
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
	}

	physx::PxMaterial* physx_impl::create_material(game::PhysPreset* preset)
	{
		return mPhysics->createMaterial(preset->friction, preset->friction, preset->bounce);
	}


	void physx_impl::obj_destroy(int id)
	{
		const auto shape = reinterpret_cast<physx::PxShape*>(id);
		const auto actor = shape->getActor();

		if (actor->userData)
		{
			const auto material = static_cast<physx::PxMaterial*>(actor->userData);
			material->release();
		}

		actor->release();
	}

	void physx_impl::obj_get_interpolated_state(int id, float* out_pos, float* out_quat)
	{
		const auto shape = reinterpret_cast<physx::PxShape*>(id);

		const physx::PxQuat quat = shape->getActor()->getGlobalPose().q;
		const auto pos = shape->getActor()->getGlobalPose().p;
		out_pos[0] = pos.x;
		out_pos[1] = pos.y;
		out_pos[2] = pos.z;

		out_quat[0] = quat.x;
		out_quat[1] = quat.y;
		out_quat[2] = quat.z;
		out_quat[3] = quat.w;
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
		m_active_body_count = 0;

		// init physx
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		if (!mFoundation)
		{
			AssertS("PxCreateFoundation failed!");
		}

		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		mPvd = PxCreatePvd(*mFoundation);
		mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		mToleranceScale.length = 100; // typical length of an object
		mToleranceScale.speed = 981;  // typical speed of an object, gravity*1s is a reasonable choice
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);
		mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

		physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -800.0f); //-9.81f);
		sceneDesc.cpuDispatcher = mDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		mScene = mPhysics->createScene(sceneDesc);

		physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}

		// add a simple ground plane for now
		mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
		physx::PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, physx::PxPlane(0, 0, 1, 50), *mMaterial);
		mScene->addActor(*groundPlane);
	}

	physx_impl::~physx_impl()
	{ }
}
