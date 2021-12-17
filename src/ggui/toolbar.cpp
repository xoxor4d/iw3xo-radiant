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

			const ImVec4 bg_col = hovered_state ?
				ImGui::ToImVec4(dvars::gui_toolbar_button_hovered_color->current.vector) : ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);
			
			if (ImGui::ImageButton(image->texture.data, IMAGEBUTTON_SIZE, uv0, uv1, 0, bg_col))
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

	bool image_togglebutton(const char* image_name, bool& hovered_state, bool toggle_state, const char* tooltip, ImVec4* bg_col, ImVec4* bg_col_hovered, ImVec4* bg_col_active, ImVec2* btn_size)
	{
		ImVec2 button_size				= IMAGEBUTTON_SIZE;
		ImVec4 background_color			= ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);
		ImVec4 background_color_hovered = ImGui::ToImVec4(dvars::gui_toolbar_button_hovered_color->current.vector);
		ImVec4 background_color_active	= ImGui::ToImVec4(dvars::gui_toolbar_button_active_color->current.vector);

		if (btn_size)		button_size					= *btn_size;
		
		if (bg_col)			background_color			= *bg_col;
		if (bg_col_hovered) background_color_hovered	= *bg_col_hovered;
		if (bg_col_active)	background_color_active		= *bg_col_active;

		bool ret_state = false;

		if (const auto	image = game::Image_RegisterHandle(image_name);
						image && image->texture.data)
		{
			const ImVec2 uv0 = toggle_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = toggle_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImVec4 color = hovered_state ? background_color_hovered
						: toggle_state ? background_color_active : background_color;
			
			if (ImGui::ImageButton(image->texture.data, button_size, uv0, uv1, 0, color)) {
				ret_state = true;
			}

			if (tooltip) {
				TT(tooltip);
			}

			hovered_state = ImGui::IsItemHovered();
		}
		else
		{
			if (ImGui::Button(image_name)) {
				ret_state = true;
			}

			if (tooltip) {
				TT(tooltip);
			}
		}
		
		return ret_state;
	}

	
	// *
	// 

	
	bool toolbar_initiated = false;
	int tb_element_id = 0;
	
	struct tb_order_element_s
	{
		std::string name;
		int			id;
		bool		visible;
		bool		is_separator;
		bool		debug;
	};

	struct tb_selection_s
	{
		bool is_selected;
		uint32_t element_id;
		uint32_t element_pos;
	};
	
	nlohmann::fifo_map<std::string, std::function<void()>> toolbar_registered_elements_callbacks;
	std::vector<tb_order_element_s> toolbar_registered_elements;

	int toolbar_sorted_element_id = 0;
	std::vector<tb_order_element_s> toolbar_sorted_list;

	tb_selection_s tbedit_selection = {};


	void register_element(const std::string& name, bool default_visible, std::function<void()> callback)
	{
		bool is_debug = false;
		toolbar_registered_elements_callbacks[name] = callback;

		// if separator
		if (utils::starts_with(name, ";"s))
		{
			// insert before selection
			if (tbedit_selection.is_selected)
			{
				toolbar_registered_elements.insert(
					toolbar_registered_elements.begin() + tbedit_selection.element_pos,
					tb_order_element_s("; ------------", tb_element_id++, default_visible, true, is_debug));
			}
			// emplace back if nothing is selected
			else
			{
				toolbar_registered_elements.emplace_back(
					tb_order_element_s("; ------------", tb_element_id++, default_visible, true, is_debug));
			}

			return;
		}

		if (utils::starts_with(name, "debug_"s))
		{
			is_debug = true;
		}

		toolbar_registered_elements.emplace_back(
			tb_order_element_s(name, tb_element_id++, default_visible, false, is_debug));
	}
	
	void register_element(const std::string& name, std::function<void()> callback)
	{
		bool is_debug = false;
		toolbar_registered_elements_callbacks[name] = callback;

		// if separator
		if (utils::starts_with(name, ";"s))
		{
			if(!toolbar_initiated)
			{
				// insert before selection
				if (tbedit_selection.is_selected)
				{
					toolbar_registered_elements.insert(
						toolbar_registered_elements.begin() + tbedit_selection.element_pos,
						tb_order_element_s("; ------------", tb_element_id++, true, true, is_debug));
				}
				// emplace back if nothing is selected
				else
				{
					toolbar_registered_elements.emplace_back(
						tb_order_element_s("; ------------", tb_element_id++, true, true, is_debug));
				}
			}
			else
			{
				// insert before selection
				if (tbedit_selection.is_selected)
				{
					toolbar_sorted_list.insert(
						toolbar_sorted_list.begin() + tbedit_selection.element_pos,
						tb_order_element_s("; ------------", toolbar_sorted_element_id++, true, true, is_debug));
				}
				// emplace back if nothing is selected
				else
				{
					toolbar_sorted_list.emplace_back(
						tb_order_element_s("; ------------", toolbar_sorted_element_id++, true, true, is_debug));
				}
			}
			

			return;
		}
		
		if(utils::starts_with(name, "debug_"s))
		{
			is_debug = true;
		}

		toolbar_registered_elements.emplace_back(
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
				static bool hov_clipper;
			
				// CMainFrame::OnViewClipper
				if (image_togglebutton("clipper", hov_clipper, game::g_bClipMode, "Toggle Clipper")) {
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

		register_element("camera_movement"s, false, []()
			{
				static bool hov_camera_movement;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnToggleCameraMovementMode
				ImGui::BeginGroup();
				{
					ImVec2 prebutton_cursor = ImGui::GetCursorScreenPos();

					if (image_togglebutton("camera_movement", hov_camera_movement, prefs->camera_mode,
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

		register_element("cubic_clip"s, false, []()
			{
				static bool hov_cubicclip;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnViewCubicclipping
				if (image_togglebutton("cubic_clip", hov_cubicclip, prefs->m_bCubicClipping,
					std::string("Cubic Clipping [" + hotkeys::get_hotkey_for_command("ToggleCubicClip") + "]").c_str()))
				{
					mainframe_thiscall(LRESULT, 0x428F90);
				}
			});

		register_element("gameview"s, false, []()
			{
				static bool hov_gameview;
				if (image_togglebutton("gameview", hov_gameview, dvars::radiant_gameview->current.enabled,
					std::string("Gameview [" + hotkeys::get_hotkey_for_command("xo_gameview") + "]").c_str()))
				{
					components::gameview::p_this->set_state(!dvars::radiant_gameview->current.enabled);
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
				static bool hov_plantmodel;
				const auto prefs = game::g_PrefsDlg();

				// CMainFrame::OnPlantModel
				if (image_togglebutton("plant_models", hov_plantmodel, prefs->m_bDropModel,
					"Plant models and apply random scale and rotation"))
				{
					mainframe_thiscall(LRESULT, 0x42A0E0);
				}
			});

		register_element("plant_orient_to_floor"s, []()
			{
				static bool hov_plantorient;
				const auto prefs = game::g_PrefsDlg();

				// CMainFrame::OnPlantModel
				if (image_togglebutton("plant_orient_to_floor", hov_plantorient, prefs->m_bOrientModel,
					"Orient dropped selection to the floor"))
				{
					mainframe_thiscall(LRESULT, 0x4258F0);
				}
			});

		register_element("plant_force_drop_height"s, []()
			{
				static bool hov_plantdrop;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnForceZeroDropHeight
				if (image_togglebutton("plant_force_drop_height", hov_plantdrop, prefs->m_bForceZeroDropHeight,
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

		register_element("guizmo_enable"s, false, []()
			{
				static bool hov_guizmo_enable;
				if (image_togglebutton("guizmo_enable", hov_guizmo_enable, dvars::guizmo_enable->current.enabled, dvars::guizmo_enable->description))
				{
					dvars::set_bool(dvars::guizmo_enable, !dvars::guizmo_enable->current.enabled);
				}
			});

		register_element("guizmo_grid_snapping"s, false, []()
			{
				ImGui::BeginDisabled(!dvars::guizmo_enable->current.enabled);
				{
					static bool hov_guizmo_grid_snapping;
					if (image_togglebutton("guizmo_grid_snapping", hov_guizmo_grid_snapping, dvars::guizmo_snapping->current.enabled, dvars::guizmo_snapping->description))
					{
						dvars::set_bool(dvars::guizmo_snapping, !dvars::guizmo_snapping->current.enabled);
					}
				}
				ImGui::EndDisabled();
			});

		register_element("guizmo_brush_mode"s, false, []()
			{
				ImGui::BeginDisabled(!dvars::guizmo_enable->current.enabled);
				{
					static bool hov_guizmo_brush_mode;
					if (image_togglebutton("guizmo_brush_mode", hov_guizmo_brush_mode, dvars::guizmo_brush_mode->current.enabled, dvars::guizmo_brush_mode->description))
					{
						dvars::set_bool(dvars::guizmo_brush_mode, !dvars::guizmo_brush_mode->current.enabled);
					}
				}
				ImGui::EndDisabled();
			});
		
		register_element("free_rotate"s, []()
			{
				static bool hov_freerotate;
			
				// CMainFrame::OnSelectMouserotate
				if (image_togglebutton("free_rotate", hov_freerotate, game::g_bRotateMode,
					std::string("Free rotation [" + hotkeys::get_hotkey_for_command("MouseRotate") + "]").c_str()))
				{
					mainframe_thiscall(LRESULT, 0x428570);
				}
			});

		register_element("free_scale"s, []()
			{
				static bool hov_freescale;
			
				// CMainFrame::OnSelectMousescale
				if (image_togglebutton("free_scale", hov_freescale, game::g_bScaleMode,
					"Free scaling"))
				{
					mainframe_thiscall(LRESULT, 0x428D20);
				}
			});

		register_element("lock_x"s, []()
			{
				static bool hov_lockx;
			
				// CMainFrame::OnScalelockX
				if (image_togglebutton("lock_x", hov_lockx,
					(game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6),
					"Lock grid along the x-axis"))
				{
					mainframe_thiscall(LRESULT, 0x428BC0);
				}
			});

		register_element("lock_y"s, []()
			{
				static bool hov_locky;
			
				// CMainFrame::OnScalelockY
				if (image_togglebutton("lock_y", hov_locky,
					(game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5),
					"Lock grid along the y-axis"))
				{
					mainframe_thiscall(LRESULT, 0x428B60);
				}
			});

		register_element("lock_z"s, []()
			{
				static bool hov_lockz;
			
				// CMainFrame::OnScalelockZ
				if (image_togglebutton("lock_z", hov_lockz,
					(game::g_nScaleHow > 0 && game::g_nScaleHow <= 3),
					"Lock grid along the z-axis"))
				{
					mainframe_thiscall(LRESULT, 0x428B90);
				}
			});

		register_element(";"s, nullptr);

		register_element("show_patches_as"s, []()
			{
				static bool hov_patches_as;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnPatchWireframe
				ImGui::BeginGroup();
				{
					ImVec2 prebutton_cursor = ImGui::GetCursorScreenPos();

					if (image_togglebutton("show_patches_as", hov_patches_as, prefs->g_nPatchAsWireframe,
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
				static bool hov_weld_patches_move;
				const auto prefs = game::g_PrefsDlg();

				// CMainFrame::OnPatchWeld
				if (image_togglebutton("weld_equal_patches_move", hov_weld_patches_move,
					prefs->g_bPatchWeld,
					"Weld equal patch points during moves"))
				{
					mainframe_thiscall(LRESULT, 0x42A400);
				}
			});

		register_element("select_drill_down_vertices"s, []()
			{
				static bool hov_drill_down;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnPatchDrilldown
				if (image_togglebutton("select_drill_down_vertices", hov_drill_down,
					prefs->patch_drill_down,
					"Select invisible vertices (drill down rows/columns)"))
				{
					mainframe_thiscall(LRESULT, 0x42A510);
				}
			});
		
		register_element("toggle_lock_vertices_mode"s, []()
			{
				static bool hov_lock_vertices;
			
				// CMainFrame::ToggleLockPatchVertMode
				if (image_togglebutton("toggle_lock_vertices_mode", hov_lock_vertices,
					game::g_qeglobals->bLockPatchVerts,
					"Toggle lock-vertex mode"))
				{
					mainframe_thiscall(LRESULT, 0x42B4F0);
				}
			});

		register_element("toggle_unlock_vertices_mode"s, []()
			{
				static bool hov_unlock_vertices;
			
				// CMainFrame::ToggleUnlockPatchVertMode
				if (image_togglebutton("toggle_unlock_vertices_mode", hov_unlock_vertices,
					game::g_qeglobals->bUnlockPatchVerts,
					"Toggle unlock-vertex mode"))
				{
					mainframe_thiscall(LRESULT, 0x42B510);
				}
			});

		register_element("cycle_patch_edge_direction"s, []()
			{
				static bool hov_cycle_edges;
			
				// CMainFrame::OnCycleTerrainEdge
				if (image_togglebutton("cycle_patch_edge_direction", hov_cycle_edges,
					game::g_qeglobals->d_select_mode == 9,
					"Toggle terrain-quad edge cycle mode"))
				{
					mainframe_thiscall(LRESULT, 0x42B530);
				}
			});

		register_element("tolerant_weld"s, []()
			{
				static bool hov_tolerant_weld;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnTolerantWeld
				if (image_togglebutton("tolerant_weld", hov_tolerant_weld,
					prefs->m_bTolerantWeld,
					"Toggle tolerant weld / Draw tolerant weld lines"))
				{
					mainframe_thiscall(LRESULT, 0x42A130);
				}
			});

		register_element(";"s, nullptr);

		register_element("toggle_draw_surfs_portal"s, []()
			{
				static bool hov_drawsurfs_portal;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnToggleDrawSurfs
				if (image_togglebutton("toggle_draw_surfs_portal", hov_drawsurfs_portal,
					prefs->draw_toggle,
					"Toggle drawing of portal no-draw surfaces"))
				{
					mainframe_thiscall(LRESULT, 0x42A040);
				}
			});

		register_element("dont_select_curve"s, []()
			{
				static bool hov_select_curve;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnDontselectcurve
				if (image_togglebutton("dont_select_curve", hov_select_curve,
					!prefs->m_bSelectCurves,
					"Disable selection of patches"))
				{
					mainframe_thiscall(LRESULT, 0x429920);
				}
			});

		register_element("dont_select_entities"s, []()
			{
				static bool hov_select_entities;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnDisableSelectionOfEntities
				if (image_togglebutton("dont_select_entities", hov_select_entities,
					prefs->entities_off,
					"Disable selection of entities"))
				{
					mainframe_thiscall(LRESULT, 0x429F60);
				}
			});

		register_element("dont_select_sky"s, []()
			{
				static bool hov_dont_sel_sky;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnDisableSelectionOfSky
				if (image_togglebutton("dont_select_sky", hov_dont_sel_sky,
					prefs->sky_brush_off,
					"Disable selection of sky brushes"))
				{
					mainframe_thiscall(LRESULT, 0x429FB0);
				}
			});

		register_element("dont_select_models"s, []()
			{
				static bool hov_dont_sel_models;
				const auto prefs = game::g_PrefsDlg();
			
				// CMainFrame::OnSelectableModels
				if (image_togglebutton("dont_select_models", hov_dont_sel_models,
					prefs->m_bSelectableModels,
					"Disable selection of static models"))
				{
					mainframe_thiscall(LRESULT, 0x42A280);
				}
			});

		register_element("lightpreview"s, false, []()
			{
				static bool hov_lightpreview;
				const auto prefs = game::g_PrefsDlg();

				if (image_togglebutton("lightpreview", hov_lightpreview, prefs->enable_light_preview, std::string("Lightpreview [" + ggui::hotkeys::get_hotkey_for_command("LightPreviewToggle") + "]").c_str()))
				{
					mainframe_thiscall(void, 0x4240C0); // cmainframe::OnEnableLightPreview
				}
			});

		register_element("sunpreview"s, false, []()
			{
				static bool hov_sunpreview;
				const auto prefs = game::g_PrefsDlg();

				if (image_togglebutton("sunpreview", hov_sunpreview, prefs->preview_sun_aswell, std::string("Sunpreview [" + ggui::hotkeys::get_hotkey_for_command("LightPreviewSun") + "]\nNeeds Lightpreview!").c_str()))
				{
					mainframe_thiscall(void, 0x424060); // cmainframe::OnPreviewSun;
				}
			});

#ifdef DEBUG
		register_element(";"s, nullptr);
		
		register_element("debug_reload_images"s, []()
			{
				if (ImGui::Button("RI", IMAGEBUTTON_SIZE)) {
					game::R_ReloadImages();
				} TT("DBG: Reload Images");
			});
#endif
	}
	
	void load_settings_ini()
	{
		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
			fs_homepath)
		{
			bool no_ini_or_empty = false;
			
			std::ifstream ini;
			std::string ini_path = fs_homepath->current.string;
						ini_path += "\\" + INI_FILENAME;
	
			ini.open(ini_path.c_str());
			if (!ini.is_open())
			{
				game::printf_to_console("[Toolbar] Failed to open ini: \"%s\"", ini_path.c_str());
				no_ini_or_empty = true;
			}
			
			// read ini line by line
			if(!no_ini_or_empty)
			{
				std::string input;
				int line = 0;
				
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
						toolbar_sorted_list.emplace_back(
							tb_order_element_s("; ------------", toolbar_sorted_element_id++, true, true, false));

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
					if (toolbar_registered_elements_callbacks.find(args[0]) != toolbar_registered_elements_callbacks.end())
					{
						toolbar_sorted_list.emplace_back(
							tb_order_element_s(args[0], toolbar_sorted_element_id++, args[1] != "0"s, false));
					}
					else
					{
						game::printf_to_console("[Toolbar] not a valid element @ line #%d (\"%s\")\n", line, input.c_str());
					}
				}

				if (!line)
				{
					no_ini_or_empty = true;
				}
			}

			// loop all registered elements to add all elements not found inside the ini (eg. new elements after an update)
			for (const auto& element : toolbar_registered_elements)
			{
				bool found = false;
				bool is_debug = false;

				// add seperators if no ini or ini was empty
				// do not add separators otherwise
				if(!no_ini_or_empty)
				{
					if (utils::starts_with(element.name, ";"s))
					{
						continue;
					}
				}

				if (utils::starts_with(element.name, "debug_"s))
				{
					is_debug = true;
				}

				for (uint32_t e = 0; e < toolbar_sorted_list.size(); e++)
				{
					if (element.name == toolbar_sorted_list[e].name)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					toolbar_sorted_list.emplace_back(
						tb_order_element_s(element.name, toolbar_sorted_element_id++, element.visible, false, is_debug));
				}
			}
		}
	}

	
	void save_settings_ini()
	{
		if (toolbar_sorted_list.empty())
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

			for (const auto& element : toolbar_sorted_list)
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
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		
		ImGui::Begin("Toolbar Editor##window", &menu.menustate, ImGuiWindowFlags_NoCollapse);
		
		if (ImGui::Button("Add Separator"))
		{
			register_element(";"s, nullptr);
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Selected Separator"))
		{
			const bool out_of_bounds = tbedit_selection.element_pos >= toolbar_sorted_list.size();

			if (tbedit_selection.is_selected 
				&& !out_of_bounds 
				&& toolbar_sorted_list[tbedit_selection.element_pos].is_separator)
			{
				game::printf_to_console("selected sep, deleting ...\n");
				toolbar_sorted_list.erase(toolbar_sorted_list.begin() + tbedit_selection.element_pos);
			}
			else if (out_of_bounds)
			{
				tbedit_selection.is_selected = false;
				game::printf_to_console("out of bounds!\n");
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
			for (uint32_t e = 0; e < toolbar_sorted_list.size(); e++)
			{
				const auto element = toolbar_sorted_list[e];

				// check if the element was registered
				if (element.is_separator || toolbar_registered_elements_callbacks.find(toolbar_sorted_list[e].name) != toolbar_registered_elements_callbacks.end())
				{
					ImGui::TableNextRow();

					for (uint32_t column = 0; column < 3; column++)
					{
						ImGui::TableNextColumn();
						switch (column)
						{
						case 0:
							if (!element.is_separator && !element.debug)
							{
								if (const auto image = game::Image_RegisterHandle(element.name.c_str());
									image && image->texture.data)
								{
									const ImVec2 uv0 = ImVec2(0.0f, 0.0f);
									const ImVec2 uv1 = ImVec2(0.5f, 1.0f);

									ImGui::Image(image->texture.data, IMAGEBUTTON_SIZE, uv0, uv1);
								}

								// placeholder image when icon is missing
								else if (const auto temp_image = game::Image_RegisterHandle("cycle_xyz");
									temp_image&& temp_image->texture.data)
								{
									const ImVec2 uv0 = ImVec2(0.5f, 0.0f);
									const ImVec2 uv1 = ImVec2(1.0f, 1.0f);

									ImGui::Image(temp_image->texture.data, IMAGEBUTTON_SIZE, uv0, uv1);
								}
							}

							break;

						case 1:
							if (element.is_separator)
							{
								// unique id for each separator
								if (ImGui::Selectable(
									utils::va("------------##%d", toolbar_sorted_list[e].id),
									e == tbedit_selection.element_pos && tbedit_selection.is_selected,
									ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, 
									ImVec2(0.0f, IMAGEBUTTON_SIZE.y)))
								{
									tbedit_selection.is_selected = true;
									tbedit_selection.element_id = toolbar_sorted_list[e].id;
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
									tbedit_selection.element_id = toolbar_sorted_list[e].id;
									tbedit_selection.element_pos = e;
								}
							}
							
							if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
							{
								int n_next = e + (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
								if (n_next >= 0 && static_cast<uint32_t>(n_next) < toolbar_sorted_list.size())
								{
									const auto temp_pair = toolbar_sorted_list[n_next];

									toolbar_sorted_list[n_next] = toolbar_sorted_list[e];
									toolbar_sorted_list[e] = temp_pair;

									ImGui::ResetMouseDragDelta();

									tbedit_selection.is_selected = true;
									tbedit_selection.element_id = toolbar_sorted_list[n_next].id;
									tbedit_selection.element_pos = n_next;
								}
							}
							break;

						case 2:
							if (!element.is_separator)
							{
								ImGui::PushID(widget_id_num); widget_id_num++;
								ImGui::Checkbox("##checkbox", &toolbar_sorted_list[e].visible);
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

	void menu_new([[maybe_unused]] ggui::imgui_context_menu& toolbar, ggui::imgui_context_menu& toolbar_edit)
	{
		const bool TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED = true;

		int _stylevars = 0;
		int _stylecolors = 0;

		// request auto-sizing on one axis
		const ImVec2 requested_size = (ggui::toolbar_axis == ImGuiAxis_X) ? ImVec2(-1.0f, 0.0f) : ImVec2(0.0f, -1.0f);

		ImGui::SetNextWindowSize(requested_size);

		const bool is_floating_toolbar = dvars::gui_floating_toolbar && dvars::gui_floating_toolbar->current.enabled;
		
		if(is_floating_toolbar)
		{
			ImGui::SetNextWindowPos(ImVec2(5.0f, ggui::menubar_height + 5.0f), ImGuiCond_FirstUseEver);
		}

		if(ggui::toolbar_reset)
		{
			if(!is_floating_toolbar)
			{
				if (ggui::toolbar_axis == ImGuiAxis_X)
				{
					ImGui::SetNextWindowDockID(ggui::toolbar_dock_top);
				}
				else
				{
					ImGui::SetNextWindowDockID(ggui::toolbar_dock_left);
				}
			}

			// reset to default floating state
			else
			{
				ImGui::SetNextWindowPos(ImVec2(5.0f, ggui::menubar_height + 5.0f));
			}

			ggui::toolbar_reset = false;
		}

		ImGuiWindowClass window_class;
		window_class.DockingAllowUnclassed = true;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoCloseButton;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingSplitMe;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverMe;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverOther;

		if (ggui::toolbar_axis == ImGuiAxis_X)
		{
			window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResizeY;
		}
		else
		{
			window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResizeX;
		}
			
		ImGui::SetNextWindowClass(&window_class);

		
		// *
		// begin into the window

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ToImVec4(dvars::gui_toolbar_bg_color->current.vector));	 _stylecolors++;
		//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TabUnfocusedActive)); _stylecolors++;

		ImGui::Begin("toolbar##window", nullptr, 
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

		// *
		// overwrite node size
		
		ImGuiDockNode* node = ImGui::GetWindowDockNode();
		if (node != nullptr)
		{
			// Overwrite size of the node
			ImGuiStyle& style = ImGui::GetStyle();
			
			const ImGuiAxis toolbar_axis_perp = (ImGuiAxis)(ggui::toolbar_axis ^ 1);
			const float TOOLBAR_SIZE_WHEN_DOCKED = style.WindowPadding[toolbar_axis_perp] * 2.0f + IMAGEBUTTON_SIZE[toolbar_axis_perp];
			
			node->WantLockSizeOnce = true;
			node->Size[toolbar_axis_perp] = node->SizeRef[toolbar_axis_perp] = TOOLBAR_SIZE_WHEN_DOCKED;

			if (TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
			{
				if (node->ParentNode && node->ParentNode->SplitAxis != ImGuiAxis_None)
				{
					ggui::toolbar_axis = (ImGuiAxis)(node->ParentNode->SplitAxis ^ 1);
				}
			}	
		}

		
		// *
		// initialize toolbar elements
		if (!toolbar_initiated)
		{
			toolbar_elements_init();
			load_settings_ini();
			
			toolbar_initiated = true;
		}
		

		// *
		// populate toolbar
		
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);							_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.0f, 4.0f));		_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));	_stylevars++;

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(70, 70, 70, 70));		_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));		_stylecolors++;

		// loop amount of registered-sorted elements
		for (uint32_t e = 0; e < toolbar_sorted_list.size(); e++)
		{
			if (toolbar_sorted_list[e].is_separator)
			{
				if (ggui::toolbar_axis == ImGuiAxis_X)
				{
					TB_SEPARATOR;
				}
				else
				{
					SPACING(0.0f, 2.0f);
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
					SPACING(0.0f, 2.0f);
				}
				
				continue;
			}

			// check if the element was registered
			if (auto element  = toolbar_registered_elements_callbacks.find(toolbar_sorted_list[e].name);
					 element != toolbar_registered_elements_callbacks.end())
			{
				if (toolbar_sorted_list[e].visible)
				{
					element->second();

					if (ggui::toolbar_axis == ImGuiAxis_X)
					{
						ImGui::SameLine();
					}
				}
			}
		}

		ImGui::PopStyleColor(_stylecolors); _stylecolors = 0;
		ImGui::PopStyleVar(_stylevars); _stylevars = 0;


		ggui::toolbar_pos = ImGui::GetWindowPos();
		ggui::toolbar_size = ImGui::GetWindowSize();

		
		// *
		// context (right click) menu
		
		if (ImGui::BeginPopupContextWindow())
		{
			// only enable horz/vert options when undocked
			if (   node == nullptr 
				|| node && node->ParentNode == nullptr 
				|| !TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
			{
				if (ImGui::MenuItem("Horizontal", "", (ggui::toolbar_axis == ImGuiAxis_X)))
				{
					ggui::toolbar_axis = ImGuiAxis_X;
				}
					
				if (ImGui::MenuItem("Vertical", "", (ggui::toolbar_axis == ImGuiAxis_Y)))
				{
					ggui::toolbar_axis = ImGuiAxis_Y;
				}
			}
			
			if (ImGui::MenuItem("Edit Toolbar ...")) 
			{
				components::gui::toggle(toolbar_edit, 0, true);
			}
			
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void register_dvars()
	{
		
	}
}
