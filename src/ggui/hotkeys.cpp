#include "std_include.hpp"
#include <iomanip>

namespace ggui
{
	void hotkey_helper_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(450, 160);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, MIN_WINDOW_SIZE);
		ImGui::SetNextWindowSize(MIN_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		ImGui::SetNextWindowSizeConstraints(ImVec2(450, 160), ImVec2(450, 160));

		if (!ImGui::Begin("Hotkeys Helper##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
						fs_homepath)
		{
			const char* apply_hint = utils::va("Could not find file 'iw3r_hotkeys.ini' in\n'%s'.", fs_homepath->current.string);
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(apply_hint).x) * 0.5f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetWindowHeight() * 0.5f - ImGui::CalcTextSize(apply_hint).y);
			ImGui::TextUnformatted(apply_hint);
		}

		ImGui::End();
	}

	REGISTER_GUI(hotkey_helper_dialog);
}

namespace ggui
{
	const char* radiant_keybind_array[] =
	{
		"##empty_label",
		"Space",
		"Backspace",
		"Escape",
		"End",
		"Insert",
		"Delete",
		"PageUp",
		"PageDown",
		"Up",
		"Down", // 10
		"Left",
		"Right",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8", // 20
		"F9",
		"F10",
		"F11",
		"F12",
		"Tab",
		"Return",
		"Comma",
		"Period",
		"Plus",
		"Multiply", // 30
		"Subtract",
		"NumPad0",
		"NumPad1",
		"NumPad2",
		"NumPad3",
		"NumPad4",
		"NumPad5",
		"NumPad6",
		"NumPad7",
		"NumPad8", // 40
		"NumPad9",
		"Minus",
		"[",
		"]",
		"\\",
		"~",
		"LWin", // 47
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
	};

	// get ascii fror keybind key
	int hotkey_dialog::cmdbinds_key_to_ascii(std::string key)
	{
		if (key.length() > 0)
		{
			if (key == "Space"s)		return 0x20;
			if (key == "Backspace"s)	return 0x8;
			if (key == "Escape"s)		return 0x1B;
			if (key == "End"s)			return 0x23;
			if (key == "Insert"s)		return 0x2D;
			if (key == "Delete"s)		return 0x2E;
			if (key == "PageUp"s)		return 0x21;
			if (key == "PageDown"s)		return 0x22;
			if (key == "Up"s)			return 0x26;
			if (key == "Down"s)			return 0x28;
			if (key == "Left"s)			return 0x25;
			if (key == "Right"s)		return 0x27;
			if (key == "F1"s)			return 0x70;
			if (key == "F2"s)			return 0x71;
			if (key == "F3"s)			return 0x72;
			if (key == "F4"s)			return 0x73;
			if (key == "F5"s)			return 0x74;
			if (key == "F6"s)			return 0x75;
			if (key == "F7"s)			return 0x76;
			if (key == "F8"s)			return 0x77;
			if (key == "F9"s)			return 0x78;
			if (key == "F10"s)			return 0x79;
			if (key == "F11"s)			return 0x7A;
			if (key == "F12"s)			return 0x7B;
			if (key == "Tab"s)			return 0x9;
			if (key == "Return"s)		return 0x0D;
			if (key == "Comma"s)		return 0x0BC;
			if (key == "Period"s)		return 0x0BE;
			if (key == "Plus"s)			return 0x6B;
			if (key == "Multiply"s)		return 0x6A;
			if (key == "Subtract"s)		return 0x6D;
			if (key == "NumPad0"s)		return 0x60;
			if (key == "NumPad1"s)		return 0x61;
			if (key == "NumPad2"s)		return 0x62;
			if (key == "NumPad3"s)		return 0x63;
			if (key == "NumPad4"s)		return 0x64;
			if (key == "NumPad5"s)		return 0x65;
			if (key == "NumPad6"s)		return 0x66;
			if (key == "NumPad7"s)		return 0x67;
			if (key == "NumPad8"s)		return 0x68;
			if (key == "NumPad9"s)		return 0x69;
			if (key == "Minus"s)		return 0x0BD;
			if (key == "["s)			return 0x0DB;
			if (key == "]"s)			return 0x0DD;
			if (key == "\\"s)			return 0x0DC;
			if (key == "~"s)			return 0x0C0;
			if (key == "LWin"s)			return 0x5B;

			//game::printf_to_console("[Hotkeys] Unkown key '%s'", key.c_str());
			//return 0;
		}

		return key[0];
	}

	// get ascii fror keybind key
	std::string hotkey_dialog::cmdbinds_ascii_to_keystr(int key)
	{
		if (key == 0x20)  return "Space"s;
		if (key == 0x8)   return "Backspace"s;
		if (key == 0x1B)  return "Escape"s;
		if (key == 0x23)  return "End"s;
		if (key == 0x2D)  return "Insert"s;
		if (key == 0x2E)  return "Delete"s;
		if (key == 0x21)  return "PageUp"s;
		if (key == 0x22)  return "PageDown"s;
		if (key == 0x26)  return "Up"s;
		if (key == 0x28)  return "Down"s;
		if (key == 0x25)  return "Left"s;
		if (key == 0x27)  return "Right"s;
		if (key == 0x70)  return "F1"s;
		if (key == 0x71)  return "F2"s;
		if (key == 0x72)  return "F3"s;
		if (key == 0x73)  return "F4"s;
		if (key == 0x74)  return "F5"s;
		if (key == 0x75)  return "F6"s;
		if (key == 0x76)  return "F7"s;
		if (key == 0x77)  return "F8"s;
		if (key == 0x78)  return "F9"s;
		if (key == 0x79)  return "F10"s;
		if (key == 0x7A)  return "F11"s;
		if (key == 0x7B)  return "F12"s;
		if (key == 0x9)   return "Tab"s;
		if (key == 0x0D)  return "Return"s;
		if (key == 0x0BC) return "Comma"s;
		if (key == 0x0BE) return "Period"s;
		if (key == 0x6B)  return "Plus"s;
		if (key == 0x6A)  return "Multiply"s;
		if (key == 0x6D)  return "Subtract"s;
		if (key == 0x60)  return "NumPad0"s;
		if (key == 0x61)  return "NumPad1"s;
		if (key == 0x62)  return "NumPad2"s;
		if (key == 0x63)  return "NumPad3"s;
		if (key == 0x64)  return "NumPad4"s;
		if (key == 0x65)  return "NumPad5"s;
		if (key == 0x66)  return "NumPad6"s;
		if (key == 0x67)  return "NumPad7"s;
		if (key == 0x68)  return "NumPad8"s;
		if (key == 0x69)  return "NumPad9"s;
		if (key == 0x0BD) return "Minus"s;
		if (key == 0x0DB) return "["s;
		if (key == 0x0DD) return "]"s;
		if (key == 0x0DC) return "\\"s;
		if (key == 0x0C0) return "~"s;
		if (key == 0x5B)  return "LWin"s;
		if (key == 0x11)  return "LCtrl"s;
		if (key == 0x12)  return "LAlt";

		std::string out; out += (char)key;
		return out;
	}

	std::string hotkey_dialog::get_hotkey_for_command(const char* command)
	{
		// find command in cmd_hotkeys (ini)
		for (const auto& bind : cmd_hotkeys)
		{
			if (!_strcmpi(command, bind.cmd_name.c_str()))
			{
				if (bind.modifier_key.empty())
				{
					return "";
				}

				return	"[" +
					(bind.modifier_shift == 1 ? "SHIFT-"s : "") +
					(bind.modifier_alt == 1 ? "ALT-"s : "") +
					(bind.modifier_ctrl == 1 ? "CTRL-"s : "") +
					bind.modifier_key + "]";
			}
		}

		// get the hotkey from the default command list if there is no ini or the command is missing
		// note: we overwrite g_Commands with our own hotkeys -> always check g_Commands?
		for (auto i = 0; i < game::g_nCommandCount; i++)
		{
			if (!_strcmpi(game::g_Commands[i].m_strCommand, command))
			{
				const unsigned int o_key = game::g_Commands[i].m_nKey;
				const unsigned int o_mod = game::g_Commands[i].m_nModifiers;

				std::string mod;
				mod += (o_mod & 1 ? "SHIFT-"s : "");
				mod += (o_mod & 2 ? "ALT-"s : "");
				mod += (o_mod & 4 ? "CTRL-"s : "");
				mod += cmdbinds_ascii_to_keystr(o_key);

				if (mod.empty())
				{
					return "";
				}

				return "[" + mod + "]";
			}
		}

		return "";
	}

	// populates std::vector<commandbinds> cmd_hotkeys
	bool hotkey_dialog::cmdbinds_load_from_file(std::string file)
	{
		cmd_hotkeys.clear();
		std::string home_path;

		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath"); 
						fs_homepath)
		{
			home_path = fs_homepath->current.string;
		}
		else
		{
			char buffer[512];
			if (!GetModuleFileNameA(nullptr, buffer, 512))
			{
				game::printf_to_console("[Hotkeys] could not get the base directory.\n");
				return false;
			}

			std::string path = buffer;
			home_path = path.substr(0, path.find_last_of("\\/"));
		}

		std::string ini_path = home_path;
					ini_path += "\\" + file;

		std::ifstream ini;
		ini.open(ini_path.c_str());

		if (!ini.is_open())
		{
			game::printf_to_console("[Hotkeys] Could not find \"iw3r_hotkeys.ini\" in \"%s\"\n", home_path.c_str());
			return false;
		}

		std::string input;
		std::vector<std::string> args;

		// read line by line
		while (std::getline(ini, input))
		{
			if (input.find("[Commands]") != std::string::npos)
			{
				//printf("[Hotkeys] Ignored '%s'\n", input.c_str());
				continue;
			}

			// ignore comments
			if (input.find(';') != std::string::npos)
			{
				game::printf_to_console("[Hotkeys] Ignored '%s'\n", input.c_str());
				continue;
			}

			// ignore lines not containing '='
			if (input.find(" =") == std::string::npos)
			{
				game::printf_to_console("[Hotkeys] Ignored '%s' => missing '='\n", input.c_str());
				continue;
			}

			// split the string on = (gets us 2 args)
			args = utils::split(input, '=');

			// remove the leftover space on the command name
			utils::rtrim(args[0]);

			// trim leading tabs and spaces on the key-bind
			utils::ltrim(args[1]);

			// split keys on space
			std::vector<std::string> keys;

			if (args[1].find(' ') != std::string::npos)
			{
				// multiple keys
				keys = utils::split(args[1], ' ');
			}
			else
			{
				// single key
				keys.push_back(args[1]);
			}

			cmd_hotkeys.push_back(
				commandbinds
				{
					args[0],
					args[1],
					input.find("+alt") != std::string::npos,
					input.find("+ctrl") != std::string::npos,
					input.find("+shift") != std::string::npos,
					keys[keys.size() - 1]
				});
		}

		return true;
	}

	// g_commandmap m_nModifiers
	// 1u = shift
	// 2u = alt
	// 4u = ctrl
	// 8u = lwin

	// overwrite hardcoded hotkeys with our own
	void hotkey_dialog::load_commandmap()
	{
		if (!cmdbinds_load_from_file("iw3r_hotkeys.ini"s))
		{
			// update 'std::map' unkown commandmap (uses g_Commands)
			cdeclcall(void, 0x420140); // the func that would normally be called

			return;
		}

		int commands_overwritten = 0;
		game::printf_to_console("[Hotkeys] Loading '%d' hotkeys from 'iw3r_hotkeys.ini'\n", cmd_hotkeys.size());

		for (auto i = 0; i < game::g_nCommandCount; i++)
		{
			for (commandbinds& hotkey : cmd_hotkeys)
			{
				if (!_strcmpi(game::g_Commands[i].m_strCommand, hotkey.cmd_name.c_str()))
				{
					const unsigned int o_key = game::g_Commands[i].m_nKey;
					const unsigned int o_mod = game::g_Commands[i].m_nModifiers;

					//printf("overwriting command '%s'\n", game::g_Commands[i].m_strCommand);
					//printf("|-> m_nKey '%d' to ", game::g_Commands[i].m_nKey);

					game::g_Commands[i].m_nKey = cmdbinds_key_to_ascii(hotkey.modifier_key);

					//printf("'%d'\n", game::g_Commands[i].m_nKey);
					//printf("|-> m_nModifiers '%d' to ", game::g_Commands[i].m_nModifiers);

					game::g_Commands[i].m_nModifiers = hotkey.modifier_shift
						| (hotkey.modifier_alt == 1 ? 2 : 0)
						| (hotkey.modifier_ctrl == 1 ? 4 : 0);

					//printf("'%d'\n\n", game::g_Commands[i].m_nModifiers);

					if (   o_key != game::g_Commands[i].m_nKey 
						|| o_mod != game::g_Commands[i].m_nModifiers)
					{
						game::printf_to_console("|-> modified hotkey '%s'\n", hotkey.cmd_name.c_str());
						commands_overwritten++;
					}

					break;
				}
			}
		}

		// exec cmainframe::on_keydown()
		for (auto& addon_bind : ggui::cmd_addon_hotkeys_builtin)
		{
			for (commandbinds& hotkey : cmd_hotkeys)
			{
				if (!_strcmpi(addon_bind.m_strCommand, hotkey.cmd_name.c_str()))
				{
					addon_bind.m_nKey = cmdbinds_key_to_ascii(hotkey.modifier_key);
					addon_bind.m_nModifiers = hotkey.modifier_shift
						| (hotkey.modifier_alt == 1 ? 2 : 0)
						| (hotkey.modifier_ctrl == 1 ? 4 : 0);
				}
			}
		}

		for (auto& addon_bind : ggui::cmd_addon_hotkeys)
		{
			for (commandbinds& hotkey : cmd_hotkeys)
			{
				if (addon_bind.m_strCommand == hotkey.cmd_name)
				{
					addon_bind.m_nKey = cmdbinds_key_to_ascii(hotkey.modifier_key);
					addon_bind.m_nModifiers = hotkey.modifier_shift
						| (hotkey.modifier_alt == 1 ? 2 : 0)
						| (hotkey.modifier_ctrl == 1 ? 4 : 0);
				}
			}
		}

		game::printf_to_console("|-> modified '%d' commands\n", commands_overwritten);
		game::printf_to_console("\n");

		// there is a second commandmap (vector/unsorted map) ... whatever
		// clear ^ and set command count to 0
		cdeclcall(void, 0x42C7A0);

		// update 'std::map' unkown commandmap (uses g_Commands)
		cdeclcall(void, 0x420140);
	}

	// load the default commandmap if we did not load our own (ini)
	void hotkey_dialog::load_default_commandmap()
	{
		if (cmd_hotkeys.empty())
		{
			cdeclcall(void, 0x421230); // CMainFrame::LoadCommandMap
		}
	}

	bool hotkey_dialog::cmdbinds_check_dupe(commandbinds& bind, std::string& o_dupebind)
	{
		for (commandbinds& binds : cmd_hotkeys)
		{
			// ignore "self"
			if (bind.cmd_name == binds.cmd_name) 
			{
				continue;
			}

			// ignore non-assigned
			if (!binds.modifier_shift && !binds.modifier_ctrl && !binds.modifier_alt
				&& binds.modifier_key.empty())
			{
				continue;
			}

			if (   bind.modifier_shift == binds.modifier_shift
				&& bind.modifier_ctrl == binds.modifier_ctrl
				&& bind.modifier_alt == binds.modifier_alt
				&& bind.modifier_key == binds.modifier_key)
			{
				o_dupebind = binds.cmd_name;
				return true;
			}
		}

		return false;
	}

	void hotkey_dialog::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(450.0f, 342.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(450.0f, 800.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(1.0f, 4.0f));
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		ImGui::Begin("Hotkeys##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse);

		const char* apply_hint = "Changes will apply upon closing the window.";
		ImGui::SetCursorPosX((ImGui::GetColumnWidth() - ImGui::CalcTextSize(apply_hint).x) * 0.5f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
		ImGui::TextUnformatted(apply_hint);

		if (ImGui::BeginTable("bind_table", 5,
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("                  Command", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 200.0f);
			ImGui::TableSetupColumn("Shift", ImGuiTableColumnFlags_WidthFixed, 34.0f);
			ImGui::TableSetupColumn("  Alt", ImGuiTableColumnFlags_WidthFixed, 34.0f);
			ImGui::TableSetupColumn(" Ctrl", ImGuiTableColumnFlags_WidthFixed, 34.0f);
			ImGui::TableSetupColumn(" Key", ImGuiTableColumnFlags_WidthStretch, 120.0f);
			ImGui::TableHeadersRow();

			int row = 0;

			ImGuiListClipper clipper;
			clipper.Begin(cmd_hotkeys.size());
			while (clipper.Step())
			{
				// for (commandbinds& bind : cmd_hotkeys)
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					commandbinds& bind = cmd_hotkeys[i];

					std::string str_dupe_bind = bind.cmd_name;
					const bool found_dupe = cmdbinds_check_dupe(bind, str_dupe_bind);

					// unique widget id's for each row
					ImGui::PushID(row); row++;
					ImGui::TableNextRow();

					for (int column = 0; column < 5; column++)
					{
						ImGui::PushID(column);
						ImGui::TableNextColumn();

						switch (column)
						{
						case 0:
							ImGui::SetCursorPosX((ImGui::GetColumnWidth() - ImGui::CalcTextSize(bind.cmd_name.c_str()).x) * 0.5f);
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

							if (found_dupe)
							{
								ImGui::TextColored(ImVec4(0.9f, 0.1f, 0.1f, 1.0f), bind.cmd_name.c_str());
								ImGui::SameLine();
								ImGui::HelpMarker(utils::va("bind conflicts with '%s'", str_dupe_bind.c_str()));
							}
							else
							{
								ImGui::TextUnformatted(bind.cmd_name.c_str());
							}

							break;
						case 1:
							ImGui::Checkbox("##2", (bool*)&bind.modifier_shift);
							break;
						case 2:
							ImGui::Checkbox("##0", (bool*)&bind.modifier_alt);
							break;
						case 3:
							ImGui::Checkbox("##1", (bool*)&bind.modifier_ctrl);
							break;
						case 4:
							ImGui::PushItemWidth(ImGui::GetColumnWidth() - 6.0f);

							if (ImGui::BeginCombo("##combokey", bind.modifier_key.c_str(), ImGuiComboFlags_NoArrowButton)) // The second parameter is the label previewed before opening the combo.
							{
								for (int n = 0; n < IM_ARRAYSIZE(radiant_keybind_array); n++)
								{
									const bool is_selected = !_stricmp(bind.modifier_key.c_str(), radiant_keybind_array[n]); // You can store your selection however you want, outside or inside your objects
									if (is_selected)
									{
										ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
									}

									if (ImGui::Selectable(radiant_keybind_array[n], is_selected))
									{
										bind.modifier_key = radiant_keybind_array[n];
										if (is_selected)
										{
											ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
										}
									}
								}
								ImGui::EndCombo();
							}

							ImGui::PopItemWidth();
							break;
						}

						// column
						ImGui::PopID();
					}

					// row
					ImGui::PopID();
				}
			} clipper.End();

			ImGui::EndTable();
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_CellPadding
		ImGui::End();
	}

	void hotkey_dialog::on_open()
	{
		if (!hotkey_dialog::cmdbinds_load_from_file("iw3r_hotkeys.ini"s))
		{
			GET_GUI(hotkey_helper_dialog)->open();
			this->close();
		}
	}

	void hotkey_dialog::on_close()
	{
		if (cmd_hotkeys.empty())
		{
			return;
		}

		if (const auto& fs_homepath = game::Dvar_FindVar("fs_homepath");
						fs_homepath)
		{
			std::ofstream ini;
			std::string ini_path = fs_homepath->current.string;
						ini_path += "\\iw3r_hotkeys.ini";

			ini.open(ini_path.c_str());

			if (!ini.is_open())
			{
				printf("[Hotkeys] Could not write to \"iw3r_hotkeys.ini\" in \"%s\"", fs_homepath->current.string);
				return;
			}

			ini << "[Commands]" << std::endl;

			for (commandbinds& bind : cmd_hotkeys)
			{
				ini << std::left << std::setw(26) << bind.cmd_name << " = ";

				ini << (bind.modifier_shift == 0 ? "" : "+shift ");
				ini << (bind.modifier_alt == 0 ? "" : "+alt ");
				ini << (bind.modifier_ctrl == 0 ? "" : "+ctrl ");
				ini <<  bind.modifier_key << std::endl;
			}

			hotkey_dialog::load_commandmap();
		}
	}

	void hotkey_dialog::hooks()
	{
		// replace hardcoded hotkeys with our own (ini)
		utils::hook(0x420A4F, hotkey_dialog::load_commandmap, HOOK_CALL).install()->quick();

		// load/skip the original commandmap (depends if iw3r_hotkeys.ini exists or not)
		utils::hook(0x4210BF, hotkey_dialog::load_default_commandmap, HOOK_CALL).install()->quick();
	}

	REGISTER_GUI(hotkey_dialog);
}