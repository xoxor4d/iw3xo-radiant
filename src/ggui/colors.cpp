#include "std_include.hpp"

namespace ggui
{
	bool gui_colors_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		const float inner_item_spacing = 4.0f;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(inner_item_spacing, 0.0f));

		if (!ImGui::Begin("Colors##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return false;
		}

		ImGui::Indent(4.0f);
		SPACING(0.0f, 2.0f);

		if (ImGui::Button("Set Default Radiant Colors", ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight())))
		{
			radiantapp::set_default_savedinfo_colors();
		}

		SPACING(0.0f, 2.0f);

		ImGui::ColorEdit4("Camera Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_CAMERABACK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Texture Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_TEXTUREBACK], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Grid Background", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBACK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Minor", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMINOR], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Major", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDMAJOR], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Block", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDBLOCK], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Grid Text", game::g_qeglobals->d_savedinfo.colors[game::COLOR_GRIDTEXT], ImGuiColorEditFlags_Float);
		//ImGui::ColorEdit4("Grid Entity Classname", game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITYUNK], ImGuiColorEditFlags_Float); // this holds the latest color used in the windows color dialog (K)
		ImGui::ColorEdit4("Grid Entity Classname", game::g_qeglobals->d_savedinfo.colors[game::COLOR_VIEWNAME], ImGuiColorEditFlags_Float);

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

		ImGui::ColorEdit4("Func Group", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_GROUP], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Func Cull Group", game::g_qeglobals->d_savedinfo.colors[game::COLOR_FUNC_CULLGROUP], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Entity", game::g_qeglobals->d_savedinfo.colors[game::COLOR_ENTITY], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Togglesurfs", game::g_qeglobals->d_savedinfo.colors[game::COLOR_DRAW_TOGGLESUFS], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Unkown##1", game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN2], ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Unkown##2", game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN3], ImGuiColorEditFlags_Float);

		SEPERATORV(0.0f);

		ImGui::ColorEdit4("Gui Menubar Bg##1", dvars::gui_menubar_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Toolbar Bg##2", dvars::gui_toolbar_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Bg##3", dvars::gui_window_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Child Bg##4", dvars::gui_window_child_bg_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Border##5", dvars::gui_border_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Docking Preview##6", dvars::gui_dockingpreview_color->current.vector, ImGuiColorEditFlags_Float);

		ImGui::ColorEdit4("Gui Toolbar Button", dvars::gui_toolbar_button_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Toolbar Button Hovered", dvars::gui_toolbar_button_hovered_color->current.vector, ImGuiColorEditFlags_Float);
		ImGui::ColorEdit4("Gui Toolbar Button Active", dvars::gui_toolbar_button_active_color->current.vector, ImGuiColorEditFlags_Float);


#if 0	// disabled until camera toolbar is fixed
		SEPERATORV(0.0f);

		static float group_width = 80.0f;
		ImGui::SetCursorPosX(clamped_min_widget_width * 0.5f - (group_width * 0.5f));

		ImGui::Checkbox("Gui RTT Border", &dvars::gui_rtt_padding_enabled->current.enabled);
		group_width = ImGui::GetItemRectSize().x;

		SPACING(0.0f, 4.0f);

		ImGui::SliderInt("Gui RTT Bordersize", &dvars::gui_rtt_padding_size->current.integer, dvars::gui_rtt_padding_size->domain.integer.min, dvars::gui_rtt_padding_size->domain.integer.max);
		ImGui::ColorEdit4("Gui RTT Border", dvars::gui_rtt_padding_color->current.vector, ImGuiColorEditFlags_Float);
#endif

		ImGui::PopStyleVar();
		ImGui::End();

		return true;
	}

	REGISTER_GUI(gui_colors_dialog);
}