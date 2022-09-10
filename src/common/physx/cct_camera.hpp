#pragma once
class	ccamwnd;
class	physx_cct_controller;

class physx_cct_camera
{
	public:
		struct cmd_s
		{
			float forwardMove;
			float rightMove;
			float upMove;
		};

		struct trace_t
		{
			float			fraction;
			PxVec3			endpos;
			PxLocationHit	hit = {};
			bool			allsolid;
		};

		enum TRACE_DIR_
		{
			TRACE_DIR_UP,
			TRACE_DIR_DOWN
		};

		physx_cct_camera();

		void			set_controlled(physx_cct_controller* controlled);
		static void		reset_enter_controller_parms();

		virtual	void	update(PxReal dtime);

		enum GROUND_TYPE_
		{
			GROUND_TYPE_NONE,
			GROUND_TYPE_GROUND,
			GROUND_TYPE_SLOPE,
		};

		PX_FORCE_INLINE physx_cct_controller*		get_controller()						{ return m_ccts; }
		PX_FORCE_INLINE void						enable_cct(bool bState)					{ m_cct_enabled = bState; }
		PX_FORCE_INLINE bool						get_cct_state()							{ return m_cct_enabled; }

		private:
						void	key_inputs();
						void	mouse_input();

						void	trace_box(trace_t& trace, const PxVec3& from, const PxVec3& to);
						void	trace_box(trace_t& trace, const PxVec3& from, const PxReal& dist, TRACE_DIR_ dir);
						void	ground_trace();

						bool	is_sprinting();
						bool	jump_check();

						void	clip_velocity(const PxVec3& normal, const PxVec3& in, PxVec3& out);
						void	project_velocity(const PxVec3& normal, float* v_in, float* v_out);

						void	accelerate(const PxVec3& wishdir, float wishspeed, float accel);
						void	pm_friction();
						void	air_move();
						void	walk_move();

						bool	slide_move(bool gravity);
						void	stepslide_move(bool gravity);

						PxObstacleContext*			m_obstacle_context;		// User-defined additional obstacles
						const PxFilterData*			m_filter_data;			// User-defined filter data for 'move' function
						PxQueryFilterCallback*		m_filter_callback;		// User-defined filter data for 'move' function
						PxControllerFilterCallback*	m_cct_filter_callback;	// User-defined filter data for 'move' function

		public:
						physx_cct_controller*		m_ccts;
						bool						m_cct_enabled;

						float						m_msec;
						bool						m_wants_sprint;
						bool						m_wants_jump;
						bool						m_is_jumping; // pm_flags PMF_JUMPING
						float						m_jump_origin_z;
						int							m_jump_timer;
						bool						m_jump_held;
						bool						m_is_flying;
						GROUND_TYPE_				m_ground_type;
						bool						m_on_ground;
						bool						m_almost_ground_plane;
						bool						m_walking;

						PxReal						m_gravity;
						PxReal						m_friction;
						PxReal						m_jump_velocity;

						PxVec3						m_player_velocity;
						PxReal						m_player_speed;
						cmd_s						m_cmd;

						PxLocationHit				m_groundtrace;
};
