#include "std_include.hpp"

namespace ggui::effects_editor_gui
{
	const char* s_elemTypeNames[11] =
	{
		"Billboard Sprite",
		"Oriented Sprite",
		"Tail",
		"Trail",
		"Cloud",
		"Model",
		"Light",
		"SpotLight",
		"Sound",
		"Decal",
		"FX Runner"
	};

	void elemdef_elem(fx_system::FxEditorElemDef* elemdef, int row, int* selected_index)
	{
		// enabled / disabled state
		bool elem_enabled = !((elemdef->editorFlags & fx_system::FX_ED_FLAG_DISABLED) != 0);

		for(auto column = 0; column < 6; column++)
		{
			ImGui::PushID(column);
			ImGui::TableNextColumn();

			switch(column)
			{
			case 0: // Enable/Disable
				ImGui::Indent(4.0f);
				if(ImGui::Checkbox("##enabled_flag", &elem_enabled))
				{
					if(elem_enabled)
					{
						elemdef->editorFlags &= ~fx_system::FX_ED_FLAG_DISABLED;
					}
					else
					{
						elemdef->editorFlags |= fx_system::FX_ED_FLAG_DISABLED;
					}

					// restart effect
					components::effects::play();
				}

				break;

			case 1: // Name
				// unique id for each separator
				if (ImGui::Selectable(elemdef->name, row == *selected_index, /*ImGuiSelectableFlags_SpanAllColumns |*/ ImGuiSelectableFlags_AllowItemOverlap))
				{
					*selected_index = row;
				}

				//ImGui::Text(elemdef->name);
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
					if(elemdef->spawnOneShot.count.amplitude)
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
	bool Modal_UnsavedChanges()
	{
		bool result = false;

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const char* str = "\nUnsaved Effect Changes!\nDo you want to continue?\n\n";
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

		return result;
	}

	// 0x402AB0
	void effect_elemdef_list()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(360.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(580.0f, 400.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("ElemDef List##window", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		const auto ed_effect = fx_system::get_editor_effect();
		if(!ed_effect)
		{
			ImGui::End();
			return;
		}

		if (selected_editor_elemdef >= ed_effect->elemCount)
		{
			selected_editor_elemdef = ed_effect->elemCount - 1;
		}

		if(ImGui::Button("Add Segment"))
		{
			components::effects_editor::editor_add_new_segment();
			editor_effect_was_modified = true;
		}

		ImGui::SameLine();
		if(ImGui::Button("Delete Segment"))
		{
			components::effects_editor::editor_delete_segment(selected_editor_elemdef);
			editor_effect_was_modified = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("Save Effect"))
		{
			if(fx_system::FX_SaveEditorEffect())
			{
				editor_effect_was_modified = false;
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Reload Effect"))
		{
			if (editor_effect_was_modified)
			{
				ImGui::OpenPopup("Unsaved Changes");
			}
		}

		if (Modal_UnsavedChanges()) // ^
		{
			components::command::execute("fx_reload");
			editor_effect_was_modified = false;
		}

		ImGui::PushFontFromIndex(ggui::BOLD_18PX);

		const char* effect_name = ed_effect->name;

		const auto width = ImGui::CalcTextSize(effect_name).x + 16.0f;
		ImGui::SameLine(ImGui::GetWindowWidth() - width);
		ImGui::Text("%s %s", effect_name, editor_effect_was_modified ? "*" : "");
		ImGui::PopFont();

		if (ImGui::BeginTable("bind_table", 6,
			ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("##enabled_disabled", ImGuiTableColumnFlags_WidthFixed, 54.0f);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 200.0f);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 120.0f);
			ImGui::TableSetupColumn("Delay", ImGuiTableColumnFlags_WidthStretch, 40.0f);
			ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthStretch, 40.0f);
			ImGui::TableSetupColumn("Sort", ImGuiTableColumnFlags_WidthStretch, 40.0f);
			ImGui::TableHeadersRow();

			for(int row = 0; row < ed_effect->elemCount; row++)
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

		ImGui::End();
	}

	// ----

#define MOD_CHECK(control) if((control)) modified = true

	/*void MOD_CHECK(bool* modified, std::function<bool(fx_system::FxEditorElemDef*)> callback)
	{
		if(callback)
		{
			*modified = true;
		}
	}*/

	/*MOD_CHECK(&modified, [elem](auto)
			{ return ImGui::DragInt2("Count", reinterpret_cast<int*>(&elem->spawnOneShot.count)); });*/

	void tab_generation([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;

		ImGui::Indent(8.0f);
		ImGui::title_with_seperator("Spawning", false);

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
		

		if(modified)
		{
			editor_effect_was_modified = true;
			components::effects::play();
		}
	}

	void tab_size([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_velocity([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_rotation([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_physics([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_color([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_visuals([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_emission([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void effect_property_window()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(360.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(580.0f, 400.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Effect Properties##window", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		const auto ed_effect = fx_system::get_editor_effect();
		if (!ed_effect)
		{
			ImGui::End();
			return;
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

			if (ImGui::BeginTabItem_SmallGap("Emission"))
			{
				tab_emission(&ed_effect->elems[selected_editor_elemdef]);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	void menu([[maybe_unused]] ggui::imgui_context_menu& menu)
	{
		effect_elemdef_list();
		effect_property_window();
	}
}