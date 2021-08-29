#include "std_include.hpp"

#define SEPERATOR_VERT									\
	ImGui::SameLine();									\
	SPACING(2.0f, 0.0f);								\
	ImGui::SameLine();									\
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);	\
	ImGui::SameLine();									\
	SPACING(4.0f, 0.0f);								\
	ImGui::SameLine();

namespace ggui::toolbar
{
	const ImVec2 toolbar_imagebutton_size = ImVec2(28.0f, 28.0f);
	
	void image_button(const char* image_name, bool& hovered_state, E_CALLTYPE calltype, uint32_t func_addr, const char* tooltip)
	{
		if (const auto image = game::Image_RegisterHandle(image_name);
			image && image->texture.data)
		{
			const ImVec2 uv0 = hovered_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = hovered_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
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
		
		if (const auto image = game::Image_RegisterHandle(image_name);
			image && image->texture.data)
		{
			const ImVec2 uv0 = toggle_state ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			const ImVec2 uv1 = toggle_state ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				ret_state = true;
			}

			if (tooltip)
			{
				TT(tooltip);
			}

			//hovered_state = ImGui::IsItemHovered();
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
	
	void menu(ggui::imgui_context_menu& menu)
	{
		int _stylevars = 0; int _stylecolors = 0;

		const auto prefs = game::g_PrefsDlg();
		
		// *
		// create toolbar window

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 4.0f));		_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));	_stylevars++;

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(100, 100, 100, 70));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));		_stylecolors++;

		ImGui::Begin("toolbar_xywnd", nullptr,
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

		//CMainFrame::OnFileOpen
		image_button("open", hov_open, MAINFRAME_CDECL, 0x423AE0, std::string("Open File [" + hotkeys::get_hotkey_for_command("FileOpen") + "]").c_str());
		ImGui::SameLine();

		//CMainFrame::OnFileSave
		image_button("save", hov_save, MAINFRAME_CDECL, 0x423B80, std::string("Save File [" + hotkeys::get_hotkey_for_command("FileSave") + "]").c_str());
		ImGui::SameLine();

		SEPERATOR_VERT;

		// CMainFrame::OnBrushFlipx
		image_button("flip_x", hov_flipx, CDECLCALL, 0x4250A0, "Flip Brush X-Axis");
		ImGui::SameLine();

		// CMainFrame::OnBrushFlipy
		image_button("flip_y", hov_flipy, CDECLCALL, 0x4250C0, "Flip Brush Y-Axis");
		ImGui::SameLine();

		// CMainFrame::OnBrushFlipz
		image_button("flip_z", hov_flipz, CDECLCALL, 0x4250E0, "Flip Brush Z-Axis");
		ImGui::SameLine();
		
		SEPERATOR_VERT;

		// CMainFrame::OnBrushRotatex
		image_button("rotate_x", hov_rotx, CDECLCALL, 0x425100, "Rotate Brush X-Axis");
		ImGui::SameLine();

		// CMainFrame::OnBrushRotatey
		image_button("rotate_y", hov_roty, CDECLCALL, 0x425190, "Rotate Brush Y-Axis");
		ImGui::SameLine();

		// CMainFrame::OnBrushRotatez
		image_button("rotate_z", hov_rotz, CDECLCALL, 0x425220, std::string("Rotate Brush Z-Axis [" + hotkeys::get_hotkey_for_command("RotateZ") + "]").c_str());
		ImGui::SameLine();

		SEPERATOR_VERT;

		// CMainFrame::OnSelectionSelectcompletetall
		image_button("select_complete_tall", hov_sel_compl_tall, CDECLCALL, 0x426340, "Select Complete Tall");
		ImGui::SameLine();

		// CMainFrame::OnSelectionSelecttouching
		image_button("select_touching", hov_sel_touching, CDECLCALL, 0x426370, "Select Touching");
		ImGui::SameLine();

		// CMainFrame::OnSelectionSelectpartialtall
		image_button("select_partial_tall", hov_sel_partial_tall, CDECLCALL, 0x426360, "Select Partial Tall");
		ImGui::SameLine();

		// CMainFrame::OnSelectionSelectinside
		image_button("select_inside", hov_sel_inside, CDECLCALL, 0x426350, "Select Inside");
		ImGui::SameLine();

		SEPERATOR_VERT;

		// CMainFrame::OnViewClipper
		if (image_togglebutton("clipper",
			game::g_bClipMode,
			"Toggle Clipper"))
		{
			mainframe_thiscall(LRESULT, 0x426510);
		} ImGui::SameLine();

		// CMainFrame::OnSelectionCsgmerge
		image_button("csg_merge", hov_csg_merge, CDECLCALL, 0x4255D0, std::string("CSG Merge [" + hotkeys::get_hotkey_for_command("CSGMerge") + "]").c_str());
		ImGui::SameLine();

		// CMainFrame::OnSelectionMakehollow
		image_button("csg_hollow", hov_csg_hollow, CDECLCALL, 0x425570, "CSG Hollow");
		ImGui::SameLine();

		SEPERATOR_VERT;

		// CMainFrame::OnTextureFlipX
		image_button("texflip_x", hov_texflipx, CDECLCALL, 0x42BF40, "Flip Texture X-Axis");
		ImGui::SameLine();

		
		// CMainFrame::OnTextureFlipY
		image_button("texflip_y", hov_texflipy, CDECLCALL, 0x42BF50, "Flip Texture Y-Axis");
		ImGui::SameLine();

		
		// CMainFrame::OnTextureFlip90
		image_button("texflip_90", hov_texflip90, CDECLCALL, 0x42BF60, "Rotate Texture 90 Degrees");
		ImGui::SameLine();

		
		// CMainFrame::OnEditLayerCycle
		image_button("cycle_layer", hov_cycle_layer, CDECLCALL, 0x424010, std::string("Cycle Texture Layer [" + hotkeys::get_hotkey_for_command("TexLayerCycle") + "]").c_str());
		ImGui::SameLine();
		
		SEPERATOR_VERT;

		// CMainFrame::OnToggleCameraMovementMode
		ImGui::BeginGroup();
		{
			ImVec2 prebutton_cursor = ImGui::GetCursorScreenPos();

			if (image_togglebutton("camera_movement",
				prefs->camera_mode,
				"Toggle Camera Movement Mode"))
			{
				mainframe_thiscall(LRESULT, 0x429EB0);
			}

			if(prefs->camera_mode)
			{
				prebutton_cursor.x += (toolbar_imagebutton_size.x * 0.45f);
				prebutton_cursor.y += (toolbar_imagebutton_size.y * 0.4f);

				ImGui::PushFontFromIndex(REGULAR_12PX);
				ImGui::SetCursorScreenPos(prebutton_cursor);
				ImGui::Text("%d/2", prefs->camera_mode);
				ImGui::PopFont();
			}
		}
		ImGui::EndGroup();
		ImGui::SameLine();

		
		// CMainFrame::OnViewCubicclipping
		if (image_togglebutton("cubic_clip",
			prefs->m_bCubicClipping,
			std::string("Cubic Clipping [" + hotkeys::get_hotkey_for_command("ToggleCubicClip") + "]").c_str()))
		{
			mainframe_thiscall(LRESULT, 0x428F90);
		} ImGui::SameLine();

		
		// CMainFrame::OnViewNextview
		ImGui::BeginGroup();
		{
			ImVec2	prebutton_cursor = ImGui::GetCursorScreenPos();
			image_button("empty_hover", hov_cycle_xyz, MAINFRAME_THIS, 0x426DB0, std::string("Cycle Grid Window View [" + hotkeys::get_hotkey_for_command("NextView") + "]").c_str());

			ImGui::PushFontFromIndex(REGULAR_18PX);
			const auto viewtype = cmainframe::activewnd->m_pXYWnd->m_nViewType;

			const char* curr_view_str = viewtype == 0 ? "YZ" : viewtype == 1 ? "XZ" : "XY";
			const auto text_size = ImGui::CalcTextSize(curr_view_str);

			prebutton_cursor.x += (toolbar_imagebutton_size.x * 0.5f - (text_size.x * 0.5f));
			prebutton_cursor.y += (toolbar_imagebutton_size.y * 0.5f - (text_size.y * 0.5f));
			
			ImGui::SetCursorScreenPos(prebutton_cursor);
			ImGui::Text("%s", curr_view_str);
			ImGui::PopFont();
		}
		ImGui::EndGroup();
		ImGui::SameLine();

		SEPERATOR_VERT;

		// CMainFrame::OnPlantModel
		if (image_togglebutton("plant_models",
			prefs->m_bDropModel,
			"Plant models and apply random scale and rotation"))
		{
			mainframe_thiscall(LRESULT, 0x42A0E0);
		} ImGui::SameLine();


		// CMainFrame::OnPlantModel
		if (image_togglebutton("plant_orient_to_floor",
			prefs->m_bOrientModel,
			"Orient dropped selection to the floor"))
		{
			mainframe_thiscall(LRESULT, 0x4258F0);
		} ImGui::SameLine();


		// CMainFrame::OnForceZeroDropHeight
		if (image_togglebutton("plant_force_drop_height",
			prefs->m_bForceZeroDropHeight,
			"Force drop height to 0"))
		{
			mainframe_thiscall(LRESULT, 0x42A000);
		} ImGui::SameLine();


		// CMainFrame::OnDropSelected
		image_button("drop_entities_floor", hov_drop_entities, CDECLCALL, 0x425BE0, "Drop selection to the floor [CTRL-ALT-MOUSE3]");
		ImGui::SameLine();

		// CMainFrame::OnDropSelectedRelativeZ
		image_button("drop_entities_floor_relative", hov_drop_entities_relative_z, MAINFRAME_THIS , 0x425940, "Drop selection to the floor with relative Z heights");
		ImGui::SameLine();
		
		SEPERATOR_VERT;
		
		// CMainFrame::OnSelectMouserotate
		if (image_togglebutton("free_rotate",
			game::g_bRotateMode,
			std::string("Free rotation [" + hotkeys::get_hotkey_for_command("MouseRotate") + "]").c_str()))
		{
			mainframe_thiscall(LRESULT, 0x428570);
		} ImGui::SameLine();

		
		// CMainFrame::OnSelectMousescale
		if (image_togglebutton("free_scale",
			game::g_bScaleMode,
			"Free scaling"))
		{
			mainframe_thiscall(LRESULT, 0x428D20);
		} ImGui::SameLine();
		

		// CMainFrame::OnScalelockX
		if(image_togglebutton("lock_x", 
			(game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6), 
			"Lock grid along the x-axis"))
		{
			mainframe_thiscall(LRESULT, 0x428BC0); 
		} ImGui::SameLine();

		
		// CMainFrame::OnScalelockY
		if (image_togglebutton("lock_y", 
			(game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5),
			"Lock grid along the y-axis"))
		{
			mainframe_thiscall(LRESULT, 0x428B60);
		} ImGui::SameLine();

		
		// CMainFrame::OnScalelockZ
		if (image_togglebutton("lock_z",
			(game::g_nScaleHow > 0 && game::g_nScaleHow <= 3),
			"Lock grid along the z-axis"))
		{
			mainframe_thiscall(LRESULT, 0x428B90);
		} ImGui::SameLine();

		SEPERATOR_VERT;

		// CMainFrame::OnPatchWireframe
		ImGui::BeginGroup();
		{
			ImVec2 prebutton_cursor = ImGui::GetCursorScreenPos();

			if (image_togglebutton("show_patches_as",
				prefs->g_nPatchAsWireframe,
				"Show patches as wireframe"))
			{
				mainframe_thiscall(LRESULT, 0x42A300);
			}

			if (prefs->g_nPatchAsWireframe)
			{
				prebutton_cursor.x += (toolbar_imagebutton_size.x * 0.45f);
				prebutton_cursor.y += (toolbar_imagebutton_size.y * 0.45f);

				ImGui::PushFontFromIndex(REGULAR_12PX);
				ImGui::SetCursorScreenPos(prebutton_cursor);
				ImGui::Text("%d/2", prefs->g_nPatchAsWireframe);
				ImGui::PopFont();
			}
		}
		ImGui::EndGroup();
		ImGui::SameLine();


		// CMainFrame::OnRedistPatchPoints
		image_button("redisperse_patch_points", hov_redisp_patch_pts, CDECLCALL, 0x42A270,
			std::string("Redisperse Patch Points [" + hotkeys::get_hotkey_for_command("RedisperseVertices") + "]").c_str());
		ImGui::SameLine();


		// CMainFrame::OnPatchWeld
		if (image_togglebutton("weld_equal_patches_move",
			prefs->g_bPatchWeld,
			"Weld equal patch points during moves"))
		{
			mainframe_thiscall(LRESULT, 0x42A400);
		} ImGui::SameLine();


		// CMainFrame::OnPatchDrilldown
		if (image_togglebutton("select_drill_down_vertices",
			prefs->patch_drill_down,
			"Select invisible vertices (drill down rows/columns)"))
		{
			mainframe_thiscall(LRESULT, 0x42A510);
		} ImGui::SameLine();


		// CMainFrame::ToggleLockPatchVertMode
		if (image_togglebutton("toggle_lock_vertices_mode",
			game::g_qeglobals->bLockPatchVerts,
			"Toggle lock-vertex mode"))
		{
			mainframe_thiscall(LRESULT, 0x42B4F0);
		} ImGui::SameLine();


		// CMainFrame::ToggleUnlockPatchVertMode
		if (image_togglebutton("toggle_unlock_vertices_mode",
			game::g_qeglobals->bUnlockPatchVerts,
			"Toggle unlock-vertex mode"))
		{
			mainframe_thiscall(LRESULT, 0x42B510);
		} ImGui::SameLine();


		// CMainFrame::OnCycleTerrainEdge
		if (image_togglebutton("cycle_patch_edge_direction",
			game::g_qeglobals->d_select_mode == 9,
			"Toggle terrain-quad edge cycle mode"))
		{
			mainframe_thiscall(LRESULT, 0x42B530);
		} ImGui::SameLine();


		// CMainFrame::OnTolerantWeld
		if (image_togglebutton("tolerant_weld",
			prefs->m_bTolerantWeld,
			"Toggle tolerant weld / Draw tolerant weld lines"))
		{
			mainframe_thiscall(LRESULT, 0x42A130);
		} ImGui::SameLine();

		SEPERATOR_VERT;
		
		// CMainFrame::OnToggleDrawSurfs
		if (image_togglebutton("toggle_draw_surfs_portal",
			prefs->draw_toggle,
			"Toggle drawing of portal no-draw surfaces"))
		{
			mainframe_thiscall(LRESULT, 0x42A040);
		} ImGui::SameLine();
		
		// CMainFrame::OnDontselectcurve
		if (image_togglebutton("dont_select_curve",
			!prefs->m_bSelectCurves,
			"Disable selection of patches"))
		{
			mainframe_thiscall(LRESULT, 0x429920);
		} ImGui::SameLine();

		
		// CMainFrame::OnDisableSelectionOfEntities
		if (image_togglebutton("dont_select_entities",
			prefs->entities_off,
			"Disable selection of entities"))
		{
			mainframe_thiscall(LRESULT, 0x429F60);
		} ImGui::SameLine();

		
		// CMainFrame::OnDisableSelectionOfSky
		if (image_togglebutton("dont_select_sky",
			prefs->sky_brush_off,
			"Disable selection of sky brushes"))
		{
			mainframe_thiscall(LRESULT, 0x429FB0);
		} ImGui::SameLine();

		
		// CMainFrame::OnSelectableModels
		if (image_togglebutton("dont_select_models",
			prefs->m_bSelectableModels,
			"Disable selection of static models"))
		{
			mainframe_thiscall(LRESULT, 0x42A280);
		} ImGui::SameLine();
		
#ifdef DEBUG
		SEPERATOR_VERT;
		
		if (ImGui::Button("Reload Images")) {
			game::R_ReloadImages();
		}
		ImGui::SameLine();
#endif
		
		// TODO! - remove me
		/*ImGui::SameLine();
		if (ImGui::Button("Lock X")) { mainframe_thiscall(LRESULT, 0x428BC0); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Y")) { mainframe_thiscall(LRESULT, 0x428B60); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Z")) { mainframe_thiscall(LRESULT, 0x428B90); }*/


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
