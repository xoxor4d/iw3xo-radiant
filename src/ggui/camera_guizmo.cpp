#include "std_include.hpp"

namespace ggui::camera_guizmo
{
	bool overwrite_activation = false;
	bool guizmo_visible = false;

	void get_matrices_for_guizmo(game::GfxMatrix* view, game::GfxMatrix* projection)
	{
		float axis[9];
		axis[0] = cmainframe::activewnd->m_pCamWnd->camera.vpn[0];
		axis[1] = cmainframe::activewnd->m_pCamWnd->camera.vpn[1];
		axis[2] = cmainframe::activewnd->m_pCamWnd->camera.vpn[2];
		axis[3] = -cmainframe::activewnd->m_pCamWnd->camera.vright[0];
		axis[4] = -cmainframe::activewnd->m_pCamWnd->camera.vright[1];
		axis[5] = -cmainframe::activewnd->m_pCamWnd->camera.vright[2];
		axis[6] = cmainframe::activewnd->m_pCamWnd->camera.vup[0];
		axis[7] = cmainframe::activewnd->m_pCamWnd->camera.vup[1];
		axis[8] = cmainframe::activewnd->m_pCamWnd->camera.vup[2];

		//MatrixForViewer
		utils::hook::call<void(__cdecl)(float(*mtx)[4], const float* origin, const float*)>(0x4A7A70)(view->m, cmainframe::activewnd->m_pCamWnd->camera.origin, axis);

		const float tanHalfFovY = tan(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f;
		const float tanHalfFovX = tanHalfFovY * ((float)cmainframe::activewnd->m_pCamWnd->camera.width / (float)cmainframe::activewnd->m_pCamWnd->camera.height);

		// R_SetupProjection ...... changing znear from "0.0099999998" (stock) to "10.0" fixed the jiggle issue
		utils::hook::call<void(__cdecl)(game::GfxMatrix*, float halfx, float halfy, float znear)>(0x4A78E0)(projection, tanHalfFovX, tanHalfFovY, 10.0f);

		// ----------

		// LH to RH conversion (not sure what I've done here .. but it works :e)

		game::GfxMatrix inv_view_mtx = {
			-1.f, 0.f, 0.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f };

		utils::mtx4x4_mul(view, &inv_view_mtx, view);
		utils::mtx4x4_mul(view, view, &inv_view_mtx);

		view->m[0][0] *= -1;
		view->m[0][1] *= -1;
		view->m[0][2] *= -1;
		view->m[1][0] *= -1;
		view->m[1][1] *= -1;
		view->m[1][2] *= -1;

		// ----------

		game::GfxMatrix inv_proj_mtx = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			0.f, 0.f, 0.f, 1.f };

		utils::mtx4x4_mul(projection, &inv_proj_mtx, projection);
		utils::mtx4x4_mul(projection, projection, &inv_proj_mtx);
	}

	void get_selection_center_movepoints(float* center_point)
	{
		if (game::g_qeglobals->d_num_move_points)
		{
			// get bounds of selection
			game::vec3_t mins, maxs;
			utils::vector::set_vec3(mins, 131072.0f);
			utils::vector::set_vec3(maxs, -131072.0f);

			// get bounds from windings
			for (auto p = 0; p < game::g_qeglobals->d_num_move_points; p++)
			{
				for (auto v = 0; v < 3; v++)
				{
					// mins :: find the closest point on each axis
					if (mins[v] > game::g_qeglobals->d_move_points[p]->xyz[v])
						mins[v] = game::g_qeglobals->d_move_points[p]->xyz[v];

					// maxs :: find the furthest point on each axis
					if (maxs[v] < game::g_qeglobals->d_move_points[p]->xyz[v])
						maxs[v] = game::g_qeglobals->d_move_points[p]->xyz[v];
				}
			}

			utils::vector::add(mins, maxs, center_point);
			utils::vector::scale(center_point, 0.5f, center_point);
		}
	}

	void get_selection_center(float* center_point)
	{
		// get bounds of selection
		game::vec3_t mins, maxs;
		utils::vector::set_vec3(mins, 131072.0f);
		utils::vector::set_vec3(maxs, -131072.0f);

		for (auto   sb = game::g_selected_brushes_next();
			(DWORD*)sb != game::currSelectedBrushes;
					sb = sb->next)
		{
			if (sb->def)
			{
				utils::vector::clamp_vec3(sb->def->mins, sb->def->maxs, mins, maxs);
			}
		}

		utils::vector::add(mins, maxs, center_point);
		utils::vector::scale(center_point, 0.5f, center_point);
	}

	bool added_undo = false;
	
	void guizmo(const ImVec2& camera_size, bool& accepted_dragdrop)
	{
		if (dvars::guizmo_enable->current.enabled)
		{
			const auto camerawnd = GET_GUI(ggui::camera_dialog);

			// do not activate guizmo till initial left click is released
			// fixes unwanted transforms on multi selection via shift (if mouse is over guizmo)
			static bool guizmo_needs_activation = true;
			static bool guizmo_capture_active = false;

			guizmo_visible = false;

			if (const auto b = game::g_selected_brushes()->def; b)
			{
				if(overwrite_activation)
				{
					guizmo_needs_activation = false;
					overwrite_activation = false;
				}

				if (guizmo_needs_activation)
				{
					camerawnd->rtt_set_lmb_capturing(true);

					if (!guizmo_capture_active)
					{
						if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
						{
							guizmo_capture_active = true;
						}
					}
					else
					{
						guizmo_needs_activation = ImGui::IsMouseDown(ImGuiMouseButton_Left);
					}
				}
			}
			else
			{
				// no brush selected
				if (added_undo)
				{
					game::Undo_EndBrushList_Selected();
					game::Undo_End();
					added_undo = false;
				}

				// always track
				camerawnd->rtt_set_lmb_capturing(true);

				guizmo_needs_activation = true;
				guizmo_capture_active = false;
			}

			if (guizmo_needs_activation && !accepted_dragdrop)
			{
				return;
			}

			if (accepted_dragdrop)
			{
				guizmo_needs_activation = false;
				accepted_dragdrop = false;
			}

			game::GfxMatrix view = {};
			game::GfxMatrix projection = {};

			// setup the view and projection matrices
			get_matrices_for_guizmo(&view, &projection);

			// imguizmo settings
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(

				camerawnd->rtt_get_position().x, 
				camerawnd->rtt_get_position().y,
				camera_size.x, 
				camera_size.y);

			const auto guizmo_mode = game::g_bRotateMode ? ImGuizmo::OPERATION::ROTATE : ImGuizmo::OPERATION::TRANSLATE;

			float mtx_scale[3] = { 1.0f, 1.0f, 1.0f };
			float angles[3] = { 0.0f, 0.0f, 0.0f };
			float snap[3] = { 0.0f, 0.0f, 0.0f };

			if (dvars::guizmo_snapping->current.enabled)
			{
				// default case
				float snap_size = xywnd::GRID_SIZES[game::g_qeglobals->d_gridsize];

				// snap to 1 2 5 10 15 30 60 90 when rotation mode is enabled 
				if (guizmo_mode == ImGuizmo::OPERATION::ROTATE)
				{
					switch (game::g_qeglobals->d_gridsize)
					{
					case 3: snap_size = 5.0f; break;
					case 4: snap_size = 10.0f; break;
					case 5: snap_size = 15.0f; break;
					case 6: snap_size = 30.0f; break;
					case 7: snap_size = 60.0f; break;
					case 9: snap_size = 90.0f; break;
					}
				}

				utils::vector::set_vec3(snap, snap_size);
			}

			//if (dvars::guizmo_brush_mode->current.enabled)
			{
				if (const auto b = game::g_selected_brushes()->def; b)
				{
					// pass mouse input to ImGui
					if (ImGuizmo::IsOver())
					{
						camerawnd->rtt_set_lmb_capturing(true);
					}

					// guizmo position
					game::vec3_t selection_center = {};

					
					//const auto num_move_points = game::g_qeglobals->d_num_move_points;
					const auto selection_mode = game::g_qeglobals->d_select_mode;
					bool in_vertex_mode = false;

					// check for vertex edit mode
					if(selection_mode == game::sel_curvepoint || selection_mode == game::sel_area)
					{
						// this can result in the guizmo not being disabled correctly, thus using IsUsing() can return the wrong value
						/*if(!num_move_points)
						{
							return;
						}*/

						in_vertex_mode = true;
					}

					if (guizmo_mode == ImGuizmo::OPERATION::ROTATE)
					{
						// use radiants rotate origin
						utils::vector::copy(game::g_vRotateOrigin, selection_center);
					}
					else
					{
						if (in_vertex_mode)
						{
							get_selection_center_movepoints(selection_center);
						}
						else
						{
							// calculate center of brush/es
							get_selection_center(selection_center);
						}
					}

					float tmp_matrix[16];
					float delta_matrix[16];

					// build guizmo matrix from components
					ImGuizmo::RecomposeMatrixFromComponents(selection_center, angles, mtx_scale, tmp_matrix);

//#define BOUNDS_TEST					
#ifdef BOUNDS_TEST
					// *
					// bounds

					game::vec3_t local_mins;
					game::vec3_t local_maxs;

					utils::vector::subtract(b->mins, selection_center, local_mins);
					utils::vector::subtract(b->maxs, selection_center, local_maxs);

					float bounds[] =
					{
						local_mins[0], local_mins[1], local_mins[2],
						local_maxs[0], local_maxs[1], local_maxs[2]
					};

					guizmo_mode = ImGuizmo::OPERATION::BOUNDS;

					// draw guizmo / manipulate
					if (ImGuizmo::Manipulate(&view.m[0][0], &projection.m[0][0], guizmo_mode, ImGuizmo::MODE::WORLD, tmp_matrix, delta_matrix, snap, bounds))
#else

					guizmo_visible = true;
					
					if (ImGuizmo::Manipulate(&view.m[0][0], &projection.m[0][0], guizmo_mode, ImGuizmo::MODE::WORLD, tmp_matrix, delta_matrix, snap))
#endif
					{
						/*if (guizmo_mode == ImGuizmo::OPERATION::BOUNDS)
						{
							utils::hook::call<void(__cdecl)(game::brush_t_with_custom_def*, float*, float*)>(0x438760)(b, bounds, &bounds[3]);
						}*/

						if (!added_undo)
						{
							game::Undo_ClearRedo();
							game::Undo_GeneralStart("move");
							game::Undo_AddBrushList_Selected();

							added_undo = true;
						}

						if (ImGuizmo::IsOver())
						{
							float delta_origin[3], delta_angles[3];

							// seperate manupulated matrix into components
							ImGuizmo::DecomposeMatrixToComponents(delta_matrix, delta_origin, delta_angles, mtx_scale);

							if (guizmo_mode == ImGuizmo::OPERATION::ROTATE)
							{
#if 0
								// original way of getting the rotation matrix (here to stay to check against the one below)
								float rotate_axis_org[4][4] = {};
								rotate_axis_org[0][0] = game::g_vRotateOrigin[0];
								rotate_axis_org[0][1] = game::g_vRotateOrigin[1];
								rotate_axis_org[0][2] = game::g_vRotateOrigin[2];
								game::Select_RotateAxis(cmainframe::activewnd->m_pXYWnd->m_nViewType, delta_angles[cmainframe::activewnd->m_pXYWnd->m_nViewType], &rotate_axis_org[0][0]);
#endif
								float rotate_axis[4][4];

								// inverse angles
								utils::vector::inverse(delta_angles);

								// build a rotation matrix
								ImGuizmo::RecomposeMatrixFromComponents(game::g_vRotateOrigin, delta_angles, mtx_scale, &rotate_axis[0][0]);

								float rotate_axis_for_radiant[4][3] = {};

								// bring it into the format radiant expects
								rotate_axis_for_radiant[0][0] = rotate_axis[3][0];
								rotate_axis_for_radiant[0][1] = rotate_axis[3][1];
								rotate_axis_for_radiant[0][2] = rotate_axis[3][2];

								rotate_axis_for_radiant[1][0] = rotate_axis[0][0]; // [1][0]
								rotate_axis_for_radiant[1][1] = rotate_axis[1][0];
								rotate_axis_for_radiant[1][2] = rotate_axis[2][0];

								rotate_axis_for_radiant[2][0] = rotate_axis[0][1];
								rotate_axis_for_radiant[2][1] = rotate_axis[1][1];
								rotate_axis_for_radiant[2][2] = rotate_axis[2][1];

								rotate_axis_for_radiant[3][0] = rotate_axis[0][2];
								rotate_axis_for_radiant[3][1] = rotate_axis[1][2];
								rotate_axis_for_radiant[3][2] = rotate_axis[2][2];

								// apply rotation to all selected brushes
								FOR_ALL_SELECTED_BRUSHES(sb)
								{
									if (const auto brush = sb->def; brush)
									{
										// degree is handled like a boolean, 0.0 will not rotate fixed size brushes / entities
										//game::Select_ApplyMatrix(&rotate_axis_for_radiant[0][0], sb, false, 1.0f, false);

										game::Select_RotateFixedSize(sb->owner, sb->def, rotate_axis_for_radiant);

										components::remote_net::cmd_send_brush_select_deselect(true);
									}
								}
							}

							else if (guizmo_mode == ImGuizmo::OPERATION::TRANSLATE)
							{
								if (in_vertex_mode)
								{
									for (auto pt = 0; pt < game::g_qeglobals->d_num_move_points; pt++)
									{
										const auto vert = game::g_qeglobals->d_move_points[pt];

										vert->xyz[0] += delta_origin[0];
										vert->xyz[1] += delta_origin[1];
										vert->xyz[2] += delta_origin[2];
									}

									FOR_ALL_SELECTED_BRUSHES(sb)
									{
										if (sb->patch && sb->patch->def)
										{
											game::Patch_UpdateSelected(sb->patch->def, true);
										}
									}
								}
								else
								{
									// move all selected brushes using the delta
									FOR_ALL_SELECTED_BRUSHES(sb)
									{
										if (const auto	brush = sb->def; 
														brush)
										{
											const float max_move = 200.0f;

											if(delta_origin[0] >= max_move) {
												delta_origin[0] = max_move;
											}

											if (delta_origin[1] >= max_move) {
												delta_origin[1] = max_move;
											}

											if (delta_origin[2] >= max_move) {
												delta_origin[2] = max_move;
											}

											if (delta_origin[0] <= -max_move) {
												delta_origin[0] = -max_move;
											}

											if (delta_origin[1] <= -max_move) {
												delta_origin[1] = -max_move;
											}

											if (delta_origin[2] <= -max_move) {
												delta_origin[2] = -max_move;
											}

											game::Brush_Move(delta_origin, brush, true);
											components::remote_net::cmd_send_brush_select_deselect(true);
										}
									}
								}
							}
						}
					} // no manipulation
					else if(added_undo)
					{
						if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
						{
							game::Undo_EndBrushList_Selected();
							game::Undo_End();
							added_undo = false;
						}
					}
				}
			}
		}
	}
}