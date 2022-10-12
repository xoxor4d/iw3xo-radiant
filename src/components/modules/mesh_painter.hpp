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

		std::vector<line_s>  m_circle_verts;
		std::vector<angle_s> m_circle_verts_angles;

		int		m_circle_line_count = 0;
		float	m_circle_radius = 64.0f;
		float	m_circle_radius_old = 0.0f;

		game::vec3_t	m_circle_origin = {};
		game::vec3_t	m_circle_normal = {};

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

		void		set_circle_color(const float r, const float g, const float b, const float a);
		void		draw_circle();

		static void circle_get_coord_for_angle(float* coord, const float* center, const float radius, const float angle, const float* right_vec, const float* up_vec);
		static void on_frame();
	};
}
