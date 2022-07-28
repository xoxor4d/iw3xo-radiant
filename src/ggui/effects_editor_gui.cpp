#include "std_include.hpp"
#include "commdlg.h"

namespace ggui
{
	//ImGradient gradient;

	//int selected_editor_elemdef = 0;
	//bool m_effect_was_modified = false;
	//bool m_pending_close = false;
	//bool m_pending_reload = false;

	void effects_editor_dialog::elemdef_elem(fx_system::FxEditorElemDef* elemdef, int row, int* selected_index)
	{
		static bool is_renaming = false;
		static char rename_buf[48] = {};
		static int remame_index = -1;
		static bool rename_focus = false;

		// enabled / disabled state
		bool elem_enabled = !((elemdef->editorFlags & fx_system::FX_ED_FLAG_DISABLED) != 0);

		for (auto column = 0; column < 6; column++)
		{
			ImGui::PushID(column);
			ImGui::TableNextColumn();

			switch (column)
			{
			case 0: // Enable/Disable
				ImGui::Indent(6.0f);

				if (ImGui::Checkbox("##enabled_flag", &elem_enabled))
				{
					if (elem_enabled)
					{
						elemdef->editorFlags &= ~fx_system::FX_ED_FLAG_DISABLED;
					}
					else
					{
						elemdef->editorFlags |= fx_system::FX_ED_FLAG_DISABLED;
					}

					// restart effect
					if (components::effects::effect_is_playing() || components::effects::effect_is_repeating() || components::effects::effect_is_paused())
					{
						//components::effects::play();

						components::effects::stop();
						components::effects::apply_changes();
					}
				}

				ImGui::Indent(-6.0f);

				break;

			case 1: // Name

				if(is_renaming && remame_index == row)
				{
					if(ImGui::InputText("##elemdef_name", rename_buf, sizeof(rename_buf), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						const auto len = strlen(rename_buf);
						if(len && len < sizeof(rename_buf))
						{
							strcpy_s(elemdef->name, sizeof(rename_buf), rename_buf);
						}

						is_renaming = false;
						rename_focus = false;
					}

					if (rename_focus && !ImGui::IsItemFocused())
					{
						is_renaming = false;
						rename_focus = false;
					}

					if(!rename_focus)
					{
						ImGui::SetKeyboardFocusHere(-1);
						rename_focus = true;
					}
				}
				else
				{
					if(*selected_index != remame_index)
					{
						is_renaming = false;
						rename_focus = false;
					}

					if (ImGui::Selectable(elemdef->name, row == *selected_index, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
					{
						*selected_index = row;
					}
				}

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
				if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
				{
					*selected_index = row;

					if(ImGui::MenuItem("Rename"))
					{
						const auto len = strlen(elemdef->name);

						if(len && len < sizeof(rename_buf))
						{
							memset(rename_buf, 0, sizeof(rename_buf));
							strcpy_s(rename_buf, sizeof(rename_buf), elemdef->name);

							is_renaming = true;
							rename_focus = false;
							remame_index = *selected_index;
						}

						ImGui::CloseCurrentPopup();
					}

					if (ImGui::MenuItem("Delete segment"))
					{
						components::effects_editor::editor_delete_segment(selected_editor_elemdef);
						m_effect_was_modified = true;
					}

					ImGui::EndPopup();
				}
				ImGui::PopStyleVar();

				break;

			case 2: // Type
				ImGui::Text(s_elemTypeNames[elemdef->elemType]);
				break;

			case 3: // Delay
				ImGui::Text("%d", elemdef->spawnDelayMsec.base);
				break;

			case 4: // Count
				if ((elemdef->editorFlags & fx_system::FX_ED_FLAG_LOOPING) != 0)
				{
					ImGui::Text("%d", elemdef->spawnLooping.count);
				}
				else
				{
					if (elemdef->spawnOneShot.count.amplitude)
					{
						ImGui::Text("%d - %d", elemdef->spawnOneShot.count.base, elemdef->spawnOneShot.count.base + elemdef->spawnOneShot.count.amplitude);
					}
					else
					{
						ImGui::Text("%d", elemdef->spawnOneShot.count.base);
					}

				}
				break;

			case 5: // Sort
				ImGui::Text("%d", elemdef->sortOrder);
				break;

			default:
				__debugbreak();
			}

			// column
			ImGui::PopID();
		}
	}

	// Unsaved Changes
	bool effects_editor_dialog::modal_unsaved_changes(const char* label)
	{
		bool result = false;

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
		if (ImGui::BeginPopupModal(label, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const char* str = "\n         Unsaved Effect!\n  This will lose changes.\n\nDo you want to continue?\n\n";
			ImGui::SetCursorForCenteredText(str);
			ImGui::TextUnformatted(str);
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				result = true;
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				result = false;
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		return result;
	}

	// 0x402AB0
	void effects_editor_dialog::effect_elemdef_list()
	{
		const auto ed_effect = fx_system::get_editor_effect();
		if (!ed_effect)
		{
			return;
		}

		const auto MIN_WINDOW_SIZE = ImVec2(360.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(580.0f, 400.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (!ImGui::Begin("FX ElemList##window", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
		{
			ImGui::PopStyleVar();
			ImGui::End();
			return;
		}

		if (selected_editor_elemdef >= ed_effect->elemCount)
		{
			selected_editor_elemdef = ed_effect->elemCount - 1;
		}

		const ImVec4 toolbar_button_background_active = ImGui::ToImVec4(dvars::gui_window_bg_color->current.vector) + ImVec4(0.2f, 0.2f, 0.2f, 0.0f);
		const ImVec4 toolbar_button_background_hovered = ImGui::ToImVec4(dvars::gui_window_bg_color->current.vector) + ImVec4(0.05f, 0.05f, 0.05f, 0.0f);
		const ImVec2 toolbar_button_size = ImVec2(ImGui::GetFrameHeight() - 5.0f, ImGui::GetFrameHeight() - 5.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
		if (ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("File##efx"))
			{
				if(ImGui::MenuItem("Save Effect"))
				{
					if (fx_system::FX_SaveEditorEffect())
					{
						m_effect_was_modified = false;
					}
				}

				if (ImGui::MenuItem("Save Effect As"))
				{
					if (components::effects_editor::save_as())
					{
						//m_effect_was_modified = false;
					}
				}

				if (ImGui::MenuItem("Save As to fx_origin"))
				{
					if (components::effects_editor::save_as(true))
					{
						m_effect_was_modified = false;
					}
				} TT("Save effect as .. and changes the key-value-pair of the fx_origin to the new effect");

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Effect##efx"))
			{
				if (ImGui::MenuItem("Add segment"))
				{
					components::effects_editor::editor_add_new_segment();
					m_effect_was_modified = true;
				}

				if (ImGui::MenuItem("Delete selected segment"))
				{
					components::effects_editor::editor_delete_segment(selected_editor_elemdef);
					m_effect_was_modified = true;
				}

				if (ImGui::MenuItem("Duplicate selected segment"))
				{
					components::effects_editor::editor_clone_segment(selected_editor_elemdef);
					m_effect_was_modified = true;
				}

				ImGui::EndMenu();
			}


			const float menubar_y_offset = 3.0f;
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + menubar_y_offset);

			static bool fx_save_hov = false;
			if (toolbar_dialog::image_button_label(""
				, "save"
				, fx_save_hov
				, fx_save_hov
				, "Save effect"
				, &toolbar_button_background_active
				, &toolbar_button_background_hovered
				, &toolbar_button_size, 0.95f))
			{
				if (fx_system::FX_SaveEditorEffect())
				{
					m_effect_was_modified = false;
				}
			}

			static bool fx_seg_plus_hov = false;
			if (toolbar_dialog::image_button_label(""
				, "plus_minus"
				, false
				, fx_seg_plus_hov
				, "Add segment to effect"
				, &toolbar_button_background_active
				, &toolbar_button_background_hovered
				, &toolbar_button_size, 0.985f))
			{
				components::effects_editor::editor_add_new_segment();
				m_effect_was_modified = true;
			}

			ImGui::SameLine();
			ImGui::PushID("##seg_delete");
			ImGui::BeginDisabled(ed_effect->elemCount <= 1);
			{
				static bool fx_seg_minus_hov = false;
				if (toolbar_dialog::image_button_label(""
					, "plus_minus"
					, true
					, fx_seg_minus_hov
					, "Delete selected segment from effect"
					, &toolbar_button_background_active
					, &toolbar_button_background_hovered
					, &toolbar_button_size, 0.985f))
				{
					components::effects_editor::editor_delete_segment(selected_editor_elemdef);
					m_effect_was_modified = true;
				}

				ImGui::EndDisabled();
			}
			ImGui::PopID();


			static bool fx_seg_dupe_hov = false;
			if (toolbar_dialog::image_button_label(""
				, "duplicate_reload"
				, false
				, fx_seg_dupe_hov
				, "Duplicate segment"
				, &toolbar_button_background_active
				, &toolbar_button_background_hovered
				, &toolbar_button_size, 0.985f))
			{
				components::effects_editor::editor_clone_segment(selected_editor_elemdef);
				m_effect_was_modified = true;
			}

			// ----

			const char* reload_fx_modal_str = "Unsaved Changes##fx_reload";

			ImGui::PushID("##reload");
			ImGui::BeginDisabled(ed_effect->elemCount <= 1);
			{
				static bool fx_reload_hov = false;
				if (m_pending_reload || 
					toolbar_dialog::image_button_label(""
					, "duplicate_reload"
					, true
					, fx_reload_hov
					, "Reload effect"
					, &toolbar_button_background_active
					, &toolbar_button_background_hovered
					, &toolbar_button_size, 0.985f)
					)
				{
					m_pending_reload = false;
					if (m_effect_was_modified)
					{
						ImGui::OpenPopup(reload_fx_modal_str);
					}
					else
					{
						m_pending_close = false;
						components::command::execute("fx_reload");
					}
				}

				if (modal_unsaved_changes(reload_fx_modal_str)) // true if clicked OK ^
				{
					m_effect_was_modified = false;
					components::command::execute("fx_reload");
				}

				// ----------

				ImGui::EndDisabled();
			}
			ImGui::PopID();

			ImGui::PushFontFromIndex(ggui::BOLD_18PX);
			const char* effect_name = ed_effect->name;
			const auto width = ImGui::CalcTextSize(effect_name).x + 16.0f;
			ImGui::SameLine(ImGui::GetWindowWidth() - width - 90.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - menubar_y_offset);
			ImGui::Text("%s %s", effect_name, m_effect_was_modified ? "*" : "");
			ImGui::PopFont();

			// ------

			const char* close_editor_modal_str = "Unsaved Changes##editor_close";
			if (m_pending_close || ImGui::MenuItem("Close Editor"))
			{
				m_pending_close = false;
				if (m_effect_was_modified)
				{
					ImGui::OpenPopup(close_editor_modal_str);
				}
				else
				{
					m_effect_was_modified = false;

					components::effects::edit();
					components::command::execute("fx_reload");
				}
			}

			if (modal_unsaved_changes(close_editor_modal_str)) // true if clicked OK ^
			{
				m_effect_was_modified = false;

				components::effects::edit();
				components::command::execute("fx_reload");
			}

			// ------

			ImGui::EndMenuBar();
		}
		ImGui::PopStyleVar();

		/*if (ImGui::Button("Add Segment"))
			{
				components::effects_editor::editor_add_new_segment();
				m_effect_was_modified = true;
		}*/

		/*if (ImGui::Button("Delete Segment"))
		{
			components::effects_editor::editor_delete_segment(selected_editor_elemdef);
			m_effect_was_modified = true;
		}*/

		/*static bool fx_save_hov = false;
		if (toolbar_dialog::image_button_label(""
			, "save"
			, fx_save_hov
			, fx_save_hov
			, "Save Effect"
			, &toolbar_button_background_hovered
			, &toolbar_button_background_active
			, &toolbar_button_size, 0.93f))
		{
			if (fx_system::FX_SaveEditorEffect())
			{
				m_effect_was_modified = false;
			}
		}*/

		/*ImGui::SameLine();
		if (ImGui::Button("Save Effect"))
		{
			if (fx_system::FX_SaveEditorEffect())
			{
				m_effect_was_modified = false;
			}
		}*/

		//ImGui::SameLine();
		//if (ImGui::Button("Save As"))
		//{
		//	if (components::effects_editor::save_as())
		//	{
		//		m_effect_was_modified = false;
		//	}
		//}

		/*ImGui::SameLine();
		static bool fx_save_as_hov = false;
		if (toolbar_dialog::image_button_label(""
			, "save_as"
			, fx_save_as_hov
			, fx_save_as_hov
			, "Save Effect As (does not affect currently loaded efx)"
			, &toolbar_button_background_hovered
			, &toolbar_button_background_active
			, &toolbar_button_size, 0.93f))
		{
			if (components::effects_editor::save_as())
			{
				m_effect_was_modified = false;
			}
		}

		ImGui::SameLine();
		static bool fx_seg_plus_hov = false;
		if (toolbar_dialog::image_button_label(""
			, "plus_minus"
			, false
			, fx_seg_plus_hov
			, "Add Segment"
			, &toolbar_button_background_hovered
			, &toolbar_button_background_active
			, &toolbar_button_size, zoom))
		{
			components::effects_editor::editor_add_new_segment();
			m_effect_was_modified = true;
		}*/

		/*if (ImGui::Button("Add Segment"))
		{
			components::effects_editor::editor_add_new_segment();
			m_effect_was_modified = true;
		}*/

		//ImGui::SameLine();
		//ImGui::PushID("##seg_delete");
		//ImGui::BeginDisabled(ed_effect->elemCount <= 1);
		//{
		//	static bool fx_seg_minus_hov = false;
		//	if (toolbar_dialog::image_button_label(""
		//		, "plus_minus"
		//		, true
		//		, fx_seg_minus_hov
		//		, "Delete Segment"
		//		, &toolbar_button_background_hovered
		//		, &toolbar_button_background_active
		//		, &toolbar_button_size, zoom))
		//	{
		//		components::effects_editor::editor_delete_segment(selected_editor_elemdef);
		//		m_effect_was_modified = true;
		//	}

		//	/*if (ImGui::Button("Delete Segment"))
		//	{
		//		components::effects_editor::editor_delete_segment(selected_editor_elemdef);
		//		m_effect_was_modified = true;
		//	}*/

		//	ImGui::EndDisabled();
		//}
		//ImGui::PopID();
		

		//ImGui::SameLine();
		//if (ImGui::Button("Save As and Overwrite"))
		//{
		//	if (components::effects_editor::save_as())
		//	{
		//		m_effect_was_modified = false;
		//	}
		//}

		//ImGui::SameLine();
		//const char* reload_fx_modal_str = "Unsaved Changes##fx_reload";
		//if (ImGui::Button("Reload Effect") || m_pending_reload)
		//{
		//	m_pending_reload = false;
		//	if (m_effect_was_modified)
		//	{
		//		ImGui::OpenPopup(reload_fx_modal_str);
		//	}
		//	else
		//	{
		//		m_pending_close = false;
		//		components::command::execute("fx_reload");
		//	}
		//}

		//if (modal_unsaved_changes(reload_fx_modal_str)) // true if clicked OK ^
		//{
		//	m_effect_was_modified = false;
		//	components::command::execute("fx_reload");
		//}

		// ----

		//ImGui::SameLine();
		//const char* close_editor_modal_str = "Unsaved Changes##editor_close";
		//if (ImGui::Button("Close Editor") || m_pending_close)
		//{
		//	m_pending_close = false;
		//	if (m_effect_was_modified)
		//	{
		//		ImGui::OpenPopup(close_editor_modal_str);
		//	}
		//	else
		//	{
		//		m_effect_was_modified = false;

		//		components::effects::edit();
		//		components::command::execute("fx_reload");
		//	}
		//}

		//if (modal_unsaved_changes(close_editor_modal_str)) // true if clicked OK ^
		//{
		//	m_effect_was_modified = false;

		//	components::effects::edit();
		//	components::command::execute("fx_reload");
		//}


		/*ImGui::PushFontFromIndex(ggui::BOLD_18PX);
		const char* effect_name = ed_effect->name;
		const auto width = ImGui::CalcTextSize(effect_name).x + 16.0f;
		ImGui::SameLine(ImGui::GetWindowWidth() - width);
		ImGui::Text("%s %s", effect_name, m_effect_was_modified ? "*" : "");
		ImGui::PopFont();*/

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 6));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(9, 9));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(9, 9));

		if (ImGui::BeginTable("bind_table", 6,
			ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoBordersInBody /*| ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH*/))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("##enabled_disabled", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 28.0f);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 200.0f);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 120.0f);
			ImGui::TableSetupColumn("Delay", ImGuiTableColumnFlags_WidthStretch, 40.0f);
			ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthStretch, 40.0f);
			ImGui::TableSetupColumn("Sort", ImGuiTableColumnFlags_WidthStretch, 40.0f);

			ImGui::TableHeadersRow();

			for (int row = 0; row < ed_effect->elemCount; row++)
			{
				// unique widget id's for each row (we get collisions otherwise)
				ImGui::PushID(row);
				ImGui::TableNextRow();

				elemdef_elem(&ed_effect->elems[row], row, &selected_editor_elemdef);

				// row
				ImGui::PopID();
			}

			ImGui::EndTable();
		}
		ImGui::PopStyleVar(4);

		ImGui::PopStyleVar(); // WindowPadding
		ImGui::End();
	}

	// ----

#define MOD_CHECK(control) if((control)) modified = true
#define MOD_CHECK_GRAPH(control) if((control) >= 0) modified = true

	fx_system::FxEffectDef* effects_editor_dialog::effectdef_fileprompt()
	{
		char filename[MAX_PATH];
		OPENFILENAMEA ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = cmainframe::activewnd->GetWindow();
		ofn.lpstrFilter = "Effect Files\0*.efx\0Any File\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "Select an effect ...";
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn))
		{
			const std::string filepath = filename;
			const std::string replace_path = "raw\\fx\\";
			const std::size_t pos = filepath.find(replace_path) + replace_path.length();

			std::string loc_filepath = filepath.substr(pos);
			utils::erase_substring(loc_filepath, ".efx"s);
			utils::replace(loc_filepath, "\\", "/");

			return fx_system::FX_Register(loc_filepath.c_str());
		}

		return nullptr;
	}

	game::Material* effects_editor_dialog::material_fileprompt()
	{
		char filename[MAX_PATH];
		OPENFILENAMEA ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = cmainframe::activewnd->GetWindow();
		ofn.lpstrFilter = "Material\0*.*\0Effect Materials\0gfx_*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "Select a material ...";
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn))
		{
			const std::string filepath = filename;
			const std::string replace_path = "raw\\materials\\";
			const std::size_t pos = filepath.find(replace_path) + replace_path.length();

			std::string loc_filepath = filepath.substr(pos);
			//utils::erase_substring(loc_filepath, ".efx"s);
			utils::replace(loc_filepath, "\\", "/");

			return game::Material_RegisterHandle(loc_filepath.c_str(), 0);
		}

		return nullptr;
	}

	game::XModel* effects_editor_dialog::xmodel_fileprompt()
	{
		char filename[MAX_PATH];
		OPENFILENAMEA ofn;
		ZeroMemory(&filename, sizeof(filename));
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = cmainframe::activewnd->GetWindow();
		ofn.lpstrFilter = "XModel\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = "Select a xmodel ...";
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn))
		{
			const std::string filepath = filename;
			const std::string replace_path = "raw\\xmodel\\";
			const std::size_t pos = filepath.find(replace_path) + replace_path.length();

			std::string loc_filepath = filepath.substr(pos);
			//utils::erase_substring(loc_filepath, ".efx"s);
			utils::replace(loc_filepath, "\\", "/");

			return game::R_RegisterModel(loc_filepath.c_str());
		}

		return nullptr;
	}

	void effects_editor_dialog::on_modified(bool modified)
	{
		if (modified)
		{
			m_effect_was_modified = true;

			if (components::effects::effect_is_playing() || components::effects::effect_is_repeating() || components::effects::effect_is_paused())
			{
				components::effects::stop();
				components::effects::apply_changes();

			}
		}
	}

	void effects_editor_dialog::tab_generation(fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;

		ImGui::BeginChild("##effect_properties_generation_child");

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		ImGui::title_with_seperator("Spawning", false, 0, 2.0f, 8.0f);
		{
			int spawn_mode = (elem->editorFlags & fx_system::FX_ED_FLAG_LOOPING) != 0;
			if (ImGui::Combo("Spawning Mode", &spawn_mode, "One-Shot\0Looping\0"))
			{
				// looping
				if (spawn_mode)
				{
					elem->editorFlags |= fx_system::FX_ED_FLAG_LOOPING;
				}
				else
				{
					elem->editorFlags &= ~fx_system::FX_ED_FLAG_LOOPING;
				}

				modified = true;
			}

			ImGui::BeginDisabled(spawn_mode);
			{
				MOD_CHECK(ImGui::DragInt2("Count", reinterpret_cast<int*>(&elem->spawnOneShot.count), 0.25f, 0, UINT16_MAX));
				ImGui::EndDisabled();
			}
			ImGui::BeginDisabled(!spawn_mode);
			{
				MOD_CHECK(ImGui::DragInt("Interval (ms)", &elem->spawnLooping.intervalMsec, 0.25f, 5, UINT16_MAX));
				MOD_CHECK(ImGui::DragInt("Loop Count", &elem->spawnLooping.count, 0.1f, 0, UINT16_MAX));
				ImGui::EndDisabled();
			}

			MOD_CHECK(ImGui::DragInt2("Life (ms)", reinterpret_cast<int*>(&elem->lifeSpanMsec), 0.25f, 0, UINT16_MAX));
			MOD_CHECK(ImGui::DragInt2("Delay (ms)", reinterpret_cast<int*>(&elem->spawnDelayMsec), 0.25f, -UINT16_MAX, UINT16_MAX));
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Range Min/Max", &elem->spawnRange, 0.25f, -FLT_MAX, FLT_MAX, "%.2f"));
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("FadeIn Start/End", &elem->fadeInRange, 0.25f, 0, FLT_MAX, "%.2f"));
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("FadeOut Start/End", &elem->fadeOutRange, 0.25f, 0, FLT_MAX, "%.2f"));

			bool frustum_cull_mode = false;
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Enable Frustum Cull Radius##2", elem, fx_system::FX_ELEM_SPAWN_FRUSTUM_CULL, &frustum_cull_mode));

			ImGui::BeginDisabled(!frustum_cull_mode);
			{
				MOD_CHECK(ImGui::DragFloat("Frustum Cull Radius", &elem->spawnFrustumCullRadius, 0.25f, 0, UINT16_MAX, "%.2f"));
				ImGui::EndDisabled();
			}
		}


		// *------------------------------
		ImGui::title_with_seperator("Origin", true, 0, 2.0f, 8.0f);
		{
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Forward", &elem->spawnOrigin[0], 0.25f, -FLT_MAX, FLT_MAX, "%.2f"));
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Right", &elem->spawnOrigin[1], 0.25f, -FLT_MAX, FLT_MAX, "%.2f"));
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Up", &elem->spawnOrigin[2], 0.25f, -FLT_MAX, FLT_MAX, "%.2f"));

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Origin Relative To Effect Axis", elem, fx_system::FX_ELEM_SPAWN_RELATIVE_TO_EFFECT));

			int origin_offset_mode =
				elem->flags & fx_system::FX_ELEM_SPAWN_OFFSET_NONE ? 0 :
				elem->flags & fx_system::FX_ELEM_SPAWN_OFFSET_SPHERE ? 1 :
				elem->flags & fx_system::FX_ELEM_SPAWN_OFFSET_CYLINDER ? 2 : 0;

			if (ImGui::Combo("Offset", &origin_offset_mode, "No Offset\0Spherical\0Cylindrical\0"))
			{
				switch (origin_offset_mode)
				{
				case 0:
					elem->flags &= ~(fx_system::FX_ELEM_SPAWN_OFFSET_SPHERE | fx_system::FX_ELEM_SPAWN_OFFSET_CYLINDER);
					break;

				case 1:
					elem->flags &= ~fx_system::FX_ELEM_SPAWN_OFFSET_CYLINDER;
					elem->flags |= fx_system::FX_ELEM_SPAWN_OFFSET_SPHERE;
					break;

				case 2:
					elem->flags &= ~fx_system::FX_ELEM_SPAWN_OFFSET_SPHERE;
					elem->flags |= fx_system::FX_ELEM_SPAWN_OFFSET_CYLINDER;
					break;
				}

				modified = true;
			}

			ImGui::BeginDisabled(!origin_offset_mode);
			{
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Radius/Width", &elem->spawnOffsetRadius, 0.25f, -FLT_MAX, FLT_MAX, "%.2f"));
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Height", &elem->spawnOffsetHeight, 0.25f, -FLT_MAX, FLT_MAX, "%.2f"));
				ImGui::EndDisabled();
			}
		}


		// *------------------------------
		ImGui::title_with_seperator("Misc", true, 0, 2.0f, 8.0f);
		{
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Disable Far plane culling (Past Fog)", elem, fx_system::FX_ELEM_DRAW_PAST_FOG));
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Blocks Sight", elem, fx_system::FX_ELEM_BLOCK_SIGHT));
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Draw With Viewmodel", elem, fx_system::FX_ELEM_DRAW_WITH_VIEWMODEL));
			MOD_CHECK(ImGui::DragInt("Sort Order", &elem->sortOrder, 0.1f, -UINT8_MAX, UINT8_MAX));
		}


		// *------------------------------
		ImGui::title_with_seperator("Death Effects", true, 0, 2.0f, 8.0f);
		{
			bool enable_death_effect = false;
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Enable Death Effects", elem, fx_system::FX_ED_FLAG_PLAY_ON_DEATH, &enable_death_effect));

			if (enable_death_effect)
			{
				std::string death_effect_string;
				if (elem->effectOnDeath && elem->effectOnDeath->name)
				{
					death_effect_string = elem->effectOnDeath->name;
				}

				ImGui::InputText("##no_label_death", &death_effect_string, ImGuiInputTextFlags_ReadOnly);
				ImGui::SameLine();
				if (ImGui::Button("..##filepromt_death", ImVec2(28, ImGui::GetFrameHeight())))
				{
					// callback called from :: ggui::file_dialog_frame
					if (dvars::gui_use_new_filedialog->current.enabled)
					{
						std::string path_str;

						const auto egui = GET_GUI(ggui::entity_dialog);
						path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
						path_str += "\\raw\\fx";


						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(FX_EDITOR_DEF);
						file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
						file->set_file_ext(".efx");
						file->set_blocking();
						file->set_callback([]
							{
								const auto dlg = GET_GUI(ggui::file_dialog);

								const std::string replace_path = "raw\\fx\\";

								if (dlg->get_path_result().contains(replace_path))
								{
									std::string loc_filepath = dlg->get_path_result().substr(
										dlg->get_path_result().find(replace_path) + replace_path.length());

									utils::erase_substring(loc_filepath, ".efx"s);
									utils::replace(loc_filepath, "\\", "/");

									if (const auto	_efx = fx_system::get_editor_effect();
													_efx)
									{
										auto* _elem = &_efx->elems[GET_GUI(ggui::effects_editor_dialog)->selected_editor_elemdef];

										if (const auto  death_elem = fx_system::FX_Register(loc_filepath.c_str());
														death_elem)
										{
											_elem->effectOnDeath = death_elem;
											GET_GUI(ggui::effects_editor_dialog)->on_modified(true);
										}
									}
								}
								else
								{
									game::printf_to_console("[ERR] [fx_editor] file not within \"raw\\fx\" directory");
								}
							});

						file->open();
					}
					else if (const auto	death_elem = effectdef_fileprompt();
										death_elem)
					{
						elem->effectOnDeath = death_elem;
						modified = true;
					}
				}

				ImGui::BeginDisabled(!elem->effectOnDeath);
				ImGui::SameLine();
				if (ImGui::Button("x##delete_death", ImVec2(28, ImGui::GetFrameHeight())))
				{
					elem->effectOnDeath = nullptr;
				}
				ImGui::EndDisabled();
			}
		}

		ImGui::title_with_seperator("Emitted Effects", true, 0, 2.0f, 8.0f);
		{
			bool enable_emission = false;
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Emit Effects", elem, fx_system::FX_ED_FLAG_PLAY_ON_RUN, &enable_emission));

			if (enable_emission)
			{
				std::string emit_effect_string;
				if (elem->emission && elem->emission->name)
				{
					emit_effect_string = elem->emission->name;
				}

				ImGui::InputText("##no_label_emission", &emit_effect_string, ImGuiInputTextFlags_ReadOnly);
				ImGui::SameLine();

				if (ImGui::Button("..##filepromt_emission", ImVec2(28, ImGui::GetFrameHeight())))
				{
					// callback called from :: ggui::file_dialog_frame
					if (dvars::gui_use_new_filedialog->current.enabled)
					{
						std::string path_str;

						const auto egui = GET_GUI(ggui::entity_dialog);
						path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
						path_str += "\\raw\\fx";


						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(FX_EDITOR_DEF);
						file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
						file->set_file_ext(".efx");
						file->set_blocking();
						file->set_callback([]
							{
								const auto dlg = GET_GUI(ggui::file_dialog);

								const std::string replace_path = "raw\\fx\\";

								if (dlg->get_path_result().contains(replace_path))
								{
									std::string loc_filepath = dlg->get_path_result().substr(
										dlg->get_path_result().find(replace_path) + replace_path.length());

									utils::erase_substring(loc_filepath, ".efx"s);
									utils::replace(loc_filepath, "\\", "/");

									if (const auto	_efx = fx_system::get_editor_effect();
													_efx)
									{
										auto* _elem = &_efx->elems[GET_GUI(ggui::effects_editor_dialog)->selected_editor_elemdef];

										if (const auto  emit_elem = fx_system::FX_Register(loc_filepath.c_str());
														emit_elem)
										{
											_elem->emission = emit_elem;
											GET_GUI(ggui::effects_editor_dialog)->on_modified(true);
										}
									}
								}
								else
								{
									game::printf_to_console("[ERR] [fx_editor] file not within \"raw\\fx\" directory");
								}
							});

						file->open();
					}
					else if (const auto	emit_elem = effectdef_fileprompt();
										emit_elem)
					{
						elem->emission = emit_elem;
						modified = true;
					}
				}

				ImGui::BeginDisabled(!elem->emission);
				ImGui::SameLine();
				if (ImGui::Button("x##delete_emission", ImVec2(28, ImGui::GetFrameHeight())))
				{
					elem->emission = nullptr;
				}
				ImGui::EndDisabled();
			}
		}

		ImGui::EndChild();
		on_modified(modified);
	}

	void effects_editor_dialog::tab_size([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;
		const ImGuiStyle& style = ImGui::GetStyle();

		const float graph_width = ImGui::GetWindowContentRegionWidth() - (style.FramePadding.x * 2) - 12.0f;
		float graph_height = graph_width;

		if (graph_height > 320) {
			graph_height = 320;
		}

		ImGui::BeginChild("##effect_properties_size_child", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		bool pre_spacing = true;

		if (elem->elemType != fx_system::FX_ELEM_TYPE_MODEL && elem->elemType != fx_system::FX_ELEM_TYPE_SOUND && elem->elemType != fx_system::FX_ELEM_TYPE_RUNNER)
		{
			// false = graph 1, true = 2
			static bool current_graph_width = false;
			ImGui::title_with_seperator(utils::va("Width / Diameter - Graph %d", current_graph_width + 1), false, 0, 2.0f, 8.0f);
			{
				int new_count = 0;
				const auto curve = elem->sizeShape[0][current_graph_width]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

				MOD_CHECK_GRAPH(ImGui::CurveEditor("width_graph", curve->keys, curve->keyCount,
					ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

				if (new_count != curve->keyCount)
				{
					curve->keyCount = new_count;
					modified = true;
				}

				if (ImGui::BeginPopupContextItem("width_graph##bg"))
				{
					if (ImGui::MenuItem("Graph 1", 0, !current_graph_width))
					{
						current_graph_width = false;
					}

					if (ImGui::MenuItem("Graph 2", 0, current_graph_width))
					{
						current_graph_width = true;
					}

					ImGui::EndPopup();
				}

				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##width", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SIZE_0));
				ImGui::SameLine(0, 14.0f);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - style.FramePadding.x);

				if (ImGui::DragFloat("##width_scale", &elem->sizeScale[0], 0.5f, 0.0f, 4000.0f, "%.1f"))
				{
					elem->sizeScale[0] = elem->sizeScale[0] < 0.0f ? 0.0f : elem->sizeScale[0];
					modified = true;

				} TT("Scale");
			}
		}
		else
		{
			pre_spacing = false;
		}

		if (elem->elemType < fx_system::FX_ELEM_TYPE_LAST_SPRITE || elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD)
		{
			// *------------------------------

			// false = graph 1, true = 2
			static bool current_graph_height = false;
			ImGui::title_with_seperator(utils::va("Height / Length - Graph %d", current_graph_height + 1), pre_spacing, 0, 2.0f, 8.0f);

			bool enable_height_graph = false;
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Non-uniform particle size", elem, fx_system::FX_ELEM_NONUNIFORM_SCALE, &enable_height_graph));

			ImGui::BeginDisabled(!enable_height_graph);
			{
				int new_count = 0;
				const auto curve = elem->sizeShape[1][current_graph_height]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

				MOD_CHECK_GRAPH(ImGui::CurveEditor("height_graph", curve->keys, curve->keyCount,
					ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

				if (ImGui::BeginPopupContextItem("height_graph##bg"))
				{
					if (ImGui::MenuItem("Graph 1", 0, !current_graph_height))
					{
						current_graph_height = false;
					}

					if (ImGui::MenuItem("Graph 2", 0, current_graph_height))
					{
						current_graph_height = true;
					}

					ImGui::EndPopup();
				}

				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##height", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SIZE_1));
				ImGui::SameLine(0, 14.0f);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - style.FramePadding.x);

				if (ImGui::DragFloat("##width_scale", &elem->sizeScale[1], 0.5f, 0.0f, 4000.0f, "%.1f"))
				{
					elem->sizeScale[1] = elem->sizeScale[1] < 0.0f ? 0.0f : elem->sizeScale[1];
					modified = true;

				} TT("Scale");

				ImGui::EndDisabled();
			}

			pre_spacing = true;
		}

		if (elem->elemType == fx_system::FX_ELEM_TYPE_MODEL || elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD)
		{
			// *------------------------------

			// false = graph 1, true = 2
			static bool current_graph_scale = false;
			ImGui::title_with_seperator(utils::va("Scale - Graph %d", current_graph_scale + 1), pre_spacing, 0, 2.0f, 8.0f);

			const bool is_scale_graph_aval = elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD || elem->elemType == fx_system::FX_ELEM_TYPE_MODEL;
			ImGui::BeginDisabled(!is_scale_graph_aval);
			{
				int new_count = 0;
				const auto curve = elem->scaleShape[current_graph_scale]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

				MOD_CHECK_GRAPH(ImGui::CurveEditor("scale_graph", curve->keys, curve->keyCount,
					ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

				if (new_count != curve->keyCount)
				{
					curve->keyCount = new_count;
					modified = true;
				}

				if (ImGui::BeginPopupContextItem("scale_graph##bg")) // BeginPopupContextWindow
				{
					if (ImGui::MenuItem("Graph 1", 0, !current_graph_scale))
					{
						current_graph_scale = false;
					}

					if (ImGui::MenuItem("Graph 2", 0, current_graph_scale))
					{
						current_graph_scale = true;
					}

					ImGui::EndPopup();
				}

				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##scale", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SCALE));
				ImGui::SameLine(0, 14.0f);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - style.FramePadding.x);

				if (ImGui::DragFloat("##width_scale", &elem->scaleScale, 0.5f, 0.0f, 4000.0f, "%.1f"))
				{
					elem->scaleScale = elem->scaleScale < 0.0f ? 0.0f : elem->scaleScale;
					modified = true;

				} TT("Scale");
			}
			ImGui::EndDisabled();
		}

		ImGui::EndChild();
		on_modified(modified);
	}

	void effects_editor_dialog::tab_velocity([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;
		const ImGuiStyle& style = ImGui::GetStyle();

		const float graph_width = ImGui::GetWindowContentRegionWidth() - (style.FramePadding.x * 2) - 12.0f;
		float graph_height = graph_width;

		if (graph_height > 320) {
			graph_height = 320;
		}

		ImGui::BeginChild("##effect_properties_velocity_child", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		ImGui::title_with_seperator("General", false, 0, 2.0f, 8.0f);
		{
			ImGui::TextUnformatted("Move relative to:");

			int move_rel_to_flag = 0;

			if (const int masked_flag = elem->flags & fx_system::FX_ELEM_RUN_MASK;
				masked_flag)
			{
				if (masked_flag == fx_system::FX_ELEM_RUN_RELATIVE_TO_SPAWN)
				{
					move_rel_to_flag = 1;
				}
				else if (masked_flag == fx_system::FX_ELEM_RUN_RELATIVE_TO_EFFECT)
				{
					move_rel_to_flag = 2;
				}
				else if (masked_flag == fx_system::FX_ELEM_RUN_RELATIVE_TO_OFFSET)
				{
					move_rel_to_flag = 3;
				}
			}

			const float radio_same_line_offset = ImGui::GetContentRegionAvail().x * 0.55f;
			bool move_rel_to_flag_modified = false;

			if (ImGui::RadioButton("World", &move_rel_to_flag, 0)) {
				move_rel_to_flag_modified = true;
			}

			ImGui::SameLine(radio_same_line_offset);
			if (ImGui::RadioButton("Spawn Offset", &move_rel_to_flag, 3)) {
				move_rel_to_flag_modified = true;
			}

			if (ImGui::RadioButton("Effect at Spawn", &move_rel_to_flag, 1)) {
				move_rel_to_flag_modified = true;
			}

			ImGui::SameLine(radio_same_line_offset);
			if (ImGui::RadioButton("Effect Now", &move_rel_to_flag, 2)) {
				move_rel_to_flag_modified = true;
			}

			// #
			auto assign_movement_flag = [&](const int flag) -> void
			{
				elem->flags &= ~(fx_system::FX_ELEM_RUN_RELATIVE_TO_SPAWN | fx_system::FX_ELEM_RUN_RELATIVE_TO_EFFECT | fx_system::FX_ELEM_RUN_RELATIVE_TO_OFFSET);
				elem->flags |= flag;
			};

			if (move_rel_to_flag_modified)
			{
				// movement flags are multiple of 64
				assign_movement_flag(move_rel_to_flag * 64);
				modified = true;
			}

			SPACING(0.0f, 8.0f);
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Gravity", &elem->gravity, 0.1f, -32768.0f, 32768.0f, "%.2f"));
		}


		// *------------------------------

		auto get_velocity_graph_str = [&](const int i) -> const char*
		{
			switch (i)
			{
			case 0:
				return "Graph 1 - Forward";
			case 1:
				return "Graph 1 - Right";
			case 2:
				return "Graph 1 - Up";
			case 3:
				return "Graph 2 - Forward";
			case 4:
				return "Graph 2 - Right";
			case 5:
				return "Graph 2 - Up";
			default:
				return "";
			}
		};

		const auto vert_center_offset = (ImGui::GetFrameHeight() - ImGui::CalcTextSize("A").y) * 0.4f;

		{
			// 0 = graph1, 1 = graph2
			static int velocity_1_graph = 0;

			// 0 = forward, 1 = right, 2 = up
			static int velocity_1_vector = 0;

			ImGui::title_with_seperator(utils::va("Velocity - %s", get_velocity_graph_str(3 * velocity_1_graph + velocity_1_vector)), true, 0, 2.0f, 8.0f);
			{
				int new_count = 0;
				const auto curve = elem->velShape[0][velocity_1_vector][velocity_1_graph]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

				MOD_CHECK_GRAPH(ImGui::CurveEditor("velocity_1_graph", curve->keys, curve->keyCount,
					ImVec2(0.0f, -0.5f), ImVec2(1.0f, 0.5f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

				if (new_count != curve->keyCount)
				{
					curve->keyCount = new_count;
					modified = true;
				}

				if (ImGui::BeginPopupContextItem("velocity_1_graph##bg"))
				{
					if (ImGui::MenuItem(get_velocity_graph_str(0), 0, !velocity_1_graph && !velocity_1_vector))
					{
						velocity_1_graph = 0;
						velocity_1_vector = 0;
					}

					if (ImGui::MenuItem(get_velocity_graph_str(1), 0, !velocity_1_graph && velocity_1_vector == 1))
					{
						velocity_1_graph = 0;
						velocity_1_vector = 1;
					}

					if (ImGui::MenuItem(get_velocity_graph_str(2), 0, !velocity_1_graph && velocity_1_vector == 2))
					{
						velocity_1_graph = 0;
						velocity_1_vector = 2;
					}

					// -

					if (ImGui::MenuItem(get_velocity_graph_str(3), 0, velocity_1_graph && !velocity_1_vector))
					{
						velocity_1_graph = 1;
						velocity_1_vector = 0;
					}

					if (ImGui::MenuItem(get_velocity_graph_str(4), 0, velocity_1_graph && velocity_1_vector == 1))
					{
						velocity_1_graph = 1;
						velocity_1_vector = 1;
					}

					if (ImGui::MenuItem(get_velocity_graph_str(5), 0, velocity_1_graph && velocity_1_vector == 2))
					{
						velocity_1_graph = 1;
						velocity_1_vector = 2;
					}

					ImGui::EndPopup();
				}

				ImGui::left_label_drag("Forward", vert_center_offset);
				MOD_CHECK(ImGui::DragFloat("##forward_velocity1", &elem->velScale[0][0], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

				ImGui::left_label_drag("Right", vert_center_offset);
				MOD_CHECK(ImGui::DragFloat("##right_velocity1", &elem->velScale[0][1], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

				ImGui::left_label_drag("Up", vert_center_offset);
				MOD_CHECK(ImGui::DragFloat("##up_velocity1", &elem->velScale[0][2], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

				ImGui::Spacing();

				static float checkbox1_width = 100.0f;
				static float checkbox2_width = 100.0f;

				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox1_width - style.FramePadding.x);
				ImGui::BeginGroup();
				{
					ImGui::left_label_checkbox("Randomize between Graph 1 and 2", vert_center_offset);
					MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity1_random", elem, fx_system::FX_ED_FLAG_USE_RANDOM_VELOCITY_0));

					ImGui::EndGroup();
					checkbox1_width = ImGui::GetItemRectSize().x;
				}

				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox2_width - style.FramePadding.x);
				ImGui::BeginGroup();
				{
					ImGui::left_label_checkbox("Relative to effect axis", vert_center_offset);
					MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity1_effect_axis", elem, fx_system::FX_ED_FLAG_ABSOLUTE_VELOCITY_0, nullptr, true));

					ImGui::EndGroup();
					checkbox2_width = ImGui::GetItemRectSize().x;
				}
			}
		}


		// *------------------------------
		{
			// 0 = graph1, 1 = graph2
			static int velocity_2_graph = 0;

			// 0 = forward, 1 = right, 2 = up
			static int velocity_2_vector = 0;

			ImGui::title_with_seperator(utils::va("Velocity 2 - %s", get_velocity_graph_str(3 * velocity_2_graph + velocity_2_vector)), true, 0, 2.0f, 8.0f);

			int new_count = 0;
			const auto curve = elem->velShape[1][velocity_2_vector][velocity_2_graph]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

			MOD_CHECK_GRAPH(ImGui::CurveEditor("velocity_2_graph", curve->keys, curve->keyCount,
				ImVec2(0.0f, -0.5f), ImVec2(1.0f, 0.5f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

			if (new_count != curve->keyCount)
			{
				curve->keyCount = new_count;
				modified = true;
			}

			if (ImGui::BeginPopupContextItem("velocity_2_graph##bg"))
			{
				if (ImGui::MenuItem(get_velocity_graph_str(0), 0, !velocity_2_graph && !velocity_2_vector))
				{
					velocity_2_graph = 0;
					velocity_2_vector = 0;
				}

				if (ImGui::MenuItem(get_velocity_graph_str(1), 0, !velocity_2_graph && velocity_2_vector == 1))
				{
					velocity_2_graph = 0;
					velocity_2_vector = 1;
				}

				if (ImGui::MenuItem(get_velocity_graph_str(2), 0, !velocity_2_graph && velocity_2_vector == 2))
				{
					velocity_2_graph = 0;
					velocity_2_vector = 2;
				}

				// -

				if (ImGui::MenuItem(get_velocity_graph_str(3), 0, velocity_2_graph && !velocity_2_vector))
				{
					velocity_2_graph = 1;
					velocity_2_vector = 0;
				}

				if (ImGui::MenuItem(get_velocity_graph_str(4), 0, velocity_2_graph && velocity_2_vector == 1))
				{
					velocity_2_graph = 1;
					velocity_2_vector = 1;
				}

				if (ImGui::MenuItem(get_velocity_graph_str(5), 0, velocity_2_graph && velocity_2_vector == 2))
				{
					velocity_2_graph = 1;
					velocity_2_vector = 2;
				}

				ImGui::EndPopup();
			}

			ImGui::left_label_drag("Forward", vert_center_offset);
			MOD_CHECK(ImGui::DragFloat("##forward_velocity2", &elem->velScale[1][0], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

			ImGui::left_label_drag("Right", vert_center_offset);
			MOD_CHECK(ImGui::DragFloat("##right_velocity2", &elem->velScale[1][1], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

			ImGui::left_label_drag("Up", vert_center_offset);
			MOD_CHECK(ImGui::DragFloat("##up_velocity2", &elem->velScale[1][2], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

			ImGui::Spacing();

			static float checkbox1_offset = 100.0f;
			static float checkbox2_offset = 100.0f;
			static float checkbox3_offset = 100.0f;

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox1_offset - style.FramePadding.x);
			ImGui::BeginGroup();
			{
				ImGui::left_label_checkbox("Randomize between Graph 1 and 2", vert_center_offset);
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity2_random", elem, fx_system::FX_ED_FLAG_USE_RANDOM_VELOCITY_1));

				ImGui::EndGroup();
				checkbox1_offset = ImGui::GetItemRectSize().x;
			}

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox2_offset - style.FramePadding.x);
			ImGui::BeginGroup();
			{
				ImGui::left_label_checkbox("Use deprecated randomize behavior", vert_center_offset);
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity2_backcomp", elem, fx_system::FX_ED_FLAG_BACKCOMPAT_VELOCITY));

				ImGui::EndGroup();
				checkbox2_offset = ImGui::GetItemRectSize().x;
			}

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox3_offset - style.FramePadding.x);
			ImGui::BeginGroup();
			{
				ImGui::left_label_checkbox("Relative to effect axis", vert_center_offset);
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity2_effect_axis", elem, fx_system::FX_ED_FLAG_ABSOLUTE_VELOCITY_1, nullptr, true));

				ImGui::EndGroup();
				checkbox3_offset = ImGui::GetItemRectSize().x;
			}
		}

		ImGui::EndChild();
		on_modified(modified);
	}

	void effects_editor_dialog::tab_rotation([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;
		const ImGuiStyle& style = ImGui::GetStyle();

		const float graph_width = ImGui::GetWindowContentRegionWidth() - (style.FramePadding.x * 2) - 12.0f;
		float graph_height = graph_width;

		if (graph_height > 320) {
			graph_height = 320;
		}

		ImGui::BeginChild("##effect_properties_rotation_child", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		// false = graph 1, true = 2
		static bool current_graph_rotation = false;
		ImGui::title_with_seperator(utils::va("Rotation - Graph %d", current_graph_rotation + 1), false, 0, 2.0f, 8.0f);
		{
			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Initial Rotation", &elem->initialRotation, 0.1f, -360.0f, 360.0f, "%.2f"));

			int new_count = 0;
			const auto curve = elem->rotationShape[current_graph_rotation];

			MOD_CHECK_GRAPH(ImGui::CurveEditor("rotation_graph", curve->keys, curve->keyCount,
				ImVec2(0.0f, -0.5f), ImVec2(1.0f, 0.5f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

			if (new_count != curve->keyCount)
			{
				curve->keyCount = new_count;
				modified = true;
			}

			if (ImGui::BeginPopupContextItem("rotation_graph##bg"))
			{
				if (ImGui::MenuItem("Graph 1", 0, !current_graph_rotation))
				{
					current_graph_rotation = false;
				}

				if (ImGui::MenuItem("Graph 2", 0, current_graph_rotation))
				{
					current_graph_rotation = true;
				}

				ImGui::EndPopup();
			}

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##height", elem, fx_system::FX_ED_FLAG_USE_RANDOM_ROTATION_DELTA));
			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - style.FramePadding.x);
			MOD_CHECK(ImGui::DragFloat("##rotation_scale", &elem->rotationScale, 0.5f, 0, 0, "%.1f")); TT("Scale");
		}

		if (elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD || elem->elemType == fx_system::FX_ELEM_TYPE_MODEL)
		{
			// *------------------------------
			ImGui::title_with_seperator("Spawn Angles", true, 0, 2.0f, 8.0f);
			{
				ImGui::PushID("Spawn");
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Pitch", &elem->spawnAngles[0], 0.1f, -360.0f, 360.0f, "%.2f"));
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Yaw", &elem->spawnAngles[1], 0.1f, -360.0f, 360.0f, "%.2f"));
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Roll", &elem->spawnAngles[2], 0.1f, -360.0f, 360.0f, "%.2f"));
				ImGui::PopID();
			}

			// *------------------------------
			ImGui::title_with_seperator("Angle Delta / Angular Velocity", true, 0, 2.0f, 8.0f);
			{
				ImGui::PushID("Delta");
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Pitch", &elem->angularVelocity[0], 0.1f, -360.0f, 360.0f, "%.2f"));
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Yaw", &elem->angularVelocity[1], 0.1f, -360.0f, 360.0f, "%.2f"));
				MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Roll", &elem->angularVelocity[2], 0.1f, -360.0f, 360.0f, "%.2f"));
				ImGui::PopID();
			}
		}

		ImGui::EndChild();
		on_modified(modified);
	}

	void effects_editor_dialog::tab_physics([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;

		ImGui::BeginChild("##effect_properties_color_child", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		bool legacy_physics_enabled = false;
		MOD_CHECK(ImGui::Checkbox_FxElemFlag("Enable Physics (General)", elem, fx_system::FX_ELEM_USE_COLLISION, &legacy_physics_enabled));

		bool physx_enabled = false;
		if (elem->elemType == fx_system::FX_ELEM_TYPE_MODEL)
		{
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Enable simulation (PhysX / ODE)", elem, fx_system::FX_ELEM_USE_MODEL_PHYSICS, &physx_enabled));
			TT("This enables PhysX (in radiant) and the stock physics engine in cod4");
		}

		if (physx_enabled)
		{
			SPACING(0.0f, 8.0f);

			int color_count = 0;
			imgui::PushStyleColor(ImGuiCol_Button, ImVec4(0.44f, 0.69f, 0.0f, 1.0f)); color_count++;
			imgui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.40f, 0.72f, 0.10f, 1.0f)); color_count++;
			imgui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.73f, 0.10f, 1.0f)); color_count++;
			imgui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f)); color_count++;
			imgui::PushFontFromIndex(ggui::BOLD_18PX);

			if (imgui::Button("Open PhysX Settings", ImVec2(imgui::GetWindowWidth() - 40.0f, imgui::GetFrameHeight())))
			{
				const auto cs = GET_GUI(ggui::camera_settings_dialog);
				cs->handle_toggle_request(camera_settings_dialog::tab_state_effects);
				cs->focus_tab(camera_settings_dialog::tab_state_effects);
			}

			imgui::PopFont();
			imgui::PopStyleColor(color_count);
		}

		if (legacy_physics_enabled)
		{
			ImGui::title_with_seperator("Legacy physics settings unrelated to PhysX", true, 0, 2.0f, 8.0f);

			MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Bounce / Elasticity", &elem->elasticity, 0.1f, 0.0f, 1.0f, "%.2f"));

			ImGui::title_with_seperator("Impacts", true, 0, 2.0f, 8.0f);
			{
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Kill effect on impact", elem, fx_system::FX_ELEM_DIE_ON_TOUCH));
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Collide with item clip", elem, fx_system::FX_ED_FLAG_USE_ITEM_CLIP));

				bool play_on_impact = false;
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Play new effect on impact", elem, fx_system::FX_ED_FLAG_PLAY_ON_TOUCH, &play_on_impact));

				if (play_on_impact)
				{
					std::string impact_effect_string;
					if (elem->effectOnImpact && elem->effectOnImpact->name)
					{
						impact_effect_string = elem->effectOnImpact->name;
					}

					ImGui::InputText("##no_label_impact", &impact_effect_string, ImGuiInputTextFlags_ReadOnly);
					ImGui::SameLine();

					if (ImGui::Button("..##filepromt_impact", ImVec2(28, ImGui::GetFrameHeight())))
					{
						// callback called from :: ggui::file_dialog_frame
						if (dvars::gui_use_new_filedialog->current.enabled)
						{
							std::string path_str;

							const auto egui = GET_GUI(ggui::entity_dialog);
							path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
							path_str += "\\raw\\fx";


							const auto file = GET_GUI(ggui::file_dialog);
							file->set_default_path(path_str);
							file->set_file_handler(FX_EDITOR_DEF);
							file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
							file->set_file_ext(".efx");
							file->set_blocking();
							file->set_callback([]
								{
									const auto dlg = GET_GUI(ggui::file_dialog);

									const std::string replace_path = "raw\\fx\\";

									if (dlg->get_path_result().contains(replace_path))
									{
										std::string loc_filepath = dlg->get_path_result().substr(
											dlg->get_path_result().find(replace_path) + replace_path.length());

										utils::erase_substring(loc_filepath, ".efx"s);
										utils::replace(loc_filepath, "\\", "/");

										if (const auto	_efx = fx_system::get_editor_effect();
														_efx)
										{
											auto* _elem = &_efx->elems[GET_GUI(ggui::effects_editor_dialog)->selected_editor_elemdef];

											if (const auto  impact_elem = fx_system::FX_Register(loc_filepath.c_str());
															impact_elem)
											{
												_elem->effectOnImpact = impact_elem;
												GET_GUI(ggui::effects_editor_dialog)->on_modified(true);
											}
										}
									}
									else
									{
										game::printf_to_console("[ERR] [fx_editor] file not within \"raw\\fx\" directory");
									}
								});

							file->open();
						}
						else if (const auto	impact_elem = effectdef_fileprompt();
											impact_elem)
						{
							elem->effectOnImpact = impact_elem;
							modified = true;
						}
					}

					ImGui::BeginDisabled(!elem->effectOnImpact);
					ImGui::SameLine();
					if (ImGui::Button("x##delete_impact", ImVec2(28, ImGui::GetFrameHeight())))
					{
						elem->effectOnImpact = nullptr;
					}
					ImGui::EndDisabled();
				}
			}

			// *------------------------------
			ImGui::title_with_seperator("Bounding Box", true, 0, 2.0f, 8.0f);
			{
				bool bounding_box_enabled = false;
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Enable physics bounding sphere", elem, fx_system::FX_ED_FLAG_BOUNDING_SPHERE, &bounding_box_enabled));

				if (bounding_box_enabled)
				{
					MOD_CHECK(ImGui::DragFloat3("Origin", elem->collOffset, 0.1f, -FLT_MIN, FLT_MAX, "%.2f"));
					MOD_CHECK(ImGui::DragFloat("Radius", &elem->collRadius, 0.1f, 0.0f, 2048.0f, "%.2f"));
				}
			}
		}

		
		ImGui::EndChild();
		on_modified(modified);
	}

	void effects_editor_dialog::tab_color([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;
		const ImGuiStyle& style = ImGui::GetStyle();

		const float graph_width = ImGui::GetWindowContentRegionWidth() - (style.FramePadding.x * 2) - 12.0f;
		float graph_height = graph_width;

		if (graph_height > 320)
		{
			graph_height = 320;
		}

		ImGui::BeginChild("##effect_properties_color_child", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Indent(8.0f);
		ImGui::Spacing();


		ImGui::title_with_seperator("RGB Color", false, 0, 2.0f, 8.0f);
		{
			if (elem && elem->color[0] && elem->color[0]->keyCount && elem->color[1] && elem->color[1]->keyCount)
			{
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##color", elem, fx_system::FX_ED_FLAG_USE_RANDOM_COLOR));
				ImGui::SameLine(0, 14.0f);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - style.FramePadding.x);

				static int color_graph_idx = false;
				ImGui::Combo("##graph_selection_color", &color_graph_idx, "Graph 1\0Graph 2\0");
				ImGui::Spacing();

				{
					std::uint32_t key = 0;
					for (auto& mark : gradient.getMarks())
					{
						const float* y_offset = &elem->color[color_graph_idx]->keys[key * 4];
						const float* rgb = &elem->color[color_graph_idx]->keys[key * 4] + 1;

						mark->position = *y_offset;

						if (key == 0)
						{
							mark->position = 0.0f;
						}
						else if (key == gradient.getMarks().size() - 1)
						{
							mark->position = 1.0f;
						}

						mark->color[0] = rgb[0];
						mark->color[1] = rgb[1];
						mark->color[2] = rgb[2];
						mark->color[3] = rgb[3];
						key++;
					}
				}

				if (ImGui::GradientEditor("test_bar", &gradient))
				{
					int key_idx = 0;
					for (auto& mark : gradient.getMarks())
					{
						float* y_offset = &elem->color[color_graph_idx]->keys[key_idx * 4];
						float* rgb = &elem->color[color_graph_idx]->keys[key_idx * 4] + 1;

						*y_offset = mark->position;
						rgb[0] = mark->color[0];
						rgb[1] = mark->color[1];
						rgb[2] = mark->color[2];

						key_idx++;
					}

					if (elem->color[color_graph_idx]->keyCount != key_idx)
					{
						game::printf_to_console("graph keycount changed from [ %d ] to [ %d ]", elem->color[color_graph_idx]->keyCount, key_idx);
					}

					elem->color[color_graph_idx]->keyCount = key_idx;

					modified = true;
				}
			}
		}


		// *------------------------------

		// false = graph 1, true = 2
		static bool current_graph_alpha = false;
		ImGui::title_with_seperator(utils::va("Alpha - Graph %d", current_graph_alpha + 1), true, 0, 2.0f, 8.0f);
		{
			if (elem && elem->alpha[0] && elem->alpha[0]->keyCount && elem->alpha[1] && elem->alpha[1]->keyCount)
			{
				int new_count = 0;
				const auto curve = elem->alpha[current_graph_alpha]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

				MOD_CHECK_GRAPH(ImGui::CurveEditor("alpha_graph", curve->keys, curve->keyCount,
					ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

				if (new_count != curve->keyCount)
				{
					curve->keyCount = new_count;
					modified = true;
				}

				if (ImGui::BeginPopupContextItem("alpha_graph##bg"))
				{
					if (ImGui::MenuItem("Graph 1", 0, !current_graph_alpha))
					{
						current_graph_alpha = false;
					}

					if (ImGui::MenuItem("Graph 2", 0, current_graph_alpha))
					{
						current_graph_alpha = true;
					}

					ImGui::EndPopup();
				}

				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##alpha", elem, fx_system::FX_ED_FLAG_USE_RANDOM_ALPHA));
			}

			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - style.FramePadding.x);
			MOD_CHECK(ImGui::DragFloat("##lighting_fraction", &elem->scaleScale, 0.01f, 0, 1.0f, "%.1f")); TT("Lighting Fraction");
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Modulate RGB values using alpha value", elem, fx_system::FX_ED_FLAG_MODULATE_COLOR_BY_ALPHA));
		}

		ImGui::EndChild();
		on_modified(modified);
	}

	void effects_editor_dialog::tab_visuals(fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;
		const ImGuiStyle& style = ImGui::GetStyle();

		const float graph_width = ImGui::GetWindowContentRegionWidth() - (style.FramePadding.x * 2) - 12.0f;
		float graph_height = graph_width;

		if (graph_height > 320) {
			graph_height = 320;
		}

		ImGui::BeginChild("##effect_properties_velocity_child", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Indent(8.0f);
		ImGui::Spacing();

		ImGui::title_with_seperator("General", false, 0, 2.0f, 8.0f);
		{
			int elem_type = static_cast<std::uint8_t>(elem->elemType);
			if (ImGui::Combo("Element Type", &elem_type, "Billboard\0Oriented Sprite\0Tail\0Geometry Trail\0Particle Cloud\0Model\0Light\0Spot Light\0Sound\0Decal\0FX Runner\0"))
			{
				// do not erase visuals if old and new elem type uses materials
				if ((elem->elemType <= fx_system::FX_ELEM_TYPE_LAST_SPRITE || elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD)
					&& (elem_type <= fx_system::FX_ELEM_TYPE_LAST_SPRITE || elem_type == fx_system::FX_ELEM_TYPE_CLOUD))
				{
					elem->elemType = static_cast<char>(elem_type);
					modified = true;
				}
				else
				{
					memset(&elem->u.visuals[0], 0, 32 * sizeof(fx_system::FxElemDefVisuals));
					elem->visualCount = 0;
				}

				elem->elemType = static_cast<char>(elem_type);
				modified = true;
			}

			if (elem->elemType == fx_system::FX_ELEM_TYPE_RUNNER)
			{
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("Rotate randomly around forward axis", elem, fx_system::FX_ELEM_RUNNER_USES_RAND_ROT));
			}
		}

		// *------------------------------
		ImGui::title_with_seperator("Visuals / Effect Materials", true, 0, 2.0f, 8.0f);

		struct visuals_helper_s
		{
			const char* name;
			int visual_index;
		};

		std::vector<visuals_helper_s> vis;
		static std::uint32_t vis_current_idx = 0;

		for (auto m = 0; m < elem->visualCount; m++)
		{
			if (elem->elemType <= fx_system::FX_ELEM_TYPE_LAST_SPRITE || elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD)
			{
				if (elem->u.visuals[m].material && elem->u.visuals[m].material->info.name)
				{
					vis.push_back({ elem->u.visuals[m].material->info.name, m });
				}
			}
			else if (elem->elemType == fx_system::FX_ELEM_TYPE_MODEL)
			{
				if (elem->u.visuals[m].model && elem->u.visuals[m].model->name)
				{
					vis.push_back({ elem->u.visuals[m].model->name, m });
				}
			}
			else if (elem->elemType == fx_system::FX_ELEM_TYPE_RUNNER)
			{
				if (elem->u.visuals[m].effectDef.handle && elem->u.visuals[m].effectDef.handle->name)
				{
					vis.push_back({ elem->u.visuals[m].effectDef.handle->name, m });
				}
			}
			else if (elem->elemType == fx_system::FX_ELEM_TYPE_SOUND)
			{
				if (elem->u.visuals[m].soundName)
				{
					vis.push_back({ elem->u.visuals[m].soundName, m });
				}
			}
			/*else if (elem->elemType == fx_system::FX_ELEM_TYPE_DECAL)
			{
				if (elem->u.markVisuals[m].materials[0] && elem->u.markVisuals[m].materials[0]->info.name)
				{
					vis.push_back({ elem->u.markVisuals[m].materials[0]->info.name, m });
				}
			}*/
			else
			{
				//listbox_enabled = false;
				break;
			}
		}

		bool listbox_enabled = true;
		if (elem->elemType == fx_system::FX_ELEM_TYPE_SOUND || elem->elemType == fx_system::FX_ELEM_TYPE_DECAL || elem->elemType == fx_system::FX_ELEM_TYPE_OMNI_LIGHT || elem->elemType == fx_system::FX_ELEM_TYPE_SPOT_LIGHT)
		{
			listbox_enabled = false;
		}

		const float listbox_height = 250.0f;

		ImGui::BeginDisabled(!listbox_enabled);
		{
			if (ImGui::BeginListBox("##visuals_listbox", ImVec2(0, listbox_height)))
			{
				if (vis_current_idx >= vis.size())
				{
					vis_current_idx = 0;
				}

				for (std::uint32_t n = 0; n < vis.size(); n++)
				{
					const bool is_selected = (vis_current_idx == n);
					if (ImGui::Selectable(vis[n].name, is_selected))
					{
						vis_current_idx = n;
					}

					// initial focus
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndListBox();
			}

			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		const auto post_listbox_cursor = ImGui::GetCursorPos();

		const bool fileprompt_enabled = elem->visualCount < 32 && listbox_enabled;
		ImGui::BeginDisabled(!fileprompt_enabled);
		{
			if (ImGui::Button("..##filepromt", ImVec2(28, ImGui::GetFrameHeight())))
			{
				if (elem->elemType <= fx_system::FX_ELEM_TYPE_LAST_SPRITE || elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD)
				{
					// callback called from :: ggui::file_dialog_frame
					if (dvars::gui_use_new_filedialog->current.enabled)
					{
						std::string path_str;

						const auto egui = GET_GUI(ggui::entity_dialog);
						path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
						path_str += "\\raw\\materials";


						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(FX_EDITOR_MATERIAL);
						file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
						file->set_file_ext("");
						file->set_blocking();
						file->set_callback([]
						{
								const auto dlg = GET_GUI(ggui::file_dialog);

								const std::string replace_path = "raw\\materials\\";

								if (dlg->get_path_result().contains(replace_path))
								{
									std::string loc_filepath = dlg->get_path_result().substr(
										dlg->get_path_result().find(replace_path) + replace_path.length());

									utils::replace(loc_filepath, "\\", "/");

									if (const auto	_efx = fx_system::get_editor_effect();
													_efx)
									{
										auto* _elem = &_efx->elems[GET_GUI(ggui::effects_editor_dialog)->selected_editor_elemdef];

										if (const auto  material = game::Material_RegisterHandle(loc_filepath.c_str(), 0);
														material && _elem->visualCount < 32)
										{
											_elem->u.visuals[_elem->visualCount].material = material;
											_elem->visualCount++;

											GET_GUI(ggui::effects_editor_dialog)->on_modified(true);
										}
									}
								}
								else
								{
									game::printf_to_console("[ERR] [fx_editor] file not within \"raw\\materials\" directory");
								}
						});

						file->open();
					}
					else if (const auto material = material_fileprompt();
										material && elem->visualCount < 32)
					{
						elem->u.visuals[elem->visualCount].material = material;
						elem->visualCount++;

						modified = true;
					}
				}
				else if (elem->elemType == fx_system::FX_ELEM_TYPE_MODEL)
				{
					// callback called from :: ggui::file_dialog_frame
					if (dvars::gui_use_new_filedialog->current.enabled)
					{
						std::string path_str;

						const auto egui = GET_GUI(ggui::entity_dialog);
						path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
						path_str += "\\raw\\xmodel";


						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(FX_EDITOR_MODEL);
						file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
						file->set_file_ext("");
						file->set_blocking();
						file->set_callback([]
							{
								const auto dlg = GET_GUI(ggui::file_dialog);

								const std::string replace_path = "raw\\xmodel\\";

								if(dlg->get_path_result().contains(replace_path))
								{
									std::string loc_filepath = dlg->get_path_result().substr(
										dlg->get_path_result().find(replace_path) + replace_path.length());

									utils::replace(loc_filepath, "\\", "/");

									if (const auto	_efx = fx_system::get_editor_effect();
													_efx)
									{
										auto* _elem = &_efx->elems[GET_GUI(ggui::effects_editor_dialog)->selected_editor_elemdef];

										if (const auto  model = game::R_RegisterModel(loc_filepath.c_str());
														model && _elem->visualCount < 32)
										{
											_elem->u.visuals[_elem->visualCount].model = model;
											_elem->visualCount++;

											GET_GUI(ggui::effects_editor_dialog)->on_modified(true);
										}
									}
								}
								else
								{
									game::printf_to_console("[ERR] [fx_editor] file not within \"raw\\xmodel\" directory");
								}
							});

						file->open();
					}
					else if (const auto	model = xmodel_fileprompt();
										model)
					{
						elem->u.visuals[elem->visualCount].model = model;
						elem->visualCount++;

						modified = true;
					}
				}
				else if (elem->elemType == fx_system::FX_ELEM_TYPE_RUNNER)
				{
					// callback called from :: ggui::file_dialog_frame
					if (dvars::gui_use_new_filedialog->current.enabled)
					{
						std::string path_str;

						const auto egui = GET_GUI(ggui::entity_dialog);
						path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
						path_str += "\\raw\\fx";


						const auto file = GET_GUI(ggui::file_dialog);
						file->set_default_path(path_str);
						file->set_file_handler(FX_EDITOR_DEF);
						file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
						file->set_file_ext(".efx");
						file->set_blocking();
						file->set_callback([]
							{
								const auto dlg = GET_GUI(ggui::file_dialog);

								const std::string replace_path = "raw\\fx\\";

								if(dlg->get_path_result().contains(replace_path))
								{
									std::string loc_filepath = dlg->get_path_result().substr(
										dlg->get_path_result().find(replace_path) + replace_path.length());

									utils::erase_substring(loc_filepath, ".efx"s);
									utils::replace(loc_filepath, "\\", "/");

									if (const auto	_efx = fx_system::get_editor_effect();
													_efx)
									{
										auto* _elem = &_efx->elems[GET_GUI(ggui::effects_editor_dialog)->selected_editor_elemdef];

										if (const auto  effect_def = fx_system::FX_Register(loc_filepath.c_str());
														effect_def && _elem->visualCount < 32)
										{
											_elem->u.visuals[_elem->visualCount].effectDef.handle = effect_def;
											_elem->visualCount++;

											GET_GUI(ggui::effects_editor_dialog)->on_modified(true);
										}
									}
								}
								else
								{
									game::printf_to_console("[ERR] [fx_editor] file not within \"raw\\fx\" directory");
								}
							});

						file->open();
					}
					else if (const auto	effect_def = effectdef_fileprompt();
										effect_def)
					{
						elem->u.visuals[elem->visualCount].effectDef.handle = effect_def;
						elem->visualCount++;

						modified = true;
					}
				}

			}

			ImGui::EndDisabled();
		}

		ImGui::SetCursorPos(ImVec2(post_listbox_cursor.x, post_listbox_cursor.y + ImGui::GetFrameHeight() + 4.0f));

		const bool can_delete_visuals = elem->visualCount > 0 && listbox_enabled;
		ImGui::BeginDisabled(!can_delete_visuals);
		{
			if (ImGui::Button("x##delete_vis", ImVec2(28, ImGui::GetFrameHeight())))
			{
				if (vis_current_idx >= vis.size())
				{
					vis_current_idx = 0;
				}

				if (elem->visualCount > 0 && vis_current_idx < vis.size())
				{
					// if last
					if (vis_current_idx == static_cast<std::uint32_t>(elem->visualCount - 1))
					{
						memset(&elem->u.visuals[vis_current_idx], 0, sizeof(fx_system::FxElemVisuals));
						elem->visualCount--;
					}
					else
					{
						const auto amount_to_copy = elem->visualCount - 1 - vis_current_idx;
						memcpy(&elem->u.visuals[vis_current_idx], &elem->u.visuals[vis_current_idx + 1], amount_to_copy * sizeof(fx_system::FxElemVisuals));
						memset(&elem->u.visuals[vis_current_idx + amount_to_copy], 0, amount_to_copy * sizeof(fx_system::FxElemVisuals));
						elem->visualCount--;
					}
				}
			}

			ImGui::EndDisabled();
		}

		ImGui::SetCursorPos(ImVec2(post_listbox_cursor.x, post_listbox_cursor.y + listbox_height));

		if (elem->elemType == fx_system::FX_ELEM_TYPE_TRAIL)
		{
			// *------------------------------
			ImGui::title_with_seperator("Geometry Trail", true, 0, 2.0f, 8.0f);

			// *
			// prepare data for the shape editor

			// max 64 verts
			// max 128 indices

			// continuous array of geotrail points/vertices
			float traildef_point_array[2 * 64] = {};

			for (auto vert = 0; vert < elem->trailDef.vertCount && vert < 64; vert++)
			{
				memcpy(&traildef_point_array[vert * 2], elem->trailDef.verts[vert].pos, sizeof(float[2]));
			}

			int trail_shapes_count = 0; // amount of individual shapes
			int shape_vertex_count = 1; // vertex count for current shape in the loop

			// individual shapes
			ImGui::traildef_shape_s trail_shapes[8] = {};

			for (auto ind_idx = 1; ind_idx < elem->trailDef.indCount && ind_idx < 128; )
			{
				// if reached end of indices -> last shape
				if (ind_idx + 1 < elem->trailDef.indCount)
				{
					// count vertices of current shape or create the next shape
					if (elem->trailDef.inds[ind_idx] == elem->trailDef.inds[ind_idx + 1])
					{
						ind_idx += 2;
						shape_vertex_count++;

						continue;
					}
				}

				// +
				// new shape 

				if (trail_shapes_count == 0)
				{
					// first shape
					auto shape_curr = &trail_shapes[trail_shapes_count];

					shape_curr->index = trail_shapes_count;

					shape_curr->offset_vertex = 0;
					shape_curr->num_vertex = shape_vertex_count + 1;

					shape_curr->offset_indices = 0;
					shape_curr->num_indices = shape_vertex_count;

					trail_shapes_count++;


					// prepare second/next shape
					shape_curr = &trail_shapes[trail_shapes_count];
					const auto shape_prev = &trail_shapes[trail_shapes_count - 1];

					shape_curr->index = trail_shapes_count;
					shape_curr->offset_vertex = shape_prev->num_vertex;
					shape_curr->offset_indices = shape_prev->num_indices * 2;

					ind_idx += 2; // advance indices
					shape_vertex_count = 1; // reset vertex counter
				}
				else
				{
					// second shape and up
					auto shape_curr = &trail_shapes[trail_shapes_count];

					shape_curr->num_vertex = shape_vertex_count + 1;
					shape_curr->num_indices = shape_vertex_count;

					trail_shapes_count++;

					if (shape_curr->offset_indices + (shape_curr->num_indices * 2) < elem->trailDef.indCount)
					{
						// +
						// prepare next shape

						if (trail_shapes_count < 8)
						{
							shape_curr = &trail_shapes[trail_shapes_count];
							const auto shape_prev = &trail_shapes[trail_shapes_count - 1];

							shape_curr->index = trail_shapes_count;
							shape_curr->offset_vertex = shape_prev->offset_vertex + shape_prev->num_vertex;
							shape_curr->offset_indices = shape_prev->offset_indices + shape_prev->num_indices;
						}
						else
						{
							break;
						}
					}

					ind_idx += 2; // advance indices
					shape_vertex_count = 1; // reset vertex counter
				}
			}


			// currently hovered graph point
			int hovered_point = -1;

			MOD_CHECK_GRAPH(ImGui::CurveEditorShapes("traildef_shape", traildef_point_array, trail_shapes, trail_shapes_count,
				ImVec2(-1.0f, -1.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &hovered_point));


			bool context_menu_open = ImGui::BeginPopupContextItem("traildef_shape##context");

			// logic to capture the first active context-menu frame
			static int  saved_hovered_point_on_context = -1;
			static bool context_menu_tracked_state = false;
			bool		context_menu_initial_frame = false;

			if (context_menu_open != context_menu_tracked_state)
			{
				context_menu_initial_frame = true;
			}

			context_menu_tracked_state = context_menu_open;

			// --

			if (context_menu_open)
			{
				if (hovered_point >= 0 || saved_hovered_point_on_context >= 0)
				{
					// only save the hovered point within the first context-menu frame to avoid shape switching
					// if the user hovers another point while the context menu is open
					if (saved_hovered_point_on_context < 0 && context_menu_initial_frame)
					{
						saved_hovered_point_on_context = hovered_point;
					}

					if (saved_hovered_point_on_context >= 0)
					{
						if (ImGui::MenuItem("Remove Shape"))
						{
							for (auto shape = 0; shape < trail_shapes_count; shape++)
							{
								if (saved_hovered_point_on_context >= trail_shapes[shape].offset_vertex
									&& saved_hovered_point_on_context < trail_shapes[shape].offset_vertex + trail_shapes[shape].num_vertex)
								{
									trail_shapes[shape].pending_deletion = true;
									modified = true;
									break;
								}

								// delete last shape if point is out of bounds
								if (shape + 1 >= trail_shapes_count)
								{
									trail_shapes[shape].pending_deletion = true;
									modified = true;
									break;
								}
							}

							// reset after action
							saved_hovered_point_on_context = -1;
						}

						SEPERATORV(0.0f);
					}
				}
			}
			else
			{
				// reset saved point when context menu is not open
				saved_hovered_point_on_context = -1;
			}


			// update effect element
			for (auto shape = 0; shape < trail_shapes_count; shape++)
			{
				// if shape was marked for deletion (context menu inside CurveEditorShapes)
				if (trail_shapes[shape].pending_deletion)
				{
					const int next = shape + 1;

					// if last shape
					if (next >= trail_shapes_count)
					{
						elem->trailDef.vertCount -= trail_shapes[shape].num_vertex;
						elem->trailDef.indCount -= (trail_shapes[shape].num_indices * 2);
					}
					else
					{
						// get amount of vertices after to-delete shape
						int verts_to_move = 0;
						for (auto c = next; c < trail_shapes_count; c++)
						{
							verts_to_move += trail_shapes[c].num_vertex;
						}

						// move vertices
						for (auto v = 0; v < verts_to_move; v++)
						{
							memcpy(elem->trailDef.verts[trail_shapes[shape].offset_vertex + v].pos, ((ImVec2*)traildef_point_array + (trail_shapes[next].offset_vertex + v)), sizeof(float[2]));
							elem->trailDef.verts[trail_shapes[shape].offset_vertex + v].texCoord = elem->trailDef.verts[trail_shapes[next].offset_vertex + v].texCoord;
						}

						elem->trailDef.vertCount -= trail_shapes[shape].num_vertex;

						memset(&elem->trailDef.inds, 0, sizeof(elem->trailDef.inds));
						elem->trailDef.indCount = 0;


						// rebuild indices
						int shape_count = 0;

						for (auto s = 0; s < trail_shapes_count; s++)
						{
							// skip deleted shape
							if (trail_shapes[s].pending_deletion)
							{
								continue;
							}

							// get prebuilt list of indices for shape with X amount of vertices
							const int indices_amount = indices_list[trail_shapes[s].num_vertex][0];
							const int last_index = elem->trailDef.indCount > 0 ? elem->trailDef.inds[elem->trailDef.indCount - 1] + 1 : 0;

							for (auto v = 0; v < indices_amount; v++)
							{
								elem->trailDef.inds[elem->trailDef.indCount++] = (indices_list[trail_shapes[s].num_vertex][v + 1] + static_cast<std::uint16_t>(last_index));
							}

							shape_count++;
						}

						break;
					}
				}
				else
				{
					// copy (modified) vertices from continuous array back into elem
					for (auto vert = 0; vert < trail_shapes[shape].num_vertex; vert++)
					{
						memcpy(elem->trailDef.verts[trail_shapes[shape].offset_vertex + vert].pos, ((ImVec2*)traildef_point_array + (trail_shapes[shape].offset_vertex + vert)), sizeof(float[2]));
					}
				}
			}


			// add new shapes via context-menu
			if (context_menu_open)
			{
				// #
				auto add_new_shape = [&](int new_shape_vertcount, const fx_system::FxTrailVertex* new_shape_vertices) -> void
				{
					const int indices_amount = indices_list[new_shape_vertcount][0]; // get prebuilt list of indices for shape with X amount of vertices

					if (elem->trailDef.vertCount + new_shape_vertcount < 64
						&& elem->trailDef.indCount + indices_amount < 128)
					{
						memcpy(&elem->trailDef.verts[elem->trailDef.vertCount], new_shape_vertices, new_shape_vertcount * sizeof(fx_system::FxTrailVertex));

						const int last_index = elem->trailDef.indCount > 0 ? elem->trailDef.inds[elem->trailDef.indCount - 1] + 1 : 0;

						for (auto v = 0; v < indices_amount; v++)
						{
							elem->trailDef.inds[elem->trailDef.indCount++] = (indices_list[new_shape_vertcount][v + 1] + static_cast<std::uint16_t>(last_index));
						}

						elem->trailDef.vertCount += new_shape_vertcount;

						modified = true;
					}
					else
					{
						game::printf_to_console("Cannot add shape. Would result in [%d/64] Vertices and [%d/128] Indices", elem->trailDef.vertCount + new_shape_vertcount, elem->trailDef.indCount + indices_amount);
					}
				};

				// --

				if (ImGui::MenuItem("New two-sided line shape")) {
					add_new_shape(3, geotrail_shape_line);
				}

				if (ImGui::MenuItem("New triangle shape")) {
					add_new_shape(4, geotrail_shape_triangle);
				}

				if (ImGui::MenuItem("New quadrilateral shape")) {
					add_new_shape(5, geotrail_shape_quad);
				}

				if (ImGui::MenuItem("New pentagon shape")) {
					add_new_shape(6, geotrail_shape_pentagon);
				}

				if (ImGui::MenuItem("New hexagon shape")) {
					add_new_shape(7, geotrail_shape_hexagon);
				}

				if (ImGui::MenuItem("New septagon shape")) {
					add_new_shape(8, geotrail_shape_septagon);
				}

				if (ImGui::MenuItem("New octagon shape")) {
					add_new_shape(9, geotrail_shape_octagon);
				}

				if (ImGui::MenuItem("New nonagon shape")) {
					add_new_shape(10, geotrail_shape_nonagon);
				}

				if (ImGui::MenuItem("New decagon shape")) {
					add_new_shape(11, geotrail_shape_decagon);
				}

				ImGui::EndPopup();
			}

			const auto vert_center_offset = (ImGui::GetFrameHeight() - ImGui::CalcTextSize("A").y) * 0.4f;

			ImGui::left_label_drag("Split Dist", vert_center_offset, 150.0f);
			MOD_CHECK(ImGui::DragInt("##split_dist", &elem->trailSplitDist, 0.1f, 1, INT16_MAX));

			ImGui::left_label_drag("Texture Repeat Dist", vert_center_offset, 150.0f);
			MOD_CHECK(ImGui::DragInt("##tex_rep_dist", &elem->trailRepeatDist, 0.1f, 1, INT16_MAX));

			ImGui::left_label_drag("Texture Scroll Time", vert_center_offset, 150.0f);
			MOD_CHECK(ImGui::DragFloat("##tex_scroll_time", &elem->trailScrollTime, 0.1f, 0, 1024.0f, "%.2f"));
		}

		if (elem->elemType <= fx_system::FX_ELEM_TYPE_CLOUD)
		{
			// *------------------------------
			ImGui::title_with_seperator("Sequence Control", true, 0, 2.0f, 8.0f);

			int atlas_start_behavior = 0;
			bool atlas_start_behavior_modified = false;

			if (const int masked_flag = elem->atlas.behavior & fx_system::FX_ATLAS_START_MASK;
				masked_flag)
			{
				if (masked_flag == fx_system::FX_ATLAS_START_FIXED)
				{
					atlas_start_behavior = 0;
				}
				else if (masked_flag == fx_system::FX_ATLAS_START_RANDOM)
				{
					atlas_start_behavior = 1;
				}
				else if (masked_flag == fx_system::FX_ATLAS_START_INDEXED)
				{
					atlas_start_behavior = 2;
				}
			}

			// Start Frame

			ImGui::TextUnformatted("Start Frame:");

			if (ImGui::RadioButton("Fixed Frame #", &atlas_start_behavior, 0)) {
				atlas_start_behavior_modified = true;
			}

			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(-ImGui::GetStyle().FramePadding.x);

			MOD_CHECK(ImGui::DragInt("##fixed_frame_idx", &elem->atlas.index, 1.0f, 0, 256));

			if (ImGui::RadioButton("Random", &atlas_start_behavior, 1)) {
				atlas_start_behavior_modified = true;
			}

			if (ImGui::RadioButton("Indexed", &atlas_start_behavior, 2)) {
				atlas_start_behavior_modified = true;
			}

			// ----

			int  atlas_play_behavior = 0; // Fixed FPS
			int  altas_loop_behavior = 0; // Forever
			bool atlas_play_behavior_modified = false;

			if (elem->atlas.behavior & fx_system::FX_ATLAS_PLAY_OVER_LIFE)
			{
				atlas_play_behavior = 4;
			}

			if (elem->atlas.behavior & fx_system::FX_ATLAS_LOOP_ONLY_N_TIMES)
			{
				altas_loop_behavior = 8;
			}

			// Play Rate
			SPACING(0.0f, 4.0f);
			ImGui::TextUnformatted("Play Rate:");

			if (ImGui::RadioButton("Fixed FPS", &atlas_play_behavior, 0)) {
				atlas_play_behavior_modified = true;
			}

			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(-ImGui::GetStyle().FramePadding.x);

			MOD_CHECK(ImGui::DragInt("##fixed_fps", &elem->atlas.fps, 1.0f, 0, 256));

			if (ImGui::RadioButton("Sync to Particle Lifetime", &atlas_play_behavior, 4)) {
				atlas_play_behavior_modified = true;
			}

			// Loop

			SPACING(0.0f, 4.0f);
			ImGui::TextUnformatted("Loop Settings:");

			if (ImGui::RadioButton("Forever", &altas_loop_behavior, 0)) {
				atlas_play_behavior_modified = true;
			}

			if (ImGui::RadioButton("Amount", &altas_loop_behavior, 8)) {
				atlas_play_behavior_modified = true;
			}

			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(-ImGui::GetStyle().FramePadding.x);

			MOD_CHECK(ImGui::DragInt("##loop_amount", &elem->atlas.loopCount, 1.0f, 0, 256));

			if (atlas_start_behavior_modified || atlas_play_behavior_modified)
			{
				elem->atlas.behavior &= ~(fx_system::FX_ATLAS_START_RANDOM | fx_system::FX_ATLAS_START_INDEXED | fx_system::FX_ATLAS_PLAY_OVER_LIFE | fx_system::FX_ATLAS_LOOP_ONLY_N_TIMES);
				elem->atlas.behavior |= (atlas_start_behavior | atlas_play_behavior | altas_loop_behavior);

				modified = true;
			}
		}

		ImGui::EndChild();
		on_modified(modified);
	}

	bool effects_editor_dialog::effect_property_window()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(360.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(580.0f, 400.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Effect Properties##window", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}

		const auto ed_effect = fx_system::get_editor_effect();
		if (!ed_effect)
		{
			ImGui::End();
			return false;
		}

		if (ImGui::BeginTabBar("##effect_properties_tabs", ImGuiTabBarFlags_None | ImGuiTabBarFlags_FittingPolicyResizeDown))
		{
			if (ImGui::BeginTabItem_SmallGap("Generation"))
			{
				tab_generation(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem_SmallGap("Size"))
			{
				tab_size(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem_SmallGap("Velocity"))
			{
				tab_velocity(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem_SmallGap("Rotation"))
			{
				tab_rotation(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem_SmallGap("Physics"))
			{
				tab_physics(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem_SmallGap("Color"))
			{
				tab_color(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem_SmallGap("Visuals"))
			{
				tab_visuals(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			// moved emission to generation

			ImGui::EndTabBar();
		}

		ImGui::End();
		return true;
	}

	bool effects_editor_dialog::gui()
	{
		effect_elemdef_list();
		return effects_editor_dialog::effect_property_window();
	}

	REGISTER_GUI(effects_editor_dialog);
}
