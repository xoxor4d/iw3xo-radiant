#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "ui.hpp"
#include "edit3d.hpp"
#include "assets.hpp"
#include "preview.hpp"
#include "ext.hpp"

#include <commdlg.h>
#include <atomic>
#include <optional>

// Editing actions, shortcuts, the File / Edit menus and the console commands. Every action is one function that
// the menu, the key and the command call - so anything the mouse can do can be driven ( and tested ) by command.

namespace editor::ui
{
	state& get()
	{
		static state s;
		return s;
	}

	namespace
	{
		// an action that waits for the answer to "unsaved changes"
		std::function<void()> g_pending;
		bool g_ask = false;

		void guarded(const bool force, std::function<void()> action)
		{
			auto& s = get_scene();

			if (force || !s.open || !s.doc.modified())
			{
				action();
				return;
			}

			g_pending = std::move(action);
			g_ask = true;
		}

		double number(const std::vector<std::string>& args, const std::size_t index, const double fallback = 0.0)
		{
			return index < args.size() ? std::atof(args[index].c_str()) : fallback;
		}

		edit::vec3 vector(const std::vector<std::string>& args, const std::size_t first)
		{
			return { number(args, first), number(args, first + 1), number(args, first + 2) };
		}

		bool need_map()
		{
			if (!get_scene().open)
			{
				game::printf_to_console("^1no map is open\n");
				return false;
			}

			return true;
		}

		void print_result(const bool ok, const char* what)
		{
			auto& doc = get_scene().doc;

			if (ok)
			{
				game::printf_to_console("%s: ok ( %d selected, undo %d / redo %d )\n", what, static_cast<int>(doc.selection().size()),
					static_cast<int>(doc.undo_count()), static_cast<int>(doc.redo_count()));
			}
			else
			{
				game::printf_to_console("^1%s: %s\n", what, doc.last_error().c_str());
			}
		}

		std::string describe(const object_id id)
		{
			auto& doc = get_scene().doc;
			char text[512];

			if (const auto* e = doc.find_entity(id))
			{
				const auto o = e->origin();
				std::snprintf(text, sizeof(text), "#%u entity %s  origin %.6g %.6g %.6g  angles %s  model %s", id, std::string(e->classname()).c_str(), o.x, o.y, o.z,
					std::string(e->value("angles", "-")).c_str(), std::string(e->value("model", "-")).c_str());
			}
			else if (const auto* b = doc.find_brush(id))
			{
				const auto box = doc.object_bounds(id);
				const auto* owner = doc.find_entity(b->owner);
				std::snprintf(text, sizeof(text), "#%u brush of %s ( #%u )  %d faces  %s  mins %.6g %.6g %.6g  maxs %.6g %.6g %.6g  layer \"%s\"%s  entity origin %s", id,
					owner ? std::string(owner->classname()).c_str() : "?", b->owner, static_cast<int>(b->faces.size()),
					b->faces.empty() ? "" : b->faces[0].texture.material.c_str(), box.mins.x, box.mins.y, box.mins.z, box.maxs.x, box.maxs.y, box.maxs.z,
					std::string(doc.layer_of(id)).c_str(), b->is_detail() ? "  detail" : "",
					owner ? std::string(owner->value("origin", "-")).c_str() : "-");
			}
			else if (const auto* p = doc.find_patch(id))
			{
				std::snprintf(text, sizeof(text), "#%u patch %d x %d  %s", id, p->width, p->height, p->material.c_str());
			}
			else
			{
				std::snprintf(text, sizeof(text), "#%u ( gone )", id);
			}

			return text;
		}

		std::filesystem::path file_dialog(const bool save, const std::filesystem::path& initial)
		{
			wchar_t file[MAX_PATH * 4] = {};
			const std::wstring start = initial.filename().wstring();
			wcsncpy_s(file, start.c_str(), _TRUNCATE);

			const std::wstring dir = initial.parent_path().wstring();
			std::error_code ec;
			std::filesystem::create_directories(initial.parent_path(), ec);

			OPENFILENAMEW ofn = {};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = main_window();
			ofn.lpstrFilter = L"Map source ( *.map )\0*.map\0All files\0*.*\0";
			ofn.lpstrFile = file;
			ofn.nMaxFile = static_cast<DWORD>(std::size(file));
			ofn.lpstrInitialDir = dir.c_str();
			ofn.lpstrDefExt = L"map";
			ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);

			const BOOL ok = save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
			return ok ? std::filesystem::path(file) : std::filesystem::path();
		}

		std::filesystem::path map_source_dir()
		{
			return std::filesystem::u8path(utils::fs::get_basepath()) / "map_source";
		}
	}

	// ---- injected input ( development aid ) ----------------------------------------------------------------

	namespace
	{
		bool g_inject_mouse = false;
		bool g_inject_buttons = false;		// ui_mouse down / up / wheel since the last "off"
		ImVec2 g_inject_pos = {};

		ImGuiKey key_by_name(const std::string& name)
		{
			if (name.size() == 1 && name[0] >= 'a' && name[0] <= 'z') { return static_cast<ImGuiKey>(ImGuiKey_A + (name[0] - 'a')); }
			if (name.size() == 1 && name[0] >= '0' && name[0] <= '9') { return static_cast<ImGuiKey>(ImGuiKey_0 + (name[0] - '0')); }

			static const std::pair<const char*, ImGuiKey> names[] = {
				{ "space", ImGuiKey_Space }, { "enter", ImGuiKey_Enter }, { "escape", ImGuiKey_Escape }, { "delete", ImGuiKey_Delete },
				{ "backspace", ImGuiKey_Backspace }, { "tab", ImGuiKey_Tab }, { "left", ImGuiKey_LeftArrow }, { "right", ImGuiKey_RightArrow },
				{ "up", ImGuiKey_UpArrow }, { "down", ImGuiKey_DownArrow }, { "pageup", ImGuiKey_PageUp }, { "pagedown", ImGuiKey_PageDown },
				{ "home", ImGuiKey_Home }, { "[", ImGuiKey_LeftBracket }, { "]", ImGuiKey_RightBracket },
				{ "ctrl", ImGuiKey_ModCtrl }, { "shift", ImGuiKey_ModShift }, { "alt", ImGuiKey_ModAlt } };

			for (const auto& [text, key] : names)
			{
				if (name == text) { return key; }
			}

			return ImGuiKey_None;
		}
	}

	bool mouse_injected()
	{
		return g_inject_mouse || g_inject_buttons;
	}

	void apply_injected_input()
	{
		// the backend reports the real cursor every frame: the injected position comes after it and wins
		if (g_inject_mouse)
		{
			ImGui::GetIO().AddMousePosEvent(g_inject_pos.x, g_inject_pos.y);
		}
	}

	bool report(const bool ok, const char* action)
	{
		if (!ok)
		{
			const auto& error = get_scene().doc.last_error();
			ImGui::Toast(ImGuiToastType_Warning, action, error.empty() ? "Nothing to do." : error.c_str(), 2500);
			game::printf_to_console("%s: %s\n", action, error.empty() ? "nothing to do" : error.c_str());
		}

		return ok;
	}

	// ---- actions ---------------------------------------------------------------------------------------------

	void undo()
	{
		auto& doc = get_scene().doc;
		const std::string name(doc.undo_name());

		if (doc.undo())
		{
			game::printf_to_console("undo: %s\n", name.c_str());
		}
	}

	void redo()
	{
		auto& doc = get_scene().doc;
		const std::string name(doc.redo_name());

		if (doc.redo())
		{
			game::printf_to_console("redo: %s\n", name.c_str());
		}
	}

	void delete_selection() { report(get_scene().doc.delete_selection(), "Delete"); }
	void clone_selection() { report(get_scene().clone_selection(), "Clone"); }

	void hide_selected()
	{
		auto& doc = get_scene().doc;
		if (!doc.selection().empty())
		{
			doc.hide_selected();
		}
	}

	void show_all() { get_scene().doc.show_all(); }
	void select_all() { get_scene().doc.select_all(); }
	void invert_selection() { get_scene().doc.invert_selection(); }

	void deselect()
	{
		auto& s = get_scene();
		s.doc.deselect_all();
		s.inspecting = false;
	}

	void select_whole_entities() { get_scene().doc.select_whole_entities(); }
	void snap_to_grid() { report(get_scene().doc.snap_to_grid(get_scene().grid), "Snap to grid"); }
	void csg_subtract() { report(get_scene().csg_subtract(), "CSG subtract"); }
	void csg_hollow() { report(get_scene().doc.csg_hollow(std::max(1.0, get_scene().grid)), "CSG hollow"); }
	void set_detail(const bool detail) { report(get_scene().doc.set_detail(detail), detail ? "Make detail" : "Make structural"); }
	void move_to_worldspawn() { report(get_scene().doc.move_selection_to_worldspawn(), "Move to worldspawn"); }

	void toggle_texture_lock()
	{
		auto& doc = get_scene().doc;
		doc.set_texture_lock(!doc.texture_lock());
		game::printf_to_console("texture lock: %s\n", doc.texture_lock() ? "on" : "off");
	}

	void connect_selected()
	{
		auto& doc = get_scene().doc;
		const auto ents = doc.selected_entities();

		if (ents.size() != 2)
		{
			ImGui::Toast(ImGuiToastType_Warning, "Connect", "Select exactly two entities: the first one will target the second.", 3000);
			game::printf_to_console("Connect: select exactly two entities\n");
			return;
		}

		if (report(doc.connect_entities(ents[0], ents[1]), "Connect"))
		{
			game::printf_to_console("connected #%u -> #%u ( target \"%s\" )\n", ents[0], ents[1], std::string(doc.find_entity(ents[0])->value("target")).c_str());
		}
	}

	void grid_smaller() { auto& s = get_scene(); s.set_grid(s.grid / 2.0); }
	void grid_larger() { auto& s = get_scene(); s.set_grid(s.grid * 2.0); }

	void nudge(const int axis, const int sign)
	{
		auto& s = get_scene();
		if (s.doc.selection().empty())
		{
			return;
		}

		edit::vec3 delta;
		delta[axis] = s.grid * sign;

		// keys pressed in a row are one undo step
		s.doc.begin_transaction("Nudge", 0x6e756467ull + s.doc.selection_counter());
		s.translate(delta);
		s.doc.end_transaction();
	}

	void nudge_view(const int dx, const int dy)
	{
		auto& s = get_scene();
		const auto& st = get();

		if (st.hovered_2d >= 0)
		{
			const int axis = grid_view_axis(st.hovered_2d);
			const int h = axis == 0 ? 1 : 0, v = axis == 2 ? 1 : 2;

			if (dx) { nudge(h, dx); }
			if (dy) { nudge(v, dy); }
			return;
		}

		// 3D: the world axis closest to "right" / "ahead" on the ground
		const auto axes = s.cam.axes();
		const auto dominant = [](const float x, const float y, int& axis, int& sign)
		{
			axis = std::fabs(x) >= std::fabs(y) ? 0 : 1;
			sign = (axis == 0 ? x : y) >= 0.0f ? 1 : -1;
		};

		int axis = 0, sign = 1;

		if (dx)
		{
			dominant(-axes.left.x, -axes.left.y, axis, sign);
			nudge(axis, sign * dx);
		}

		if (dy)
		{
			dominant(axes.forward.x, axes.forward.y, axis, sign);
			nudge(axis, sign * dy);
		}
	}

	void set_clipper(const bool on)
	{
		auto& st = get();
		st.clipper.on = on;
		st.clipper.points.clear();
		st.clipper.dragging = -1;

		if (on)
		{
			st.vertex_mode = false;
		}
	}

	void set_vertex_mode(const bool on)
	{
		auto& st = get();
		st.vertex_mode = on;

		if (on)
		{
			st.clipper.on = false;
			st.clipper.points.clear();
		}
	}

	bool apply_clip(const int keep)
	{
		auto& st = get();
		auto& s = get_scene();
		auto& points = st.clipper.points;

		if (points.size() < 2)
		{
			game::printf_to_console("Clip: place two or three points first\n");
			return false;
		}

		// two points: the plane runs along the view axis of the 2D view they were placed in
		edit::vec3 third;
		if (points.size() >= 3)
		{
			third = points[2];
		}
		else
		{
			third = points[0];
			third[st.clipper.axis] += 64.0;
		}

		const auto mode = keep == 2 ? edit::clip_keep::both : (keep == 1 ? edit::clip_keep::back : edit::clip_keep::front);
		const bool ok = report(s.doc.clip(points[0], points[1], third, mode), keep == 2 ? "Split" : "Clip");

		if (ok)
		{
			points.clear();
		}

		return ok;
	}

	// ---- files -----------------------------------------------------------------------------------------------

	void open_zone(const std::string& map, const bool force)
	{
		guarded(force, [map]
		{
			std::string error;
			if (!get_scene().open_zone(map, error))
			{
				game::printf_to_console("^1[CoDOL] %s\n", error.c_str());
				ImGui::Toast(ImGuiToastType_Error, "Open map", error.c_str(), 6000);
			}
		});
	}

	void open_map_file(const std::filesystem::path& file, const bool force)
	{
		guarded(force, [file]
		{
			std::string error;
			if (!get_scene().open_map_file(file, error))
			{
				game::printf_to_console("^1[CoDOL] %s\n", error.c_str());
				ImGui::Toast(ImGuiToastType_Error, "Open .map", error.c_str(), 6000);
			}
		});
	}

	void open_map_dialog()
	{
		guarded(false, []
		{
			if (const auto file = file_dialog(false, map_source_dir() / ""); !file.empty())
			{
				open_map_file(file, true);
			}
		});
	}

	bool save()
	{
		auto& s = get_scene();
		std::string error;

		if (!s.save(error))
		{
			game::printf_to_console("^1[CoDOL] %s\n", error.c_str());
			ImGui::Toast(ImGuiToastType_Error, "Save", error.c_str(), 6000);
			return false;
		}

		game::printf_to_console("[CoDOL] saved %s\n", to_utf8(s.map_file).c_str());
		ImGui::Toast(ImGuiToastType_Success, "Saved", to_utf8(s.map_file).c_str(), 2500);
		return true;
	}

	void save_as_dialog()
	{
		auto& s = get_scene();
		if (!s.open)
		{
			return;
		}

		const auto file = file_dialog(true, s.map_file.empty() ? s.default_map_file() : s.map_file);
		if (file.empty())
		{
			return;
		}

		std::string error;
		if (!s.save_as(file, error))
		{
			game::printf_to_console("^1[CoDOL] %s\n", error.c_str());
			ImGui::Toast(ImGuiToastType_Error, "Save as", error.c_str(), 6000);
			return;
		}

		game::printf_to_console("[CoDOL] saved %s\n", to_utf8(s.map_file).c_str());
	}

	void export_entities(const bool reload)
	{
		auto& s = get_scene();
		std::filesystem::path written;
		std::string summary;

		if (!s.export_entities(written, summary))
		{
			game::printf_to_console("^1[CoDOL] %s\n", summary.c_str());
			ImGui::Toast(ImGuiToastType_Error, "Export", summary.c_str(), 6000);
			return;
		}

		game::printf_to_console("[CoDOL] %s\n", summary.c_str());
		ImGui::Toast(ImGuiToastType_Success, "Export", summary.c_str(), 5000);

		if (reload)
		{
			// rcon to the local server; it reports by itself when nobody answers
			components::codol_game::load_map(s.map_name);
		}
	}

	// Compile: apply the edits of the map to its fastfile ( codol_mapc ). The map is saved first; the compiler runs
	// as its own process so the editor stays usable, and writes <editor>\compiled\<map>.ff. Nothing is copied into
	// the game: that is the user's step.
	void compile_map()
	{
		auto& s = get_scene();

		if (!s.open || s.map_name.empty())
		{
			ImGui::Toast(ImGuiToastType_Warning, "Compile", "Open a map of the game first.");
			return;
		}

		static std::atomic_bool busy = false;
		if (busy)
		{
			ImGui::Toast(ImGuiToastType_Warning, "Compile", "A compile is still running.");
			return;
		}

		if (!save())
		{
			return;
		}

		const auto base_dir = std::filesystem::u8path(utils::fs::get_basepath());
		const auto install = std::filesystem::u8path(components::codol_game::install_path());
		const auto tool = base_dir / "codol_mapc.exe";

		// the patch folder overrides the base zones, like in the game
		std::filesystem::path base;
		std::error_code ec;

		for (const auto& dir : { install / "client_shipRetail" / "patch", install / "client_shipRetail" / "chinese" })
		{
			if (const auto candidate = dir / (s.map_name + ".ff"); std::filesystem::is_regular_file(candidate, ec))
			{
				base = candidate;
				break;
			}
		}

		const std::filesystem::path exe = install / "codoMP_client_shipRetail.exe";

		std::string problem;
		if (!std::filesystem::is_regular_file(tool, ec)) { problem = "codol_mapc.exe is not next to the editor."; }
		else if (base.empty()) { problem = "The install has no " + s.map_name + ".ff."; }
		else if (!std::filesystem::is_regular_file(exe, ec)) { problem = "The client exe was not found in the install."; }

		if (!problem.empty())
		{
			game::printf_to_console("^1[CoDOL] compile: %s\n", problem.c_str());
			ImGui::Toast(ImGuiToastType_Error, "Compile", problem.c_str(), 6000);
			return;
		}

		const auto out_dir = base_dir / "compiled";
		std::filesystem::create_directories(out_dir, ec);

		const auto out = out_dir / (s.map_name + ".ff");
		const auto report = out_dir / (s.map_name + ".report.txt");
		const auto map_file = s.map_file;
		const auto map_name = s.map_name;

		// The editor keeps the brushes of brush models ( "model" "*N" ) in world space; the zone, and so the
		// compiler, has them relative to their entity. The compiler gets a copy of the map with those brushes
		// taken back, so that only real edits count as changes.
		const auto compile_map_file = out_dir / (map_name + ".compile.map");
		{
			edit::document copy;
			std::string error;

			if (!copy.load_map_text(s.doc.save_map_text(), error))
			{
				game::printf_to_console("^1[CoDOL] compile: %s\n", error.c_str());
				return;
			}

			std::vector<object_id> ids(copy.entities().begin(), copy.entities().end());

			for (const auto id : ids)
			{
				const auto* e = copy.find_entity(id);
				if (!e || e->children.empty() || e->value("model").rfind('*', 0) != 0)
				{
					continue;
				}

				const edit::vec3 origin = e->origin(), angles = e->angles();
				if (origin == edit::vec3{} && angles == edit::vec3{})
				{
					continue;
				}

				const auto keep = [e](const char* key) { const auto* v = e->find(key); return v ? std::optional<std::string>(*v) : std::nullopt; };
				const auto old_origin = keep("origin"), old_angles = keep("angles"), old_angle = keep("angle");

				// the inverse of "rotate by the angles, then move to the origin"
				const auto forward = edit::transform::rotation_angles({}, angles);
				edit::transform back;

				for (int r = 0; r < 3; r++)
				{
					for (int c = 0; c < 3; c++)
					{
						back.m[r][c] = forward.m[c][r];
					}
				}

				copy.select(id);
				copy.transform_selection(edit::transform::concat(edit::transform::translation(origin * -1.0), back), "Place");

				const auto restore = [&](const char* key, const std::optional<std::string>& value)
				{
					if (value)
					{
						copy.set_key(id, key, *value);
					}
					else if (copy.find_entity(id)->find(key))
					{
						copy.remove_key(id, key);
					}
				};

				restore("origin", old_origin);
				restore("angles", old_angles);
				restore("angle", old_angle);
			}

			if (!copy.save_map_file(compile_map_file, error))
			{
				game::printf_to_console("^1[CoDOL] compile: %s\n", error.c_str());
				return;
			}
		}

		// the state right after the import, saved next to the map
		auto baseline = map_file;
		baseline.replace_extension(".codol_base.map");

		std::string command_line = "\"" + to_utf8(tool) + "\" --base \"" + to_utf8(base) + "\" --exe \"" + to_utf8(exe)
			+ "\" --map \"" + to_utf8(compile_map_file) + "\" --out \"" + to_utf8(out) + "\" --report \"" + to_utf8(report) + "\"";

		if (std::filesystem::is_regular_file(baseline, ec))
		{
			command_line += " --baseline \"" + to_utf8(baseline) + "\"";
		}

		// the zone of the project's assets: everything a level script can name ( animations, sounds, effects, models )
		// is carried into the map zone, with the project's animlist, animtrees and scripts as rawfiles; models the
		// map's entities use and its own zone does not have are taken from it.
		// What of the project cannot be in the compiled map is said before the compiler runs and again with its report.
		command_line += assets::compile_arguments();
		assets::begin_compile();

		for (const auto& line : assets::compile_problems())
		{
			game::printf_to_console("^1[CoDOL] compile: %s\n", line.c_str());
		}

		busy = true;
		game::printf_to_console("[CoDOL] compiling %s ..\n", map_name.c_str());
		ImGui::Toast(ImGuiToastType_Info, "Compile", ("Compiling " + map_name + " ..").c_str());

		std::thread([command_line, out, report, map_name, base_dir]
		{
			std::string output;
			const int code = components::codol_game::run_command(command_line, to_utf8(base_dir), output);

			components::codol_game::post_to_gui([code, output, out, report, map_name]
			{
				busy = false;

				if (!output.empty())
				{
					game::printf_to_console("%s\n", output.c_str());
				}

				if (code != 0)
				{
					game::printf_to_console("^1[CoDOL] compile of %s failed ( exit code %d )\n", map_name.c_str(), code);
					ImGui::Toast(ImGuiToastType_Error, "Compile", ("Compile of " + map_name + " failed, see the console.").c_str(), 8000);
					assets::compile_finished(false, output);
					return;
				}

				// every face and model that will not be in the game, and why ( console + toast + Assets window )
				assets::compile_finished(true, output);

				for (const auto& line : assets::compile_extras(out.parent_path(), map_name))
				{
					game::printf_to_console("[CoDOL] compile: %s\n", line.c_str());
				}

				game::printf_to_console("[CoDOL] compiled: %s\n[CoDOL] report: %s\n", to_utf8(out).c_str(), to_utf8(report).c_str());
				game::printf_to_console("[CoDOL] the fastfile is unsigned: a patched client ( Kernel33.dll ) or the dedicated server loads it from the install's zone folder under the map's name. Keep a copy of the original.\n");
				ImGui::Toast(ImGuiToastType_Success, "Compile", ("Compiled " + map_name + ": compiled\\" + map_name + ".ff").c_str(), 8000);
			});
		}).detach();
	}

	// A new, empty map: written to map_source\<name>.map and opened. Such a map has no zone of the game behind
	// it; "Build map" compiles it from nothing.
	void new_map(const std::string& name, const bool force)
	{
		std::string clean;
		for (const char c : name)
		{
			if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
			{
				clean.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
			}
		}

		if (clean.empty())
		{
			ImGui::Toast(ImGuiToastType_Warning, "New map", "Give the map a name: letters, digits and _ ( mp_mymap ).");
			return;
		}

		const auto file = std::filesystem::u8path(utils::fs::get_basepath()) / "map_source" / (clean + ".map");
		std::error_code ec;

		if (std::filesystem::exists(file, ec))
		{
			ImGui::Toast(ImGuiToastType_Warning, "New map", ("map_source\\" + clean + ".map exists already: open it, or pick another name.").c_str(), 6000);
			return;
		}

		guarded(force, [file]
		{
			edit::document fresh;
			std::string error;
			std::error_code dir_error;
			std::filesystem::create_directories(file.parent_path(), dir_error);

			if (!fresh.save_map_file(file, error) || !get_scene().open_map_file(file, error))
			{
				game::printf_to_console("^1[CoDOL] new map: %s\n", error.c_str());
				ImGui::Toast(ImGuiToastType_Error, "New map", error.c_str(), 6000);
				return;
			}

			game::printf_to_console("[CoDOL] new map %s\n", to_utf8(file).c_str());
		});
	}

	// ---- what the game's scripts ask of every multiplayer map --------------------------------------------------

	namespace
	{
		struct gameplay_state
		{
			int spawns = 0;
			bool intermission = false;
			bool care_package = false;		// _airdrop.gsc: script error at every start without it
			bool heli = false;				// _helicopter.gsc: heli_start and heli_loop_start
			edit::bounds world;
			edit::vec3 first_spawn;
		};

		gameplay_state inspect_gameplay(const edit::document& doc)
		{
			gameplay_state g;
			bool heli_start = false, heli_loop = false;

			for (const object_id id : doc.entities())
			{
				const auto* e = doc.find_entity(id);
				if (!e) { continue; }

				const auto cls = e->classname();
				const auto name = e->value("targetname");

				if (e->is_worldspawn())
				{
					for (const object_id child : e->children)
					{
						const auto b = doc.object_bounds(child);
						if (b.valid()) { g.world.add(b.mins); g.world.add(b.maxs); }
					}
				}

				if (cls.starts_with("mp_") && cls.find("spawn") != std::string_view::npos)
				{
					if (!g.spawns) { g.first_spawn = e->origin(); }
					g.spawns++;
				}

				g.intermission |= cls == "mp_global_intermission";
				g.care_package |= name == "care_package";
				heli_start |= name == "heli_start";
				heli_loop |= name == "heli_loop_start";
			}

			g.heli = heli_start && heli_loop;
			return g;
		}

		std::vector<std::string> gameplay_problems(const gameplay_state& g)
		{
			std::vector<std::string> out;
			if (!g.spawns) { out.emplace_back("no spawn points ( mp_dm_spawn, mp_tdm_spawn .. ): nobody can spawn"); }
			if (!g.intermission) { out.emplace_back("no mp_global_intermission: the camera of the end of a match has no place"); }
			if (!g.care_package) { out.emplace_back("no care package ( script_model \"care_package\" that targets a script_brushmodel ): script error at every map start"); }
			if (!g.heli) { out.emplace_back("no helicopter path ( script_origin heli_start, heli_loop_start ): air killstreaks run into script errors"); }
			return out;
		}
	}

	// Adds what is missing of the care package and the helicopter path. One undo step.
	void add_gameplay_entities()
	{
		auto& s = get_scene();
		if (!s.open) { return; }

		auto& doc = s.doc;
		const auto g = inspect_gameplay(doc);

		if (g.care_package && g.heli)
		{
			ImGui::Toast(ImGuiToastType_Info, "Gameplay entities", "The map has a care package and a helicopter path already.");
			return;
		}

		using pairs = std::vector<std::pair<std::string, std::string>>;
		const bool has_world = g.world.valid();
		const edit::vec3 centre = has_world ? edit::vec3{ (g.world.mins.x + g.world.maxs.x) * 0.5, (g.world.mins.y + g.world.maxs.y) * 0.5, (g.world.mins.z + g.world.maxs.z) * 0.5 } : edit::vec3{};

		doc.begin_transaction("Add gameplay entities");
		doc.deselect_all();
		int added = 0;

		if (!g.care_package)
		{
			// beside the first spawn point; the script takes the crate and its collision away when the match starts
			const edit::vec3 at = g.spawns ? edit::vec3{ g.first_spawn.x + 64.0, g.first_spawn.y, g.first_spawn.z } : edit::vec3{ centre.x, centre.y, has_world ? g.world.mins.z + 16.0 : 0.0 };
			const object_id box = doc.create_box({ at.x - 29, at.y - 15, at.z }, { at.x + 29, at.y + 15, at.z + 30 }, "clip");

			if (box)
			{
				doc.deselect_all();
				doc.select(box);
				const pairs collision = { { "targetname", "airdrop_collision" }, { "script_gameobjectname", "airdrop_pallet" } };
				doc.create_brush_entity("script_brushmodel", collision);
				doc.deselect_all();
				const pairs crate = { { "model", "com_plasticcase_green_big_us_dirt" }, { "targetname", "care_package" }, { "target", "airdrop_collision" } };
				doc.create_point_entity("script_model", at, crate);
				added++;
			}
		}

		if (!g.heli)
		{
			// a way in, a closed loop, a way out and a crash path, all inside the map's box near its top
			const double reach = has_world ? std::max(64.0, 0.4 * std::min(g.world.maxs.x - g.world.mins.x, g.world.maxs.y - g.world.mins.y) * 0.5) : 512.0;
			const double z = has_world ? g.world.maxs.z - std::min(64.0, (g.world.maxs.z - g.world.mins.z) * 0.25) : 512.0;
			const double low = has_world ? g.world.mins.z + (g.world.maxs.z - g.world.mins.z) * 0.25 : 128.0;

			const auto node = [&](const double x, const double y, const double height, const char* name, const char* target)
			{
				pairs keys = { { "targetname", name } };
				if (target) { keys.emplace_back("target", target); }
				doc.create_point_entity("script_origin", { centre.x + x, centre.y + y, height }, keys);
			};

			node(-reach, -reach, z, "heli_start", "heli_dest_1");
			node(0, 0, z, "heli_dest_1", nullptr);
			node(reach, -reach, z, "heli_loop_start", "heli_loop_1");
			node(reach * 0.6, -reach * 0.6, z, "heli_loop_1", "heli_loop_2");
			node(reach * 0.6, reach * 0.6, z, "heli_loop_2", "heli_loop_3");
			node(-reach * 0.6, reach * 0.6, z, "heli_loop_3", "heli_loop_4");
			node(-reach * 0.6, -reach * 0.6, z, "heli_loop_4", "heli_loop_1");
			node(-reach, reach, z, "heli_leave", nullptr);
			node(reach, reach, z, "heli_crash_start", "heli_crash_1");
			node(reach * 0.5, reach * 0.3, low, "heli_crash_1", nullptr);
			added++;
		}

		doc.deselect_all();
		doc.end_transaction();

		game::printf_to_console("[CoDOL] gameplay entities: %s%s added\n", !g.care_package ? "care package " : "", !g.heli ? "helicopter path " : "");
		ImGui::Toast(ImGuiToastType_Success, "Gameplay entities", added ? "Added what the game's scripts ask of every map." : "Nothing added.");
	}

	// Build: the whole map zone from the .map alone ( codol_bsp ): collision, BSP, cells and portals, render
	// geometry, light bake, and its load zone. For maps of your own; a map imported from the game only holds that
	// map's collision brushes, so use "Compile map to a fastfile" for edits of a shipped map.
	void build_map(const bool fast)
	{
		auto& s = get_scene();

		if (!s.open)
		{
			ImGui::Toast(ImGuiToastType_Warning, "Build", "Open or create a map first.");
			return;
		}

		static std::atomic_bool busy = false;
		if (busy)
		{
			ImGui::Toast(ImGuiToastType_Warning, "Build", "A build is still running.");
			return;
		}

		if (!save())
		{
			return;
		}

		const auto base_dir = std::filesystem::u8path(utils::fs::get_basepath());
		const auto install = std::filesystem::u8path(components::codol_game::install_path());
		const auto tool = base_dir / "codol_bsp.exe";
		std::error_code ec;

		const std::filesystem::path exe = install / "codoMP_client_shipRetail.exe";

		std::string problem;
		if (!std::filesystem::is_regular_file(tool, ec)) { problem = "codol_bsp.exe is not next to the editor."; }
		else if (!std::filesystem::is_regular_file(exe, ec)) { problem = "The client exe was not found in the install ( File > Settings )."; }

		if (!problem.empty())
		{
			game::printf_to_console("^1[CoDOL] build: %s\n", problem.c_str());
			ImGui::Toast(ImGuiToastType_Error, "Build", problem.c_str(), 6000);
			return;
		}

		// what the scripts will miss: said before the build, it does not stop it
		{
			const auto g = inspect_gameplay(s.doc);

			for (const auto& line : gameplay_problems(g))
			{
				game::printf_to_console("^3[CoDOL] build: %s\n", line.c_str());
			}

			if (!g.care_package || !g.heli)
			{
				game::printf_to_console("^3[CoDOL] build: Edit > Add gameplay entities puts the care package and the helicopter path in\n");
			}

			for (const auto& line : assets::package_problems())
			{
				game::printf_to_console("^3[CoDOL] build: %s\n", line.c_str());
			}
		}

		const auto map_file = build_source_file(s.map_file);
		const std::string name = to_utf8(map_file.stem());
		const auto out_dir = base_dir / "compiled";
		std::filesystem::create_directories(out_dir / "cache", ec);

		// The whole package ( tools\package_map.ps1 ): map zone with its minimap, load zone, level scripts, the files
		// of the create-room list and INSTALL.txt, in compiled\<map>\. Without the script: the two zones alone.
		const auto script = base_dir / "tools" / "package_map.ps1";
		const auto assetc = base_dir / "codol_assetc.exe";
		const bool package = std::filesystem::is_regular_file(script, ec) && std::filesystem::is_regular_file(assetc, ec);
		const auto assets_zone = assets::zone_file();
		const bool with_assets = std::filesystem::is_regular_file(assets_zone, ec);

		std::filesystem::path out, report;
		std::string command_line;

		if (package)
		{
			const auto package_dir = out_dir / name;
			out = package_dir / "zone" / (name + ".ff");
			report = package_dir / "work" / (name + ".report.txt");

			command_line = "powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" + to_utf8(script) + "\" -Map \"" + to_utf8(map_file) + "\" -Name " + name
				+ " -Out \"" + to_utf8(package_dir) + "\" -Game \"" + to_utf8(install) + "\" -Cache \"" + to_utf8(out_dir / "cache") + "\" -Bsp \"" + to_utf8(tool)
				+ "\" -Assetc \"" + to_utf8(assetc) + "\"";

			if (with_assets) { command_line += " -Assets \"" + to_utf8(assets_zone) + "\""; }
			command_line += assets::package_arguments();
			if (const auto picture = map_file.parent_path() / (name + "_load.png"); std::filesystem::is_regular_file(picture, ec)) { command_line += " -Loadscreen \"" + to_utf8(picture) + "\""; }
			if (fast) { command_line += " -Fast"; }
		}
		else
		{
			out = out_dir / (name + ".ff");
			report = out_dir / (name + ".build.txt");

			command_line = "\"" + to_utf8(tool) + "\" --map \"" + to_utf8(map_file) + "\" --exe \"" + to_utf8(exe) + "\" --install \"" + to_utf8(install)
				+ "\" --out \"" + to_utf8(out) + "\" --name " + name + " --report \"" + to_utf8(report) + "\" --cache \"" + to_utf8(out_dir / "cache") + "\"";

			if (with_assets) { command_line += " --assets \"" + to_utf8(assets_zone) + "\""; }
			if (fast) { command_line += " --fast"; }
		}

		busy = true;
		game::printf_to_console("[CoDOL] building %s ..\n", name.c_str());
		ImGui::Toast(ImGuiToastType_Info, "Build", ("Building " + name + " ..").c_str());

		std::thread([command_line, out, report, name, base_dir, package]
		{
			std::string output;
			const int code = components::codol_game::run_command(command_line, to_utf8(base_dir), output);

			components::codol_game::post_to_gui([code, output, out, report, name, package]
			{
				busy = false;

				if (!output.empty())
				{
					game::printf_to_console("%s\n", output.c_str());
				}

				if (code != 0)
				{
					game::printf_to_console("^1[CoDOL] build of %s failed ( exit code %d )\n", name.c_str(), code);
					ImGui::Toast(ImGuiToastType_Error, "Build", ("Build of " + name + " failed, see the console.").c_str(), 8000);
					return;
				}

				game::printf_to_console("[CoDOL] built: %s ( and its _load zone )\n[CoDOL] report: %s\n", to_utf8(out).c_str(), to_utf8(report).c_str());

				if (package)
				{
					game::printf_to_console("[CoDOL] package: compiled\\%s\\ - zones, level scripts, create-room list files; presence\\INSTALL.txt says where each file goes\n", name.c_str());
				}
				else
				{
					game::printf_to_console("[CoDOL] to play it: copy both .ff files into the game's usermaps\\%s\\ folder and start the lobby with CODOL_FORCE_MAP=%s\n", name.c_str(), name.c_str());
				}

				ImGui::Toast(ImGuiToastType_Success, "Build", ("Built " + name + ": " + to_utf8(out)).c_str(), 8000);
			});
		}).detach();
	}

	void close_map(const bool force)
	{
		guarded(force, [] { get_scene().close(); });
	}

	void exit_editor()
	{
		guarded(false, [] { request_exit(); });
	}

	void draw_modals()
	{
		if (g_ask)
		{
			g_ask = false;
			ImGui::OpenPopup("Unsaved changes##modal");
		}

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Unsaved changes##modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			auto& s = get_scene();
			ImGui::Text("%s has changes that are not saved.", s.map_name.c_str());
			ImGui::Spacing();

			const auto finish = [](const bool run)
			{
				auto action = std::move(g_pending);
				g_pending = nullptr;
				ImGui::CloseCurrentPopup();

				if (run && action)
				{
					action();
				}
			};

			if (ImGui::Button("Save", ImVec2(110.0f, 0.0f)))
			{
				finish(save());
			}

			ImGui::SameLine();
			if (ImGui::Button("Discard", ImVec2(110.0f, 0.0f)))
			{
				finish(true);
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
			{
				finish(false);
			}

			ImGui::EndPopup();
		}
	}

	// ---- menus -----------------------------------------------------------------------------------------------

	void draw_file_menu_items()
	{
		auto& s = get_scene();

		if (editor::menu::begin("New map"))
		{
			static std::string name = "mp_mymap";
			ImGui::SetNextItemWidth(220.0f);
			const bool enter = ImGui::InputText("##new_map_name", &name, ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::SameLine();

			if (ImGui::Button("Create") || enter)
			{
				new_map(name, false);
				ImGui::CloseCurrentPopup();
			}

			ImGui::TextDisabled("saved as map_source\\<name>.map");
			editor::menu::end();
		}

		if (editor::menu::item("Open map ..", editor::menu::key("file.open")))
		{
			GET_GUI(ggui::codol_map_ents_dialog)->open();
			ImGui::SetWindowFocus("Maps##window");
		}

		if (editor::menu::item("Open .map .."))
		{
			open_map_dialog();
		}

		if (editor::menu::item("Save", editor::menu::key("file.save"), false, s.open))
		{
			save();
		}

		if (editor::menu::item("Save as ..", nullptr, false, s.open))
		{
			save_as_dialog();
		}

		if (editor::menu::item("Close map", nullptr, false, s.open))
		{
			close_map();
		}

		ImGui::Separator();

		auto* maps = GET_GUI(ggui::codol_map_ents_dialog);

		if (editor::menu::item("Export entities to the server", editor::menu::key("file.export"), false, s.open))
		{
			export_entities(maps->m_reload_after_export);
		}

		editor::menu::item("Load the map on the server after the export", nullptr, &maps->m_reload_after_export);

		ImGui::Separator();

		if (editor::menu::item("Build map ( from scratch )", nullptr, false, s.open))
		{
			build_map(false);
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("For maps of your own: compiles the whole map zone from the .map alone - collision, BSP, cells and portals,\nrender geometry and a light bake - into compiled\\<map>.ff and its load zone.");
		}

		if (editor::menu::item("Build map, fast ( no light bake quality )", nullptr, false, s.open))
		{
			build_map(true);
		}

		if (editor::menu::item("Install the built map ..", nullptr, false, s.open))
		{
			open_install_window();
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Where the package of the last build goes: the zones to <game>\\usermaps\\<map>\\, the level scripts to the server's script\n"
				"folder, and - when asked for - the map into the create-room list. Replaced files are kept as backups; Uninstall undoes it.");
		}

		if (editor::menu::item("Compile map to a fastfile", nullptr, false, s.open))
		{
			compile_map();
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Saves the map and applies its edits ( entities, static models, collision brushes, new brush faces )\nto the map's fastfile. The result is written to compiled\\<map>.ff next to the editor.");
		}
	}

	void draw_edit_menu()
	{
		auto& s = get_scene();
		auto& doc = s.doc;
		auto& st = get();
		const bool open = s.open;
		const bool any = open && !doc.selection().empty();

		const std::string undo_label = open && doc.can_undo() ? "Undo " + std::string(doc.undo_name()) : "Undo";
		const std::string redo_label = open && doc.can_redo() ? "Redo " + std::string(doc.redo_name()) : "Redo";

		if (editor::menu::item(undo_label.c_str(), editor::menu::key("edit.undo"), false, open && doc.can_undo())) { undo(); }
		if (editor::menu::item(redo_label.c_str(), editor::menu::key("edit.redo"), false, open && doc.can_redo())) { redo(); }
		ImGui::Separator();

		if (editor::menu::item("Add gameplay entities", nullptr, false, open)) { add_gameplay_entities(); }
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Adds what the game's scripts ask of every multiplayer map and the map does not have yet:\na care package with its collision ( airdrop ) and a helicopter path ( script_origins ).");
		}

		ImGui::Separator();

		if (editor::menu::item("Delete", editor::menu::key("edit.delete"), false, any)) { delete_selection(); }
		if (editor::menu::item("Clone", editor::menu::key("edit.clone"), false, any)) { clone_selection(); }
		if (editor::menu::item("Hide selected", editor::menu::key("view.hide_selected"), false, any)) { hide_selected(); }
		if (editor::menu::item("Show all", editor::menu::key("view.show_all"), false, open)) { show_all(); }
		ImGui::Separator();

		if (editor::menu::item("Select all", editor::menu::key("select.all"), false, open)) { select_all(); }
		if (editor::menu::item("Invert selection", editor::menu::key("select.invert"), false, open)) { invert_selection(); }
		if (editor::menu::item("Select whole entities", editor::menu::key("select.whole_entities"), false, any)) { select_whole_entities(); }
		if (editor::menu::item("Deselect", editor::menu::key("edit.escape"), false, any || s.inspecting)) { deselect(); }
		ImGui::Separator();

		if (editor::menu::item("Snap to grid", editor::menu::key("edit.snap_to_grid"), false, any)) { snap_to_grid(); }
		if (editor::menu::item("CSG subtract", nullptr, false, any)) { csg_subtract(); }
		if (editor::menu::item("CSG hollow", nullptr, false, any)) { csg_hollow(); }
		if (editor::menu::item("Make detail", editor::menu::key("brush.make_detail"), false, any)) { set_detail(true); }
		if (editor::menu::item("Make structural", editor::menu::key("brush.make_structural"), false, any)) { set_detail(false); }
		if (editor::menu::item("Connect entities", editor::menu::key("entity.connect"), false, any)) { connect_selected(); }
		if (editor::menu::item("Move brushes to the worldspawn", nullptr, false, any)) { move_to_worldspawn(); }
		ImGui::Separator();

		bool lock = doc.texture_lock();
		if (editor::menu::item("Texture lock", editor::menu::key("texture.lock_toggle"), &lock, open)) { toggle_texture_lock(); }

		bool clipper = st.clipper.on;
		if (editor::menu::item("Clipper ( 2D views )", editor::menu::key("tool.clipper"), &clipper, open)) { set_clipper(clipper); }

		bool vertex = st.vertex_mode;
		if (editor::menu::item("Vertex / edge mode ( 2D views )", editor::menu::key("tool.vertex"), &vertex, open)) { set_vertex_mode(vertex); }
	}

	// ---- shortcuts -------------------------------------------------------------------------------------------

	void process_shortcuts()
	{
		auto& st = get();

		st.hovered_2d = st.next_hovered_2d;
		st.hovered_3d = st.next_hovered_3d;
		st.next_hovered_2d = -1;
		st.next_hovered_3d = false;

		// Every key is an action of the registry ( hotkeys.cpp: actions::register_defaults ), so that it can be given
		// another key in Edit > Edit Hotkeys. A new shortcut is one line there:
		//   actions::add( "edit.clone", "Clone", editor::menu::key("edit.clone"), [] { clone_selection(); }, actions::views );
		actions::process_keys();
	}

	// ---- console commands ------------------------------------------------------------------------------------

	void register_commands()
	{
		using args_t = const std::vector<std::string>&;

		// ---- files
		command::register_command("map_open", [](args_t args)
		{
			if (args.size() < 2) { game::printf_to_console("usage: map_open <zone>   eg. map_open mp_rust\n"); return; }
			open_zone(args[1], true);
		});

		command::register_command("map_load", [](args_t args)
		{
			if (args.size() < 2) { game::printf_to_console("usage: map_load <file.map>\n"); return; }
			open_map_file(std::filesystem::u8path(args[1]), true);
		});

		command::register_command("map_save", [](args_t args)
		{
			if (!need_map()) { return; }

			if (args.size() > 1)
			{
				std::string error;
				if (!get_scene().save_as(std::filesystem::u8path(args[1]), error)) { game::printf_to_console("^1%s\n", error.c_str()); return; }
				game::printf_to_console("[CoDOL] saved %s\n", args[1].c_str());
			}
			else
			{
				save();
			}
		});

		command::register_command("map_export", [](args_t args)
		{
			if (need_map()) { export_entities(args.size() > 1 && args[1] == "reload"); }
		});

		command::register_command("map_compile", [](args_t)
		{
			if (need_map()) { compile_map(); }
		});

		command::register_command("map_new", [](args_t args)
		{
			if (args.size() > 1) { new_map(args[1], true); } else { game::printf_to_console("usage: map_new <name>\n"); }
		});

		command::register_command("map_build", [](args_t args)
		{
			if (need_map()) { build_map(args.size() > 1 && args[1] == "fast"); }
		});

		command::register_command("ed_gameplay", [](args_t)
		{
			if (!need_map()) { return; }
			add_gameplay_entities();
			for (const auto& line : gameplay_problems(inspect_gameplay(get_scene().doc))) { game::printf_to_console("^3still missing: %s\n", line.c_str()); }
		});

		// map_install [zone folder] [script folder]
		// map_install [zone folder] [script folder] [game=<folder>] [scripts=<folder>] [zones=<folder>] [list]
		//   without arguments: the folders of File > Settings. game=: zones to <folder>\usermaps\<map>, the sound archive
		//   to <folder>\main; with "list" the map is added to the create-room list of that game folder.
		command::register_command("map_install", [](args_t args)
		{
			if (!need_map()) { return; }

			// map_install window: the window of File > Install the built map ..
			if (args.size() > 1 && args[1] == "window") { open_install_window(); return; }

			install_request request;
			bool keywords = false;
			std::vector<std::string> plain;

			for (std::size_t i = 1; i < args.size(); i++)
			{
				const std::string& a = args[i];
				if (a.rfind("game=", 0) == 0) { request.game = a.substr(5); keywords = true; }
				else if (a.rfind("scripts=", 0) == 0) { request.script_dir = a.substr(8); keywords = true; }
				else if (a.rfind("zones=", 0) == 0) { request.zone_dir = a.substr(6); keywords = true; }
				else if (a == "list") { request.list = true; keywords = true; }
				else { plain.push_back(a); }
			}

			if (!keywords)
			{
				install_map(plain.size() > 0 ? plain[0] : std::string(), plain.size() > 1 ? plain[1] : std::string());
				return;
			}

			if (!plain.empty() && request.zone_dir.empty()) { request.zone_dir = plain[0]; }
			if (plain.size() > 1 && request.script_dir.empty()) { request.script_dir = plain[1]; }
			install_map(request);
		});

		// ed_restage: a map of your own - the stage of the 3D view again, with the light keys the worldspawn has now
		// ( the "Show in 3D" button of the Entity window )
		command::register_command("ed_restage", [](args_t)
		{
			if (!need_map()) { return; }

			std::string error;
			if (get_scene().restage(error)) { game::printf_to_console("[CoDOL] 3D view: the stage has the map's light keys\n"); }
			else { game::printf_to_console("^1[CoDOL] 3D view: %s\n", error.c_str()); }
		});

		// map_list [game folder]: the create-room list of a game folder ( read only )
		command::register_command("map_list", [](args_t args)
		{
			print_map_list(args.size() > 1 ? args[1] : std::string());
		});

		// map_uninstall [map]: undoes what map_install wrote ( compiled\<map>\work\installed.txt )
		command::register_command("map_uninstall", [](args_t args)
		{
			uninstall_map(args.size() > 1 ? args[1] : std::string());
		});

		command::register_command("map_close", [](args_t) { close_map(true); });
		command::register_command("quit", [](args_t) { request_exit(); });

		// ---- selection
		command::register_command("sel_id", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			auto& doc = get_scene().doc;
			const auto id = static_cast<object_id>(std::atoi(args[1].c_str()));

			if (doc.kind(id) == edit::object_kind::none) { game::printf_to_console("^1no object #%u\n", id); return; }
			doc.select(id, args.size() > 2 ? edit::select_mode::add : edit::select_mode::replace);
			game::printf_to_console("%s\n", describe(id).c_str());
		});

		// sel_class <classname> [n]: every entity of the class, or the n-th
		command::register_command("sel_class", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			auto& doc = get_scene().doc;
			std::vector<object_id> ids;

			for (const object_id id : doc.entities())
			{
				if (doc.find_entity(id)->classname() == args[1]) { ids.push_back(id); }
			}

			if (args.size() > 2)
			{
				const auto n = static_cast<std::size_t>(std::atoi(args[2].c_str()));
				ids = n < ids.size() ? std::vector<object_id>{ ids[n] } : std::vector<object_id>{};
			}

			doc.select(std::span<const object_id>(ids), args.size() > 3 ? edit::select_mode::add : edit::select_mode::replace);
			game::printf_to_console("%d entities of class %s, %d objects selected\n", static_cast<int>(ids.size()), args[1].c_str(), static_cast<int>(doc.selection().size()));
		});

		// sel_codol <codol_id>: the entity with that index in the entity string of the zone
		command::register_command("sel_codol", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			auto& doc = get_scene().doc;

			for (const object_id id : doc.entities())
			{
				if (doc.find_entity(id)->value("codol_id") == args[1])
				{
					doc.select(id, args.size() > 2 ? edit::select_mode::add : edit::select_mode::replace);
					game::printf_to_console("%s\n", describe(id).c_str());
					return;
				}
			}

			game::printf_to_console("^1no entity with codol_id %s\n", args[1].c_str());
		});

		// sel_brush <n> [entity id]: the n-th brush of the worldspawn ( or of an entity )
		command::register_command("sel_brush", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			auto& doc = get_scene().doc;
			const auto* e = doc.find_entity(args.size() > 2 ? static_cast<object_id>(std::atoi(args[2].c_str())) : doc.worldspawn());
			const auto n = static_cast<std::size_t>(std::atoi(args[1].c_str()));

			if (!e || n >= e->children.size()) { game::printf_to_console("^1no such brush\n"); return; }
			doc.select(e->children[n], args.size() > 3 ? edit::select_mode::add : edit::select_mode::replace);
			game::printf_to_console("%s\n", describe(e->children[n]).c_str());
		});

		command::register_command("sel_face", [](args_t args)
		{
			if (!need_map() || args.size() < 3) { return; }
			get_scene().doc.select_face({ static_cast<object_id>(std::atoi(args[1].c_str())), std::atoi(args[2].c_str()) },
				args.size() > 3 ? edit::select_mode::add : edit::select_mode::replace);
		});

		// sel_rect <axis x|y|z> <lo a> <lo b> <hi a> <hi b> [inside]: the drag rectangle of a 2D view
		command::register_command("sel_rect", [](args_t args)
		{
			if (!need_map() || args.size() < 6) { return; }
			auto& doc = get_scene().doc;
			const int axis = args[1] == "x" ? 0 : (args[1] == "y" ? 1 : 2);
			const auto ids = doc.query_rect(axis, { number(args, 2), number(args, 3) }, { number(args, 4), number(args, 5) }, args.size() > 6);
			doc.select(std::span<const object_id>(ids), edit::select_mode::replace);
			game::printf_to_console("%d objects in the rectangle\n", static_cast<int>(ids.size()));
		});

		command::register_command("sel_all", [](args_t) { if (need_map()) { select_all(); print_result(true, "select all"); } });
		command::register_command("sel_none", [](args_t) { if (need_map()) { deselect(); } });
		command::register_command("sel_invert", [](args_t) { if (need_map()) { invert_selection(); print_result(true, "invert"); } });
		command::register_command("sel_entity", [](args_t) { if (need_map()) { select_whole_entities(); print_result(true, "whole entities"); } });

		command::register_command("sel_info", [](args_t)
		{
			if (!need_map()) { return; }
			auto& s = get_scene();
			auto& doc = s.doc;
			const auto sel = doc.selection();

			game::printf_to_console("%s: %d entities, %d brushes, %d patches; %d selected, %d faces; modified %d; undo %d ( %s ), redo %d ( %s ); grid %g\n",
				s.map_name.c_str(), static_cast<int>(doc.entities().size()), static_cast<int>(doc.brush_count()), static_cast<int>(doc.patch_count()),
				static_cast<int>(sel.size()), static_cast<int>(doc.selected_faces().size()), doc.modified() ? 1 : 0,
				static_cast<int>(doc.undo_count()), std::string(doc.undo_name()).c_str(), static_cast<int>(doc.redo_count()), std::string(doc.redo_name()).c_str(), s.grid);

			for (std::size_t i = 0; i < sel.size() && i < 8; i++)
			{
				game::printf_to_console("  %s\n", describe(sel[i]).c_str());
			}

			if (const auto box = doc.selection_bounds(); box.valid())
			{
				game::printf_to_console("  bounds %.6g %.6g %.6g .. %.6g %.6g %.6g\n", box.mins.x, box.mins.y, box.mins.z, box.maxs.x, box.maxs.y, box.maxs.z);
			}

			const auto stats = s.renderer.stats();
			game::printf_to_console("  renderer: %d hidden, %d moved, %d placed models, %d meshes\n", stats.hidden_objects, stats.moved_objects, stats.placed_model_count, stats.mesh_count);
		});

		// ed_pick [yaw offset] [pitch offset] [toggle] [face]: click on what the camera looks at
		command::register_command("ed_pick", [](args_t args)
		{
			if (!need_map()) { return; }
			auto& s = get_scene();
			auto c = s.cam;
			c.yaw += static_cast<float>(number(args, 1));
			c.pitch += static_cast<float>(number(args, 2));
			pick_ray(c.origin, c.axes().forward, args.size() > 3 && args[3] != "0", args.size() > 4 && args[4] != "0");

			for (const object_id id : s.doc.selection())
			{
				game::printf_to_console("picked %s\n", describe(id).c_str());
			}

			if (s.inspecting)
			{
				game::printf_to_console("looking at a world surface ( %s ), not a document object\n", s.inspected.material.c_str());
			}
		});

		// ed_click <px> <py> [toggle] [face]: a left click at a pixel of the 3D view
		command::register_command("ed_click", [](args_t args)
		{
			if (need_map() && args.size() > 2)
			{
				viewport_click(static_cast<float>(number(args, 1)), static_cast<float>(number(args, 2)), args.size() > 3 && args[3] != "0", args.size() > 4 && args[4] != "0");
			}
		});

		// ---- editing
		command::register_command("ed_undo", [](args_t args)
		{
			if (!need_map()) { return; }
			for (int i = std::max(1, static_cast<int>(number(args, 1, 1))); i > 0 && get_scene().doc.can_undo(); i--) { undo(); }
		});

		command::register_command("ed_redo", [](args_t args)
		{
			if (!need_map()) { return; }
			for (int i = std::max(1, static_cast<int>(number(args, 1, 1))); i > 0 && get_scene().doc.can_redo(); i--) { redo(); }
		});

		command::register_command("ed_move", [](args_t args)
		{
			if (need_map() && args.size() > 3) { print_result(get_scene().translate(vector(args, 1)), "move"); }
		});

		// ed_rotate <x|y|z> <degrees>: about the centre of the selection
		command::register_command("ed_rotate", [](args_t args)
		{
			if (!need_map() || args.size() < 3) { return; }
			auto& s = get_scene();
			edit::vec3 axis;
			axis[args[1] == "x" ? 0 : (args[1] == "y" ? 1 : 2)] = 1.0;
			print_result(s.rotate(s.selection_pivot(), axis, number(args, 2)), "rotate");
		});

		command::register_command("ed_scale", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			auto& s = get_scene();
			const double f = number(args, 1, 1.0);
			print_result(s.scale(s.selection_pivot(), { f, number(args, 2, f), number(args, 3, f) }), "scale");
		});

		command::register_command("ed_delete", [](args_t) { if (need_map()) { print_result(get_scene().doc.delete_selection(), "delete"); } });
		command::register_command("ed_clone", [](args_t) { if (need_map()) { print_result(get_scene().clone_selection(), "clone"); } });
		command::register_command("ed_hide", [](args_t) { if (need_map()) { hide_selected(); } });
		command::register_command("ed_showall", [](args_t) { if (need_map()) { show_all(); } });
		command::register_command("ed_snap", [](args_t) { if (need_map()) { print_result(get_scene().doc.snap_to_grid(get_scene().grid), "snap"); } });
		command::register_command("ed_subtract", [](args_t) { if (need_map()) { print_result(get_scene().csg_subtract(), "csg subtract"); } });
		command::register_command("ed_worldspawn", [](args_t) { if (need_map()) { print_result(get_scene().doc.move_selection_to_worldspawn(), "to worldspawn"); } });
		command::register_command("ed_connect", [](args_t) { if (need_map()) { connect_selected(); } });

		command::register_command("ed_hollow", [](args_t args)
		{
			if (need_map()) { print_result(get_scene().doc.csg_hollow(number(args, 1, std::max(1.0, get_scene().grid))), "csg hollow"); }
		});

		command::register_command("ed_detail", [](args_t args)
		{
			if (need_map()) { print_result(get_scene().doc.set_detail(number(args, 1, 1.0) != 0.0), "detail"); }
		});

		command::register_command("ed_texlock", [](args_t args)
		{
			auto& doc = get_scene().doc;
			doc.set_texture_lock(args.size() > 1 ? number(args, 1) != 0.0 : !doc.texture_lock());
			game::printf_to_console("texture lock: %s\n", doc.texture_lock() ? "on" : "off");
		});

		command::register_command("ed_grid", [](args_t args)
		{
			auto& s = get_scene();
			if (args.size() > 1)
			{
				if (args[1] == "+") { grid_larger(); }
				else if (args[1] == "-") { grid_smaller(); }
				else { s.set_grid(number(args, 1, 8.0)); }
			}

			if (args.size() > 2) { s.snap = number(args, 2) != 0.0; }
			game::printf_to_console("grid %g, snap %s\n", s.grid, s.snap ? "on" : "off");
		});

		// ed_nudge <left|right|up|down|in|out>: what the arrow keys do in the 3D view
		command::register_command("ed_nudge", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			if (args[1] == "left") { nudge_view(-1, 0); }
			else if (args[1] == "right") { nudge_view(1, 0); }
			else if (args[1] == "up") { nudge_view(0, 1); }
			else if (args[1] == "down") { nudge_view(0, -1); }
			else if (args[1] == "in") { nudge(2, 1); }
			else if (args[1] == "out") { nudge(2, -1); }
			print_result(true, "nudge");
		});

		// ed_box x1 y1 z1 x2 y2 z2 [material]
		command::register_command("ed_box", [](args_t args)
		{
			if (!need_map() || args.size() < 7) { game::printf_to_console("usage: ed_box x1 y1 z1 x2 y2 z2 [material]\n"); return; }
			auto& doc = get_scene().doc;
			const auto id = doc.create_box(vector(args, 1), vector(args, 4), args.size() > 7 ? std::string_view(args[7]) : std::string_view());

			if (id) { game::printf_to_console("created %s\n", describe(id).c_str()); }
			else { game::printf_to_console("^1%s\n", doc.last_error().c_str()); }
		});

		// ed_entity <classname> [x y z]: a point entity ( at the position, else where the camera looks ), or a
		// brush entity around the selected brushes when the class is one
		command::register_command("ed_entity", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			auto& s = get_scene();
			const auto pos = args.size() > 4 ? vector(args, 2) : edit::snap_point(s.ray_target(s.cam.origin, s.cam.axes().forward), s.snap ? s.grid : 0.0);
			const auto id = s.create_entity(args[1], pos);

			if (id) { game::printf_to_console("created %s\n", describe(id).c_str()); }
			else { game::printf_to_console("^1%s\n", s.doc.last_error().c_str()); }
		});

		// ed_place <xmodel> [x y z]: a model entity ( class of the Models window ) where the camera looks
		command::register_command("ed_place", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: ed_place <xmodel> [x y z]\n"); return; }
			auto& s = get_scene();
			const auto pos = args.size() > 4 ? vector(args, 2) : s.ray_target(s.cam.origin, s.cam.axes().forward);
			const auto id = s.create_model_entity(args[1], pos, 0.0);

			if (id) { game::printf_to_console("created %s\n", describe(id).c_str()); }
			else { game::printf_to_console("^1%s\n", s.doc.last_error().c_str()); }
		});

		// ed_model_class <script_model | misc_model>: what ed_place and the Models browser make ( a misc_model is a
		// static model of the compiled map )
		command::register_command("ed_model_class", [](args_t args)
		{
			auto& s = get_scene();
			if (args.size() > 1 && (args[1] == "script_model" || args[1] == "misc_model")) { s.model_class = args[1]; }
			game::printf_to_console("new model entities: %s\n", s.model_class.c_str());
		});

		// ed_key <key> <value>: on every selected entity; with none selected on the worldspawn, as the Entity window
		command::register_command("ed_key", [](args_t args)
		{
			if (!need_map() || args.size() < 3) { return; }
			auto& doc = get_scene().doc;
			bool ok = false;

			auto entities = doc.selected_entities();
			if (entities.empty()) { entities.push_back(doc.worldspawn()); }

			doc.begin_transaction("Set key");
			for (const object_id id : entities) { ok |= doc.set_key(id, args[1], args[2]); }
			doc.end_transaction();
			print_result(ok, "set key");
		});

		// ed_keys: the keys of the selected entities ( of the worldspawn when none is selected )
		command::register_command("ed_keys", [](args_t)
		{
			if (!need_map()) { return; }
			auto& doc = get_scene().doc;
			auto ents = doc.selected_entities();

			if (ents.empty()) { ents.push_back(doc.worldspawn()); }

			for (std::size_t i = 0; i < ents.size() && i < 8; i++)
			{
				const auto* e = doc.find_entity(ents[i]);
				std::string line = "#" + std::to_string(ents[i]) + ":";

				for (const auto& [key, value] : e->pairs) { line += "  \"" + key + "\" \"" + value + "\""; }
				game::printf_to_console("%s\n", line.c_str());

				for (const object_id id : doc.targets_of(ents[i])) { game::printf_to_console("  targets #%u\n", id); }
				for (const object_id id : doc.targeting(ents[i])) { game::printf_to_console("  targeted by #%u\n", id); }
			}
		});

		command::register_command("ed_delkey", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { return; }
			auto& doc = get_scene().doc;
			bool ok = false;

			auto entities = doc.selected_entities();
			if (entities.empty()) { entities.push_back(doc.worldspawn()); }

			doc.begin_transaction("Remove key");
			for (const object_id id : entities) { ok |= doc.remove_key(id, args[1]); }
			doc.end_transaction();
			print_result(ok, "remove key");
		});

		// ed_clip ax ay az bx by bz cx cy cz [front|back|both]
		command::register_command("ed_clip", [](args_t args)
		{
			if (!need_map() || args.size() < 10) { return; }
			const auto keep = args.size() > 10 && args[10] == "both" ? edit::clip_keep::both : (args.size() > 10 && args[10] == "back" ? edit::clip_keep::back : edit::clip_keep::front);
			print_result(get_scene().doc.clip(vector(args, 1), vector(args, 4), vector(args, 7), keep), "clip");
		});

		// ed_clip_apply [front|back|both]: what Enter / Ctrl+Enter / Shift+Enter do with the points of the clipper
		command::register_command("ed_clip_apply", [](args_t args)
		{
			if (need_map()) { print_result(apply_clip(args.size() > 1 && args[1] == "both" ? 2 : (args.size() > 1 && args[1] == "back" ? 1 : 0)), "clipper"); }
		});

		// ed_resize <x|y|z> <+|-> <distance>: move the faces of the selected brushes on that side
		command::register_command("ed_resize", [](args_t args)
		{
			if (!need_map() || args.size() < 4) { return; }
			const int axis = args[1] == "x" ? 0 : (args[1] == "y" ? 1 : 2);
			print_result(get_scene().resize_selection(axis, args[2] == "-" ? -1 : 1, number(args, 3)), "resize");
		});

		// ed_corner cx cy cz dx dy dz: move the corner at c of the selected brushes by d
		command::register_command("ed_corner", [](args_t args)
		{
			if (!need_map() || args.size() < 7) { return; }
			auto& doc = get_scene().doc;
			const edit::vec3 corner = vector(args, 1);
			bool ok = false;

			doc.begin_transaction("Move corner");
			for (const object_id id : std::vector<object_id>(doc.selection().begin(), doc.selection().end()))
			{
				if (doc.kind(id) == edit::object_kind::brush) { ok |= doc.move_brush_corners(id, std::span<const edit::vec3>(&corner, 1), vector(args, 4)); }
			}

			doc.end_transaction();
			print_result(ok, "move corner");
		});

		// ---- textures
		command::register_command("tex_default", [](args_t args)
		{
			auto& doc = get_scene().doc;
			if (args.size() > 1) { doc.set_default_material(args[1]); }
			game::printf_to_console("material of new brushes: %s\n", std::string(doc.default_material()).c_str());
		});

		command::register_command("tex_set", [](args_t args)
		{
			if (need_map()) { print_result(get_scene().doc.set_material(args.size() > 1 ? std::string_view(args[1]) : get_scene().doc.default_material()), "set material"); }
		});

		command::register_command("tex_shift", [](args_t args) { if (need_map()) { print_result(get_scene().doc.shift_texture(number(args, 1), number(args, 2)), "shift texture"); } });
		command::register_command("tex_scale", [](args_t args) { if (need_map()) { print_result(get_scene().doc.scale_texture(number(args, 1, 1.0), number(args, 2, number(args, 1, 1.0))), "scale texture"); } });
		command::register_command("tex_rotate", [](args_t args) { if (need_map()) { print_result(get_scene().doc.rotate_texture(number(args, 1)), "rotate texture"); } });
		command::register_command("tex_fit", [](args_t args) { if (need_map()) { print_result(get_scene().doc.fit_texture(number(args, 1, 1.0), number(args, 2, 1.0)), "fit texture"); } });

		// ---- layers
		command::register_command("layer_add", [](args_t args) { if (need_map() && args.size() > 1) { print_result(get_scene().doc.add_layer(args[1]), "add layer"); } });
		command::register_command("layer_remove", [](args_t args) { if (need_map() && args.size() > 1) { print_result(get_scene().doc.remove_layer(args[1]), "remove layer"); } });
		command::register_command("layer_rename", [](args_t args) { if (need_map() && args.size() > 2) { print_result(get_scene().doc.rename_layer(args[1], args[2]), "rename layer"); } });
		command::register_command("layer_active", [](args_t args) { if (need_map() && args.size() > 1) { print_result(get_scene().doc.set_active_layer(args[1]), "active layer"); } });
		command::register_command("layer_move", [](args_t args) { if (need_map() && args.size() > 1) { print_result(get_scene().doc.move_selection_to_layer(args[1]), "move to layer"); } });

		command::register_command("layer_hide", [](args_t args)
		{
			if (need_map() && args.size() > 1) { get_scene().doc.set_layer_hidden(args[1], number(args, 2, 1.0) != 0.0); }
		});

		command::register_command("layer_list", [](args_t)
		{
			for (const auto& l : get_scene().doc.layers())
			{
				game::printf_to_console("  \"%s\"%s%s\n", l.name.c_str(), l.active ? "  active" : "", l.hidden ? "  hidden" : "");
			}
		});

		// ---- view
		command::register_command("ed_camera", [](args_t args)
		{
			auto& s = get_scene();
			if (args.size() > 3) { s.cam.origin = { static_cast<float>(number(args, 1)), static_cast<float>(number(args, 2)), static_cast<float>(number(args, 3)) }; }
			if (args.size() > 5) { s.cam.pitch = static_cast<float>(number(args, 4)); s.cam.yaw = static_cast<float>(number(args, 5)); }
			game::printf_to_console("camera %.0f %.0f %.0f  pitch %.1f yaw %.1f\n", s.cam.origin.x, s.cam.origin.y, s.cam.origin.z, s.cam.pitch, s.cam.yaw);
		});

		command::register_command("ed_focus", [](args_t) { get_scene().look_at_selection(); });

		// ed_brushes <off|selected|all>: the collision brushes of the zone in the 3D view
		command::register_command("ed_brushes", [](args_t args)
		{
			auto& s = get_scene();
			if (args.size() > 1)
			{
				s.show_brushes = args[1] == "all" ? brush_display::all : (args[1] == "selected" ? brush_display::selected : brush_display::off);
			}
		});

		// ed_windows <letters>: s selection, e entity, l layers, m models, t materials, u surface, a assets
		command::register_command("ed_windows", [](args_t args)
		{
			auto& st = get();
			const std::string which = args.size() > 1 ? args[1] : "se";
			st.show_selection = which.find('s') != std::string::npos;
			st.show_entity = which.find('e') != std::string::npos;
			st.show_layers = which.find('l') != std::string::npos;
			st.show_models = which.find('m') != std::string::npos;
			st.show_materials = which.find('t') != std::string::npos;
			st.show_surface = which.find('u') != std::string::npos;
			st.show_assets = which.find('a') != std::string::npos;
			st.show_preview = which.find('p') != std::string::npos;
		});

		// ed_focus_window <title>: bring a docked window to the front, eg. ed_focus_window Materials##window
		command::register_command("ed_focus_window", [](args_t args)
		{
			if (args.size() > 1) { ImGui::SetWindowFocus(args[1].c_str()); }
		});

		command::register_command("ed_gizmo", [](args_t args)
		{
			if (args.size() > 1)
			{
				get().gizmo = args[1] == "rotate" ? gizmo_tool::rotate : (args[1] == "scale" ? gizmo_tool::scale : gizmo_tool::move);
			}
		});

		// ed_tool <clip|vertex|none>
		command::register_command("ed_tool", [](args_t args)
		{
			const std::string tool = args.size() > 1 ? args[1] : "none";
			set_clipper(tool == "clip");
			set_vertex_mode(tool == "vertex");
		});

		// ed_material_filter <text> [group 0|1|2]
		command::register_command("ed_material_filter", [](args_t args)
		{
			set_material_filter(args.size() > 1 ? args[1] : "", args.size() > 2 ? std::atoi(args[2].c_str()) : -1);
		});

		// ed_materials <text> / ed_models <text>: names that contain the text ( the first 40 )
		const auto list_names = [](const std::vector<std::string>& names, const std::string& needle)
		{
			int shown = 0, total = 0;

			for (const auto& name : names)
			{
				if (name.find(needle) == std::string::npos)
				{
					continue;
				}

				if (++total <= 40)
				{
					game::printf_to_console("  %s\n", name.c_str());
					shown++;
				}
			}

			game::printf_to_console("%d of %d names with \"%s\"\n", shown, total, needle.c_str());
		};

		command::register_command("ed_materials", [list_names](args_t args)
		{
			if (need_map()) { list_names(get_scene().renderer.material_names(), args.size() > 1 ? args[1] : ""); }
		});

		command::register_command("ed_models", [list_names](args_t args)
		{
			if (need_map()) { list_names(get_scene().renderer.model_names(), args.size() > 1 ? args[1] : ""); }
		});

		command::register_command("ed_stats", [](args_t)
		{
			auto& st = get();
			auto& s = get_scene();
			const auto stats = s.renderer.stats();

			game::printf_to_console("frame: %.2f ms editor work ( max %.2f since the last ed_stats ), %.1f fps shown; renderer %.2f ms, %d draws, %lld triangles; cache: %d + %d objects, %d + %d entities, %d target lines\n",
				st.frame_ms, st.frame_ms_max, ImGui::GetIO().Framerate, stats.cpu_ms, stats.draw_calls, static_cast<long long>(stats.triangles),
				static_cast<int>(s.statics.objects.size()), static_cast<int>(s.selected.objects.size()), static_cast<int>(s.statics.entities.size()), static_cast<int>(s.selected.entities.size()), static_cast<int>(s.connections.size()));
			game::printf_to_console("  slowest since the last ed_stats: document -> renderer %.2f ms, cache of everything %.2f ms, cache of the selection %.2f ms, 3D line buffers %.2f ms\n",
				s.slowest.renderer, s.slowest.statics, s.slowest.selected, s.slowest.view_lines);

			st.frame_ms_max = 0.0;
			s.slowest = {};
		});

		// view2d <axis|center|zoom|fit|new|click|drag|rect> ..: the 2D views ( grid_view.cpp )
		command::register_command("view2d", [](args_t args) { grid_view_command(args); });

		// ---- injected input: ui_mouse <x> <y> | down [button] | up [button] | wheel <d> | off;  ui_key <name> [down|up];
		// ui_text <characters>. Positions are pixels of the window. Without "down" / "up" a key is pressed and released.
		command::register_command("ui_mouse", [](args_t args)
		{
			auto& io = ImGui::GetIO();
			const std::string what = args.size() > 1 ? args[1] : "";

			if (what == "down" || what == "up")
			{
				g_inject_buttons = true;
				io.AddMouseButtonEvent(args.size() > 2 ? std::atoi(args[2].c_str()) : 0, what == "down");
			}
			else if (what == "wheel")
			{
				g_inject_buttons = true;
				io.AddMouseWheelEvent(0.0f, static_cast<float>(number(args, 2, 1.0)));
			}
			else if (what == "off")
			{
				g_inject_mouse = false;
				g_inject_buttons = false;
			}
			else if (args.size() > 2)
			{
				g_inject_mouse = true;
				g_inject_pos = ImVec2(static_cast<float>(number(args, 1)), static_cast<float>(number(args, 2)));
				io.AddMousePosEvent(g_inject_pos.x, g_inject_pos.y);
			}
		});

		command::register_command("ui_key", [](args_t args)
		{
			const ImGuiKey key = args.size() > 1 ? key_by_name(args[1]) : ImGuiKey_None;
			if (key == ImGuiKey_None)
			{
				game::printf_to_console("usage: ui_key <a..z | 0..9 | space enter escape delete backspace tab left right up down pageup pagedown home [ ] ctrl shift alt> [down|up]\n");
				return;
			}

			auto& io = ImGui::GetIO();
			const std::string state = args.size() > 2 ? args[2] : "";

			if (state != "up") { io.AddKeyEvent(key, true); }
			if (state != "down") { io.AddKeyEvent(key, false); }
		});

		command::register_command("ui_text", [](args_t args)
		{
			if (args.size() > 1) { ImGui::GetIO().AddInputCharactersUTF8(args[1].c_str()); }
		});

		// ---- the project's assets: asset_new_material, asset_import_model, asset_gdt, asset_build, asset_list ..
		assets::register_commands();
		preview::register_commands();
		register_brush_patch_commands();
		register_extra_commands();

		command::register_command("help", [](args_t)
		{
			std::string line;
			for (const auto& name : command::names())
			{
				line += name + "  ";
				if (line.size() > 110) { game::printf_to_console("%s\n", line.c_str()); line.clear(); }
			}

			if (!line.empty()) { game::printf_to_console("%s\n", line.c_str()); }
		});
	}
}
