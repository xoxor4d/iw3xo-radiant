#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "ui.hpp"
#include "ext.hpp"
#include "menu_ext.hpp"

#include "codol/path_picker.hpp"

// Preferences ( grouped settings instead of the raw list ), the colours of the views with presets, autosave with
// rotating backups and the list of recent maps. Every value is a saved dvar of codol_editor.cfg, so the console sets
// it by name ( ed_cam_fov 90 ) and unattended runs can ask for a state.

namespace editor::prefs
{
	namespace
	{
		constexpr int RECENT_COUNT = 10;

		bool g_show = false;
		int g_group = 0;

		game::dvar_s* need(const char* name)
		{
			auto* dvar = dvars::find(name);
			assert(dvar && "setting is not registered");
			return dvar;
		}

		// A value that lives somewhere in the editor ( the grid of the scene, the field of view of the 3D view ) and
		// its setting: whichever of the two changed since the last frame is copied to the other.
		template <typename T>
		struct link
		{
			bool started = false;
			T seen_setting{}, seen_live{};

			template <typename Get, typename Set>
			void sync(T& live, const Get& get_setting, const Set& set_setting)
			{
				const T setting = get_setting();

				if (!started)
				{
					started = true;
					live = setting;
				}
				else if (setting != seen_setting)
				{
					live = setting;
				}
				else if (live != seen_live)
				{
					set_setting(live);
				}

				seen_setting = get_setting();
				seen_live = live;
			}
		};

		// ---- autosave
		std::uint64_t g_autosave_change = 0;
		std::uint64_t g_autosave_serial = ~0ull;
		double g_autosave_due = 0.0;
		std::string g_autosave_last;

		// ---- recent
		std::uint64_t g_recent_serial = ~0ull;
		std::filesystem::path g_recent_file;

		std::string recent_name(const int index)
		{
			return "ed_recent_" + std::to_string(index);
		}

		void note_recent(const std::string& entry)
		{
			if (entry.empty()) { return; }

			std::vector<std::string> list{ entry };

			for (int i = 0; i < RECENT_COUNT; i++)
			{
				const std::string old = get_string(recent_name(i).c_str());
				if (!old.empty() && _stricmp(old.c_str(), entry.c_str()) != 0) { list.push_back(old); }
			}

			for (int i = 0; i < RECENT_COUNT; i++)
			{
				const std::string value = i < static_cast<int>(list.size()) ? list[static_cast<std::size_t>(i)] : std::string();
				if (get_string(recent_name(i).c_str()) != value) { set_string(recent_name(i).c_str(), value); }
			}
		}

		// the settings that hold a path get a ".." button ( Explorer's folder picker or file dialog )
		bool path_setting(const game::dvar_s* dvar, codol::ui::path_kind& kind, const char*& filter)
		{
			using codol::ui::path_kind;
			filter = nullptr;

			if (dvar == dvars::codol_install_path || dvar == dvars::codol_script_root || dvar == dvars::codol_script_ref_root
				|| dvar == dvars::codol_reload_precheck_cwd)
			{
				kind = path_kind::folder;
				return true;
			}

			kind = path_kind::open_file;
			if (dvar == dvars::codol_rcon_password_file || dvar == dvars::codol_xanim_list) { filter = codol::ui::filter_text; return true; }
			if (dvar == dvars::codol_launch_cmd || dvar == dvars::codol_reload_precheck_cmd) { filter = codol::ui::filter_programs; return true; }
			if (dvar == dvars::codol_asset_tool) { filter = codol::ui::filter_exe; return true; }
			return false;
		}

		void dvar_row(game::dvar_s* dvar, const char* label = nullptr)
		{
			ImGui::PushID(dvar->name);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label ? label : dvar->name);

			if (ImGui::IsItemHovered() && dvar->description && *dvar->description)
			{
				ImGui::SetTooltip("%s\n( %s )", dvar->description, dvar->name);
			}

			ImGui::SameLine(250.0f);
			ImGui::SetNextItemWidth(-1.0f);

			switch (dvar->type)
			{
			case game::dvar_type::boolean:
			{
				bool value = dvar->current.enabled;
				if (ImGui::Checkbox("##value", &value)) { dvars::set_bool(dvar, value); }
				break;
			}

			case game::dvar_type::integer:
			{
				int value = dvar->current.integer;
				if (ImGui::InputInt("##value", &value, 0) && value != dvar->current.integer)
				{
					dvars::set_int(dvar, std::clamp(value, dvar->min, dvar->max));
				}

				break;
			}

			default:
			{
				std::string value = dvar->current.string ? dvar->current.string : "";
				codol::ui::path_kind kind;
				const char* filter = nullptr;

				if (path_setting(dvar, kind, filter))
				{
					if (codol::ui::path_input("##value", value, kind, filter, nullptr, -1.0f, label ? label : dvar->name)) { dvars::set_string(dvar, value.c_str()); }
				}
				else if (ImGui::InputText("##value", &value)) { dvars::set_string(dvar, value.c_str()); }
				break;
			}
			}

			ImGui::PopID();
		}

		void slider_row(const char* name, const char* label, const int lo, const int hi, const char* format)
		{
			auto* dvar = need(name);
			ImGui::PushID(name);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);

			if (ImGui::IsItemHovered() && dvar->description && *dvar->description)
			{
				ImGui::SetTooltip("%s\n( %s )", dvar->description, dvar->name);
			}

			ImGui::SameLine(250.0f);
			ImGui::SetNextItemWidth(-1.0f);
			int value = dvar->current.integer;

			if (ImGui::SliderInt("##value", &value, lo, hi, format) && value != dvar->current.integer)
			{
				dvars::set_int(dvar, value);
			}

			ImGui::PopID();
		}

		std::filesystem::path autosave_dir()
		{
			return std::filesystem::u8path(utils::fs::get_basepath()) / "map_source" / "autosave";
		}
	}

	bool get_bool(const char* name) { return need(name)->current.enabled; }
	int get_int(const char* name) { return need(name)->current.integer; }
	float get_float(const char* name) { return static_cast<float>(std::atof(need(name)->current.string)); }
	std::string get_string(const char* name) { const char* text = need(name)->current.string; return text ? text : ""; }
	void set_bool(const char* name, const bool value) { dvars::set_bool(need(name), value); }
	void set_int(const char* name, const int value) { dvars::set_int(need(name), value); }
	void set_string(const char* name, const std::string& value) { dvars::set_string(need(name), value.c_str()); }

	void set_float(const char* name, const float value)
	{
		char text[32];
		std::snprintf(text, sizeof(text), "%g", value);
		dvars::set_string(need(name), text);
	}

	void init()
	{
		const auto saved = game::dvar_flags::saved;

		dvars::register_int("ed_ui_scale", 100, 60, 250, saved, "size of the text of the editor, percent");
		dvars::register_int("ed_undo_levels", 256, 0, 100000, saved, "steps Edit > Undo can take back ( 0: no limit )");

		dvars::register_int("ed_cam_fov", 80, 40, 120, saved, "field of view of the 3D view, degrees");
		dvars::register_int("ed_cam_speed", 600, 50, 20000, saved, "fly speed of the 3D view, units per second");
		dvars::register_bool("ed_cam_invert_y", 0, saved, "looking around with the right button: mouse up looks down");
		dvars::register_int("ed_wire_distance", 6000, 500, 40000, saved, "how far the wires of the zone's collision brushes are drawn");

		dvars::register_string("ed_grid", "8", saved, "grid size: 0.5 .. 512");
		dvars::register_bool("ed_snap", 1, saved, "drags move in steps of the grid ( Ctrl while dragging: the opposite )");
		dvars::register_int("ed_snap_angle", 15, 1, 90, saved, "the gizmo rotates in steps of this many degrees");

		dvars::register_bool("ed_autosave", 1, saved, "write a copy of a changed map to map_source\\autosave every few minutes");
		dvars::register_int("ed_autosave_minutes", 5, 1, 120, saved, "minutes between two autosaves");
		dvars::register_int("ed_autosave_keep", 5, 1, 50, saved, "autosaves kept per map: <map>.autosave.1.map is the newest");

		dvars::register_bool("ed_texlock_moves", 1, saved, "textures stay on their surface when brushes are moved");
		dvars::register_bool("ed_texlock_rotations", 1, saved, "textures stay on their surface when brushes are rotated, scaled or flipped");
		dvars::register_bool("ed_texlock_lightmaps", 0, saved, "the lightmap layer goes with brushes and patches when they are transformed ( off: it stays in the world )");

		dvars::register_bool("ed_toolbar", 1, saved, "the toolbar under the menu bar");
		dvars::register_int("ed_toolbar_size", 26, 18, 48, saved, "size of the toolbar buttons, pixels");

		for (int i = 0; i < RECENT_COUNT; i++)
		{
			dvars::register_string(_strdup(recent_name(i).c_str()), "", saved, "File > Open Recent: a .map, or zone:<name> for a map of the game");
		}

		using args_t = const std::vector<std::string>&;

		// autosave_now: what the timer does, at once ( also for an unchanged map )
		command::register_command("autosave_now", [](args_t)
		{
			std::string written;
			if (autosave_now(&written)) { game::printf_to_console("[autosave] %s\n", written.c_str()); }
			else { game::printf_to_console("^1[autosave] %s\n", written.c_str()); }
		});

		// ed_prefs [group 0..]: the Preferences window
		command::register_command("ed_prefs", [](args_t args)
		{
			g_show = true;
			if (args.size() > 1) { g_group = std::atoi(args[1].c_str()); }
		});

		command::register_command("ed_recent", [](args_t args)
		{
			if (args.size() > 1 && args[1] == "clear")
			{
				for (int i = 0; i < RECENT_COUNT; i++) { set_string(recent_name(i).c_str(), ""); }
			}

			// ed_recent <n>: open the n-th
			if (args.size() > 1 && std::isdigit(static_cast<unsigned char>(args[1][0])))
			{
				const std::string entry = get_string(recent_name(std::atoi(args[1].c_str())).c_str());
				if (entry.rfind("zone:", 0) == 0) { ui::open_zone(entry.substr(5), true); }
				else if (!entry.empty()) { ui::open_map_file(std::filesystem::u8path(entry), true); }
				return;
			}

			for (int i = 0; i < RECENT_COUNT; i++)
			{
				if (const std::string entry = get_string(recent_name(i).c_str()); !entry.empty()) { game::printf_to_console("  %d  %s\n", i, entry.c_str()); }
			}
		});
	}

	float look_sign_y()
	{
		return get_bool("ed_cam_invert_y") ? -1.0f : 1.0f;
	}

	void sync_camera(float& fov, float& fly_speed, float& wire_distance)
	{
		static link<int> fov_link, speed_link, wire_link;
		int v = static_cast<int>(std::lround(fov));
		fov_link.sync(v, [] { return get_int("ed_cam_fov"); }, [](const int value) { set_int("ed_cam_fov", std::clamp(value, 40, 120)); });
		if (v != static_cast<int>(std::lround(fov))) { fov = static_cast<float>(v); }

		v = static_cast<int>(std::lround(fly_speed));
		speed_link.sync(v, [] { return get_int("ed_cam_speed"); }, [](const int value) { set_int("ed_cam_speed", std::clamp(value, 50, 20000)); });
		if (v != static_cast<int>(std::lround(fly_speed))) { fly_speed = static_cast<float>(v); }

		v = static_cast<int>(std::lround(wire_distance));
		wire_link.sync(v, [] { return get_int("ed_wire_distance"); }, [](const int value) { set_int("ed_wire_distance", std::clamp(value, 500, 40000)); });
		if (v != static_cast<int>(std::lround(wire_distance))) { wire_distance = static_cast<float>(v); }
	}

	bool autosave_now(std::string* written)
	{
		auto& s = get_scene();
		const auto say = [&](const std::string& text) { if (written) { *written = text; } };

		if (!s.open)
		{
			say("no map is open");
			return false;
		}

		std::string name = s.map_file.empty() ? s.map_name : to_utf8(s.map_file.stem());
		if (name.empty()) { name = "unnamed"; }

		const auto dir = autosave_dir();
		std::error_code ec;
		std::filesystem::create_directories(dir, ec);

		const auto file_of = [&](const int n) { return dir / std::filesystem::u8path(name + ".autosave." + std::to_string(n) + ".map"); };
		const int keep = std::clamp(get_int("ed_autosave_keep"), 1, 50);

		// written under a temporary name first: a crash while writing never costs the newest backup
		const auto temp = dir / std::filesystem::u8path(name + ".autosave.tmp");
		{
			std::ofstream out(temp, std::ios::binary | std::ios::trunc);
			const std::string text = s.doc.save_map_text();
			out.write(text.data(), static_cast<std::streamsize>(text.size()));

			if (!out)
			{
				say("could not write " + to_utf8(temp));
				return false;
			}
		}

		// .1 is the newest: the others move up, the oldest goes
		std::filesystem::remove(file_of(keep), ec);

		for (int n = keep - 1; n >= 1; n--)
		{
			if (std::filesystem::exists(file_of(n), ec))
			{
				std::filesystem::rename(file_of(n), file_of(n + 1), ec);
			}
		}

		std::filesystem::rename(temp, file_of(1), ec);

		if (ec)
		{
			say("could not write " + to_utf8(file_of(1)) + ": " + ec.message());
			return false;
		}

		g_autosave_change = s.doc.change_counter();
		g_autosave_last = to_utf8(file_of(1));
		say(g_autosave_last);
		return true;
	}

	void frame()
	{
		auto& s = get_scene();
		auto& io = ImGui::GetIO();

		io.FontGlobalScale = static_cast<float>(std::clamp(get_int("ed_ui_scale"), 60, 250)) / 100.0f;

		// ---- settings that are values of the scene / the document
		{
			static link<double> grid_link;
			static link<bool> snap_link, lock_moves, lock_rotations, lock_lightmaps;
			static link<int> angle_link;

			grid_link.sync(s.grid, [] { return std::clamp(static_cast<double>(get_float("ed_grid")), 0.125, 512.0); }, [](const double value) { set_float("ed_grid", static_cast<float>(value)); });
			snap_link.sync(s.snap, [] { return get_bool("ed_snap"); }, [](const bool value) { set_bool("ed_snap", value); });

			int angle = static_cast<int>(std::lround(s.snap_angle));
			angle_link.sync(angle, [] { return get_int("ed_snap_angle"); }, [](const int value) { set_int("ed_snap_angle", std::clamp(value, 1, 90)); });
			if (angle != static_cast<int>(std::lround(s.snap_angle))) { s.snap_angle = static_cast<float>(angle); }

			bool moves = s.doc.texture_lock_moves(), rotations = s.doc.texture_lock_rotations();
			lock_moves.sync(moves, [] { return get_bool("ed_texlock_moves"); }, [](const bool value) { set_bool("ed_texlock_moves", value); });
			lock_rotations.sync(rotations, [] { return get_bool("ed_texlock_rotations"); }, [](const bool value) { set_bool("ed_texlock_rotations", value); });
			if (moves != s.doc.texture_lock_moves() || rotations != s.doc.texture_lock_rotations()) { s.doc.set_texture_lock(moves, rotations); }
			bool lightmaps = s.doc.texture_lock_lightmap();
			lock_lightmaps.sync(lightmaps, [] { return get_bool("ed_texlock_lightmaps"); }, [](const bool value) { set_bool("ed_texlock_lightmaps", value); });
			if (lightmaps != s.doc.texture_lock_lightmap()) { s.doc.set_texture_lock_lightmap(lightmaps); }

			s.doc.set_undo_limit(static_cast<std::size_t>(std::max(0, get_int("ed_undo_levels"))));
		}

		// ---- recent maps: whatever was opened, however it was opened
		if (s.open && (g_recent_serial != s.open_serial || g_recent_file != s.map_file))
		{
			g_recent_serial = s.open_serial;
			g_recent_file = s.map_file;
			note_recent(s.map_file.empty() ? (s.map_name.empty() ? std::string() : "zone:" + s.map_name) : to_utf8(s.map_file));
		}

		// ---- autosave
		if (s.open && g_autosave_serial != s.open_serial)
		{
			g_autosave_serial = s.open_serial;
			g_autosave_change = s.doc.change_counter();
			g_autosave_due = ImGui::GetTime() + 60.0 * get_int("ed_autosave_minutes");
		}

		// another interval counts from now
		static int interval = get_int("ed_autosave_minutes");
		if (interval != get_int("ed_autosave_minutes"))
		{
			interval = get_int("ed_autosave_minutes");
			g_autosave_due = ImGui::GetTime() + 60.0 * interval;
		}

		if (s.open && get_bool("ed_autosave") && ImGui::GetTime() >= g_autosave_due && !s.dragging() && !s.doc.in_transaction())
		{
			g_autosave_due = ImGui::GetTime() + 60.0 * get_int("ed_autosave_minutes");

			if (s.doc.modified() && s.doc.change_counter() != g_autosave_change)
			{
				std::string written;
				if (autosave_now(&written)) { game::printf_to_console("[autosave] %s\n", written.c_str()); }
				else { game::printf_to_console("^1[autosave] %s\n", written.c_str()); }
			}
		}
	}

	void draw_recent_menu()
	{
		if (!editor::menu::begin("Open Recent"))
		{
			return;
		}

		int shown = 0;

		for (int i = 0; i < RECENT_COUNT; i++)
		{
			const std::string entry = get_string(recent_name(i).c_str());
			if (entry.empty()) { continue; }

			const bool zone = entry.rfind("zone:", 0) == 0;
			const std::string label = zone ? entry.substr(5) + "   ( map of the game )" : to_utf8(std::filesystem::u8path(entry).filename());
			ImGui::PushID(i);

			if (editor::menu::item(label.c_str()))
			{
				if (zone) { ui::open_zone(entry.substr(5)); }
				else { ui::open_map_file(std::filesystem::u8path(entry)); }
			}

			if (!zone && ImGui::IsItemHovered()) { ImGui::SetTooltip("%s", entry.c_str()); }
			ImGui::PopID();
			shown++;
		}

		if (!shown)
		{
			editor::menu::item("( nothing yet )", nullptr, false, false);
		}
		else
		{
			ImGui::Separator();

			if (editor::menu::item("Clear the list"))
			{
				for (int i = 0; i < RECENT_COUNT; i++) { set_string(recent_name(i).c_str(), ""); }
			}
		}

		editor::menu::end();
	}

	void open_window()
	{
		g_show = true;
		ImGui::SetWindowFocus("Preferences##window");
	}

	void draw_window(bool* open)
	{
		// File > Settings and Edit > Preferences are the same window
		if (open && *open) { g_show = true; *open = false; }

		if (!g_show)
		{
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(860.0f, 560.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Preferences##window", &g_show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
		{
			ImGui::End();
			return;
		}

		static const char* groups[] = { "General", "Paths", "Camera", "Grid and snapping", "Autosave", "Selection", "Textures", "Game and scripts" };

		ImGui::BeginChild("##pref_groups", ImVec2(190.0f, 0.0f), true);

		for (int i = 0; i < static_cast<int>(std::size(groups)); i++)
		{
			if (ImGui::Selectable(groups[i], g_group == i)) { g_group = i; }
		}

		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("##pref_values", ImVec2(0.0f, 0.0f), false);

		const ImVec4 title(1.0f, 0.72f, 0.25f, 1.0f);
		ImGui::TextColored(title, "%s", groups[std::clamp(g_group, 0, static_cast<int>(std::size(groups)) - 1)]);
		ImGui::TextDisabled("Saved to codol_editor.cfg next to the editor. Hover a name for its meaning and its console name.");
		ImGui::Separator();
		ImGui::Spacing();

		auto& rs = render_modes::get();

		switch (g_group)
		{
		case 0:
			slider_row("ed_ui_scale", "Text size", 60, 250, "%d %%");
			dvar_row(need("ed_toolbar"), "Toolbar under the menu bar");
			slider_row("ed_toolbar_size", "Toolbar button size", 18, 48, "%d px");
			dvar_row(need("ed_undo_levels"), "Undo levels ( 0: no limit )");
			ImGui::Spacing();

			if (ImGui::Button("Edit Hotkeys ..")) { actions::open_hotkeys_window(); }
			ImGui::SameLine();
			if (ImGui::Button("Edit Colors ..")) { colors::open_window(); }
			ImGui::SameLine();
			if (ImGui::Button("Edit Toolbar ..")) { toolbar::open_edit_window(); }
			break;

		case 1:
			dvar_row(dvars::codol_install_path, "CoDOL 1.1.1.13 install");
			dvar_row(dvars::codol_script_root, "Script folder of the server");
			dvar_row(dvars::codol_script_ref_root, "Reference scripts");
			dvar_row(dvars::codol_client_exe, "Client exe");
			ImGui::Spacing();
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled("Maps are saved to map_source\\ next to the editor, autosaves to map_source\\autosave\\, built maps to compiled\\. "
				"The install is only read. A changed install path counts for the next map that is opened.");
			ImGui::PopTextWrapPos();
			break;

		case 2:
			slider_row("ed_cam_fov", "Field of view", 40, 120, "%d deg");
			slider_row("ed_cam_speed", "Fly speed", 50, 20000, "%d units/s");
			dvar_row(need("ed_cam_invert_y"), "Invert mouse Y");
			slider_row("ed_wire_distance", "Collision brush wire distance", 500, 40000, "%d");
			ImGui::Spacing();
			ImGui::Checkbox("Cubic clipping ( far clip cube )", &rs.cubic_clip);
			ImGui::SetNextItemWidth(260.0f);
			ImGui::SliderFloat("Cubic clip distance", &rs.cubic_distance, 256.0f, 65536.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
			break;

		case 3:
		{
			auto& s = get_scene();
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Grid size");
			ImGui::SameLine(250.0f);
			ImGui::SetNextItemWidth(-1.0f);

			char label[32];
			std::snprintf(label, sizeof(label), "%g", s.grid);

			if (ImGui::BeginCombo("##pref_grid", label))
			{
				for (const double g : { 0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0, 512.0 })
				{
					std::snprintf(label, sizeof(label), "%g", g);
					if (ImGui::Selectable(label, s.grid == g)) { s.set_grid(g); }
				}

				ImGui::EndCombo();
			}

			dvar_row(need("ed_snap"), "Snap drags to the grid");
			slider_row("ed_snap_angle", "Angle snap of the gizmo", 1, 90, "%d deg");
			ImGui::Spacing();
			ImGui::Checkbox("2D views: names of entities", &rs.grid_names);
			ImGui::Checkbox("2D views: grid blocks ( 1024 units )", &ui::ext::get().show_grid_blocks);
			ImGui::Checkbox("2D views: coordinates at the edges and of the blocks", &rs.grid_coordinates);
			ImGui::Checkbox("2D views: crosshair under the mouse", &rs.grid_crosshair);
			break;
		}

		case 4:
			dvar_row(need("ed_autosave"), "Autosave");
			slider_row("ed_autosave_minutes", "Every", 1, 120, "%d minutes");
			slider_row("ed_autosave_keep", "Backups kept per map", 1, 50, "%d");
			ImGui::Spacing();

			if (ImGui::Button("Autosave now"))
			{
				std::string written;
				const bool ok = autosave_now(&written);
				ImGui::Toast(ok ? ImGuiToastType_Success : ImGuiToastType_Warning, "Autosave", written.c_str(), 4000);
			}

			ImGui::SameLine();

			if (ImGui::Button("Open the autosave folder"))
			{
				std::error_code ec;
				std::filesystem::create_directories(autosave_dir(), ec);
				ShellExecuteW(nullptr, L"open", autosave_dir().wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			}

			ImGui::Spacing();
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled("A changed map is written to map_source\\autosave\\<map>.autosave.1.map; the older copies move up to .2, .3 .. and the oldest goes. "
				"The map itself and its \"modified\" mark are not touched. Open a backup with File > Open .map.");
			if (!g_autosave_last.empty()) { ImGui::TextDisabled("Last: %s", g_autosave_last.c_str()); }
			ImGui::PopTextWrapPos();
			break;

		case 5:
			ImGui::Checkbox("Outline what is selected ( 3D view )", &rs.outline_selected);
			ImGui::Checkbox("Tint what is selected ( 3D view )", &rs.tint_selected);
			ImGui::Checkbox("Show the target lines of entities", &get_scene().show_connections);
			ImGui::Checkbox("Show the angles of entities", &rs.show_angles);
			ImGui::Checkbox("Draw the origins of models", &rs.model_origins);
			ImGui::Spacing();
			ImGui::TextDisabled("The colours of the selection: Edit > Edit Colors.");
			break;

		case 6:
			dvar_row(need("ed_texlock_moves"), "Texture lock: moves");
			dvar_row(need("ed_texlock_rotations"), "Texture lock: rotations, scales, flips");
			dvar_row(need("ed_texlock_lightmaps"), "Texture lock: lightmaps");
			ImGui::Spacing();
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled("With a lock the texture stays where it is on the surface when the brush is transformed; without, it stays where it is in the world. "
				"The lightmap layer has a lock of its own: on, it goes with the brushes and patches; off ( as radiant ), faces keep their lightmap projection in the world and patches get their lightmap coordinates from their new place.");
			ImGui::PopTextWrapPos();
			break;

		default:
			for (auto* dvar : dvars::all())
			{
				const std::string_view name(dvar->name);

				// the asset tool is only used by the radiant plugin's history; nothing here converts assets
				if (!(dvar->flags & game::dvar_flags::saved) || name == "codol_asset_tool" || name.rfind("ed_", 0) == 0
					|| dvar == dvars::codol_install_path || dvar == dvars::codol_script_root || dvar == dvars::codol_script_ref_root || dvar == dvars::codol_client_exe)
				{
					continue;
				}

				dvar_row(dvar);
			}

			break;
		}

		ImGui::EndChild();
		ImGui::End();
	}
}

// ---- colours -----------------------------------------------------------------------------------------------------

namespace editor::colors
{
	namespace
	{
		struct entry
		{
			const char* key;
			const char* label;
			const char* group;
		};

		constexpr entry ENTRIES[count] = {
			{ "grid_background", "Background", "2D views" },
			{ "grid_minor", "Grid lines", "2D views" },
			{ "grid_major", "Major grid lines", "2D views" },
			{ "grid_axis", "The axes ( 0 )", "2D views" },
			{ "grid_text", "Coordinates", "2D views" },
			{ "grid_block", "Grid blocks", "2D views" },
			{ "selection", "Selected brushes and boxes", "Selection" },
			{ "selection_model", "Outline of a selected model", "Selection" },
			{ "selection_face", "Selected faces", "Selection" },
			{ "brush", "New and edited brushes", "Objects" },
			{ "brush_zone", "Collision brushes of the zone", "Objects" },
			{ "brush_detail", "Detail brushes", "Objects" },
			{ "brush_entity", "Tool brushes of the collision map", "Objects" },
			{ "patch", "Patches", "Objects" },
			{ "entity", "Entities without a class colour", "Objects" },
			{ "connection", "Target lines", "Objects" },
			{ "camera_icon", "The camera in the 2D views", "Objects" },
			{ "clipper", "Clipper", "Objects" },
			{ "view_background", "Background of the 3D view", "3D view" } };

		struct preset
		{
			const char* name;
			ImU32 values[count];
		};

		constexpr preset PRESETS[] = {
			{ "CoDOL dark", {
				IM_COL32(26, 27, 30, 255), IM_COL32(46, 48, 52, 255), IM_COL32(68, 71, 77, 255), IM_COL32(112, 116, 128, 255), IM_COL32(150, 154, 162, 255), IM_COL32(60, 80, 150, 255),
				IM_COL32(255, 58, 32, 255), IM_COL32(32, 160, 255, 255), IM_COL32(32, 224, 255, 255),
				IM_COL32(235, 235, 235, 255), IM_COL32(150, 152, 158, 255), IM_COL32(95, 165, 215, 255), IM_COL32(150, 105, 175, 255), IM_COL32(200, 170, 110, 255),
				IM_COL32(205, 205, 205, 255), IM_COL32(255, 64, 255, 255), IM_COL32(255, 255, 255, 255), IM_COL32(80, 200, 255, 255), IM_COL32(32, 32, 32, 255) } },
			{ "Radiant classic", {
				IM_COL32(255, 255, 255, 255), IM_COL32(192, 192, 192, 255), IM_COL32(128, 128, 128, 255), IM_COL32(64, 64, 64, 255), IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 255, 255),
				IM_COL32(255, 0, 0, 255), IM_COL32(255, 128, 0, 255), IM_COL32(255, 200, 0, 255),
				IM_COL32(0, 0, 0, 255), IM_COL32(96, 96, 96, 255), IM_COL32(0, 96, 160, 255), IM_COL32(128, 0, 160, 255), IM_COL32(160, 96, 0, 255),
				IM_COL32(0, 96, 0, 255), IM_COL32(200, 0, 200, 255), IM_COL32(0, 0, 255, 255), IM_COL32(0, 0, 255, 255), IM_COL32(64, 64, 64, 255) } },
			{ "Night blue", {
				IM_COL32(18, 22, 34, 255), IM_COL32(32, 40, 58, 255), IM_COL32(52, 64, 92, 255), IM_COL32(110, 130, 180, 255), IM_COL32(140, 160, 200, 255), IM_COL32(90, 110, 200, 255),
				IM_COL32(255, 110, 60, 255), IM_COL32(60, 200, 255, 255), IM_COL32(60, 240, 255, 255),
				IM_COL32(220, 228, 245, 255), IM_COL32(120, 132, 160, 255), IM_COL32(90, 190, 230, 255), IM_COL32(170, 120, 210, 255), IM_COL32(220, 190, 120, 255),
				IM_COL32(200, 210, 230, 255), IM_COL32(255, 90, 220, 255), IM_COL32(255, 255, 160, 255), IM_COL32(110, 220, 255, 255), IM_COL32(14, 18, 28, 255) } },
			{ "High contrast", {
				IM_COL32(0, 0, 0, 255), IM_COL32(40, 40, 40, 255), IM_COL32(90, 90, 90, 255), IM_COL32(200, 200, 200, 255), IM_COL32(255, 255, 255, 255), IM_COL32(0, 120, 255, 255),
				IM_COL32(255, 0, 0, 255), IM_COL32(0, 255, 255, 255), IM_COL32(0, 255, 255, 255),
				IM_COL32(255, 255, 255, 255), IM_COL32(170, 170, 170, 255), IM_COL32(0, 200, 255, 255), IM_COL32(220, 0, 255, 255), IM_COL32(255, 200, 0, 255),
				IM_COL32(0, 255, 0, 255), IM_COL32(255, 0, 255, 255), IM_COL32(255, 255, 0, 255), IM_COL32(0, 255, 255, 255), IM_COL32(0, 0, 0, 255) } } };

		ImU32 g_values[count];
		bool g_ready = false;
		bool g_show = false;
		std::uint64_t g_serial = 1;

		std::string setting_of(const int which)
		{
			return std::string("ed_color_") + ENTRIES[which].key;
		}

		std::string to_text(const ImU32 c)
		{
			char text[16];
			std::snprintf(text, sizeof(text), "#%02X%02X%02X%02X", static_cast<int>(c & 0xFF), static_cast<int>((c >> 8) & 0xFF), static_cast<int>((c >> 16) & 0xFF), static_cast<int>(c >> 24));
			return text;
		}

		bool from_text(const std::string& text, ImU32& out)
		{
			if (text.size() != 9 || text[0] != '#') { return false; }

			unsigned r = 0, g = 0, b = 0, a = 0;
			if (std::sscanf(text.c_str() + 1, "%02x%02x%02x%02x", &r, &g, &b, &a) != 4) { return false; }
			out = IM_COL32(r, g, b, a);
			return true;
		}

		void ensure()
		{
			if (g_ready) { return; }

			g_ready = true;

			for (int i = 0; i < count; i++)
			{
				g_values[i] = PRESETS[0].values[i];
				auto* dvar = dvars::register_string(_strdup(setting_of(i).c_str()), to_text(PRESETS[0].values[i]).c_str(), game::dvar_flags::saved, ENTRIES[i].label);
				from_text(dvar->current.string ? dvar->current.string : "", g_values[i]);
			}
		}

		// the settings are what counts ( the file is read after the colours are registered, the console sets them
		// by name ): the table follows them
		void refresh()
		{
			for (int i = 0; i < count; i++)
			{
				ImU32 value = g_values[i];

				if (from_text(prefs::get_string(setting_of(i).c_str()), value) && value != g_values[i])
				{
					g_values[i] = value;
					g_serial++;
				}
			}
		}

		void set(const int which, const ImU32 value)
		{
			if (g_values[which] != value)
			{
				g_values[which] = value;
				g_serial++;
			}

			if (const std::string text = to_text(value); prefs::get_string(setting_of(which).c_str()) != text)
			{
				prefs::set_string(setting_of(which).c_str(), text);
			}
		}

		void apply_preset(const preset& p)
		{
			for (int i = 0; i < count; i++) { set(i, p.values[i]); }
		}
	}

	ImU32 get(const id which)
	{
		ensure();
		return g_values[which];
	}

	DWORD d3d(const id which)
	{
		return to_d3d_color(get(which));
	}

	std::uint64_t serial()
	{
		return g_serial;
	}

	void open_window()
	{
		g_show = true;
		ImGui::SetWindowFocus("Edit Colors##window");
	}

	void draw_window()
	{
		ensure();
		refresh();		// once per frame

		// ed_colors [preset]: the window; a preset by name or number
		static bool registered = false;
		if (!registered)
		{
			registered = true;
			command::register_command("ed_colors", [](const std::vector<std::string>& args)
			{
				g_show = true;
				if (args.size() < 2) { return; }

				for (std::size_t i = 0; i < std::size(PRESETS); i++)
				{
					if (args[1] == std::to_string(i) || _stricmp(args[1].c_str(), PRESETS[i].name) == 0)
					{
						apply_preset(PRESETS[i]);
						game::printf_to_console("[colors] preset %s\n", PRESETS[i].name);
					}
				}
			});
		}

		if (!g_show)
		{
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(460.0f, 620.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Edit Colors##window", &g_show, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Preset");
		ImGui::SameLine();

		for (const auto& p : PRESETS)
		{
			if (ImGui::Button(p.name)) { apply_preset(p); }
			ImGui::SameLine();
		}

		ImGui::NewLine();
		ImGui::Separator();

		const char* group = "";

		for (int i = 0; i < count; i++)
		{
			if (std::strcmp(group, ENTRIES[i].group) != 0)
			{
				group = ENTRIES[i].group;
				ImGui::Spacing();
				ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.25f, 1.0f), "%s", group);
			}

			ImVec4 value = ImGui::ColorConvertU32ToFloat4(g_values[i]);
			ImGui::PushID(i);

			if (ImGui::ColorEdit4(ENTRIES[i].label, &value.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
			{
				set(i, ImGui::ColorConvertFloat4ToU32(value));
			}

			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%s", setting_of(i).c_str()); }
			ImGui::PopID();
		}

		ImGui::Spacing();
		ImGui::PushTextWrapPos(0.0f);
		ImGui::TextDisabled("Entities are drawn in the colour of their class ( the catalog of the game's classes ); brush entities in the colour of theirs.");
		ImGui::PopTextWrapPos();
		ImGui::End();
	}
}
