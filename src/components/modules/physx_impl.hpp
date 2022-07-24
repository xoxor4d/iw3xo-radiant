#pragma once

namespace components
{
	//class SampleSubmarine : physx::PxSimulationEventCallback
	//{
	//	// Implements PxSimulationEventCallback
	//	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
	//};

	class physx_impl : public component
	{
		class collision_feedback : public physx::PxSimulationEventCallback
		{
		public:
			collision_feedback() = default;

			// Implements PxSimulationEventCallback
			virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
			virtual void onTrigger([[maybe_unused]] physx::PxTriggerPair* pairs, [[maybe_unused]] physx::PxU32 count) override {}
			virtual void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) override {}
			virtual void onWake(physx::PxActor**, physx::PxU32) override {}
			virtual void onSleep(physx::PxActor**, physx::PxU32) override {}
			virtual void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) override {}
		};

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

		uint32_t m_static_terrain_estimated_count;
		uint32_t m_static_terrain_count;
		std::vector<physx::PxRigidStatic*> m_static_terrain;

		struct userdata_s
		{
			physx::PxMaterial* material;
			//fx_system::FxElem* fx_elem;
		};

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
		void create_static_terrain(game::selbrush_def_t* sb, const game::vec3_t position_offset = nullptr, const float* quat = nullptr);
		static void create_static_collision();

		void obj_destroy(int id);
		void obj_get_interpolated_state(int id, float* out_pos, float* out_quat);

		int create_physx_object(game::XModel* model, const float* world_pos, const float* quat, const float* velocity = nullptr, const float* angular_velocity = nullptr);
		void create_plane();
	};
}
