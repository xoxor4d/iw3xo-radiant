#include "std_include.hpp"
//#include <PxPhysicsAPI.h>
//using namespace physx;

namespace components
{
	physx_impl* physx_impl::p_this = nullptr;

	

	void physx_impl::frame()
	{
		mScene->simulate(1.0f / 60.0f);
		mScene->fetchResults(true);
	}

	void physx_impl::obj_destroy(int id)
	{
		const auto body = reinterpret_cast<physx::PxShape*>(id);

		body->getActor()->release();
		//auto act = body->getActor();

		//body->getActor()->detachShape(*body);
		//act->release();

		//dBodyDestroy(body);
		//Pool_Free(data, &physGlob.userDataPool);
	}

	void physx_impl::obj_get_interpolated_state(int id, float* out_pos, float* out_quat)
	{
		/*float obj_rot[3][3];
		float snap_rot[3][3];
		float pos_quat[4];
		float rot_quat[4];
		float snap_pos[3];
		float obj_pos[3];*/

		//const auto frac = physGlob.worldData[worldIndex].timeNowLerpFrac;
		//Phys_ObjGetSnapshot(static_cast<PhysWorld>(worldIndex), reinterpret_cast<dxBody*>(id), snap_pos, snap_rot);

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
		
		/*Phys_ObjGetSnapshot(static_cast<PhysWorld>(worldIndex), reinterpret_cast<dxBody*>(id), snap_pos, snap_rot);
		Phys_ObjGetPosition(reinterpret_cast<dxBody*>(id), obj_pos, obj_rot);

		Vec3Lerp(snap_pos, obj_pos, frac, outPos);

		fx_system::AxisToQuat(snap_rot, rot_quat);
		fx_system::AxisToQuat(obj_rot, pos_quat);

		QuatLerp(rot_quat, pos_quat, frac, out_quat);
		fx_system::Vec4Normalize(out_quat);*/
	}

	physx_impl::physx_impl()
	{
		physx_impl::p_this = this;

		// declare variables
		


		// init physx
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		if (!mFoundation) throw("PxCreateFoundation failed!");
		mPvd = PxCreatePvd(*mFoundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
		//mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(),true, mPvd);
		mToleranceScale.length = 100;        // typical length of an object
		mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);
		//mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale);

		physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, 0.0f, -800.0f);//-9.81f);
		mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = mDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		mScene = mPhysics->createScene(sceneDesc);

		physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}


		// create simulation
		mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
		physx::PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, physx::PxPlane(0, 0, 1, 50), *mMaterial);
		mScene->addActor(*groundPlane);

		float halfExtent = .5f;
		physx::PxShape* shape = mPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *mMaterial);
		physx::PxU32 size = 30;
		physx::PxTransform t(physx::PxVec3(0));

		for (physx::PxU32 i = 0; i < size; i++) 
		{
			for (physx::PxU32 j = 0; j < size - i; j++) 
			{
				physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
				physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
				body->attachShape(*shape);
				physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
				mScene->addActor(*body);
			}
		}
		shape->release();
	}

	physx_impl::~physx_impl()
	{ }
}
