#include "std_include.hpp"

namespace ggui::colors
{
	void menu(ggui::imgui_context_menu& menu)
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		
		ImGui::Begin("Colors##xywnd", &menu.menustate, ImGuiWindowFlags_NoCollapse);

		ImGui::ColorEdit4("Camera Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_CAMERABACK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Texture Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_TEXTUREBACK], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Grid Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBACK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Minor", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMINOR], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Major", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMAJOR], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Block", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBLOCK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Text", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDTEXT], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Unselected Brushes", game::g_qeglobals->d_savedinfo.colors[game::COLOR_BRUSHES], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Selected Brushes", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELBRUSHES], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Selected Brushes Cam", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELBRUSHES_CAMERA], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Selected Face Cam", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SELFACE_CAMERA], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Detail", game::g_qeglobals->d_savedinfo.colors[game::COLOR_DETAIL_BRUSH], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Non-Colliding", game::g_qeglobals->d_savedinfo.colors[game::COLOR_NONCOLLIDING], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Weapon Clip", game::g_qeglobals->d_savedinfo.colors[game::COLOR_WEAPON_CLIP], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Size Info", game::g_qeglobals->d_savedinfo.colors[game::COLOR_SIZE_INFO], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Clipper", game::g_qeglobals->d_savedinfo.colors[game::COLOR_CLIPPER], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Model", game::g_qeglobals->d_savedinfo.colors[game::COLOR_MODEL], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Wireframe", game::g_qeglobals->d_savedinfo.colors[game::COLOR_WIREFRAME], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Frozen Layers", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FROZEN_LAYERS], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Viewname", game::g_qeglobals->d_savedinfo.colors[game::COLOR_VIEWNAME], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Func Group", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_GROUP], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Func Cull Group", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_CULLGROUP], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Entity", game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITY], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Entity Unkown", game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITYUNK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Togglesurfs", game::g_qeglobals->d_savedinfo.colors[game::COLOR_DRAW_TOGGLESUFS], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Unkown", game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN2], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Unkown", game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN3], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Gui Menubar Bg", dvars::gui_menubar_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Docked Bg", dvars::gui_dockedwindow_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Undocked Bg", dvars::gui_window_bg_color->current.vector, ImGuiColorEditFlags_Float);

		ImGui::End();
	}
}