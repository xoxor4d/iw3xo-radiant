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
			//float angles[2];
		};

		struct angle_s
		{
			float angles[2];
		};

		std::vector<line_s>  m_circle_verts;
		std::vector<angle_s> m_circle_verts_angles;
		int		m_circle_line_count = 0;
		float	m_circle_radius = 64.0f;
		float	m_circle_radius_old = 0.0f;

		game::vec3_t m_circle_origin = {};
		game::vec3_t m_circle_normal = {};

		void		set_circle_color(const float r, const float g, const float b, const float a);

		static void circle_get_coord_for_angle(float* coord, const float* center, const float radius, const float angle);
		static void on_frame();
	};
}
