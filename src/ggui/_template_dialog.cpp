#include "std_include.hpp"

namespace ggui
{
	void template_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(400.0f, 390.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);


		if (ImGui::Begin("Template##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);

			auto test = this->rtt_get_texture();

			ImGui::Text("Test");
			ImGui::End();
		}
	}

	REGISTER_GUI(template_dialog);
}
