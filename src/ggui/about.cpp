#include "std_include.hpp"

#define CENTER_URL(text, link)					\
	ImGui::SetCursorForCenteredText((text));	\
	ImGui::TextURL((text), (link));

namespace ggui
{
	void about_dialog::gui()
	{
		ImGui::SetNextWindowSize(game::glob::gh_update_avail ? ImVec2(480.0f, 810.0f) : ImVec2(400.0f, 560.0f));
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("About##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
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

			CENTER_URL("Github Repository", "https://github.com/xoxor4d/iw3xo-radiant");
			CENTER_URL("Github Project Page", "https://xoxor4d.github.io/projects/iw3xo-radiant/");
			CENTER_URL("Latest build", "https://github.com/xoxor4d/iw3xo-radiant/releases");
			CENTER_URL("Discord", "https://discord.gg/t5jRGbj");

			SPACING(0.0f, 8.0f);
			ImGui::Separator();
			SPACING(0.0f, 8.0f);

			const char* credits_title_str = "Credits / Thanks to:";
			ImGui::PushFontFromIndex(BOLD_18PX);
			ImGui::SetCursorForCenteredText(credits_title_str);
			ImGui::TextUnformatted(credits_title_str);
			ImGui::PopFont();

			SPACING(0.0f, 8.0f);

			CENTER_URL("The X-Labs Team (especially Snake)", "https://github.com/XLabsProject/");
			CENTER_URL("The Plutonium Project Team (especially Rektinator)", "https://plutonium.pw/");
			CENTER_URL("Nukem (LinkerMod / Detours)", "https://github.com/Nukem9");
			CENTER_URL("JTAG (NootNoot)", "https://twitter.com/imjtagmodz");
			CENTER_URL("ocornut (Dear ImGui)", "https://github.com/ocornut/imgui");
			CENTER_URL("CedricGuillemet (ImGuizmo)", "https://github.com/CedricGuillemet/ImGuizmo");
			CENTER_URL("nlohmann (fifo_map)", "https://github.com/nlohmann/fifo_map");
			CENTER_URL("David Gallardo (imgui_color_gradient)", "https://gist.github.com/galloscript/8a5d179e432e062550972afcd1ecf112");
			CENTER_URL("nem0 (CurveEditor)", "https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930");
			CENTER_URL("zfedoran (ImGui Spinner)", "https://github.com/ocornut/imgui/issues/1901");
			CENTER_URL("maluoi (tga writer)", "https://gist.github.com/maluoi/ade07688e741ab188841223b8ffeed22");
			CENTER_URL("Infinity Ward (OG. Radiant and Effects Framework)", "https://www.infinityward.com/");
			CENTER_URL("id-Software (OG. OG. Radiant)", "https://github.com/id-Software/Quake-III-Arena/tree/master/q3radiant");

			SPACING(0.0f, 8.0f);

			ImGui::End();
		}
	}

	REGISTER_GUI(about_dialog);
}
