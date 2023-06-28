#include "std_include.hpp"
#include "commdlg.h"

namespace ggui
{
	const entity_dialog::template_kvp eclass_worldspawn_templates[] =
	{
		{ "_color", "0.9 0.9 1.0" },
		{ "ambient", "0.2" },
		{ "bouncefraction", "0.2" },
		{ "contrastgain", "0.3" },
		{ "diffusefraction", "0.5" },
		{ "radiosityscale", "1.4" },
		{ "reflection_ignore_portals", "1" },
		{ "suncolor", "1.0 0.9 0.8" },
		{ "sundiffusecolor", "0.95 0.8 0.85" },
		{ "sundirection", "-60.0 275.0 0.0" },
		{ "sunlight", "2.4" },
		{ "sunradiosity", "1.5" },
		{ "sunIsPrimaryLight", "1" },
	};

	const entity_dialog::template_kvp eclass_light_templates[] =
	{
		{ "_color", "1.0 1.0 1.0" },
		{ "exponent", "1.0" },
		{ "def", "light_point_linear" },
		{ "fov_inner", "60.0" },
		{ "fov_outer", "90.0" },
		{ "intensity", "1.0" },
		{ "radius", "400" },
	};

	const entity_dialog::template_kvp eclass_trigger_templates[] =
	{
		{ "cursorhint", "HINT_ACTIVATE" },
		{ "delay", "1" },
		{ "hintstring", "my hintstring" },
		{ "target", "my_target" },
		{ "targetname", "my_targetname" },
	};

	const entity_dialog::template_kvp eclass_misc_templates[] =
	{
		{ "model", "axis" },
		{ "modelscale", "1" },
		{ "target", "my_target" },
		{ "targetname", "my_targetname" },
	};

	const entity_dialog::template_kvp eclass_fx_templates[] =
	{
		{ "loopfx", "1" },
		{ "loop_wait", "5" },
		{ "is_sound", "1" },
		{ "soundalias", "emt_metal_rattle_ring" },
	};

	const entity_dialog::template_kvp eclass_generic_templates[] =
	{
		{ "target", "my_target" },
		{ "targetname", "my_targetname" },
	};

	void entity_dialog::get_eclass_template(const template_kvp*& tkvp, int* size_out)
	{
		const auto eent = game::g_edit_entity();

		const auto ctype = static_cast<ECLASS_TYPE>(eent->eclass->classtype);
		const bool is_trigger = utils::starts_with(eent->eclass->name, "trigger_");

		if (is_trigger || ctype & (CLASS_TRIGGER_RADIUS | CLASS_TRIGGER_DISC))
		{
			tkvp = eclass_trigger_templates;
			*size_out = IM_ARRAYSIZE(eclass_trigger_templates);
		}
		else if (ctype & CLASS_LIGHT)
		{
			tkvp = eclass_light_templates;
			*size_out = IM_ARRAYSIZE(eclass_light_templates);
		}
		else if ((ctype & CLASS_WORLDSPAWN) == 0)
		{
			tkvp = eclass_worldspawn_templates;
			*size_out = IM_ARRAYSIZE(eclass_worldspawn_templates);
		}
		else if (ctype & (CLASS_MODEL | CLASS_PREFAB))
		{
			tkvp = eclass_misc_templates;
			*size_out = IM_ARRAYSIZE(eclass_misc_templates);
		}
		else if (utils::string_equals(eent->eclass->name, "fx_origin"))
		{
			tkvp = eclass_fx_templates;
			*size_out = IM_ARRAYSIZE(eclass_fx_templates);
		}
		else
		{
			tkvp = eclass_generic_templates;
			*size_out = IM_ARRAYSIZE(eclass_generic_templates);
		}
	}

	// *
	// Helper functions

	void entity_dialog::add_prop(const char* key, const char* value, addprop_helper_s* helper)
	{
		// undo if no helper defined or helper wants to add an undo
		const bool should_add_undo = !helper || helper && helper->add_undo;

		if (const auto	edit_entity = game::g_edit_entity();
						edit_entity)
		{
			if (should_add_undo)
			{
				//if(!helper) game::printf_to_console("Start Undo without helper.");
				//else game::printf_to_console("Start Undo with helper->add_undo = %d", helper->add_undo);

				game::Undo_ClearRedo();
				game::Undo_GeneralStart("set key value pair");
			}

			if (game::multiple_edit_entities)
			{
				FOR_ALL_SELECTED_BRUSHES(sb)
				{
					std::string class_str;

					if (sb->owner->firstActive->eclass && sb->owner->firstActive->eclass->name)
					{
						class_str = utils::str_to_lower(sb->owner->firstActive->eclass->name);
					}

					if (class_str != "worldspawn")
					{

						if (should_add_undo)
						{
							game::Undo_AddEntity_W(sb->owner->firstActive);
						}

						game::SetKeyValue(sb->owner->firstActive, key, value);

						if (!strcmp("origin", key) && edit_entity)
						{
							get_vec3_for_key_from_entity((game::entity_s*)edit_entity, edit_entity->origin, "origin");
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
					get_vec3_for_key_from_entity((game::entity_s*)edit_entity, edit_entity->origin, "origin");
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

	void entity_dialog::del_prop(const char* key, bool overwrite_classname_check)
	{
		if (const auto	edit_entity = game::g_edit_entity();
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
				FOR_ALL_SELECTED_BRUSHES(sb)
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

	char* entity_dialog::get_value_for_key_from_epairs(game::epair_t*& e, const char* key)
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

	bool entity_dialog::get_value_for_key_from_entity(game::entity_s* ent, float* value, const char* keyname)
	{
		if (const auto	value_str = get_value_for_key_from_epairs(ent->epairs, keyname);
						value_str)
		{
			return sscanf(value_str, "%f", value) == 1;
		}

		return false;
	}

	bool entity_dialog::get_vec3_for_key_from_entity(game::entity_s* ent, float* vec3, const char* keyname)
	{
		if (const auto	value_str = get_value_for_key_from_epairs(ent->epairs, keyname);
						value_str)
		{
			return sscanf(value_str, "%f %f %f", &vec3[0], &vec3[1], &vec3[2]) == 3;
		}

		return false;
	}

	bool entity_dialog::has_key_value_pair(game::entity_s_def* ent, const char* key)
	{
		game::epair_t* ep = ent->epairs;
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

	bool entity_dialog::has_key_with_value(game::entity_s_def* ent, const char* key, const char* value)
	{
		game::epair_t* ep = ent->epairs;
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

		return strcmp(ep->value, value) == 0;
	}


	// adds grey horizontal separator
	void entity_dialog::separator_for_treenode()
	{
		SPACING(0.0f, 6.0f);
		ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.3f, 0.3f, 0.3f, 0.9f));
		ImGui::Separator();
		ImGui::PopStyleColor();
	}

	// *
	// dragfloat helper - add a single undo before starting to edit the value
	bool entity_dialog::drag_float_helper_undo(addprop_helper_s* helper, const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
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
	bool entity_dialog::color_edit3_helper_undo(addprop_helper_s* helper, const char* label, float col[3], ImGuiColorEditFlags flags, const ImVec2& color_button_size)
	{
		auto im_col = ImVec4(col[0], col[1], col[2], 1.0f);
		if (ImGui::ColorButton(label, im_col, flags, color_button_size))
		//if (ImGui::ColorEdit3(label, col, flags))
		{
			ImGui::OpenPopup("hi-picker");
			
		}

		if (ImGui::BeginPopup("hi-picker"))
		{
			ImGui::ColorPicker4("##picker", &im_col.x, ImGuiColorEditFlags_None, nullptr);
			ImGui::EndPopup();

			col[0] = im_col.x;
			col[1] = im_col.y;
			col[2] = im_col.z;
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

	void entity_dialog::on_mapload_intercept()
	{
		const auto gui = GET_GUI(ggui::entity_dialog);
		gui->m_sel_list_ent = nullptr;
		gui->m_edit_entity_class = nullptr;
		gui->m_edit_entity_changed = false;
		gui->m_edit_entity_changed_should_scroll = false;
	}

	// UpdateSelection
	void entity_dialog::on_update_selection_intercept()
	{
		const auto prefs = GET_GUI(entity_info);
		prefs->update_entity_list();
		prefs->m_update_on_unselect = true;

		// #

		if (const auto	g_edit_ent = game::g_edit_entity();
						g_edit_ent)
		{
			const auto gui = GET_GUI(ggui::entity_dialog);

			// update our selected entity (also updates on escape)
			if (gui->m_edit_entity_class != g_edit_ent->eclass)
			{
				gui->m_sel_list_ent = g_edit_ent->eclass;
				gui->m_edit_entity_class = g_edit_ent->eclass;
				gui->m_edit_entity_changed = true;
				gui->m_edit_entity_changed_should_scroll = true;
			}

			if (g_edit_ent && g_edit_ent->eclass->name)
			{
				if (utils::str_to_lower(g_edit_ent->eclass->name) != "worldspawn"s)
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
							gui->add_prop("origin", origin_str_buf, &helper);
						}
					}
				}
			}
		}
	}

	// the classlist is only build once at startup. We need to do the same as "game::g_eclass"
	// will hold pretty much every model/prefab that is placed on the map -> list indices wont match up with the original entity window
	void entity_dialog::fill_classlist_intercept()
	{
		auto& classlist = GET_GUI(ggui::entity_dialog)->m_classlist;

		for (auto pec = game::g_eclass(); pec; pec = pec->next)
		{
			classlist.push_back(pec);
		}
	}



	// *
	// gui

	void entity_dialog::draw_classlist()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

		if (ImGui::TreeNodeEx("Classlist", dvars::gui_props_classlist_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			SPACING(0.0f, 0.01f);

			if (ImGui::BeginListBox("##entlistbox", ImVec2(-4, 12 * ImGui::GetTextLineHeightWithSpacing())))
			{
				int index = 0;
				for (const auto pec : m_classlist)
				{
					const bool is_selected = (m_sel_list_ent == pec);
					if (ImGui::Selectable(pec->name, is_selected, ImGuiSelectableFlags_AllowDoubleClick))
					{
						m_sel_list_ent = pec;
						m_edit_entity_changed = true;
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

					if (is_selected && m_edit_entity_changed_should_scroll)
					{
						ImGui::SetScrollHereY();
						m_edit_entity_changed_should_scroll = false;
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

	void entity_dialog::draw_comments(const float start_indent)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

		{
			static char empty_text = '\0';

			char* comment_buf = &empty_text;
			auto  comment_buf_len = strlen(comment_buf);

			if (m_sel_list_ent && m_sel_list_ent->comments)
			{
				comment_buf = m_sel_list_ent->comments;
				comment_buf_len = strlen(m_sel_list_ent->comments);
			}

			ImGui::InputTextMultiline("##entcomments", comment_buf, comment_buf_len, ImVec2(-4 - start_indent, 0), ImGuiInputTextFlags_ReadOnly);
		}

		ImGui::PopStyleVar();
	}

	void entity_dialog::draw_checkboxes()
	{
		// on changed selection -> update spawnflags / checkboxes
		if (const auto	selected_brush = game::g_selected_brushes();
						selected_brush && selected_brush->def)
		{
			if (const auto	edit_entity = game::g_edit_entity();
							edit_entity && edit_entity->epairs)
			{
				// do spawnflags
				int flag = 0;

				if (char*	value_str = get_value_for_key_from_epairs(edit_entity->epairs, "spawnflags");
							value_str)
				{
					flag = atoi(value_str);
				}

				// set checkboxes accordingly, TODO :: move this
				for (auto i = 0; i < 12; i++)
				{
					m_checkboxflags_states[i] = !!(flag & 1 << i);
				}
			}
		}

		// render checkboxes

		const auto wnd_size = ImGui::GetWindowSize();
		

		//if (ImGui::TreeNodeEx("Spawnflags", dvars::gui_props_spawnflags_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			if (ImGui::TreeNodeEx("Gametype Specific##who_needs_these", ImGuiTreeNodeFlags_SpanFullWidth))
			{
				SPACING(0.0f, 0.01f);
				ImGui::PushStyleCompact();

				int cb_num = 8;
				
				const auto pre_checkbox_cursor = ImGui::GetCursorPos();

				static float spawnflags_gametype_width = 0.0f;
				ggui::toolbox_dialog::center_horz_begin(spawnflags_gametype_width);
				{
					if (ImGui::Checkbox("!Easy", &m_checkboxflags_states[cb_num]))
					{
						SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, m_checkboxflags_states[cb_num], 0);
						game::SetSpawnFlags(cb_num);
					} cb_num++;

					ImGui::SameLine(90.0f + pre_checkbox_cursor.x, 8.0f);
					if (ImGui::Checkbox("!Hard", &m_checkboxflags_states[cb_num]))
					{
						SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, m_checkboxflags_states[cb_num], 0);
						game::SetSpawnFlags(cb_num);
					} cb_num++;


					if (ImGui::Checkbox("!Medium", &m_checkboxflags_states[cb_num]))
					{
						SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, m_checkboxflags_states[cb_num], 0);
						game::SetSpawnFlags(cb_num);
					} cb_num++;

					ImGui::SameLine(90.0f + pre_checkbox_cursor.x, 8.0f);
					if (ImGui::Checkbox("!Deathmatch", &m_checkboxflags_states[cb_num]))
					{
						SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + cb_num], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, m_checkboxflags_states[cb_num], 0);
						game::SetSpawnFlags(cb_num);
					} cb_num++;

					ggui::toolbox_dialog::center_horz_end(spawnflags_gametype_width);
				}

				ImGui::PopStyleCompact();
				SPACING(0.0f, 0.01f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("General", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
			{
				SPACING(0.0f, 0.01f);
				ImGui::PushStyleCompact();

				const auto pre_checkbox_cursor = ImGui::GetCursorPos();
				static float spawnflags_general_width = 0.0f;
				ggui::toolbox_dialog::center_horz_begin(spawnflags_general_width);
				{
					for (auto i = 0; i < 8; i++)
					{
						/*if(i % 2)
						{
							ImGui::SameLine(90.0f + pre_checkbox_cursor.x, 8.0f);
						}*/

						/*if (i == 4) {
							ImGui::SetCursorPosY(pre_checkbox_cursor.y);
						}

						if (i >= 4) {
							ImGui::SetCursorPosX(pre_checkbox_cursor.x + (wnd_size.x * 0.5f));
						}*/

						bool flag_in_use = false;
						std::string flagname;

						if (m_sel_list_ent && m_sel_list_ent->flagnames[i] && m_sel_list_ent->flagnames[i][0] != 0)
						{
							flag_in_use = true;
							flagname = m_sel_list_ent->flagnames[i];
						}

						if(flagname.empty())
						{
							break;
						}

						flagname += "##entcheckbox"s + std::to_string(i);

						if (ImGui::Checkbox(flagname.c_str(), &m_checkboxflags_states[i]))
						{
							if (flag_in_use)
							{
								SendMessageA(game::entitywnd_hwnds[game::E_ENTITYWND_HWNDS::ENTWND_CHECK1 + i], WM_NCLBUTTONDOWN | WM_INPUTLANGCHANGEREQUEST, m_checkboxflags_states[i], 0);
								game::SetSpawnFlags(i);
							}
						}
					}

					ggui::toolbox_dialog::center_horz_end(spawnflags_general_width);
				}
				

				ImGui::PopStyleCompact();
				SPACING(0.0f, 0.01f);
				ImGui::TreePop();
			}

			//ImGui::TreePop();
			//separator_for_treenode();
		}

		//ImGui::PopStyleVar(); // ImGuiStyleVar_IndentSpacing
		//SPACING(0.0f, 0.01f);
	}

	void entity_dialog::gui_entprop_new_keyvalue_pair()
	{
		// full width input text without label spacing
		ImGui::SetNextItemWidth(-1);

		// set focus on key input?
		if (kvp_helper.key_set_focus)
		{
			ImGui::SetKeyboardFocusHere();
		}

		const bool key_has_color = !kvp_helper.key_valid;
		if (key_has_color) 
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.49f, 0.2f, 0.2f, 1.0f));
		}

		if (ImGui::InputText("##key", kvp_helper.key_buffer, EPROP_INPUTLEN))
		{
			kvp_helper.key_valid = strlen(kvp_helper.key_buffer);
		}

		if (key_has_color) 
		{
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

				const bool value_has_color = !kvp_helper.value_valid;
				if (value_has_color) 
				{
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.49f, 0.2f, 0.2f, 1.0f));
				}

				if (ImGui::InputText("##value", kvp_helper.value_buffer, EPROP_INPUTLEN/*, ImGuiInputTextFlags_EnterReturnsTrue*/))
				{
					kvp_helper.value_valid = strlen(kvp_helper.value_buffer);
				}

				if (value_has_color) 
				{
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
					add_prop(kvp_helper.key_buffer, kvp_helper.value_buffer);
					kvp_helper.reset();

					return;
				}

				if (!ImGui::IsItemActive())
				{
					// if value is no longer active but had focus on the last frame + key input text is not active
					if (kvp_helper.value_active && !kvp_helper.key_active)
					{
						// is the value valid? -> add KVP
						if (kvp_helper.value_valid)
						{
							add_prop(kvp_helper.key_buffer, kvp_helper.value_buffer);
						}

						// reset everything otherwise or after the KVP was added
						kvp_helper.reset();
						return;
					}

					// key and value valid and none of them are active? -> add KVP
					if (kvp_helper.key_valid && kvp_helper.value_valid && !kvp_helper.key_active && !kvp_helper.value_active)
					{
						add_prop(kvp_helper.key_buffer, kvp_helper.value_buffer);
						kvp_helper.reset();
						return;
					}

					// reset when key nor value active
					if (!kvp_helper.key_active)
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

	void entity_dialog::gui_entprop_add_key(game::epair_t* epair, int row)
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
			add_prop(_edit_buf_key[row], epair->value);

			// delete the old key
			del_prop(epair->key, true);

			// ^ needs a sorting system as newest is always on top

			_edit_buf_key_dirty[row] = false;
		}

			// if InputText ^ is not active and buf dirty (user did not submit the change via enter) -> restore buf on next frame
			if (!ImGui::IsItemActive() && _edit_buf_key_dirty[row])
			{
				_edit_buf_key_dirty[row] = false;
			}
	}

	void entity_dialog::gui_entprop_add_value_text(const epair_wrapper& epw, int row)
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
				add_prop(epw.epair->key, _edit_buf_value[row]);

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
						const auto m_selector = GET_GUI(ggui::modelselector_dialog);
						add_prop(epw.epair->key, m_selector->m_preview_model_name.c_str());
					}
				}
			}
	}

	void entity_dialog::gui_entprop_effect_fileprompt(const epair_wrapper& epw, [[maybe_unused]] int row)
	{
		ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(ImGuiCol_FrameBg));
		imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

		if (ImGui::Button("..##filepromt", ImVec2(28, ImGui::GetFrameHeight())))
		{
			// logic :: ggui::file_dialog_frame
			if (dvars::gui_use_new_filedialog->current.enabled)
			{
				std::string path_str;

				const auto egui = GET_GUI(ggui::entity_dialog);
				path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
				path_str += "\\raw\\fx";
				

				const auto file = GET_GUI(ggui::file_dialog);
				file->set_default_path(path_str);
				file->set_file_handler(FX_CHANGE);
				file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
				file->set_file_ext(".efx");
				file->set_blocking();
				file->open();
			}
			else
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

					add_prop(epw.epair->key, loc_filepath.c_str());

					// stop old effect
					components::command::execute("fx_stop");
				}
			}
		}

		imgui::PopStyleColor();
		imgui::PopStyleVar();
	}

	// both misc_model and misc_prefab
	void entity_dialog::gui_entprop_model_fileprompt(const epair_wrapper& epw, [[maybe_unused]] int row)
	{
		ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(ImGuiCol_FrameBg));
		imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

		if (ImGui::Button("..##filepromt", ImVec2(28, ImGui::GetFrameHeight())))
		{
			// logic :: ggui::file_dialog_frame
			if (dvars::gui_use_new_filedialog->current.enabled)
			{
				std::string path_str;

				const auto egui = GET_GUI(ggui::entity_dialog);
				const bool is_prefab = (game::g_edit_entity()->eclass->classtype & 0x10) != 0;

				if (is_prefab)
				{
					path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "mapspath");
					path_str += "\\prefabs";
				}
				else
				{
					path_str = egui->get_value_for_key_from_epairs(game::g_qeglobals->d_project_entity->epairs, "basepath");
					path_str += "\\raw\\xmodel";
				}

				const auto file = GET_GUI(ggui::file_dialog);
				file->set_default_path(path_str);
				file->set_file_handler(is_prefab ? ggui::FILE_DIALOG_HANDLER::MISC_PREFAB_CHANGE : ggui::FILE_DIALOG_HANDLER::MISC_MODEL_CHANGE);
				file->set_file_op_type(ggui::file_dialog::FileDialogType::OpenFile);
				file->set_file_ext(is_prefab ? ".map" : "");
				file->set_blocking();
				file->open();
			}
			else
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

					add_prop(epw.epair->key, loc_filepath.c_str());
				}
			}
		}

		imgui::PopStyleColor();
		imgui::PopStyleVar();
	}

	void entity_dialog::gui_entprop_add_value_slider(const epair_wrapper& epw)
	{
		addprop_helper_s helper = {};
		helper.is_generic_slider = true;

		bool dirty = false;
		auto val = static_cast<float>(atof(epw.epair->value));

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

		const float line_height = ImGui::GetFrameHeight();
		const auto  button_size = ImVec2(line_height - 2.0f, line_height);

		const auto framebg = imgui::ColorConvertU32ToFloat4(imgui::GetColorU32(ImGuiCol_FrameBg));
		if (imgui::pre_description_button(ICON_FA_EXCHANGE_ALT, button_size, ImVec4(0.55f, 0.55f, 0.55f, 1.0f), framebg + ImVec4(0.05f, 0.05f, 0.05f, 0.0f)))
		{
			val = 0.0f;
			dirty = true;
		}

		// full with input text without label spacing
		ImGui::SetNextItemWidth(-1);

		if (drag_float_helper_undo(&helper, "##value_slider", &val, epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		if (dirty)
		{
			char val_str_buf[32] = {};
			if (sprintf_s(val_str_buf, "%.2f", val))
			{
				add_prop(epw.epair->key, val_str_buf, &helper);
			}
		}

		imgui::PopStyleVar();
	}

	void entity_dialog::gui_entprop_add_value_color(const epair_wrapper& epw)
	{
		addprop_helper_s helper = {};
		helper.is_color = true;

		float col[3] = { 1.0f, 1.0f, 1.0f };
		if (sscanf(epw.epair->value, "%f %f %f", &col[0], &col[1], &col[2]) == 3)
		{
			// full with input text without label spacing
			ImGui::SetNextItemWidth(-1);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(4.0f, 0.0f));

			const auto calc_witdth = ImGui::CalcItemWidth();

			if (color_edit3_helper_undo(&helper, "##value_color", col, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs, ImVec2(calc_witdth, 0.0f)))
			{
				char col_str_buf[64] = {};
				if (sprintf_s(col_str_buf, "%.3f %.3f %.3f", col[0], col[1], col[2]))
				{
					add_prop(epw.epair->key, col_str_buf, &helper);
				}
			}
			ImGui::PopStyleVar(2);
		}
	}

	// move object to new point (does not change "local" object origin)
	void entity_dialog::brush_moveto(game::brush_t_with_custom_def* b, const float* new_origin, const float* old_origin, bool snap)
	{
		game::vec3_t delta;

		// calc distance between centers
		VectorSubtract(new_origin, old_origin, delta);

		game::Brush_Move(delta, b, snap);
	}

	// move and center object (using bounds) around new point (changes "local" object origin)
	void entity_dialog::brush_moveto_center(game::brush_t_with_custom_def* b, const float* new_origin, bool snap)
	{
		game::vec3_t v_mid;

		// get center of the brush
		for (int j = 0; j < 3; j++) 
		{
			v_mid[j] = b->mins[j] + abs((b->maxs[j] - b->mins[j]) * 0.5f);
		}

		// calc distance between centers
		VectorSubtract(new_origin, v_mid, v_mid);

		game::Brush_Move(v_mid, b, snap);
	}

	void entity_dialog::gui_entprop_add_value_vec3(const epair_wrapper& epw, float* vec_in, int row)
	{
		bool dirty = false;
		char vec3_str_buf[64] = {};

		float* vec3 = vec_in;
		float temp_origin[] = { vec_in[0], vec_in[1], vec_in[2] };

		// do not edit the input vec directly if we modify an entity thats build using brushes (eg. lights, script_origins etc)
		// Brush_Move updates the key
		if (epw.is_fixedsize)
		{
			vec3 = temp_origin;
		}

		// avoid hash collisions
		ImGui::PushID(row);

		addprop_helper_s helper = {};
		helper.is_origin = epw.type == ORIGIN;
		helper.is_angle = epw.type == ANGLES;

		if (game::multiple_edit_entities) 
		{
			ImGui::BeginDisabled(true);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

		const float line_height = ImGui::GetFrameHeight();
		const auto  button_size = ImVec2(line_height - 2.0f, line_height);
		const float widget_spacing = 4.0f;
		const float widget_width = (ImGui::GetContentRegionAvail().x - (3.0f * button_size.x) - (2.0f * widget_spacing)) * 0.333333f;

		const float window_width = ImGui::GetWindowWidth();
		const bool  min_window_width_origin = window_width < 460.0f;

		// -------
		// -- X --

		if (imgui::pre_description_button("X", button_size, ImVec4(0.84f, 0.55f, 0.53f, 1.0f), ImVec4(0.21f, 0.16f, 0.16f, 1.0f)))
		{
			vec3[0] = 0.0f;
			dirty = true;
		}

		if (!min_window_width_origin)
		{
			ImGui::SetNextItemWidth(widget_width);
		}
		else
		{
			ImGui::SetNextItemWidth(-1);
		}

		if (drag_float_helper_undo(&helper, "##X", &vec3[0], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		// -------
		// -- Y --

		if (!min_window_width_origin)
		{
			ImGui::SameLine(0, widget_spacing);
		}

		if (imgui::pre_description_button("Y", button_size, ImVec4(0.73f, 0.78f, 0.5f, 1.0f), ImVec4(0.17f, 0.18f, 0.15f, 1.0f)))
		{
			{
				vec3[1] = 0.0f;
				dirty = true;
			}
		}

		if (!min_window_width_origin)
		{
			ImGui::SetNextItemWidth(widget_width);
		}
		else
		{
			ImGui::SetNextItemWidth(-1);
		}

		if (drag_float_helper_undo(&helper, "##Y", &vec3[1], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
		{
			dirty = true;
		}

		// -------
		// -- Z --

		if (!min_window_width_origin)
		{
			ImGui::SameLine(0, widget_spacing);
		}

		if (imgui::pre_description_button("Z", button_size, ImVec4(0.67f, 0.71f, 0.79f, 1.0f), ImVec4(0.18f, 0.21f, 0.23f, 1.0f)))
		{
			vec3[2] = 0.0f;
			dirty = true;
		}

		if (!min_window_width_origin)
		{
			// fixes stutter
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 1.0f); // widget_width
		}
		else
		{
			ImGui::SetNextItemWidth(-1);
		}

		if (drag_float_helper_undo(&helper, "##Z", &vec3[2], epw.v_speed, epw.v_min, epw.v_max, "%.2f"))
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

		if (dirty)
		{
			if (const auto ctype = epw.eclass->classtype;
				epw.type == ORIGIN && (ctype == game::ECLASS_LIGHT || ctype == game::ECLASS_NODE || ctype == game::ECLASS_RADIANT_NODE))
			{
				//game::printf_to_console("Inspector: moving entity/brush using Brush_Move");

				const auto sb = game::g_selected_brushes();
				brush_moveto(sb->def, vec3, vec_in);

				if (sprintf_s(vec3_str_buf, "%.3f %.3f %.3f", vec3[0], vec3[1], vec3[2])) 
				{
					add_prop(epw.epair->key, vec3_str_buf, &helper);
				}
			}

			else if (sprintf_s(vec3_str_buf, "%.3f %.3f %.3f", vec3[0], vec3[1], vec3[2])) 
			{
				add_prop(epw.epair->key, vec3_str_buf, &helper);
			}
		}
	}

	void entity_dialog::gui_entprop_add_value_vec3(const epair_wrapper& epw, int row)
	{
		switch (epw.type)
		{

		case ORIGIN:
			if (const auto	edit_entity = game::g_edit_entity();
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

	void entity_dialog::draw_entprops(const float max_width, const float start_indent)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 2.0f));

		{
			// call EditProp (on Enter/submission)
			static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp;

			if (ImGui::BeginTable("##entprop_list", 3, flags, ImVec2(max_width, 0.0f)))
			{
				//ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 80.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 120.0f); //, 64.0f);
				ImGui::TableSetupColumn("##delete", ImGuiTableColumnFlags_WidthFixed, 32.0f);
				//ImGui::TableHeadersRow();

				int row = 0;
				eprop_sorted.clear();

				bool is_worldspawn = (m_sel_list_ent && !_stricmp(m_sel_list_ent->name, "worldspawn"));
				const auto selbrush = game::g_selected_brushes();

				// do not call every frame (fps) - call 'UpdateEntitySel' once on startup to pre-select the worldspawn
				if (/*selbrush && selbrush->def || */ m_update_worldspawn)
				{
					cdeclcall(void, 0x4972F0); // UpdateEntitySel - updates edit_entity <- worldspawn
					m_update_worldspawn = false;
				}

				// only draw entprops if something is selected or if nothing is selected and sel_list_ent == the worldspawn
				if (selbrush && selbrush->def || is_worldspawn)

				{
					if (const auto	edit_entity = game::g_edit_entity(); 
									edit_entity && edit_entity->epairs)
					{

						// #
						// #

						auto is_custom_type = [&](const std::string key, EPAIR_VALUETYPE& out_type) -> bool
						{
							for (auto& i : overriden_valuetypes)
							{
								if (i.key == key)
								{
									out_type = i.type;
									return true;
								}
							}

							return false;
						};

						// #
						// #

						// add all epairs to our vector
						for (auto epair = edit_entity->epairs; epair; epair = epair->next)
						{
							std::string key = utils::str_to_lower(epair->key);

							epair_wrapper eprop = {};
							eprop.entity = edit_entity;
							eprop.eclass = edit_entity->eclass;
							eprop.epair = epair;

							EPAIR_VALUETYPE temp_type;
							if (is_custom_type(key, temp_type))
							{
								eprop.type = temp_type;
							}
							else
							{
								eprop.type = EPAIR_VALUETYPE::TEXT;
							}
							
							eprop.v_speed = 1.0f;
							eprop.v_min = -FLT_MAX;
							eprop.v_max = FLT_MAX;
							eprop.is_fixedsize = edit_entity->eclass->fixedsize;

							if (!is_worldspawn)
							{
								if (key == "origin")
								{
									eprop.type = EPAIR_VALUETYPE::ORIGIN;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "angles")
								{
									eprop.type = EPAIR_VALUETYPE::ANGLES;
									eprop.v_speed = 0.1f;
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
									eprop.type = EPAIR_VALUETYPE::ANGLES;
									eprop.v_speed = 0.1f;
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

							if (!is_worldspawn)
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
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 100.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "fov_inner")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.1f;
									eprop.v_min = 1.0f;
									eprop.v_max = 136.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "fov_outer")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.1f;
									eprop.v_min = 1.0f;
									eprop.v_max = 136.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "intensity")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "modelscale")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.05f;
									eprop.v_max = 100.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}
							}
							else
							{
								if (key == "ambient")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 2.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "sunlight")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 8.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "sunradiosity")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 100.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "diffusefraction")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 1.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "radiosityscale")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 100.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "contrastgain")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 1.0f;
									eprop_sorted.push_back(eprop);
									continue;
								}

								if (key == "bouncefraction")
								{
									eprop.type = EPAIR_VALUETYPE::FLOAT;
									eprop.v_speed = 0.01f;
									eprop.v_min = 0.0f;
									eprop.v_max = 100.0f;
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

							if (key == "fx")
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
						for (auto& ep : eprop_sorted)
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
								if (ep.type == EPAIR_VALUETYPE::FX)
								{
									gui_entprop_effect_fileprompt(ep, row);
								}

								else if (ep.type == EPAIR_VALUETYPE::MODEL)
								{
									gui_entprop_model_fileprompt(ep, row);
								}

								else if (ep.type != EPAIR_VALUETYPE::ORIGIN /*&& !is_classname*/)
								{
									const auto bg_color = imgui::ColorConvertU32ToFloat4(imgui::GetColorU32(ImGuiCol_Button));

									bool show_delete_only = is_worldspawn && !is_classname;

									if (show_delete_only)
									{
										ImGui::PushFontFromIndex(ggui::E_FONT::BOLD_18PX);
										ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 5.0f));
										imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
										//ImGui::PushStyleColor(ImGuiCol_Button, bg_color);
										ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bg_color - ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
										ImGui::PushStyleColor(ImGuiCol_ButtonActive, bg_color + ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
										ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));
										ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetColorU32(ImGuiCol_FrameBg));

										if (ImGui::Button("x", ImVec2(28, ImGui::GetFrameHeight())))
										{
											del_prop(ep.epair->key);
										}

										ImGui::PopStyleColor(4);
										ImGui::PopStyleVar(2);
										ImGui::PopFont();
									}
									/*else if (ImGui::Button("v", ImVec2(28, ImGui::GetFrameHeight())))
									{
										ImGui::OpenPopup("kvp_popup");
									}*/

									else
									{
										ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
										ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
										ggui::context_menu_style_begin();

										if (ImGui::BeginCombo("##kvp_popup", nullptr, ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft))
										{
											imgui::BeginDisabled(is_classname);
											{
												if (ImGui::Selectable("Delete KVP"))
												{
													del_prop(ep.epair->key);
												}

												imgui::EndDisabled();
											}

											imgui::BeginDisabled(is_worldspawn);
											{
												// part of CMainFrame::OnSelectionKeyValue
												if (ImGui::Selectable("Select all by KVP"))
												{
													// # checks if only a single ent is selected
													// const auto sb_next = game::g_selected_brushes_next();
													// const auto sb_next_next = sb_next->next;
													// if (reinterpret_cast<DWORD>(sb_next) != 0x23F1864 /*game::currSelectedBrushes*/ && reinterpret_cast<DWORD>(sb_next_next) == 0x23F1864)

													game::Select_Deselect(true);

													auto b = game::g_active_brushes_next();
													game::selbrush_def_t** saved;

													if (reinterpret_cast<DWORD>(b) != *game::active_brushes_ptr)
													{
														do
														{
															saved = &b->next->prev;
															if (!game::FilterBrush(b, 0) && (b->brushflags & 2) == 0 && (b->brushflags & 0x20) == 0)
															{
																if (!b->owner)
																{
																	AssertS("!b->owner");
																}

																if (b->owner != game::g_world_entity())
																{
																	if (b->owner->firstActive != b->def->owner)
																	{
																		AssertS("b->owner->firstActive != b->def->owner");
																	}

																	if (has_key_with_value(reinterpret_cast<game::entity_s_def*>(b->owner->firstActive), ep.epair->key, ep.epair->value))
																	{
																		game::Brush_RemoveFromList(b);
																		game::Brush_AddToList2((game::brush_t_with_custom_def*)b); // these structs are a pain
																	}
																}
															}

															b = (game::selbrush_def_t*)saved;

														} while (reinterpret_cast<DWORD>(saved) != *game::active_brushes_ptr);
													}
												}

												imgui::EndDisabled();
											}

											if (ep.epair->key == "targetname"s || ep.epair->key == "target"s)
											{
												if (ImGui::Selectable("Select Connected"))
												{
													cdeclcall(void, 0x425550); // CMainFrame::OnSelectConneted
												}
											}

											// #

											auto assign_custom_type = [&](EPAIR_VALUETYPE type, std::string key) -> void
											{
												auto exists = false;
												for (auto& i : overriden_valuetypes)
												{
													if (i.key == key)
													{
														i.type = type;
														exists = true;
														break;
													}
												}

												if (!exists)
												{
													overriden_valuetypes.emplace_back(type, key);
												}
											};

											// #

											if (!is_classname)
											{
												SEPERATORV(0.0f);
												if (imgui::BeginMenu("Change Type"))
												{
													EPAIR_VALUETYPE temp_type;
													if (!is_classname && ep.type == TEXT || is_custom_type(ep.epair->key, temp_type))
													{
														if (ImGui::Selectable("Force Type Vec3"))
														{
															assign_custom_type(ANGLES, ep.epair->key);
														}

														if (ImGui::Selectable("Force Type Float"))
														{
															assign_custom_type(FLOAT, ep.epair->key);
														}

														if (ImGui::Selectable("Force Type Color"))
														{
															assign_custom_type(COLOR, ep.epair->key);
														}

														/*if (ImGui::Selectable("Force Type Model"))
														{
															assign_custom_type(MODEL, ep.epair->key);
														}

														if (ImGui::Selectable("Force Type Fx"))
														{
															assign_custom_type(FX, ep.epair->key);
														}*/

														if (ImGui::Selectable("Force Type Text"))
														{
															assign_custom_type(TEXT, ep.epair->key);
														}
													}

													imgui::EndMenu();
												}
											}

											ImGui::EndCombo();
										}

										imgui::PopStyleColor(2);
										ggui::context_menu_style_end();
									}
								}
							}

							ImGui::PopID();

							if (row < EPROP_MAX_ROWS)
							{
								row++;
							}
							else
							{
								game::console_error("MAX \"EPROP_MAX_ROWS\". Not drawing additional rows.");
								break;
							}
						}

						if (kvp_helper._in_use)
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

			if (const auto	 selbrush = game::g_selected_brushes();
							(selbrush && selbrush->def) || (m_sel_list_ent && !_stricmp(m_sel_list_ent->name, "worldspawn")))
			{
				const auto pbutton_cursorpos = ImGui::GetCursorPos();
				if (ImGui::Button("Add new key-value pair", ImVec2(-38.0f - start_indent, 0.0f)))
				{
					kvp_helper._in_use = true;
					kvp_helper.key_set_focus = true;
				}

				ImGui::SetCursorPos(ImVec2(pbutton_cursorpos.x + ImGui::GetItemRectSize().x + 6.0f, pbutton_cursorpos.y));

				// #

				int style_colors = 0;
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f)); style_colors++;
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.0f, 1.0f, 1.0f, 0.3f)); style_colors++;
				ggui::context_menu_style_begin();

				if (ImGui::BeginCombo("##kvp_combo", nullptr, ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft)) // The second parameter is the label previewed before opening the combo.
				{
					const auto edit_entity = game::g_edit_entity();

					int   template_size = 0;
					const template_kvp* templates = nullptr;

					if (edit_entity && edit_entity->eclass && edit_entity->epairs)
					{
						int vis_count = 0;
						entity_dialog::get_eclass_template(templates, &template_size);

						for (int n = 0; n < template_size; n++)
						{
							if (!has_key_value_pair(edit_entity, templates[n].key))
							{
								vis_count++;
								if (ImGui::Selectable(templates[n].key, false))
								{
									add_prop(templates[n].key, templates[n].val);
								}
							}
						}

						if (!vis_count)
						{
							ImGui::TextUnformatted("...");
						}
					}

					ImGui::EndCombo();
				}

				ggui::context_menu_style_end();
				imgui::PopStyleColor(style_colors);
			}
		}

		ImGui::PopStyleVar(2);
		SPACING(0.0f, 0.01f);
	}

	bool entity_dialog::gui()
	{
		if (dvars::gui_props_toolbox && dvars::gui_props_toolbox->current.enabled)
		{
			return false;
		}

		const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Entity##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}

		draw_classlist();

		if (ImGui::TreeNodeEx("Class Comments", dvars::gui_props_comments_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			SPACING(0.0f, 0.01f);
			draw_comments();

			ImGui::TreePop();
			separator_for_treenode();
		}

		SPACING(0.0f, 0.01f);

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
		if (ImGui::TreeNodeEx("Spawnflags", dvars::gui_props_spawnflags_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
		{
			draw_checkboxes();

			ImGui::TreePop();
			separator_for_treenode();
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_IndentSpacing
		SPACING(0.0f, 0.01f);
		

		if (ImGui::TreeNodeEx("Entity Properties", ImGuiTreeNodeFlags_DefaultOpen))
		{
			draw_entprops();
			ImGui::TreePop();

			if (dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.integer == 1)
			{
				separator_for_treenode();
			}
		}

		m_edit_entity_changed = false;

		if (dvars::gui_props_surfinspector && dvars::gui_props_surfinspector->current.integer == 1)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);

			if (ImGui::TreeNodeEx("Surface Inspector", dvars::gui_props_classlist_defaultopen->current.enabled ? ImGuiTreeNodeFlags_DefaultOpen : 0))
			{
				SPACING(0.0f, 0.01f);
				GET_GUI(ggui::surface_dialog)->inspector_controls();

				ImGui::TreePop();
			}

			ImGui::PopStyleVar();
		}

		ImGui::End();
		return true;
	}

	// ----------------------------------
	// ASM

	__declspec(naked) void entity_dialog::init_classlist_stub()
	{
		const static uint32_t retn_pt = 0x496816;
		__asm
		{
			pushad;
			call	entity_dialog::fill_classlist_intercept;
			popad;

			push	0x184;
			jmp		retn_pt;
		}
	}

	__declspec(naked) void entity_dialog::on_update_selection_stub()
	{
		const static uint32_t retn_pt = 0x497225;
		__asm
		{
			pushad;
			call	entity_dialog::on_update_selection_intercept;
			popad;

			push    0x186;		// overwritten op
			jmp		retn_pt;
		}
	}

	__declspec(naked) void entity_dialog::on_mapload_stub()
	{
		const static uint32_t retn_pt = 0x418814;
		__asm
		{
			pushad;
			call	entity_dialog::on_mapload_intercept;
			popad;

			push    0x1101;
			jmp		retn_pt;
		}
	}

	// CMainFrame::OnViewEntity
	void entity_dialog::on_viewentity_command()
	{
		if(dvars::gui_props_toolbox && dvars::gui_props_toolbox->current.enabled)
		{
			const auto tb = GET_GUI(ggui::toolbox_dialog);

			tb->set_bring_to_front(true);
			tb->focus_child(toolbox_dialog::TB_CHILD::ENTITY_PROPS);
			tb->open();
		}
		else
		{
			const auto gui = GET_GUI(ggui::entity_dialog);

			if (gui->is_inactive_tab() && gui->is_active())
			{
				gui->set_bring_to_front(true);
				return;
			}

			gui->toggle();
		}
	}

	void entity_dialog::register_dvars()
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

	void entity_dialog::hooks()
	{
		// init classlist on startup
		utils::hook(0x496811, entity_dialog::init_classlist_stub, HOOK_JUMP).install()->quick();

		// update edit_entity with radiant routines
		utils::hook(0x497220, entity_dialog::on_update_selection_stub, HOOK_JUMP).install()->quick();

		// reset classlist on map re/load
		utils::hook(0x41880F, entity_dialog::on_mapload_stub, HOOK_JUMP).install()->quick();

		// make entity-view hotkey open the imgui variant :: CMainFrame::OnFilterDlg
		utils::hook::detour(0x423F00, entity_dialog::on_viewentity_command, HK_JUMP);

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

	REGISTER_GUI(entity_dialog);
}
