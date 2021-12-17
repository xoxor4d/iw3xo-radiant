#include "std_include.hpp"

namespace ggui::fakesun_settings
{
	float	sun_dir[3] = { 210.0f, 60.0f, 0.0f };
	float	sun_diffuse[3] = { 2.45f, 2.1f, 1.8f };
	float	sun_specular[4] = { 3.0f, 3.2f, 3.0f, 0.0f };
	float	material_specular[4] = { 0.35f, 0.45f, 1.0f, 4.0f };
	float	ambient[4] = { 0.3f, 0.3f, 0.3f, 0.35f };
	
	void menu(ggui::imgui_context_menu& menu)
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 220.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 460.0f);
		
		auto initial_window_pos = ggui::get_initial_window_pos();
		
		if(auto camerawnd = ggui::get_rtt_camerawnd(); 
				camerawnd)
		{
			initial_window_pos = ImVec2(camerawnd->scene_pos_imgui.x + camerawnd->scene_size_imgui.x - INITIAL_WINDOW_SIZE.x - 48.0f, camerawnd->scene_pos_imgui.y + 32.0f);
		}

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(initial_window_pos, ImGuiCond_FirstUseEver);

		if(ImGui::Begin("Fakesun Settings##window", &menu.menustate, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			SPACING(0.0f, 2.0f);
			ImGui::Indent(8.0f);
			
			ImGui::Checkbox("Use Worldspawn Settings", &dvars::r_fakesun_use_worldspawn->current.enabled);
				TT("Uses some of the default values below if a required worldspawn key can not be found");

			ImGui::DragFloat3("Sun Dir", sun_dir, 0.1f);
			ImGui::ColorEdit3("Sun Diffuse", sun_diffuse, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit4("Sun Specular", sun_specular, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			
			ImGui::ColorEdit4("EnvMapParms", material_specular, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			TT(	" controls specular highlights/reflections (lightSpotDir constant) \n" \
				" x: Amount of reflection looking directly at a surface  \n" \
				" y: Amount of reflection at full glancing angle \n" \
				" z: How quickly spec-strength goes from min to max as the angle approaches glancing \n" \
				" w: Sun Intensity ");
			
			ImGui::ColorEdit4("Ambient", ambient, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			TT(	" general ambient color (lightSpotFactors constant) \n" \
				" rgb: base ambient color \n" \
				" alpha: sunlight strength ");

			// -----------------
			ImGui::title_with_seperator("Fog Settings");

			ImGui::Checkbox("Enable Fog", &dvars::r_fakesun_fog_enabled->current.enabled);
			ImGui::DragFloat("Fog Start", &dvars::r_fakesun_fog_start->current.value);
			ImGui::DragFloat("Fog Half Dist", &dvars::r_fakesun_fog_half->current.value);
			ImGui::ColorEdit4("Fog Color", dvars::r_fakesun_fog_color->current.vector, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

			SPACING(0.0f, 2.0f);
			
			ImGui::End();
		}
	}
}