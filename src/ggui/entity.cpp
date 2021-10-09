#include "std_include.hpp"

namespace ggui::entity
{
	std::vector<game::eclass_t*> classlist;
	game::eclass_t* sel_list_ent = nullptr;
	game::eclass_t* edit_entity_class = nullptr;
	bool edit_entity_changed = false;
	bool edit_entity_changed_should_scroll = false;
	bool checkboxflags_states[12];

	// *
	// ASM

	void SetSpawnFlags(int flag)
	{
		const static uint32_t func_addr = 0x496F00;
		__asm
		{
			pushad;
			mov		ebx, flag;
			call	func_addr;
			popad;
		}
	}

	void UpdateSel(int wParam, game::eclass_t* e_class)
	{
		const static uint32_t func_addr = 0x497180;
		__asm
		{
			pushad;
			mov		ecx, wParam;
			mov		eax, e_class;
			call	func_addr;
			popad;
		}
	}

	void Brush_Move(const float* delta, game::brush_t* def, int snap)
	{
		const static uint32_t func_addr = 0x47BA40;
		__asm
		{
			pushad;
			push	snap;
			push	def;
			mov		ebx, delta;
			call	func_addr;
			add     esp, 8;
			popad;
		}
	}

	
	// *
	// Helper functions

	char* ValueForKey(game::epair_t*& e, const char* key)
	{
		for (auto ep = e; ep; ep = ep->next)
		{
			if (!strcmp(ep->key, key))
			{
				return ep->value;
			}
		}

		return nullptr;
	}

	bool Entity_GetPlacement(game::entity_s* ent, float* origin, const char* keyname)
	{
		auto value_str = ValueForKey(ent->epairs, keyname);
		return sscanf(value_str, "%f %f %f", &origin[0], &origin[1], &origin[2]) == 3;
		//return sscanf(value_str, "%f %f %f", origin[0], origin[1], origin[2]) == 3;
	}

	void AddProp(const char* key, const char* value)
	{
		if(const auto edit_entity = game::g_edit_entity();
					  edit_entity)
		{
			game::Undo_ClearRedo();
			game::Undo_GeneralStart("set key value pair");

			if(game::multiple_edit_entities)
			{
				for (auto sb = game::g_selected_brushes_next();
					sb != game::g_selected_brushes();
					sb = sb->next)
				{
					game::Undo_AddEntity_W(sb->owner->firstActive);
					game::SetKeyValue(sb->owner->firstActive, key, value);
				}
				
				//int i = 0;
				//for (auto sb = game::g_selected_brushes_next();
				//		  i < 2;//sb != game::g_selected_brushes();
				//		  sb = sb->next)
				//{
				//	game::Undo_AddEntity_W(sb->owner->firstActive);

				//	if (!strcmp("origin", key))
				//	{
				//		float org[3] = { 0.0f };
				//		if(sscanf(value, "%f %f %f", &org[0], &org[1], &org[2]) == 3)
				//		{
				//			float delta[3];
				//			utils::vector::subtract(org, sb->owner->firstActive->origin, delta);
				//			
				//			Brush_Move(delta, sb->currSelection, 0);
				//			game::SetKeyValue(sb->owner->firstActive, key, value);

				//			i++;
				//		}
				//	}
				//	else
				//	{
				//		game::SetKeyValue(sb->owner->firstActive, key, value);
				//	}
				//}
			}
			else
			{
				Undo_AddEntity_W((game::entity_s*)edit_entity);

				if (!strcmp("origin", key))
				{
					float org[3] = { 0.0f };
					if (sscanf(value, "%f %f %f", &org[0], &org[1], &org[2]) == 3)
					{
						float delta[3];
						utils::vector::subtract(org, edit_entity->origin, delta);

						Brush_Move(delta, game::g_selected_brushes()->currSelection, 0);
					}
				}
				else
				{
					// original code without brush move ^
					game::SetKeyValue((game::entity_s*)edit_entity, key, value);
					if (!strcmp("origin", key))
					{
						Entity_GetPlacement((game::entity_s*)edit_entity, edit_entity->origin, "origin");
						++edit_entity->version;
					}
				}
			}
			
			game::SetKeyValuePairs(); // refresh the prop listbox
			game::Undo_End();
		}
	}

	void DelProp(const char* key, bool overwrite_classname_check = false)
	{
		if (const auto edit_entity = game::g_edit_entity();
					   edit_entity)
		{
			if (!overwrite_classname_check && !_stricmp(key, "classname"))
			{
				game::console_error("You can't delete the classname of an entity.  If you want to move this geometry to the world entity then select \"Ungroup Entity\" from the right-click menu.");
				return;
			}

			game::Undo_ClearRedo();
			game::Undo_GeneralStart("delete key value pair");

			if (game::multiple_edit_entities)
			{
				for (auto sb = game::g_selected_brushes_next();
						  sb != game::g_selected_brushes();
						  sb = sb->next)
				{
					game::Undo_AddEntity_W(sb->owner->firstActive);
					game::DeleteKey(sb->owner->firstActive->epairs, key);
					game::Checkkey_Model(sb->owner->firstActive, key);
					game::Checkkey_Color(sb->owner->firstActive, key);
				}
			}
			else
			{
				game::Undo_AddEntity_W((game::entity_s*)edit_entity);
				game::DeleteKey(edit_entity->epairs, key);
				game::Checkkey_Model((game::entity_s*)edit_entity, key);
				game::Checkkey_Color((game::entity_s*)edit_entity, key);
			}

			game::SetKeyValuePairs(); // refresh the prop listbox
			game::Undo_End();
		}
	}

	
	// *
	// Intercepted original functions
	
	void on_update_selection_intercept()
	{
		if (const auto selected_brush = game::g_selected_brushes();
			selected_brush && selected_brush->currSelection)
		{
			if (const auto	g_edit_ent = game::g_edit_entity();
				g_edit_ent && edit_entity_class != g_edit_ent->eclass)
			{
				sel_list_ent = g_edit_ent->eclass;
				edit_entity_class = g_edit_ent->eclass;
				edit_entity_changed = true;
				edit_entity_changed_should_scroll = true;
			}
		}
	}

	// the classlist is only build once at startup. We need to do the same as "game::g_eclass"
	// will hold pretty much every model/prefab that is placed on the map -> list indices wont match up with the original entity window
	void fill_classlist_intercept()
	{
		for (auto pec = game::g_eclass(); pec; pec = pec->next)
		{
			classlist.push_back(pec);
		}
	}
	
	// - FillClassList
	void draw_classlist()
	{
		if (ImGui::BeginListBox("##entlistbox", ImVec2(-FLT_MIN, 12 * ImGui::GetTextLineHeightWithSpacing())))
		{
			int index = 0;
			for(const auto pec : classlist)
			{
				const bool is_selected = (sel_list_ent == pec);
				if (ImGui::Selectable(pec->name, is_selected))
				{
					sel_list_ent = pec;
					edit_entity_changed = true;
					UpdateSel(index, pec);
				}

				// initial focus
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}

				if(is_selected && edit_entity_changed_should_scroll)
				{
					ImGui::SetScrollHereY();
					edit_entity_changed_should_scroll = false;
				}

				index++;
			}
			
			ImGui::EndListBox();
		}
	}

	void draw_comments()
	{
		static char empty_text = '\0';
		
		char* comment_buf = &empty_text;
		auto  comment_buf_len = strlen(comment_buf);
		
		if(sel_list_ent && sel_list_ent->comments)
		{
			comment_buf = sel_list_ent->comments;
			comment_buf_len = strlen(sel_list_ent->comments);
		}
		
		ImGui::InputTextMultiline("##entcomments", comment_buf, comment_buf_len, ImVec2(-FLT_MIN,0), ImGuiInputTextFlags_ReadOnly);
	}

	void draw_checkboxes()
	{
		// on changed selection -> update spawnflags / checkboxes
		if (const auto	selected_brush = game::g_selected_brushes();
			selected_brush && selected_brush->currSelection)
		{
			if (const auto edit_entity = game::g_edit_entity();
				edit_entity && edit_entity->epairs)
			{
				// do spawnflags
				int flag = 0;
				if(char* value_str = ValueForKey(edit_entity->epairs, "spawnflags"); 
						 value_str)
				{
					flag = atoi(value_str);
				}

				// set checkboxes accordingly, TODO :: move this
				for (auto i = 0; i < 12; i++)
				{
					checkboxflags_states[i] = !!(flag & 1 << i);
				}
			}
		}

		// render checkboxes
		
		const auto wnd_size = ImGui::GetWindowSize();
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
		
		if (ImGui::TreeNodeEx("Spawnflags", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SPACING(0.0f, 0.01f);
			ImGui::PushStyleCompact();
			
			const auto pre_checkbox_cursor = ImGui::GetCursorPos();
			for (auto i = 0; i < 8; i++)
			{
				if (i == 4) {
					ImGui::SetCursorPosY(pre_checkbox_cursor.y);
				}

				if (i >= 4) {
					ImGui::SetCursorPosX(pre_checkbox_cursor.x + (wnd_size.x * 0.5f));
				}

				bool flag_in_use = false;
				std::string flagname;

				if (sel_list_ent && sel_list_ent->flagnames[i] && sel_list_ent->flagnames[i][0] != 0)
				{
					flag_in_use = true;
					flagname = sel_list_ent->flagnames[i];
				}

				flagname += "##entcheckbox"s + std::to_string(i);

				if (ImGui::Checkbox(flagname.c_str(), &checkboxflags_states[i]))
				{
					if (flag_in_use)
					{
						SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + i], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[i], 0);
						SetSpawnFlags(i);
					}
				}
			}
			
			ImGui::PopStyleCompact();
			SPACING(0.0f, 0.01f);
			ImGui::TreePop();
		}

		SPACING(0.0f, 0.01f);

		if(ImGui::TreeNode("Spawnflags Easy / Medium / Hard / Deathmatch##who_needs_these"))
		{
			SPACING(0.0f, 0.01f);
			ImGui::PushStyleCompact();
			
			int cb_num = 8;
			const auto pre_checkbox_cursor = ImGui::GetCursorPos();
			
			if (ImGui::Checkbox("!Easy", &checkboxflags_states[cb_num]))
			{
				SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[cb_num], 0);
				SetSpawnFlags(cb_num);
			} cb_num++;

			
			if (ImGui::Checkbox("!Medium", &checkboxflags_states[cb_num]))
			{
				SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[cb_num], 0);
				SetSpawnFlags(cb_num);
			} cb_num++;

			ImGui::SetCursorPosX(pre_checkbox_cursor.x + (wnd_size.x * 0.5f));
			ImGui::SetCursorPosY(pre_checkbox_cursor.y);

			if (ImGui::Checkbox("!Hard", &checkboxflags_states[cb_num]))
			{
				SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[cb_num], 0);
				SetSpawnFlags(cb_num);
			} cb_num++;

			ImGui::SetCursorPosX(pre_checkbox_cursor.x + (wnd_size.x * 0.5f));
			
			if (ImGui::Checkbox("!Deathmatch", &checkboxflags_states[cb_num]))
			{
				SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[cb_num], 0);
				SetSpawnFlags(cb_num);
			} cb_num++;

			ImGui::PopStyleCompact();
			SPACING(0.0f, 0.01f);
			ImGui::TreePop();
		}

		ImGui::PopStyleVar();
	}






	
	const int EPROP_MAX_ROWS = 20;
	const int EPROP_INPUTLEN = 256;

	char edit_buf_key [EPROP_INPUTLEN][EPROP_MAX_ROWS] = {};
	bool edit_buf_key_dirty [EPROP_MAX_ROWS] = { false };

	char edit_buf_value [EPROP_INPUTLEN][EPROP_MAX_ROWS] = {};
	bool edit_buf_value_dirty [EPROP_MAX_ROWS] = { false };

	enum EPAIR_VALUETYPE
	{
		TEXT = 0,
		FLOAT = 1,
		COLOR = 2,
		ORIGIN = 3,
		ANGLES = 4,
	};
	
	struct epair_wrapper
	{
		game::epair_t* epair;
		EPAIR_VALUETYPE type;
		float v_speed = 0.1f;
		float v_min = 0.0f;
		float v_max = FLT_MAX;
	};

	std::vector<epair_wrapper> eprop_sorted;
	

	void gui_entprop_add_key(game::epair_t* epair, int row)
	{
		// update inputbuf (text shown) using the epair key if inputbuf is not dirty, aka. not modified
		if (!edit_buf_key_dirty[row])
		{
			strcpy(edit_buf_key[row], epair->key);
		}

		// show source epair for debugging reasons
		//ImGui::TextUnformatted(epair->key);


		if (ImGui::InputText("##key", edit_buf_key[row], EPROP_INPUTLEN, ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_EnterReturnsTrue,
			[](ImGuiInputTextCallbackData* data)
			{
				// dirty once modified
				edit_buf_key_dirty[(int)data->UserData] = true;
				return 0;

			}, (void*)row))
		// if(ImGui::InputText) -> on enter
		{
			// add as new key with old value
			AddProp(edit_buf_key[row], epair->value);

			// delete the old key
			DelProp(epair->key, true);

			// ^ needs a sorting system as newest is always on top

			edit_buf_key_dirty[row] = false;
		}

		// if InputText ^ is not active and buf dirty (user did not submit the change via enter) -> restore buf on next frame
		if (!ImGui::IsItemActive() && edit_buf_key_dirty[row])
		{
			edit_buf_key_dirty[row] = false;
		}
	}


	
	void gui_entprop_add_value_text(const epair_wrapper& epw, int row)
	{
		// update inputbuf (text shown) using the epair value if inputbuf is not dirty, aka. not modified
		if (!edit_buf_value_dirty[row])
		{
			strcpy(edit_buf_value[row], epw.epair->value);
		}

		// show source epair for debugging reasons
		//ImGui::TextUnformatted(epair->value);


		if (ImGui::InputText("##value", edit_buf_value[row], 256, ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_EnterReturnsTrue,
			[](ImGuiInputTextCallbackData* data)
			{
				// dirty once modified
				edit_buf_value_dirty[(int)data->UserData] = true;
				return 0;

			}, (void*)row))
			// if(ImGui::InputText) -> on enter
		{
			if (!strlen(edit_buf_value[row]))
			{
				game::console_error("Value for key '"s + epw.epair->key + "' cannot be empty!");
			}
			else
			{
				// add "new" value with old key
				AddProp(epw.epair->key, edit_buf_value[row]);

				// ^ needs a sorting system as newest is always on top
				edit_buf_value_dirty[row] = false;
			}
		}


			// if InputText ^ is not active and buf dirty (user did not submit the change via enter) -> restore buf on next frame
			if (!ImGui::IsItemActive() && edit_buf_value_dirty[row])
			{
				edit_buf_value_dirty[row] = false;
			}
	}

	void gui_entprop_add_value_slider(const epair_wrapper& epw, int row)
	{
		// show source epair for debugging reasons
		//ImGui::TextUnformatted(epair->value);

		float val = static_cast<float>(atof(epw.epair->value));
		
		if(ImGui::DragFloat("##value_slider", &val, epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			char val_str_buf[32] = {};
			if (sprintf_s(val_str_buf, "%.2f", val))
			{
				AddProp(epw.epair->key, val_str_buf);
			}
		}
	}

	void gui_entprop_add_value_color(const epair_wrapper& epw, int row)
	{
		// show source epair for debugging reasons
		//ImGui::TextUnformatted(epair->value);

		float col[3] = { 1.0f, 1.0f, 1.0f };
		if(sscanf(epw.epair->value, "%f %f %f", &col[0], &col[1], &col[2]) == 3)
		{
			if(ImGui::ColorEdit3("##value_color", col, ImGuiColorEditFlags_Float))
			{
				char col_str_buf[64] = {};
				if (sprintf_s(col_str_buf, "%.3f %.3f %.3f", col[0], col[1], col[2]))
				{
					AddProp(epw.epair->key, col_str_buf);
				}
			}
		}
	}

	void gui_entprop_add_value_vec3(const epair_wrapper& epw, float* vec3)
	{
		char vec3_str_buf[64] = {};
		bool dirty = false;

		if (game::multiple_edit_entities) {
			ImGui::BeginDisabled(true);
		}
		
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		const float line_height = ImGui::GetFrameHeight();
		const auto  button_size = ImVec2(line_height, line_height);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.17f, 0.17f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.17f, 0.17f, 1.0f));
		ImGui::ButtonEx("X", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();
		
		if(ImGui::DragFloat("##X", &vec3[0], epw.v_speed, epw.v_min, epw.v_max, "%.3f")) 
		{
			dirty = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 8.0f);

		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.185f, 0.6f, 0.23f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.185f, 0.6f, 0.23f, 1.0f));
		ImGui::ButtonEx("Y", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();
		
		if(ImGui::DragFloat("##Y", &vec3[1], epw.v_speed, epw.v_min, epw.v_max, "%.3f"))
		{
			dirty = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 8.0f);

		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.165f, 0.375f, 0.69f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.165f, 0.375f, 0.69f, 1.0f));
		ImGui::ButtonEx("Z", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();
		
		if(ImGui::DragFloat("##Z", &vec3[2], epw.v_speed, epw.v_min, epw.v_max, "%.3f"))
		{
			dirty = true;
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		if (game::multiple_edit_entities) 
		{
			ImGui::EndDisabled();
			return;
		}
		
		if(dirty)
		{
			if (sprintf_s(vec3_str_buf, "%.3f %.3f %.3f", vec3[0], vec3[1], vec3[2])) {
				AddProp(epw.epair->key, vec3_str_buf);
			}
		}
	}

	
	
	void draw_entprops()
	{
		// call EditProp (on Enter/submission)

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersOuter;
		if (ImGui::BeginTable("##entprop_list", 2, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 160.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 64.0f);
			ImGui::TableHeadersRow();

			int row = 0;
			eprop_sorted.clear();
			
			// on changed selection -> add epairs
			if (const auto	selected_brush = game::g_selected_brushes();
				selected_brush && selected_brush->currSelection)
			{
				if (const auto edit_entity = game::g_edit_entity();
					edit_entity && edit_entity->epairs)
				{
					// add all epairs to our vector
					for (auto epair = edit_entity->epairs; epair; epair = epair->next)
					{
						if (!strcmp(epair->key, "origin")) {
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::ORIGIN, 1.0f, -FLT_MAX, FLT_MAX));
							continue;
						}

						if (!strcmp(epair->key, "angles")) {
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::ANGLES));
							continue;
						}

						if (!strcmp(epair->key, "_color")) {
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::COLOR));
							continue;
						}

						// *
						// type float
						
						if (!strcmp(epair->key, "radius")) {
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 1.0f));
							continue;
						}

						if (!strcmp(epair->key, "exponent")) {
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 100.0f));
							continue;
						}

						if (!strcmp(epair->key, "fov_inner")) {
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.1f, 1.0f, 136.0f));
							continue;
						}

						if (!strcmp(epair->key, "fov_outer")) {
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.1f, 1.0f, 136.0f));
							continue;
						}

						if(!strcmp(epair->key, "intensity")) 
						{
							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f));
							continue;
						}

						// *
						// everything else is text

						eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::TEXT));
					}

					// sort the vector alphabetically by epair->key
					std::sort(eprop_sorted.begin(), eprop_sorted.end(), 
						[](epair_wrapper a, epair_wrapper b)
						{
							return strcmp(a.epair->key, b.epair->key) < 0;
						});

					// for each entry in our sorted vector
					for(auto ep : eprop_sorted)
					{
						// epair origin is not up to date
						/*if (!strcmp(ep.epair->key, "origin"))
						{
							continue;
						}*/

						ImGui::PushID(row);
						ImGui::TableNextColumn();
						{
							gui_entprop_add_key(ep.epair, row);
						}

						ImGui::TableNextColumn();
						{
							switch(ep.type)
							{
							case EPAIR_VALUETYPE::FLOAT:
								gui_entprop_add_value_slider(ep, row);
								break;
								
							case EPAIR_VALUETYPE::COLOR:
								gui_entprop_add_value_color(ep, row);
								break;
								
							case EPAIR_VALUETYPE::ORIGIN:
								gui_entprop_add_value_vec3(ep, edit_entity->origin);
								break;
								
							case EPAIR_VALUETYPE::ANGLES:
								//break;

							default:
							case EPAIR_VALUETYPE::TEXT:
								gui_entprop_add_value_text(ep, row);
								break;
							}
						}

						ImGui::PopID();
						row++;
					}

					/*if (edit_entity->origin[0] != 0.0f || edit_entity->origin[1] != 0.0f || edit_entity->origin[2] != 0.0f)
					{
						ImGui::TableNextColumn();
						ImGui::TextUnformatted("origin");

						ImGui::TableNextColumn();
						ImGui::Text("%.2f %.2f %.2f", edit_entity->origin[0], edit_entity->origin[1], edit_entity->origin[2]);

						row++;
					}*/
				}
			}
			
			for(; row < 12; row++)
			{
				ImGui::TableNextRow(0, ImGui::GetTextLineHeightWithSpacing());
				ImGui::TableNextColumn();
				//ImGui::Text("Hello %d", row);
			}

			ImGui::EndTable();
		}
	}
	
	void menu(ggui::imgui_context_menu& menu)
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		
		ImGui::Begin("Entity##window", &menu.menustate, ImGuiWindowFlags_NoCollapse);

		draw_classlist();
		draw_comments();
		draw_checkboxes();
		draw_entprops();
		
		edit_entity_changed = false;
		
		// Funcs of interest:
		// - EntityWndProc
		// - GetEntityControls
		// - FillClassList
		
		// 12 Checkboxes
		// checkbox 1 setting spawnflags 1 << 1 using "SetSpawnFlags(1)"
		// checkbox 2 setting spawnflags 1 << 2 etc
		
		ImGui::End();
	}


	// ----------------------------------
	// ASM

	__declspec(naked) void init_classlist_stub()
	{
		const static uint32_t retn_pt = 0x496816;
		__asm
		{
			pushad;
			call	fill_classlist_intercept;
			popad;

			push	0x184;
			jmp		retn_pt;
		}
	}
	
	__declspec(naked) void on_update_selection_stub()
	{
		const static uint32_t retn_pt = 0x497225;
		__asm
		{
			pushad;
			call	on_update_selection_intercept;
			popad;

			push    0x186;		// overwritten op
			jmp		retn_pt;
		}
	}

	void hooks()
	{
		utils::hook(0x496811, init_classlist_stub, HOOK_JUMP).install()->quick();
		// update our edit_entity with radiant routines
		utils::hook(0x497220, on_update_selection_stub, HOOK_JUMP).install()->quick();
	}
}
