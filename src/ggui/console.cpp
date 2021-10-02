#include "std_include.hpp"

const int	MAX_MATCHES_TO_SHOW = 12;

namespace ggui
{
	console* _console = nullptr;

	console::console()
	{
		if (!ggui::_console)
		{
			clear_log();
			memset(m_input_buf, 0, sizeof(m_input_buf));
			m_input_reclaim_focus = false;
			m_should_search_candidates = false;
			m_history_pos = -1;
			m_autocomplete_pos = -1;
			m_old_input_but_len = 0;
			m_auto_scroll = true;
			m_scroll_to_bottom = false;
			m_post_inputbox_cursor = ImVec2(0.0f, 0.0f);

			ggui::_console = this;
		}
	}
	
	console::~console()
	{
		clear_log();
		
		for (int i = 0; i < m_history.Size; i++)
		{
			free(m_history[i]);
		}
	}

	// Portable helpers
	static int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
	static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
	static char* Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
	static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

	void console::clear_log()
	{
		for (int i = 0; i < m_items.Size; i++)
		{
			free(m_items[i]);
		}
			
		m_items.clear();
	}

	void console::addline_no_format(const char* text)
	{
		m_items.push_back(Strdup(text));
	}
	
	void console::addline(const char* fmt, ...) IM_FMTARGS(2)
	{
		char buf[1024];
		va_list args;
		
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		
		m_items.push_back(Strdup(buf));
	}

	void console::addtext_with_color(const char* text)
	{
		ImVec4 color;
		bool has_color = false;

		std::string item_s = text;

		if (   utils::starts_with(item_s, "^1")
			|| utils::starts_with(item_s, "ERROR:")
			|| utils::starts_with(item_s, "Error:"))
		{
			color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
			has_color = true;
		}
		else if (utils::starts_with(item_s, "^3")
			  || utils::starts_with(item_s, "WARNING:")
			  || utils::starts_with(item_s, "Warning:"))
		{
			color = ImVec4(1.0f, 0.65f, 0.1f, 1.0f);
			has_color = true;
		}
		else if (utils::starts_with(item_s, "["))
		{
			color = ImVec4(0.75f, 0.95f, 0.825f, 1.0f);
			has_color = true;
		}

		if (has_color) { ImGui::PushStyleColor(ImGuiCol_Text, color); }
		ImGui::TextUnformatted(text);
		if (has_color) { ImGui::PopStyleColor(); }
	}

	void console::draw(const char* title, bool* p_open)
	{
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		bool copy_to_clipboard = false;

		// Reserve enough left-over height for 1 separator + 1 input text
		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Copy All")) {
				copy_to_clipboard = true;
			}
			
			if (ImGui::Selectable("Clear")) {
				clear_log();
			}

			if (ImGui::Selectable("Auto-Scroll", m_auto_scroll)) {
				m_auto_scroll = m_auto_scroll ? false : true;
			}
			
			ImGui::EndPopup();
		}


		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); // Tighten spacing

		if (copy_to_clipboard) {
			ImGui::LogToClipboard();
		}

		if (m_filter.IsActive())
		{
			for (int i = 0; i < m_items.Size; i++)
			{
				const char* item = m_items[i];

				if (!m_filter.PassFilter(item)) {
					continue;
				}

				addtext_with_color(item);
			}
		}
		else
		{
			ImGuiListClipper clipper;
			clipper.Begin(m_items.Size);
			
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
					addtext_with_color(m_items[i]);
				}
			}
			clipper.End();
		}

		ImGui::PopStyleVar();
		
		if (copy_to_clipboard)  {
			ImGui::LogFinish();
		}
			
		if (m_scroll_to_bottom || (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))  {
			ImGui::SetScrollHereY(1.0f);
		}
			
		m_scroll_to_bottom = false;

		
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		const ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways;
		if (ImGui::InputText("##console_input", m_input_buf, IM_ARRAYSIZE(m_input_buf), input_text_flags, &console::text_edit_callback_stub, (void*)this))
		{
			char* s = m_input_buf;
			Strtrim(s);
			
			if (s[0]) 
			{
				exec_command(s);
			}

			m_autocomplete_candidates.clear();
			m_autocomplete_pos = -1;
				
			strcpy(s, "");
			m_input_reclaim_focus = true;
		}
		m_post_inputbox_cursor = ImGui::GetCursorScreenPos();

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();

		if (m_input_reclaim_focus) 
		{
			m_input_reclaim_focus = false;
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}

		ImGui::SameLine(0, 20.0f);
		ImGui::TextUnformatted("Filter");
		ImGui::SameLine();
		m_filter.Draw("#console_filter", ImGui::GetContentRegionAvailWidth());
		
		if (ImGui::IsWindowFocused() && m_autocomplete_candidates.Size != 0 && ggui::_console->m_old_input_but_len > 0)
		{
			ImGui::SetNextWindowPos(m_post_inputbox_cursor);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.85f));
			ImGui::Begin("##console_autocomplete", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize);

			if (m_matched_dvar)
			{
				ImGui::TextUnformatted(m_matched_dvar->name);
				ImGui::SameLine(0, 10.0f);
				
				std::string dvar_value;
				switch (m_matched_dvar->type)
				{
				case game::dvar_type::boolean:
					dvar_value = "[BOOL]";
					break;

				case game::dvar_type::value:
					dvar_value = "[FLOAT]";
					break;

				case game::dvar_type::vec2:
					dvar_value = "[VEC2]";
					break;

				case game::dvar_type::vec3:
					dvar_value = "[VEC3]";
					break;

				case game::dvar_type::vec4:
					dvar_value = "[VEC4]";
					break;

				case game::dvar_type::integer:
					dvar_value = "[INT]";
					break;

				case game::dvar_type::enumeration:
					dvar_value = "[ENUM]";
					break;

				case game::dvar_type::string:
					dvar_value = " [STRING]";
					break;

				case game::dvar_type::color:
					dvar_value = " [COLOR]";
					break;

				case game::dvar_type::rgb:
					dvar_value = "[RGB]";
					break;

				default:
					dvar_value = "[UNKOWN]";
					break;
				}

				const char* value_to_str = game::Dvar_DisplayableValue(m_matched_dvar);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.5f, 0.2f, 1.0f));
				ImGui::TextUnformatted(value_to_str);
				ImGui::PopStyleColor();

				ImGui::SameLine(0, 10.0f);
				ImGui::TextUnformatted(dvar_value.c_str());

				char buf[1028];
				const char* domain_str = game::Dvar_DomainToString_Internal(1024, buf, static_cast<int>(m_matched_dvar->type), 0, m_matched_dvar->domain.value.min, m_matched_dvar->domain.value.max);

				if(domain_str)
				{
					ImGui::TextUnformatted(domain_str);
				}

				if(m_matched_dvar->description)
				{
					ImGui::TextUnformatted(m_matched_dvar->description);
				}
			}
			else
			{
				if (m_autocomplete_candidates.Size <= MAX_MATCHES_TO_SHOW)
				{
					for (auto i = 0; i < m_autocomplete_candidates.Size; i++)
					{
						bool is_highlighted = false;
						if (i == m_autocomplete_pos)
						{
							is_highlighted = true;
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.1f, 1.0f));
						}

						ImGui::TextUnformatted(m_autocomplete_candidates[i]);

						if (is_highlighted) {
							ImGui::PopStyleColor();
						}
					}
				}
				else
				{
					ImGui::Text("Too many matches to draw. [%d] Matches.", m_autocomplete_candidates.Size);
				}
			}

			ImGui::PopStyleColor();
			ImGui::End(); // autocomplete
		}
		
		ImGui::End(); // console
	}

	void console::exec_command(const char* command_line)
	{
		addline("# %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back.
		// This isn't trying to be smart or optimal.
		m_history_pos = -1;
		for (int i = m_history.Size - 1; i >= 0; i--)
		{
			if (Stricmp(m_history[i], command_line) == 0)
			{
				free(m_history[i]);
				m_history.erase(m_history.begin() + i);
				break;
			}
		}
			
		m_history.push_back(Strdup(command_line));

		// Process command
		//game::dvar_s* dvar = nullptr;

		bool was_dvar = false;
		std::string command_s = command_line;
		const auto	space_pos = command_s.find(' ');

		if (space_pos != std::string::npos)
		{
			const std::string dvar_str = command_s.substr(0, space_pos);
			
			if (const auto dvar = game::Dvar_FindVar(dvar_str.c_str());
						   dvar)
			{
				const auto value_string = command_s.substr(space_pos);
				game::Dvar_SetFromStringFromSource(value_string.c_str(), dvar, 0);
				was_dvar = true;
			}
		}

		if (!was_dvar && !command_s.empty())
		{
			std::vector<std::string> args;

			if (command_s.find(' ') != std::string::npos)
			{
				args = utils::split(command_s, ' ');
			}
			else
			{
				args.push_back(command_s);
			}

			components::command::execute_command(args);
		}

		// On command input, we scroll to bottom even if m_auto_scroll==false
		m_scroll_to_bottom = true;
	}

	// In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
	int console::text_edit_callback_stub(ImGuiInputTextCallbackData* data)
	{
		if(data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
		{
			// on autocomplete tab -> m_should_search_candidates
			if(ggui::_console->m_should_search_candidates)
			{
				data->EventFlag = ImGuiInputTextFlags_CallbackEdit;
				return ggui::_console->text_edit_callback(data);
			}
		}
		
		if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
		{
			if(ggui::_console->m_autocomplete_candidates.Size == 1)
			{
				ggui::_console->m_autocomplete_pos = 0;
			}

			if(ggui::_console->m_autocomplete_pos >= 0)
			{
				const char* autocomplete_str = (ggui::_console->m_autocomplete_pos >= 0) ? ggui::_console->m_autocomplete_candidates[ggui::_console->m_autocomplete_pos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, autocomplete_str);
				data->InsertChars(data->CursorPos, " ");

				ggui::_console->m_should_search_candidates = true;
			}
		}

		if (data->EventKey == ImGuiKey_UpArrow || data->EventKey == ImGuiKey_DownArrow)
		{
			if(ggui::_console->m_autocomplete_candidates.Size > 0 && data->CursorPos > 0)
			{
				const int prev_history_pos = ggui::_console->m_autocomplete_pos;

				if (data->EventKey == ImGuiKey_UpArrow)
				{
					if (ggui::_console->m_autocomplete_pos - 1 >= -1)
					{
						ggui::_console->m_autocomplete_pos--;
					}
				}

				if (data->EventKey == ImGuiKey_DownArrow)
				{
					if (ggui::_console->m_autocomplete_pos + 1 < ggui::_console->m_autocomplete_candidates.Size)
					{
						ggui::_console->m_autocomplete_pos++;
					}
				}
			}
			else
			{
				return ggui::_console->text_edit_callback(data);
			}
		}

		if(ggui::_console->m_old_input_but_len != data->BufTextLen)
		{
			// reset autocomplete on normal input
			ggui::_console->m_autocomplete_pos = -1;
			
			ggui::_console->m_old_input_but_len = data->BufTextLen;
			return ggui::_console->text_edit_callback(data);
		}

		return 0;
	}

	int console::text_edit_callback(ImGuiInputTextCallbackData* data)
	{
		//addline("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackEdit:
			{
				std::string input = data->Buf;
				const auto	space_pos = input.find(' ');
				
				if(m_should_search_candidates)
				{
					input = input.substr(0, space_pos);
				}

				if(!m_should_search_candidates)
				{
					if (space_pos != std::string::npos)
					{
						if (m_autocomplete_candidates.Size == 1)
						{
							if (const auto dvar = game::Dvar_FindVar(m_autocomplete_candidates[0]);
										   dvar)
							{
								m_matched_dvar = dvar;
							}
						}

						break;
					}
				}

				m_autocomplete_candidates.clear();
				m_matched_dvar = nullptr;
	
				for (auto dvarIter = 0; dvarIter < *game::dvarCount; ++dvarIter)
				{
					// get the dvar from the "sorted" dvar* list
					const auto dvar = reinterpret_cast<game::dvar_s*>(game::sortedDvars[dvarIter]);
					if (!dvar)
					{
						game::printf_to_console("Invalid dvar while trying to autocomplete ...\n");
						break;
					}

					if(Strnicmp(dvar->name, input.c_str(), data->CursorPos) == 0)
					{
						m_autocomplete_candidates.push_back(dvar->name);
					}
				}

				if(m_autocomplete_candidates.Size == 1 && m_should_search_candidates)
				{
					if (const auto dvar = game::Dvar_FindVar(m_autocomplete_candidates[0]);
						dvar)
					{
						m_matched_dvar = dvar;
					}
				}
	
				m_should_search_candidates = false;
				
				break;
			}
			
		case ImGuiInputTextFlags_CallbackHistory:
			{
				// Example of HISTORY
				const int prev_history_pos = m_history_pos;
				if (data->EventKey == ImGuiKey_UpArrow)
				{
					if (m_history_pos == -1) {
						m_history_pos = m_history.Size - 1;
					}
					else if (m_history_pos > 0) {
						m_history_pos--;
					}
						
				}
				else if (data->EventKey == ImGuiKey_DownArrow)
				{
					if (m_history_pos != -1)
					{
						if (++m_history_pos >= m_history.Size) {
							m_history_pos = -1;
						}
					}	
				}

				// A better implementation would preserve the data on the current input line along with cursor position.
				if (prev_history_pos != m_history_pos)
				{
					const char* history_str = (m_history_pos >= 0) ? m_history[m_history_pos] : "";
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str);

					std::string input = history_str;
					if (input.find(' ') != std::string::npos)
					{
						m_should_search_candidates = true;
					}
				}
			}
		}
		return 0;
	}

	void console::menu(ggui::imgui_context_menu & menu)
	{
		if (ggui::_console)
		{
			const auto MIN_WINDOW_SIZE = ImVec2(400.0f, 200.0f);
			const auto INITIAL_WINDOW_SIZE = ImVec2(520, 600);

			ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
			ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

			ggui::_console->draw("Console##window", &menu.menustate);
		}
	}
};
