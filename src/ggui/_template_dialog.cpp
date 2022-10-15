#include "std_include.hpp"

namespace ggui
{
	bool template_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(200.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 400.0f);

		imgui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		imgui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!imgui::Begin("Template##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			imgui::End();
			return false;
		}

		// #

		SPACING(0.0f, 2.0f);
		imgui::Indent(8.0f);

		if (imgui::Button("Button"))
		{
			// logic
		}

		imgui::End();
		return true;
	}

	void template_dialog::on_init()
	{ }

	void template_dialog::on_open()
	{ }

	void template_dialog::on_close()
	{ }

	REGISTER_GUI(template_dialog);
}
