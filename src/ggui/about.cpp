#include "std_include.hpp"

namespace ggui
{
	void about_dialog::gui()
	{
		ImGui::SetNextWindowSize(game::glob::gh_update_avail ? ImVec2(480.0f, 740.0f) : ImVec2(400.0f, 490.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("About##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse /*| ImGuiWindowFlags_NoResize*/ | ImGuiWindowFlags_NoDocking))
		{
			SPACING(0.0f, 8.0f);

			if (game::glob::gh_update_avail)
			{
				ImGui::PushFontFromIndex(BOLD_18PX);
				const char* tag_date = utils::va("[TAG %s] :: [%s]", game::glob::gh_update_tag.c_str(), game::glob::gh_update_date.c_str());
				ImGui::SetCursorForCenteredText(tag_date);
				ImGui::TextWrapped(tag_date);
				ImGui::PopFont();

				const char* tag_title = game::glob::gh_update_title.c_str();
				ImGui::SetCursorForCenteredText(tag_title);
				ImGui::TextWrapped(tag_title);

				SPACING(0.0f, 12.0f);

				const char* gh_changelog = utils::va("https://github.com/xoxor4d/iw3xo-radiant/wiki/Changelog#%s", game::glob::gh_update_tag.c_str());
				ImGui::SetCursorForCenteredText("Changelog:"); ImGui::TextUnformatted("Changelog:");
				ImGui::SetCursorForCenteredText(gh_changelog); ImGui::TextURL(gh_changelog, gh_changelog);

				SPACING(0.0f, 2.0f);

				const char* gh_releases = "https://github.com/xoxor4d/iw3xo-radiant/releases";
				ImGui::SetCursorForCenteredText("GitHub Releases:"); ImGui::TextUnformatted("GitHub Releases:");
				ImGui::SetCursorForCenteredText(gh_releases); ImGui::TextURL(gh_releases, gh_releases);

				SPACING(0.0f, 2.0f);

				ImGui::SetCursorForCenteredText("Direct download:");  ImGui::TextUnformatted("Direct download:");
				ImGui::SetCursorForCenteredText(game::glob::gh_update_zip_name.c_str()); ImGui::TextURL(game::glob::gh_update_zip_name.c_str(), game::glob::gh_update_link.c_str());

				SPACING(0.0f, 16.0f);
				ImGui::Separator();
				SPACING(0.0f, 8.0f);
			}

			ImGui::SetCursorForCenteredText("current version");
			ImGui::TextUnformatted("current version");
			const char* version_str = utils::va("%s :: %s", VERSION, __DATE__);
			ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::SetCursorForCenteredText(version_str);
			ImGui::TextUnformatted(version_str);
#if DEBUG
			ImGui::SetCursorForCenteredText("DEBUG BUILD");
			ImGui::TextColored(ImVec4(0.64f, 0.23f, 0.18f, 1.0f), "DEBUG BUILD");
#endif
			ImGui::PopFont();

			SPACING(0.0f, 8.0f);

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

			SPACING(0.0f, 8.0f);
			ImGui::Separator();
			SPACING(0.0f, 8.0f);

			const char* credits_title_str = "Credits / Thanks to:";
			ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::SetCursorForCenteredText(credits_title_str);
			ImGui::TextUnformatted(credits_title_str);
			ImGui::PopFont();

			SPACING(0.0f, 8.0f);

			const char* cred01_str = "The X-Labs Team (especially Snake)";
			ImGui::SetCursorForCenteredText(cred01_str);
			ImGui::TextURL(cred01_str, "https://github.com/XLabsProject/");

			const char* cred02_str = "The Plutonium Project Team (especially Rektinator)";
			ImGui::SetCursorForCenteredText(cred02_str);
			ImGui::TextURL(cred02_str, "https://plutonium.pw/");

			const char* cred03_str = "Nukem (LinkerMod / Detours)";
			ImGui::SetCursorForCenteredText(cred03_str);
			ImGui::TextURL(cred03_str, "https://github.com/Nukem9");

			const char* cred04_str = "ocornut (Dear ImGui)";
			ImGui::SetCursorForCenteredText(cred04_str);
			ImGui::TextURL(cred04_str, "https://github.com/ocornut/imgui");

			const char* cred05_str = "CedricGuillemet (ImGuizmo)";
			ImGui::SetCursorForCenteredText(cred05_str);
			ImGui::TextURL(cred05_str, "https://github.com/CedricGuillemet/ImGuizmo");

			const char* cred06_str = "nlohmann (fifo_map)";
			ImGui::SetCursorForCenteredText(cred06_str);
			ImGui::TextURL(cred06_str, "https://github.com/nlohmann/fifo_map");

			const char* cred07_str = "David Gallardo (imgui_color_gradient)";
			ImGui::SetCursorForCenteredText(cred07_str);
			ImGui::TextURL(cred07_str, "https://gist.github.com/galloscript/8a5d179e432e062550972afcd1ecf112");

			const char* cred08_str = "nem0 (CurveEditor)";
			ImGui::SetCursorForCenteredText(cred08_str);
			ImGui::TextURL(cred08_str, "https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930");

			const char* cred09_str = "Infinity Ward (OG. Radiant and Effects Framework)";
			ImGui::SetCursorForCenteredText(cred09_str);
			ImGui::TextURL(cred09_str, "https://www.infinityward.com/");

			SPACING(0.0f, 8.0f);

			ImGui::End();
		}
	}

	REGISTER_GUI(about_dialog);
}
