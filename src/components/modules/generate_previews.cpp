#include "std_include.hpp"

namespace components
{
	void generate_previews::generate_prefab_previews(const std::filesystem::path& path)
	{
		const auto egui = GET_GUI(ggui::entity_dialog);

		std::filesystem::path m_prefab_directory = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");
							  //m_prefab_directory /= "prefabs";

		// setup camera res
		const int shot_rest = 256;
		const auto camgui = GET_GUI(ggui::camera_dialog);
		const auto saved_width = static_cast<int>(camgui->rtt_get_size().x);
		const auto saved_height = static_cast<int>(camgui->rtt_get_size().y);

		SetWindowPos(
		  cmainframe::activewnd->m_pCamWnd->GetWindow(),
		  HWND_BOTTOM,
		  static_cast<int>(camgui->rtt_get_position().x),
		  static_cast<int>(camgui->rtt_get_position().y),
		  shot_rest,
		  shot_rest,
		  SWP_NOZORDER);

		game::dx->targetWindowIndex = -1; // needed or R_SetupRendertarget does not set the resolution
		game::R_SetupRendertarget_CheckDevice(cmainframe::activewnd->m_pCamWnd->GetWindow());

		// #
		// background color

		//game::vec4_t color_back;
		//utils::vector::copy(game::g_qeglobals->d_savedinfo.colors[game::COLOR_CAMERABACK], color_back, 4);
		//utils::vector::copy(dvars::gui_dockingpreview_color->current.vector, game::g_qeglobals->d_savedinfo.colors[game::COLOR_CAMERABACK], 4);


		// #
		// prefab render loop

		for (auto& it : std::filesystem::directory_iterator(path))
		{
			const auto& full_path = it.path();
			const auto rel_path = std::filesystem::relative(full_path, m_prefab_directory);

			//game::printf_to_console("thumb > %s\n", full_path.string().c_str());

			if (!it.is_directory() && full_path.extension() == ".map")
			{
				game::map_load_from_file(full_path.string().c_str());

				// #
				// center camera on prefab

				game::vec3_t mins, maxs;
				utils::vector::set_vec3(mins, 131072.0f);
				utils::vector::set_vec3(maxs, -131072.0f);

				int num_brushes = 0;
				FOR_ALL_ACTIVE_BRUSHES(sb)
				{
					utils::vector::clamp_vec3(sb->def->mins, sb->def->maxs, mins, maxs);
					num_brushes++;
				}

				const game::vec3_t center = {
					( (maxs[0] + mins[0]) * 0.5f ),
					( (maxs[1] + mins[1]) * 0.5f ),
					( (maxs[2] + mins[2]) * 0.5f ) };

				const game::vec3_t dimensions = {
					( maxs[0] - mins[0] ),
					( maxs[1] - mins[1] ),
					( maxs[2] - mins[2] ) };


				utils::vector::copy(center, cmainframe::activewnd->m_pCamWnd->camera.origin);

				const float largest_side = fabsf(fmaxf(dimensions[0], fmaxf(dimensions[1], dimensions[2])));
				const float distance_to_fit = (largest_side / (tanf(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f) * 0.75f);

				if(dimensions[0] < dimensions[1])
				{
					cmainframe::activewnd->m_pCamWnd->camera.origin[0] -= distance_to_fit;
				}
				else
				{
					game::vec3_t dir, angles = {};
					cmainframe::activewnd->m_pCamWnd->camera.origin[1] -= distance_to_fit;

					utils::vector::subtract(center, cmainframe::activewnd->m_pCamWnd->camera.origin, dir);
					utils::vector::vectoangles(dir, angles);

					utils::vector::copy(angles, cmainframe::activewnd->m_pCamWnd->camera.angles);
				}

				// #
				// re-render

				game::R_BeginFrame();

				//game::R_Clear(7, game::g_qeglobals->d_savedinfo.colors[game::COLOR_UNKOWN3], 1.0f, false);
				game::R_Clear(7, dvars::gui_window_bg_color->current.vector, 1.0f, false);

				// CCamWnd::Cam_Draw
				utils::hook::call<void(__fastcall)(ccamwnd*)>(0x407DC0)(cmainframe::activewnd->m_pCamWnd);

				game::R_EndFrame();
				game::R_IssueRenderCommands(-1);
				game::R_SortMaterials();


				// #
				// take screenshot

				std::string file_path = game::Dvar_FindVar("fs_homepath")->current.string;
							file_path += R"(\IW3xRadiant\prefab_thumbs\)";

				auto thumbnail_str = GET_GUI(ggui::prefab_preview_dialog)->get_thumbnail_string(rel_path.has_parent_path() ? rel_path.parent_path() : "", rel_path.filename().string());
					 thumbnail_str += ".jpg";

				file_path += thumbnail_str;

				IDirect3DSurface9* surf_backbuffer = nullptr;
				
				components::renderer::get_window(components::renderer::CCAMERAWND)->swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surf_backbuffer);
				D3DXSaveSurfaceToFileA(file_path.c_str(), D3DXIFF_JPG, surf_backbuffer, nullptr, nullptr);

				surf_backbuffer->Release();

				//break;
			}
			/*else
			{
				game::printf_to_console("> skipped\n");
			}*/
		}

		// #
		// restore orig resolution

		SetWindowPos(
			cmainframe::activewnd->m_pCamWnd->GetWindow(),
			HWND_BOTTOM,
			static_cast<int>(camgui->rtt_get_position().x),
			static_cast<int>(camgui->rtt_get_position().y),
			saved_width,
			saved_height,
			SWP_NOZORDER);

		game::dx->targetWindowIndex = -1; // needed or R_SetupRendertarget does not set the resolution
		game::R_SetupRendertarget_CheckDevice(cmainframe::activewnd->m_pCamWnd->GetWindow());

		//utils::vector::copy(color_back, game::g_qeglobals->d_savedinfo.colors[game::COLOR_CAMERABACK], 4);

		cdeclcall(void, 0x4870C0); // Map_New
		game::R_ReloadImages();
	}

	generate_previews::generate_previews()
	{ }

	generate_previews::~generate_previews()
	{ }
}
