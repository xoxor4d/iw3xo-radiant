#pragma once

namespace components
{
	class physx_impl : public component
	{
		class behavior_feedback : public PxControllerBehaviorCallback
		{
		public:
			behavior_feedback() = default;
			// Implements PxControllerBehaviorCallback
			virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) override;
			virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) override;
			virtual PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle) override;
		};

#if 0
		class cct_hit_feedback : public PxUserControllerHitReport
		{
		public:

			// Called when current controller hits a shape.
			virtual void onShapeHit(const PxControllerShapeHit& hit) override;

			// Called when current controller hits another controller.
			virtual void onControllerHit(const PxControllersHit& hit) override {};

			// Called when current controller hits a user-defined obstacle
			virtual void onObstacleHit(const PxControllerObstacleHit& hit) override {};
		};
#endif

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
		physx::PxPvd* mPvd = nullptr;

		physx::PxControllerManager* m_manager = nullptr;
		physx_cct_controller* m_cct_controller = nullptr;
		physx_cct_camera* m_cct_camera;
		bool m_character_controller_enabled = false;
		bool m_cctrl_skip_first_mouse_frame = false;

		physx::PxMaterial* m_cct_material = nullptr;

		bool m_fx_sim_running = false;
		bool m_effect_is_using_physics = false;

		uint32_t m_fx_time_last_snapshot = 0;
		uint32_t m_fx_time_last_update = 0;
		uint32_t m_phys_msec_step = 3;
		float m_time_now_lerp_frac;

		uint32_t m_fx_active_actor_count = 0;
		uint32_t m_converted_misc_model_count = 0;

		float m_visualization_cullingbox_size = 1000.0f;

		struct EFFECT_PHYSX_SHAPE
		{
			enum ENUM : int
			{
				CUBE,
				SPHERE,
				CUSTOM
			};

			const char* strings[3] =
			{
				"Cube",
				"Sphere",
				"Custom"
			};

			int index = 0;
			float scalar = 1.0f;
			physx::PxConvexMesh* custom_shape;
		};

		EFFECT_PHYSX_SHAPE m_effect_shape = {};


		physx::PxMaterial* m_static_collision_material = nullptr;

		uint32_t m_static_brush_estimated_count = 0;
		uint32_t m_static_brush_count = 0;
		std::vector<physx::PxRigidStatic*> m_static_brushes;

		uint32_t m_static_terrain_estimated_count = 0;
		uint32_t m_static_terrain_count = 0;
		std::vector<physx::PxRigidStatic*> m_static_terrain;

		std::vector<physx::PxRigidDynamic*> m_dynamic_prefabs;

		bool m_phys_sim_run = false;
		bool m_phys_sim_pause = false;
		bool m_phys_sim_running = false;
		uint32_t m_phys_sim_tick_old = 0;
		uint32_t m_phys_sim_tick = 0;
		uint32_t m_phys_time_last_snapshot = 0;
		uint32_t m_phys_time_last_update = 0;
		uint32_t m_phys_active_actor_count = 0;

		struct userdata_s
		{
			physx::PxMaterial* material = nullptr;
			std::string model_name;
		};

		struct userdata_prefab_s
		{
			physx::PxMaterial* material = nullptr;
			game::entity_s* entity = nullptr;
			game::brush_t_with_custom_def* def = nullptr;
			game::vec3_t initial_ent_origin = {};
			game::vec3_t initial_ent_angles = {};
			physx::PxTransform initial_transform;
			physx::PxTransform last_transform;
		};


	public:
		physx_impl();
		~physx_impl();
		const char* get_name() override { return "physx_impl"; };

		static void register_dvars();

		static physx_impl* p_this;
		static physx_impl* get() { return p_this; }

		void tick_playback();

	private:
		void run_frame(float seconds);
		void draw_debug_visualization();

		bool exclude_brushes_from_static_collision(game::selbrush_def_t* b);
		void create_static_brush(game::selbrush_def_t* sb, bool is_prefab = false, const game::vec3_t position_offset = nullptr, const float* quat = nullptr);
		void create_static_terrain(game::selbrush_def_t* sb, const game::vec3_t position_offset = nullptr, const float* quat = nullptr);

	public:
		void phys_frame();
		void fx_frame();

		physx::PxMaterial* create_material(game::PhysPreset* preset);

		physx::PxConvexMesh* create_convex_mesh_from_brush(game::selbrush_def_t* sb);
		void create_custom_shape_from_selection(game::selbrush_def_t* sb);

		void clear_static_collision();
		static void create_static_collision();

		void convert_phys_to_misc_models();

		void obj_destroy(int id);
		void obj_get_interpolated_state(int id, float* out_pos, float* out_quat);

		void clear_dynamic_prefabs(bool clear_state = true);
		void reset_dynamic_prefabs();

		void create_dynamic_prefab(game::selbrush_def_t* sb);
		int  create_physx_object(game::XModel* model, const float* world_pos, const float* quat, const float* velocity = nullptr, const float* angular_velocity = nullptr);
		void update_static_collision_material();

		static void spawn_character();
	};
}
