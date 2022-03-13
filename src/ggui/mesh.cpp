#include "std_include.hpp"
#include <random>

namespace ggui::mesh
{
	void noise()
	{
		ImGui::title_with_seperator_helpmark("Noise", false, 0, 2.0f, 8.0f, 
			"[1] Select a single patch\n[2] Select vertices you want to apply noise to");

		static game::vec3_t noise_scale = { 0.0f, 0.0f, 10.0f };

		const auto patch = game::g_selected_brushes_next()->patch;

		const bool enable_noise = 
			   game::is_single_brush_selected(false)
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
					const auto vec3 = game::g_qeglobals->d_move_points[pt];

					const float rand1 = distr(gen) * 0.01f;
					const float rand2 = distr(gen) * 0.01f;

					vec3[0] += (rand1 * noise_scale[0]);
					vec3[1] += (rand2 * noise_scale[1]);
					vec3[2] += (rand1 * noise_scale[2]);
				}

				Patch_UpdateSelected(patch->def, true);
			}

			ImGui::EndDisabled();
		}
	}
			
		
	

	void vertex_edit_dialog(ggui::imgui_context_menu& menu)
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Vertex Edit Dialog##window", &menu.menustate, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		ImGui::Indent(4.0f);
		SPACING(0.0f, 2.0f);

		noise();

		ImGui::End();
	}
}
