#pragma once

namespace components
{
	class physx_impl : public component
	{
	public:
		physx::PxDefaultAllocator      mDefaultAllocatorCallback;
		physx::PxDefaultErrorCallback  mDefaultErrorCallback;
		physx::PxDefaultCpuDispatcher* mDispatcher = nullptr;
		physx::PxTolerancesScale       mToleranceScale;

		physx::PxFoundation* mFoundation = nullptr;
		physx::PxPhysics* mPhysics = nullptr;
		physx::PxCooking* mCooking = nullptr;

		physx::PxScene* mScene = nullptr;
		physx::PxMaterial* mMaterial = nullptr;

		physx::PxPvd* mPvd = nullptr;

		bool m_simulation_running;
		bool m_effect_is_using_physics;

		int m_time_last_snapshot;
		int m_time_last_update;
		float m_time_now_lerp_frac;
		int m_phys_msec_step;
		uint32_t m_active_body_count;

		float m_visualization_cullingbox_size = 1000.0f;

		uint32_t m_static_brush_estimated_count;
		uint32_t m_static_brush_count;
		std::vector<physx::PxRigidStatic*> m_static_brushes;

	public:
		physx_impl();
		~physx_impl();
		const char* get_name() override { return "physx_impl"; };

		static void register_dvars();

		static physx_impl* p_this;
		static physx_impl* get() { return p_this; }

		void run_frame(float seconds);
		void frame();
		physx::PxMaterial* create_material(game::PhysPreset* preset);


		void create_static_brush(game::selbrush_def_t* sb, bool is_prefab = false, const game::vec3_t position_offset = nullptr, const float* quat = nullptr);
		static void create_static_collision();

		void obj_destroy(int id);
		void obj_get_interpolated_state(int id, float* out_pos, float* out_quat);

		int create_physx_object(game::XModel* model, const float* world_pos, const float* quat);
		void create_plane();
	};
}
