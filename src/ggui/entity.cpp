#include "std_include.hpp"
#include "commdlg.h"

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
		FX = 6
	};

	struct epair_wrapper
	{
		game::entity_s_def* entity;
		game::eclass_t* eclass;
		game::epair_t* epair;
		EPAIR_VALUETYPE type;
		float v_speed = 0.1f;
		float v_min = 0.0f;
		float v_max = FLT_MAX;
		bool  is_fixedsize;
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

	bool Entity_GetValueForKey(game::entity_s* ent, float* value, const char* keyname)
	{
		if(auto value_str = ValueForKey(ent->epairs, keyname);
				value_str)
		{
			return sscanf(value_str, "%f", value) == 1;
		}

		return false;
	}

	bool Entity_GetVec3ForKey(game::entity_s* ent, float* vec3, const char* keyname)
	{
		if(auto value_str = ValueForKey(ent->epairs, keyname);
				value_str)
		{
			return sscanf(value_str, "%f %f %f", &vec3[0], &vec3[1], &vec3[2]) == 3;
		}

		return false;
	}

	bool HasKeyValuePair(game::entity_s_def* ent, const char* key)
	{
		game::epair_t* ep;

		ep = ent->epairs;
		if (!ep)
		{
			return false;
		}
		
		while (strcmp(ep->key, key))
		{
			ep = ep->next;
			if (!ep)
			{
				return false;
			}
		}
		
		return true;
	}

	
	/*struct addprop_helper_s
	{
		bool is_origin;
		bool is_angle;
		bool is_generic_slider;
		bool is_color;
		bool add_undo;
	};*/

	void AddProp(const char* key, const char* value, addprop_helper_s* helper)
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
				for (auto	sb = game::g_selected_brushes_next();
					(DWORD*)sb != game::currSelectedBrushes; // sb->next really points to &selected_brushes(currSelectedBrushes) eventually
							sb = sb->next)
				{
					std::string class_str;

					if(sb->owner->firstActive->eclass && sb->owner->firstActive->eclass->name)
					{
						class_str = utils::str_to_lower(sb->owner->firstActive->eclass->name);
					}

					if(class_str != "worldspawn")
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

	// adds grey horizontal separator
	void separator_for_treenode()
	{
		SPACING(0.0f, 6.0f);
		ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.3f, 0.3f, 0.3f, 0.9f));
		ImGui::Separator();
		ImGui::PopStyleColor();
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

	// UpdateSelection
	void on_update_selection_intercept()
	{
		if (const auto	g_edit_ent = game::g_edit_entity();
						g_edit_ent)
		{
			// update our selected entity (also updates on escape)
			if (edit_entity_class != g_edit_ent->eclass)
			{
				sel_list_ent = g_edit_ent->eclass;
				edit_entity_class = g_edit_ent->eclass;
				edit_entity_changed = true;
				edit_entity_changed_should_scroll = true;
			}

			if(g_edit_ent && g_edit_ent->eclass->name)
			{
				std::string class_str = utils::str_to_lower(g_edit_ent->eclass->name);

				if(class_str != "worldspawn")
				{
					// update / set initial origin on selection (freshly spawned prefabs wont have an origin key otherwise)
					// og. radiant does this aswell by writing into the key/value field and "simulating" the enter key
					if (0.0f != g_edit_ent->origin[0] || 0.0f != g_edit_ent->origin[1] || 0.0f != g_edit_ent->origin[2])
					{
						addprop_helper_s helper = {};
						helper.add_undo = false;
						helper.is_origin = true;

						char origin_str_buf[64] = {};
						if (sprintf_s(origin_str_buf, "%.3f %.3f %.3f", g_edit_ent->origin[0], g_edit_ent->origin[1], g_edit_ent->origin[2])) 
						{
							AddProp("origin", origin_str_buf, &helper);
						}
					}
				}
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


	
	// *
	// gui
	
	void draw_classlist()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

		if (ImGui::TreeNodeEx("Classlist", dvars::gui_props_classlist_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
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
			separator_for_treenode();
		}

		ImGui::PopStyleVar();

		SPACING(0.0f, 0.01f);
	}

	void draw_comments()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

		if (ImGui::TreeNodeEx("Class Comments", dvars::gui_props_comments_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
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
			separator_for_treenode();
		}
		
		ImGui::PopStyleVar();
		SPACING(0.0f, 0.01f);
	}

	void draw_checkboxes()
	{
		// on changed selection -> update spawnflags / checkboxes
		if (const auto	selected_brush = game::g_selected_brushes();
			selected_brush && selected_brush->def)
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
		
		if (ImGui::TreeNodeEx("Spawnflags", dvars::gui_props_spawnflags_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			if (ImGui::TreeNodeEx("Gametype Specific##who_needs_these", ImGuiTreeNodeFlags_SpanFullWidth))
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

			if (ImGui::TreeNodeEx("General", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
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

			ImGui::TreePop();
			separator_for_treenode();
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_IndentSpacing
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


		// *
		// drop model from modelselector into the textbox

		if (epw.type == EPAIR_VALUETYPE::MODEL)
		{
			// model selection drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (ImGui::AcceptDragDropPayload("MODEL_SELECTOR_ITEM"))
				{
					const auto m_selector = ggui::get_rtt_modelselector();
					AddProp(epw.epair->key, m_selector->preview_model_name.c_str());
				}
			}
		}
	}

	void gui_entprop_effect_fileprompt(const epair_wrapper& epw, [[maybe_unused]] int row)
	{
		if (ImGui::Button("..##filepromt", ImVec2(28, ImGui::GetFrameHeight())))
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

				std::size_t pos = filepath.find(replace_path) + replace_path.length();
				std::string loc_filepath = filepath.substr(pos);
				utils::erase_substring(loc_filepath, ".efx"s);

				AddProp(epw.epair->key, loc_filepath.c_str());

				// stop old effect
				components::command::execute("fx_stop");
			}
			else
			{
				//game::printf_to_console("filedialog: canceled");
			}
		}
	}

	void gui_entprop_model_fileprompt(const epair_wrapper& epw, [[maybe_unused]] int row)
	{
		if (ImGui::Button("..##filepromt", ImVec2(28, ImGui::GetFrameHeight())))
		{
			char filename[MAX_PATH];
			OPENFILENAMEA ofn;
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = cmainframe::activewnd->GetWindow();
			ofn.lpstrFilter = "Any File\0*.*\0";
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = "Select a XModel ...";
			ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

			if (GetOpenFileNameA(&ofn))
			{
				const std::string filepath = filename;
				const std::string replace_path = "raw\\xmodel\\";

				std::size_t pos = filepath.find(replace_path) + replace_path.length();
				std::string loc_filepath = filepath.substr(pos);

				AddProp(epw.epair->key, loc_filepath.c_str());
			}
			else
			{
				//game::printf_to_console("filedialog: canceled");
			}
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

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(4.0f, 0.0f));
			if(ColorEdit3_HelperUndo(&helper, "##value_color", col, ImGuiColorEditFlags_Float))
			{
				char col_str_buf[64] = {};
				if (sprintf_s(col_str_buf, "%.3f %.3f %.3f", col[0], col[1], col[2]))
				{
					AddProp(epw.epair->key, col_str_buf, &helper);
				}
			}
			ImGui::PopStyleVar(2);
		}
	}

	// move object to new point (does not change "local" object origin)
	void brush_moveto(game::brush_t_with_custom_def* b, const float* new_origin, const float* old_origin, bool snap = true)
	{
		game::vec3_t delta;

		// calc distance between centers
		VectorSubtract(new_origin, old_origin, delta);
		
		game::Brush_Move(delta, b, snap);
	}

	// move and center object (using bounds) around new point (changes "local" object origin)
	void brush_moveto_center(game::brush_t_with_custom_def* b, const float* new_origin, bool snap = true)
	{
		game::vec3_t v_mid;
		
		// get center of the brush
		for (int j = 0; j < 3; j++) {
			v_mid[j] = b->mins[j] + abs((b->maxs[j] - b->mins[j]) * 0.5f);
		}
		
		// calc distance between centers
		VectorSubtract(new_origin, v_mid, v_mid);
		
		game::Brush_Move(v_mid, b, snap);
	}

	void gui_entprop_add_value_vec3(const epair_wrapper& epw, float* vec_in, int row = 0)
	{
		bool dirty = false;
		char vec3_str_buf[64] = {};
		
		float* vec3 = vec_in;
		float temp_origin[] = { vec_in[0], vec_in[1], vec_in[2] };

		// do not edit the input vec directly if we modify an entity thats build using brushes (eg. lights, script_origins etc)
		// Brush_Move updates the key
		if(epw.is_fixedsize)
		{
			vec3 = temp_origin;
		}

		// avoid hashe collisions
		ImGui::PushID(row);

		addprop_helper_s helper = {};
		helper.is_origin = epw.type == ORIGIN;
		helper.is_angle = epw.type == ANGLES;
		
		if (game::multiple_edit_entities) {
			ImGui::BeginDisabled(true);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

		const float line_height		= ImGui::GetFrameHeight();
		const auto  button_size		= ImVec2(line_height, line_height);
		const float widget_spacing	= 4.0f;
		const float widget_width	= (ImGui::GetContentRegionAvail().x - (3.0f * button_size.x) - (2.0f * widget_spacing)) * 0.33333f;

		const float window_width	= ImGui::GetWindowWidth();
		const bool  min_window_width_origin = window_width < 460.0f;

		// -------
		// -- X --
		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.17f, 0.17f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.17f, 0.17f, 1.0f));
		ImGui::ButtonEx("X", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		if (!min_window_width_origin)
		{
			ImGui::SetNextItemWidth(widget_width);
		}
		else
		{
			ImGui::SetNextItemWidth(-1);
		}
		
		if(DragFloat_HelperUndo(&helper, "##X", &vec3[0], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		// -------
		// -- Y --
		
		if(!min_window_width_origin) 
		{
			ImGui::SameLine(0, widget_spacing);
		}
			
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.185f, 0.6f, 0.23f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.185f, 0.6f, 0.23f, 1.0f));
		ImGui::ButtonEx("Y", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		if (!min_window_width_origin) 
		{
			ImGui::SetNextItemWidth(widget_width);
		}
		else
		{
			ImGui::SetNextItemWidth(-1);
		}
		
		if(DragFloat_HelperUndo(&helper, "##Y", &vec3[1], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		// -------
		// -- Z --

		if(!min_window_width_origin) 
		{
			ImGui::SameLine(0, widget_spacing);
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.165f, 0.375f, 0.69f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.165f, 0.375f, 0.69f, 1.0f));
		ImGui::ButtonEx("Z", button_size, ImGuiButtonFlags_MouseButtonMiddle);
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		if (!min_window_width_origin)
		{
			ImGui::SetNextItemWidth(widget_width);
		}
		else
		{
			ImGui::SetNextItemWidth(-1);
		}
		
		if(DragFloat_HelperUndo(&helper, "##Z", &vec3[2], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		ImGui::PopStyleVar();
		ImGui::PopID();
		
		if (game::multiple_edit_entities) 
		{
			ImGui::EndDisabled();
			return;
		}
		
		if(dirty)
		{
			if(const auto ctype = epw.eclass->classtype; 
				epw.type == ORIGIN && (ctype == game::ECLASS_LIGHT || ctype == game::ECLASS_NODE || ctype == game::ECLASS_RADIANT_NODE))
			{
				//game::printf_to_console("Inspector: moving entity/brush using Brush_Move");
				
				const auto sb = game::g_selected_brushes();
				brush_moveto(sb->def, vec3, vec_in);

				if (sprintf_s(vec3_str_buf, "%.3f %.3f %.3f", vec3[0], vec3[1], vec3[2])) {
					AddProp(epw.epair->key, vec3_str_buf, &helper);
				}
			}
			
			else if (sprintf_s(vec3_str_buf, "%.3f %.3f %.3f", vec3[0], vec3[1], vec3[2])) {
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
				gui_entprop_add_value_vec3(epw, edit_entity->origin, row);
				return;
			}
			
			
		case ANGLES:
			float angles[3] = { 0.0f, 0.0f, 0.0f };
			if (sscanf(epw.epair->value, "%f %f %f", &angles[0], &angles[1], &angles[2]) == 3)
			{
				gui_entprop_add_value_vec3(epw, angles, row);
				return;
			}
		}

		gui_entprop_add_value_text(epw, row);
	}
	
	
	void draw_entprops()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 2.0f));

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
						(selbrush && selbrush->def) || is_worldspawn)
				{
					if (const auto edit_entity = game::g_edit_entity();
							edit_entity && edit_entity->epairs)
					{
						// add all epairs to our vector
						for (auto epair = edit_entity->epairs; epair; epair = epair->next)
						{
							std::string key = utils::str_to_lower(epair->key);

							epair_wrapper eprop = {};
							eprop.entity	= edit_entity;
							eprop.eclass	= edit_entity->eclass;
							eprop.epair		= epair;
							eprop.type		= EPAIR_VALUETYPE::TEXT;
							eprop.v_speed	=  1.0f;
							eprop.v_min		= -FLT_MAX;
							eprop.v_max		=  FLT_MAX;
							eprop.is_fixedsize = edit_entity->eclass->fixedsize;
							
							if(!is_worldspawn)
							{
								if (key == "origin") 
								{
									eprop.type = EPAIR_VALUETYPE::ORIGIN;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "angles") 
								{
									eprop.type		= EPAIR_VALUETYPE::ANGLES;
									eprop.v_speed	= 0.1f;
									eprop_sorted.push_back(eprop);
									continue;
								}
							}
							else
							{
								if (key == "suncolor") 
								{
									eprop.type = EPAIR_VALUETYPE::COLOR;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "sundiffusecolor") 
								{
									eprop.type = EPAIR_VALUETYPE::COLOR;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "sundirection") 
								{
									eprop.type		= EPAIR_VALUETYPE::ANGLES;
									eprop.v_speed	= 0.1f;
									eprop_sorted.push_back(eprop);
									continue;
								}
							}

							if (key == "_color") 
							{
								eprop.type = EPAIR_VALUETYPE::COLOR;
								eprop_sorted.push_back(eprop);
								continue;
							}

							

							// *
							// type float

							if(!is_worldspawn)
							{
								if (key == "radius") 
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "height")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "exponent") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop.v_min		= 0.0f;
									eprop.v_max		= 100.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "fov_inner") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.1f;
									eprop.v_min		= 1.0f;
									eprop.v_max		= 136.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "fov_outer") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.1f;
									eprop.v_min		= 1.0f;
									eprop.v_max		= 136.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "intensity")
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop_sorted.push_back(eprop);
									continue;
								}
							}
							else
							{
								if (key == "ambient") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop.v_min		= 0.0f;
									eprop.v_max		= 2.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "sunlight") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop.v_min		= 0.0f;
									eprop.v_max		= 8.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "sunradiosity") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop.v_min		= 0.0f;
									eprop.v_max		= 100.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "diffusefraction") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop.v_min		= 0.0f;
									eprop.v_max		= 1.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "radiosityscale") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop.v_min		= 0.0f;
									eprop.v_max		= 100.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "contrastgain") 
								{
									eprop.type		= EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed	= 0.01f;
									eprop.v_min		= 0.0f;
									eprop.v_max		= 1.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}
							}

							if (key == "model") 
							{
								eprop.type = EPAIR_VALUETYPE::MODEL;
								eprop_sorted.push_back(eprop);
								continue;
							}

							if(key == "fx")
							{
								eprop.type = EPAIR_VALUETYPE::FX;
								eprop_sorted.push_back(eprop);
								continue;
							}

							// everything else is text
							eprop_sorted.push_back(eprop);
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
							const bool is_classname = !_stricmp(ep.epair->key, "classname");
							
							ImGui::PushID(row);
							ImGui::TableNextColumn();
							{
								// do not allow to edit the origin key
								ImGui::BeginDisabled(is_classname || ep.type == EPAIR_VALUETYPE::ORIGIN || ep.type == EPAIR_VALUETYPE::FX);
								gui_entprop_add_key(ep.epair, row);
								ImGui::EndDisabled();
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
									gui_entprop_add_value_vec3(ep, row);
									break;

								case EPAIR_VALUETYPE::ANGLES:
									gui_entprop_add_value_vec3(ep, row);
									break;

								/*case EPAIR_VALUETYPE::FX:
									gui_entprop_add_text_fileprompt(ep, row);
									break;*/

								default:
								case EPAIR_VALUETYPE::FX:
								case EPAIR_VALUETYPE::MODEL:
								case EPAIR_VALUETYPE::TEXT:

									// do not allow to edit classname - worldspawn
									ImGui::BeginDisabled(is_classname && is_worldspawn);
									gui_entprop_add_value_text(ep, row);
									ImGui::EndDisabled();
									break;
								}
							}

							ImGui::TableNextColumn();
							{
								if(ep.type == EPAIR_VALUETYPE::FX)
								{
									gui_entprop_effect_fileprompt(ep, row);
								}

								else if (ep.type == EPAIR_VALUETYPE::MODEL)
								{
									gui_entprop_model_fileprompt(ep, row);
								}

								else if (ep.type != EPAIR_VALUETYPE::ORIGIN && !is_classname)
								{
									if (ImGui::Button("x", ImVec2(28, ImGui::GetFrameHeight())))
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
				(selbrush && selbrush->def) || (sel_list_ent && !_stricmp(sel_list_ent->name, "worldspawn")))
			{
				if (ImGui::Button("Add new key-value pair", ImVec2(-6.0f, 0.0f)))
				{
					kvp_helper._in_use = true;
					kvp_helper.key_set_focus = true;
				}
			}

			ImGui::TreePop();

			if(dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.enabled)
			{
				separator_for_treenode();
			}
		}

		ImGui::PopStyleVar(2);
		SPACING(0.0f, 0.01f);
	}
	
	void menu(ggui::imgui_context_menu& menu)
	{
		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (menu.bring_tab_to_front)
		{
			menu.bring_tab_to_front = false;
			ImGui::SetNextWindowFocus();
		}
		
		if(!ImGui::Begin("Entity##window", &menu.menustate, ImGuiWindowFlags_NoCollapse))
		{
			menu.inactive_tab = true;
			ImGui::End();
			return;
		}

		menu.inactive_tab = false;

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

		//SEPERATORV(0.0f);

		if(dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.enabled)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

			if (ImGui::TreeNodeEx("Surface Inspector", dvars::gui_props_classlist_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
			{
				SPACING(0.0f, 0.01f);
				ggui::surface_inspector::controls();

				ImGui::TreePop();
			}

			ImGui::PopStyleVar();
		}

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
		auto& menu = ggui::state.czwnd.m_entity;
		if (menu.inactive_tab && menu.menustate)
		{
			menu.bring_tab_to_front = true;
			return;
		}
		
		components::gui::toggle(ggui::state.czwnd.m_entity, 0, true);
	}

	void register_dvars()
	{
		dvars::gui_props_classlist_defaultopen = dvars::register_bool(
			/* name		*/ "gui_props_classlist_defaultopen",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "property editor - default state for treenode classlist");
		
		dvars::gui_props_comments_defaultopen = dvars::register_bool(
			/* name		*/ "gui_props_comments_defaultopen",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "property editor - default state for treenode comments");

		dvars::gui_props_spawnflags_defaultopen = dvars::register_bool(
			/* name		*/ "gui_props_spawnflags_defaultopen",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "property editor - default state for treenode spawnflags");
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
