#include "std_include.hpp"

namespace ggui::entity
{
	// *
	// entity list and property list vars
	
	enum EPAIR_VALUETYPE
	{
		TEXT = 0,
		FLOAT = 1,
		COLOR = 2,
		ORIGIN = 3,
		ANGLES = 4,
		MODEL = 5,
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
	std::vector<game::eclass_t*> classlist;
	
	game::eclass_t* sel_list_ent = nullptr;
	game::eclass_t* edit_entity_class = nullptr;
	
	bool edit_entity_changed = false;
	bool edit_entity_changed_should_scroll = false;
	bool checkboxflags_states[12];

	
	// *
	// key value pair vars
	
	const int EPROP_MAX_ROWS = 32; // you are doing something wrong if you hit this limit D:
	const int EPROP_INPUTLEN = 256;

	char _edit_buf_key[EPROP_INPUTLEN][EPROP_MAX_ROWS] = {};
	bool _edit_buf_key_dirty[EPROP_MAX_ROWS] = { false };

	char _edit_buf_value[EPROP_INPUTLEN][EPROP_MAX_ROWS] = {};
	bool _edit_buf_value_dirty[EPROP_MAX_ROWS] = { false };

	struct new_kvp_helper
	{
		bool _in_use;

		char key_buffer[EPROP_INPUTLEN];
		bool key_active;
		bool key_set_focus;
		bool key_valid;

		char value_buffer[EPROP_INPUTLEN];
		bool value_active;
		bool value_set_focus;
		bool value_valid;

		void reset()
		{
			memset(this, 0, sizeof(new_kvp_helper));
		}
	};

	new_kvp_helper kvp_helper = {};

	
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

	bool Entity_GetVec3ForKey(game::entity_s* ent, float* vec3, const char* keyname)
	{
		auto value_str = ValueForKey(ent->epairs, keyname);
		return sscanf(value_str, "%f %f %f", &vec3[0], &vec3[1], &vec3[2]) == 3;
	}

	
	struct addprop_helper_s
	{
		bool is_origin;
		bool is_angle;
		bool is_generic_slider;
		bool is_color;
		bool add_undo;
	};

	void AddProp(const char* key, const char* value, addprop_helper_s* helper = nullptr)
	{
		// undo if no helper defined or helper wants to add an undo
		const bool should_add_undo = !helper || helper && helper->add_undo;
		
		if(auto edit_entity = game::g_edit_entity();
				edit_entity)
		{
			if(should_add_undo)
			{
				//if(!helper) game::printf_to_console("Start Undo without helper.");
				//else game::printf_to_console("Start Undo with helper->add_undo = %d", helper->add_undo);
				
				game::Undo_ClearRedo();
				game::Undo_GeneralStart("set key value pair");
			}

			if(game::multiple_edit_entities)
			{
				for (auto sb = game::g_selected_brushes_next();
					(DWORD*)sb != game::currSelectedBrushes; // sb->next really points to &selected_brushes(currSelectedBrushes) eventually
					sb = sb->next)
				{
					if (should_add_undo)
					{
						game::Undo_AddEntity_W(sb->owner->firstActive);
					}
					
					game::SetKeyValue(sb->owner->firstActive, key, value);

					if (!strcmp("origin", key) && edit_entity)
					{
						Entity_GetVec3ForKey((game::entity_s*)edit_entity, edit_entity->origin, "origin");
						++edit_entity->version;
					}
				}
				/* // not working yet
				{
					if (should_add_undo)
					{
						game::Undo_AddEntity_W(sb->owner->firstActive);
					}

					if (!strcmp("origin", key))
					{
						float org[3] = { 0.0f };
						if(sscanf(value, "%f %f %f", &org[0], &org[1], &org[2]) == 3)
						{
							float delta[3];
							utils::vector::subtract(org, edit_entity->origin, delta);
							
							Brush_Move(delta, sb->currSelection, 0);
							game::SetKeyValue(sb->owner->firstActive, key, value);
						}
					}
					else
					{
						game::SetKeyValue(sb->owner->firstActive, key, value);
					}
				}*/
			}
			else
			{
				if (should_add_undo)
				{
					Undo_AddEntity_W((game::entity_s*)edit_entity);
				}
				
				game::SetKeyValue((game::entity_s*)edit_entity, key, value);
				
				if (!strcmp("origin", key))
				{
					Entity_GetVec3ForKey((game::entity_s*)edit_entity, edit_entity->origin, "origin");
					++edit_entity->version;
				}
			}
			
			game::SetKeyValuePairs(); // refresh the prop listbox

			if (should_add_undo)
			{
				game::Undo_End();
			}
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
					(DWORD*)sb != game::currSelectedBrushes; // sb->next really points to &selected_brushes(currSelectedBrushes) eventually
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
	// dragfloat helper - add a single undo before starting to edit the value
	bool DragFloat_HelperUndo(addprop_helper_s* helper, const char* label, float* v, float v_speed, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		// dirty on edit
		if (ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, flags))
		{
			return true;
		}

		// dirty on activate (triggers before the actual edit), add an undo
		if (ImGui::IsItemActivated())
		{
			//game::printf_to_console("Helper UNDO TRUE");
			helper->add_undo = true;
			return true;
		}

		return false;
	}

	// *
	// coloredit3 helper - add a single undo before starting to edit the values
	bool ColorEdit3_HelperUndo(addprop_helper_s* helper, const char* label, float col[3], ImGuiColorEditFlags flags = 0)
	{
		if (ImGui::ColorEdit3(label, col, flags))
		{
			return true;
		}

		// dirty on activate (triggers before the actual edit), add an undo
		if (ImGui::IsItemActivated())
		{
			//game::printf_to_console("Helper UNDO TRUE");
			helper->add_undo = true;
			return true;
		}

		return false;
	}


	
	// *
	// Intercepted original functions
	
	void on_mapload_intercept()
	{
		sel_list_ent = nullptr;
		edit_entity_class = nullptr;
		edit_entity_changed = false;
		edit_entity_changed_should_scroll = false;
	}
	
	void on_update_selection_intercept()
	{
		// update our selected entity (also updates on escape)
		if (const auto	g_edit_ent = game::g_edit_entity();
			g_edit_ent && edit_entity_class != g_edit_ent->eclass)
		{
			sel_list_ent = g_edit_ent->eclass;
			edit_entity_class = g_edit_ent->eclass;
			edit_entity_changed = true;
			edit_entity_changed_should_scroll = true;
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


	
	// *
	// gui
	
	void draw_classlist()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

		if (ImGui::TreeNodeEx("Classlist", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SPACING(0.0f, 0.01f);

			if (ImGui::BeginListBox("##entlistbox", ImVec2(-4, 12 * ImGui::GetTextLineHeightWithSpacing())))
			{
				int index = 0;
				for (const auto pec : classlist)
				{
					const bool is_selected = (sel_list_ent == pec);
					if (ImGui::Selectable(pec->name, is_selected, ImGuiSelectableFlags_AllowDoubleClick))
					{
						sel_list_ent = pec;
						edit_entity_changed = true;
						game::UpdateSel(index, pec);

						if (ImGui::IsMouseDoubleClicked(0))
						{
							game::CreateEntity();
						}
					}

					// initial focus
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

					if (is_selected && edit_entity_changed_should_scroll)
					{
						ImGui::SetScrollHereY();
						edit_entity_changed_should_scroll = false;
					}

					index++;
				}

				ImGui::EndListBox();
			}
			
			ImGui::TreePop();
		}

		ImGui::PopStyleVar();

		SPACING(0.0f, 0.01f);
	}

	void draw_comments()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

		if (ImGui::TreeNodeEx("Class Comments", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SPACING(0.0f, 0.01f);

			static char empty_text = '\0';

			char* comment_buf = &empty_text;
			auto  comment_buf_len = strlen(comment_buf);

			if (sel_list_ent && sel_list_ent->comments)
			{
				comment_buf = sel_list_ent->comments;
				comment_buf_len = strlen(sel_list_ent->comments);
			}

			ImGui::InputTextMultiline("##entcomments", comment_buf, comment_buf_len, ImVec2(-4, 0), ImGuiInputTextFlags_ReadOnly);

			ImGui::TreePop();
		}
		
		ImGui::PopStyleVar();

		SPACING(0.0f, 0.01f);
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
						game::SetSpawnFlags(i);
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
				game::SetSpawnFlags(cb_num);
			} cb_num++;

			
			if (ImGui::Checkbox("!Medium", &checkboxflags_states[cb_num]))
			{
				SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[cb_num], 0);
				game::SetSpawnFlags(cb_num);
			} cb_num++;

			ImGui::SetCursorPosX(pre_checkbox_cursor.x + (wnd_size.x * 0.5f));
			ImGui::SetCursorPosY(pre_checkbox_cursor.y);

			if (ImGui::Checkbox("!Hard", &checkboxflags_states[cb_num]))
			{
				SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[cb_num], 0);
				game::SetSpawnFlags(cb_num);
			} cb_num++;

			ImGui::SetCursorPosX(pre_checkbox_cursor.x + (wnd_size.x * 0.5f));
			
			if (ImGui::Checkbox("!Deathmatch", &checkboxflags_states[cb_num]))
			{
				SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, checkboxflags_states[cb_num], 0);
				game::SetSpawnFlags(cb_num);
			} cb_num++;

			ImGui::PopStyleCompact();
			SPACING(0.0f, 0.01f);
			ImGui::TreePop();
		}

		ImGui::PopStyleVar();

		SPACING(0.0f, 0.01f);
	}
	

	void gui_entprop_new_keyvalue_pair()
	{
		// full width input text without label spacing
		ImGui::SetNextItemWidth(-1);

		// set focus on key input?
		if (kvp_helper.key_set_focus) 
		{
			ImGui::SetKeyboardFocusHere();
		}

		bool key_has_color = !kvp_helper.key_valid;
		if ( key_has_color ) {
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.49f, 0.2f, 0.2f, 1.0f));
		}
		
		if (ImGui::InputText("##key", kvp_helper.key_buffer, EPROP_INPUTLEN))
		{
			kvp_helper.key_valid = strlen(kvp_helper.key_buffer);
		}

		if (key_has_color) {
			ImGui::PopStyleColor();
		}

		if (ImGui::IsItemActive())
		{
			kvp_helper.key_set_focus = false;			// reset when inputtext actually got the focus
			kvp_helper.key_active = true;
		}
		else
		{
			kvp_helper.key_active = false;
		}

		// we need atleast 1 frame to actually set the focus so return early if set_focus is still true ^
		if (kvp_helper.key_set_focus)
		{
			return;
		}

		// if key inputtext active + valid + pressed enter -> set focus on value inputttext
		if (kvp_helper.key_active && kvp_helper.key_valid && ImGui::IsKeyPressedMap(ImGuiKey_Enter))
		{
			kvp_helper.value_set_focus = true;
		}

		// draw value inputtext when key is valid
		if (kvp_helper.key_valid)
		{
			ImGui::TableNextColumn();
			{
				// set focus on value input?
				if (kvp_helper.value_set_focus) 
				{
					ImGui::SetKeyboardFocusHere();
				}

				// full width input text without label spacing
				ImGui::SetNextItemWidth(-1);

				bool value_has_color = !kvp_helper.value_valid;
				if ( value_has_color ) {
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.49f, 0.2f, 0.2f, 1.0f));
				}
				
				if (ImGui::InputText("##value", kvp_helper.value_buffer, EPROP_INPUTLEN/*, ImGuiInputTextFlags_EnterReturnsTrue*/))
				{
					kvp_helper.value_valid = strlen(kvp_helper.value_buffer);
				}

				if (value_has_color) {
					ImGui::PopStyleColor();
				}

				// we need atleast 1 frame to actually set the focus so return early if set_focus is still true
				if (kvp_helper.value_set_focus && !ImGui::IsItemActive())
				{
					return;
				}

				// if value inputtext active + valid + pressed enter -> add KVP
				if (kvp_helper.value_active && kvp_helper.value_valid && ImGui::IsKeyPressedMap(ImGuiKey_Enter))
				{
					AddProp(kvp_helper.key_buffer, kvp_helper.value_buffer);
					kvp_helper.reset();
					return;
				}

				if(!ImGui::IsItemActive())
				{
					// if value is no longer active but had focus on the last frame + key input text is not active
					if(kvp_helper.value_active && !kvp_helper.key_active)
					{
						// is the value valid? -> add KVP
						if(kvp_helper.value_valid)
						{
							AddProp(kvp_helper.key_buffer, kvp_helper.value_buffer);
						}

						// reset everything otherwise or after the KVP was added
						kvp_helper.reset();
						return;
					}

					// key and value valid and none of them are active? -> add KVP
					if(kvp_helper.key_valid && kvp_helper.value_valid && !kvp_helper.key_active && !kvp_helper.value_active)
					{
						AddProp(kvp_helper.key_buffer, kvp_helper.value_buffer);
						kvp_helper.reset();
						return;
					}

					// reset when key nor value active
					if(!kvp_helper.key_active)
					{
						kvp_helper.reset();
						return;
					}
				}

				if (ImGui::IsItemActive())
				{
					kvp_helper.value_set_focus = false;		// reset when inputtext actually got the focus
					kvp_helper.value_active = true;
				}
				else
				{
					kvp_helper.value_active = false;
				}

				// !
				return;
			}
		}

		// aaaaaa
		if (!ImGui::IsItemActive())
		{
			kvp_helper.reset();
		}
	}

	
	void gui_entprop_add_key(game::epair_t* epair, int row)
	{
		// update inputbuf (text shown) using the epair key if inputbuf is not dirty, aka. not modified
		if (!_edit_buf_key_dirty[row])
		{
			strcpy(_edit_buf_key[row], epair->key);
		}
		
		// full with input text without label spacing
		ImGui::SetNextItemWidth(-1);

		if (ImGui::InputText("##key", _edit_buf_key[row], EPROP_INPUTLEN, ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_EnterReturnsTrue,
			[](ImGuiInputTextCallbackData* data)
			{
				// dirty once modified
				_edit_buf_key_dirty[(int)data->UserData] = true;
				return 0;

			}, (void*)row))
		// if(ImGui::InputText) -> on enter
		{
			// add as new key with old value
			AddProp(_edit_buf_key[row], epair->value);

			// delete the old key
			DelProp(epair->key, true);

			// ^ needs a sorting system as newest is always on top

			_edit_buf_key_dirty[row] = false;
		}

		// if InputText ^ is not active and buf dirty (user did not submit the change via enter) -> restore buf on next frame
		if (!ImGui::IsItemActive() && _edit_buf_key_dirty[row])
		{
			_edit_buf_key_dirty[row] = false;
		}
	}

	void gui_entprop_add_value_text(const epair_wrapper& epw, int row)
	{
		// update inputbuf (text shown) using the epair value if inputbuf is not dirty, aka. not modified
		if (!_edit_buf_value_dirty[row])
		{
			strcpy(_edit_buf_value[row], epw.epair->value);
		}

		// full with input text without label spacing
		ImGui::SetNextItemWidth(-1);

		if (ImGui::InputText("##value", _edit_buf_value[row], 256, ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EnterReturnsTrue,
			[](ImGuiInputTextCallbackData* data)
			{
				// dirty once modified
				_edit_buf_value_dirty[(int)data->UserData] = true;
				return 0;

			}, (void*)row))
		// if(ImGui::InputText) -> on enter
		{
			if (!strlen(_edit_buf_value[row]))
			{
				game::console_error("Value for key '"s + epw.epair->key + "' cannot be empty!");
			}
			else
			{
				// add "new" value with old key
				AddProp(epw.epair->key, _edit_buf_value[row]);

				// ^ needs a sorting system as newest is always on top
				_edit_buf_value_dirty[row] = false;
			}
		}

		// if InputText ^ is not active and buf dirty (user did not submit the change via enter) -> restore buf on next frame
		if (!ImGui::IsItemActive() && _edit_buf_value_dirty[row])
		{
			_edit_buf_value_dirty[row] = false;
		}
	}

	void gui_entprop_add_value_slider(const epair_wrapper& epw)
	{
		addprop_helper_s helper = {};
		helper.is_generic_slider = true;

		// full with input text without label spacing
		ImGui::SetNextItemWidth(-1);
		
		float val = static_cast<float>(atof(epw.epair->value));
		if (DragFloat_HelperUndo(&helper, "##value_slider", &val, epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			char val_str_buf[32] = {};
			if (sprintf_s(val_str_buf, "%.2f", val))
			{
				AddProp(epw.epair->key, val_str_buf, &helper);
			}
		}
	}

	void gui_entprop_add_value_color(const epair_wrapper& epw)
	{
		addprop_helper_s helper = {};
		helper.is_color = true;

		float col[3] = { 1.0f, 1.0f, 1.0f };
		if(sscanf(epw.epair->value, "%f %f %f", &col[0], &col[1], &col[2]) == 3)
		{
			// full with input text without label spacing
			ImGui::SetNextItemWidth(-1);
			
			if(ColorEdit3_HelperUndo(&helper, "##value_color", col, ImGuiColorEditFlags_Float))
			{
				char col_str_buf[64] = {};
				if (sprintf_s(col_str_buf, "%.3f %.3f %.3f", col[0], col[1], col[2]))
				{
					AddProp(epw.epair->key, col_str_buf, &helper);
				}
			}
		}
	}

	void gui_entprop_add_value_vec3(const epair_wrapper& epw, float* vec3)
	{
		bool dirty = false;
		
		char vec3_str_buf[64] = {};
		addprop_helper_s helper = {};
		
		helper.is_origin = epw.type && epw.type == ORIGIN;
		helper.is_angle = epw.type && epw.type == ANGLES;
		
		if (game::multiple_edit_entities) {
			ImGui::BeginDisabled(true);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		const float line_height		= ImGui::GetFrameHeight();
		const auto  button_size		= ImVec2(line_height, line_height);
		const float widget_spacing	= 8.0f;
		const float widget_width	= (ImGui::GetContentRegionAvailWidth() - (3.0f * button_size.x) - (2.0f * widget_spacing)) * 0.33333f;

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.17f, 0.17f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.17f, 0.17f, 1.0f));
		ImGui::ButtonEx("X", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		ImGui::SetNextItemWidth(widget_width);
		if(DragFloat_HelperUndo(&helper, "##X", &vec3[0], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		ImGui::SameLine(0, widget_spacing);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.185f, 0.6f, 0.23f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.185f, 0.6f, 0.23f, 1.0f));
		ImGui::ButtonEx("Y", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		ImGui::SetNextItemWidth(widget_width);
		if(DragFloat_HelperUndo(&helper, "##Y", &vec3[1], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		ImGui::SameLine(0, widget_spacing);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.165f, 0.375f, 0.69f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.165f, 0.375f, 0.69f, 1.0f));
		ImGui::ButtonEx("Z", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		ImGui::SetNextItemWidth(widget_width);
		if(DragFloat_HelperUndo(&helper, "##Z", &vec3[2], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		ImGui::PopStyleVar();

		if (game::multiple_edit_entities) 
		{
			ImGui::EndDisabled();
			return;
		}
		
		if(dirty)
		{
			if (sprintf_s(vec3_str_buf, "%.3f %.3f %.3f", vec3[0], vec3[1], vec3[2])) {
				AddProp(epw.epair->key, vec3_str_buf, &helper);
			}
		}
	}

	void gui_entprop_add_value_vec3(const epair_wrapper& epw, int row)
	{
		switch(epw.type)
		{
			
		case ORIGIN:
			if(const auto edit_entity = game::g_edit_entity(); 
						  edit_entity)
			{
				gui_entprop_add_value_vec3(epw, edit_entity->origin);
				return;
			}
			
			
		case ANGLES:
			float angles[3] = { 0.0f, 0.0f, 0.0f };
			if (sscanf(epw.epair->value, "%f %f %f", &angles[0], &angles[1], &angles[2]) == 3)
			{
				gui_entprop_add_value_vec3(epw, angles);
				return;
			}
		}

		gui_entprop_add_value_text(epw, row);
	}
	
	
	void draw_entprops()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

		if (ImGui::TreeNodeEx("Entity Properties", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//call EditProp (on Enter/submission)
			static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp;
			if (ImGui::BeginTable("##entprop_list", 3, flags))
			{
				//ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 60.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 140.0f); //, 64.0f);
				ImGui::TableSetupColumn("##delete", ImGuiTableColumnFlags_WidthFixed, 32.0f);
				//ImGui::TableHeadersRow();

				int row = 0;
				eprop_sorted.clear();

				const bool is_worldspawn = (sel_list_ent && !_stricmp(sel_list_ent->name, "worldspawn"));

				// only draw entprops if something is selected or if nothing is selected and sel_list_ent == the worldspawn
				if (const auto selbrush = game::g_selected_brushes();
					(selbrush && selbrush->currSelection) || is_worldspawn)
				{
					if (const auto edit_entity = game::g_edit_entity();
						edit_entity && edit_entity->epairs)
					{
						// add all epairs to our vector
						for (auto epair = edit_entity->epairs; epair; epair = epair->next)
						{
							std::string key = utils::str_to_lower(epair->key);
							 
							if(!is_worldspawn)
							{
								if (key == "origin") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::ORIGIN, 1.0f, -FLT_MAX, FLT_MAX));
									continue;
								}

								if (key == "angles") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::ANGLES, 0.1f, -FLT_MAX, FLT_MAX));
									continue;
								}
							}
							else
							{
								if (key == "suncolor") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::COLOR));
									continue;
								}

								if (key == "sundiffusecolor") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::COLOR));
									continue;
								}

								if (key == "sundirection") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::ANGLES, 0.1f, -FLT_MAX, FLT_MAX));
									continue;
								}
							}

							if (key == "_color") {
								eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::COLOR));
								continue;
							}

							

							// *
							// type float

							if(!is_worldspawn)
							{
								if (key == "radius") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 1.0f));
									continue;
								}

								if (key == "exponent") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 100.0f));
									continue;
								}

								if (key == "fov_inner") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.1f, 1.0f, 136.0f));
									continue;
								}

								if (key == "fov_outer") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.1f, 1.0f, 136.0f));
									continue;
								}

								if (key == "intensity")
								{
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f));
									continue;
								}
							}
							else
							{
								if (key == "ambient") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 2.0f));
									continue;
								}

								if (key == "sunlight") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 8.0f));
									continue;
								}

								if (key == "sunradiosity") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 100.0f));
									continue;
								}

								if (key == "diffusefraction") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 1.0f));
									continue;
								}

								if (key == "radiosityscale") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 100.0f));
									continue;
								}

								if (key == "contrastgain") {
									eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::FLOAT, 0.01f, 0.0f, 1.0f));
									continue;
								}
							}
							

							// *
							// everything else is text

							if (key == "model") {
								eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::MODEL));
								continue;
							}

							eprop_sorted.push_back(epair_wrapper(epair, EPAIR_VALUETYPE::TEXT));
						}


						// sort the vector alphabetically by epair->key
						std::sort(eprop_sorted.begin(), eprop_sorted.end(),
							[](epair_wrapper a, epair_wrapper b)
							{
								return strcmp(a.epair->key, b.epair->key) < 0;
							});


						// for each entry in our sorted vector
						for (auto ep : eprop_sorted)
						{
							ImGui::PushID(row);
							ImGui::TableNextColumn();
							{
								gui_entprop_add_key(ep.epair, row);
							}

							ImGui::TableNextColumn();
							{
								switch (ep.type)
								{
								case EPAIR_VALUETYPE::FLOAT:
									gui_entprop_add_value_slider(ep);
									break;

								case EPAIR_VALUETYPE::COLOR:
									gui_entprop_add_value_color(ep);
									break;

								case EPAIR_VALUETYPE::ORIGIN:
									gui_entprop_add_value_vec3(ep, edit_entity->origin);
									break;

								case EPAIR_VALUETYPE::ANGLES:
									gui_entprop_add_value_vec3(ep, row);
									break;

								default:
								case EPAIR_VALUETYPE::MODEL:
								case EPAIR_VALUETYPE::TEXT:
									gui_entprop_add_value_text(ep, row);
									break;
								}
							}

							ImGui::TableNextColumn();
							{
								if (ep.type != ORIGIN && _stricmp(ep.epair->key, "classname") != 0)
								{
									if (ImGui::Button("x"))
									{
										DelProp(ep.epair->key);
									}
								}
							}

							ImGui::PopID();

							if(row < EPROP_MAX_ROWS) 
							{
								row++;
							}
							else
							{
								game::console_error("MAX \"EPROP_MAX_ROWS\". Not drawing additional rows.");
								break;
							}
						}

						if(kvp_helper._in_use)
						{
							ImGui::TableNextColumn();
							{
								gui_entprop_new_keyvalue_pair();
							}
						}
					}
				}

				ImGui::EndTable();
			}

			if(const auto selbrush = game::g_selected_brushes();
				(selbrush && selbrush->currSelection) || (sel_list_ent && !_stricmp(sel_list_ent->name, "worldspawn")))
			{
				if (ImGui::Button("Add new key-value pair", ImVec2(-6.0f, 0.0f)))
				{
					kvp_helper._in_use = true;
					kvp_helper.key_set_focus = true;
				}
			}

			ImGui::TreePop();
		}
		
		ImGui::PopStyleVar();
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

	__declspec(naked) void on_mapload_stub()
	{
		const static uint32_t retn_pt = 0x418814;
		__asm
		{
			pushad;
			call	on_mapload_intercept;
			popad;

			push    0x1101;
			jmp		retn_pt;
		}
	}

	// CMainFrame::OnViewEntity
	void on_viewentity_command()
	{
		components::gui::toggle(ggui::state.czwnd.m_entity, 0, true);
	}

	void hooks()
	{
		// init classlist on startup
		utils::hook(0x496811, init_classlist_stub, HOOK_JUMP).install()->quick();

		// update edit_entity with radiant routines
		utils::hook(0x497220, on_update_selection_stub, HOOK_JUMP).install()->quick();

		// reset classlist on map re/load
		utils::hook(0x41880F, on_mapload_stub, HOOK_JUMP).install()->quick();

		// make entity-view hotkey open the imgui variant :: CMainFrame::OnFilterDlg
		utils::hook::detour(0x423F00, on_viewentity_command, HK_JUMP);

		// do not show original entity window
		utils::hook::set<BYTE>(0x423D0A + 1, 0x0);
		utils::hook::set<BYTE>(0x423DCF + 1, 0x0);
		utils::hook::set<BYTE>(0x423DDD + 1, 0x0);

		// do not show original entity window when opening the misc_model dialog from the context menu
		utils::hook::nop(0x46624A, 5);
		
		// (can be disabled once the imgui-entity window is done and the original hidden)
		// disable top-most mode for inspector/entity window
		//utils::hook::nop(0x496CB6, 13); // clear instructions
		//utils::hook::set<BYTE>(0x496CB6, 0xB9); // mov ecx,00000000 (0xB9 00 00 00 00)
		//utils::hook::set<DWORD>(0x496CB6 + 1, 0x0); // mov ecx,00000000 (0xB9 00 00 00 00)
	}
}
