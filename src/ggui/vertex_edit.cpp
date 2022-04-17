#include "std_include.hpp"
#include <random>

namespace ggui
{
	void vertex_edit_dialog::feature_noise()
	{
		ImGui::title_with_seperator_helpmark("Noise", true, 0, 2.0f, 8.0f,
			"[1] Select a single patch\n[2] Select vertices you want to apply noise to");

		static game::vec3_t noise_scale = { 0.0f, 0.0f, 10.0f };

		const auto patch = game::g_selected_brushes_next()->patch;
		const bool enable_noise =  game::is_single_brush_selected(false)
								&& patch && patch->def
								&& game::g_qeglobals->d_num_move_points > 0;

		ImGui::BeginDisabled(!enable_noise);
		{
			ImGui::DragFloat3("XYZ Noise Scale", noise_scale, 0.1f, -FLT_MAX, FLT_MAX, "%.2f");

			if (ImGui::Button("Add Noise", ImVec2(ImGui::GetItemRectSize().x, ImGui::GetFrameHeight())))
			{
				// generate a random number within a certain range
				std::random_device	rd;
				std::mt19937		gen(rd());

				const std::uniform_real_distribution<float> distr(-100, 100);

				for (auto pt = 0; pt < game::g_qeglobals->d_num_move_points; pt++)
				{
					const auto vert = game::g_qeglobals->d_move_points[pt];

					const float rand1 = distr(gen) * 0.01f;
					const float rand2 = distr(gen) * 0.01f;

					vert->xyz[0] += (rand1 * noise_scale[0]);
					vert->xyz[1] += (rand2 * noise_scale[1]);
					vert->xyz[2] += (rand1 * noise_scale[2]);
				}

				if (patch && patch->def)
				{
					game::Patch_UpdateSelected(patch->def, true);
				}
			}

			ImGui::EndDisabled();
		}
	}

	void vertex_edit_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Vertex Edit Dialog##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		ImGui::Indent(4.0f);
		SPACING(0.0f, 2.0f);

		ImGui::title_with_seperator_helpmark("Vertex Color", false, 0, 2.0f, 8.0f,
			"Select vertices to enable feature");

		static float vertex_edit_color[4] = {};
		const bool enable_vert_color_edit = game::g_qeglobals->d_num_move_points > 0;

		ImGui::BeginDisabled(!enable_vert_color_edit);
		{
			if (ImGui::ColorPicker4("Vertex Color", vertex_edit_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB))
			{
				for (auto pt = 0; pt < game::g_qeglobals->d_num_move_points; pt++)
				{
					const auto vert = game::g_qeglobals->d_move_points[pt];
					vert->vert_color.r = utils::pack_float(vertex_edit_color[0]);
					vert->vert_color.g = utils::pack_float(vertex_edit_color[1]);
					vert->vert_color.b = utils::pack_float(vertex_edit_color[2]);
					vert->vert_color.a = utils::pack_float(vertex_edit_color[3]);
				}

				FOR_ALL_SELECTED_BRUSHES(sb)
				{
					if (sb->patch && sb->patch->def)
					{
						game::Patch_UpdateSelected(sb->patch->def, true);
					}
				}
			}

			ImGui::EndDisabled();
		}

		// add random noise to selected vertices
		vertex_edit_dialog::feature_noise();

		ImGui::End();
	}

	void vertex_edit_dialog::on_vertex_edit_dialog_command()
	{
		if (dvars::gui_use_new_vertedit_dialog->current.enabled)
		{
			GET_GUI(vertex_edit_dialog)->toggle();
			return;
		}

		// original dialog
		auto* v_edit = reinterpret_cast<CWnd*>(0x25D65B0);
		utils::hook::call<void(__fastcall)(CWnd*, int, int)>(0x58EA4F)(v_edit, 0, IsWindowVisible(v_edit->GetWindow()) ? SW_HIDE : SW_SHOW);
	}

	void vertex_edit_dialog::hooks()
	{
		utils::hook::detour(0x42BCD0, vertex_edit_dialog::on_vertex_edit_dialog_command, HK_JUMP);
	}

	void vertex_edit_dialog::register_dvars()
	{
		dvars::gui_use_new_vertedit_dialog = dvars::register_bool(
			/* name		*/ "gui_use_new_vertedit_dialog",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Overwrites default hotkey");
	}

	REGISTER_GUI(vertex_edit_dialog);
}
