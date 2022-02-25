#include "std_include.hpp"
#include "commdlg.h"

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
		if (ImGui::Button("Reload Effect"))
		{
			if (editor_effect_was_modified)
			{
				ImGui::OpenPopup("Unsaved Changes");
			}
			else
			{
				components::command::execute("fx_reload");
			}
		}

		if (Modal_UnsavedChanges()) // true if clicked OK ^
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

		if(modified)
		{
			editor_effect_was_modified = true;
			components::effects::play();
		}

		ImGui::EndChild();
	}

	

	void tab_size([[maybe_unused]] fx_system::FxEditorElemDef* elem)
	{
		bool modified = false;

		const ImGuiStyle& style = ImGui::GetStyle();
		const int graph_flags = (int)ImGui::CurveEditorFlags::NO_TANGENTS | (int)ImGui::CurveEditorFlags::SHOW_GRID;

		static bool scrollbar_visible = false;

		const float graph_width = ImGui::GetWindowContentRegionWidth() - (style.FramePadding.x * 2) - (scrollbar_visible ? 12.0f : -2.0f);// - 24.0f;
		float graph_height = graph_width;

		if (graph_height > 320)
		{
			graph_height = 320;
		}

		ImGui::BeginChild("##effect_properties_size_child");

		ImGui::Indent(8.0f);
		ImGui::Spacing();
		ImGui::title_with_seperator("Width / Diameter", false, 0, 2.0f, 8.0f);

		if (elem && elem->sizeShape[0][0] && elem->sizeShape[0][0]->keyCount 
			&& elem->sizeShape[0][1] && elem->sizeShape[0][1]->keyCount)
		{
			// false = graph 1, true = 2
			static bool current_graph_scale = false;

			const auto points = &elem->sizeShape[0][current_graph_scale]->keys[0];
			int new_count = 0;

			if (ImGui::CurveEditor("width_graph", points, elem->sizeShape[0][current_graph_scale]->keyCount, ImVec2(graph_width, graph_height), graph_flags,
				&new_count))
			{
				elem->sizeShape[0][current_graph_scale]->keyCount = new_count;
			};

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

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graph 1/2##width", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SIZE_0));
			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);
			ImGui::DragFloat("##width_scale", &elem->sizeScale[0], 0.5f, 0, 0, "%.1f"); TT("Scale");
		}


		// *------------------------------
		ImGui::title_with_seperator("Height / Length", true, 0, 2.0f, 8.0f);

		if (elem && elem->sizeShape[1][0] && elem->sizeShape[1][0]->keyCount && elem->sizeShape[1][1] && elem->sizeShape[1][1]->keyCount)
		{
			// false = graph 1, true = 2
			static bool current_graph_scale = false;

			const auto points = &elem->sizeShape[1][current_graph_scale]->keys[0];
			int new_count = 0;

			if (ImGui::CurveEditor("height_graph", points, elem->sizeShape[1][current_graph_scale]->keyCount, ImVec2(graph_width, graph_height), graph_flags,
				&new_count))
			{
				elem->sizeShape[1][current_graph_scale]->keyCount = new_count;
			};

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

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graph 1/2##height", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SIZE_1));
			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);
			ImGui::DragFloat("##height_scale", &elem->sizeScale[1], 0.5f, 0, 0, "%.1f"); TT("Scale");
		}


		// *------------------------------
		ImGui::title_with_seperator("Scale", true, 0, 2.0f, 8.0f);

		if(elem && elem->scaleShape[0] && elem->scaleShape[0]->keyCount && elem->scaleShape[1] && elem->scaleShape[1]->keyCount)
		{
			// false = graph 1, true = 2
			static bool current_graph_scale = false;

			const auto points = &elem->scaleShape[current_graph_scale]->keys[0];
			int new_count = 0;

			if (ImGui::CurveEditor("scale_graph", points, elem->scaleShape[current_graph_scale]->keyCount, ImVec2(graph_width, graph_height), graph_flags,
				&new_count))
			{
				elem->scaleShape[current_graph_scale]->keyCount = new_count;
			};

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

			MOD_CHECK(ImGui::Checkbox_FxElemFlag("Randomize between Graph 1/2##scale", elem, fx_system::FX_ED_FLAG_USE_RANDOM_SCALE));
			ImGui::SameLine(0, 14.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - style.FramePadding.x);
			ImGui::DragFloat("##scale_scale", &elem->scaleScale, 0.5f, 0, 0, "%.1f"); TT("Scale");
		}

		

		// width/dia
		// graph1: elem->sizeShape[0][0], elem->sizeScale[0][0]
		// graph2: elem->sizeShape[0][1], elem->sizeScale[0][1]

		// height/len 
		// graph1: elem->sizeShape[1][0], elem->sizeScale[1][0]
		// graph2: elem->sizeShape[1][1], elem->sizeScale[1][1]

		// scale (cloud)
		// elem->scaleShape, elem->scaleScale

		scrollbar_visible = ImGui::IsVertScollbarVisible();

		ImGui::EndChild();

		if (modified)
		{
			editor_effect_was_modified = true;
			components::effects::play();
		}
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