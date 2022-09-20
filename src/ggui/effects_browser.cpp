#include "std_include.hpp"

namespace ggui
{
	bool effects_browser::gui()
	{
		const auto io = ImGui::GetIO();

		const auto MIN_WINDOW_SIZE = ImVec2(500.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(550.0f, 600.0f);

		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		if (!ImGui::Begin("Effects Browser##rtt", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}

		ImGui::BeginChild("##pref_child", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			const auto child_size = ImGui::GetContentRegionAvail();
			SetWindowPos(components::renderer::get_window(components::renderer::CFXWND)->hwnd, HWND_BOTTOM, 0, 0, static_cast<int>(child_size.x), static_cast<int>(child_size.y), SWP_NOZORDER);
			this->rtt_set_size(child_size);

			if (this->rtt_get_texture())
			{
				ImGui::Image(this->rtt_get_texture(), child_size);
				this->m_scene_texture_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				if (this->m_scene_texture_hovered)
				{
					// zoom
					if (io.MouseWheel != 0.0f)
					{
						this->m_camera_distance -= (io.MouseWheel * 10.0f);
					}
				}
			}
		}

		ImGui::EndChild();
		ImGui::End();

		return true;
	}

	void effects_browser::on_open()
	{ }

	void effects_browser::on_close()
	{ }

	REGISTER_GUI(effects_browser);
}
