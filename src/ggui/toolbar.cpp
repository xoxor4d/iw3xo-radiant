#include "std_include.hpp"

#define TB_SEPARATOR											\
	ImGui::SameLine(); SPACING(2.0f, 0.0f); ImGui::SameLine();	\
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);			\
	ImGui::SameLine(); SPACING(4.0f, 0.0f); ImGui::SameLine();	\


namespace ggui::toolbar
{
	const ImVec2		IMAGEBUTTON_SIZE = ImVec2(28.0f, 28.0f);
	const std::string	INI_FILENAME = "iw3r_toolbar.ini"s;

	
	void image_button(const char* image_name, bool& hovered_state, E_CALLTYPE calltype, uint32_t func_addr, const char* tooltip)
	{
		if (const auto	image = game::Image_RegisterHandle(image_name);
						image && image->texture.data)
		{
			const ImVec2 uv0 = hovered_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = hovered_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, IMAGEBUTTON_SIZE, uv0, uv1, 0))
			{
				switch(calltype)
				{
				case MAINFRAME_CDECL:
					mainframe_cdeclcall(void, func_addr);
					break;
					
				case MAINFRAME_THIS:
					mainframe_thiscall(void, func_addr);
					break;
					
				case MAINFRAME_STD:
					mainframe_stdcall(void, func_addr);
					break;
					
				case CDECLCALL:
					cdeclcall(void, func_addr);
					break;
				}
			}
			
			if(tooltip) 
			{
				TT(tooltip);
			}

			hovered_state = ImGui::IsItemHovered();
		}
		else
		{
			if(ImGui::Button(image_name))
			{
				switch (calltype)
				{
				case MAINFRAME_CDECL:
					mainframe_cdeclcall(void, func_addr);
					break;

				case MAINFRAME_THIS:
					mainframe_thiscall(void, func_addr);
					break;

				case MAINFRAME_STD:
					mainframe_stdcall(void, func_addr);
					break;

				case CDECLCALL:
					cdeclcall(void, func_addr);
					break;
				}
			}
			
			if (tooltip)
			{
				TT(tooltip);
			}
		}
	}

	bool image_togglebutton(const char* image_name, bool toggle_state, const char* tooltip)
	{
		bool ret_state = false;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(25, 25, 25, 50));
		
		if (const auto	image = game::Image_RegisterHandle(image_name);
						image && image->texture.data)
		{
			const ImVec2 uv0 = toggle_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = toggle_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, IMAGEBUTTON_SIZE, uv0, uv1, 0)) 
			{
				ret_state = true;
			}

			if (tooltip) 
			{
				TT(tooltip);
			}
		}
		else
		{
			if (ImGui::Button(image_name))
			{
				ret_state = true;
			}

			if (tooltip)
			{
				TT(tooltip);
			}
		}

		ImGui::PopStyleColor();
		return ret_state;
	}

	
	// *
	// 
	
	bool toolbar_initiated = false;
	nlohmann::fifo_map<std::string, std::function<void()>> tbedit_elements;

	int tb_element_id = 0;
	struct tb_order_element_s
	{
		std::string name;
		int			id;
		bool		visible;
		bool		is_separator;
		bool		debug;
	};
	std::vector<tb_order_element_s> tbedit_ordered_list;

	
	struct tb_selection_s
	{
		bool is_selected;
		uint32_t element_id;
		uint32_t element_pos;
	};
	tb_selection_s tbedit_selection = {};


	void register_element(const std::string& name, std::function<void()> callback)
	{
		bool is_debug = false;
		tbedit_elements[name] = callback;

		// if separator
		if (utils::starts_with(name, ";"s))
		{
			// insert before selection
			if(tbedit_selection.is_selected)
			{
				tbedit_ordered_list.insert(
					tbedit_ordered_list.begin() + tbedit_selection.element_pos,
					tb_order_element_s("; ------------", tb_element_id++, true, true, is_debug));
			}
			// emplace back if nothing is selected
			else
			{
				tbedit_ordered_list.emplace_back(
					tb_order_element_s("; ------------", tb_element_id++, true, true, is_debug));
			}

			return;
		}
		
		if(utils::starts_with(name, "debug_"s))
		{
			is_debug = true;
		}

		tbedit_ordered_list.emplace_back(
			tb_order_element_s(name, tb_element_id++, true, false, is_debug));
	}

	
	// *
	// all toolbar elements are registered here (default sort if no ini is present)
	void toolbar_elements_init()
	{
		register_element("open"s, []()
			{
				static bool hov_open;
			
				//CMainFrame::OnFileOpen
				image_button("open", hov_open, MAINFRAME_CDECL, 0x423AE0, std::string("Open File [" + hotkeys::get_hotkey_for_command("FileOpen") + "]").c_str());
			});

		register_element("save"s, []()
			{
				static bool hov_save;
			
				//CMainFrame::OnFileSave
				image_button("save", hov_save, MAINFRAME_CDECL, 0x423B80, std::string("Save File [" + hotkeys::get_hotkey_for_command("FileSave") + "]").c_str());
			});

		register_element("flip_x"s, []()
			{
				static bool hov_flipx;

				// CMainFrame::OnBrushFlipx
				image_button("flip_x", hov_flipx, CDECLCALL, 0x4250A0, "Flip Brush X-Axis");
			});

		register_element("flip_y"s, []()
			{
				static bool hov_flipy;

				// CMainFrame::OnBrushFlipy
				image_button("flip_y", hov_flipy, CDECLCALL, 0x4250C0, "Flip Brush Y-Axis");
			});

		register_element("flip_z"s, []()
			{
				static bool hov_flipz;

				// CMainFrame::OnBrushFlipz
				image_button("flip_z", hov_flipz, CDECLCALL, 0x4250E0, "Flip Brush Z-Axis");
			});

		register_element(";"s, nullptr);

		register_element("rotate_x"s, []()
			{
				static bool hov_rotx;

				// CMainFrame::OnBrushRotatex
				image_button("rotate_x", hov_rotx, CDECLCALL, 0x425100, "Rotate Brush X-Axis");
			});

		register_element("rotate_y"s, []()
			{
				static bool hov_roty;

				// CMainFrame::OnBrushRotatey
				image_button("rotate_y", hov_roty, CDECLCALL, 0x425190, "Rotate Brush Y-Axis");
			});

		register_element("rotate_z"s, []()
			{
				static bool hov_rotz;

				// CMainFrame::OnBrushRotatez
				image_button("rotate_z", hov_rotz, CDECLCALL, 0x425220, std::string("Rotate Brush Z-Axis [" + hotkeys::get_hotkey_for_command("RotateZ") + "]").c_str());
			});
		
		register_element(";"s, nullptr);

		register_element("select_complete_tall"s, []()
			{
				static bool hov_sel_compl_tall;

				// CMainFrame::OnSelectionSelectcompletetall
				image_button("select_complete_tall", hov_sel_compl_tall, CDECLCALL, 0x426340, "Select Complete Tall");
			});

		register_element("select_touching"s, []()
			{
				static bool hov_sel_touching;

				// CMainFrame::OnSelectionSelecttouching
				image_button("select_touching", hov_sel_touching, CDECLCALL, 0x426370, "Select Touching");
			});

		register_element("select_partial_tall"s, []()
			{
				static bool hov_sel_partial_tall;

				// CMainFrame::OnSelectionSelectpartialtall
				image_button("select_partial_tall", hov_sel_partial_tall, CDECLCALL, 0x426360, "Select Partial Tall");
			});

		register_element("select_inside"s, []()
			{
				static bool hov_sel_inside;

				// CMainFrame::OnSelectionSelectinside
				image_button("select_inside", hov_sel_inside, CDECLCALL, 0x426350, "Select Inside");
			});

		register_element(";"s, nullptr);

		register_element("clipper"s, []()
			{
				// CMainFrame::OnViewClipper
				if (image_togglebutton("clipper", game::g_bClipMode, "Toggle Clipper")) {
					mainframe_thiscall(LRESULT, 0x426510);
				}
			});

		register_element("csg_merge"s, []()
			{
				static bool hov_csg_merge;

				// CMainFrame::OnSelectionCsgmerge
				image_button("csg_merge", hov_csg_merge, CDECLCALL, 0x4255D0, std::string("CSG Merge [" + hotkeys::get_hotkey_for_command("CSGMerge") + "]").c_str());
			});

		register_element("csg_hollow"s, []()
			{
				static bool hov_csg_hollow;

				// CMainFrame::OnSelectionMakehollow
				image_button("csg_hollow", hov_csg_hollow, CDECLCALL, 0x425570, "CSG Hollow");
			});

		register_element("texflip_x"s, []()
			{
				static bool hov_texflipx;

				// CMainFrame::OnTextureFlipX
				image_button("texflip_x", hov_texflipx, CDECLCALL, 0x42BF40, "Flip Texture X-Axis");
			});

		register_element("texflip_y"s, []()
			{
				static bool hov_texflipy;

				// CMainFrame::OnTextureFlipY
				image_button("texflip_y", hov_texflipy, CDECLCALL, 0x42BF50, "Flip Texture Y-Axis");
			});

		register_element("texflip_90"s, []()
			{
				static bool hov_texflip90;

				// CMainFrame::OnTextureFlip90
				image_button("texflip_90", hov_texflip90, CDECLCALL, 0x42BF60, "Rotate Texture 90 Degrees");
			});

		register_element("cycle_layer"s, []()
			{
				static bool hov_cycle_layer;

				// CMainFrame::OnEditLayerCycle
				image_button("cycle_layer", hov_cycle_layer, CDECLCALL, 0x424010, std::string("Cycle Texture Layer [" + hotkeys::get_hotkey_for_command("TexLayerCycle") + "]").c_str());
			});
		
		register_element(";"s, nullptr);

		register_element("camera_movement"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnToggleCameraMovementMode
				ImGui::BeginGroup();
				{
					ImVec2 prebutton_cursor = ImGui::GetCursorScreenPos();

					if (image_togglebutton("camera_movement", prefs->camera_mode,
						"Toggle Camera Movement Mode"))
					{
						mainframe_thiscall(LRESULT, 0x429EB0);
					}

					if (prefs->camera_mode)
					{
						prebutton_cursor.x += (IMAGEBUTTON_SIZE.x * 0.45f);
						prebutton_cursor.y += (IMAGEBUTTON_SIZE.y * 0.4f);

						ImGui::PushFontFromIndex(REGULAR_12PX);
						ImGui::SetCursorScreenPos(prebutton_cursor);
						ImGui::Text("%d/2", prefs->camera_mode);
						ImGui::PopFont();
					}
				}
				ImGui::EndGroup();
			});

		register_element("cubic_clip"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnViewCubicclipping
				if (image_togglebutton("cubic_clip", prefs->m_bCubicClipping, 
					std::string("Cubic Clipping [" + hotkeys::get_hotkey_for_command("ToggleCubicClip") + "]").c_str()))
				{
					mainframe_thiscall(LRESULT, 0x428F90);
				}
			});

		register_element("cycle_xyz"s, []()
			{
				static bool hov_cycle_xyz;
			
				// CMainFrame::OnViewNextview
				ImGui::BeginGroup();
				{
					ImVec2	prebutton_cursor = ImGui::GetCursorScreenPos();
					image_button("empty_hover", hov_cycle_xyz, MAINFRAME_THIS, 0x426DB0, std::string("Cycle Grid Window View [" + hotkeys::get_hotkey_for_command("NextView") + "]").c_str());

					ImGui::PushFontFromIndex(REGULAR_18PX);
					const auto viewtype = cmainframe::activewnd->m_pXYWnd->m_nViewType;

					const char* curr_view_str = viewtype == 0 ? "YZ" : viewtype == 1 ? "XZ" : "XY";
					const auto text_size = ImGui::CalcTextSize(curr_view_str);

					prebutton_cursor.x += (IMAGEBUTTON_SIZE.x * 0.5f - (text_size.x * 0.5f));
					prebutton_cursor.y += (IMAGEBUTTON_SIZE.y * 0.5f - (text_size.y * 0.5f));

					ImGui::SetCursorScreenPos(prebutton_cursor);
					ImGui::Text("%s", curr_view_str);
					ImGui::PopFont();
				}
				ImGui::EndGroup();
			});

		register_element("plant_models"s, []()
			{
				const auto prefs = game::g_PrefsDlg();

				// CMainFrame::OnPlantModel
				if (image_togglebutton("plant_models", prefs->m_bDropModel,
					"Plant models and apply random scale and rotation"))
				{
					mainframe_thiscall(LRESULT, 0x42A0E0);
				}
			});

		register_element("plant_orient_to_floor"s, []()
			{
				const auto prefs = game::g_PrefsDlg();

				// CMainFrame::OnPlantModel
				if (image_togglebutton("plant_orient_to_floor", prefs->m_bOrientModel,
					"Orient dropped selection to the floor"))
				{
					mainframe_thiscall(LRESULT, 0x4258F0);
				}
			});

		register_element("plant_force_drop_height"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnForceZeroDropHeight
				if (image_togglebutton("plant_force_drop_height", prefs->m_bForceZeroDropHeight,
					"Force drop height to 0"))
				{
					mainframe_thiscall(LRESULT, 0x42A000);
				}
			});

		register_element("drop_entities_floor"s, []()
			{
				static bool hov_drop_entities;

				// CMainFrame::OnDropSelected
				image_button("drop_entities_floor", hov_drop_entities, CDECLCALL, 0x425BE0, "Drop selection to the floor [CTRL-ALT-MOUSE3]");
			});

		register_element("drop_entities_floor_relative"s, []()
			{
				static bool hov_drop_entities_relative_z;

				// CMainFrame::OnDropSelectedRelativeZ
				image_button("drop_entities_floor_relative", hov_drop_entities_relative_z, MAINFRAME_THIS, 0x425940, "Drop selection to the floor with relative Z heights");
			});

		register_element("free_rotate"s, []()
			{
				// CMainFrame::OnSelectMouserotate
				if (image_togglebutton("free_rotate", game::g_bRotateMode,
					std::string("Free rotation [" + hotkeys::get_hotkey_for_command("MouseRotate") + "]").c_str()))
				{
					mainframe_thiscall(LRESULT, 0x428570);
				}
			});

		register_element("free_scale"s, []()
			{
				// CMainFrame::OnSelectMousescale
				if (image_togglebutton("free_scale", game::g_bScaleMode,
					"Free scaling"))
				{
					mainframe_thiscall(LRESULT, 0x428D20);
				}
			});

		register_element("lock_x"s, []()
			{
				// CMainFrame::OnScalelockX
				if (image_togglebutton("lock_x",
					(game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6),
					"Lock grid along the x-axis"))
				{
					mainframe_thiscall(LRESULT, 0x428BC0);
				}
			});

		register_element("lock_y"s, []()
			{
				// CMainFrame::OnScalelockY
				if (image_togglebutton("lock_y",
					(game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5),
					"Lock grid along the y-axis"))
				{
					mainframe_thiscall(LRESULT, 0x428B60);
				}
			});

		register_element("lock_z"s, []()
			{
				// CMainFrame::OnScalelockZ
				if (image_togglebutton("lock_z",
					(game::g_nScaleHow > 0 && game::g_nScaleHow <= 3),
					"Lock grid along the z-axis"))
				{
					mainframe_thiscall(LRESULT, 0x428B90);
				}
			});

		register_element(";"s, nullptr);

		register_element("show_patches_as"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnPatchWireframe
				ImGui::BeginGroup();
				{
					ImVec2 prebutton_cursor = ImGui::GetCursorScreenPos();

					if (image_togglebutton("show_patches_as", prefs->g_nPatchAsWireframe,
						"Show patches as wireframe"))
					{
						mainframe_thiscall(LRESULT, 0x42A300);
					}

					if (prefs->g_nPatchAsWireframe)
					{
						prebutton_cursor.x += (IMAGEBUTTON_SIZE.x * 0.45f);
						prebutton_cursor.y += (IMAGEBUTTON_SIZE.y * 0.45f);

						ImGui::PushFontFromIndex(REGULAR_12PX);
						ImGui::SetCursorScreenPos(prebutton_cursor);
						ImGui::Text("%d/2", prefs->g_nPatchAsWireframe);
						ImGui::PopFont();
					}
				}
				ImGui::EndGroup();
			});

		register_element("redisperse_patch_points"s, []()
			{
				static bool hov_redisp_patch_pts;

				// CMainFrame::OnRedistPatchPoints
				image_button("redisperse_patch_points", hov_redisp_patch_pts, CDECLCALL, 0x42A270,
					std::string("Redisperse Patch Points [" + hotkeys::get_hotkey_for_command("RedisperseVertices") + "]").c_str());
			});

		register_element("weld_equal_patches_move"s, []()
			{
				const auto prefs = game::g_PrefsDlg();

				// CMainFrame::OnPatchWeld
				if (image_togglebutton("weld_equal_patches_move", 
					prefs->g_bPatchWeld,
					"Weld equal patch points during moves"))
				{
					mainframe_thiscall(LRESULT, 0x42A400);
				}
			});

		register_element("select_drill_down_vertices"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnPatchDrilldown
				if (image_togglebutton("select_drill_down_vertices", 
					prefs->patch_drill_down,
					"Select invisible vertices (drill down rows/columns)"))
				{
					mainframe_thiscall(LRESULT, 0x42A510);
				}
			});
		
		register_element("toggle_lock_vertices_mode"s, []()
			{
				// CMainFrame::ToggleLockPatchVertMode
				if (image_togglebutton("toggle_lock_vertices_mode", 
					game::g_qeglobals->bLockPatchVerts,
					"Toggle lock-vertex mode"))
				{
					mainframe_thiscall(LRESULT, 0x42B4F0);
				}
			});

		register_element("toggle_unlock_vertices_mode"s, []()
			{
				// CMainFrame::ToggleUnlockPatchVertMode
				if (image_togglebutton("toggle_unlock_vertices_mode", 
					game::g_qeglobals->bUnlockPatchVerts,
					"Toggle unlock-vertex mode"))
				{
					mainframe_thiscall(LRESULT, 0x42B510);
				}
			});

		register_element("cycle_patch_edge_direction"s, []()
			{
				// CMainFrame::OnCycleTerrainEdge
				if (image_togglebutton("cycle_patch_edge_direction",
					game::g_qeglobals->d_select_mode == 9,
					"Toggle terrain-quad edge cycle mode"))
				{
					mainframe_thiscall(LRESULT, 0x42B530);
				}
			});

		register_element("tolerant_weld"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnTolerantWeld
				if (image_togglebutton("tolerant_weld",
					prefs->m_bTolerantWeld,
					"Toggle tolerant weld / Draw tolerant weld lines"))
				{
					mainframe_thiscall(LRESULT, 0x42A130);
				}
			});

		register_element(";"s, nullptr);

		register_element("toggle_draw_surfs_portal"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnToggleDrawSurfs
				if (image_togglebutton("toggle_draw_surfs_portal",
					prefs->draw_toggle,
					"Toggle drawing of portal no-draw surfaces"))
				{
					mainframe_thiscall(LRESULT, 0x42A040);
				}
			});

		register_element("dont_select_curve"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnDontselectcurve
				if (image_togglebutton("dont_select_curve",
					!prefs->m_bSelectCurves,
					"Disable selection of patches"))
				{
					mainframe_thiscall(LRESULT, 0x429920);
				}
			});

		register_element("dont_select_entities"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnDisableSelectionOfEntities
				if (image_togglebutton("dont_select_entities",
					prefs->entities_off,
					"Disable selection of entities"))
				{
					mainframe_thiscall(LRESULT, 0x429F60);
				}
			});

		register_element("dont_select_sky"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnDisableSelectionOfSky
				if (image_togglebutton("dont_select_sky",
					prefs->sky_brush_off,
					"Disable selection of sky brushes"))
				{
					mainframe_thiscall(LRESULT, 0x429FB0);
				}
			});

		register_element("dont_select_models"s, []()
			{
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnSelectableModels
				if (image_togglebutton("dont_select_models",
					prefs->m_bSelectableModels,
					"Disable selection of static models"))
				{
					mainframe_thiscall(LRESULT, 0x42A280);
				}
			});

#ifdef DEBUG
		register_element(";"s, nullptr);
		
		register_element("debug_reload_images"s, []()
			{
				if (ImGui::Button("Reload Images")) {
					game::R_ReloadImages();
				}
			});
#endif
	}

	
	void load_settings_ini()
	{
		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
						fs_homepath)
		{
			std::ifstream ini;
			std::string ini_path = fs_homepath->current.string;
						ini_path += "\\" + INI_FILENAME;

			ini.open(ini_path.c_str());
			if (!ini.is_open())
			{
				printf("[Toolbar] Failed to open file: \"%s\"", ini_path.c_str());
				return;
			}

			tb_element_id = 0;
			tbedit_ordered_list.clear();

			std::string input;
			int line = 0;

			// read line by line
			while (std::getline(ini, input))
			{
				if (input.find("//") != std::string::npos)
				{
					continue;
				}

				line++;

				// separators
				if (input.find(';') != std::string::npos)
				{
					tbedit_ordered_list.emplace_back(
						tb_order_element_s("; ------------", tb_element_id++, true, true));
					
					continue;
				}

				// split the string on ','
				std::vector<std::string> args = utils::split(input, ',');

				if (args.size() != 2)
				{
					printf("[Toolbar] malformed element @ line #%d (\"%s\")\n", line, input.c_str());
					continue;
				}

				utils::trim(args[0]);
				utils::trim(args[1]);

				// check if the element exists
				if(tbedit_elements.find(args[0]) != tbedit_elements.end())
				{
					tbedit_ordered_list.emplace_back(
						tb_order_element_s(args[0], tb_element_id++, args[1] != "0"s, false));
				}
				else
				{
					printf("[Toolbar] not a valid element @ line #%d (\"%s\")\n", line, input.c_str());
				}
			}

			// add registered elements that are not part of the ini (eg. radiant was updated -> new elements)
			for (const auto& element : tbedit_elements)
			{
				bool found = false;
				bool is_debug = false;

				// do not add separators
				if(utils::starts_with(element.first, ";"s))
				{
					continue;
				}

				if (utils::starts_with(element.first, "debug_"s))
				{
					is_debug = true;
				}
				
				for (uint32_t e = 0; e < tbedit_ordered_list.size(); e++)
				{
					if(element.first == tbedit_ordered_list[e].name)
					{
						found = true;
						break;
					}
				}

				if(!found)
				{
					tbedit_ordered_list.emplace_back(
						tb_order_element_s(element.first, tb_element_id++, true, false, is_debug));
				}
			}
		}
	}

	
	void save_settings_ini()
	{
		if (tbedit_ordered_list.empty())
		{
			return;
		}

		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
						fs_homepath)
		{
			std::ofstream ini;
			std::string ini_path = fs_homepath->current.string;
						ini_path += "\\" + INI_FILENAME;

			ini.open(ini_path.c_str());
			if (!ini.is_open())
			{
				printf("[Toolbar] Failed to write to file: \"%s\"", ini_path.c_str());
				return;
			}

			ini << "// [Name] [IsVisible] (';' Separator)" << std::endl;

			for (const auto& element : tbedit_ordered_list)
			{
				if(element.debug)
				{
					continue;
				}
				
				if(element.is_separator) 
				{
					ini << "; ------------" << std::endl;
				}
				else
				{
					ini << element.name << "," << element.visible << std::endl;
				}
			}
		}
	}

	
	void menu_toolbar_edit(ggui::imgui_context_menu& menu)
	{
		ImGui::Begin("Toolbar Elements##xywnd", &menu.menustate, ImGuiWindowFlags_NoCollapse);

		
		if (ImGui::Button("Add Separator"))
		{
			register_element(";"s, nullptr);
		}

		
		ImGui::SameLine();
		if (ImGui::Button("Delete Selected Separator"))
		{
			const bool out_of_bounds = tbedit_selection.element_pos >= tbedit_ordered_list.size();

			if (tbedit_selection.is_selected 
				&& !out_of_bounds 
				&& tbedit_ordered_list[tbedit_selection.element_pos].is_separator)
			{
				printf("selected sep, deleting ...\n");
				tbedit_ordered_list.erase(tbedit_ordered_list.begin() + tbedit_selection.element_pos);
			}
			else if (out_of_bounds)
			{
				tbedit_selection.is_selected = false;
				printf("out of bounds!\n");
			}
		}
		

		// this does not work with the way we order the list -> flickering
		//ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(1.0f, 4.0f));

		if (ImGui::BeginTable("element_sort_table", 3,
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_ScrollY,
			ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 6.0f)))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, IMAGEBUTTON_SIZE.y);
			ImGui::TableSetupColumn("Element", ImGuiTableColumnFlags_WidthStretch, 200.0f);
			ImGui::TableSetupColumn("Visible", ImGuiTableColumnFlags_WidthFixed, 48.0f);
			ImGui::TableHeadersRow();

			int widget_id_num = 0;

			// loop amount of registered-sorted elements
			for (uint32_t e = 0; e < tbedit_ordered_list.size(); e++)
			{
				const auto element = tbedit_ordered_list[e];

				// check if the element was registered
				if (element.is_separator || tbedit_elements.find(tbedit_ordered_list[e].name) != tbedit_elements.end())
				{
					ImGui::TableNextRow();

					for (uint32_t column = 0; column < 3; column++)
					{
						ImGui::TableNextColumn();
						switch (column)
						{

						case 0:
							if (!element.is_separator)
							{
								if (const auto image = game::Image_RegisterHandle(element.name.c_str());
									image && image->texture.data)
								{
									const ImVec2 uv0 = ImVec2(0.0f, 0.0f);
									const ImVec2 uv1 = ImVec2(0.5f, 1.0f);

									ImGui::Image(image->texture.data, IMAGEBUTTON_SIZE, uv0, uv1);
								}

								// TODO! - add placeholder image or text when icon is missing
							}

							break;


						case 1:
							if (element.is_separator)
							{
								// unique id for each separator
								if (ImGui::Selectable(
									utils::va("------------##%d", tbedit_ordered_list[e].id),
									e == tbedit_selection.element_pos && tbedit_selection.is_selected,
									ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, 
									ImVec2(0.0f, IMAGEBUTTON_SIZE.y)))
								{
									tbedit_selection.is_selected = true;
									tbedit_selection.element_id = tbedit_ordered_list[e].id;
									tbedit_selection.element_pos = e;
								}
							}
							else
							{
								if (ImGui::Selectable(
									element.name.c_str(),
									e == tbedit_selection.element_pos && tbedit_selection.is_selected,
									ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, 
									ImVec2(0.0f, IMAGEBUTTON_SIZE.y)))
								{
									tbedit_selection.is_selected = true;
									tbedit_selection.element_id = tbedit_ordered_list[e].id;
									tbedit_selection.element_pos = e;
								}
							}



							if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
							{
								int n_next = e + (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
								if (n_next >= 0 && static_cast<uint32_t>(n_next) < tbedit_ordered_list.size())
								{
									const auto temp_pair = tbedit_ordered_list[n_next];

									tbedit_ordered_list[n_next] = tbedit_ordered_list[e];
									tbedit_ordered_list[e] = temp_pair;

									ImGui::ResetMouseDragDelta();

									tbedit_selection.is_selected = true;
									tbedit_selection.element_id = tbedit_ordered_list[n_next].id;
									tbedit_selection.element_pos = n_next;
								}
							}
							break;

						case 2:
							if (!element.is_separator)
							{
								ImGui::PushID(widget_id_num); widget_id_num++;
								ImGui::Checkbox("##checkbox", &tbedit_ordered_list[e].visible);
								ImGui::PopID(); // widget_id_num
								break;
							}

							break;

						default:
							break;
						}
					}
				}
			}

			//ImGui::PopStyleVar(1);
			ImGui::EndTable();
		}

		ImGui::End();
	}
	
	void menu(ggui::imgui_context_menu& menu)
	{
		int _stylevars = 0; int _stylecolors = 0;
		const auto prefs = game::g_PrefsDlg();

		if(!toolbar_initiated)
		{
			toolbar_elements_init();
			load_settings_ini();
			toolbar_initiated = true;
		}

		// *
		// create toolbar window

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 4.0f));		_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));	_stylevars++;

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(100, 100, 100, 70));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));		_stylecolors++;

		ImGui::Begin("toolbar##xywnd", nullptr,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar
		);


		// *
		// sizes (save current window position and size)

		menu.position[0] = ImGui::GetWindowPos().x;
		menu.position[1] = ImGui::GetWindowPos().y;
		menu.size[0] = ImGui::GetWindowSize().x;
		menu.size[1] = ImGui::GetWindowSize().y;


		// *
		// gui elements

		static bool hov_open, hov_save, hov_flipx, hov_flipy, hov_flipz, hov_rotx, hov_roty, hov_rotz,
					hov_sel_compl_tall, hov_sel_touching, hov_sel_partial_tall, hov_sel_inside,
					hov_csg_merge, hov_csg_hollow, hov_texflipx, hov_texflipy, hov_texflip90, hov_cycle_layer, hov_cycle_xyz,
					hov_redisp_patch_pts, hov_drop_entities, hov_drop_entities_relative_z;

		SPACING(2.0f, 0.0f);
		ImGui::SameLine();

		// loop amount of registered-sorted elements
		for (uint32_t e = 0; e < tbedit_ordered_list.size(); e++)
		{
			if(tbedit_ordered_list[e].is_separator)
			{
				TB_SEPARATOR;
				continue;
			}
			
			// check if the element was registered
			if (auto element  = tbedit_elements.find(tbedit_ordered_list[e].name);
					 element != tbedit_elements.end())
			{
				if (tbedit_ordered_list[e].visible)
				{
					element->second();
					ImGui::SameLine();
				}
			}
		}

		// TODO! - remove me
		/*ImGui::SameLine();
		if (ImGui::Button("Switch Console <-> Splitter"))
		{
			const auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);

			const auto pTop = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 0, 0);
			const auto pBottom = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 1, 0);

			if (!pTop || !pBottom)
			{
				goto END_GUI;
			}

			const auto _top = pTop->m_hWnd;
			const auto idTop = GetWindowLongA(_top, GWL_ID);

			const auto _bottom = pBottom->m_hWnd;
			const auto idBottom = GetWindowLongA(_bottom, GWL_ID);

			SetWindowLongA(_top, GWL_ID, idBottom);
			SetWindowLongA(_bottom, GWL_ID, idTop);

			vtable->RecalcLayout(&cmainframe::activewnd->m_wndSplit);
		}

		ImGui::SameLine();
		if (ImGui::Button("Hide Console"))
		{
			const auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);
			vtable->DeleteRow(&cmainframe::activewnd->m_wndSplit, 1);
		}*/

		// TODO! - remove me
		/*ImGui::SameLine();
		if (ImGui::Button("Set Statustext"))
		{
			const auto vtable = reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable);
			vtable->SetStatusText(&cmainframe::activewnd->m_wndStatusBar, 0x75);
		}*/

		/*ImGui::SameLine();
		if (ImGui::Button("Toggle Toolbar"))
		{
			typedef void(__thiscall* CFrameWnd_ShowControlBar_t)(CFrameWnd*, CControlBar*, BOOL bShow, BOOL bDelay);
			CFrameWnd_ShowControlBar_t CFrameWnd_ShowControlBar = reinterpret_cast<CFrameWnd_ShowControlBar_t>(0x59E9DD);

			auto vtable = reinterpret_cast<CToolBar_vtbl*>(cmainframe::activewnd->m_wndToolBar.__vftable);
			CFrameWnd_ShowControlBar(cmainframe::activewnd, &cmainframe::activewnd->m_wndToolBar, vtable->IsVisible(&cmainframe::activewnd->m_wndToolBar) ? 0 : 1, 1);
		}*/

		/*ImGui::SameLine();
		if (ImGui::Button("Toggle Menubar"))
		{
			if (!ggui::mainframe_menubar_enabled)
			{
				components::command::execute("menubar_show");
			}
			else
			{
				components::command::execute("menubar_hide");
			}

			game::CPrefsDlg_SavePrefs();
		}*/

		//ImGui::SameLine();
		//if (ImGui::Button("Reload Commandmap"))
		//{
		//	// CMainFrame::LoadCommandMap
		//	cdeclcall(void, 0x421230);
		//}

	//END_GUI:
		ImGui::PopStyleColor(_stylecolors);
		ImGui::PopStyleVar(_stylevars);
		ImGui::End();
	}
}
