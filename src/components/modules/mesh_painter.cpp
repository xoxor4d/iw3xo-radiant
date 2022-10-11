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

	void mesh_painter::circle_get_coord_for_angle(float* coord, const float* center, const float radius, const float angle)
	{
		// sin / cos uses radians ...
		const float radians = utils::vector::deg_to_rad(angle + 90);

		coord[0] = radius * sinf(radians);
		coord[1] = radius * cosf(radians);
		coord[2] = 0.0f;
		utils::vector::add(coord, center, coord);
	}

	void mesh_painter::on_frame()
	{
		const auto painter = mesh_painter::get();


		float dir[3];
		const auto gui = GET_GUI(ggui::camera_dialog);

		ccamwnd::calculate_ray_direction(
			gui->rtt_get_cursor_pos_cpoint().x,
			static_cast<int>(gui->rtt_get_size().y) - gui->rtt_get_cursor_pos_cpoint().y,
			dir);

		game::trace_t cam_trace = {};
		const auto camera = &cmainframe::activewnd->m_pCamWnd->camera;
		game::Test_Ray(camera->origin, dir, 0, &cam_trace, 1);

		utils::vector::scale(dir, cam_trace.dist, painter->m_circle_origin);
		utils::vector::add(painter->m_circle_origin, camera->origin, painter->m_circle_origin);
		painter->m_circle_origin[2] += 1.0f;

		if (painter->m_circle_radius != painter->m_circle_radius_old)
		{
			painter->m_circle_verts.clear();
			painter->m_circle_verts_angles.clear();
			painter->m_circle_line_count = 0;

			const int num_points = 360 / 2;
			for (auto p = 0; p < num_points; p += 2)
			{
				painter->m_circle_verts_angles.emplace_back(angle_s
					{
						360.0f * static_cast<float>((p + 0)) / static_cast<float>(num_points),
						360.0f * static_cast<float>((p + 1)) / static_cast<float>(num_points),
					});

				auto pt = mesh_painter::line_s {};
				mesh_painter::circle_get_coord_for_angle(pt.vert[0].xyz, game::vec3_origin, painter->m_circle_radius, painter->m_circle_verts_angles[painter->m_circle_line_count].angles[0]);
				mesh_painter::circle_get_coord_for_angle(pt.vert[1].xyz, game::vec3_origin, painter->m_circle_radius, painter->m_circle_verts_angles[painter->m_circle_line_count].angles[1]);

				painter->m_circle_verts.emplace_back(pt);
				painter->m_circle_line_count++;
			}

			painter->set_circle_color(1.0f, 1.0f, 0.0f, 1.0f);
			painter->m_circle_radius_old = painter->m_circle_radius;
		}
		else
		{
			for (auto line = 0; line < painter->m_circle_line_count; line++)
			{
				mesh_painter::circle_get_coord_for_angle(painter->m_circle_verts[line].vert[0].xyz, painter->m_circle_origin, painter->m_circle_radius, painter->m_circle_verts_angles[line].angles[0]);
				mesh_painter::circle_get_coord_for_angle(painter->m_circle_verts[line].vert[1].xyz, painter->m_circle_origin, painter->m_circle_radius, painter->m_circle_verts_angles[line].angles[1]);

				//if (utils::vector::length_squared(cam_trace.face_normal))
				//{
				//	fx_system::RotatePointAroundVector(painter->m_circle_verts[line].vert[0].xyz, cam_trace.face_normal, painter->m_circle_verts[line].vert[0].xyz, 180.0f);
				//	fx_system::RotatePointAroundVector(painter->m_circle_verts[line].vert[1].xyz, cam_trace.face_normal, painter->m_circle_verts[line].vert[1].xyz, 180.0f);
				//	//utils::vector::rotate_point(painter->m_circle_verts[line].vert[0].xyz, hit_normal_quat, painter->m_circle_verts[line].vert[0].xyz);
				//	//utils::vector::rotate_point(painter->m_circle_verts[line].vert[1].xyz, hit_normal_quat, painter->m_circle_verts[line].vert[1].xyz);
				//}
			}
		}

		if (painter->m_circle_line_count)
		{
			renderer::R_AddLineCmd(painter->m_circle_line_count, 4, 3, painter->m_circle_verts[0].vert);
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
