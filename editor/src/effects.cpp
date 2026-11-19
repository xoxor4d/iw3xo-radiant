#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "codol/packed_scripts.hpp"
#include "ui.hpp"
#include "assets.hpp"
#include "preview.hpp"
#include "ext.hpp"

#include <codol_assets/fx_preview.hpp>
#include <codol_zone/zone.hpp>

#include <set>

// The Effects menu: effects in the map view.
//
// An effect stands where an entity has an "fx" key ( the name of the effect: fx\<name>.efx of the project, or an
// effect of the map's zone or a common zone ), and where the createfx script of the map puts one
// ( <script root>\maps\createfx\<map>_fx.gsc, with the names of <script root>\maps\mp\<map>_fx.gsc ). The particles
// are what the Preview window draws - codol_assets simulate_fx, not the game's effect system - as textured quads
// over the picture of the 3D view: no depth test, no lighting.

namespace editor::fx
{
	namespace
	{
		namespace fs = std::filesystem;
		namespace ca = codol::assets;
		namespace cz = codol::zone;

		struct loaded_effect
		{
			bool ok = false;
			ca::EditorEffect effect;
			float duration = 1.0f;
			std::string note;
		};

		struct instance
		{
			std::string effect;
			float3 origin{};
			object_id entity = edit::no_object;		// no_object: from the createfx script
			float delay = 0.0f;
		};

		struct settings
		{
			bool show = true;				// draw the effects of the map in the 3D view
			bool playing = true;
			bool repeat = true;
			float timescale = 1.0f;
			float repeat_delay = 0.5f;
			bool debug_tris = false;
			float distance = 3000.0f;
			bool createfx = true;			// also the positions of the map's createfx script
		};

		settings g_set;
		float g_time = 0.0f;
		bool g_show_browser = false, g_show_settings = false;
		std::string g_filter, g_current;

		std::map<std::string, loaded_effect> g_effects;
		std::map<std::string, IDirect3DTexture9*> g_pictures;	// owned by the renderer
		std::uint64_t g_pictures_for = 0;			// the renderer's host_texture_generation() of the cached pointers

		// the effects of the zones ( names ), read once per map in a thread
		std::vector<std::pair<std::string, std::string>> g_zone_effects;		// name, zone
		std::atomic_bool g_zone_busy = false;
		bool g_zone_read = false;
		std::string g_zone_for;

		// createfx positions of the map's scripts
		std::vector<instance> g_script_instances;
		std::string g_script_for = "\x01";
		std::string g_script_note;

		std::vector<instance> g_instances;
		std::uint64_t g_instances_change = ~0ull;
		std::size_t g_drawn = 0, g_particles = 0;

		std::string lower(std::string text)
		{
			for (char& c : text) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
			return text;
		}

		fs::path zone_path(const std::string& zone)
		{
			const auto install = fs::u8path(components::codol_game::install_path());
			std::error_code ec;

			for (const auto& dir : { install / "client_shipRetail" / "patch", install / "client_shipRetail" / "chinese" })
			{
				if (fs::is_regular_file(dir / (zone + ".ff"), ec)) { return dir / (zone + ".ff"); }
			}

			return {};
		}

		std::vector<std::string> effect_zones()
		{
			std::vector<std::string> zones;
			if (const auto& map = get_scene().map_name; !map.empty()) { zones.push_back(map); }
			zones.insert(zones.end(), { "common_mp", "common_gametype_mp", "code_post_gfx_mp" });
			return zones;
		}

		// An effect by name: the project's .efx, else the compiled effect of a zone turned back into curves. Opening
		// a zone takes a moment; an effect is read once and kept ( Effects > Reload reads again ).
		const loaded_effect& load(const std::string& name)
		{
			if (const auto it = g_effects.find(name); it != g_effects.end())
			{
				return it->second;
			}

			loaded_effect out;
			std::error_code ec;
			const fs::path source = assets::project_dir() / "fx" / fs::u8path(name + ".efx");

			if (!assets::project().empty() && fs::is_regular_file(source, ec))
			{
				if (const auto status = ca::load_efx(source, out.effect); status)
				{
					out.ok = true;
					out.note = "the project's fx\\" + name + ".efx";
				}
				else
				{
					out.note = to_utf8(source.filename()) + ": " + status.message;
				}
			}
			else
			{
				cz::ClientKey key;
				const auto status = key.load_from_exe(fs::u8path(components::codol_game::install_path()) / "codoMP_client_shipRetail.exe");
				out.note = status ? "no effect " + name + " in the project, the zone of the map or the common zones" : status.message;

				for (const auto& zone_name : status ? effect_zones() : std::vector<std::string>{})
				{
					const fs::path file = zone_path(zone_name);
					cz::Zone zone;
					if (file.empty() || !zone.open(file, key)) { continue; }

					const auto header = zone.find_asset(codol::c1::ASSET_TYPE_FX, lower(name));
					ca::CompiledFx compiled;

					if (header && ca::read_fx(zone, header, compiled) && ca::extract_fx(compiled, out.effect))
					{
						out.ok = true;
						out.note = "read out of " + to_utf8(file.filename());
						break;
					}
				}
			}

			if (out.ok)
			{
				// atlas cells as the effects compiler takes them from the material ( see preview.cpp )
				ca::FxAtlasLibrary atlas;

				if (atlas.load(assets::project_dir().parent_path() / "codol_fxatlas.txt"))
				{
					for (auto& e : out.effect.elems)
					{
						int columns = 1, rows = 1;

						if (e.atlasColIndexBits == 0 && e.atlasRowIndexBits == 0 && !e.visuals.empty() && atlas.find(e.visuals[0], columns, rows))
						{
							while ((1 << e.atlasColIndexBits) < columns) { e.atlasColIndexBits++; }
							while ((1 << e.atlasRowIndexBits) < rows) { e.atlasRowIndexBits++; }
						}
					}
				}

				out.duration = ca::fx_preview_duration(out.effect);
			}
			else
			{
				game::printf_to_console("^1[fx] %s\n", out.note.c_str());
			}

			return g_effects.emplace(name, std::move(out)).first->second;
		}

		void read_zone_effects()
		{
			if (g_zone_busy.exchange(true)) { return; }

			g_zone_for = get_scene().map_name;
			const auto zones = effect_zones();
			const auto install = fs::u8path(components::codol_game::install_path());

			std::thread([zones, install]
			{
				std::vector<std::pair<std::string, std::string>> found;
				cz::ClientKey key;

				if (key.load_from_exe(install / "codoMP_client_shipRetail.exe"))
				{
					for (const auto& name : zones)
					{
						const fs::path file = zone_path(name);
						cz::Zone zone;
						if (file.empty() || !zone.open(file, key)) { continue; }

						for (const auto& entry : zone.assets())
						{
							if (entry.type == codol::c1::ASSET_TYPE_FX)
							{
								std::string asset(zone.asset_name(entry.type, entry.header));
								if (!asset.empty() && asset[0] != ',') { found.emplace_back(std::move(asset), name); }
							}
						}
					}
				}

				std::sort(found.begin(), found.end());

				components::codol_game::post_to_gui([found = std::move(found)]() mutable
				{
					g_zone_effects = std::move(found);
					g_zone_read = true;
					g_zone_busy = false;
					game::printf_to_console("[fx] %d effects in the zones of the map\n", static_cast<int>(g_zone_effects.size()));
				});
			}).detach();
		}

		// ---- createfx scripts: where the level script of a map plays its effects
		std::string read_text(const fs::path& file)
		{
			std::ifstream in(file, std::ios::binary);
			return in ? std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()) : std::string();
		}

		void read_createfx()
		{
			const auto& s = get_scene();
			g_script_for = s.map_name;
			g_script_instances.clear();
			g_script_note.clear();

			if (s.map_name.empty()) { return; }

			// the script folder, the reference folder, then sh1x_svr_scripts.iwd of the install
			std::string placement_text, placement;

			if (codol::packed_scripts::read("maps/createfx/" + s.map_name + "_fx.gsc", placement_text, &placement))
			{
				// level._effect[ "id" ] = loadfx( "path" );
				std::map<std::string, std::string> ids;
				{
					std::string text;
					codol::packed_scripts::read("maps/mp/" + s.map_name + "_fx.gsc", text);
					static const std::regex effect(R"re(_effect\s*\[\s*"([^"]+)"\s*\]\s*=\s*loadfx\s*\(\s*"([^"]+)"\s*\))re", std::regex::icase);

					for (std::sregex_iterator it(text.begin(), text.end(), effect), end; it != end; ++it)
					{
						ids[lower((*it)[1].str())] = lower((*it)[2].str());
					}
				}

				// ent = createOneshotEffect( "id" ); ent.v[ "origin" ] = ( x, y, z ); .. ent.v[ "delay" ] = n;
				const std::string& text = placement_text;
				static const std::regex create(R"re(create(?:Oneshot|Loop)Effect\s*\(\s*"([^"]+)"\s*\))re", std::regex::icase);
				static const std::regex origin(R"re(\[\s*"origin"\s*\]\s*=\s*\(\s*([-0-9.eE]+)\s*,\s*([-0-9.eE]+)\s*,\s*([-0-9.eE]+)\s*\))re");
				static const std::regex delay(R"re(\[\s*"delay"\s*\]\s*=\s*([-0-9.eE]+))re");

				std::vector<std::pair<std::size_t, std::string>> starts;
				for (std::sregex_iterator it(text.begin(), text.end(), create), end; it != end; ++it)
				{
					starts.emplace_back(static_cast<std::size_t>(it->position()), lower((*it)[1].str()));
				}

				for (std::size_t i = 0; i < starts.size(); i++)
				{
					const std::string block = text.substr(starts[i].first, (i + 1 < starts.size() ? starts[i + 1].first : text.size()) - starts[i].first);
					std::smatch m;
					if (!std::regex_search(block, m, origin)) { continue; }

					instance inst;
					const auto id = ids.find(starts[i].second);
					inst.effect = id != ids.end() ? id->second : starts[i].second;
					inst.origin = { static_cast<float>(std::atof(m[1].str().c_str())), static_cast<float>(std::atof(m[2].str().c_str())), static_cast<float>(std::atof(m[3].str().c_str())) };
					if (std::regex_search(block, m, delay)) { inst.delay = std::max(0.0f, static_cast<float>(std::atof(m[1].str().c_str()))); }
					g_script_instances.push_back(std::move(inst));
				}

				g_script_note = placement + ": " + std::to_string(g_script_instances.size()) + " effects, " + std::to_string(ids.size()) + " names";
				game::printf_to_console("[fx] %s\n", g_script_note.c_str());
				return;
			}

			g_script_note = "no maps\\createfx\\" + s.map_name + "_fx.gsc in the script folders of File > Settings or in sh1x_svr_scripts.iwd";
		}

		void collect_instances()
		{
			auto& s = get_scene();
			const auto& doc = s.doc;

			if (g_script_for != s.map_name)
			{
				read_createfx();
				g_instances_change = ~0ull;
			}

			if (g_instances_change == doc.change_counter() + doc.visibility_counter())
			{
				return;
			}

			g_instances_change = doc.change_counter() + doc.visibility_counter();
			g_instances.clear();

			for (const object_id id : doc.entities())
			{
				const auto* e = doc.find_entity(id);
				if (!e || !doc.is_visible(id)) { continue; }

				const auto name = e->value("fx");
				if (name.empty()) { continue; }

				instance inst;
				inst.effect = lower(std::string(name));
				const auto o = e->origin();
				inst.origin = { static_cast<float>(o.x), static_cast<float>(o.y), static_cast<float>(o.z) };
				inst.entity = id;
				inst.delay = static_cast<float>(std::atof(std::string(e->value("delay", "0")).c_str()));
				g_instances.push_back(std::move(inst));
			}

			if (g_set.createfx)
			{
				g_instances.insert(g_instances.end(), g_script_instances.begin(), g_script_instances.end());
			}
		}

		// the effect of the selected entity, else the one picked in the browser
		std::string current_effect()
		{
			const auto& doc = get_scene().doc;

			for (const object_id id : doc.selection())
			{
				if (const auto* e = doc.find_entity(id); e && !e->value("fx").empty()) { return lower(std::string(e->value("fx"))); }
			}

			return g_current;
		}

		bool additive(const std::string_view material)
		{
			for (const char* word : { "spark", "fire", "flare", "glow", "flash", "light", "tracer", "_add", "ember", "muzzle" })
			{
				if (material.find(word) != std::string_view::npos) { return true; }
			}

			return false;
		}

		void place(const std::string& name)
		{
			auto& s = get_scene();
			if (!s.open) { return; }

			const auto at = edit::snap_point(s.ray_target(s.cam.origin, s.cam.axes().forward), s.snap ? s.grid : 0.0);
			const std::vector<std::pair<std::string, std::string>> pairs = { { "targetname", "fx_origin" }, { "fx", name } };
			const object_id id = s.doc.create_point_entity("script_origin", at, pairs);

			if (id) { game::printf_to_console("[fx] %s placed: script_origin #%u at %g %g %g\n", name.c_str(), id, at.x, at.y, at.z); }
			else { game::printf_to_console("^1[fx] %s\n", s.doc.last_error().c_str()); }
		}

		// the effects of the map's entities as a createfx script the level script can call
		bool write_createfx(std::string& message)
		{
			auto& s = get_scene();
			const auto& doc = s.doc;
			const std::string map = s.map_file.empty() ? s.map_name : to_utf8(s.map_file.stem());

			if (!s.open || map.empty())
			{
				message = "no map is open";
				return false;
			}

			std::set<std::string> names;
			std::string body;
			int count = 0;

			for (const object_id id : doc.entities())
			{
				const auto* e = doc.find_entity(id);
				if (!e || e->value("fx").empty()) { continue; }

				const std::string name = lower(std::string(e->value("fx")));
				std::string fxid = name.substr(name.find_last_of('/') == std::string::npos ? 0 : name.find_last_of('/') + 1);
				names.insert(name);

				const auto o = e->origin();
				const auto a = e->angles();
				char text[512];
				std::snprintf(text, sizeof(text),
					"\tent = maps\\mp\\_utility::createOneshotEffect( \"%s\" );\r\n\tent.v[ \"origin\" ] = ( %g, %g, %g );\r\n\tent.v[ \"angles\" ] = ( %g, %g, %g );\r\n\tent.v[ \"fxid\" ] = \"%s\";\r\n\tent.v[ \"delay\" ] = %s;\r\n\r\n",
					fxid.c_str(), o.x, o.y, o.z, a.x == 0.0 && a.y == 0.0 && a.z == 0.0 ? 270.0 : a.x, a.y, a.z, fxid.c_str(), std::string(e->value("delay", "-15")).c_str());
				body += text;
				count++;
			}

			if (!count)
			{
				message = "the map has no entity with an \"fx\" key";
				return false;
			}

			std::string out = "// " + map + ": effects placed in CoDOL Radiant ( entities with an \"fx\" key ). Call main() from the level script before _load::main().\r\n\r\nmain()\r\n{\r\n";

			for (const auto& name : names)
			{
				out += "\tlevel._effect[ \"" + name.substr(name.find_last_of('/') == std::string::npos ? 0 : name.find_last_of('/') + 1) + "\" ] = loadfx( \"" + name + "\" );\r\n";
			}

			out += "\r\n" + body + "}\r\n";

			const fs::path file = fs::u8path(utils::fs::get_basepath()) / "map_source" / (map + "_createfx.gsc");
			std::ofstream stream(file, std::ios::binary | std::ios::trunc);
			stream << out;

			message = stream ? to_utf8(file) + ": " + std::to_string(count) + " effects" : "could not write " + to_utf8(file);
			return static_cast<bool>(stream);
		}
	}

	// ---- actions -------------------------------------------------------------------------------------------------

	void register_actions()
	{
		using namespace actions;
		const auto saved = game::dvar_flags::saved;
		dvars::register_bool("ed_fx_show", 1, saved, "3D view: play the effects of the map ( entities with an \"fx\" key, createfx positions )");

		add("fx.play", "Effects: play", "", [] { g_set.show = true; g_set.playing = true; }, anywhere, no_map).toggle([] { return g_set.show && g_set.playing; });
		add("fx.pause", "Effects: pause", "", [] { g_set.playing = false; }, anywhere, no_map).toggle([] { return g_set.show && !g_set.playing; });
		add("fx.stop", "Effects: stop", "", [] { g_set.show = false; g_set.playing = false; g_time = 0.0f; }, anywhere, no_map).toggle([] { return !g_set.show; });
		add("fx.repeat", "Effects: repeat", "", [] { g_set.repeat = !g_set.repeat; g_time = 0.0f; }, anywhere, no_map).toggle([] { return g_set.repeat; });
		add("fx.edit", "Edit the current effect", "", []
		{
			const std::string name = current_effect();
			if (name.empty()) { ImGui::Toast(ImGuiToastType_Warning, "Effects", "Select an entity with an \"fx\" key, or pick an effect in the Effects Browser."); return; }

			ui::get().show_preview = true;
			preview::show_fx(name);

			// a source of the project opens in whatever edits .efx on this machine
			std::error_code ec;
			if (const fs::path source = assets::project_dir() / "fx" / fs::u8path(name + ".efx"); !assets::project().empty() && fs::is_regular_file(source, ec))
			{
				ShellExecuteW(nullptr, L"open", source.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			}
		});
		add("fx.reload", "Reload the effects", "", []
		{
			g_effects.clear();
			g_pictures.clear();
			g_script_for = "\x01";
			g_zone_read = false;
			game::printf_to_console("[fx] effects are read again\n");
		}, anywhere, no_map);
		add("fx.browser", "Effects Browser", "", [] { g_show_browser = !g_show_browser; }, anywhere, no_map).toggle([] { return g_show_browser; });
		add("fx.settings", "Effect Settings ..", "", [] { g_show_settings = !g_show_settings; }, anywhere, no_map).toggle([] { return g_show_settings; });
		add("fx.debug_tris", "Effects: draw debug tris", "", [] { g_set.debug_tris = !g_set.debug_tris; }, anywhere, no_map).toggle([] { return g_set.debug_tris; });
		add("fx.write_createfx", "Write the createfx script of the map's effects", "", []
		{
			std::string message;
			const bool ok = write_createfx(message);
			game::printf_to_console("%s[fx] %s\n", ok ? "" : "^1", message.c_str());
			ImGui::Toast(ok ? ImGuiToastType_Success : ImGuiToastType_Warning, "Createfx", message.c_str(), 5000);
		});

		using args_t = const std::vector<std::string>&;

		// fx <play|pause|stop|repeat 0|1|time <s>|timescale <f>|delay <s>|tris 0|1|place <effect>|list|zone [text]|status|browser|settings|reload|createfx>
		command::register_command("fx", [](args_t args)
		{
			const std::string what = args.size() > 1 ? args[1] : "status";
			const std::string value = args.size() > 2 ? args[2] : "";

			if (what == "play") { run("fx.play"); }
			else if (what == "pause") { run("fx.pause"); }
			else if (what == "stop") { run("fx.stop"); }
			else if (what == "repeat") { g_set.repeat = value != "0"; }
			else if (what == "time") { g_time = static_cast<float>(std::atof(value.c_str())); g_set.playing = false; g_set.show = true; }
			else if (what == "timescale") { g_set.timescale = std::clamp(static_cast<float>(std::atof(value.c_str())), 0.01f, 10.0f); }
			else if (what == "delay") { g_set.repeat_delay = std::max(0.0f, static_cast<float>(std::atof(value.c_str()))); }
			else if (what == "tris") { g_set.debug_tris = value != "0"; }
			else if (what == "place" && !value.empty()) { g_current = lower(value); place(g_current); }
			else if (what == "browser") { g_show_browser = true; g_filter = value; }
			else if (what == "settings") { g_show_settings = true; }
			else if (what == "reload") { run("fx.reload"); }
			else if (what == "createfx") { run("fx.write_createfx"); }
			else if (what == "zone")
			{
				// the effects of the zones of the map ( what the Effects Browser lists ), by a part of the name
				if (!g_zone_read) { read_zone_effects(); game::printf_to_console("[fx] reading the zones ..\n"); return; }

				int shown = 0;
				for (const auto& [name, zone] : g_zone_effects)
				{
					if (!value.empty() && name.find(value) == std::string::npos) { continue; }
					if (++shown <= 40) { game::printf_to_console("  %s  ( %s )\n", name.c_str(), zone.c_str()); }
				}

				game::printf_to_console("%d effects\n", shown);
			}
			else if (what == "list")
			{
				collect_instances();
				for (const auto& i : g_instances)
				{
					const auto& effect = load(i.effect);
					game::printf_to_console("  %s  at %.0f %.0f %.0f  %s  %s\n", i.effect.c_str(), i.origin.x, i.origin.y, i.origin.z, i.entity ? "entity" : "createfx", effect.ok ? effect.note.c_str() : "NOT FOUND");
				}

				game::printf_to_console("%d effects in the map\n", static_cast<int>(g_instances.size()));
			}
			else
			{
				game::printf_to_console("[fx] %s, %s, repeat %d, time %.2f s, timescale %g, repeat delay %g s; %d effects in the map, %d drawn with %d particles last frame; current \"%s\"\n",
					g_set.show ? "shown" : "stopped", g_set.playing ? "playing" : "paused", g_set.repeat ? 1 : 0, g_time, g_set.timescale, g_set.repeat_delay,
					static_cast<int>(g_instances.size()), static_cast<int>(g_drawn), static_cast<int>(g_particles), current_effect().c_str());
				if (!g_script_note.empty()) { game::printf_to_console("[fx] %s\n", g_script_note.c_str()); }
			}
		});
	}

	void draw_menu()
	{
		if (!editor::menu::begin("Effects"))
		{
			return;
		}

		actions::menu_item("fx.settings");
		actions::menu_item("fx.browser");
		ImGui::Separator();
		actions::menu_item("fx.play", "Play");
		actions::menu_item("fx.pause", "Pause");
		actions::menu_item("fx.stop", "Stop");
		actions::menu_item("fx.repeat", "Repeat");
		ImGui::Separator();
		actions::menu_item("fx.edit", "Edit Current Effect");
		actions::menu_item("fx.reload", "Reload Current Effect ( all )");
		actions::menu_item("fx.debug_tris", "Draw Debug Tris");
		ImGui::Separator();
		actions::menu_item("fx.write_createfx");
		editor::menu::end();
	}

	// ---- windows ---------------------------------------------------------------------------------------------------

	void draw_windows()
	{
		// the timeline of every effect in the view
		if (g_set.show && g_set.playing)
		{
			g_time += ImGui::GetIO().DeltaTime * g_set.timescale;
		}

		if (g_show_settings)
		{
			ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f), ImGuiCond_FirstUseEver);

			if (ImGui::Begin("Effect Settings##window", &g_show_settings, ImGuiWindowFlags_NoCollapse))
			{
				ImGui::Checkbox("Play the effects of the map in the 3D view", &g_set.show);
				ImGui::Checkbox("Playing", &g_set.playing);
				ImGui::SameLine();
				ImGui::Checkbox("Repeat", &g_set.repeat);
				ImGui::SliderFloat("timescale", &g_set.timescale, 0.05f, 4.0f, "x %.2f");
				ImGui::SliderFloat("repeat delay", &g_set.repeat_delay, 0.0f, 10.0f, "%.2f s");
				ImGui::SliderFloat("draw distance", &g_set.distance, 200.0f, 20000.0f, "%.0f units", ImGuiSliderFlags_Logarithmic);
				ImGui::Checkbox("Draw debug tris", &g_set.debug_tris);

				if (ImGui::Checkbox("Also the positions of the map's createfx script", &g_set.createfx)) { g_instances_change = ~0ull; }
				ImGui::Separator();
				ImGui::DragFloat("time", &g_time, 0.01f, 0.0f, 3600.0f, "%.2f s");
				ImGui::TextDisabled("%d effects in the map, %d drawn with %d particles.", static_cast<int>(g_instances.size()), static_cast<int>(g_drawn), static_cast<int>(g_particles));
				if (!g_script_note.empty()) { ImGui::TextWrapped("%s", g_script_note.c_str()); }
				ImGui::PushTextWrapPos(0.0f);
				ImGui::TextDisabled("An effect plays where an entity has an \"fx\" key, and where the createfx script of the map puts one. The particles are the editor's own simulation ( as in the Preview window ): "
					"sprites, tails and lines; no trails, models, lights or sounds; drawn over the picture without depth.");
				ImGui::PopTextWrapPos();
			}

			ImGui::End();
		}

		if (!g_show_browser)
		{
			return;
		}

		auto& s = get_scene();

		if (!g_zone_busy && (!g_zone_read || g_zone_for != s.map_name))
		{
			read_zone_effects();
		}

		ImGui::SetNextWindowSize(ImVec2(460.0f, 600.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Effects Browser##window", &g_show_browser, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##fx_filter", "filter", &g_filter);
		const std::string needle = lower(g_filter);

		const bool can_place = s.open && !g_current.empty();
		if (!can_place) { ImGui::BeginDisabled(); }
		if (ImGui::Button("Place where the camera looks")) { place(g_current); }
		if (!can_place) { ImGui::EndDisabled(); }
		ImGui::SameLine();
		if (ImGui::Button("Preview") && !g_current.empty()) { ui::get().show_preview = true; preview::show_fx(g_current); }
		ImGui::SameLine();
		ImGui::TextDisabled("%s", g_current.empty() ? "pick an effect" : g_current.c_str());

		// transport of the effects in the 3D view
		for (const char* id : { "fx.play", "fx.pause", "fx.stop", "fx.repeat" })
		{
			if (const auto* a = actions::find(id))
			{
				const bool on = a->checked && a->checked();
				const ImVec2 at = ImGui::GetCursorScreenPos();
				const float size = ImGui::GetFrameHeight();
				if (on) { ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive)); }
				if (ImGui::Button((std::string("##") + id).c_str(), ImVec2(size * 1.3f, size))) { a->run(); }
				if (on) { ImGui::PopStyleColor(); }
				if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%s", a->label.c_str()); }
				toolbar::draw_icon(ImGui::GetWindowDrawList(), id, ImVec2(at.x + size * 0.3f, at.y + size * 0.15f), size * 0.7f, ImGui::GetColorU32(ImGuiCol_Text));
				ImGui::SameLine(0.0f, 2.0f);
			}
		}

		ImGui::NewLine();

		if (ImGui::BeginChild("##fx_list", ImVec2(0.0f, 0.0f), true))
		{
			const auto row = [&](const std::string& name, const char* where)
			{
				if (!needle.empty() && name.find(needle) == std::string::npos) { return; }

				ImGui::PushID(where);
				const bool clicked = ImGui::Selectable(name.c_str(), name == g_current, ImGuiSelectableFlags_AllowDoubleClick);
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextDisabled("%s", where);

				if (clicked)
				{
					g_current = name;

					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) { place(name); }
					else if (ui::get().show_preview) { preview::show_fx(name); }
				}
			};

			// the project's sources
			std::error_code ec;
			const fs::path dir = assets::project_dir() / "fx";
			std::vector<std::string> own;

			if (!assets::project().empty() && fs::is_directory(dir, ec))
			{
				for (const auto& entry : fs::recursive_directory_iterator(dir, ec))
				{
					if (entry.is_regular_file(ec) && lower(to_utf8(entry.path().extension())) == ".efx")
					{
						std::string name = to_utf8(fs::relative(entry.path(), dir, ec));
						std::replace(name.begin(), name.end(), '\\', '/');
						own.push_back(lower(name.substr(0, name.size() - 4)));
					}
				}
			}

			std::sort(own.begin(), own.end());
			for (const auto& name : own) { row(name, "project"); }

			if (g_zone_busy) { ImGui::TextDisabled("reading the zones .."); }
			for (const auto& [name, zone] : g_zone_effects) { row(name, zone.c_str()); }
		}

		ImGui::EndChild();
		ImGui::End();
	}

	// ---- the 3D view ---------------------------------------------------------------------------------------------

	void draw_in_view(ImDrawList* dl, const ImVec2& origin, const int width, const int height)
	{
		g_drawn = g_particles = 0;

		auto& s = get_scene();
		if (!g_set.show || !s.open || !dvars::find("ed_fx_show")->current.enabled)
		{
			return;
		}

		collect_instances();
		if (g_instances.empty())
		{
			return;
		}

		// the pictures belong to the renderer: gone with the map
		if (g_pictures_for != s.renderer.host_texture_generation())
		{
			g_pictures_for = s.renderer.host_texture_generation();
			g_pictures.clear();
		}

		const auto axes = s.cam.axes();
		const float fwd[3] = { axes.forward.x, axes.forward.y, axes.forward.z }, right[3] = { -axes.left.x, -axes.left.y, -axes.left.z }, up[3] = { axes.up.x, axes.up.y, axes.up.z };
		const float eye[3] = { s.cam.origin.x, s.cam.origin.y, s.cam.origin.z };
		const float focal = static_cast<float>(width) * 0.5f / std::tan(s.cam.fov_x * 0.5f * 0.0174532925f);
		const float cx = origin.x + static_cast<float>(width) * 0.5f, cy = origin.y + static_cast<float>(height) * 0.5f;

		const auto project = [&](const float p[3], ImVec2& out) -> float
		{
			const float d[3] = { p[0] - eye[0], p[1] - eye[1], p[2] - eye[2] };
			const float z = d[0] * fwd[0] + d[1] * fwd[1] + d[2] * fwd[2];
			if (z < 1.0f) { return z; }
			out = ImVec2(cx + (d[0] * right[0] + d[1] * right[1] + d[2] * right[2]) / z * focal, cy - (d[0] * up[0] + d[1] * up[1] + d[2] * up[2]) / z * focal);
			return z;
		};

		struct drawn
		{
			ca::FxParticle p;
			float depth;
		};

		static std::vector<drawn> order;
		static ca::FxPreview moment;
		order.clear();

		for (const auto& inst : g_instances)
		{
			const float d[3] = { inst.origin.x - eye[0], inst.origin.y - eye[1], inst.origin.z - eye[2] };
			const float distance2 = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];

			if (distance2 > g_set.distance * g_set.distance || (d[0] * fwd[0] + d[1] * fwd[1] + d[2] * fwd[2]) < -256.0f)
			{
				continue;
			}

			const auto& effect = load(inst.effect);

			if (!effect.ok)
			{
				// an effect that cannot be read: a mark where it would be
				ImVec2 at;
				const float where[3] = { inst.origin.x, inst.origin.y, inst.origin.z };
				if (project(where, at) >= 1.0f) { dl->AddText(ImVec2(at.x + 6.0f, at.y - 6.0f), IM_COL32(245, 130, 110, 255), ("fx? " + inst.effect).c_str()); }
				continue;
			}

			// where on its timeline the effect is: it starts again after its length and the delay
			const float period = effect.duration + std::max(0.0f, g_set.repeat_delay + std::max(0.0f, inst.delay));
			float t = g_time;
			if (g_set.repeat) { t = std::fmod(t, std::max(period, 0.05f)); }
			if (t > effect.duration) { continue; }

			ca::simulate_fx(effect.effect, t, moment);
			g_drawn++;

			for (const auto& p : moment.particles)
			{
				drawn item{ p, 0.0f };
				item.p.position[0] += inst.origin.x;
				item.p.position[1] += inst.origin.y;
				item.p.position[2] += inst.origin.z;
				const float pd[3] = { item.p.position[0] - eye[0], item.p.position[1] - eye[1], item.p.position[2] - eye[2] };
				item.depth = pd[0] * fwd[0] + pd[1] * fwd[1] + pd[2] * fwd[2];
				if (item.depth >= 1.0f && item.p.width > 0.0f && item.p.color[3] > 0.003f && order.size() < 6000) { order.push_back(item); }
			}
		}

		if (order.empty())
		{
			return;
		}

		std::sort(order.begin(), order.end(), [](const drawn& a, const drawn& b) { return a.depth > b.depth; });
		g_particles = order.size();
		int mode = 0;		// 0: as ImGui blends, 1: additive

		for (const auto& item : order)
		{
			const auto& p = item.p;
			const std::string material(p.material);
			auto found = g_pictures.find(material);

			if (found == g_pictures.end())
			{
				found = g_pictures.emplace(material, s.renderer.loaded() ? s.renderer.material_thumbnail(material, 512) : nullptr).first;
			}

			// two edges of the quad, half lengths ( as preview.cpp draws an effect )
			float e1[3], e2[3], centre[3] = { p.position[0], p.position[1], p.position[2] };
			const float turn = p.rotation * 0.0174532925f, c = std::cos(turn), sn = std::sin(turn);

			if (p.shape == ca::FxShape::tail || p.shape == ca::FxShape::line)
			{
				float along[3] = { p.velocity[0], p.velocity[1], p.velocity[2] };
				float len = std::sqrt(along[0] * along[0] + along[1] * along[1] + along[2] * along[2]);
				if (len < 1e-4f) { along[0] = up[0]; along[1] = up[1]; along[2] = up[2]; len = 1.0f; }
				for (float& v : along) { v /= len; }
				float across[3] = { along[1] * fwd[2] - along[2] * fwd[1], along[2] * fwd[0] - along[0] * fwd[2], along[0] * fwd[1] - along[1] * fwd[0] };
				float al = std::sqrt(across[0] * across[0] + across[1] * across[1] + across[2] * across[2]);
				if (al < 1e-4f) { across[0] = right[0]; across[1] = right[1]; across[2] = right[2]; al = 1.0f; }

				for (int k = 0; k < 3; k++)
				{
					e1[k] = across[k] / al * p.width * 0.5f;
					e2[k] = along[k] * p.height * 0.5f;
					centre[k] -= along[k] * p.height * 0.5f;
				}
			}
			else if (p.shape == ca::FxShape::oriented)
			{
				const float n[3] = { p.normal[0], p.normal[1], p.normal[2] };
				float a[3] = { n[1], -n[0], 0.0f };
				if (std::fabs(a[0]) + std::fabs(a[1]) < 1e-4f) { a[0] = 1.0f; a[1] = 0.0f; }
				const float al = std::sqrt(a[0] * a[0] + a[1] * a[1]);
				a[0] /= al; a[1] /= al;
				const float b[3] = { n[1] * a[2] - n[2] * a[1], n[2] * a[0] - n[0] * a[2], n[0] * a[1] - n[1] * a[0] };

				for (int k = 0; k < 3; k++)
				{
					e1[k] = (a[k] * c + b[k] * sn) * p.width * 0.5f;
					e2[k] = (b[k] * c - a[k] * sn) * p.height * 0.5f;
				}
			}
			else
			{
				for (int k = 0; k < 3; k++)
				{
					e1[k] = (right[k] * c + up[k] * sn) * p.width * 0.5f;
					e2[k] = (up[k] * c - right[k] * sn) * p.height * 0.5f;
				}
			}

			ImVec2 q[4];
			bool visible = true;
			const float sign[4][2] = { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };

			for (int k = 0; k < 4 && visible; k++)
			{
				const float corner[3] = { centre[0] + e1[0] * sign[k][0] + e2[0] * sign[k][1], centre[1] + e1[1] * sign[k][0] + e2[1] * sign[k][1], centre[2] + e1[2] * sign[k][0] + e2[2] * sign[k][1] };
				visible = project(corner, q[k]) >= 1.0f;
			}

			if (!visible) { continue; }

			const ImU32 colour = IM_COL32(static_cast<int>(std::clamp(p.color[0], 0.0f, 1.0f) * 255.0f), static_cast<int>(std::clamp(p.color[1], 0.0f, 1.0f) * 255.0f),
				static_cast<int>(std::clamp(p.color[2], 0.0f, 1.0f) * 255.0f), static_cast<int>(std::clamp(p.color[3], 0.0f, 1.0f) * 255.0f));
			const int wanted = additive(p.material) ? 1 : 0;

			if (wanted != mode)
			{
				if (wanted)
				{
					dl->AddCallback([](const ImDrawList*, const ImDrawCmd*)
					{
						if (game::glob::d3d9_device)
						{
							game::glob::d3d9_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
							game::glob::d3d9_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
						}
					}, nullptr);
				}
				else
				{
					dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
				}

				mode = wanted;
			}

			if (found->second)
			{
				dl->AddImageQuad(found->second, q[0], q[1], q[2], q[3], ImVec2(p.u0, p.v0), ImVec2(p.u1, p.v0), ImVec2(p.u1, p.v1), ImVec2(p.u0, p.v1), colour);
			}
			else
			{
				const float radius = 0.5f * std::max(std::fabs(q[1].x - q[0].x) + std::fabs(q[1].y - q[0].y), std::fabs(q[3].x - q[0].x) + std::fabs(q[3].y - q[0].y)) * 0.6f;
				const ImVec2 mid((q[0].x + q[2].x) * 0.5f, (q[0].y + q[2].y) * 0.5f);
				dl->AddCircleFilled(mid, std::max(radius, 1.0f), (colour & 0x00FFFFFF) | (static_cast<ImU32>(std::clamp(p.color[3], 0.0f, 1.0f) * 110.0f) << 24), 20);
			}

			if (g_set.debug_tris)
			{
				dl->AddQuad(q[0], q[1], q[2], q[3], IM_COL32(255, 255, 0, 200));
				dl->AddLine(q[0], q[2], IM_COL32(255, 255, 0, 200));
			}
		}

		if (mode)
		{
			dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
		}
	}
}
