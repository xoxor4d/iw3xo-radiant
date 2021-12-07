#include "std_include.hpp"

#define IW3XO_CHANGELOG_TITLE_FMT	

namespace ggui::about
{
	void menu(ggui::imgui_context_menu& menu)
	{
		ImGui::SetNextWindowSize(ImVec2(400.0f, 390.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if(ImGui::Begin("About##window", &menu.menustate, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			SPACING(0.0f, 2.0f);

			const char* version_str = utils::va("IW3xRadiant :: %.lf :: %s", REVISION, __TIMESTAMP__);
			ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::SetCursorForCenteredText(version_str);
			ImGui::TextUnformatted(version_str);
			ImGui::PopFont();

			SPACING(0.0f, 2.0f);
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
			SPACING(0.0f, 2.0f);
			
			const char* github_repo_str = "Github Repository";
			ImGui::SetCursorForCenteredText(github_repo_str);
			ImGui::TextURL(github_repo_str, "https://github.com/xoxor4d/iw3xo-radiant");

			const char* github_project_str = "Github Project Page";
			ImGui::SetCursorForCenteredText(github_project_str);
			ImGui::TextURL(github_project_str, "https://xoxor4d.github.io/projects/iw3xo-radiant/");

			const char* latest_release_str = "Latest build";
			ImGui::SetCursorForCenteredText(latest_release_str);
			ImGui::TextURL(latest_release_str, "https://github.com/xoxor4d/iw3xo-radiant/releases");
			
			const char* discord_invite_str = "Discord";
			ImGui::SetCursorForCenteredText(discord_invite_str);
			ImGui::TextURL(discord_invite_str, "https://discord.gg/t5jRGbj");

			SPACING(0.0f, 2.0f);
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
			SPACING(0.0f, 2.0f);

			const char* credits_title_str = "Credits / Thanks to:";

			ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::SetCursorForCenteredText(credits_title_str);
			ImGui::TextUnformatted(credits_title_str);
			ImGui::PopFont();

			SPACING(0.0f, 2.0f);

			const char* cred01_str = "The X-Labs Team (especially Snake)";
			ImGui::SetCursorForCenteredText(cred01_str);
			ImGui::TextURL(cred01_str, "https://github.com/XLabsProject/");

			const char* cred02_str = "The Plutonium Project Team (especially Rektinator)";
			ImGui::SetCursorForCenteredText(cred02_str);
			ImGui::TextURL(cred02_str, "https://plutonium.pw/");

			const char* cred03_str = "Nukem (LinkerMod / Detours)";
			ImGui::SetCursorForCenteredText(cred03_str);
			ImGui::TextURL(cred03_str, "https://github.com/Nukem9");

			const char* cred04_str = "ocornut (ImGui)";
			ImGui::SetCursorForCenteredText(cred04_str);
			ImGui::TextURL(cred04_str, "https://github.com/ocornut/imgui");

			const char* cred05_str = "CedricGuillemet (ImGuizmo)";
			ImGui::SetCursorForCenteredText(cred05_str);
			ImGui::TextURL(cred05_str, "https://github.com/CedricGuillemet/ImGuizmo");

			const char* cred06_str = "nlohmann (fifo_map)";
			ImGui::SetCursorForCenteredText(cred06_str);
			ImGui::TextURL(cred06_str, "https://github.com/nlohmann/fifo_map");
			
			ImGui::End();
		}
	}
}