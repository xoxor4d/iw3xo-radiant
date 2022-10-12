#include "std_include.hpp"

namespace components
{
	mesh_painter* mesh_painter::p_this = nullptr;

	void mesh_painter::set_circle_color(const float r, const float g, const float b, const float a)
	{
		if (!m_circle_verts.empty())
		{
			game::vec4_t	color = { r, g, b, a };
			game::GfxColor	color_packed = {};
			game::Byte4PackPixelColor(color, &color_packed);

			for (auto& p : m_circle_verts)
			{
				p.vert[0].color.packed = color_packed.packed;
				p.vert[1].color.packed = color_packed.packed;
			}
		}
	}

	void mesh_painter::circle_get_coord_for_angle(float* coord, const float* center, const float radius, const float angle, const float* right_vec, const float* up_vec)
	{
		// sin / cos uses radians ...
		const float radians = utils::vector::deg_to_rad(angle + 90);

		coord[0] = (sinf(radians) * radius) * up_vec[0] + center[0];
		coord[1] = (sinf(radians) * radius) * up_vec[1] + center[1];
		coord[2] = (sinf(radians) * radius) * up_vec[2] + center[2];

		coord[0] = (cosf(radians) * radius) * right_vec[0] + coord[0];
		coord[1] = (cosf(radians) * radius) * right_vec[1] + coord[1];
		coord[2] = (cosf(radians) * radius) * right_vec[2] + coord[2];
	}

	void mesh_painter::draw_circle()
	{
		const auto camera_gui = GET_GUI(ggui::camera_dialog);
		const auto camera = &cmainframe::activewnd->m_pCamWnd->camera;

		// get direction into the scene
		float dir_into_scene[3];
		ccamwnd::calculate_ray_direction(camera_gui->rtt_get_cursor_pos_cpoint().x, static_cast<int>(camera_gui->rtt_get_size().y) - camera_gui->rtt_get_cursor_pos_cpoint().y, dir_into_scene);

		// trace along direction
		game::trace_t cam_trace = {};
		game::Test_Ray(camera->origin, dir_into_scene, 0, &cam_trace, 1);

		if (cam_trace.dist <= 5000.0f)
		{
			// calculate hit origin
			utils::vector::scale(dir_into_scene, cam_trace.dist, m_circle_origin);
			utils::vector::add(m_circle_origin, camera->origin, m_circle_origin);

			// slightly offset circle from the hit face
			game::vec3_t height_offset = {};
			utils::vector::scale(cam_trace.face_normal, 4.0f, height_offset);
			utils::vector::add(m_circle_origin, height_offset, m_circle_origin);

			// calculate right and up vectors of the hit face normal to align the circle to the face
			game::vec3_t right = { 0.0f, 1.0f, 0.0f };
			game::vec3_t up = { 0.0f, 0.0f, 1.0f };

			if (utils::vector::length_squared(cam_trace.face_normal) != 0.0f)
			{
				game::PerpendicularVector(cam_trace.face_normal, right);
				utils::vector::cross_product(cam_trace.face_normal, right, up);
			}

			// radius changed
			if (m_circle_radius != m_circle_radius_old)
			{
				m_circle_verts.clear();
				m_circle_verts_angles.clear();
				m_circle_line_count = 0;

				const int num_points = 360 / 4;
				for (auto p = 0; p < num_points; p += 2)
				{
					// cache angles
					// separated from line_s vector so that vertex points are continuous and can be draw with a single render command
					m_circle_verts_angles.emplace_back(angle_s
						{
							360.0f * static_cast<float>((p + 0)) / static_cast<float>(num_points),
							360.0f * static_cast<float>((p + 1)) / static_cast<float>(num_points),
						});

					auto pt = mesh_painter::line_s{};
					mesh_painter::circle_get_coord_for_angle(pt.vert[0].xyz, game::vec3_origin, m_circle_radius, m_circle_verts_angles[m_circle_line_count].angles[0], right, up);
					mesh_painter::circle_get_coord_for_angle(pt.vert[1].xyz, game::vec3_origin, m_circle_radius, m_circle_verts_angles[m_circle_line_count].angles[1], right, up);

					m_circle_verts.emplace_back(pt);
					m_circle_line_count++;
				}

				set_circle_color(1.0f, 1.0f, 0.0f, 1.0f);
				m_circle_radius_old = m_circle_radius;
			}
			else // unchanged radius
			{
				for (auto line = 0; line < m_circle_line_count; line++)
				{
					mesh_painter::circle_get_coord_for_angle(m_circle_verts[line].vert[0].xyz, m_circle_origin, m_circle_radius, m_circle_verts_angles[line].angles[0], right, up);
					mesh_painter::circle_get_coord_for_angle(m_circle_verts[line].vert[1].xyz, m_circle_origin, m_circle_radius, m_circle_verts_angles[line].angles[1], right, up);
				}
			}

			if (m_circle_line_count)
			{
				renderer::R_AddLineCmd(static_cast<std::uint16_t>(m_circle_line_count), 4, 3, m_circle_verts[0].vert);
			}
		}
	}

	void mesh_painter::on_frame()
	{
		// TODO! - add dist param.
		// TODO! - line width param.
		// TODO! - line color param.

		const auto painter = mesh_painter::get();
		const auto camera_gui = GET_GUI(ggui::camera_dialog);

		if (painter->is_active() && camera_gui->rtt_is_hovered())
		{
			painter->draw_circle();
		}
	}


	mesh_painter::mesh_painter()
	{
		mesh_painter::p_this = this;
		m_circle_verts.reserve(256);
		m_circle_verts_angles.reserve(256);
	}

	mesh_painter::~mesh_painter()
	{ }
}
