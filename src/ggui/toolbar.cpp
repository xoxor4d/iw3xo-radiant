#include "std_include.hpp"

#define TB_SEPARATOR											\
	ImGui::SameLine(); SPACING(2.0f, 0.0f); ImGui::SameLine();	\
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);			\
	ImGui::SameLine(); SPACING(4.0f, 0.0f); ImGui::SameLine();	\

namespace ggui
{
	const ImVec2		TB_IMAGEBUTTON_SIZE = ImVec2(28.0f, 28.0f);
	const std::string	TB_INI_FILENAME = "toolbar.ini"s;

	bool toolbar_dialog::image_button(const char* image_name, bool& hovered_state, const char* tooltip)
	{
		bool ret_state = false;

		if (const auto	image = game::Image_RegisterHandle(image_name);
						image && image->texture.data)
		{
			const ImVec2 uv0 = hovered_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = hovered_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			const ImVec4 bg_col = hovered_state ?
				ImGui::ToImVec4(dvars::gui_toolbar_button_hovered_color->current.vector) : ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);

			if (ImGui::ImageButton(image->texture.data, TB_IMAGEBUTTON_SIZE, uv0, uv1, 0, bg_col))
			{
				ret_state = true;
			}

			if (tooltip)
			{
				TT(tooltip);
			}

			hovered_state = ImGui::IsItemHovered();
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

		return ret_state;
	}

	void toolbar_dialog::image_button(const char* image_name, bool& hovered_state, E_CALLTYPE calltype, uint32_t func_addr, const char* tooltip)
	{
		if (const auto	image = game::Image_RegisterHandle(image_name);
						image && image->texture.data)
		{
			const ImVec2 uv0 = hovered_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = hovered_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			const ImVec4 bg_col = hovered_state ?
				ImGui::ToImVec4(dvars::gui_toolbar_button_hovered_color->current.vector) : ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);

			if (ImGui::ImageButton(image->texture.data, TB_IMAGEBUTTON_SIZE, uv0, uv1, 0, bg_col))
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

			hovered_state = ImGui::IsItemHovered();
		}
		else
		{
			if (ImGui::Button(image_name))
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

	bool toolbar_dialog::image_button_label(const char* label, const char* image_name, bool image_uv, bool& hovered_state_bg, const char* tooltip, const ImVec4* bg_col_hovered, const ImVec4* bg_col_active, const ImVec2* btn_size, float zoom)
	{
		bool ret_state = false;

		if (const auto	image = game::Image_RegisterHandle(image_name);
						image && image->texture.data)
		{
			const ImVec2 zoom_factor = ImVec2(1.0f - zoom, 1.0f - zoom) * 0.5f;
			const ImVec2 uv0 = (image_uv ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f)) + zoom_factor;
			const ImVec2 uv1 = (image_uv ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f)) - zoom_factor;

			const ImVec4 bg_col = hovered_state_bg ? *bg_col_hovered : *bg_col_active;

			ImGui::BeginGroup();
			{
				if(label && !utils::starts_with(label, "##"))
				{
					const float cursor_y = ImGui::GetCursorPos().y;
					ImGui::SetCursorPosY(cursor_y - 4.0f);
					ImGui::TextUnformatted(label);

					ImGui::SameLine(0, 2.0f);
					ImGui::SetCursorPosY(cursor_y);
				}

				ImGui::PushID(label);
				if (ImGui::ImageButton(image->texture.data, *btn_size, uv0, uv1, 0, bg_col))
				{
					ret_state = true;
				}
				ImGui::PopID();

				hovered_state_bg = ImGui::IsItemHovered();

				ImGui::EndGroup();
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

		return ret_state;
	}

	bool toolbar_dialog::image_togglebutton(const char* image_name, bool& hovered_state, bool toggle_state, const char* tooltip, ImVec4* bg_col, ImVec4* bg_col_hovered, ImVec4* bg_col_active, ImVec2* btn_size)
	{
		ImVec2 button_size = TB_IMAGEBUTTON_SIZE;
		ImVec4 background_color = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);
		ImVec4 background_color_hovered = ImGui::ToImVec4(dvars::gui_toolbar_button_hovered_color->current.vector);
		ImVec4 background_color_active = ImGui::ToImVec4(dvars::gui_toolbar_button_active_color->current.vector);

		if (btn_size) button_size = *btn_size;
		if (bg_col)	background_color = *bg_col;
		if (bg_col_hovered) background_color_hovered = *bg_col_hovered;
		if (bg_col_active) background_color_active = *bg_col_active;

		bool ret_state = false;

		if (const auto	image = game::Image_RegisterHandle(image_name);
						image && image->texture.data)
		{
			const ImVec2 uv0 = toggle_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = toggle_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			const ImVec4 color  = hovered_state ? background_color_hovered
								: toggle_state ? background_color_active : background_color;

			if (ImGui::ImageButton(image->texture.data, button_size, uv0, uv1, 0, color)) 
			{
				ret_state = true;
			}

			if (tooltip) 
			{
				TT(tooltip);
			}

			hovered_state = ImGui::IsItemHovered();
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

		return ret_state;
	}

	void toolbar_dialog::register_element(const std::string& name, bool default_visible, std::function<void()> callback)
	{
		const auto tbedit = GET_GUI(ggui::toolbar_edit_dialog);

		bool is_debug = false;
		m_registered_elements_callbacks[name] = callback;

		// if separator
		if (utils::starts_with(name, ";"s))
		{
			// insert before selection
			if (tbedit->tbedit_selection.is_selected)
			{
				m_registered_elements.insert(
					m_registered_elements.begin() + tbedit->tbedit_selection.element_pos,
					tb_order_element_s("; ------------", m_element_id++, default_visible, true, is_debug));
			}
			// emplace back if nothing is selected
			else
			{
				m_registered_elements.emplace_back(
					tb_order_element_s("; ------------", m_element_id++, default_visible, true, is_debug));
			}

			return;
		}

		if (utils::starts_with(name, "debug_"s))
		{
			is_debug = true;
		}

		m_registered_elements.emplace_back(
			tb_order_element_s(name, m_element_id++, default_visible, false, is_debug));
	}

	void toolbar_dialog::register_element(const std::string& name, std::function<void()> callback)
	{
		const auto tbedit = GET_GUI(ggui::toolbar_edit_dialog);

		bool is_debug = false;
		m_registered_elements_callbacks[name] = callback;

		// if separator
		if (utils::starts_with(name, ";"s))
		{
			if(!this->is_initiated())
			{
				// insert before selection
				if (tbedit->tbedit_selection.is_selected)
				{
					m_registered_elements.insert(
						m_registered_elements.begin() + tbedit->tbedit_selection.element_pos,
						tb_order_element_s("; ------------", m_element_id++, true, true, is_debug));
				}
				// emplace back if nothing is selected
				else
				{
					m_registered_elements.emplace_back(
						tb_order_element_s("; ------------", m_element_id++, true, true, is_debug));
				}
			}
			else
			{
				// insert before selection
				if (tbedit->tbedit_selection.is_selected)
				{
					m_sorted_elements.insert(
						m_sorted_elements.begin() + tbedit->tbedit_selection.element_pos,
						tb_order_element_s("; ------------", m_sorted_element_id++, true, true, is_debug));
				}
				// emplace back if nothing is selected
				else
				{
					m_sorted_elements.emplace_back(
						tb_order_element_s("; ------------", m_sorted_element_id++, true, true, is_debug));
				}
			}


			return;
		}

		if (utils::starts_with(name, "debug_"s))
		{
			is_debug = true;
		}

		m_registered_elements.emplace_back(
			tb_order_element_s(name, m_element_id++, true, false, is_debug));
	}

	// *
	// all toolbar elements are registered here (default sort if no ini is present)
	void toolbar_dialog::toolbar_elements_init()
	{
		register_element("open"s, []()
			{
				static bool hov_open;

				// logic :: ggui::file_dialog_frame
				if (dvars::gui_use_new_filedialog->current.enabled)
				{
					if (image_button("open", hov_open, std::string("Open File " + ggui::hotkey_dialog::get_hotkey_for_command("FileOpen")).c_str()))
					{
						const auto egui = GET_GUI(ggui::entity_dialog);
						const std::string path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");

						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(ggui::FILE_DIALOG_HANDLER::MAP_LOAD);
						file->set_file_op_type(file_dialog::FileDialogType::OpenFile);
						file->set_file_ext(".map");
						file->open();
					}
				}
				else
				{
					//CMainFrame::OnFileOpen
					image_button("open", hov_open, MAINFRAME_CDECL, 0x423AE0, std::string("Open File " + ggui::hotkey_dialog::get_hotkey_for_command("FileOpen")).c_str());
				}
			});

		register_element("save"s, []()
			{
				static bool hov_save;

				//CMainFrame::OnFileSave
				image_button("save", hov_save, MAINFRAME_CDECL, 0x423B80, std::string("Save File " + ggui::hotkey_dialog::get_hotkey_for_command("FileSave")).c_str());
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
				image_button("rotate_z", hov_rotz, CDECLCALL, 0x425220, std::string("Rotate Brush Z-Axis " + ggui::hotkey_dialog::get_hotkey_for_command("RotateZ")).c_str());
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
				image_button("csg_merge", hov_csg_merge, CDECLCALL, 0x4255D0, std::string("CSG Merge " + ggui::hotkey_dialog::get_hotkey_for_command("CSGMerge")).c_str());
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
				image_button("cycle_layer", hov_cycle_layer, CDECLCALL, 0x424010, std::string("Cycle Texture Layer " + ggui::hotkey_dialog::get_hotkey_for_command("TexLayerCycle")).c_str());
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
						prebutton_cursor.x += (TB_IMAGEBUTTON_SIZE.x * 0.45f);
						prebutton_cursor.y += (TB_IMAGEBUTTON_SIZE.y * 0.4f);

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
					std::string("Cubic Clipping" + ggui::hotkey_dialog::get_hotkey_for_command("ToggleCubicClip")).c_str()))
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
					image_button("empty_hover", hov_cycle_xyz, MAINFRAME_THIS, 0x426DB0, std::string("Cycle Grid Window View " + ggui::hotkey_dialog::get_hotkey_for_command("NextView")).c_str());

					ImGui::PushFontFromIndex(REGULAR_18PX);
					const auto viewtype = cmainframe::activewnd->m_pXYWnd->m_nViewType;

					const char* curr_view_str = viewtype == 0 ? "YZ" : viewtype == 1 ? "XZ" : "XY";
					const auto text_size = ImGui::CalcTextSize(curr_view_str);

					prebutton_cursor.x += (TB_IMAGEBUTTON_SIZE.x * 0.5f - (text_size.x * 0.5f));
					prebutton_cursor.y += (TB_IMAGEBUTTON_SIZE.y * 0.5f - (text_size.y * 0.5f));

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

				// CMainFrame::OnOrientToFloor
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

		register_element("free_rotate"s, []()
			{
				static bool hov_freerotate;

				// CMainFrame::OnSelectMouserotate
				if (image_togglebutton("free_rotate", hov_freerotate, game::g_bRotateMode,
					std::string("Free rotation " + ggui::hotkey_dialog::get_hotkey_for_command("MouseRotate")).c_str()))
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
						prebutton_cursor.x += (TB_IMAGEBUTTON_SIZE.x * 0.45f);
						prebutton_cursor.y += (TB_IMAGEBUTTON_SIZE.y * 0.45f);

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
					std::string("Redisperse Patch Points " + ggui::hotkey_dialog::get_hotkey_for_command("RedisperseVertices")).c_str());
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
					mainframe_thiscall(LRESULT, 0x42B530); // CMainFrame::OnCycleTerrainEdge
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
					mainframe_thiscall(LRESULT, 0x42A130); // CMainFrame::OnTolerantWeld
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

		/*register_element("guizmo_brush_mode"s, false, []()
			{
				ImGui::BeginDisabled(!dvars::guizmo_enable->current.enabled);
				{
					static bool hov_guizmo_brush_mode;
					if (image_togglebutton(
						"guizmo_brush_mode",
						hov_guizmo_brush_mode,
						dvars::guizmo_brush_mode->current.enabled,
						dvars::guizmo_brush_mode->description))
					{
						dvars::set_bool(dvars::guizmo_brush_mode, !dvars::guizmo_brush_mode->current.enabled);
					}
				}
				ImGui::EndDisabled();
			});*/

		register_element("gameview"s, false, []()
			{
				static bool hov_gameview;
				if (image_togglebutton(
					"gameview",
					hov_gameview,
					dvars::radiant_gameview->current.enabled,
					std::string("Gameview " + ggui::hotkey_dialog::get_hotkey_for_command("xo_gameview")).c_str()))
				{
					components::gameview::p_this->set_state(!dvars::radiant_gameview->current.enabled);
				}
			});

		register_element("sunpreview"s, false, []()
			{
				static bool hov_fakesunpreview;

				if (image_togglebutton(
					"sunpreview",
					hov_fakesunpreview,
					dvars::r_fakesun_preview->current.enabled,
					std::string("Fake sun preview " + ggui::hotkey_dialog::get_hotkey_for_command("fakesun_toggle") + "\nSupports specular and bump mapping.").c_str()))
				{
					components::command::execute("fakesun_toggle");
				}
			});

		register_element("fakesun_fog"s, false, []()
			{
				static bool hov_fakesun_fog;

				if (image_togglebutton(
					"fakesun_fog",
					hov_fakesun_fog,
					dvars::r_fakesun_fog_enabled->current.enabled,
					std::string("Toggle Fog " + ggui::hotkey_dialog::get_hotkey_for_command("fakesun_fog_toggle")).c_str()))
				{
					components::command::execute("fakesun_fog_toggle");
				}
			});

		register_element("filmtweaks"s, false, []()
			{
				static bool hov_filmtweaks_settings;
				const auto r_filmtweakenable = game::Dvar_FindVar("r_filmtweakenable");

				if (image_togglebutton(
					"filmtweaks",
					hov_filmtweaks_settings,
					r_filmtweakenable->current.enabled,
					std::string("Toggle filmtweaks " + ggui::hotkey_dialog::get_hotkey_for_command("filmtweak_toggle")).c_str()))
				{
					components::command::execute("filmtweak_toggle");
				}
			});

		register_element("fx_play"s, false, []()
			{
				static bool hov_fx_play;

				ImVec4 overwrite_active_bg = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);

				const bool can_fx_play = components::effects::effect_can_play();
				ImGui::BeginDisabled(!can_fx_play);

				if (image_togglebutton(
					"fx_play",
					hov_fx_play,
					can_fx_play,
					std::string("Play Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_play")).c_str(),
					nullptr,
					nullptr,
					&overwrite_active_bg))
				{
					components::effects::play();
				}

				ImGui::EndDisabled();
			});

		register_element("fx_repeat"s, false, []()
			{
				static bool hov_fx_repeat;

				ImVec4 overwrite_active_bg = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);

				const bool can_fx_play = components::effects::effect_can_play();
				ImGui::BeginDisabled(!can_fx_play);

				if (image_togglebutton(
					"fx_repeat",
					hov_fx_repeat,
					can_fx_play,
					std::string("Re-trigger Effect every X seconds for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_repeat")).c_str(),
					nullptr,
					nullptr,
					&overwrite_active_bg))
				{
					components::effects::repeat();
				}

				ImGui::EndDisabled();
			});

		register_element("fx_pause"s, false, []()
			{
				static bool hov_fx_pause;

				ImVec4 overwrite_active_bg = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);

				const bool can_fx_play = components::effects::effect_can_play();
				ImGui::BeginDisabled(!can_fx_play);

				if (image_togglebutton(
					"fx_pause",
					hov_fx_pause,
					can_fx_play,
					std::string("Stop Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_pause")).c_str(),
					nullptr,
					nullptr,
					&overwrite_active_bg))
				{
					components::effects::pause();
				}

				ImGui::EndDisabled();
			});

		register_element("fx_stop"s, false, []()
			{
				static bool hov_fx_stop;

				ImVec4 overwrite_active_bg = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);

				const bool can_fx_play = components::effects::effect_can_play();
				ImGui::BeginDisabled(!can_fx_play);

				if (image_togglebutton(
					"fx_stop",
					hov_fx_stop,
					can_fx_play,
					std::string("Stop Effect for last selected fx_origin " + ggui::hotkey_dialog::get_hotkey_for_command("fx_stop")).c_str(),
					nullptr,
					nullptr,
					&overwrite_active_bg))
				{
					components::effects::stop();
				}

				ImGui::EndDisabled();
			});

		register_element("fx_edit"s, false, []()
			{
				static bool hov_fx_edit;

				ImVec4 overwrite_active_bg = ImGui::ToImVec4(dvars::gui_toolbar_button_color->current.vector);

				const bool can_fx_play = components::effects::effect_can_play();
				ImGui::BeginDisabled(!can_fx_play);

				if (image_togglebutton(
					"fx_edit",
					hov_fx_edit,
					hov_fx_edit,
					"Edit Effect for last selected fx_origin",
					nullptr,
					nullptr,
					&overwrite_active_bg))
				{
					components::effects::edit();
				}

				ImGui::EndDisabled();
			});



#ifdef DEBUG
		register_element(";"s, nullptr);

		register_element("debug_reload_images"s, []()
			{
				if (ImGui::Button("RI", TB_IMAGEBUTTON_SIZE)) {
					game::R_ReloadImages();
				} TT("DBG: Reload Images");
			});
#endif
	}

	void toolbar_dialog::load_settings_ini()
	{
		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
						fs_homepath)
		{
			bool no_ini_or_empty = false;

			
			std::string ini_path = fs_homepath->current.string;
						ini_path += "\\IW3xRadiant\\" + TB_INI_FILENAME;

			std::ifstream ini;
			ini.open(ini_path.c_str());

			if (!ini.is_open())
			{
				//game::printf_to_console("[!][Toolbar] Failed to open ini: '%s'", ini_path.c_str());
				no_ini_or_empty = true;
			}

			// read ini line by line
			if (!no_ini_or_empty)
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
						m_sorted_elements.emplace_back(
							tb_order_element_s("; ------------", m_sorted_element_id++, true, true, false));

						continue;
					}

					// split the string on ','
					std::vector<std::string> args = utils::split(input, ',');

					if (args.size() != 2)
					{
						printf("[!][Toolbar] malformed element @ line #%d ('%s')", line, input.c_str());
						continue;
					}

					utils::trim(args[0]);
					utils::trim(args[1]);

					// check if the element exists
					if (m_registered_elements_callbacks.find(args[0]) != m_registered_elements_callbacks.end())
					{
						m_sorted_elements.emplace_back(
							tb_order_element_s(args[0], m_sorted_element_id++, args[1] != "0"s, false));
					}
					else
					{
						game::printf_to_console("[!][Toolbar] not a valid element @ line #%d ('%s')", line, input.c_str());
					}
				}

				if (!line)
				{
					no_ini_or_empty = true;
				}
			}

			// loop all registered elements to add all elements not found inside the ini (eg. new elements after an update)
			for (const auto& element : m_registered_elements)
			{
				bool found = false;
				bool is_debug = false;

				// add seperators if no ini or ini was empty
				// do not add separators otherwise
				if (!no_ini_or_empty)
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

				for (uint32_t e = 0; e < m_sorted_elements.size(); e++)
				{
					if (element.name == m_sorted_elements[e].name)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					m_sorted_elements.emplace_back(
						tb_order_element_s(element.name, m_sorted_element_id++, element.visible, false, is_debug));
				}
			}
		}
	}

	void toolbar_dialog::save_settings_ini()
	{
		if (m_sorted_elements.empty())
		{
			return;
		}

		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
						fs_homepath)
		{
			std::ofstream ini;
			std::string ini_path = fs_homepath->current.string;
						ini_path += "\\IW3xRadiant\\" + TB_INI_FILENAME;

			ini.open(ini_path.c_str());
			if (!ini.is_open())
			{
				printf("[ERR][Toolbar] Failed to write to file: '%s'", ini_path.c_str());
				return;
			}

			ini << "// [Name] [IsVisible] (';' Separator)" << std::endl;

			for (const auto& element : m_sorted_elements)
			{
				if (element.debug)
				{
					continue;
				}

				if (element.is_separator)
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

	void toolbar_dialog::toolbar()
	{
		const bool TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED = true;

		int _stylevars = 0;
		int _stylecolors = 0;

		// request auto-sizing on one axis
		const ImVec2 requested_size = (this->m_toolbar_axis == ImGuiAxis_X) ? ImVec2(-1.0f, 0.0f) : ImVec2(0.0f, -1.0f);

		ImGui::SetNextWindowSize(requested_size);

		if (this->m_toolbar_reset)
		{
			if (this->m_toolbar_axis == ImGuiAxis_X)
			{
				ImGui::SetNextWindowDockID(this->m_toolbar_dock_top);
			}
			else
			{
				ImGui::SetNextWindowDockID(this->m_toolbar_dock_left);
			}
			
			this->m_toolbar_reset = false;
		}

		ImGuiWindowClass window_class;
		window_class.DockingAllowUnclassed = true;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoCloseButton;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingSplitMe;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverMe;
		window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverOther;

		if (this->m_toolbar_axis == ImGuiAxis_X)
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

			const ImGuiAxis toolbar_axis_perp = (ImGuiAxis)(this->m_toolbar_axis ^ 1);
			const float TOOLBAR_SIZE_WHEN_DOCKED = style.WindowPadding[toolbar_axis_perp] * 2.0f + TB_IMAGEBUTTON_SIZE[toolbar_axis_perp];

			node->WantLockSizeOnce = true;
			node->Size[toolbar_axis_perp] = node->SizeRef[toolbar_axis_perp] = TOOLBAR_SIZE_WHEN_DOCKED;

			if (TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
			{
				if (node->ParentNode && node->ParentNode->SplitAxis != ImGuiAxis_None)
				{
					this->m_toolbar_axis = (ImGuiAxis)(node->ParentNode->SplitAxis ^ 1);
				}
			}
		}

		// on_open would be to late since its called after main gui creation
		if (!this->is_initiated())
		{
			this->toolbar_elements_init();
			this->load_settings_ini();
			this->set_initiated();
		}

		// *
		// populate toolbar

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);							_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.0f, 4.0f));		_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));		_stylevars++;

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(70, 70, 70, 70));			_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));		_stylecolors++;

		// loop amount of registered-sorted elements
		for (uint32_t e = 0; e < m_sorted_elements.size(); e++)
		{
			if (m_sorted_elements[e].is_separator)
			{
				if (this->m_toolbar_axis == ImGuiAxis_X)
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
			if (auto	element = m_registered_elements_callbacks.find(m_sorted_elements[e].name);
						element != m_registered_elements_callbacks.end())
			{
				if (m_sorted_elements[e].visible)
				{
					element->second();

					if (this->m_toolbar_axis == ImGuiAxis_X)
					{
						ImGui::SameLine();
					}
				}
			}
		}

		ImGui::PopStyleColor(_stylecolors); _stylecolors = 0;
		ImGui::PopStyleVar(_stylevars); _stylevars = 0;


		this->m_toolbar_pos = ImGui::GetWindowPos();
		this->m_toolbar_size = ImGui::GetWindowSize();


		// *
		// context (right click) menu

		if (ImGui::BeginPopupContextWindow())
		{
			// only enable horz/vert options when undocked
			if (node == nullptr
				|| node && node->ParentNode == nullptr
				|| !TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
			{
				if (ImGui::MenuItem("Horizontal", "", (this->m_toolbar_axis == ImGuiAxis_X)))
				{
					this->m_toolbar_axis = ImGuiAxis_X;
				}

				if (ImGui::MenuItem("Vertical", "", (this->m_toolbar_axis == ImGuiAxis_Y)))
				{
					this->m_toolbar_axis = ImGuiAxis_Y;
				}
			}

			if (ImGui::MenuItem("Edit Toolbar ..."))
			{
				GET_GUI(ggui::toolbar_edit_dialog)->toggle();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}


	bool toolbar_dialog::gui()
	{
		return false;
	}

	void toolbar_dialog::on_open()
	{ }

	void toolbar_dialog::on_close()
	{ }

	REGISTER_GUI(toolbar_dialog);
}


// -----------------------------


namespace ggui
{
	bool toolbar_edit_dialog::gui()
	{
		const auto tb = GET_GUI(ggui::toolbar_dialog);

		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if(!ImGui::Begin("Toolbar Editor##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}


		static float button_group_width = 140.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_group_width) * 0.5f - 8.0f);

		ImGui::BeginGroup();
		{
			if (ImGui::Button("Add Separator"))
			{
				tb->register_element(";"s, nullptr);
			}

			ImGui::SameLine();
			if (ImGui::Button("Delete Selected Separator"))
			{
				const bool out_of_bounds = this->tbedit_selection.element_pos >= tb->m_sorted_elements.size();

				if (this->tbedit_selection.is_selected
					&& !out_of_bounds
					&& tb->m_sorted_elements[this->tbedit_selection.element_pos].is_separator)
				{
					//game::printf_to_console("selected sep, deleting ...\n");
					tb->m_sorted_elements.erase(tb->m_sorted_elements.begin() + this->tbedit_selection.element_pos);
				}
				else if (out_of_bounds)
				{
					this->tbedit_selection.is_selected = false;
					game::printf_to_console("Selection out of bounds!\n");
				}
			}

			ImGui::SameLine();
			ImGui::HelpMarker("Rearrange elements by dragging them up and down the list using the left mousebutton");

			ImGui::EndGroup();
			button_group_width = ImGui::GetItemRectSize().x;
		}

		// this does not work with the way we order the list -> flickering
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 2.0f)); // Y needs to be at 2 so each elem slightly overlaps the next one

		if (ImGui::BeginTable(
			"element_sort_table", 
			3,
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX | /*ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH |*/ ImGuiTableFlags_ScrollY,
			ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 6.0f)))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, TB_IMAGEBUTTON_SIZE.y);
			ImGui::TableSetupColumn("Element", ImGuiTableColumnFlags_WidthStretch, 200.0f);
			ImGui::TableSetupColumn("Visible", ImGuiTableColumnFlags_WidthFixed, 48.0f);
			ImGui::TableHeadersRow();

			int widget_id_num = 0;

			// loop amount of registered-sorted elements
			for (uint32_t e = 0; e < tb->m_sorted_elements.size(); e++)
			{
				const auto element = tb->m_sorted_elements[e];

				// check if the element was registered
				if (element.is_separator || tb->m_registered_elements_callbacks.find(tb->m_sorted_elements[e].name) != tb->m_registered_elements_callbacks.end())
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

									ImGui::Image(image->texture.data, TB_IMAGEBUTTON_SIZE, uv0, uv1);
								}

								// placeholder image when icon is missing
								else if (const auto temp_image = game::Image_RegisterHandle("cycle_xyz");
									temp_image && temp_image->texture.data)
								{
									const ImVec2 uv0 = ImVec2(0.5f, 0.0f);
									const ImVec2 uv1 = ImVec2(1.0f, 1.0f);

									ImGui::Image(temp_image->texture.data, TB_IMAGEBUTTON_SIZE, uv0, uv1);
								}
							}

							break;

						case 1:
							if (element.is_separator)
							{
								// unique id for each separator
								if (ImGui::Selectable(
									utils::va("------------##%d", tb->m_sorted_elements[e].id),
									e == this->tbedit_selection.element_pos && this->tbedit_selection.is_selected,
									ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
									ImVec2(0.0f, TB_IMAGEBUTTON_SIZE.y)))
								{
									this->tbedit_selection.is_selected = true;
									this->tbedit_selection.element_id = tb->m_sorted_elements[e].id;
									this->tbedit_selection.element_pos = e;
								}
							}
							else
							{
								if (ImGui::Selectable(
									element.name.c_str(),
									e == this->tbedit_selection.element_pos && this->tbedit_selection.is_selected,
									ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
									ImVec2(0.0f, TB_IMAGEBUTTON_SIZE.y)))
								{
									this->tbedit_selection.is_selected = true;
									this->tbedit_selection.element_id = tb->m_sorted_elements[e].id;
									this->tbedit_selection.element_pos = e;
								}
							}

							if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
							{
								int n_next = e + (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
								if (n_next >= 0 && static_cast<uint32_t>(n_next) < tb->m_sorted_elements.size())
								{
									const auto temp_pair = tb->m_sorted_elements[n_next];

									tb->m_sorted_elements[n_next] = tb->m_sorted_elements[e];
									tb->m_sorted_elements[e] = temp_pair;

									ImGui::ResetMouseDragDelta();

									this->tbedit_selection.is_selected = true;
									this->tbedit_selection.element_id = tb->m_sorted_elements[n_next].id;
									this->tbedit_selection.element_pos = n_next;
								}
							}
							break;

						case 2:
							if (!element.is_separator)
							{
								ImGui::PushID(widget_id_num); widget_id_num++;
								ImGui::Checkbox("##checkbox", &tb->m_sorted_elements[e].visible);
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

		ImGui::PopStyleVar();
		ImGui::End();
		return true;
	}

	void toolbar_edit_dialog::on_open()
	{ }

	void toolbar_edit_dialog::on_close()
	{
		GET_GUI(ggui::toolbar_dialog)->save_settings_ini();
	}

	REGISTER_GUI(toolbar_edit_dialog);
}
