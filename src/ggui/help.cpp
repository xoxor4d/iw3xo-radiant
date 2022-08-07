#include "std_include.hpp"

#define BULLET_LINK(text, link)		\
	ImGui::Bullet();				\
	ImGui::TextURL(text, (link));

namespace ggui
{
	bool help_dialog::gui()
	{
		ImGui::SetNextWindowSize(ImVec2(400.0f, 490.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Help##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
		{
			SPACING(0.0f, 8.0f);

			if(ImGui::TreeNodeEx("General", ImGuiTreeNodeFlags_DefaultOpen))
			{
				BULLET_LINK("Features / Buttons explained", "https://xoxor4d.github.io/tutorials/iw3xradiant-features-and-buttons/");
				BULLET_LINK("Prefab from selection and stamping", "https://xoxor4d.github.io/tutorials/iw3xradiant-prefab/");
				BULLET_LINK("Live Link", "https://xoxor4d.github.io/tutorials/iw3xradiant-livelink/");
				BULLET_LINK("Extend/Extrude Brushes to nearby faces", "https://xoxor4d.github.io/tutorials/iw3xradiant-brush-face-extending/");

				ImGui::TreePop();
			}

			SPACING(0.0f, 0.0f);

			if (ImGui::TreeNodeEx("d3dbsp", ImGuiTreeNodeFlags_DefaultOpen))
			{
				BULLET_LINK("Loading and Compiling", "https://xoxor4d.github.io/tutorials/iw3xradiant-d3dbsp/");
				BULLET_LINK("Generating Reflections", "https://xoxor4d.github.io/tutorials/iw3xradiant-d3dbsp-reflections/");
				ImGui::TreePop();
			}

			SPACING(0.0f, 0.0f);

			if (ImGui::TreeNodeEx("Effects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				BULLET_LINK("Placement and general usage", "https://xoxor4d.github.io/tutorials/iw3xradiant-using-effects/");
				BULLET_LINK("Editing Effects using the Editor", "https://xoxor4d.github.io/tutorials/iw3xradiant-effects-editor/");
				BULLET_LINK("Generate CreateFX files", "https://xoxor4d.github.io/tutorials/iw3xradiant-createfx/");
				ImGui::TreePop();
			}

			SPACING(0.0f, 0.0f);

			if (ImGui::TreeNodeEx("Physics (Nvidia PhysX)", ImGuiTreeNodeFlags_DefaultOpen))
			{
				BULLET_LINK("Effects with enabled physics and debug data", "https://xoxor4d.github.io/tutorials/iw3xradiant-physx-effects/");
				BULLET_LINK("Enable physics on prefabs", "https://xoxor4d.github.io/tutorials/iw3xradiant-physx-prefabs/");
				ImGui::TreePop();
			}

			
			SPACING(0.0f, 8.0f);

			ImGui::End();
		}
		else
		{
			return false;
		}

		return true;
	}

	REGISTER_GUI(help_dialog);
}
