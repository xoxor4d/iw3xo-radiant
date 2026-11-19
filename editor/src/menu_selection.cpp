#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "codol/packed_scripts.hpp"
#include "menu_ext.hpp"
#include "assets.hpp"

#include <commdlg.h>

// What CoD4 radiant has in its File, Edit, View, Selection, Grid, Misc and Region menus and the editor did not:
// Save Selected / Save Region / import, the leak line of the last build, copy / paste through the Windows
// clipboard as .map text, Map Info, Entity Info, select by key / link / volume, flip / rotate / scale dialogs,
// CSG merge, auto caulk, link, cycle group, the content flags, hide unselected / show last hidden, floors,
// regions, find brush, go to position, entity colour, replace models, drop to floor, the grid sizes below 1.
//
// As everywhere in the editor an action is one function that the menu entry, the key and the console command
// call. The document side of each is in codol_edit ( select_ops.cpp ) with its tests; filters are in filters.cpp,
// prefabs in prefabs.cpp. Nothing here calls into CoD4 radiant.

namespace editor::ui::ext
{
	state& get()
	{
		static state s;
		return s;
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

	std::filesystem::path map_source_dir()
	{
		return std::filesystem::u8path(utils::fs::get_basepath()) / "map_source";
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

	bool read_text(const std::filesystem::path& file, std::string& out)
	{
		std::ifstream in(file, std::ios::binary);
		if (!in.is_open())
		{
			return false;
		}

		out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
		return true;
	}

	bool write_text(const std::filesystem::path& file, const std::string& text, std::string& error)
	{
		std::error_code ec;
		std::filesystem::create_directories(file.parent_path(), ec);

		std::ofstream out(file, std::ios::binary | std::ios::trunc);
		if (!out.is_open())
		{
			error = "Could not write " + to_utf8(file);
			return false;
		}

		out.write(text.data(), static_cast<std::streamsize>(text.size()));
		out.close();

		if (!out)
		{
			error = "Could not write " + to_utf8(file);
			return false;
		}

		return true;
	}

	namespace
	{
		void warn(const char* title, const std::string& text)
		{
			game::printf_to_console("%s: %s\n", title, text.c_str());
			ImGui::Toast(ImGuiToastType_Warning, title, text.c_str(), 4000);
		}

		void fail(const char* title, const std::string& text)
		{
			game::printf_to_console("^1%s: %s\n", title, text.c_str());
			ImGui::Toast(ImGuiToastType_Error, title, text.c_str(), 6000);
		}

		// ---- views ------------------------------------------------------------------------------------

		void center_2d(const edit::vec3& at)
		{
			view2d_info info;
			for (int i = 0; grid_view_info(i, info); i++)
			{
				grid_view_center(i, at[info.h()], at[info.v()]);
			}
		}

		// where something new goes: under the mouse in a 2D view ( the third axis: where the selection was last ),
		// else where the camera looks
		edit::vec3 cursor_position(bool* in_2d = nullptr)
		{
			auto& s = get_scene();
			view2d_info info;

			if (in_2d) { *in_2d = false; }

			if (ui::get().hovered_2d >= 0 && grid_view_info(ui::get().hovered_2d, info) && info.zoom > 0.0)
			{
				const ImVec2 mouse = ImGui::GetIO().MousePos;
				edit::vec3 at;
				at[info.h()] = info.center[0] + (mouse.x - info.canvas_pos.x - info.canvas_size.x * 0.5f) / info.zoom;
				at[info.v()] = info.center[1] - (mouse.y - info.canvas_pos.y - info.canvas_size.y * 0.5f) / info.zoom;
				at[info.axis] = s.last_selection_box.valid() ? s.last_selection_box.center()[info.axis] : 0.0;
				if (in_2d) { *in_2d = true; }
				return at;
			}

			return s.ray_target(s.cam.origin, s.cam.axes().forward);
		}

		void center_2d_on_camera()
		{
			const auto& cam = get_scene().cam;
			center_2d({ cam.origin.x, cam.origin.y, cam.origin.z });
		}

		// Up Floor / Down Floor: the camera goes to eye height above the next brush top above / below it, among the
		// brushes it stands over
		void change_floor(const bool up)
		{
			auto& s = get_scene();
			auto& doc = s.doc;
			constexpr double eye = 60.0;
			const double x = s.cam.origin.x, y = s.cam.origin.y, z = s.cam.origin.z;
			double best = up ? 1e30 : -1e30;

			for (const object_id ent : doc.entities())
			{
				for (const object_id child : doc.find_entity(ent)->children)
				{
					if (doc.kind(child) != edit::object_kind::brush || !doc.is_visible(child))
					{
						continue;
					}

					const auto box = doc.object_bounds(child);
					if (!box.valid() || x < box.mins.x || x > box.maxs.x || y < box.mins.y || y > box.maxs.y)
					{
						continue;
					}

					const double stand = box.maxs.z + eye;
					if (up ? (stand > z + 1.0 && stand < best) : (stand < z - 1.0 && stand > best))
					{
						best = stand;
					}
				}
			}

			if (std::fabs(best) > 1e29)
			{
				warn(up ? "Up floor" : "Down floor", up ? "No floor above the camera." : "No floor below the camera.");
				return;
			}

			s.cam.origin.z = static_cast<float>(best);
			game::printf_to_console("camera at %.0f %.0f %.0f\n", s.cam.origin.x, s.cam.origin.y, s.cam.origin.z);
		}

		// ---- clipboard --------------------------------------------------------------------------------

		// What this editor copied last. The Windows clipboard holds the same text ( that is how objects go from one
		// CoDOL Radiant to another ); this copy is used when the clipboard cannot be opened - another program that
		// watches the clipboard may hold it for a moment right after a copy.
		std::string g_copied;

		std::string clipboard_text()
		{
			for (int attempt = 0; attempt < 5; attempt++)
			{
				if (const char* clip = ImGui::GetClipboardText(); clip && *clip)
				{
					return clip;
				}

				Sleep(15);
			}

			return g_copied;
		}

		bool copy_selection(const bool cut)
		{
			auto& doc = get_scene().doc;
			const std::string text = doc.selection_map_text();

			if (text.empty())
			{
				warn(cut ? "Cut" : "Copy", "Nothing selected.");
				return false;
			}

			ImGui::SetClipboardText(text.c_str());
			g_copied = text;
			game::printf_to_console("%s: %d object(s) on the clipboard as .map text ( %d bytes )\n", cut ? "cut" : "copy", static_cast<int>(doc.selection().size()),
				static_cast<int>(text.size()));

			if (cut)
			{
				doc.begin_transaction("Cut");
				doc.delete_selection();
				doc.end_transaction();
			}

			return true;
		}

		// in place, or ( `at` ) with the middle of its floor at a position
		bool paste_text(const std::string_view text, const edit::vec3* at, const bool keep_depth_axis, const int depth_axis, const char* name)
		{
			auto& s = get_scene();
			auto& doc = s.doc;

			doc.begin_transaction(name);
			bool ok = doc.paste_map_text(text, nullptr, 0.0, name);

			if (ok && at)
			{
				const auto box = doc.selection_bounds();
				if (box.valid())
				{
					const auto c = box.center();
					edit::vec3 delta = *at - edit::vec3{ c.x, c.y, keep_depth_axis ? c.z : box.mins.z };

					if (keep_depth_axis)
					{
						delta[depth_axis] = 0.0;
					}

					delta = edit::snap_point(delta, s.snap ? s.grid : 0.0);
					if (!(delta == edit::vec3{}))
					{
						doc.translate(delta);
					}
				}
			}

			if (ok)
			{
				doc.end_transaction();
			}
			else
			{
				const std::string why = doc.last_error();
				doc.cancel_transaction();
				fail(name, why);
			}

			return ok;
		}

		bool paste_clipboard(const bool at_cursor, const edit::vec3* position = nullptr)
		{
			const std::string clip = clipboard_text();
			std::string_view text = clip;

			while (!text.empty() && (text.front() == ' ' || text.front() == '\r' || text.front() == '\n' || text.front() == '\t'))
			{
				text.remove_prefix(1);
			}

			if (text.rfind("iwmap", 0) != 0)
			{
				warn("Paste", "The clipboard holds no map objects ( copy brushes or entities in this or another CoDOL Radiant first ).");
				return false;
			}

			bool in_2d = false;
			edit::vec3 at;
			int axis = 2;

			if (position)
			{
				at = *position;
			}
			else if (at_cursor)
			{
				at = cursor_position(&in_2d);
				axis = in_2d ? grid_view_axis(ui::get().hovered_2d) : 2;
			}

			const bool ok = paste_text(text, at_cursor || position ? &at : nullptr, in_2d, axis, "Paste");
			if (ok)
			{
				game::printf_to_console("paste: %d object(s)%s\n", static_cast<int>(get_scene().doc.selection().size()), at_cursor || position ? " at the cursor" : " where they were");
			}

			return ok;
		}

		// ---- files ------------------------------------------------------------------------------------

		bool save_selected(const std::filesystem::path& file)
		{
			auto& doc = get_scene().doc;
			const std::string text = doc.selection_map_text(true);
			std::string error;

			if (text.empty())
			{
				warn("Save selected", "Nothing selected.");
				return false;
			}

			if (!write_text(file, text, error))
			{
				fail("Save selected", error);
				return false;
			}

			game::printf_to_console("[CoDOL] selection ( %d objects ) saved as %s\n", static_cast<int>(doc.selection().size()), to_utf8(file).c_str());
			ImGui::Toast(ImGuiToastType_Success, "Save selected", to_utf8(file).c_str(), 3000);
			return true;
		}

		bool save_region(const std::filesystem::path& file)
		{
			auto& doc = get_scene().doc;
			std::string error;

			if (!write_text(file, doc.region_map_text(false), error))
			{
				fail("Save region", error);
				return false;
			}

			game::printf_to_console("[CoDOL] %s saved as %s\n", doc.has_region() ? "the region" : "the map ( no region is set )", to_utf8(file).c_str());
			ImGui::Toast(ImGuiToastType_Success, "Save region", to_utf8(file).c_str(), 3000);
			return true;
		}

		bool import_map(const std::filesystem::path& file)
		{
			std::string text;
			if (!read_text(file, text))
			{
				fail("Import", "Could not read " + to_utf8(file));
				return false;
			}

			if (!paste_text(text, nullptr, false, 2, "Import map"))
			{
				return false;
			}

			game::printf_to_console("[CoDOL] imported %s: %d object(s), selected\n", to_utf8(file).c_str(), static_cast<int>(get_scene().doc.selection().size()));
			return true;
		}

		// ---- leak line / error brushes ----------------------------------------------------------------

		std::filesystem::path default_report()
		{
			auto& s = get_scene();
			const auto base = std::filesystem::u8path(utils::fs::get_basepath()) / "compiled";
			const std::string name = to_utf8(s.map_file.stem());
			std::error_code ec;

			for (const auto& candidate : { base / name / "work" / (name + ".report.txt"), base / (name + ".build.txt"), base / (name + ".report.txt") })
			{
				if (std::filesystem::is_regular_file(candidate, ec))
				{
					return candidate;
				}
			}

			return {};
		}

		// "LEAK: entity <class> at ( x y z ) reaches the outside of the map; N points:" and N lines "  x y z"
		bool load_leak(std::filesystem::path file)
		{
			auto& st = get();
			st.leak.clear();
			st.leak_text.clear();
			st.spot = -1;

			if (file.empty())
			{
				file = default_report();
			}

			std::string text;
			if (file.empty() || !read_text(file, text))
			{
				warn("Pointfile", "No build report of this map was found: build the map first ( File > Build map ).");
				return false;
			}

			const auto at = text.find("LEAK: entity");
			if (at == std::string::npos)
			{
				game::printf_to_console("[CoDOL] %s: the last build had no leak\n", to_utf8(file.filename()).c_str());
				ImGui::Toast(ImGuiToastType_Success, "Pointfile", "The last build had no leak.", 3000);
				return true;
			}

			auto line_end = text.find('\n', at);
			st.leak_text = text.substr(at, line_end == std::string::npos ? std::string::npos : line_end - at);

			while (line_end != std::string::npos)
			{
				const auto next = text.find('\n', line_end + 1);
				const std::string line = text.substr(line_end + 1, next == std::string::npos ? std::string::npos : next - line_end - 1);
				double x, y, z;

				if (std::sscanf(line.c_str(), " %lf %lf %lf", &x, &y, &z) != 3)
				{
					break;
				}

				st.leak.push_back({ x, y, z });
				line_end = next;
			}

			game::printf_to_console("[CoDOL] %s\n[CoDOL] the leak line has %d points: Misc > next / previous leak spot ( leak_next, leak_prev ) walks along it\n", st.leak_text.c_str(),
				static_cast<int>(st.leak.size()));
			ImGui::Toast(ImGuiToastType_Warning, "Pointfile", st.leak_text.c_str(), 6000);
			return true;
		}

		// brushes the compiler leaves out: no closed convex volume
		int collect_errors()
		{
			auto& st = get();
			auto& doc = get_scene().doc;
			st.errors.clear();
			st.spot = -1;

			for (const object_id ent : doc.entities())
			{
				for (const object_id child : doc.find_entity(ent)->children)
				{
					const auto* b = doc.find_brush(child);
					const auto* geo = b && !b->opaque ? doc.geometry(child) : nullptr;

					if (b && !b->opaque && (!geo || !geo->valid))
					{
						st.errors.push_back(child);
					}
				}
			}

			game::printf_to_console("[CoDOL] %d brush(es) that are no closed convex volume ( the compiler leaves them out )%s\n", static_cast<int>(st.errors.size()),
				st.errors.empty() ? "" : ": Misc > next / previous leak spot selects them one by one");
			return static_cast<int>(st.errors.size());
		}

		void step_spot(const int step)
		{
			auto& st = get();
			auto& s = get_scene();
			const int total = static_cast<int>(st.leak.size() + st.errors.size());

			if (total == 0)
			{
				warn("Leak spot", "No leak line and no error brushes are loaded ( File > Generate File > Pointfile / Errorfile ).");
				return;
			}

			st.spot = ((st.spot < 0 ? (step > 0 ? -1 : 0) : st.spot) + step + total) % total;

			if (st.spot < static_cast<int>(st.leak.size()))
			{
				const auto& p = st.leak[static_cast<std::size_t>(st.spot)];
				s.cam.origin = { static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z) };

				// look along the line
				const auto& q = st.leak[static_cast<std::size_t>(std::min(st.spot + 1, static_cast<int>(st.leak.size()) - 1))];
				const edit::vec3 d = q - p;
				if (edit::length(d) > 1.0)
				{
					s.cam.yaw = static_cast<float>(std::atan2(d.y, d.x) * 180.0 / edit::PI);
					s.cam.pitch = static_cast<float>(-std::atan2(d.z, std::sqrt(d.x * d.x + d.y * d.y)) * 180.0 / edit::PI);
				}

				center_2d(p);
				game::printf_to_console("leak spot %d of %d: %.1f %.1f %.1f\n", st.spot + 1, static_cast<int>(st.leak.size()), p.x, p.y, p.z);
				return;
			}

			const object_id id = st.errors[static_cast<std::size_t>(st.spot) - st.leak.size()];
			if (s.doc.kind(id) == edit::object_kind::none)
			{
				game::printf_to_console("error brush #%u is gone\n", id);
				return;
			}

			s.doc.select(id);
			focus_selection();
			game::printf_to_console("error brush #%u ( %d of %d )\n", id, st.spot - static_cast<int>(st.leak.size()) + 1, static_cast<int>(st.errors.size()));
		}

		// script_fx / script_sound entities are what codol_bsp turns into the createfx script of the map
		void createfx_info()
		{
			auto& s = get_scene();
			int fx = 0, sounds = 0;

			for (const object_id id : s.doc.entities())
			{
				const auto cls = s.doc.find_entity(id)->classname();
				fx += cls == "script_fx" ? 1 : 0;
				sounds += cls == "script_sound" ? 1 : 0;
			}

			const std::string name = to_utf8(s.map_file.stem());
			game::printf_to_console("[CoDOL] createfx: %d script_fx and %d script_sound entities. File > Build map writes them as compiled\\%s\\scripts\\maps\\createfx\\%s_fx.gsc\n"
				"[CoDOL] ( and maps\\mp\\%s_fx.gsc ); there is no separate export - the script is generated from the map at every build.\n", fx, sounds, name.c_str(), name.c_str(), name.c_str());
			ImGui::Toast(ImGuiToastType_Info, "Createfx", "Written by File > Build map from the script_fx / script_sound entities. See the console.", 5000);
		}

		// ---- selection --------------------------------------------------------------------------------

		void select_same(const char* key)
		{
			auto& doc = get_scene().doc;
			const auto ents = doc.selected_entities();
			const auto* e = ents.empty() ? nullptr : doc.find_entity(ents[0]);
			const std::string* value = e ? e->find(key) : nullptr;

			if (!value || value->empty())
			{
				// nothing to take the value from: ask
				auto& st = get();
				st.key = key;
				st.pattern.clear();
				st.open_key_value = true;
				return;
			}

			const std::string wanted = *value;
			const auto count = doc.select_by_key(key, wanted);
			game::printf_to_console("%d entities with \"%s\" \"%s\"\n", static_cast<int>(count), key, wanted.c_str());
		}

		void select_key_value(const std::string& key, const std::string& pattern, const bool add)
		{
			auto& doc = get_scene().doc;
			const auto count = doc.select_by_key(key, pattern, add ? edit::select_mode::add : edit::select_mode::replace);
			game::printf_to_console("%d entities with \"%s\" \"%s\", %d objects selected\n", static_cast<int>(count), key.c_str(), pattern.empty() ? "*" : pattern.c_str(),
				static_cast<int>(doc.selection().size()));

			if (!count)
			{
				ImGui::Toast(ImGuiToastType_Warning, "Select", "No entity matches.", 2500);
			}
		}

		void select_connected()
		{
			auto& doc = get_scene().doc;
			const auto count = doc.select_connected();

			if (!count)
			{
				warn("Select connected", doc.selected_entities().empty() ? "Select an entity first." : "Nothing is linked to the selection by target / targetname.");
				return;
			}

			game::printf_to_console("select connected: %d more entities\n", static_cast<int>(count));
		}

		void select_volume(const edit::volume_select how)
		{
			const int axis = ui::get().hovered_2d >= 0 ? grid_view_axis(ui::get().hovered_2d) : 2;
			report(get_scene().doc.select_by_volume(how, axis), "Select");
		}

		void flip(const int axis)
		{
			auto& s = get_scene();
			report(s.doc.flip(s.selection_pivot(), axis), axis == 0 ? "Flip X" : (axis == 1 ? "Flip Y" : "Flip Z"));
		}

		void rotate(const int axis, const double degrees)
		{
			auto& s = get_scene();
			edit::vec3 dir;
			dir[axis] = 1.0;
			report(s.rotate(s.selection_pivot(), dir, degrees), "Rotate");
		}

		// about x, then y, then z, as one step
		bool rotate_xyz(const edit::vec3& degrees)
		{
			auto& s = get_scene();
			if (s.doc.selection().empty())
			{
				return report(false, "Rotate");
			}

			const auto pivot = s.selection_pivot();
			bool ok = false;
			s.doc.begin_transaction("Rotate");

			for (int axis = 0; axis < 3; axis++)
			{
				if (degrees[axis] != 0.0)
				{
					edit::vec3 dir;
					dir[axis] = 1.0;
					ok |= s.rotate(pivot, dir, degrees[axis]);
				}
			}

			s.doc.end_transaction();
			return ok;
		}

		bool scale_by(edit::vec3 factors)
		{
			auto& s = get_scene();
			const auto& st = get();

			for (int axis = 0; axis < 3; axis++)
			{
				if (st.scale_lock[axis])
				{
					factors[axis] = 1.0;
				}
			}

			if (factors.x == 0.0 || factors.y == 0.0 || factors.z == 0.0)
			{
				warn("Scale", "A factor of 0 would flatten the selection.");
				return false;
			}

			return report(s.scale(s.selection_pivot(), factors), "Scale");
		}

		bool all_have(const std::string_view word, const bool tool)
		{
			auto& doc = get_scene().doc;
			bool any = false;

			for (const object_id id : doc.selection())
			{
				const edit::object_header* h = doc.find_brush(id);
				if (!h) { h = doc.find_patch(id); }
				if (!h) { continue; }

				const auto& words = tool ? h->tool_flags : h->contents;
				if (std::find(words.begin(), words.end(), word) == words.end())
				{
					return false;
				}

				any = true;
			}

			return any;
		}

		void toggle_content(const char* word, const char* name)
		{
			auto& doc = get_scene().doc;
			const bool on = !all_have(word, false);

			if (report(doc.set_content_flag(word, on), name))
			{
				game::printf_to_console("contents %s: %s on %d object(s)\n", word, on ? "set" : "cleared", static_cast<int>(doc.selection().size()));
			}
		}

		void auto_caulk(const bool all)
		{
			auto& s = get_scene();

			// a material that is drawn see-through hides nothing
			const auto opaque = [&s](const std::string_view material)
			{
				const auto c = edit::classify_material(material);
				if (c != edit::material_class::normal)
				{
					return c == edit::material_class::caulk || c == edit::material_class::sky;
				}

				static std::unordered_map<std::string, bool> known;
				const std::string name(material);

				if (const auto it = known.find(name); it != known.end())
				{
					return it->second;
				}

				bool solid = true;
				if (s.renderer.loaded())
				{
					// names that say what they are; the renderer has no "see through" flag to ask for
					for (const char* word : { "glass", "fence", "grate", "foliage", "decal", "water", "wire", "net_", "chainlink" })
					{
						solid = solid && name.find(word) == std::string::npos;
					}
				}

				known.emplace(name, solid);
				return solid;
			};

			const int count = s.doc.auto_caulk(!all, edit::CAULK_MATERIAL, opaque);

			if (count <= 0)
			{
				warn("Auto caulk", s.doc.last_error());
				return;
			}

			game::printf_to_console("auto caulk: %d hidden face(s) got \"%s\"\n", count, std::string(edit::CAULK_MATERIAL).c_str());
			ImGui::Toast(ImGuiToastType_Success, "Auto caulk", (std::to_string(count) + " hidden faces are caulk now.").c_str(), 3000);
		}

		// ---- regions ----------------------------------------------------------------------------------

		std::vector<line_vertex> g_region_lines;

		void add_box(std::vector<line_vertex>& out, const edit::bounds& b, const DWORD color)
		{
			const auto corner = [&](const int i) { return float3{ static_cast<float>((i & 1) ? b.maxs.x : b.mins.x), static_cast<float>((i & 2) ? b.maxs.y : b.mins.y), static_cast<float>((i & 4) ? b.maxs.z : b.mins.z) }; };
			static const int edges[12][2] = { { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }, { 0, 2 }, { 1, 3 }, { 4, 6 }, { 5, 7 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };

			for (const auto& e : edges)
			{
				const auto p = corner(e[0]), q = corner(e[1]);
				out.push_back({ p.x, p.y, p.z, color });
				out.push_back({ q.x, q.y, q.z, color });
			}
		}

		void set_region(edit::bounds box, const char* what)
		{
			auto& s = get_scene();
			s.doc.set_region(box);

			// for the picture an endless side ends where the map does
			edit::bounds shown = box;
			edit::bounds map;
			static_cast<void>(s.document_bounds(map));

			for (int i = 0; i < 3; i++)
			{
				if (shown.mins[i] < -1e29) { shown.mins[i] = map.valid() ? map.mins[i] - 64.0 : -4096.0; }
				if (shown.maxs[i] > 1e29) { shown.maxs[i] = map.valid() ? map.maxs[i] + 64.0 : 4096.0; }
			}

			g_region_lines.clear();
			add_box(g_region_lines, shown, 0xFFE8C020);

			std::size_t visible = 0;
			for (const object_id ent : s.doc.entities())
			{
				const auto* e = s.doc.find_entity(ent);
				visible += e->children.empty() && ent != s.doc.worldspawn() && s.doc.is_visible(ent) ? 1 : 0;
				for (const object_id child : e->children) { visible += s.doc.is_visible(child) ? 1 : 0; }
			}

			game::printf_to_console("region ( %s ): %g %g %g .. %g %g %g, %d objects shown\n", what, shown.mins.x, shown.mins.y, shown.mins.z, shown.maxs.x, shown.maxs.y, shown.maxs.z,
				static_cast<int>(visible));
		}

		void region_off()
		{
			get_scene().doc.clear_region();
			g_region_lines.clear();
			game::printf_to_console("region off\n");
		}

		// what the top view shows, any height
		void region_xy()
		{
			view2d_info info, found;
			bool any = false;

			for (int i = 0; grid_view_info(i, info); i++)
			{
				if (info.open && info.axis == 2 && (!any || i == ui::get().hovered_2d))
				{
					found = info;
					any = true;
				}
			}

			if (!any || found.zoom <= 0.0)
			{
				warn("Region", "There is no XY ( top ) 2D view.");
				return;
			}

			edit::bounds box;
			const double half_w = found.canvas_size.x * 0.5 / found.zoom, half_h = found.canvas_size.y * 0.5 / found.zoom;
			box.mins = { found.center[0] - half_w, found.center[1] - half_h, -1e30 };
			box.maxs = { found.center[0] + half_w, found.center[1] + half_h, 1e30 };
			set_region(box, "XY view");
		}

		// tall: the selection's x / y, any height; brush: its box. As in radiant the brush that marked the region goes
		// ( an undo step ); "selected" keeps it.
		void region_from_selection(const bool tall, const bool keep)
		{
			auto& s = get_scene();
			edit::bounds box = s.doc.selection_bounds();

			if (!box.valid())
			{
				warn("Region", "Select the brush that marks the region first.");
				return;
			}

			if (tall)
			{
				box.mins.z = -1e30;
				box.maxs.z = 1e30;
			}

			if (keep)
			{
				s.doc.deselect_all();
			}
			else
			{
				s.doc.delete_selection();
			}

			set_region(box, tall ? "tall brush" : (keep ? "selected brushes" : "brush"));
		}

		// ---- misc -------------------------------------------------------------------------------------

		void find_brush(const int entity, const int brush)
		{
			auto& s = get_scene();
			const object_id id = s.doc.find_by_number(entity, brush);

			if (id == edit::no_object)
			{
				fail("Find brush", brush >= 0 ? "There is no brush " + std::to_string(brush) + " in entity " + std::to_string(entity) + "." : "There is no entity " + std::to_string(entity) + ".");
				return;
			}

			s.doc.select(id);
			focus_selection();
			game::printf_to_console("entity %d%s: #%u, %d object(s) selected\n", entity, brush >= 0 ? (" brush " + std::to_string(brush)).c_str() : "", id, static_cast<int>(s.doc.selection().size()));
		}

		bool set_entity_color(const float color[3])
		{
			auto& doc = get_scene().doc;
			const auto ents = doc.selected_entities();

			if (ents.empty())
			{
				warn("Entity color", "Select an entity ( a light ) first.");
				return false;
			}

			char text[96];
			std::snprintf(text, sizeof(text), "%g %g %g", std::round(color[0] * 1000.0f) / 1000.0f, std::round(color[1] * 1000.0f) / 1000.0f, std::round(color[2] * 1000.0f) / 1000.0f);

			bool ok = false;
			doc.begin_transaction("Entity color", 0x636f6c6full + doc.selection_counter());
			for (const object_id id : ents) { ok |= doc.set_key(id, "_color", text); }
			doc.end_transaction();
			return ok;
		}

		void replace_models(const std::string& from, const std::string& to, const bool selected_only)
		{
			auto& doc = get_scene().doc;

			if (from.empty() || to.empty())
			{
				warn("Replace models", "Give the model to replace and the model that takes its place.");
				return;
			}

			const int count = doc.replace_key_value("model", from, to, selected_only);
			if (!count)
			{
				warn("Replace models", doc.last_error());
				return;
			}

			game::printf_to_console("replace models: %d entities now use %s\n", count, to.c_str());
			ImGui::Toast(ImGuiToastType_Success, "Replace models", (std::to_string(count) + " entities changed.").c_str(), 3000);
		}

		void drop_to_floor(const bool orient)
		{
			auto& doc = get_scene().doc;
			const int count = doc.drop_to_floor(orient);

			if (!count)
			{
				warn("Drop to floor", doc.last_error());
				return;
			}

			game::printf_to_console("drop to floor: %d entities moved%s\n", count, orient ? " and turned to the surface" : "");
		}

		// "plant models": a model entity that was just made ( Models browser, ed_place ) falls to the floor
		void plant_new_models()
		{
			static std::uint64_t seen_change = ~0ull, seen_serial = ~0ull;
			static object_id highest = 0;
			auto& s = get_scene();
			auto& doc = s.doc;

			if (!s.open || seen_change == doc.change_counter())
			{
				return;
			}

			const bool fresh_map = seen_serial != s.open_serial;
			seen_change = doc.change_counter();
			seen_serial = s.open_serial;

			object_id top = 0;
			for (const object_id id : doc.entities()) { top = std::max(top, id); }

			const object_id before = highest;
			highest = std::max(fresh_map ? 0u : highest, top);

			if (fresh_map || !get().plant_models || s.dragging() || doc.selection().size() != 1)
			{
				return;
			}

			const object_id id = doc.selection()[0];
			const auto* e = doc.find_entity(id);

			if (e && id > before && !e->value("model").empty() && !edit::document::is_prefab(*e) && doc.undo_name() == "Create entity")
			{
				doc.drop_to_floor(get().orient_to_floor);
				seen_change = doc.change_counter();
			}
		}

		void print_map_info()
		{
			auto& s = get_scene();
			const auto info = s.doc.info();

			game::printf_to_console("%s: %d entities ( %d point, %d brush ), %d brushes ( %d in the world, %d detail, %d faces ), %d patches ( %d curves, %d terrain ), %d models, %d lights, %d prefabs\n",
				s.map_name.c_str(), static_cast<int>(info.entities), static_cast<int>(info.point_entities), static_cast<int>(info.brush_entities), static_cast<int>(info.brushes),
				static_cast<int>(info.world_brushes), static_cast<int>(info.detail_brushes), static_cast<int>(info.faces), static_cast<int>(info.patches), static_cast<int>(info.curves),
				static_cast<int>(info.terrain), static_cast<int>(info.models), static_cast<int>(info.lights), static_cast<int>(info.prefabs));

			if (info.box.valid())
			{
				const auto size = info.box.size();
				game::printf_to_console("  bounds %g %g %g .. %g %g %g ( %g x %g x %g )\n", info.box.mins.x, info.box.mins.y, info.box.mins.z, info.box.maxs.x, info.box.maxs.y, info.box.maxs.z,
					size.x, size.y, size.z);
			}

			std::string line;
			for (const auto& [name, count] : info.classes)
			{
				line += "  " + name + " " + std::to_string(count);
				if (line.size() > 110) { game::printf_to_console("%s\n", line.c_str()); line.clear(); }
			}

			if (!line.empty()) { game::printf_to_console("%s\n", line.c_str()); }
		}

		// ---- windows ----------------------------------------------------------------------------------

		void draw_map_info()
		{
			auto& st = get();
			auto& s = get_scene();

			if (!st.show_map_info)
			{
				return;
			}

			ImGui::SetNextWindowPos(ImVec2(420.0f, 90.0f), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(420.0f, 520.0f), ImGuiCond_FirstUseEver);

			if (ImGui::Begin("Map Info##window", &st.show_map_info, ImGuiWindowFlags_NoCollapse))
			{
				if (!s.open)
				{
					ImGui::TextDisabled("No map is open.");
					ImGui::End();
					return;
				}

				// counted again when the map changed
				static edit::map_info info;
				static std::uint64_t counted = ~0ull, serial = ~0ull;

				if (counted != s.doc.change_counter() || serial != s.open_serial)
				{
					counted = s.doc.change_counter();
					serial = s.open_serial;
					info = s.doc.info();
				}

				const auto row = [](const char* label, const std::size_t value, const char* note = nullptr)
				{
					ImGui::TextUnformatted(label);
					ImGui::SameLine(190.0f);
					ImGui::Text("%d", static_cast<int>(value));
					if (note) { ImGui::SameLine(); ImGui::TextDisabled("%s", note); }
				};

				ImGui::TextUnformatted(s.map_name.c_str());
				ImGui::Separator();
				row("Brushes", info.brushes);
				row("   in the world", info.world_brushes);
				row("   detail", info.detail_brushes);
				row("   faces", info.faces);
				row("Patches", info.patches);
				row("   curves", info.curves);
				row("   terrain", info.terrain);
				row("Entities", info.entities, "with the worldspawn");
				row("   point entities", info.point_entities);
				row("   brush entities", info.brush_entities);
				row("   models", info.models);
				row("   lights", info.lights);
				row("   prefabs", info.prefabs);

				if (info.box.valid())
				{
					const auto size = info.box.size();
					ImGui::Text("Bounds  %g %g %g  ..  %g %g %g", info.box.mins.x, info.box.mins.y, info.box.mins.z, info.box.maxs.x, info.box.maxs.y, info.box.maxs.z);
					ImGui::Text("Size    %g x %g x %g", size.x, size.y, size.z);
				}

				if (ImGui::CollapsingHeader("Entities by class", ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (const auto& [name, count] : info.classes)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(s.class_color(name)));
						const bool clicked = ImGui::Selectable(name.c_str(), false);
						ImGui::PopStyleColor();
						ImGui::SameLine(300.0f);
						ImGui::Text("%d", static_cast<int>(count));

						if (clicked && name != "worldspawn")
						{
							select_key_value("classname", name, false);
						}
					}
				}

				if (ImGui::CollapsingHeader("Materials"))
				{
					for (std::size_t i = 0; i < info.materials.size() && i < 200; i++)
					{
						ImGui::TextUnformatted(info.materials[i].first.c_str());
						ImGui::SameLine(300.0f);
						ImGui::Text("%d", static_cast<int>(info.materials[i].second));
					}
				}
			}

			ImGui::End();
		}

		struct entity_row
		{
			object_id id;
			std::string classname, targetname, target, model;
			edit::vec3 origin;
		};

		void list_entities(const std::string& filter, std::vector<entity_row>& rows)
		{
			auto& doc = get_scene().doc;
			rows.clear();

			for (const object_id id : doc.entities())
			{
				const auto* e = doc.find_entity(id);
				if (!e || id == doc.worldspawn())
				{
					continue;
				}

				entity_row row;
				row.id = id;
				row.classname = std::string(e->classname());
				row.targetname = std::string(e->value("targetname"));
				row.target = std::string(e->value("target"));
				row.model = std::string(e->value("model"));
				row.origin = e->children.empty() ? e->origin() : doc.object_bounds(id).center();

				if (!filter.empty())
				{
					const std::string pattern = "*" + filter + "*";
					if (!edit::wildcard_match(pattern, row.classname) && !edit::wildcard_match(pattern, row.targetname) && !edit::wildcard_match(pattern, row.model)
						&& !edit::wildcard_match(pattern, row.target))
					{
						continue;
					}
				}

				rows.push_back(std::move(row));
			}
		}

		void draw_entity_info()
		{
			auto& st = get();
			auto& s = get_scene();

			if (!st.show_entity_info)
			{
				return;
			}

			ImGui::SetNextWindowPos(ImVec2(860.0f, 90.0f), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(640.0f, 460.0f), ImGuiCond_FirstUseEver);

			if (ImGui::Begin("Entity Info##window", &st.show_entity_info, ImGuiWindowFlags_NoCollapse))
			{
				static std::vector<entity_row> rows;
				static std::uint64_t listed = ~0ull, serial = ~0ull;
				static std::string listed_filter;

				ImGui::SetNextItemWidth(260.0f);
				ImGui::InputTextWithHint("##entity_filter", "class, targetname, target or model", &st.entity_filter);

				if (s.open && (listed != s.doc.change_counter() || serial != s.open_serial || listed_filter != st.entity_filter))
				{
					listed = s.doc.change_counter();
					serial = s.open_serial;
					listed_filter = st.entity_filter;
					list_entities(st.entity_filter, rows);
				}
				else if (!s.open)
				{
					rows.clear();
				}

				ImGui::SameLine();
				ImGui::TextDisabled("%d entities", static_cast<int>(rows.size()));
				ImGui::SameLine();

				if (ImGui::SmallButton("Select all listed") && !rows.empty())
				{
					std::vector<object_id> ids;
					for (const auto& row : rows) { ids.push_back(row.id); }
					s.doc.select(std::span<const object_id>(ids));
				}

				if (ImGui::BeginTable("##entities", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 56.0f);
					ImGui::TableSetupColumn("class");
					ImGui::TableSetupColumn("targetname");
					ImGui::TableSetupColumn("model / target");
					ImGui::TableSetupColumn("origin");
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(static_cast<int>(rows.size()));

					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						{
							const auto& row = rows[static_cast<std::size_t>(i)];
							ImGui::TableNextRow();
							ImGui::TableNextColumn();

							char label[32];
							std::snprintf(label, sizeof(label), "%u", row.id);
							const bool selected = !s.doc.selected_entities().empty() && s.doc.selected_entities()[0] == row.id;

							if (ImGui::Selectable(label, selected, ImGuiSelectableFlags_SpanAllColumns))
							{
								// a click selects and shows it; Ctrl adds
								s.doc.select(row.id, ImGui::GetIO().KeyCtrl ? edit::select_mode::toggle : edit::select_mode::replace);
								focus_selection();
							}

							ImGui::TableNextColumn();
							ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(s.class_color(row.classname)));
							ImGui::TextUnformatted(row.classname.c_str());
							ImGui::PopStyleColor();
							ImGui::TableNextColumn();
							ImGui::TextUnformatted(row.targetname.c_str());
							ImGui::TableNextColumn();
							ImGui::TextUnformatted(row.model.empty() ? row.target.c_str() : row.model.c_str());
							ImGui::TableNextColumn();
							ImGui::Text("%.0f %.0f %.0f", row.origin.x, row.origin.y, row.origin.z);
						}
					}

					ImGui::EndTable();
				}
			}

			ImGui::End();
		}

		// a small window that asks for a few values; `open` is its switch. Returns true while it is shown.
		bool begin_dialog(const char* title, bool& open)
		{
			if (!open)
			{
				return false;
			}

			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (!ImGui::Begin(title, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::End();
				return false;
			}

			return true;
		}

		void draw_dialogs()
		{
			auto& st = get();
			auto& s = get_scene();
			const bool open = s.open;

			if (begin_dialog("Select By Key / Value##dialog", st.open_key_value))
			{
				ImGui::SetNextItemWidth(260.0f);
				ImGui::InputTextWithHint("##key", "key: classname, targetname, model ..", &st.key);
				ImGui::SetNextItemWidth(260.0f);
				const bool enter = ImGui::InputTextWithHint("##value", "value ( * = any text; empty = has the key )", &st.pattern, ImGuiInputTextFlags_EnterReturnsTrue);

				ImGui::BeginDisabled(!open || st.key.empty());
				if (ImGui::Button("Select") || (enter && open && !st.key.empty())) { select_key_value(st.key, st.pattern, false); }
				ImGui::SameLine();
				if (ImGui::Button("Add to the selection")) { select_key_value(st.key, st.pattern, true); }
				ImGui::EndDisabled();
				ImGui::End();
			}

			if (begin_dialog("Arbitrary Rotation##dialog", st.open_rotate))
			{
				ImGui::SetNextItemWidth(260.0f);
				ImGui::InputFloat3("degrees about X, Y, Z", st.rotate_angles, "%.3f");

				ImGui::BeginDisabled(!open || s.doc.selection().empty());
				if (ImGui::Button("Rotate"))
				{
					report(rotate_xyz({ st.rotate_angles[0], st.rotate_angles[1], st.rotate_angles[2] }), "Rotate");
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::TextDisabled("about the middle of the selection, X first");
				ImGui::End();
			}

			if (begin_dialog("Arbitrary Scale##dialog", st.open_scale))
			{
				ImGui::SetNextItemWidth(260.0f);
				ImGui::InputFloat3("factors X, Y, Z", st.scale_factors, "%.4f");
				ImGui::Checkbox("Lock X", &st.scale_lock[0]);
				ImGui::SameLine();
				ImGui::Checkbox("Lock Y", &st.scale_lock[1]);
				ImGui::SameLine();
				ImGui::Checkbox("Lock Z", &st.scale_lock[2]);

				ImGui::BeginDisabled(!open || s.doc.selection().empty());
				if (ImGui::Button("Scale"))
				{
					scale_by({ st.scale_factors[0], st.scale_factors[1], st.scale_factors[2] });
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::TextDisabled("a locked axis keeps its size");
				ImGui::End();
			}

			if (begin_dialog("Find Brush##dialog", st.open_find))
			{
				ImGui::SetNextItemWidth(160.0f);
				ImGui::InputInt("entity number", &st.find_entity);
				ImGui::SetNextItemWidth(160.0f);
				ImGui::InputInt("brush number ( -1: the entity )", &st.find_brush);
				ImGui::TextDisabled("Counted from 0 in the order of the .map file, as \"// entity N\" / \"// brush M\" say.");

				ImGui::BeginDisabled(!open);
				if (ImGui::Button("Find")) { find_brush(st.find_entity, st.find_brush); }
				ImGui::EndDisabled();
				ImGui::End();
			}

			if (begin_dialog("Go To Position##dialog", st.open_goto))
			{
				ImGui::SetNextItemWidth(260.0f);
				ImGui::InputFloat3("x y z", st.goto_position, "%.1f");

				ImGui::BeginDisabled(!open);
				if (ImGui::Button("Go")) { go_to({ st.goto_position[0], st.goto_position[1], st.goto_position[2] }); }
				ImGui::SameLine();
				if (ImGui::Button("Where the camera is"))
				{
					st.goto_position[0] = s.cam.origin.x;
					st.goto_position[1] = s.cam.origin.y;
					st.goto_position[2] = s.cam.origin.z;
				}
				ImGui::EndDisabled();
				ImGui::End();
			}

			if (begin_dialog("Entity Color##dialog", st.open_color))
			{
				const auto ents = open ? s.doc.selected_entities() : std::vector<object_id>();

				if (ImGui::IsWindowAppearing() && !ents.empty())
				{
					// start from the colour the entity has
					edit::vec3 now{ 1, 1, 1 };
					if (edit::parse_vec3(s.doc.find_entity(ents[0])->value("_color", "1 1 1"), now))
					{
						st.color[0] = static_cast<float>(now.x);
						st.color[1] = static_cast<float>(now.y);
						st.color[2] = static_cast<float>(now.z);
					}
				}

				ImGui::BeginDisabled(ents.empty());
				if (ImGui::ColorPicker3("##entity_color", st.color, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoSidePreview))
				{
					set_entity_color(st.color);
				}
				ImGui::EndDisabled();
				ImGui::TextDisabled(ents.empty() ? "Select an entity: the colour goes to its \"_color\" key." : "\"_color\" of the selected entities ( lights: the hue of the light ).");
				ImGui::End();
			}

			if (begin_dialog("Replace Models##dialog", st.open_model_replace))
			{
				if (ImGui::IsWindowAppearing() && open && st.replace_from.empty())
				{
					const auto ents = s.doc.selected_entities();
					if (!ents.empty()) { st.replace_from = std::string(s.doc.find_entity(ents[0])->value("model")); }
				}

				ImGui::SetNextItemWidth(320.0f);
				ImGui::InputTextWithHint("##from", "model to replace ( * = any text )", &st.replace_from);
				ImGui::SetNextItemWidth(320.0f);
				ImGui::InputTextWithHint("##to", "model that takes its place", &st.replace_to);
				ImGui::Checkbox("Only in the selection", &st.replace_selected_only);

				ImGui::BeginDisabled(!open);
				if (ImGui::Button("Replace")) { replace_models(st.replace_from, st.replace_to, st.replace_selected_only); }
				ImGui::EndDisabled();
				ImGui::End();
			}

			if (begin_dialog("Convert Selection To Prefab##dialog", st.open_convert_prefab))
			{
				ImGui::SetNextItemWidth(300.0f);
				const bool enter = ImGui::InputTextWithHint("##prefab_name", "name ( folders with / )", &st.prefab_name, ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::TextDisabled("saved as map_source\\prefabs\\<name>.map; the selection becomes a misc_prefab");

				ImGui::BeginDisabled(!open || s.doc.selection().empty());
				if ((ImGui::Button("Convert") || enter) && convert_to_prefab(st.prefab_name, false))
				{
					st.open_convert_prefab = false;
				}
				ImGui::EndDisabled();
				ImGui::End();
			}
		}

		// ---- lines for the views ----------------------------------------------------------------------

		void add_grid_blocks(std::vector<line_vertex>& out)
		{
			// the 1024 unit blocks of radiant, around the map
			constexpr float block = 1024.0f, reach = 32768.0f;
			constexpr DWORD color = 0xFF4060B0;

			for (float k = -reach; k <= reach; k += block)
			{
				// in the plane of each view: lines along one axis at multiples of the other
				out.push_back({ -reach, k, 0, color }); out.push_back({ reach, k, 0, color });		// along x, steps in y
				out.push_back({ k, -reach, 0, color }); out.push_back({ k, reach, 0, color });		// along y, steps in x
				out.push_back({ -reach, 0, k, color }); out.push_back({ reach, 0, k, color });		// along x, steps in z
				out.push_back({ k, 0, -reach, color }); out.push_back({ k, 0, reach, color });		// along z, steps in x
				out.push_back({ 0, -reach, k, color }); out.push_back({ 0, reach, k, color });		// along y, steps in z
				out.push_back({ 0, k, -reach, color }); out.push_back({ 0, k, reach, color });		// along z, steps in y
			}
		}

		void build_lines(std::vector<line_vertex>& out, const bool view2d)
		{
			const auto& st = get();
			out.clear();

			if (!get_scene().open)
			{
				return;
			}

			if (view2d && st.show_grid_blocks)
			{
				add_grid_blocks(out);
			}

			append_prefab_lines(out);
			out.insert(out.end(), g_region_lines.begin(), g_region_lines.end());

			// the leak: from the entity to the outside
			for (std::size_t i = 0; i + 1 < st.leak.size(); i++)
			{
				const auto& p = st.leak[i];
				const auto& q = st.leak[i + 1];
				const DWORD color = static_cast<int>(i) == st.spot ? 0xFFFFFF40 : 0xFFFF2020;
				out.push_back({ static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z), color });
				out.push_back({ static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z), color });
			}
		}

		// ---- menus ------------------------------------------------------------------------------------

		void not_applicable(const char* label, const char* why)
		{
			editor::menu::item(label, nullptr, false, false);
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("%s", why);
			}
		}

		void draw_selection_menu()
		{
			auto& s = get_scene();
			auto& doc = s.doc;
			auto& st = get();
			auto& ust = ui::get();
			const bool open = s.open;
			const bool any = open && !doc.selection().empty();

			if (editor::menu::begin("Select"))
			{
				if (editor::menu::item("Select Targetname", nullptr, false, open)) { select_same("targetname"); }
				if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Every entity with the targetname of the selected one ( asks when nothing is selected )."); }
				if (editor::menu::item("Select Classname", nullptr, false, open)) { select_same("classname"); }
				if (editor::menu::item("Select By Key / Value ..", editor::menu::key("select.key_value"), false, open)) { st.open_key_value = true; }
				if (editor::menu::item("Select Connected", editor::menu::key("select.connected"), false, any)) { select_connected(); }
				ImGui::Separator();
				if (editor::menu::item("Select Complete Tall", nullptr, false, any)) { select_volume(edit::volume_select::complete_tall); }
				if (editor::menu::item("Select Partial Tall", nullptr, false, any)) { select_volume(edit::volume_select::partial_tall); }
				if (editor::menu::item("Select Touching", nullptr, false, any)) { select_volume(edit::volume_select::touching); }
				if (editor::menu::item("Select Inside", nullptr, false, any)) { select_volume(edit::volume_select::inside); }
				ImGui::Separator();
				if (editor::menu::item("Select All", editor::menu::key("select.all"), false, open)) { select_all(); }
				if (editor::menu::item("Select Whole Entities", editor::menu::key("select.whole_entities"), false, any)) { select_whole_entities(); }
				editor::menu::end();
			}

			if (editor::menu::item("Deselect", editor::menu::key("edit.escape"), false, any)) { deselect(); }
			if (editor::menu::item("Clone", editor::menu::key("edit.clone"), false, any)) { clone_selection(); }
			if (editor::menu::item("Invert", editor::menu::key("select.invert"), false, open)) { invert_selection(); }
			if (editor::menu::item("Delete", editor::menu::key("edit.delete"), false, any)) { delete_selection(); }
			ImGui::Separator();

			if (editor::menu::begin("Flip", any))
			{
				if (editor::menu::item("Flip X")) { flip(0); }
				if (editor::menu::item("Flip Y")) { flip(1); }
				if (editor::menu::item("Flip Z")) { flip(2); }
				editor::menu::end();
			}

			if (editor::menu::begin("Rotate", open))
			{
				if (editor::menu::item("Rotate X", nullptr, false, any)) { rotate(0, 90.0); }
				if (editor::menu::item("Rotate Y", nullptr, false, any)) { rotate(1, 90.0); }
				if (editor::menu::item("Rotate Z", editor::menu::key("select.rotate_z"), false, any)) { rotate(2, 90.0); }
				if (editor::menu::item("Free Rotation", editor::menu::key("gizmo.rotate_toggle"), ust.gizmo == gizmo_tool::rotate)) { ust.gizmo = ust.gizmo == gizmo_tool::rotate ? gizmo_tool::move : gizmo_tool::rotate; }
				if (editor::menu::item("Arbitrary Rotation ..")) { st.open_rotate = true; }
				editor::menu::end();
			}

			if (editor::menu::begin("Scale", open))
			{
				editor::menu::item("Lock X", nullptr, &st.scale_lock[0]);
				editor::menu::item("Lock Y", nullptr, &st.scale_lock[1]);
				editor::menu::item("Lock Z", nullptr, &st.scale_lock[2]);
				if (editor::menu::item("Free Scaling", nullptr, ust.gizmo == gizmo_tool::scale)) { ust.gizmo = ust.gizmo == gizmo_tool::scale ? gizmo_tool::move : gizmo_tool::scale; }
				if (editor::menu::item("Arbitrary Scale ..")) { st.open_scale = true; }
				editor::menu::end();
			}

			if (editor::menu::item("Gizmo: World / Local", nullptr, !ust.gizmo_world)) { ust.gizmo_world = !ust.gizmo_world; }
			ImGui::Separator();

			if (editor::menu::begin("CSG", open))
			{
				if (editor::menu::item("Hollow", nullptr, false, any)) { csg_hollow(); }
				if (editor::menu::item("Subtract", nullptr, false, any)) { csg_subtract(); }
				if (editor::menu::item("Merge", editor::menu::key("csg.merge"), false, any)) { report(doc.csg_merge(), "CSG merge"); }
				if (ImGui::IsItemHovered()) { ImGui::SetTooltip("One brush out of the selected ones, when together they are convex."); }
				if (editor::menu::item("Auto Caulk", editor::menu::key("csg.auto_caulk"), false, any)) { auto_caulk(false); }
				if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Faces of the selected brushes that other brushes cover completely get the material \"caulk\"."); }
				if (editor::menu::item("Auto Caulk The Whole Map")) { auto_caulk(true); }
				editor::menu::end();
			}

			if (editor::menu::begin("Clipper", open))
			{
				bool clipper = ust.clipper.on;
				if (editor::menu::item("Toggle Clipper", editor::menu::key("tool.clipper"), &clipper)) { set_clipper(clipper); }
				if (editor::menu::item("Clip Selected", editor::menu::key("clip.apply"), false, ust.clipper.on)) { apply_clip(0); }
				if (editor::menu::item("Split Selected", editor::menu::key("clip.split"), false, ust.clipper.on)) { apply_clip(2); }
				if (editor::menu::item("Clip, Other Side ( flip )", editor::menu::key("clip.apply_back"), false, ust.clipper.on)) { apply_clip(1); }
				editor::menu::end();
			}

			ImGui::Separator();

			if (editor::menu::begin("Prefabs", open))
			{
				draw_prefab_menu_items();
				editor::menu::end();
			}

			ImGui::Separator();
			if (editor::menu::item("Connect Entities", editor::menu::key("entity.connect"), false, any)) { connect_selected(); }
			if (editor::menu::item("Link Entities", nullptr, false, any)) { report(doc.move_selection_to_entity(), "Link"); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("The selected brushes join the brush entity that has a brush in the selection."); }
			if (editor::menu::item("Ungroup Entity", nullptr, false, any)) { move_to_worldspawn(); }
			if (editor::menu::item("Cycle Group Selection", editor::menu::key("select.next_in_group"), false, any)) { report(doc.select_next_in_group(1), "Cycle group"); }
			ImGui::Separator();

			if (editor::menu::item("Make Detail", editor::menu::key("brush.make_detail"), false, any)) { set_detail(true); }
			if (editor::menu::item("Make Structural", editor::menu::key("brush.make_structural"), false, any)) { set_detail(false); }
			if (editor::menu::item("Make Weapon Clip", editor::menu::key("brush.weapon_clip"), any && all_have("weaponClip", false), any)) { toggle_content("weaponClip", "Weapon clip"); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("\"contents weaponClip;\": the brush stops bullets and grenades, players walk through it. Again: off."); }
			if (editor::menu::item("Make Non-Colliding", editor::menu::key("brush.non_colliding"), any && all_have("nonColliding", false), any)) { toggle_content("nonColliding", "Non-colliding"); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("\"contents nonColliding;\": drawn, but nothing collides with it. Again: off."); }
			ImGui::Separator();
			if (editor::menu::item("Make Split Coplanar Geo", nullptr, any && all_have("splitGeo", true), any)) { report(doc.set_tool_flag("splitGeo", true), "Split coplanar"); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("\"toolFlags splitGeo;\" - kept in the .map; codol_bsp does not merge coplanar faces of different brushes, so it changes nothing there."); }
			if (editor::menu::item("Make Don't Split Coplanar Geo", nullptr, false, any)) { report(doc.set_tool_flag("splitGeo", false), "Don't split coplanar"); }
			ImGui::Separator();

			if (editor::menu::begin("Don't Select"))
			{
				// the same switches as in the Filter window
				auto filter = doc.current_select_filter();
				bool changed = false;
				changed |= editor::menu::item("Curves / Terrain", nullptr, &filter.curves);
				changed |= editor::menu::item("Entities", nullptr, &filter.entities);
				changed |= editor::menu::item("Sky", nullptr, &filter.sky);
				changed |= editor::menu::item("Models", nullptr, &filter.models);
				changed |= editor::menu::item("World Brushes", nullptr, &filter.brushes);

				if (changed)
				{
					command::execute(std::string("ed_noselect curves ") + (filter.curves ? "1" : "0"));
					command::execute(std::string("ed_noselect entities ") + (filter.entities ? "1" : "0"));
					command::execute(std::string("ed_noselect sky ") + (filter.sky ? "1" : "0"));
					command::execute(std::string("ed_noselect models ") + (filter.models ? "1" : "0"));
					command::execute(std::string("ed_noselect brushes ") + (filter.brushes ? "1" : "0"));
				}

				editor::menu::end();
			}
		}

		void draw_grid_menu()
		{
			auto& s = get_scene();
			static const double sizes[] = { 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };
			static const char* keys[] = { nullptr, "`", "1", "2", "3", "4", "5", "6", "7", nullptr, "8", "9" };

			for (std::size_t i = 0; i < std::size(sizes); i++)
			{
				char label[32];
				std::snprintf(label, sizeof(label), "Grid %g", sizes[i]);

				char id[32];
				std::snprintf(id, sizeof(id), "grid.set_%g", sizes[i]);

				// the keys of the grid sizes are actions ( ext.cpp ); 0.25 has none
				if (editor::menu::item(label, actions::find(id) ? editor::menu::key(id) : nullptr, s.grid == sizes[i]))
				{
					s.set_grid(sizes[i]);
				}
			}

			ImGui::Separator();
			if (editor::menu::item("Smaller", editor::menu::key("grid.smaller"))) { grid_smaller(); }
			if (editor::menu::item("Larger", editor::menu::key("grid.larger"))) { grid_larger(); }
			ImGui::Separator();
			editor::menu::item("Snap To Grid", editor::menu::key("grid.snap"), &s.snap);
			if (editor::menu::item("Snap Selection To Grid", editor::menu::key("edit.snap_to_grid"), false, s.open && !s.doc.selection().empty())) { snap_to_grid(); }
			editor::menu::item("Show Grid Blocks ( 1024 )", nullptr, &get().show_grid_blocks);
		}

		void draw_misc_menu()
		{
			auto& s = get_scene();
			auto& st = get();
			const bool open = s.open;
			const bool any = open && !s.doc.selection().empty();

			if (editor::menu::item("Find Brush ..", nullptr, false, open)) { st.open_find = true; }
			if (editor::menu::item("Go To Position ..", nullptr, false, open)) { st.open_goto = true; }
			if (editor::menu::item("-> Leak Spot / Error", editor::menu::key("misc.next_leak"), false, open)) { step_spot(1); }
			if (editor::menu::item("<- Leak Spot / Error", editor::menu::key("misc.previous_leak"), false, open)) { step_spot(-1); }
			ImGui::Separator();
			if (editor::menu::item("Entity Color ..", editor::menu::key("entity.color"), false, open)) { st.open_color = true; }
			if (editor::menu::item("Models Replace ..", nullptr, false, open)) { st.open_model_replace = true; }
			if (editor::menu::item("Cycle Preview Models", nullptr, false, open)) { cycle_model_filter(); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Models shown / hidden ( the \"Models\" switch of the Filter window )."); }
			ImGui::Separator();
			if (editor::menu::item("Drop Selected To Floor", nullptr, false, any)) { drop_to_floor(st.orient_to_floor); }
			editor::menu::item("Orient To Floor When Dropping", nullptr, &st.orient_to_floor);
			editor::menu::item("Plant New Models", nullptr, &st.plant_models);
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("A model that is placed ( Models window, ed_place ) falls onto the brush or terrain below it."); }
			ImGui::Separator();
			not_applicable("Script Group Dialog", "Not applicable: no shipped CoDOL map has a script_group key. Use Selection > Select > By Key / Value for script_noteworthy, script_gameobjectname ..");
			not_applicable("Vehicle Group Dialog", "Not applicable: the 1.1.1.13 multiplayer maps have no vehicle groups ( script_vehiclegroup is in none of them; 22 script_vehicle_* entities over all maps ).");
			not_applicable("Dyn Entity Dialog", "Not applicable: dyn_brushmodel / dyn_model do not occur in the shipped maps ( only 64 dyn_hingeconstraint, edited as keys in the Entity window ).");
			not_applicable("Delete Exportables", "Not applicable: \"exportables\" are brushes marked for CoD4 radiant's Maya export, which this editor does not have.");
		}

		void draw_region_menu()
		{
			auto& s = get_scene();
			const bool open = s.open;
			const bool any = open && !s.doc.selection().empty();

			if (editor::menu::item("Region Off", nullptr, false, open && s.doc.has_region())) { region_off(); }
			if (editor::menu::item("Set XY", nullptr, false, open)) { region_xy(); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("What the top view shows, at any height."); }
			if (editor::menu::item("Set Tall Brush", nullptr, false, any)) { region_from_selection(true, false); }
			if (editor::menu::item("Set Brush", nullptr, false, any)) { region_from_selection(false, false); }
			if (editor::menu::item("Set Selected Brush", nullptr, false, any)) { region_from_selection(false, true); }
			ImGui::Separator();
			if (editor::menu::item("Save Region ..", nullptr, false, open))
			{
				if (const auto file = file_dialog(true, map_source_dir() / (to_utf8(s.map_file.stem()) + "_region.map")); !file.empty()) { save_region(file); }
			}
			if (editor::menu::item("Build Region", nullptr, false, open)) { get().region_build = true; build_map(false); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Build map with only what the region shows, inside a box of caulk ( compiled\\region\\<map>.map )."); }
			if (editor::menu::item("Build Region, Fast", nullptr, false, open)) { get().region_build = true; build_map(true); }
		}
	}

	// ---- shared with filters.cpp / prefabs.cpp ------------------------------------------------------------

	void focus_selection()
	{
		auto& s = get_scene();
		s.look_at_selection();

		if (const auto box = s.doc.selection_bounds(); box.valid())
		{
			center_2d(box.center());
		}
	}

	void go_to(const edit::vec3& position)
	{
		auto& s = get_scene();
		s.cam.origin = { static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z) };
		center_2d(position);
		game::printf_to_console("camera at %g %g %g\n", position.x, position.y, position.z);
	}
}

namespace editor::ui
{
	using namespace ext;

	// ---- menus: called from the menu bar ( main.cpp ) -------------------------------------------------------

	void draw_file_menu_extra()
	{
		auto& s = get_scene();
		const bool open = s.open;
		const bool any = open && !s.doc.selection().empty();

		ImGui::Separator();

		if (editor::menu::item("Import .map into the map ..", nullptr, false, open))
		{
			if (const auto file = file_dialog(false, map_source_dir() / ""); !file.empty()) { import_map(file); }
		}

		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Brushes of the file's worldspawn join this worldspawn, its entities are added. One undo step."); }

		if (editor::menu::item("Unpack the game's scripts again", nullptr, false, !codol::packed_scripts::archive_path().empty()))
		{
			std::string error;
			const int n = codol::packed_scripts::extract_all({}, &error);
			if (n < 0) { ImGui::Toast(ImGuiToastType_Error, "Scripts", error.c_str(), 6000); }
			else { ImGui::Toast(ImGuiToastType_Success, "Scripts", (std::to_string(n) + " scripts unpacked to " + to_utf8(codol::packed_scripts::reference_root())).c_str(), 5000); }
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("The scripts of <game>\\main\\sh1x_svr_scripts.iwd, unpacked to packed_scripts\\ next to the editor: a read-only reference\n"
				"( the Script Workbench shows it ). This is done by itself when the archive changes. A script saved from it goes to the\n"
				"script folder ( main\\sh1x_rawfiles ), which the server lays over the archive. Console: scripts_extract [folder], scripts_unpack <path>.");
		}

		if (editor::menu::begin("Generate File", open))
		{
			if (editor::menu::item("Save Selected ..", nullptr, false, any))
			{
				if (const auto file = file_dialog(true, map_source_dir() / "selection.map"); !file.empty()) { save_selected(file); }
			}

			if (editor::menu::item("Save Region ..", nullptr, false, open))
			{
				if (const auto file = file_dialog(true, map_source_dir() / (to_utf8(s.map_file.stem()) + "_region.map")); !file.empty()) { save_region(file); }
			}

			if (editor::menu::item("Createfx")) { createfx_info(); }
			if (ImGui::IsItemHovered()) { ImGui::SetTooltip("The createfx script is written by File > Build map from the script_fx / script_sound entities."); }
			if (editor::menu::item("Pointfile ( leak line of the last build )")) { load_leak({}); }
			if (editor::menu::item("Errorfile ( brushes the compiler leaves out )")) { collect_errors(); }
			if (editor::menu::item("Clear leak line / errors", nullptr, false, !ext::get().leak.empty() || !ext::get().errors.empty()))
			{
				ext::get().leak.clear();
				ext::get().errors.clear();
				ext::get().spot = -1;
			}

			editor::menu::end();
		}

		if (editor::menu::item("Build region ( what the region shows )", nullptr, false, open && s.doc.has_region()))
		{
			ext::get().region_build = true;
			build_map(true);
		}
	}

	void draw_edit_menu_extra()
	{
		auto& s = get_scene();
		const bool open = s.open;
		const bool any = open && !s.doc.selection().empty();

		ImGui::Separator();
		if (editor::menu::item("Copy", editor::menu::key("edit.copy"), false, any)) { copy_selection(false); }
		if (editor::menu::item("Cut", editor::menu::key("edit.cut"), false, any)) { copy_selection(true); }
		if (editor::menu::item("Paste", editor::menu::key("edit.paste"), false, open)) { paste_clipboard(false); }
		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Where the objects were copied from. The clipboard holds .map text: it also goes from one CoDOL Radiant to another."); }
		if (editor::menu::item("Paste At Cursor / Camera", editor::menu::key("edit.paste_here"), false, open)) { paste_clipboard(true); }
		ImGui::Separator();
		editor::menu::item("Map Info", editor::menu::key("window.map_info"), &ext::get().show_map_info);
		editor::menu::item("Entity Info", nullptr, &ext::get().show_entity_info);
		ImGui::Separator();
		if (editor::menu::item("Enter Prefab", nullptr, false, prefab_selected())) { enter_prefab(); }
		if (editor::menu::item("Leave Prefab", nullptr, false, prefab_depth() > 0)) { leave_prefab(); }
	}

	void draw_view_menu_extra()
	{
		auto& s = get_scene();
		auto& st = ext::get();
		const bool open = s.open;
		const bool any = open && !s.doc.selection().empty();

		ImGui::Separator();
		editor::menu::item("Filter", editor::menu::key("filter.window"), &st.show_filter);
		editor::menu::item("Map Info", editor::menu::key("window.map_info"), &st.show_map_info);
		editor::menu::item("Entity Info", nullptr, &st.show_entity_info);
		editor::menu::item("Prefab Browser", nullptr, &st.show_prefab_browser);
		ImGui::Separator();
		if (editor::menu::item("Center 2D On Camera", nullptr, false, open)) { center_2d_on_camera(); }
		if (editor::menu::item("Center Camera On Selection", nullptr, false, any)) { focus_selection(); }
		if (editor::menu::item("Up Floor", editor::menu::key("view.floor_up"), false, open)) { change_floor(true); }
		if (editor::menu::item("Down Floor", editor::menu::key("view.floor_down"), false, open)) { change_floor(false); }
		ImGui::Separator();
		if (editor::menu::item("Hide Selected", editor::menu::key("view.hide_selected"), false, any)) { hide_selected(); }
		if (editor::menu::item("Hide Unselected", editor::menu::key("view.hide_unselected"), false, any)) { s.doc.hide_unselected(); }
		if (editor::menu::item("Show Last Hidden", editor::menu::key("view.show_last_hidden"), false, open)) { s.doc.show_last_hidden(); }
		if (editor::menu::item("Show Hidden", editor::menu::key("view.show_all"), false, open)) { show_all(); }
		ImGui::Separator();
		editor::menu::item("Show Prefab Contents", nullptr, &st.show_prefab_contents);
		editor::menu::item("Show Grid Blocks ( 2D )", nullptr, &st.show_grid_blocks);
	}

	void draw_extra_menus()
	{
		if (editor::menu::begin("Selection"))
		{
			draw_selection_menu();
			editor::menu::end();
		}

		if (editor::menu::begin("Grid"))
		{
			draw_grid_menu();
			editor::menu::end();
		}

		if (editor::menu::begin("Misc"))
		{
			draw_misc_menu();
			editor::menu::end();
		}

		if (editor::menu::begin("Region"))
		{
			draw_region_menu();
			editor::menu::end();
		}

		// where one is: inside a prefab, inside a region
		draw_prefab_breadcrumb();

		if (get_scene().open && get_scene().doc.has_region())
		{
			ImGui::TextColored(ImVec4(0.91f, 0.75f, 0.13f, 1.0f), "  region");
		}
	}

	// ---- per frame ------------------------------------------------------------------------------------------

	void draw_extra_windows()
	{
		apply_filters();
		prefab_frame();
		plant_new_models();

		draw_filter_window();
		draw_prefab_browser();
		draw_map_info();
		draw_entity_info();
		draw_dialogs();
	}

	const std::vector<line_vertex>& extra_view_lines(const bool view2d)
	{
		static std::vector<line_vertex> lines[2];
		static int built[2] = { -1, -1 };
		const int which = view2d ? 1 : 0;

		if (built[which] != ImGui::GetFrameCount())
		{
			built[which] = ImGui::GetFrameCount();
			build_lines(lines[which], view2d);
		}

		return lines[which];
	}

	// File > Build map compiles this file: the map itself, or - once, after "Build region" - what the region shows
	// inside a box of caulk
	std::filesystem::path build_source_file(const std::filesystem::path& map_file)
	{
		auto& st = ext::get();
		if (!st.region_build)
		{
			return map_file;
		}

		st.region_build = false;
		auto& doc = get_scene().doc;
		const auto file = std::filesystem::u8path(utils::fs::get_basepath()) / "compiled" / "region" / map_file.filename();
		std::string error;

		if (!write_text(file, doc.region_map_text(true, edit::CAULK_MATERIAL), error))
		{
			game::printf_to_console("^1[CoDOL] build region: %s; the whole map is built\n", error.c_str());
			return map_file;
		}

		game::printf_to_console("[CoDOL] build region: %s inside a box of caulk -> %s\n", doc.has_region() ? "what the region shows" : "no region is set, the whole map", to_utf8(file).c_str());
		return file;
	}

	// ---- shortcuts ------------------------------------------------------------------------------------------

	// the keys of these menus are actions: action_table.cpp

	// ---- console commands -----------------------------------------------------------------------------------

	void register_extra_commands()
	{
		// ---- files
		command::register_command("map_save_selected", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: map_save_selected <file.map>\n"); return; }
			save_selected(std::filesystem::u8path(args[1]));
		});

		command::register_command("map_save_region", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: map_save_region <file.map>\n"); return; }
			save_region(std::filesystem::u8path(args[1]));
		});

		// map_import <file.map>: the objects of another map into this one
		command::register_command("map_import", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: map_import <file.map>\n"); return; }
			import_map(std::filesystem::u8path(args[1]));
		});

		// map_build_region [fast]: File > Build map of what the region shows
		command::register_command("map_build_region", [](args_t args)
		{
			if (!need_map()) { return; }
			ext::get().region_build = true;
			build_map(args.size() > 1 && args[1] == "fast");
		});

		command::register_command("map_createfx", [](args_t) { if (need_map()) { createfx_info(); } });

		// leak_load [report file]: the leak line of the last build; leak_next / leak_prev walk along it ( and through
		// the brushes of map_errors ); leak_clear takes the line away
		command::register_command("leak_load", [](args_t args)
		{
			if (need_map()) { load_leak(args.size() > 1 ? std::filesystem::u8path(args[1]) : std::filesystem::path()); }
		});

		command::register_command("leak_next", [](args_t) { if (need_map()) { step_spot(1); } });
		command::register_command("leak_prev", [](args_t) { if (need_map()) { step_spot(-1); } });

		command::register_command("leak_clear", [](args_t)
		{
			ext::get().leak.clear();
			ext::get().errors.clear();
			ext::get().spot = -1;
		});

		command::register_command("map_errors", [](args_t) { if (need_map()) { collect_errors(); } });

		// ---- clipboard
		command::register_command("ed_copy", [](args_t) { if (need_map()) { copy_selection(false); } });
		command::register_command("ed_cut", [](args_t) { if (need_map()) { copy_selection(true); } });

		// ed_paste [here | x y z]: where the objects were, where the camera looks ( the mouse in a 2D view ), or at a position
		command::register_command("ed_paste", [](args_t args)
		{
			if (!need_map()) { return; }

			if (args.size() > 3)
			{
				const edit::vec3 at = vector(args, 1);
				print_result(paste_clipboard(true, &at), "paste");
			}
			else
			{
				print_result(paste_clipboard(args.size() > 1 && args[1] == "here"), "paste");
			}
		});

		// ---- windows
		command::register_command("ed_mapinfo", [](args_t args)
		{
			if (!need_map()) { return; }
			print_map_info();
			if (args.size() > 1 && args[1] == "window") { ext::get().show_map_info = true; }
		});

		// ed_entinfo [filter] [window]: the entities whose class, targetname, target or model holds the text
		command::register_command("ed_entinfo", [](args_t args)
		{
			if (!need_map()) { return; }
			std::vector<entity_row> rows;
			const std::string filter = args.size() > 1 && args[1] != "window" ? args[1] : "";
			list_entities(filter, rows);

			for (std::size_t i = 0; i < rows.size() && i < 30; i++)
			{
				const auto& row = rows[i];
				game::printf_to_console("  #%u %s  targetname \"%s\"  target \"%s\"  model \"%s\"  at %.0f %.0f %.0f\n", row.id, row.classname.c_str(), row.targetname.c_str(), row.target.c_str(),
					row.model.c_str(), row.origin.x, row.origin.y, row.origin.z);
			}

			game::printf_to_console("%d entities%s\n", static_cast<int>(rows.size()), rows.size() > 30 ? " ( the first 30 are listed )" : "");

			if (args.size() > 1 && args.back() == "window")
			{
				ext::get().show_entity_info = true;
				ext::get().entity_filter = filter;
			}
		});

		// ed_window <filter|mapinfo|entinfo|prefabs> [0|1]
		command::register_command("ed_window", [](args_t args)
		{
			auto& st = ext::get();
			if (args.size() < 2) { game::printf_to_console("usage: ed_window <filter|mapinfo|entinfo|prefabs> [0|1]\n"); return; }

			bool* flag = args[1] == "filter" ? &st.show_filter : (args[1] == "mapinfo" ? &st.show_map_info : (args[1] == "entinfo" ? &st.show_entity_info
				: (args[1] == "prefabs" ? &st.show_prefab_browser : nullptr)));

			if (flag) { *flag = args.size() > 2 ? number(args, 2) != 0.0 : !*flag; }
		});

		// ---- select
		// sel_key <key> [pattern] [add]: entities whose key matches ( * = any text; no pattern: the key exists )
		command::register_command("sel_key", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: sel_key <key> [pattern] [add]\n"); return; }
			select_key_value(args[1], args.size() > 2 ? args[2] : "", args.size() > 3);
		});

		command::register_command("sel_targetname", [](args_t args)
		{
			if (!need_map()) { return; }
			if (args.size() > 1) { select_key_value("targetname", args[1], args.size() > 2); } else { select_same("targetname"); }
		});

		command::register_command("sel_classname", [](args_t args)
		{
			if (!need_map()) { return; }
			if (args.size() > 1) { select_key_value("classname", args[1], args.size() > 2); } else { select_same("classname"); }
		});

		command::register_command("sel_connected", [](args_t) { if (need_map()) { select_connected(); } });

		// sel_volume <touching|inside|tall|partial> [x|y|z]: the selected brushes mark the volume
		command::register_command("sel_volume", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: sel_volume <touching|inside|tall|partial> [x|y|z]\n"); return; }
			const auto how = args[1] == "inside" ? edit::volume_select::inside : (args[1] == "tall" ? edit::volume_select::complete_tall
				: (args[1] == "partial" ? edit::volume_select::partial_tall : edit::volume_select::touching));
			print_result(get_scene().doc.select_by_volume(how, args.size() > 2 ? axis_of(args[2]) : 2), "select");
		});

		command::register_command("sel_touching", [](args_t) { if (need_map()) { print_result(get_scene().doc.select_by_volume(edit::volume_select::touching), "select touching"); } });
		command::register_command("sel_inside", [](args_t) { if (need_map()) { print_result(get_scene().doc.select_by_volume(edit::volume_select::inside), "select inside"); } });
		command::register_command("sel_tall", [](args_t) { if (need_map()) { print_result(get_scene().doc.select_by_volume(edit::volume_select::complete_tall), "select complete tall"); } });
		command::register_command("sel_partial_tall", [](args_t) { if (need_map()) { print_result(get_scene().doc.select_by_volume(edit::volume_select::partial_tall), "select partial tall"); } });

		// sel_group [prev]: the next brush of the brush entity
		command::register_command("sel_group", [](args_t args)
		{
			if (need_map()) { print_result(get_scene().doc.select_next_in_group(args.size() > 1 && args[1] == "prev" ? -1 : 1), "cycle group"); }
		});

		// ---- transform
		command::register_command("ed_flip", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: ed_flip <x|y|z>\n"); return; }
			auto& s = get_scene();
			print_result(s.doc.flip(s.selection_pivot(), axis_of(args[1])), "flip");
		});

		// ed_rotate3 <x> <y> <z>: degrees about each axis, one step ( the Arbitrary Rotation dialog )
		command::register_command("ed_rotate3", [](args_t args)
		{
			if (!need_map() || args.size() < 4) { game::printf_to_console("usage: ed_rotate3 <x> <y> <z>\n"); return; }
			print_result(rotate_xyz(vector(args, 1)), "rotate");
		});

		// ed_scale_lock [xyz letters | none]: axes ed_scale3 and the dialog leave alone
		command::register_command("ed_scale_lock", [](args_t args)
		{
			auto& st = ext::get();
			if (args.size() > 1)
			{
				for (int axis = 0; axis < 3; axis++) { st.scale_lock[axis] = args[1].find("xyz"[axis]) != std::string::npos; }
			}

			game::printf_to_console("scale lock:%s%s%s%s\n", st.scale_lock[0] ? " x" : "", st.scale_lock[1] ? " y" : "", st.scale_lock[2] ? " z" : "",
				st.scale_lock[0] || st.scale_lock[1] || st.scale_lock[2] ? "" : " none");
		});

		// ed_scale3 <fx> [fy] [fz]: the Arbitrary Scale dialog ( locked axes keep their size )
		command::register_command("ed_scale3", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: ed_scale3 <fx> [fy] [fz]\n"); return; }
			const double f = number(args, 1, 1.0);
			print_result(scale_by({ f, number(args, 2, f), number(args, 3, f) }), "scale");
		});

		// ---- brushes
		command::register_command("ed_merge", [](args_t) { if (need_map()) { print_result(get_scene().doc.csg_merge(), "csg merge"); } });

		// ed_autocaulk [all]: hidden faces of the selected brushes ( of every brush ) get caulk
		command::register_command("ed_autocaulk", [](args_t args) { if (need_map()) { auto_caulk(args.size() > 1 && args[1] == "all"); } });

		// ed_contents <weaponClip|nonColliding|detail|..> [0|1]: one word of the "contents ..;" line; without a value: toggle
		command::register_command("ed_contents", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: ed_contents <weaponClip|nonColliding|detail> [0|1]\n"); return; }
			const bool on = args.size() > 2 ? number(args, 2) != 0.0 : !all_have(args[1], false);
			print_result(get_scene().doc.set_content_flag(args[1], on), on ? "contents on" : "contents off");
		});

		// ed_toolflag <splitGeo> [0|1]
		command::register_command("ed_toolflag", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: ed_toolflag <splitGeo> [0|1]\n"); return; }
			const bool on = args.size() > 2 ? number(args, 2) != 0.0 : !all_have(args[1], true);
			print_result(get_scene().doc.set_tool_flag(args[1], on), on ? "tool flag on" : "tool flag off");
		});

		// ed_link: the selected brushes join the brush entity that has a brush in the selection
		command::register_command("ed_link", [](args_t) { if (need_map()) { print_result(get_scene().doc.move_selection_to_entity(), "link"); } });

		// ---- view
		command::register_command("ed_hide_unselected", [](args_t) { if (need_map()) { get_scene().doc.hide_unselected(); } });
		command::register_command("ed_show_last", [](args_t) { if (need_map()) { get_scene().doc.show_last_hidden(); } });
		command::register_command("view2d_camera", [](args_t) { if (need_map()) { center_2d_on_camera(); } });

		command::register_command("ed_floor", [](args_t args)
		{
			if (need_map()) { change_floor(!(args.size() > 1 && args[1] == "down")); }
		});

		// ed_gridblocks [0|1]: the 1024 unit blocks in the 2D views
		command::register_command("ed_gridblocks", [](args_t args)
		{
			ext::get().show_grid_blocks = args.size() > 1 ? number(args, 1) != 0.0 : !ext::get().show_grid_blocks;
		});

		// ---- region
		// region <off | xy | tall | brush | selected | x1 y1 z1 x2 y2 z2>
		command::register_command("region", [](args_t args)
		{
			if (!need_map()) { return; }
			const std::string what = args.size() > 1 ? args[1] : "";

			if (what == "off") { region_off(); }
			else if (what == "xy") { region_xy(); }
			else if (what == "tall") { region_from_selection(true, false); }
			else if (what == "brush") { region_from_selection(false, false); }
			else if (what == "selected") { region_from_selection(false, true); }
			else if (args.size() > 6)
			{
				edit::bounds box;
				box.add(vector(args, 1));
				box.add(vector(args, 4));
				set_region(box, "box");
			}
			else
			{
				const auto box = get_scene().doc.region();
				if (get_scene().doc.has_region()) { game::printf_to_console("region %g %g %g .. %g %g %g\n", box.mins.x, box.mins.y, box.mins.z, box.maxs.x, box.maxs.y, box.maxs.z); }
				else { game::printf_to_console("no region. usage: region <off | xy | tall | brush | selected | x1 y1 z1 x2 y2 z2>\n"); }
			}
		});

		// ---- misc
		// ed_find <entity number> [brush number]: as the .map counts them
		command::register_command("ed_find", [](args_t args)
		{
			if (!need_map() || args.size() < 2) { game::printf_to_console("usage: ed_find <entity number> [brush number]\n"); return; }
			find_brush(std::atoi(args[1].c_str()), args.size() > 2 ? std::atoi(args[2].c_str()) : -1);
		});

		command::register_command("ed_goto", [](args_t args)
		{
			if (!need_map() || args.size() < 4) { game::printf_to_console("usage: ed_goto <x> <y> <z>\n"); return; }
			go_to(vector(args, 1));
		});

		// ed_color <r> <g> <b>: "_color" of the selected entities
		command::register_command("ed_color", [](args_t args)
		{
			if (!need_map() || args.size() < 4) { game::printf_to_console("usage: ed_color <r> <g> <b>   ( 0 .. 1 )\n"); return; }
			const float color[3] = { static_cast<float>(number(args, 1)), static_cast<float>(number(args, 2)), static_cast<float>(number(args, 3)) };
			print_result(set_entity_color(color), "entity color");
		});

		// ed_model_replace <from> <to> [selected]: "model" of every entity ( of the selected ones ); * in <from>
		command::register_command("ed_model_replace", [](args_t args)
		{
			if (!need_map() || args.size() < 3) { game::printf_to_console("usage: ed_model_replace <from> <to> [selected]\n"); return; }
			replace_models(args[1], args[2], args.size() > 3 && args[3] == "selected");
		});

		command::register_command("ed_models_cycle", [](args_t) { if (need_map()) { cycle_model_filter(); } });

		// ed_drop [orient]: the selected point entities fall onto the brush / terrain below
		command::register_command("ed_drop", [](args_t args)
		{
			if (need_map()) { drop_to_floor(ext::get().orient_to_floor || (args.size() > 1 && args[1] == "orient")); }
		});

		// ed_plant [0|1] [orient 0|1]: new models fall to the floor ( and turn to it )
		command::register_command("ed_plant", [](args_t args)
		{
			auto& st = ext::get();
			if (args.size() > 1) { st.plant_models = number(args, 1) != 0.0; }
			if (args.size() > 2) { st.orient_to_floor = number(args, 2) != 0.0; }
			game::printf_to_console("plant new models: %s, orient to floor: %s\n", st.plant_models ? "on" : "off", st.orient_to_floor ? "on" : "off");
		});

		register_filter_commands();
		register_prefab_commands();
	}
}
