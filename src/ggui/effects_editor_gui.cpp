#include "std_include.hpp"
#include "commdlg.h"

namespace ggui::effects_editor_gui
{
	ImGradient gradient;

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
				ImGui::Indent(6.0f);
				
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

				ImGui::Indent(-6.0f);

				break;

			case 1: // Name
				if (ImGui::Selectable(elemdef->name, row == *selected_index, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
				{
					*selected_index = row;
				}

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
	bool Modal_UnsavedChanges(const char* label)
	{
		bool result = false;

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal(label, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
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

		if (!ImGui::Begin("ElemList##window", nullptr, ImGuiWindowFlags_NoCollapse))
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
		const char* reload_fx_modal_str = "Unsaved Changes##fx_reload";
		if (ImGui::Button("Reload Effect"))
		{
			if (editor_effect_was_modified)
			{
				ImGui::OpenPopup(reload_fx_modal_str);
			}
			else
			{
				components::command::execute("fx_reload");
			}
		}

		if (Modal_UnsavedChanges(reload_fx_modal_str)) // true if clicked OK ^
		{
			components::command::execute("fx_reload");
			editor_effect_was_modified = false;
		}

		// ----

		ImGui::SameLine();
		const char* close_editor_modal_str = "Unsaved Changes##editor_close";
		if (ImGui::Button("Close Editor"))
		{
			if (editor_effect_was_modified)
			{
				ImGui::OpenPopup(close_editor_modal_str);
			}
			else
			{
				components::command::execute("fx_edit");
				components::command::execute("fx_reload");
			}
		}

		if (Modal_UnsavedChanges(close_editor_modal_str)) // true if clicked OK ^
		{
			components::command::execute("fx_edit");
			components::command::execute("fx_reload");
			editor_effect_was_modified = false;
		}


		ImGui::PushFontFromIndex(ggui::BOLD_18PX);
		const char* effect_name = ed_effect->name;
		const auto width = ImGui::CalcTextSize(effect_name).x + 16.0f;
		ImGui::SameLine(ImGui::GetWindowWidth() - width);
		ImGui::Text("%s %s", effect_name, editor_effect_was_modified ? "*" : "");
		ImGui::PopFont();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 6));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(9, 9));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(9, 9));

		if (ImGui::BeginTable("bind_table", 6,
			ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("##enabled_disabled", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 28.0f);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 200.0f);
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
			ImGui::PopStyleVar(4);
		}

		ImGui::End();
	}

	// ----

#define MOD_CHECK(control) if((control)) modified = true
#define MOD_CHECK_GRAPH(control) if((control) >= 0) modified = true

	fx_system::FxEffectDef* effectdef_fileprompt()
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

	void on_modified(bool modified)
	{
		if (modified)
		{
			editor_effect_was_modified = true;

			if(components::effects::effect_is_playing() || components::effects::effect_is_repeating() || components::effects::effect_is_paused())
			{
				components::effects::play();
			}
		}
	}

	void tab_generation(fx_system::FxEditorElemDef* elem)
	{
		ImGui::BeginChild("##effect_properties_generation_child");

		bool modified = false;

		ImGui::Indent(8.0f);
		ImGui::Spacing();
		ImGui::title_with_seperator("Spawning", false, 0, 2.0f, 8.0f);

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


		// *------------------------------
		ImGui::title_with_seperator("Origin", true, 0, 2.0f, 8.0f);

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
			switch(origin_offset_mode)
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


		// *------------------------------
		ImGui::title_with_seperator("Misc", true, 0, 2.0f, 8.0f);

		MOD_CHECK(ImGui::Checkbox_FxElemFlag("Disable Far plane culling (Past Fog)", elem, fx_system::FX_ELEM_DRAW_PAST_FOG));
		MOD_CHECK(ImGui::Checkbox_FxElemFlag("Blocks Sight", elem, fx_system::FX_ELEM_BLOCK_SIGHT));
		MOD_CHECK(ImGui::Checkbox_FxElemFlag("Draw With Viewmodel", elem, fx_system::FX_ELEM_DRAW_WITH_VIEWMODEL));
		MOD_CHECK(ImGui::DragInt("Sort Order", &elem->sortOrder, 0.1f, -UINT8_MAX, UINT8_MAX));


		// *------------------------------
		ImGui::title_with_seperator("Death Effects", true, 0, 2.0f, 8.0f);

		bool enable_death_effect = false;
		MOD_CHECK(ImGui::Checkbox_FxElemFlag("Enable Death Effects", elem, fx_system::FX_ED_FLAG_PLAY_ON_DEATH, &enable_death_effect));

		if(enable_death_effect)
		{
			std::string death_effect_string;
			if(elem->effectOnDeath && elem->effectOnDeath->name)
			{
				death_effect_string = elem->effectOnDeath->name;
			}

			ImGui::InputText("##no_label", &death_effect_string, ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("..##filepromt", ImVec2(28, ImGui::GetFrameHeight())))
			{
				if(const auto death_elem = effectdef_fileprompt(); 
							  death_elem)
				{
					elem->effectOnDeath = death_elem;
					modified = true;
				}
			}
		}

		ImGui::EndChild();

		on_modified(modified);
	}
	

	void tab_size([[maybe_unused]] fx_system::FxEditorElemDef* elem)
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

		ImGui::title_with_seperator("Width / Diameter", false, 0, 2.0f, 8.0f);
		{
			// false = graph 1, true = 2
			static bool current_graph_scale = false;

			int new_count = 0;
			const auto curve = elem->sizeShape[0][current_graph_scale]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

			MOD_CHECK_GRAPH(ImGui::CurveEditor("width_graph", curve->keys, curve->keyCount,
				ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

			if (new_count != curve->keyCount)
			{
				curve->keyCount = new_count;
				modified = true;
			}

			if (ImGui::BeginPopupContextItem("width_graph##bg"))
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

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##width", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SIZE_0));
			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);
			MOD_CHECK(ImGui::DragFloat("##width_scale", &elem->sizeScale[0], 0.5f, 0, 0, "%.1f")); TT("Scale");
		}


		// *------------------------------
		ImGui::title_with_seperator("Height / Length", true, 0, 2.0f, 8.0f);
		{
			// false = graph 1, true = 2
			static bool current_graph_scale = false;

			int new_count = 0;
			const auto curve = elem->sizeShape[1][current_graph_scale]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

			MOD_CHECK_GRAPH(ImGui::CurveEditor("height_graph", curve->keys, curve->keyCount,
				ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

			if (ImGui::BeginPopupContextItem("height_graph##bg"))
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

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##height", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SIZE_1));
			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);
			MOD_CHECK(ImGui::DragFloat("##height_scale", &elem->sizeScale[1], 0.5f, 0, 0, "%.1f")); TT("Scale");
		}


		// *------------------------------
		ImGui::title_with_seperator("Scale", true, 0, 2.0f, 8.0f);

		const bool is_scale_graph_aval = elem->elemType == fx_system::FX_ELEM_TYPE_CLOUD || elem->elemType == fx_system::FX_ELEM_TYPE_MODEL;
		ImGui::BeginDisabled(!is_scale_graph_aval);
		{
			// false = graph 1, true = 2
			static bool current_graph_scale = false;

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
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);
			MOD_CHECK(ImGui::DragFloat("##scale_scale", &elem->scaleScale, 0.5f, 0, 0, "%.1f")); TT("Scale");
		}

		ImGui::EndDisabled();
		ImGui::EndChild();

		on_modified(modified);
	}

	void tab_velocity([[maybe_unused]] fx_system::FxEditorElemDef* elem)
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

		ImGui::TextUnformatted("Move relative to:");

		int move_rel_to_flag = 0;
		
		if (const int masked_flag = elem->flags & fx_system::FX_ELEM_RUN_MASK; 
					  masked_flag)
		{
			if(masked_flag == fx_system::FX_ELEM_RUN_RELATIVE_TO_SPAWN)
			{
				move_rel_to_flag = 1;
			}
			else if(masked_flag == fx_system::FX_ELEM_RUN_RELATIVE_TO_EFFECT)
			{
				move_rel_to_flag = 2;
			}
			else if(masked_flag == fx_system::FX_ELEM_RUN_RELATIVE_TO_OFFSET)
			{
				move_rel_to_flag = 3;
			}
		}

		const float radio_same_line_offset = ImGui::GetContentRegionAvailWidth() * 0.55f;
		bool move_rel_to_flag_modified = false;

		if(ImGui::RadioButton("World", &move_rel_to_flag, 0)) {
			move_rel_to_flag_modified = true;
		}

		ImGui::SameLine(radio_same_line_offset);
		if(ImGui::RadioButton("Spawn Offset", &move_rel_to_flag, 3)) {
			move_rel_to_flag_modified = true;
		}
	
		if(ImGui::RadioButton("Effect at Spawn", &move_rel_to_flag, 1)) {
			move_rel_to_flag_modified = true;
		}

		ImGui::SameLine(radio_same_line_offset);
		if(ImGui::RadioButton("Effect Now", &move_rel_to_flag, 2)) {
			move_rel_to_flag_modified = true;
		}

		// #
		auto assign_movement_flag = [&](const int flag) -> void
		{
			elem->flags &= ~(fx_system::FX_ELEM_RUN_RELATIVE_TO_SPAWN | fx_system::FX_ELEM_RUN_RELATIVE_TO_EFFECT | fx_system::FX_ELEM_RUN_RELATIVE_TO_OFFSET);
			elem->flags |= flag;
		};

		if(move_rel_to_flag_modified)
		{
			// movement flags are multiple of 64
			assign_movement_flag(move_rel_to_flag * 64);
			modified = true;
		}

		SPACING(0.0f, 8.0f);
		MOD_CHECK(ImGui::DragFloat2_FxFloatRange("Gravity", &elem->gravity, 0.1f, -32768.0f, 32768.0f, "%.2f"));

		

		// *------------------------------

		auto get_velocity_graph_str = [&](const int i) -> const char*
		{
			switch(i)
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

		auto og_cursor_y = ImGui::GetCursorPosY();
		const auto vert_center_offset = (ImGui::GetFrameHeight() - ImGui::CalcTextSize("A").y) * 0.4f;

		// #
		auto left_label_dragfloat = [&](const char* label) -> void
		{
			og_cursor_y = ImGui::GetCursorPosY();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + vert_center_offset);
			ImGui::TextUnformatted(label);
			ImGui::SameLine(80.0f);
			ImGui::SetCursorPosY(og_cursor_y);
			ImGui::SetNextItemWidth(-style.FramePadding.x);
		};

		auto left_label_checkbox = [&](const char* label) -> void
		{
			og_cursor_y = ImGui::GetCursorPosY();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + vert_center_offset);
			ImGui::TextUnformatted(label);
			ImGui::SameLine();
			ImGui::SetCursorPosY(og_cursor_y);
		};

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

				left_label_dragfloat("Forward");
				MOD_CHECK(ImGui::DragFloat("##forward_velocity1", &elem->velScale[0][0], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

				left_label_dragfloat("Right");
				MOD_CHECK(ImGui::DragFloat("##right_velocity1", &elem->velScale[0][1], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

				left_label_dragfloat("Up");
				MOD_CHECK(ImGui::DragFloat("##up_velocity1", &elem->velScale[0][2], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

				ImGui::Spacing();

				static float checkbox1_width = 100.0f;
				static float checkbox2_width = 100.0f;

				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox1_width - style.FramePadding.x);
				ImGui::BeginGroup();
				{
					left_label_checkbox("Randomize between Graph 1 and 2");
					MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity1_random", elem, fx_system::FX_ED_FLAG_USE_RANDOM_VELOCITY_0));

					ImGui::EndGroup();
					checkbox1_width = ImGui::GetItemRectSize().x;
				}

				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox2_width - style.FramePadding.x);
				ImGui::BeginGroup();
				{
					left_label_checkbox("Relative to effect axis");
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

			if(new_count != curve->keyCount)
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

			left_label_dragfloat("Forward");
			MOD_CHECK(ImGui::DragFloat("##forward_velocity2", &elem->velScale[1][0], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

			left_label_dragfloat("Right");
			MOD_CHECK(ImGui::DragFloat("##right_velocity2", &elem->velScale[1][1], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

			left_label_dragfloat("Up");
			MOD_CHECK(ImGui::DragFloat("##up_velocity2", &elem->velScale[1][2], 0.1f, -32768.0f, 32768.0f, "%.2f")); TT("Scale");

			ImGui::Spacing();

			static float checkbox1_offset = 100.0f;
			static float checkbox2_offset = 100.0f;
			static float checkbox3_offset = 100.0f;

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox1_offset - style.FramePadding.x);
			ImGui::BeginGroup();
			{
				left_label_checkbox("Randomize between Graph 1 and 2");
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity2_random", elem, fx_system::FX_ED_FLAG_USE_RANDOM_VELOCITY_1));

				ImGui::EndGroup();
				checkbox1_offset = ImGui::GetItemRectSize().x;
			}

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox2_offset - style.FramePadding.x);
			ImGui::BeginGroup();
			{
				left_label_checkbox("Use deprecated randomize behavior");
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity2_backcomp", elem, fx_system::FX_ED_FLAG_BACKCOMPAT_VELOCITY));

				ImGui::EndGroup();
				checkbox2_offset = ImGui::GetItemRectSize().x;
			}

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - checkbox3_offset - style.FramePadding.x);
			ImGui::BeginGroup();
			{
				left_label_checkbox("Relative to effect axis");
				MOD_CHECK(ImGui::Checkbox_FxElemFlag("##velocity2_effect_axis", elem, fx_system::FX_ED_FLAG_ABSOLUTE_VELOCITY_1, nullptr, true));

				ImGui::EndGroup();
				checkbox3_offset = ImGui::GetItemRectSize().x;
			}
		}

		ImGui::EndChild();
		on_modified(modified);
	}

	void tab_rotation([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_physics([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		
	}

	void tab_color([[maybe_unused]] fx_system::FxEditorElemDef* elem)
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

		if (elem && elem->color[0] && elem->color[0]->keyCount && elem->color[1] && elem->color[1]->keyCount)
		{
			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##color", elem, fx_system::FX_ED_FLAG_USE_RANDOM_COLOR));
			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);

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

		// *------------------------------
		ImGui::title_with_seperator("Alpha", true, 0, 2.0f, 8.0f);

		if (elem && elem->alpha[0] && elem->alpha[0]->keyCount && elem->alpha[1] && elem->alpha[1]->keyCount)
		{
			// false = graph 1, true = 2
			static bool current_graph_scale = false;

			int new_count = 0;
			const auto curve = elem->alpha[current_graph_scale]; // velShape[VELOCITY_1/2][VECTOR_F/R_U][GRAPH_1/2]

			MOD_CHECK_GRAPH(ImGui::CurveEditor("alpha_graph", curve->keys, curve->keyCount, 
				ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), &new_count));

			if (new_count != curve->keyCount)
			{
				curve->keyCount = new_count;
				modified = true;
			}

			if (ImGui::BeginPopupContextItem("alpha_graph##bg"))
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

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graphs##alpha", elem, fx_system::FX_ED_FLAG_USE_RANDOM_ALPHA));
		}

		ImGui::SameLine(0, 14.0f);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);
		ImGui::DragFloat("##lighting_fraction", &elem->scaleScale, 0.01f, 0, 1.0f, "%.1f"); TT("Lighting Fraction");

		ImGui::EndChild();
		on_modified(modified);
	}


	const std::uint16_t indices_list_line[] = {  4, 0, 1, 1, 2 };
	const std::uint16_t indices_list_tri[]  = {  6, 0, 1, 1, 2, 2, 3 };
	const std::uint16_t indices_list_quad[] = {  8, 0, 1, 1, 2, 2, 3, 3, 4 };
	const std::uint16_t indices_list_pent[] = { 10, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5 };
	const std::uint16_t indices_list_hex[]  = { 12, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6 };
	const std::uint16_t indices_list_sep[]  = { 14, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7 };
	const std::uint16_t indices_list_oct[]  = { 16, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8 };
	const std::uint16_t indices_list_non[]  = { 18, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9 };
	const std::uint16_t indices_list_dec[]  = { 20, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10 };

	const std::uint16_t* indices_list[] =
	{
		nullptr,			// 0
		nullptr,			// 1
		nullptr,			// 2
		indices_list_line,	// 3
		indices_list_tri,	// 4
		indices_list_quad,	// 5
		indices_list_pent,	// 6
		indices_list_hex,	// 7
		indices_list_sep,	// 8
		indices_list_oct,	// 9
		indices_list_non,	// 10
		indices_list_dec	// 11
	};

	void tab_visuals([[maybe_unused]] fx_system::FxEditorElemDef* elem)
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


		// max 64 verts
		// max 128 indices

		// copy verts into continuous array
		float traildef_shape_vertices[2 * 64] = {};
		for(auto vert = 0; vert < elem->trailDef.vertCount; vert++)
		{
			memcpy(&traildef_shape_vertices[vert * 2], elem->trailDef.verts[vert].pos, sizeof(float[2]));
		}

		int shape_vertex_count = 1;
		int trail_shapes_count = 0;
		ImGui::traildef_shape_s trail_shapes[8] = {};

		for(auto ind_idx = 1; ind_idx < elem->trailDef.indCount; )
		{
			// end of inds ++ shape
			if(ind_idx + 1 < elem->trailDef.indCount)
			{
				// if current vert == next vert
				if (elem->trailDef.inds[ind_idx] == elem->trailDef.inds[ind_idx + 1])
				{
					shape_vertex_count++;
					ind_idx += 2;
					continue;
				}
			}

			// +
			// new shape

			if (trail_shapes_count == 0)
			{
				auto shape_curr = &trail_shapes[trail_shapes_count];

				shape_curr->index = trail_shapes_count;

				shape_curr->offset_vertex = 0;
				shape_curr->num_vertex = shape_vertex_count + 1;

				shape_curr->offset_indices = 0;
				shape_curr->num_indices = shape_vertex_count;

				trail_shapes_count++;

				// +
				// prepare next shape

				if (trail_shapes_count < 8)
				{
					shape_curr = &trail_shapes[trail_shapes_count];
					const auto shape_prev = &trail_shapes[trail_shapes_count - 1];

					shape_curr->index			= trail_shapes_count;
					shape_curr->offset_vertex	= shape_prev->num_vertex;
					shape_curr->offset_indices	= shape_prev->num_indices * 2;
				}
				else
				{
					break;
				}

				shape_vertex_count = 1; // reset
				ind_idx += 2; // advance
			}
			else
			{
				auto shape_curr = &trail_shapes[trail_shapes_count];

				shape_curr->num_vertex  = shape_vertex_count + 1;
				shape_curr->num_indices = shape_vertex_count;

				trail_shapes_count++;

				if(shape_curr->offset_indices + (shape_curr->num_indices * 2) < elem->trailDef.indCount)
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

				shape_vertex_count = 1; // reset
				ind_idx += 2; // advance
			}
		}

		// unused
		int new_count[8] = {};

		const auto hov_idx = (ImGui::CurveEditorShapes("traildef_shape", traildef_shape_vertices, trail_shapes, trail_shapes_count,
			ImVec2(-1.0f, -1.0f), ImVec2(1.0f, 1.0f), ImVec2(graph_width, graph_height), static_cast<int>(ImGui::CurveEditorFlags::SHOW_GRID), new_count));

		if(hov_idx > 0)
		{
			modified = true;
		}

		// update elem - ouch
		for (auto shape = 0; shape < trail_shapes_count; shape++)
		{
			// if shape was marked for deletion (context menu inside CurveEditorShapes)
			if(trail_shapes[shape].pending_deletion)
			{
				const int next = shape + 1;

				// if last shape
				if(next >= trail_shapes_count)
				{
					elem->trailDef.vertCount -= trail_shapes[shape].num_vertex;
					elem->trailDef.indCount  -= (trail_shapes[shape].num_indices * 2);
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
						memcpy(elem->trailDef.verts[trail_shapes[shape].offset_vertex + v].pos, ((ImVec2*)traildef_shape_vertices + (trail_shapes[next].offset_vertex + v)), sizeof(float[2]));
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

						// get prebuilt list of indices for shape with X amout of vertices
						const int indices_amount = indices_list[trail_shapes[s].num_vertex][0];
						const int last_index = elem->trailDef.indCount > 0 ? elem->trailDef.inds[elem->trailDef.indCount - 1] + 1 : 0;

						for (auto v = 0; v < indices_amount; v++)
						{
							elem->trailDef.inds[elem->trailDef.indCount++] = (indices_list[trail_shapes[s].num_vertex][v + 1] + (last_index));
							
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
					memcpy(elem->trailDef.verts[trail_shapes[shape].offset_vertex + vert].pos, ((ImVec2*)traildef_shape_vertices + (trail_shapes[shape].offset_vertex + vert)), sizeof(float[2]));
				}
			}
		}
		
		ImGui::EndChild();
		on_modified(modified);
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