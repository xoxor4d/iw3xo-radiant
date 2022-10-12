#include "std_include.hpp"

namespace ggui
{
	bool mesh_painter_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(200.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 400.0f);

		imgui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		imgui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		const auto painter = components::mesh_painter::get();

		if (!imgui::Begin("Mesh Painter##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			imgui::End();
			return false;
		}

		// #

		SPACING(0.0f, 2.0f);

		if (imgui::Button("Toggle Painter"))
		{
			painter->toggle();
		}

		imgui::End();
		
		return true;
	}

	void mesh_painter_dialog::on_init()
	{ }

	void mesh_painter_dialog::on_open()
	{ }

	void mesh_painter_dialog::on_close()
	{ }

	REGISTER_GUI(mesh_painter_dialog);
}
