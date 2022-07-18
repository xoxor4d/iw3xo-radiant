#pragma once

namespace components
{
	class physx_impl : public component
	{
	public:
		physx::PxDefaultAllocator      mDefaultAllocatorCallback;
		physx::PxDefaultErrorCallback  mDefaultErrorCallback;
		physx::PxDefaultCpuDispatcher* mDispatcher = NULL;
		physx::PxTolerancesScale       mToleranceScale;

		physx::PxFoundation* mFoundation = NULL;
		physx::PxPhysics* mPhysics = NULL;

		physx::PxScene* mScene = NULL;
		physx::PxMaterial* mMaterial = NULL;

		physx::PxPvd* mPvd = NULL;

	public:
		physx_impl();
		~physx_impl();
		const char* get_name() override { return "physx_impl"; };

		static physx_impl* p_this;
		static physx_impl* get() { return p_this; }

		void frame();
		void obj_destroy(int id);
		void obj_get_interpolated_state(int id, float* out_pos, float* out_quat);

	};
}
