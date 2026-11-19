#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "codol/packed_scripts.hpp"
#include "assets.hpp"
#include "preview.hpp"
#include "scene.hpp"
#include "ext.hpp"
#include "ui.hpp"
#include <codol_edit/patch_ops.hpp>

#include <charconv>

// The scene: files, and the state based sync between the document and the renderer ( see scene.hpp ).

namespace editor
{
	scene& get_scene()
	{
		static scene s;
		return s;
	}

	namespace
	{
		std::uint64_t ref_key(const view::object_ref& ref)
		{
			return (static_cast<std::uint64_t>(ref.kind) << 32) | static_cast<std::uint32_t>(ref.id);
		}

		bool is_xmodel_name(const std::string_view model)
		{
			// "model" of a misc_prefab is a .map, not an xmodel
			return !model.empty() && model[0] != '*' && model[0] != '?' && model.find(".map") == std::string_view::npos;
		}

		bool read_file(const std::filesystem::path& file, std::string& out)
		{
			std::ifstream in(file, std::ios::binary);
			if (!in)
			{
				return false;
			}

			out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
			return true;
		}

		bool write_file(const std::filesystem::path& file, const std::string_view text)
		{
			std::ofstream out(file, std::ios::binary | std::ios::trunc);
			if (!out)
			{
				return false;
			}

			out.write(text.data(), static_cast<std::streamsize>(text.size()));
			return out.good();
		}

		std::filesystem::path sidecar(const std::filesystem::path& map_file, const char* extension)
		{
			auto p = map_file;
			p.replace_extension(extension);
			return p;
		}

		int int_value(const std::string_view text, const int fallback)
		{
			int v = fallback;
			const auto r = std::from_chars(text.data(), text.data() + text.size(), v);
			return r.ec == std::errc() ? v : fallback;
		}

		float3 to_float3(const edit::vec3& v)
		{
			return { static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z) };
		}

		// colour of a brush made of a material the renderer does not have ( tool textures of the collision map )
		std::uint32_t fallback_color(const std::string_view material)
		{
			std::uint32_t h = 2166136261u;
			for (const char c : material)
			{
				h = (h ^ static_cast<std::uint8_t>(c)) * 16777619u;
			}

			const std::uint32_t r = 110 + (h & 0x3F), g = 110 + ((h >> 6) & 0x3F), b = 110 + ((h >> 12) & 0x3F);
			return 0xFF000000u | (r << 16) | (g << 8) | b;
		}

		// The item of `id` in a cache that was built in document order: at the cursor, or a few items on when
		// something in between is gone. ( What is new, or moved far, is simply built again. )
		template <typename T>
		const T* find_cached(const std::vector<T>& items, std::size_t& cursor, const object_id id)
		{
			for (std::size_t i = cursor; i < items.size() && i < cursor + 16; i++)
			{
				if (items[i].id == id)
				{
					cursor = i + 1;
					return &items[i];
				}
			}

			return nullptr;
		}

		bool same_vec(const edit::vec3& a, const edit::vec3& b)
		{
			return std::fabs(a.x - b.x) < 1e-4 && std::fabs(a.y - b.y) < 1e-4 && std::fabs(a.z - b.z) < 1e-4;
		}
	}

	scene::scene()
	{
		// uv of new and edited brushes need the size of the colour map
		doc.set_texture_size_callback([this](const std::string_view material, int& w, int& h)
		{
			const std::string name(material);

			if (const auto it = texture_sizes_.find(name); it != texture_sizes_.end())
			{
				w = it->second.first;
				h = it->second.second;
				return w > 0;
			}

			std::pair<int, int> size{ 0, 0 };

			if (renderer.loaded())
			{
				view::material_info info;
				if ((renderer.material_details(name, info) || renderer.material_details("wc/" + name, info)) && info.width > 0 && info.height > 0)
				{
					size = { info.width, info.height };
				}
			}

			texture_sizes_.emplace(name, size);
			w = size.first;
			h = size.second;
			return w > 0;
		});

		// box of a point entity: what the renderer draws for it, else the class catalog
		doc.set_entity_box_callback([this](const edit::entity& e, edit::bounds& box)
		{
			// a prefab: the box of what it holds
			if (ui::prefab_box(e, box))
			{
				return true;
			}

			const auto model = e.value("model");

			// the box of the object in the zone, as long as the entity is turned and scaled as it is there
			if (const auto it = links_.find(e.id); it != links_.end() && it->second.has_box && model == it->second.base_model
				&& (it->second.brush_model || (same_vec(e.angles(), it->second.base_angles) && std::fabs(model_scale(e) - it->second.base_scale) < 1e-6)))
			{
				box = it->second.local_box;
				return true;
			}

			float3 mins, maxs;
			if (!is_xmodel_name(model) || !model_box(model, mins, maxs))
			{
				return false;
			}

			// the box of the model, turned by the entity's angles ( the result is axis aligned again )
			edit::vec3 axes[3];
			edit::angles_to_axes(e.angles(), axes);
			const double s = model_scale(e);

			for (int i = 0; i < 8; i++)
			{
				const double x = ((i & 1) ? maxs.x : mins.x) * s, y = ((i & 2) ? maxs.y : mins.y) * s, z = ((i & 4) ? maxs.z : mins.z) * s;
				box.add(axes[0] * x + axes[1] * y + axes[2] * z);
			}

			return true;
		});
	}

	bool scene::model_box(const std::string_view model, float3& mins, float3& maxs)
	{
		if (!renderer.loaded())
		{
			return false;
		}

		const std::string name(model);

		if (const auto it = model_boxes_.find(name); it != model_boxes_.end())
		{
			mins = it->second.first;
			maxs = it->second.second;
			return true;
		}

		if (no_model_box_.contains(name))
		{
			return false;
		}

		if (!renderer.model_bounds(name, mins, maxs))
		{
			no_model_box_.insert(name);
			return false;
		}

		model_boxes_.emplace(name, std::make_pair(mins, maxs));
		return true;
	}

	double scene::model_scale(const edit::entity& e)
	{
		const auto text = e.value("modelscale");
		if (text.empty())
		{
			return 1.0;
		}

		const double v = std::atof(std::string(text).c_str());
		return v > 0.0 ? v : 1.0;
	}

	ImU32 scene::class_color(const std::string_view classname) const
	{
		if (const auto* info = edit::class_catalog::c1().find(classname); info && info->has_box)
		{
			return IM_COL32(static_cast<int>(info->color[0] * 255.0f), static_cast<int>(info->color[1] * 255.0f), static_cast<int>(info->color[2] * 255.0f), 255);
		}

		// the catalog has no hand written colour for most classes: one per family
		static constexpr std::pair<std::string_view, ImU32> families[] = {
			{ "trigger_", IM_COL32(240, 150, 50, 255) },
			{ "light", IM_COL32(250, 230, 90, 255) },
			{ "mp_", IM_COL32(90, 220, 110, 255) },
			{ "info_", IM_COL32(110, 200, 130, 255) },
			{ "script_model", IM_COL32(90, 200, 230, 255) },
			{ "script_", IM_COL32(110, 160, 240, 255) },
			{ "misc_", IM_COL32(150, 170, 200, 255) },
			{ "codol_", IM_COL32(200, 120, 220, 255) },
		};

		for (const auto& [prefix, color] : families)
		{
			if (classname.rfind(prefix, 0) == 0)
			{
				return color;
			}
		}

		return colors::get(colors::entity);
	}

	// ---- files ------------------------------------------------------------------------------------------

	namespace
	{
		constexpr const char* STAGE_LIGHT_KEYS[] = { "sundirection", "suncolor", "sunlight", "ambient", "_color", "skylight", "skycolor" };

		std::filesystem::path stage_dir()
		{
			return std::filesystem::u8path(utils::fs::get_basepath()) / "compiled" / "cache" / "stage";
		}

		// the light keys of a worldspawn as one text: what a stage is built from and named after
		std::string stage_light_keys(const edit::document& doc)
		{
			std::string text;

			if (const auto* world = doc.find_entity(doc.worldspawn()))
			{
				for (const char* key : STAGE_LIGHT_KEYS)
				{
					if (const auto* value = world->find(key))
					{
						text += std::string(key) + "=" + *value + ";";
					}
				}
			}

			return text;
		}

		std::string stage_name(const std::string& keys)
		{
			// FNV-1a: a stage per set of light keys stays in the cache
			std::uint32_t hash = 2166136261u;
			for (const char c : keys)
			{
				hash = (hash ^ static_cast<std::uint8_t>(c)) * 16777619u;
			}

			char name[40];
			std::snprintf(name, sizeof(name), "codol_stage_%08x", hash);
			return name;
		}
	}

	// The stage: a zone the renderer can load for a map that has none. A closed box of 128 units far below the
	// origin with the light keys of the map's worldspawn, compiled by codol_bsp ( a second or two, once per set of
	// light keys ): it brings the sun, the sky light and the ambient light the map will have, and nothing to look at.
	bool scene::load_stage(std::string& error)
	{
		const auto base = std::filesystem::u8path(utils::fs::get_basepath());
		const auto install = std::filesystem::u8path(components::codol_game::install_path());
		const auto dir = stage_dir();
		const std::string keys = stage_light_keys(doc);
		const std::string name = stage_name(keys);
		const auto zone = dir / (name + ".ff");
		std::error_code ec;
		std::filesystem::create_directories(dir, ec);

		if (!std::filesystem::is_regular_file(zone, ec))
		{
			const auto tool = base / "codol_bsp.exe";
			const auto exe = install / "codoMP_client_shipRetail.exe";

			if (!std::filesystem::is_regular_file(tool, ec) || !std::filesystem::is_regular_file(exe, ec))
			{
				error = "the 3D view of a map of your own needs codol_bsp.exe next to the editor and codoMP_client_shipRetail.exe in the install ( File > Settings )";
				return false;
			}

			edit::document stage_doc;
			constexpr double z = -30000.0, r = 64.0, t = 16.0;
			const char* material = "wc/me_metal_rust_03";
			stage_doc.create_box({ -r, -r, z - r - t }, { r, r, z - r }, material);
			stage_doc.create_box({ -r, -r, z + r }, { r, r, z + r + t }, material);
			stage_doc.create_box({ -r - t, -r, z - r }, { -r, r, z + r }, material);
			stage_doc.create_box({ r, -r, z - r }, { r + t, r, z + r }, material);
			stage_doc.create_box({ -r, -r - t, z - r }, { r, -r, z + r }, material);
			stage_doc.create_box({ -r, r, z - r }, { r, r + t, z + r }, material);
			stage_doc.create_point_entity("mp_global_intermission", { 0.0, 0.0, z });
			stage_doc.create_point_entity("mp_dm_spawn", { 0.0, 0.0, z - r + 8.0 });

			if (const auto* world = doc.find_entity(doc.worldspawn()))
			{
				for (const char* key : STAGE_LIGHT_KEYS)
				{
					if (const auto* value = world->find(key))
					{
						stage_doc.set_key(stage_doc.worldspawn(), key, *value);
					}
				}
			}

			const auto map = dir / (name + ".map");
			std::string text;

			if (!stage_doc.save_map_file(map, text))
			{
				error = "could not write " + to_utf8(map) + ": " + text;
				return false;
			}

			std::filesystem::create_directories(base / "compiled" / "cache", ec);
			const std::string command = "\"" + to_utf8(tool) + "\" --map \"" + to_utf8(map) + "\" --exe \"" + to_utf8(exe) + "\" --install \"" + to_utf8(install)
				+ "\" --out \"" + to_utf8(zone) + "\" --name " + name + " --cache \"" + to_utf8(base / "compiled" / "cache") + "\" --fast";

			game::printf_to_console("[CoDOL] 3D view: building the stage of this map's light ( %s ) ..\n", name.c_str());
			std::string output;
			const int code = components::codol_game::run_command(command, to_utf8(base), output);

			if (code != 0 || !std::filesystem::is_regular_file(zone, ec))
			{
				game::printf_to_console("%s\n", output.c_str());
				error = "the stage of the 3D view could not be built ( codol_bsp, exit code " + std::to_string(code) + " )";
				return false;
			}
		}

		if (!load_renderer(name, error, to_utf8(map_file.stem())))
		{
			return false;
		}

		stage = true;
		stage_keys_ = keys;

		// look at the map, not at the stage's box
		edit::bounds box;

		if (document_bounds(box))
		{
			const double reach = std::max({ box.maxs.x - box.mins.x, box.maxs.y - box.mins.y, box.maxs.z - box.mins.z, 256.0 });
			cam.origin = { static_cast<float>((box.mins.x + box.maxs.x) * 0.5 - reach * 0.7), static_cast<float>((box.mins.y + box.maxs.y) * 0.5 - reach * 0.7),
				static_cast<float>(box.maxs.z + reach * 0.5) };
			cam.pitch = 35.0f;
			cam.yaw = 45.0f;
		}
		else
		{
			cam.origin = { -384.0f, -384.0f, 320.0f };
			cam.pitch = 30.0f;
			cam.yaw = 45.0f;
		}

		cam.roll = 0.0f;
		return true;
	}

	bool scene::document_bounds(edit::bounds& box) const
	{
		for (const object_id id : doc.entities())
		{
			if (const auto* e = doc.find_entity(id))
			{
				for (const object_id child : e->children)
				{
					if (const auto* geo = doc.geometry(child))
					{
						for (const auto& p : geo->vertices)
						{
							box.add(p);
						}
					}
				}
			}
		}

		return box.valid();
	}

	bool scene::document_spawn_camera(const int spawn)
	{
		std::vector<const edit::entity*> spawns;

		// the intermission first, then the spawn points
		for (const bool want_intermission : { true, false })
		{
			for (const object_id id : doc.entities())
			{
				const auto* e = doc.find_entity(id);
				const std::string_view cls = e ? e->classname() : std::string_view();
				const bool intermission = cls == "mp_global_intermission";

				if (e && (want_intermission ? intermission : (cls.rfind("mp_", 0) == 0 && cls.find("spawn") != std::string_view::npos)))
				{
					spawns.push_back(e);
				}
			}
		}

		if (spawn < 0 || static_cast<std::size_t>(spawn) >= spawns.size())
		{
			return false;
		}

		const auto origin = spawns[static_cast<std::size_t>(spawn)]->origin();
		const auto angles = spawns[static_cast<std::size_t>(spawn)]->angles();
		cam.origin = { static_cast<float>(origin.x), static_cast<float>(origin.y), static_cast<float>(origin.z + 60.0) };
		cam.pitch = static_cast<float>(angles.x);
		cam.yaw = static_cast<float>(angles.y);
		cam.roll = 0.0f;
		return true;
	}

	bool scene::stage_outdated() const
	{
		return open && stage && stage_keys_ != stage_light_keys(doc);
	}

	bool scene::restage(std::string& error)
	{
		if (!open || (!stage && renderer.loaded()))
		{
			error = "the map is shown in its zone of the game: the light there is the light of the compiled map";
			return false;
		}

		const auto keep = cam;
		const bool had = stage;

		// everything the renderer drew for the document is made again in the new stage
		if (renderer.loaded())
		{
			for (auto& [id, link] : links_)
			{
				drop_placed(link);
				link.failed_model.clear();
				link.pushed = false;
			}

			for (auto& [id, m] : meshes_)
			{
				destroy_mesh(m);
				m.revision = ~0ull;
			}
		}

		ref_owner_.clear();
		texture_sizes_.clear();
		model_boxes_.clear();
		no_model_box_.clear();
		caches_dirty_ = true;
		seen_change_ = ~0ull;

		if (!load_stage(error))
		{
			return false;
		}

		if (had)
		{
			cam = keep;
		}

		return true;
	}

	bool scene::load_renderer(const std::string& map, std::string& error, const std::string& project_map)
	{
		const auto install = components::codol_game::install_path();
		if (install.empty())
		{
			error = "Set the CoDOL install folder in File > Settings.";
			return false;
		}

		if (!game::glob::d3d9_device)
		{
			error = "No Direct3D device.";
			return false;
		}

		if (!renderer.initialised())
		{
			view::options options;
			options.game_root = std::filesystem::u8path(install);
			options.load_model_lods = true;
			options.keep_pick_geometry = true;
			options.keep_light_grid = true;

			// the zones of the game, and the stages of maps of your own ( load_stage )
			{
				std::error_code ec;
				std::filesystem::create_directories(stage_dir(), ec);
				options.zone_directories = { options.game_root / "client_shipRetail" / "chinese", options.game_root / "client_shipRetail" / "patch", stage_dir() };
			}

			if (!renderer.init(game::glob::d3d9_device, options, &error))
			{
				error = "Could not start the CoDOL renderer: " + error;
				return false;
			}
		}

		// the zone of the map's asset project is one of the zones the map is loaded with
		assets::map_opening(project_map.empty() ? map : project_map);
		stage = false;

		if (!renderer.load_map(map, &error))
		{
			error = "Could not load " + map + ": " + error;
			return false;
		}

		const auto stats = renderer.stats();
		game::printf_to_console("[CoDOL] %s: %d surfaces, %d static models, %d entity models, %d brush models ( %.1f s )\n",
			map.c_str(), stats.world_surface_count, stats.static_model_count, stats.entity_model_count, stats.brush_model_count, stats.load_ms / 1000.0);

		for (const auto& warning : renderer.warnings())
		{
			game::printf_to_console("  %s\n", warning.c_str());
		}

		spawn_index = 0;
		renderer.spawn_camera(0, cam);
		preview::renderer_changed();
		return true;
	}

	// everything that refers to the renderer's map or to objects of the document
	void scene::clear_view_state()
	{
		if (drag_active_)
		{
			cancel_drag();
		}

		if (renderer.loaded())
		{
			for (auto& [id, link] : links_)
			{
				drop_placed(link);
			}

			for (auto& [id, m] : meshes_)
			{
				destroy_mesh(m);
			}
		}

		links_.clear();
		meshes_.clear();
		ref_owner_.clear();
		import_revision_.clear();
		texture_sizes_.clear();
		model_boxes_.clear();
		no_model_box_.clear();
		statics.clear();
		selected.clear();
		connections.clear();
		statics_serial++;
		selected_serial++;
		inspecting = false;
		caches_dirty_ = true;
		open_serial++;
		last_selection_box = {};
		seen_change_ = seen_visibility_ = seen_selection_ = statics_change_ = ~0ull;
	}

	void scene::refresh_assets()
	{
		if (!renderer.loaded())
		{
			return;
		}

		for (auto& [id, link] : links_)
		{
			if (link.placed || !link.failed_model.empty())
			{
				drop_placed(link);
				link.failed_model.clear();
				link.pushed = false;
			}
		}

		for (auto& [id, m] : meshes_)
		{
			destroy_mesh(m);
			m.revision = ~0ull;
		}

		texture_sizes_.clear();
		model_boxes_.clear();
		no_model_box_.clear();
		caches_dirty_ = true;
		seen_change_ = ~0ull;
		preview::assets_changed();
	}

	void scene::close()
	{
		clear_view_state();
		doc.new_map();
		renderer.unload();
		preview::renderer_changed();
		map_name.clear();
		map_file.clear();
		open = false;
		stage = false;
		stage_keys_.clear();
	}

	bool scene::open_zone(const std::string& map, std::string& error)
	{
		const auto install = components::codol_game::install_path();
		if (install.empty())
		{
			error = "Set the CoDOL install folder in File > Settings.";
			return false;
		}

		const auto root = std::filesystem::u8path(install);
		std::filesystem::path fastfile;

		// the patch folder overrides the base zones, like in the game
		for (const auto& dir : { root / "client_shipRetail" / "patch", root / "client_shipRetail" / "chinese" })
		{
			std::error_code ec;
			if (const auto candidate = dir / (map + ".ff"); std::filesystem::is_regular_file(candidate, ec))
			{
				fastfile = candidate;
				break;
			}
		}

		if (fastfile.empty())
		{
			error = "There is no " + map + ".ff in the install.";
			return false;
		}

		const auto t0 = std::chrono::steady_clock::now();
		close();

		if (!load_renderer(map, error))
		{
			return false;
		}

		const auto t1 = std::chrono::steady_clock::now();

		if (!doc.import_fastfile(fastfile, root / "codoMP_client_shipRetail.exe", error, map))
		{
			error = "Could not import " + map + ": " + error;
			renderer.unload();
			return false;
		}

		map_name = map;
		open = true;
		place_brush_models(nullptr);
		build_links(doc, true);

		const auto t2 = std::chrono::steady_clock::now();
		load_seconds = std::chrono::duration<double>(t2 - t0).count();

		game::printf_to_console("[CoDOL] %s: document with %d entities, %d brushes, %d patches ( import %.1f s, total %.1f s )\n", map.c_str(),
			static_cast<int>(doc.entities().size()), static_cast<int>(doc.brush_count()), static_cast<int>(doc.patch_count()),
			std::chrono::duration<double>(t2 - t1).count(), load_seconds);

		for (const auto& warning : doc.load_warnings())
		{
			game::printf_to_console("  %s\n", warning.c_str());
		}

		return true;
	}

	bool scene::open_map_file(const std::filesystem::path& file, std::string& error)
	{
		const auto t0 = std::chrono::steady_clock::now();
		close();

		if (!doc.load_map_file(file, error))
		{
			error = "Could not read " + to_utf8(file) + ": " + error;
			return false;
		}

		map_file = file;
		open = true;

		// the baseline of the import, saved next to the map
		std::string base_text, orig_ents;
		const bool has_baseline = read_file(sidecar(file, ".codol_base.map"), base_text) && read_file(sidecar(file, ".codol_orig.ents"), orig_ents);

		if (has_baseline)
		{
			doc.set_import_baseline(base_text, std::move(orig_ents));
		}

		std::string zone;
		if (const auto* world = doc.find_entity(doc.worldspawn()))
		{
			zone = std::string(world->value(codol::mapents::KEY_MAP_NAME));
		}

		map_name = zone.empty() ? to_utf8(file.stem()) : zone;

		if (zone.empty())
		{
			// a map of your own: the 3D view shows it in the stage
			std::string stage_error;
			if (!load_stage(stage_error))
			{
				game::printf_to_console("^3[CoDOL] %s: %s. Use a 2D view.\n", to_utf8(file.filename()).c_str(), stage_error.c_str());
			}
		}
		else
		{
			std::string render_error;
			if (!load_renderer(zone, render_error))
			{
				game::printf_to_console("^1[CoDOL] %s\n", render_error.c_str());
			}
			else if (has_baseline)
			{
				edit::document baseline;
				std::string base_error;

				if (baseline.load_map_text(std::move(base_text), base_error))
				{
					place_brush_models(&baseline);
					build_links(baseline, false);
				}
				else
				{
					game::printf_to_console("^1[CoDOL] the import baseline next to the map cannot be read ( %s ): objects are matched by order.\n", base_error.c_str());
					build_links(doc, true);
				}
			}
			else
			{
				game::printf_to_console("[CoDOL] no import baseline next to the map ( .codol_base.map / .codol_orig.ents ): the export writes all entities as they are.\n");
				build_links(doc, true);
			}
		}

		load_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
		game::printf_to_console("[CoDOL] %s: %d entities, %d brushes, %d patches ( %.1f s )\n", to_utf8(file.filename()).c_str(),
			static_cast<int>(doc.entities().size()), static_cast<int>(doc.brush_count()), static_cast<int>(doc.patch_count()), load_seconds);

		for (const auto& warning : doc.load_warnings())
		{
			game::printf_to_console("  %s\n", warning.c_str());
		}

		return true;
	}

	std::filesystem::path scene::default_map_file() const
	{
		return std::filesystem::u8path(utils::fs::get_basepath()) / "map_source" / (map_name + ".map");
	}

	bool scene::save(std::string& error)
	{
		return save_as(map_file.empty() ? default_map_file() : map_file, error);
	}

	bool scene::save_as(const std::filesystem::path& file, std::string& error)
	{
		if (!open)
		{
			error = "No map is open.";
			return false;
		}

		std::error_code ec;
		std::filesystem::create_directories(file.parent_path(), ec);

		if (!doc.save_map_file(file, error))
		{
			error = "Could not write " + to_utf8(file) + ": " + error;
			return false;
		}

		// the baseline of the import goes next to the map: the entity override is built against it
		if (doc.has_import_baseline())
		{
			if (!write_file(sidecar(file, ".codol_base.map"), doc.import_baseline_map())
				|| !write_file(sidecar(file, ".codol_orig.ents"), doc.import_original_ents()))
			{
				error = "The map was saved, but its import baseline ( .codol_base.map / .codol_orig.ents ) could not be written.";
				return false;
			}
		}

		map_file = file;
		doc.mark_saved();
		return true;
	}

	bool scene::export_entities(std::filesystem::path& written, std::string& summary)
	{
		if (!open)
		{
			summary = "No map is open.";
			return false;
		}

		const std::string root = to_utf8(codol::packed_scripts::script_root());
		if (root.empty())
		{
			summary = "Set the CoDOL install folder first ( File > Settings ): the export goes to <game>\\main\\sh1x_rawfiles.";
			return false;
		}

		// the renderer side of the links tells which static models were touched
		sync();

		edit::override_export out;
		std::string error;

		if (!doc.export_entity_override(out, error))
		{
			summary = "Export failed: " + error;
			return false;
		}

		const std::string name = out.map_name.empty() ? map_name : out.map_name;
		const auto file = std::filesystem::u8path(root) / "maps" / "mp" / (name + ".d3dbsp.ents");

		std::error_code ec;
		std::filesystem::create_directories(file.parent_path(), ec);

		if (!write_file(file, out.text))
		{
			summary = "Could not write " + to_utf8(file);
			return false;
		}

		written = file;
		summary = "Exported " + std::to_string(out.exported) + " entities to " + to_utf8(file);

		for (const auto& warning : out.warnings)
		{
			game::printf_to_console("  export: %s\n", warning.c_str());
		}

		// static models are baked into the zone: the override cannot move or remove them
		int static_edits = 0;
		for (const auto& [id, link] : links_)
		{
			if (link.is_static && (link.stamp != stamp_ || link.moved || !link.shows_map_objects))
			{
				static_edits++;
			}
		}

		if (static_edits)
		{
			const std::string note = std::to_string(static_edits) + " static model(s) of the zone were moved, changed or deleted: they are baked into the zone and not part of the entity override.";
			game::printf_to_console("  export: %s\n", note.c_str());
			summary += "\n" + note;
		}

		return true;
	}

	std::string scene::title() const
	{
		if (!open)
		{
			return {};
		}

		std::string t = map_name;

		if (doc.modified())
		{
			t += " *";
		}

		t += map_file.empty() ? "  ( imported, not saved )" : "  ( " + to_utf8(map_file) + " )";
		return t;
	}

	// ---- document <-> renderer ----------------------------------------------------------------------------

	// The zone stores the brushes of a brush model relative to the origin of its entity ( and unrotated ). A map
	// editor wants them where they are in the world: with "origin" and "angles" applied, once, right after the
	// import. From then on brushes and keys move together. This is not an edit of the map: no undo step, not
	// "modified", and the keys are exactly as they were, so the export still sees an untouched entity.
	// `baseline`: for a map that was saved - only entities whose brushes are still as the zone has them are placed
	// ( a map saved by this editor has them in the world already ).
	void scene::place_brush_models(const edit::document* baseline)
	{
		std::unordered_set<std::string_view> base_bodies;

		if (baseline)
		{
			for (const object_id id : baseline->entities())
			{
				const auto* e = baseline->find_entity(id);

				if (e->value("model").rfind('*', 0) == 0)
				{
					for (const object_id child : e->children)
					{
						if (const auto* b = baseline->find_brush(child))
						{
							base_bodies.insert(b->source_body.view());
						}
					}
				}
			}
		}

		const std::vector<object_id> ids(doc.entities().begin(), doc.entities().end());
		int placed = 0;

		for (const object_id id : ids)
		{
			const auto* e = doc.find_entity(id);

			if (e->children.empty() || e->value("model").rfind('*', 0) != 0)
			{
				continue;
			}

			const edit::vec3 origin = e->origin(), angles = e->angles();
			if (origin == edit::vec3{} && angles == edit::vec3{})
			{
				continue;
			}

			if (baseline)
			{
				const auto* first = doc.find_brush(e->children.front());
				if (!first || first->source_body.empty() || !base_bodies.contains(first->source_body.view()))
				{
					continue; // already in the world
				}
			}

			// the keys as they are written, to put them back exactly
			const auto keep = [e](const char* key) { const auto* v = e->find(key); return v ? std::optional<std::string>(*v) : std::nullopt; };
			const auto old_origin = keep("origin"), old_angles = keep("angles"), old_angle = keep("angle");

			doc.select(id);
			doc.transform_selection(edit::transform::concat(edit::transform::rotation_angles({}, angles), edit::transform::translation(origin)), "Place");

			const auto restore = [&](const char* key, const std::optional<std::string>& value)
			{
				if (value)
				{
					doc.set_key(id, key, *value);
				}
				else if (doc.find_entity(id)->find(key))
				{
					doc.remove_key(id, key);
				}
			};

			restore("origin", old_origin);
			restore("angles", old_angles);
			restore("angle", old_angle);
			placed++;
		}

		if (placed)
		{
			doc.deselect_all();
			doc.clear_undo();
			doc.mark_saved();
			game::printf_to_console("[CoDOL] %d brush models: collision brushes placed at the origin of their entity\n", placed);
		}
	}

	// Which renderer objects show which entity. `baseline` is the document as it came from the zone ( the document
	// itself right after an import ): placements are compared with it, so that only what really moved gets a
	// transform override in the renderer.
	void scene::build_links(const edit::document& baseline, const bool fresh_import)
	{
		links_.clear();
		ref_owner_.clear();
		import_revision_.clear();

		if (!renderer.loaded())
		{
			return;
		}

		// ---- renderer objects by entity index / static model index
		std::unordered_map<int, std::vector<view::object_ref>> by_entity;
		std::vector<std::string> static_names;

		for (const auto& object : renderer.objects())
		{
			if ((object.kind == view::object_kind::entity_model || object.kind == view::object_kind::brush_model) && object.entity >= 0)
			{
				by_entity[object.entity].push_back(object.ref());
			}
			else if (object.kind == view::object_kind::static_model && object.id >= 0)
			{
				if (static_names.size() <= static_cast<std::size_t>(object.id))
				{
					static_names.resize(static_cast<std::size_t>(object.id) + 1);
				}

				static_names[static_cast<std::size_t>(object.id)] = object.name;
			}
		}

		// ---- the baseline: entities by codol_id, static models in order, brush texts
		std::unordered_map<int, object_id> base_by_id;
		std::vector<object_id> base_statics;
		std::unordered_set<std::string_view> base_bodies;

		for (const object_id id : baseline.entities())
		{
			const auto* e = baseline.find_entity(id);

			if (e->find(codol::mapents::KEY_STATIC))
			{
				base_statics.push_back(id);
			}
			else if (const auto* text = e->find(codol::mapents::KEY_ID))
			{
				base_by_id.emplace(int_value(*text, -1), id);
			}

			if (!fresh_import)
			{
				for (const object_id child : e->children)
				{
					if (const auto* b = baseline.find_brush(child))
					{
						base_bodies.insert(b->source_body.view());
					}
					else if (const auto* p = baseline.find_patch(child))
					{
						base_bodies.insert(p->source_body.view());
					}
				}
			}
		}

		// ---- static models of the document -> index in the zone
		std::vector<object_id> doc_statics;
		for (const object_id id : doc.entities())
		{
			if (doc.find_entity(id)->find(codol::mapents::KEY_STATIC))
			{
				doc_statics.push_back(id);
			}
		}

		std::vector<int> static_index(doc_statics.size(), -1);

		if (fresh_import)
		{
			for (std::size_t i = 0; i < doc_statics.size(); i++)
			{
				static_index[i] = static_cast<int>(i);
			}
		}
		else
		{
			// The saved map keeps the order of the zone; some models are gone, some were edited. Unedited ones are
			// found exactly and anchor the rest, which is matched by model name between its neighbours.
			const auto same = [&](const edit::entity& a, const edit::entity& b)
			{
				return a.value("model") == b.value("model") && a.value("origin") == b.value("origin")
					&& a.value("angles") == b.value("angles") && a.value("modelscale") == b.value("modelscale");
			};

			std::vector<char> taken(base_statics.size(), 0);
			std::size_t next = 0;

			for (std::size_t i = 0; i < doc_statics.size(); i++)
			{
				const auto* e = doc.find_entity(doc_statics[i]);

				for (std::size_t k = next; k < base_statics.size(); k++)
				{
					if (same(*e, *baseline.find_entity(base_statics[k])))
					{
						static_index[i] = static_cast<int>(k);
						taken[k] = 1;
						next = k + 1;
						break;
					}
				}
			}

			int unmatched = 0;

			for (std::size_t i = 0; i < doc_statics.size(); i++)
			{
				if (static_index[i] >= 0)
				{
					continue;
				}

				std::size_t lo = 0, hi = base_statics.size();

				for (std::size_t j = i; j-- > 0;)
				{
					if (static_index[j] >= 0) { lo = static_cast<std::size_t>(static_index[j]) + 1; break; }
				}

				for (std::size_t j = i + 1; j < doc_statics.size(); j++)
				{
					if (static_index[j] >= 0) { hi = static_cast<std::size_t>(static_index[j]); break; }
				}

				const auto model = doc.find_entity(doc_statics[i])->value("model");

				for (std::size_t k = lo; k < hi; k++)
				{
					if (!taken[k] && baseline.find_entity(base_statics[k])->value("model") == model)
					{
						static_index[i] = static_cast<int>(k);
						taken[k] = 1;
						break;
					}
				}

				unmatched += static_index[i] < 0;
			}

			if (unmatched)
			{
				game::printf_to_console("[CoDOL] %d static model(s) of the map are not in the zone: new ones ( the map compile adds them ), or edited beyond recognition.\n", unmatched);
			}
		}

		// ---- links
		int mismatched = 0;
		std::size_t static_cursor = 0;
		std::unordered_set<int> used_statics, used_entities;

		for (const object_id id : doc.entities())
		{
			const auto* e = doc.find_entity(id);
			const edit::entity* base = nullptr;
			entity_link link;

			if (e->find(codol::mapents::KEY_STATIC))
			{
				const int k = static_index[static_cursor++];

				if (k >= 0 && static_cast<std::size_t>(k) < base_statics.size())
				{
					base = baseline.find_entity(base_statics[static_cast<std::size_t>(k)]);

					// the k-th misc_model of the import is the k-th static model of the zone: make sure
					if (static_cast<std::size_t>(k) < static_names.size() && _stricmp(static_names[static_cast<std::size_t>(k)].c_str(), std::string(base->value("model")).c_str()) == 0)
					{
						link.refs.push_back({ view::object_kind::static_model, k });
						link.is_static = true;
						used_statics.insert(k);
					}
					else
					{
						mismatched++;
					}
				}
			}
			else if (const auto* text = e->find(codol::mapents::KEY_ID))
			{
				const int index = int_value(*text, -1);

				if (const auto it = base_by_id.find(index); it != base_by_id.end())
				{
					base = baseline.find_entity(it->second);
				}

				if (const auto it = by_entity.find(index); it != by_entity.end() && base)
				{
					link.refs = it->second;
					used_entities.insert(index);
				}
			}

			if (base)
			{
				link.base_model = std::string(base->value("model"));
				link.base_origin = base->origin();
				link.base_angles = base->angles();
				link.base_scale = model_scale(*base);
			}

			for (const auto& ref : link.refs)
			{
				view::transform t;
				renderer.get_transform(ref, t);
				link.base.push_back(t);
				link.brush_model |= ref.kind == view::object_kind::brush_model;
				ref_owner_[ref_key(ref)] = id;

				view::scene_object object;
				if (renderer.object(ref, object))
				{
					const edit::vec3 c{ object.center.x, object.center.y, object.center.z }, h{ object.half_size.x, object.half_size.y, object.half_size.z };
					link.local_box.add(c - h - link.base_origin);
					link.local_box.add(c + h - link.base_origin);
					link.has_box = true;
				}
			}

			if (!link.refs.empty())
			{
				links_.emplace(id, std::move(link));
			}

			// brushes and patches exactly as the zone has them are "imported": shown as wires on request only
			for (const object_id child : e->children)
			{
				if (fresh_import)
				{
					import_revision_.emplace(child, doc.revision(child));
				}
				else
				{
					const auto* b = doc.find_brush(child);
					const auto* p = b ? nullptr : doc.find_patch(child);
					const auto body = b ? b->source_body.view() : (p ? p->source_body.view() : std::string_view());

					if (!body.empty() && base_bodies.contains(body))
					{
						import_revision_.emplace(child, doc.revision(child));
					}
				}
			}
		}

		// What the zone has and the document does not: deleted before the map was saved. No entity will ever show
		// these objects again, so they are hidden here.
		if (!fresh_import)
		{
			int gone = 0;

			for (std::size_t k = 0; k < static_names.size(); k++)
			{
				if (!used_statics.contains(static_cast<int>(k)) && k < base_statics.size())
				{
					gone += renderer.set_hidden({ view::object_kind::static_model, static_cast<int>(k) }, true);
				}
			}

			for (const auto& [index, refs] : by_entity)
			{
				if (!used_entities.contains(index))
				{
					for (const auto& ref : refs)
					{
						gone += renderer.set_hidden(ref, true);
					}
				}
			}

			if (gone)
			{
				game::printf_to_console("[CoDOL] %d object(s) of the zone have no entity in the map any more ( deleted ): hidden\n", gone);
			}
		}

		if (mismatched)
		{
			game::printf_to_console("^1[CoDOL] %d static model(s) of the document do not match the zone ( different model at the same index ): they are not linked.\n", mismatched);
		}

		game::printf_to_console("[CoDOL] linked %d entities to objects of the zone, %d imported brushes / patches\n",
			static_cast<int>(links_.size()), static_cast<int>(import_revision_.size()));

		// The renderer draws what the game draws at the start of a match. Entities with a model it leaves out
		// ( objects of other game types, turrets .. ) are shown by the editor: say which.
		std::map<std::string, int> extra;
		for (const object_id id : doc.entities())
		{
			const auto* e = doc.find_entity(id);
			if (e->children.empty() && is_xmodel_name(e->value("model")) && !links_.contains(id))
			{
				extra[std::string(e->classname())]++;
			}
		}

		if (!extra.empty())
		{
			std::string list;
			for (const auto& [name, count] : extra)
			{
				list += (list.empty() ? "" : ", ") + std::to_string(count) + " " + name;
			}

			game::printf_to_console("[CoDOL] entities with a model that the game does not draw at the start of a match are shown as well: %s\n", list.c_str());
		}
	}

	void scene::drop_placed(entity_link& link)
	{
		if (link.placed)
		{
			ref_owner_.erase(ref_key({ view::object_kind::placed_model, link.placed }));
			renderer.remove_model(link.placed);
			link.placed = 0;
			link.placed_model.clear();
		}
	}

	void scene::destroy_mesh(brush_mesh& m)
	{
		for (const int handle : m.handles)
		{
			ref_owner_.erase(ref_key({ view::object_kind::dynamic_mesh, handle }));
			renderer.destroy_mesh(handle);
		}

		m.handles.clear();
	}

	// a new or edited brush / patch as triangles with its game materials
	void scene::rebuild_mesh(const object_id id, brush_mesh& m)
	{
		destroy_mesh(m);
		m.revision = doc.revision(id);

		// a curve at its own subdivision ( Patch > Inc / Dec Subdivision ); nothing solid while patches are shown as wires
		const edit::patch* as_patch = doc.find_patch(id);
		if (as_patch && ui::patches_as_wire())
		{
			return;
		}

		edit::mesh mesh;
		if (!doc.build_mesh(id, mesh, as_patch ? edit::patch_subdivisions(*as_patch) : 4) || mesh.indices.empty())
		{
			return;
		}

		const bool is_patch = doc.kind(id) == edit::object_kind::patch;

		const auto& names = renderer.material_names();
		std::vector<char> done(mesh.parts.size(), 0);
		std::vector<view::mesh_vertex> vertices;
		std::vector<std::uint32_t> indices;
		std::vector<std::uint32_t> remap;

		for (std::size_t first = 0; first < mesh.parts.size(); first++)
		{
			if (done[first])
			{
				continue;
			}

			// all faces with the same material are one mesh
			const std::string& material = mesh.parts[first].material;
			vertices.clear();
			indices.clear();
			remap.assign(mesh.vertices.size(), 0xFFFFFFFFu);

			for (std::size_t p = first; p < mesh.parts.size(); p++)
			{
				if (done[p] || mesh.parts[p].material != material)
				{
					continue;
				}

				done[p] = 1;

				for (std::uint32_t i = 0; i < mesh.parts[p].index_count; i++)
				{
					const std::uint32_t src = mesh.indices[mesh.parts[p].first_index + i];

					if (remap[src] == 0xFFFFFFFFu)
					{
						const auto& v = mesh.vertices[src];
						view::mesh_vertex out;
						out.position = { v.position[0], v.position[1], v.position[2] };
						out.normal = { v.normal[0], v.normal[1], v.normal[2] };
						out.color = (static_cast<std::uint32_t>(v.color[3]) << 24) | (static_cast<std::uint32_t>(v.color[0]) << 16) | (static_cast<std::uint32_t>(v.color[1]) << 8) | v.color[2];
						out.uv[0] = v.uv[0];
						out.uv[1] = v.uv[1];
						remap[src] = static_cast<std::uint32_t>(vertices.size());
						vertices.push_back(out);
					}

					indices.push_back(remap[src]);
				}
			}

			view::mesh_data data;
			data.vertices = vertices.data();
			data.vertex_count = vertices.size();
			data.indices = indices.data();
			data.index_count = indices.size();
			data.fallback_color = fallback_color(material);
			data.two_sided = is_patch;

			// a .map names world materials without the "wc/" the zone gives them
			data.material = material;
			if (!std::binary_search(names.begin(), names.end(), material) && std::binary_search(names.begin(), names.end(), "wc/" + material))
			{
				data.material = "wc/" + material;
			}

			std::string error;
			if (const int handle = renderer.create_mesh(data, &error))
			{
				m.handles.push_back(handle);
				ref_owner_[ref_key({ view::object_kind::dynamic_mesh, handle })] = id;
			}
			else
			{
				game::printf_to_console("^1[CoDOL] brush %u: %s\n", id, error.c_str());
			}
		}
	}

	void scene::push_entity(const edit::entity& e, entity_link& link, const bool visible)
	{
		link.revision = e.revision;
		link.visible = visible;
		link.pushed = true;

		const auto model = e.value("model");
		const edit::vec3 origin = e.origin(), angles = e.angles();
		const double scale = model_scale(e);

		// the objects of the zone stand for the entity as long as it shows the model it had there
		const bool use_map = !link.refs.empty() && (link.brush_model || model == link.base_model);
		link.shows_map_objects = use_map;

		const bool at_base = same_vec(origin, link.base_origin) && same_vec(angles, link.base_angles) && std::fabs(scale - link.base_scale) < 1e-6;

		for (std::size_t i = 0; i < link.refs.size(); i++)
		{
			const auto& ref = link.refs[i];
			renderer.set_hidden(ref, !(visible && use_map));

			if (!use_map)
			{
				continue;
			}

			if (at_base)
			{
				// back where the zone has it: exactly the shipped placement and lighting
				if (link.moved)
				{
					renderer.reset_transform(ref);
				}
			}
			else
			{
				view::transform t = link.base[i];

				if (ref.kind == view::object_kind::brush_model)
				{
					// the brushes of the zone are in world space: the model follows the change of the keys
					const edit::vec3 d = origin - link.base_origin, a = angles - link.base_angles;
					t.origin = { t.origin.x + static_cast<float>(d.x), t.origin.y + static_cast<float>(d.y), t.origin.z + static_cast<float>(d.z) };
					t.angles = { t.angles.x + static_cast<float>(a.x), t.angles.y + static_cast<float>(a.y), t.angles.z + static_cast<float>(a.z) };
				}
				else
				{
					t.origin = to_float3(origin);
					t.angles = to_float3(angles);
					t.scale = static_cast<float>(scale);
				}

				renderer.set_transform(ref, t);
			}
		}

		if (use_map)
		{
			link.moved = !at_base;
		}

		// a model the zone has no object for
		if (is_xmodel_name(model) && !use_map)
		{
			if (link.placed && link.placed_model != model)
			{
				drop_placed(link);
			}

			const view::transform t{ to_float3(origin), to_float3(angles), static_cast<float>(scale) };

			if (!link.placed && link.failed_model != model)
			{
				std::string error;
				link.placed = renderer.add_model(std::string(model), t, &error);

				if (link.placed)
				{
					link.placed_model = std::string(model);
					link.failed_model.clear();
					ref_owner_[ref_key({ view::object_kind::placed_model, link.placed })] = e.id;
				}
				else
				{
					// not again on every change of the entity
					link.failed_model = std::string(model);
					missing_models_[link.failed_model]++;
				}
			}

			if (link.placed)
			{
				const view::object_ref ref{ view::object_kind::placed_model, link.placed };
				renderer.set_transform(ref, t);
				renderer.set_hidden(ref, !visible);
			}
		}
		else
		{
			drop_placed(link);
		}
	}

	// the renderer objects of one entity; returns whether the entity is a compiled brush model
	bool scene::sync_entity(const object_id id, const edit::entity& e)
	{
		const bool has_children = !e.children.empty();
		auto it = links_.find(id);

		// entities without a link only get one when they have a model to show
		if (it == links_.end() && !has_children && is_xmodel_name(e.value("model")))
		{
			it = links_.emplace(id, entity_link{}).first;
		}

		// A brush model ( "model" "*N" ) is compiled into the zone: the renderer draws it, or it is invisible
		// ( clip, triggers ). Its brushes are never drawn solid.
		bool compiled = has_children && e.value("model").rfind('*', 0) == 0;

		if (it == links_.end())
		{
			return compiled;
		}

		auto& link = it->second;
		link.stamp = stamp_;
		compiled |= link.brush_model;

		bool visible = doc.is_visible(id);

		// a brush model is hidden with its brushes
		if (visible && has_children && link.brush_model)
		{
			visible = false;
			for (const object_id child : e.children)
			{
				if (doc.is_visible(child))
				{
					visible = true;
					break;
				}
			}
		}

		if (!link.pushed || link.revision != e.revision || link.visible != visible)
		{
			push_entity(e, link, visible);
		}

		return compiled;
	}

	// new and edited geometry is drawn solid ( not for a compiled brush model: the caller does not ask )
	void scene::sync_child(const object_id child)
	{
		const std::uint64_t revision = doc.revision(child);

		if (const auto imported = import_revision_.find(child); imported != import_revision_.end() && imported->second == revision)
		{
			return;
		}

		if (!doc.is_visible(child))
		{
			return;
		}

		auto& m = meshes_[child];
		m.stamp = stamp_;

		if (m.revision != revision)
		{
			rebuild_mesh(child, m);
		}
	}

	// `selection_only`: while something is dragged only the selection changes - with tens of thousands of objects
	// a pass over all of them on every mouse move would be felt. The full pass follows when the drag is over.
	void scene::sync_renderer(const bool selection_only)
	{
		if (!renderer.loaded())
		{
			return;
		}

		if (selection_only)
		{
			partial_sync_ = true;

			// the stamp stays: nothing is looked at for "gone" in this pass
			std::unordered_map<object_id, bool> compiled_owner;

			for (const object_id id : doc.selection())
			{
				if (const auto* e = doc.find_entity(id))
				{
					sync_entity(id, *e);
					continue;
				}

				const object_id owner = doc.owner_of(id);
				auto known = compiled_owner.find(owner);

				if (known == compiled_owner.end())
				{
					const auto* e = doc.find_entity(owner);
					known = compiled_owner.emplace(owner, e ? sync_entity(owner, *e) : true).first;
				}

				if (!known->second)
				{
					sync_child(id);
				}
			}

			return;
		}

		partial_sync_ = false;
		stamp_++;

		for (const object_id id : doc.entities())
		{
			const auto* e = doc.find_entity(id);

			if (!sync_entity(id, *e))
			{
				for (const object_id child : e->children)
				{
					sync_child(child);
				}
			}
		}

		// entities that are gone ( deleted; an undo brings them back with the same id )
		for (auto it = links_.begin(); it != links_.end();)
		{
			auto& link = it->second;

			if (link.stamp == stamp_)
			{
				++it;
				continue;
			}

			if (link.pushed)
			{
				for (const auto& ref : link.refs)
				{
					renderer.set_hidden(ref, true);
				}

				drop_placed(link);
				link.pushed = false;
				link.visible = false;
			}

			it = link.refs.empty() ? links_.erase(it) : std::next(it);
		}

		// models no zone of the map has ( weapons, characters of other game modes ): the entity keeps its box
		if (!missing_models_.empty())
		{
			std::string list;
			for (const auto& [name, count] : missing_models_)
			{
				list += (list.empty() ? "" : ", ") + name + (count > 1 ? " x" + std::to_string(count) : "");
			}

			game::printf_to_console("[CoDOL] models that are not in the zones of this map, shown as boxes: %s\n", list.c_str());
			missing_models_.clear();
		}

		for (auto it = meshes_.begin(); it != meshes_.end();)
		{
			if (it->second.stamp != stamp_)
			{
				destroy_mesh(it->second);
				it = meshes_.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	object_id scene::object_of(const view::object_ref& ref) const
	{
		const auto it = ref_owner_.find(ref_key(ref));
		return it != ref_owner_.end() ? it->second : edit::no_object;
	}

	void scene::refs_of(const object_id entity, std::vector<view::object_ref>& out) const
	{
		const auto it = links_.find(entity);
		if (it == links_.end() || !it->second.visible)
		{
			return;
		}

		if (it->second.shows_map_objects)
		{
			out.insert(out.end(), it->second.refs.begin(), it->second.refs.end());
		}

		if (it->second.placed)
		{
			out.push_back({ view::object_kind::placed_model, it->second.placed });
		}
	}

	bool scene::has_model(const object_id entity) const
	{
		const auto it = links_.find(entity);
		return it != links_.end() && ((it->second.shows_map_objects && !it->second.refs.empty()) || it->second.placed);
	}

	bool scene::imported(const object_id id) const
	{
		const auto it = import_revision_.find(id);
		return it != import_revision_.end() && it->second == doc.revision(id);
	}

	// ---- caches for the views ---------------------------------------------------------------------------------

	void scene::cache_entity(object_cache& cache, const object_id id, const edit::entity& e)
	{
		cached_entity item;
		item.id = id;
		item.revision = e.revision;

		const auto box = doc.object_bounds(id);
		const auto origin = e.origin();

		for (int i = 0; i < 3; i++)
		{
			item.mins[i] = static_cast<float>(box.mins[i]);
			item.maxs[i] = static_cast<float>(box.maxs[i]);
			item.origin[i] = static_cast<float>(origin[i]);
		}

		item.yaw = static_cast<float>(e.angles().y);
		item.classname = std::string(e.classname());
		item.color = class_color(item.classname);
		item.has_model = has_model(id);
		cache.entities.push_back(std::move(item));
	}

	void scene::cache_child(object_cache& cache, const object_id child, const edit::entity& owner, const bool compiled, const ImU32 entity_color,
		std::vector<edit::vec3>& scratch)
	{
		cached_object item;
		item.id = child;
		item.owner = owner.id;
		item.revision = doc.revision(child);
		item.owner_revision = owner.revision;
		item.compiled = compiled;
		item.first = static_cast<std::uint32_t>(cache.points.size());

		const auto imported = import_revision_.find(child);
		item.imported = imported != import_revision_.end() && imported->second == item.revision;

		edit::bounds box;
		bool detail = false;

		if (const auto* geo = doc.geometry(child))
		{
			if (geo->vertices.empty())
			{
				return; // a block the editor does not understand
			}

			for (const auto& loop : geo->face_vertices)
			{
				for (std::size_t i = 0; i < loop.size(); i++)
				{
					const int a = loop[i], b = loop[(i + 1) % loop.size()];

					// every edge is shared by two faces: once is enough
					if (a < b || !geo->closed)
					{
						cache.points.push_back(to_float3(geo->vertices[static_cast<std::size_t>(a)]));
						cache.points.push_back(to_float3(geo->vertices[static_cast<std::size_t>(b)]));
					}
				}
			}

			box = geo->box;
			detail = doc.find_brush(child)->is_detail();
		}
		else if (const auto* p = doc.find_patch(child))
		{
			int columns = 0, rows = 0;
			edit::tessellate_patch(*p, p->type == edit::patch_type::curve ? 3 : 1, scratch, columns, rows);

			for (int c = 0; c < columns; c++)
			{
				for (int r = 0; r < rows; r++)
				{
					const auto& at = scratch[static_cast<std::size_t>(c) * rows + r];
					box.add(at);

					if (r + 1 < rows)
					{
						cache.points.push_back(to_float3(at));
						cache.points.push_back(to_float3(scratch[static_cast<std::size_t>(c) * rows + r + 1]));
					}

					if (c + 1 < columns)
					{
						cache.points.push_back(to_float3(at));
						cache.points.push_back(to_float3(scratch[static_cast<std::size_t>(c + 1) * rows + r]));
					}
				}
			}

			item.patch = true;
			detail = p->is_detail();
		}
		else
		{
			return;
		}

		item.count = static_cast<std::uint32_t>(cache.points.size()) - item.first;

		for (int i = 0; i < 3; i++)
		{
			item.mins[i] = static_cast<float>(box.mins[i]);
			item.maxs[i] = static_cast<float>(box.maxs[i]);
		}

		// colour: brush entities by class, tool brushes of the collision map, patches, detail, new, structural
		if (entity_color)
		{
			item.color = entity_color;
		}
		else if (doc.layer_of(child) == codol::mapgeo::TOOL_LAYER)
		{
			item.color = colors::get(colors::brush_entity);
		}
		else if (item.patch)
		{
			item.color = colors::get(colors::patch);
		}
		else if (detail)
		{
			item.color = colors::get(colors::brush_detail);
		}
		else if (!item.imported)
		{
			item.color = colors::get(colors::brush);
		}
		else
		{
			item.color = colors::get(colors::brush_zone);
		}

		cache.objects.push_back(item);
	}

	// Everything visible. Most of the time one object changed and thousands did not: the document is walked in
	// its order next to the cache of the last time, and an item whose object ( and entity ) has the same revision
	// is taken over as it is.
	void scene::rebuild_statics()
	{
		const object_cache old = std::move(statics);
		statics.clear();
		statics.objects.reserve(old.objects.size() + 16);
		statics.points.reserve(old.points.size() + 256);
		statics.entities.reserve(old.entities.size() + 16);

		const object_id world = doc.worldspawn();
		std::vector<edit::vec3> scratch;
		std::size_t object_cursor = 0, entity_cursor = 0;

		for (const object_id id : doc.entities())
		{
			const auto* e = doc.find_entity(id);

			if (e->children.empty())
			{
				if (id == world || !doc.is_visible(id))
				{
					continue;
				}

				const auto* before = find_cached(old.entities, entity_cursor, id);

				if (before && before->revision == e->revision && before->has_model == has_model(id))
				{
					statics.entities.push_back(*before);
				}
				else
				{
					cache_entity(statics, id, *e);
				}

				continue;
			}

			const auto link = links_.find(id);
			const bool compiled = (link != links_.end() && link->second.brush_model) || e->value("model").rfind('*', 0) == 0;
			const ImU32 entity_color = id == world ? 0 : class_color(e->classname());

			for (const object_id child : e->children)
			{
				if (!doc.is_visible(child))
				{
					continue;
				}

				const auto* before = find_cached(old.objects, object_cursor, child);

				if (before && before->revision == doc.revision(child) && before->owner_revision == e->revision && before->compiled == compiled
					&& before->imported == imported(child))
				{
					cached_object item = *before;
					item.first = static_cast<std::uint32_t>(statics.points.size());
					statics.points.insert(statics.points.end(), old.points.begin() + before->first, old.points.begin() + before->first + before->count);
					statics.objects.push_back(item);
				}
				else
				{
					cache_child(statics, child, *e, compiled, entity_color, scratch);
				}
			}
		}
	}

	// the selection: rebuilt on every step of a drag, so it only looks at what is selected
	void scene::rebuild_selected()
	{
		selected.clear();

		const object_id world = doc.worldspawn();
		std::vector<edit::vec3> scratch;

		for (const object_id id : doc.selection())
		{
			if (!doc.is_visible(id))
			{
				continue;
			}

			if (const auto* e = doc.find_entity(id))
			{
				if (e->children.empty() && id != world)
				{
					cache_entity(selected, id, *e);
				}

				continue;
			}

			const object_id owner = doc.owner_of(id);

			if (const auto* e = doc.find_entity(owner))
			{
				const auto link = links_.find(owner);
				const bool compiled = (link != links_.end() && link->second.brush_model) || e->value("model").rfind('*', 0) == 0;
				cache_child(selected, id, *e, compiled, owner == world ? 0 : class_color(e->classname()), scratch);
			}
		}
	}

	// which entity targets which ( "target" = "targetname" )
	void scene::find_connections()
	{
		connected_.clear();

		std::unordered_map<std::string_view, std::vector<object_id>> by_name;
		std::vector<object_id> sources;

		for (const object_id id : doc.entities())
		{
			const auto* e = doc.find_entity(id);

			if (const auto* name = e->find("targetname"); name && !name->empty())
			{
				by_name[*name].push_back(id);
			}

			if (const auto* target = e->find("target"); target && !target->empty())
			{
				sources.push_back(id);
			}
		}

		for (const object_id id : sources)
		{
			if (const auto it = by_name.find(*doc.find_entity(id)->find("target")); it != by_name.end())
			{
				for (const object_id other : it->second)
				{
					if (other != id)
					{
						connected_.emplace_back(id, other);
					}
				}
			}
		}
	}

	// the lines between them, where they are now
	void scene::place_connections()
	{
		connections.clear();

		for (const auto& [from_id, to_id] : connected_)
		{
			if (!doc.is_visible(from_id) || !doc.is_visible(to_id))
			{
				continue;
			}

			const auto from = doc.object_bounds(from_id), to = doc.object_bounds(to_id);
			if (from.valid() && to.valid())
			{
				connections.push_back({ to_float3(from.center()), to_float3(to.center()) });
			}
		}
	}

	void scene::sync()
	{
		// Start of a frame: the last one is drawn, nothing refers to the textures the renderer dropped since ( the
		// picture of the 3D view and the thumbnails of a map that was closed in the middle of that frame - by the
		// "unsaved changes" question, a command typed into the console, the Assets window .. ).
		renderer.release_retired();

		if (!open)
		{
			return;
		}

		const auto change = doc.change_counter(), visibility = doc.visibility_counter(), selection = doc.selection_counter();
		const bool changed = change != seen_change_ || visibility != seen_visibility_;
		const bool in_drag = doc.in_transaction();

		auto clock = std::chrono::steady_clock::now();
		const auto lap = [&clock](double& slowest_ms)
		{
			const auto now = std::chrono::steady_clock::now();
			slowest_ms = std::max(slowest_ms, std::chrono::duration<double, std::milli>(now - clock).count());
			clock = now;
		};

		if (changed || (partial_sync_ && !in_drag))
		{
			sync_renderer(in_drag && visibility == seen_visibility_ && !caches_dirty_);
		}

		lap(slowest.renderer);

		// the counter moves on every select call ( and on every step of a drag ): only a different set matters
		bool selection_changed = false;

		if (selection != seen_selection_)
		{
			seen_selection_ = selection;

			std::uint64_t h = 1469598103934665603ull;
			for (const object_id id : doc.selection())
			{
				h = (h ^ id) * 1099511628211ull;
			}

			h = (h ^ 0xFFFFFFFFull) * 1099511628211ull;

			for (const auto& f : doc.selected_faces())
			{
				h = (h ^ f.brush) * 1099511628211ull;
				h = (h ^ static_cast<std::uint32_t>(f.face)) * 1099511628211ull;
			}

			if (h != selection_hash_)
			{
				selection_hash_ = h;
				selection_changed = true;
			}
		}

		// only the selection changes while something is dragged
		static std::uint64_t seen_colors = colors::serial();
		if (seen_colors != colors::serial())
		{
			seen_colors = colors::serial();
			statics.clear();
			caches_dirty_ = true;
		}

		const bool statics_stale = caches_dirty_ || visibility != seen_visibility_ || (change != statics_change_ && !in_drag);

		if (statics_stale)
		{
			rebuild_statics();
			find_connections();
			statics_change_ = change;
			statics_serial++;
		}

		lap(slowest.statics);

		if (statics_stale || selection_changed || changed)
		{
			rebuild_selected();
			place_connections();
			selected_serial++;
		}

		lap(slowest.selected);

		caches_dirty_ = false;
		seen_change_ = change;
		seen_visibility_ = visibility;

		if (!doc.selection().empty())
		{
			if (const auto box = doc.selection_bounds(); box.valid())
			{
				last_selection_box = box;
			}

			inspecting = false;
		}
	}

	// ---- editing --------------------------------------------------------------------------------------------

	void scene::begin_drag(const std::string_view name)
	{
		if (drag_active_)
		{
			end_drag();
		}

		drag_name_ = std::string(name);
		drag_active_ = true;
		doc.begin_transaction(drag_name_);
	}

	void scene::restart_drag()
	{
		if (drag_active_)
		{
			doc.cancel_transaction();
			doc.begin_transaction(drag_name_);
		}
	}

	bool scene::end_drag()
	{
		if (!drag_active_)
		{
			return false;
		}

		drag_active_ = false;
		return doc.end_transaction();
	}

	void scene::cancel_drag()
	{
		if (drag_active_)
		{
			drag_active_ = false;
			doc.cancel_transaction();
		}
	}

	// A brush model of the zone that moves needs an "origin" for the game ( and for the renderer ) to move it:
	// the document only carries the key along when there is one.
	void scene::ensure_origin_keys()
	{
		for (const object_id id : doc.selected_entities())
		{
			const auto* e = doc.find_entity(id);

			if (!e || e->children.empty() || e->find("origin") || e->value("model").rfind('*', 0) != 0)
			{
				continue;
			}

			bool whole = true;
			for (const object_id child : e->children)
			{
				whole &= doc.is_selected(child);
			}

			if (whole)
			{
				doc.set_key(id, "origin", "0 0 0");
			}
		}
	}

	bool scene::translate(const edit::vec3& delta)
	{
		if (delta == edit::vec3{})
		{
			return false;
		}

		doc.begin_transaction("Move");
		ensure_origin_keys();
		const bool ok = doc.translate(delta);
		doc.end_transaction();
		return ok;
	}

	bool scene::rotate(const edit::vec3& origin, const edit::vec3& axis, const double degrees)
	{
		if (degrees == 0.0)
		{
			return false;
		}

		doc.begin_transaction("Rotate");
		ensure_origin_keys();
		const bool ok = doc.rotate(origin, axis, degrees);
		doc.end_transaction();
		return ok;
	}

	bool scene::scale(const edit::vec3& origin, const edit::vec3& factors)
	{
		if (factors == edit::vec3{ 1.0, 1.0, 1.0 })
		{
			return false;
		}

		doc.begin_transaction("Scale");
		ensure_origin_keys();
		const bool ok = doc.scale(origin, factors);

		// the document scales positions only; a model has one uniform "modelscale": the factor that differs most from 1
		double uniform = 1.0;
		for (int i = 0; i < 3; i++)
		{
			if (std::fabs(std::fabs(factors[i]) - 1.0) > std::fabs(uniform - 1.0))
			{
				uniform = std::fabs(factors[i]);
			}
		}

		if (ok && uniform != 1.0)
		{
			for (const object_id id : std::vector<object_id>(doc.selection().begin(), doc.selection().end()))
			{
				if (const auto* e = doc.find_entity(id); e && is_xmodel_name(e->value("model")))
				{
					doc.set_key(id, "modelscale", edit::format_key_number(std::clamp(model_scale(*e) * uniform, 0.01, 100.0)));
				}
			}
		}

		doc.end_transaction();
		return ok;
	}

	bool scene::clone_selection()
	{
		if (doc.selection().empty())
		{
			return false;
		}

		const std::unordered_set<object_id> before(doc.entities().begin(), doc.entities().end());

		doc.begin_transaction("Clone");
		const bool ok = doc.clone_selection({ grid, grid, 0.0 });

		if (ok)
		{
			// A copy is a new entity: it must not claim the place of the original in the entity string of the zone.
			// The copy of a static model cannot be static ( those are baked into the zone ): it becomes a script_model.
			for (const object_id id : doc.selected_entities())
			{
				if (before.contains(id))
				{
					continue;
				}

				const auto* e = doc.find_entity(id);

				if (e->find(codol::mapents::KEY_STATIC))
				{
					doc.remove_key(id, codol::mapents::KEY_STATIC);
					doc.set_key(id, "classname", "script_model");
				}

				if (doc.find_entity(id)->find(codol::mapents::KEY_ID))
				{
					doc.remove_key(id, codol::mapents::KEY_ID);
				}
			}
		}

		doc.end_transaction();
		return ok;
	}

	bool scene::csg_subtract()
	{
		const auto box = doc.selection_bounds();
		if (!box.valid())
		{
			return doc.csg_subtract(); // reports "nothing selected"
		}

		// The document cuts every visible brush. The collision brushes of the zone are not drawn unless asked for:
		// cutting what cannot be seen would be a surprise, so those are set aside for the operation.
		std::vector<object_id> aside;

		if (show_brushes != brush_display::all)
		{
			sync();

			for (const auto& o : statics.objects)
			{
				if ((o.imported || o.compiled) && !doc.is_selected(o.id) && o.mins[0] <= box.maxs.x && o.maxs[0] >= box.mins.x && o.mins[1] <= box.maxs.y
					&& o.maxs[1] >= box.mins.y && o.mins[2] <= box.maxs.z && o.maxs[2] >= box.mins.z)
				{
					aside.push_back(o.id);
				}
			}
		}

		for (const object_id id : aside)
		{
			doc.set_hidden(id, true);
		}

		const bool ok = doc.csg_subtract();

		for (const object_id id : aside)
		{
			doc.set_hidden(id, false);
		}

		return ok;
	}

	bool scene::resize_selection(const int axis, const int side, const double distance)
	{
		if (distance == 0.0)
		{
			return false;
		}

		std::vector<std::pair<edit::face_ref, double>> moves;

		for (const object_id id : doc.selection())
		{
			const auto* geo = doc.geometry(id);
			if (!geo)
			{
				continue;
			}

			for (std::size_t f = 0; f < geo->planes.size(); f++)
			{
				const double n = geo->planes[f].normal[axis];

				if (!geo->face_vertices[f].empty() && n * side >= 0.7)
				{
					// along the normal, so that the face moves `distance` along the axis
					moves.push_back({ { id, static_cast<int>(f) }, distance * n });
				}
			}
		}

		if (moves.empty())
		{
			return false;
		}

		doc.begin_transaction("Resize");
		bool any = false;

		for (const auto& [face, amount] : moves)
		{
			any |= doc.move_face(face, amount);
		}

		doc.end_transaction();
		return any;
	}

	object_id scene::create_model_entity(const std::string_view model, const edit::vec3& origin, const double yaw)
	{
		std::vector<std::pair<std::string, std::string>> pairs;
		pairs.emplace_back("model", std::string(model));

		if (yaw != 0.0)
		{
			pairs.emplace_back("angles", edit::format_key_vec3({ 0.0, yaw, 0.0 }));
		}

		// a misc_model only exists as a static model of the compiled map: the key is what makes the map compile
		// ( codol_mapc ) add it, with a model the map zone does not have taken from the project's zone
		if (model_class == "misc_model")
		{
			pairs.emplace_back(codol::mapents::KEY_STATIC, "1");
		}

		return doc.create_point_entity(model_class, origin, pairs);
	}

	object_id scene::create_entity(const std::string_view classname, const edit::vec3& origin)
	{
		const auto* info = edit::class_catalog::c1().find(classname);

		if (info && !info->is_point())
		{
			return doc.create_brush_entity(classname);
		}

		if (classname == "light")
		{
			// a light comes with its keys, so that the mapper sees what there is to set: the values the compiler uses
			// for a light without them ( codol_bsp README, "Light keys" ). spawnflags 1 / 2 make it a primary light.
			std::vector<std::pair<std::string, std::string>> pairs;
			pairs.emplace_back("radius", "600");
			pairs.emplace_back("intensity", "1");
			pairs.emplace_back("_color", "1 1 1");
			return doc.create_point_entity(classname, origin, pairs);
		}

		return doc.create_point_entity(classname, origin);
	}

	edit::vec3 scene::selection_pivot() const
	{
		const auto sel = doc.selection();

		if (sel.size() == 1)
		{
			if (const auto* e = doc.find_entity(sel[0]))
			{
				return e->origin();
			}
		}

		const auto box = doc.selection_bounds();
		return box.valid() ? box.center() : edit::vec3{};
	}

	void scene::set_grid(const double g)
	{
		grid = std::clamp(g, 0.125, 512.0);
	}

	edit::vec3 scene::ray_target(const float3& origin, const float3& direction, const float fallback) const
	{
		view::pick_result hit;
		if (renderer.loaded() && renderer.pick_ray(origin, direction, hit) && hit.distance < 8000.0f)
		{
			return { hit.point.x, hit.point.y, hit.point.z };
		}

		const float len = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		const float s = len > 0.0f ? fallback / len : 0.0f;
		return { origin.x + direction.x * s, origin.y + direction.y * s, origin.z + direction.z * s };
	}

	void scene::look_at_selection()
	{
		edit::bounds box = doc.selection_bounds();

		if (!box.valid() && inspecting)
		{
			view::oriented_box ob;
			if (renderer.object_box(inspected.object, ob))
			{
				box.add({ ob.center.x - ob.half_size.x, ob.center.y - ob.half_size.y, ob.center.z - ob.half_size.z });
				box.add({ ob.center.x + ob.half_size.x, ob.center.y + ob.half_size.y, ob.center.z + ob.half_size.z });
			}
		}

		if (!box.valid())
		{
			return;
		}

		const auto c = box.center();
		const float radius = std::max(32.0f, static_cast<float>(edit::length(box.size()) * 0.5));
		const float back = radius / std::tan(cam.fov_x * 0.5f * 0.0174532925f) * 1.4f;
		const auto axes = cam.axes();

		cam.origin = { static_cast<float>(c.x) - axes.forward.x * back, static_cast<float>(c.y) - axes.forward.y * back, static_cast<float>(c.z) - axes.forward.z * back };
	}
}
