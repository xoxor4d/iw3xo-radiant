#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

using namespace codol;

namespace ggui
{
	namespace
	{
		std::string dvar_str(const game::dvar_s* dvar)
		{
			return (dvar && dvar->current.string) ? dvar->current.string : "";
		}

		std::string lower(std::string s)
		{
			return script::to_lower(s);
		}

		bool read_binary(const std::string& path, std::string& out)
		{
			std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
			if (!file.is_open())
			{
				return false;
			}

			out.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
			return true;
		}

		bool write_binary(const std::string& path, const std::string& data)
		{
			std::error_code ec;
			std::filesystem::create_directories(std::filesystem::u8path(path).parent_path(), ec);

			std::ofstream file(std::filesystem::u8path(path), std::ios::binary | std::ios::trunc);
			if (!file.is_open())
			{
				return false;
			}

			file.write(data.data(), static_cast<std::streamsize>(data.size()));
			return file.good();
		}

		std::string file_stem(const std::string& path)
		{
			auto name = path;
			if (const auto slash = name.find_last_of("\\/"); slash != std::string::npos)
			{
				name.erase(0, slash + 1);
			}

			// mp_x.d3dbsp.ents -> mp_x
			if (const auto dot = name.find('.'); dot != std::string::npos)
			{
				name.erase(dot);
			}

			return name;
		}

		// files that sit next to an imported map: what it was imported from, and how radiant saves it untouched
		std::string sidecar_original(const std::string& map_path)
		{
			return map_path.substr(0, map_path.size() - 4) + ".codol_orig.ents";
		}

		std::string sidecar_baseline(const std::string& map_path)
		{
			return map_path.substr(0, map_path.size() - 4) + ".codol_base.map";
		}

		bool same_path(std::string a, std::string b)
		{
			std::replace(a.begin(), a.end(), '/', '\\');
			std::replace(b.begin(), b.end(), '/', '\\');
			return lower(a) == lower(b);
		}

		// same steps the "open map" file dialog does
		void load_map_into_radiant(const std::string& path)
		{
			// checks for unsaved changes / being inside a prefab
			if (!utils::hook::call<bool(__cdecl)()>(0x489D90)() || utils::hook::call<bool(__cdecl)()>(0x49A030)())
			{
				utils::hook::call<void(__cdecl)()>(0x489D50)(); // leave prefabs
				utils::hook::call<void(__cdecl)()>(0x410600)(); // Pointfile_Clear
				game::map_load_from_file(path.c_str());

				ImGui::GetIO().AddMouseButtonEvent(0, false);
			}
		}
	}

	void codol_map_ents_dialog::log(const std::string& line, const bool error)
	{
		m_log.emplace_back(error, line);
		game::printf_to_console("%s[CoDOL] %s\n", error ? "^1" : "", line.c_str());

		if (m_log.size() > 200)
		{
			m_log.erase(m_log.begin(), m_log.begin() + 50);
		}
	}

	void codol_map_ents_dialog::on_open()
	{
		refresh_sources();
	}

	void codol_map_ents_dialog::refresh_sources()
	{
		m_sources.clear();
		m_selected = -1;

		std::error_code ec;

		// overrides that already exist in the script tree: this is what the server runs right now
		for (const auto& root : components::codol_game::script_roots())
		{
			const auto dir = std::filesystem::u8path(root) / "maps" / "mp";

			for (auto it = std::filesystem::directory_iterator(dir, ec); !ec && it != std::filesystem::directory_iterator(); it.increment(ec))
			{
				const auto u8 = it->path().u8string();
				const std::string path(u8.begin(), u8.end());

				if (lower(path).ends_with(".d3dbsp.ents"))
				{
					const auto map = file_stem(path);
					m_sources.push_back({ source_kind::override_file, map, path, map + "   ( current override in the script folder )" });
				}
			}

			ec.clear();
		}

		// map zones of the install
		if (const auto install = components::codol_game::install_path(); !install.empty())
		{
			for (const char* sub : { "\\client_shipRetail\\chinese", "\\usermaps" })
			{
				const auto dir = std::filesystem::u8path(install + sub);

				for (auto it = std::filesystem::recursive_directory_iterator(dir, ec); !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
				{
					const auto u8 = it->path().u8string();
					const std::string path(u8.begin(), u8.end());
					const auto name = lower(file_stem(path));

					if (!lower(path).ends_with(".ff") || name.ends_with("_load"))
					{
						continue;
					}

					if (name.starts_with("mp_") || name.starts_with("coop_") || name.starts_with("zm_"))
					{
						m_sources.push_back({ source_kind::zone, file_stem(path), path, file_stem(path) + "   ( zone )" });
					}
				}

				ec.clear();
			}
		}

		std::stable_sort(m_sources.begin(), m_sources.end(), [](const source& a, const source& b)
		{
			return lower(a.map) < lower(b.map);
		});
	}

	void codol_map_ents_dialog::open_imported_map(const std::string& map_name, const std::string& out_path, const std::string& entity_text, const std::string& summary)
	{
		log(summary);

		// what the map was imported from: the export restores untouched values from it
		write_binary(sidecar_original(out_path), entity_text);

		std::error_code ec;
		std::filesystem::remove(std::filesystem::u8path(sidecar_baseline(out_path)), ec);

		load_map_into_radiant(out_path);

		// baseline: the untouched map exactly as radiant writes it ( it reformats origins, forces the model of
		// classes it knows .. ). Whatever differs from it at export time is an edit.
		if (game::current_map_filepath && same_path(game::current_map_filepath, out_path))
		{
			game::map_save_file(out_path.c_str(), 0, 0);
			std::filesystem::copy_file(std::filesystem::u8path(out_path), std::filesystem::u8path(sidecar_baseline(out_path)),
				std::filesystem::copy_options::overwrite_existing, ec);

			if (ec)
			{
				log("could not write the baseline of " + out_path + ": the export will take every value as radiant saves it", true);
			}
		}
		else
		{
			log("the map was not opened ( unsaved changes? ). Open " + out_path + " and import again to edit it.", true);
			return;
		}

		ImGui::Toast(ImGuiToastType_Success, "CoDOL", ("Imported " + map_name).c_str());

		// show the same map in the native viewport
		GET_GUI(ggui::codol_viewport_dialog)->load_map(map_name);
	}

	void codol_map_ents_dialog::finish_import(const std::string& map_name, const std::string& entity_text, const std::string& origin, const int trigger_models)
	{
		// entities only ( an override file, a decrypted zone or plain text: there is no geometry to read )
		mapents::entity_set set;
		std::string error;

		if (!mapents::parse_entity_string(entity_text, set, error))
		{
			log("import of " + origin + " failed: " + error, true);
			return;
		}

		set.trigger_models = trigger_models;

		if (!set.has_crc)
		{
			log("import of " + origin + " failed: the file has no \"// base_crc32\" line, so the zone it belongs to is unknown", true);
			return;
		}

		const std::string out_path = utils::fs::get_basepath() + "\\map_source\\codol\\" + map_name + ".map";

		if (!write_binary(out_path, mapents::to_radiant_map(set, map_name)))
		{
			log("could not write " + out_path, true);
			return;
		}

		char crc[16];
		std::snprintf(crc, sizeof(crc), "%08x", set.base_crc);

		open_imported_map(map_name, out_path, entity_text, map_name + ": " + std::to_string(set.ents.size())
			+ " entities ( no geometry: import the map's .ff for brushes and static models ), base_crc32 " + crc + " -> " + out_path);
	}

	void codol_map_ents_dialog::import_geometry(const std::string& map_name, const std::string& fastfile, const std::string&, int)
	{
		// the whole map, read straight from the fastfile by the native zone reader ( in its own process: a loaded
		// zone is a few hundred MB )
		const std::string base = utils::fs::get_basepath();
		const std::string converter = base + "\\bin\\codol_mapconv.exe";
		const std::string out_path = base + "\\map_source\\codol\\" + map_name + ".map";
		const std::string ents_path = sidecar_original(out_path);

		std::error_code ec;
		if (!std::filesystem::exists(std::filesystem::u8path(converter), ec))
		{
			log(converter + " is missing ( it is built together with iw3r.dll )", true);
			return;
		}

		// the key of the fastfiles is read from the client executable
		std::string exe;
		for (const char* name : { "\\codoMP_client_shipRetail.exe" })
		{
			const auto candidate = components::codol_game::install_path() + name;
			if (std::filesystem::exists(std::filesystem::u8path(candidate), ec))
			{
				exe = candidate;
				break;
			}
		}

		if (exe.empty())
		{
			log("the client executable ( codoMP_client_shipRetail.exe ) was not found: set the CoDOL install folder in Script Workbench > Setup", true);
			return;
		}

		m_busy = true;
		log("reading " + fastfile + " ...");

		std::thread([this, converter, fastfile, exe, out_path, ents_path, map_name]
		{
			std::string summary;
			const int code = components::codol_game::run_command("\"" + converter + "\" --map " + map_name + " --ff \"" + fastfile + "\" --exe \"" + exe
				+ "\" --out \"" + out_path + "\" --ents-out \"" + ents_path + "\"", "", summary);

			std::string entity_text;
			const bool ok = code == 0 && read_binary(ents_path, entity_text);

			components::codol_game::post_to_gui([this, ok, code, summary, map_name, out_path, entity_text]
			{
				m_busy = false;

				std::string line = summary;
				while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
				{
					line.pop_back();
				}

				if (!ok)
				{
					log("could not read the map ( exit code " + std::to_string(code) + " ): " + line, true);
					return;
				}

				open_imported_map(map_name, out_path, entity_text, line);
			});

		}).detach();
	}

	void codol_map_ents_dialog::import_file(const std::string& path, const std::string& map_name)
	{
		const auto ext = lower(path);

		if (ext.ends_with(".ff"))
		{
			import_geometry(map_name, path, "", -1);
			return;
		}

		std::string data;
		if (!read_binary(path, data))
		{
			log("could not read " + path, true);
			return;
		}

		if (ext.ends_with(".zone"))
		{
			std::string text;
			int trigger_models = -1;
			if (!mapents::extract_from_zone(data, text, &trigger_models))
			{
				log(path + " does not contain a map entity string", true);
				return;
			}

			finish_import(map_name, text, path, trigger_models);
			return;
		}

		// .d3dbsp.ents / plain entity text
		data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
		finish_import(map_name, data, path);
	}

	void codol_map_ents_dialog::import_source(const source& src)
	{
		import_file(src.path, src.map);
	}

	void codol_map_ents_dialog::export_current_map()
	{
		const std::string map_path = game::current_map_filepath ? game::current_map_filepath : "";

		if (map_path.empty() || lower(map_path).ends_with("unnamed.map"))
		{
			log("export: the current map has no file yet. Import the entities of a CoDOL map first.", true);
			return;
		}

		// write pending edits to disk, the export reads the .map file
		game::map_save_file(map_path.c_str(), 0, 0);

		std::string map_text;
		if (!read_binary(map_path, map_text))
		{
			log("export: could not read " + map_path, true);
			return;
		}

		map_text.erase(std::remove(map_text.begin(), map_text.end(), '\r'), map_text.end());

		mapents::export_result result;
		std::string error;

		std::string baseline, original;
		const bool has_baseline = lower(map_path).ends_with(".map")
			&& read_binary(sidecar_baseline(map_path), baseline) && read_binary(sidecar_original(map_path), original);

		if (has_baseline)
		{
			baseline.erase(std::remove(baseline.begin(), baseline.end(), '\r'), baseline.end());
			original.erase(std::remove(original.begin(), original.end(), '\r'), original.end());

			if (!mapents::merge_radiant_map(map_text, baseline, original, result, error))
			{
				log("export failed: " + error, true);
				return;
			}
		}
		else
		{
			// no import data next to the map ( moved / renamed map ): every value is taken as radiant saved it
			if (!mapents::from_radiant_map(map_text, result, error))
			{
				log("export failed: " + error, true);
				return;
			}

			log("  note: " + sidecar_baseline(map_path) + " is missing, so untouched entities are exported the way radiant "
				"rewrites them ( origins rounded to 0.1, models of known classes replaced ). Import the map again to avoid that.", true);
		}

		const auto roots = components::codol_game::script_roots();
		if (roots.empty())
		{
			log("export failed: no script folder is set ( Script Workbench > Setup )", true);
			return;
		}

		const std::string out_path = roots.front() + "\\maps\\mp\\" + result.map_name + ".d3dbsp.ents";

		// keep the file that was there before the first export
		std::error_code ec;
		if (std::filesystem::exists(std::filesystem::u8path(out_path), ec) && !std::filesystem::exists(std::filesystem::u8path(out_path + ".bak"), ec))
		{
			std::filesystem::copy_file(std::filesystem::u8path(out_path), std::filesystem::u8path(out_path + ".bak"), ec);
		}

		if (!write_binary(out_path, result.text))
		{
			log("export failed: could not write " + out_path, true);
			return;
		}

		for (const auto& warning : result.warnings)
		{
			log("  " + warning, true);
		}

		log(result.map_name + ": exported " + std::to_string(result.exported) + " entities -> " + out_path
			+ ( result.warnings.empty() ? "" : "  ( " + std::to_string(result.warnings.size()) + " skipped, see above )" ));

		ImGui::Toast(ImGuiToastType_Success, "CoDOL", ("Exported the entities of " + result.map_name).c_str());

		if (m_reload_after_export)
		{
			components::codol_game::load_map(result.map_name);
		}
	}

	bool codol_map_ents_dialog::gui()
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(560.0f, 420.0f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(ImVec2(820.0f, 640.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("CoDOL Map Entities##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}

		ImGui::PushTextWrapPos(0.0f);
		ImGui::TextDisabled(
			"Edit the entities of a shipped CoDOL map ( spawns, objectives, script_models, nodes, triggers .. ) without recompiling it: "
			"import them as a radiant map, move / add / remove entities, then export. The server uses the exported "
			"<map>.d3dbsp.ents in place of the entities of the map zone ( mp matches ).");
		ImGui::PopTextWrapPos();

		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		// ------------------------------------------------------------------ import
		ImGui::TextColored(ImVec4(0.31f, 0.76f, 1.0f, 1.0f), "1. Import");

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.0f);
		ImGui::InputTextWithHint("##src_filter", "Filter maps", &m_filter);
		ImGui::SameLine();

		if (ImGui::Button("Refresh", ImVec2(100.0f, 0.0f)))
		{
			refresh_sources();
		}

		ImGui::BeginChild("##src_list", ImVec2(0.0f, std::max(120.0f, ImGui::GetContentRegionAvail().y * 0.34f)), true);

		if (m_sources.empty())
		{
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled("No maps found. Set the CoDOL install folder and the script folder in Script Workbench > Setup, "
				"or import a file below.");
			ImGui::PopTextWrapPos();
		}

		for (int i = 0; i < static_cast<int>(m_sources.size()); i++)
		{
			const auto& src = m_sources[i];

			if (!m_filter.empty() && lower(src.label).find(lower(m_filter)) == std::string::npos)
			{
				continue;
			}

			if (ImGui::Selectable((src.label + "##" + std::to_string(i)).c_str(), m_selected == i, ImGuiSelectableFlags_AllowDoubleClick))
			{
				m_selected = i;

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !m_busy)
				{
					import_source(src);
				}
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", src.path.c_str());
			}
		}

		ImGui::EndChild();

		ImGui::BeginDisabled(m_busy || m_selected < 0 || m_selected >= static_cast<int>(m_sources.size()));
		if (ImGui::Button(m_busy ? "Working ..." : "Import selected map", ImVec2(200.0f, 0.0f)))
		{
			import_source(m_sources[m_selected]);
		}
		ImGui::EndDisabled();

		ImGui::SameLine(0.0f, 16.0f);
		ImGui::TextDisabled("or a file:");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(std::max(120.0f, ImGui::GetContentRegionAvail().x - 250.0f));
		ImGui::InputTextWithHint("##manual", ".d3dbsp.ents / .zone / .ff / entity text", &m_manual_path);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(130.0f);
		ImGui::InputTextWithHint("##manual_map", "map name", &m_manual_map);
		ImGui::SameLine();

		ImGui::BeginDisabled(m_busy || m_manual_path.empty());
		if (ImGui::Button("Import file"))
		{
			import_file(m_manual_path, m_manual_map.empty() ? file_stem(m_manual_path) : m_manual_map);
		}
		ImGui::EndDisabled();

		ImGui::Dummy(ImVec2(0.0f, 6.0f));

		// ------------------------------------------------------------------ export
		ImGui::TextColored(ImVec4(0.31f, 0.76f, 1.0f, 1.0f), "2. Export");

		const std::string current = game::current_map_filepath ? game::current_map_filepath : "";
		ImGui::TextDisabled("current map: %s", current.empty() ? "-" : current.c_str());

		if (ImGui::Button("Save + export override", ImVec2(200.0f, 0.0f)))
		{
			export_current_map();
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Saves the map and writes <script folder>\\maps\\mp\\<map>.d3dbsp.ents");
		}

		ImGui::SameLine();
		ImGui::Checkbox("then load the map on the local server", &m_reload_after_export);

		ImGui::Dummy(ImVec2(0.0f, 6.0f));

		// ------------------------------------------------------------------ notes + log
		if (ImGui::CollapsingHeader("What can be edited"))
		{
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled(
				"- Point entities ( spawn points, script_origin / script_struct, script_model, nodes, lights' entity keys .. ) "
				"can be moved, changed, added and deleted freely.\n"
				"- Entities shown as \"codol_bmodel\" use a brush model or trigger hull that is compiled into the map zone "
				"( \"model\" \"*N\" / \"?N\" ). Their real classname is kept in \"codol_classname\". They can be moved "
				"( the hull moves with the origin ), re-keyed or deleted, and a hull can be reused by copying the entity - "
				"but new hulls can not be made.\n"
				"- Brushes drawn in radiant are reference geometry only: there is no CoDOL bsp compiler, so world geometry, "
				"static models ( misc_model ) and lighting of a shipped map can not be changed this way.\n"
				"- The worldspawn keys \"codol_map\" and \"codol_base_crc32\" tie the map to its zone. Do not remove them.");
			ImGui::PopTextWrapPos();
		}

		ImGui::BeginChild("##ents_log", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

		for (const auto& [error, text] : m_log)
		{
			if (error)
			{
				ImGui::TextColored(ImVec4(0.96f, 0.45f, 0.4f, 1.0f), "%s", text.c_str());
			}
			else
			{
				ImGui::TextUnformatted(text.c_str());
			}
		}

		if (!m_log.empty() && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 2.0f)
		{
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();

		ImGui::End();
		return true;
	}

	REGISTER_GUI(codol_map_ents_dialog);
}
