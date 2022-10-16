#pragma once

namespace components
{
	class mesh_painter : public component
	{
	public:
		mesh_painter();
		~mesh_painter() override;
		const char* get_name() override { return "mesh_painter"; }

		static mesh_painter* p_this;
		static mesh_painter* get() { return p_this; }

		struct line_s
		{
			game::GfxPointVertex vert[2];
		};

		struct angle_s
		{
			float angles[2];
		};

		bool	m_active = false;
		bool	m_stick_to_first_surface = false;

		bool	m_drag_down = false;
		float	m_drag_threshold = 10.0f;
		int		m_paint_loop_count = 1;

		// #

		struct list_object
		{
			void init_defaults()
			{
				size_range[0] = 0.8f;
				size_range[1] = 1.2f;

				max_align_to_ground_angle[0] = 90.0f;
				max_align_to_ground_angle[1] = 90.0f;

				z_offset = 0.0f;
				paint_weight = 1.0f;

				random_size = true;
				random_rotation = true;
				align_to_ground = true;
				enabled = true;
			}

			list_object(const std::string& name_)
			{
				name = name_;
				init_defaults();
			}

			std::string name;

			game::vec2_t size_range;
			game::vec2_t max_align_to_ground_angle;
			float z_offset;
			float paint_weight;

			bool random_size;
			bool random_rotation;
			bool align_to_ground;
			bool enabled;
		};

		std::vector<list_object>  m_objects;

		// returns true if object is already part of the list
		bool list_object_exists(std::string new_object_name)
		{
			for (const auto& obj : m_objects)
			{
				if (obj.name == new_object_name)
				{
					return true;
				}
			}

			return false;
		}


		// #

		std::vector<line_s>  m_circle_verts;
		std::vector<angle_s> m_circle_verts_angles;

		int		m_circle_line_count = 0;
		float	m_circle_radius = 64.0f;
		float	m_circle_radius_old = 0.0f;

		game::vec3_t	m_circle_origin = {};
		game::vec3_t	m_circle_normal = {};

		game::vec3_t	m_circle_vright = { 0.0f, 1.0f, 0.0f };
		game::vec3_t	m_circle_vup = { 0.0f, 0.0f, 1.0f };

		game::GfxPointVertex random_point = {};
		line_s random_point_trace_to_face = {};
		line_s random_point_trace_to_upper = {};

		// #

		[[nodiscard]] bool is_active() const
		{
			return m_active;
		}

		void toggle(bool force_state = false, bool state = false)
		{
			if (force_state)
			{
				m_active = state;
				return;
			}

			m_active = !m_active;
		}

		void		write_list();
		void		load_list();

		void		set_circle_color(const float r, const float g, const float b, const float a);
		void		random_point_on_circle(float& x, float& y, float& z);
		void		draw_circle();

		void		circle_get_coord_for_angle(float* coord, const float angle);

		const list_object* get_random_weighted_object(const float rand);

		void paint_frame();
		static void on_frame();
		static bool block_lbutton_logic();
	};
}
