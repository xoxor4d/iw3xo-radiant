#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "mouse_capture.hpp"
#include "assets.hpp"
#include "preview.hpp"
#include "ui.hpp"
#include "ext.hpp"
#include "edit3d.hpp"

#include <ImGuizmo.h>
#include <psapi.h>

// The 3D view of the standalone editor: the map drawn by codol_view, as a view of the document. Selection, the
// transform gizmo and every edit go to the document ( scene.hpp ); this file draws, picks and navigates. Also here:
// the Selection window ( numeric transforms ) and the browsers for the models and materials of the game.

namespace editor
{
	namespace
	{
		constexpr float DEG = 0.0174532925f;

		// rows forward, left, up, translation: a D3D row vector world matrix
		void matrix_of(const edit::vec3& origin, const edit::vec3& angles, float m[16])
		{
			edit::vec3 axes[3];
			edit::angles_to_axes(angles, axes);

			for (int r = 0; r < 3; r++)
			{
				m[r * 4 + 0] = static_cast<float>(axes[r].x);
				m[r * 4 + 1] = static_cast<float>(axes[r].y);
				m[r * 4 + 2] = static_cast<float>(axes[r].z);
				m[r * 4 + 3] = 0.0f;
			}

			m[12] = static_cast<float>(origin.x);
			m[13] = static_cast<float>(origin.y);
			m[14] = static_cast<float>(origin.z);
			m[15] = 1.0f;
		}

		// a value of a float field of the gui, without the noise of the float ( 350.762 stays 350.762 in a key )
		double field_value(const float v)
		{
			return std::round(static_cast<double>(v) * 1e4) / 1e4;
		}

		const char* kind_name(const view::object_kind kind)
		{
			switch (kind)
			{
			case view::object_kind::world_surface: return "World surface";
			case view::object_kind::static_model: return "Static model";
			case view::object_kind::entity_model: return "Entity model";
			case view::object_kind::brush_model: return "Brush model";
			case view::object_kind::placed_model: return "Model";
			case view::object_kind::dynamic_mesh: return "Brush";
			}

			return "";
		}

		// The rotation that takes the gizmo from where the drag began to where it is now, as axis and angle.
		// `start` and `now` are row vector matrices ( rows = axes ), so now = start * R.
		bool rotation_between(const float start[16], const float now[16], edit::vec3& axis, double& degrees)
		{
			double s[3][3], c[3][3], r[3][3];

			for (int i = 0; i < 3; i++)
			{
				const double ls = std::sqrt(static_cast<double>(start[i * 4]) * start[i * 4] + static_cast<double>(start[i * 4 + 1]) * start[i * 4 + 1] + static_cast<double>(start[i * 4 + 2]) * start[i * 4 + 2]);
				const double lc = std::sqrt(static_cast<double>(now[i * 4]) * now[i * 4] + static_cast<double>(now[i * 4 + 1]) * now[i * 4 + 1] + static_cast<double>(now[i * 4 + 2]) * now[i * 4 + 2]);

				for (int j = 0; j < 3; j++)
				{
					s[i][j] = ls > 0.0 ? start[i * 4 + j] / ls : 0.0;
					c[i][j] = lc > 0.0 ? now[i * 4 + j] / lc : 0.0;
				}
			}

			// R = transpose( start ) * now
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					r[i][j] = s[0][i] * c[0][j] + s[1][i] * c[1][j] + s[2][i] * c[2][j];
				}
			}

			const double cosine = std::clamp((r[0][0] + r[1][1] + r[2][2] - 1.0) * 0.5, -1.0, 1.0);
			const double angle = std::acos(cosine);

			if (angle < 1e-6)
			{
				return false;
			}

			// the matrix for column vectors is the transpose of R
			axis = { r[1][2] - r[2][1], r[2][0] - r[0][2], r[0][1] - r[1][0] };

			if (edit::length(axis) < 1e-6)
			{
				// half a turn: the axis is the eigenvector of the symmetric matrix
				const int k = r[0][0] >= r[1][1] ? (r[0][0] >= r[2][2] ? 0 : 2) : (r[1][1] >= r[2][2] ? 1 : 2);
				axis = { r[k][0] + r[0][k], r[k][1] + r[1][k], r[k][2] + r[2][k] };
				axis[k] = r[k][k] + 1.0;
			}

			axis = edit::normalize(axis);
			degrees = angle / edit::PI * 180.0;
			return edit::length(axis) > 0.5;
		}
	}
}

namespace ggui
{
	using editor::float3;
	using editor::line_vertex;
	using editor::object_id;
	namespace view = codol::view;
	namespace edit = codol::edit;
	namespace ui = editor::ui;

	// state of the 3D view ( the dialog's impl; a named type so that the console commands can reach it )
	struct viewport_state
	{
		bool world = true;
		bool static_models = true;
		bool entity_models = true;
		bool brush_models = true;
		bool placed_models = true;
		bool meshes = true;
		bool sun_shadows = true;
		bool wireframe = false;
		bool model_lods = false;
		bool show_outline = true;

		float fov = 80.0f;
		float fly_speed = 600.0f;
		float wire_distance = 6000.0f;

		bool next_spawn = false;
		bool overview = false;
		bool focus = false;

		// the frame as it was drawn last ( for clicks that come from the console )
		int view_width = 0;
		int view_height = 0;

		ImVec2 press_pos = {};
		bool press_valid = false;
		bool press_dragged = false;		// the left press moved more than a few pixels ( stays so: the captured cursor sits still )
		ImVec2 right_press_pos = {};
		bool right_press_valid = false;

		// gizmo: the matrix lives across the frames of a drag, the document gets the total every time
		float gizmo_matrix[16] = {};
		float gizmo_start[16] = {};
		bool gizmo_active = false;
		bool gizmo_hover = false;		// the mouse was on the gizmo in the last frame
		edit::vec3 gizmo_pivot;

		// ---- line buffers of the overlay, world space
		struct wire_cell
		{
			float mins[3] = { 1e30f, 1e30f, 1e30f };
			float maxs[3] = { -1e30f, -1e30f, -1e30f };
			std::vector<line_vertex> lines;
		};

		std::vector<wire_cell> wire_cells;			// collision brushes of the zone, by area
		std::vector<line_vertex> entity_boxes;		// point entities without a model
		std::vector<line_vertex> link_lines;		// target -> targetname
		std::vector<line_vertex> selection_lines;	// brushes, entity boxes
		std::vector<line_vertex> model_outline;		// triangles of selected models
		std::vector<line_vertex> face_fill;			// selected faces, triangle list
		std::uint64_t built_statics = ~0ull;
		std::uint64_t built_selected = ~0ull;
		editor::brush_display built_mode = editor::brush_display::off;
		bool built_inspect = false;
		int built_inspect_id = -1;

		// browsers
		std::string model_filter;
		std::string material_filter;
		int material_group = 0;

		view::pick_options pick_options() const;
		void rebuild_static_lines();
		void rebuild_selection_lines();
		void draw_overlay(const view::overlay_context& ctx);
		void draw_labels(ImDrawList* dl, const ImVec2& origin, int width, int height);
		bool draw_gizmo(ImDrawList* dl, const ImVec2& origin, int width, int height);
		void draw_selection_window();
		void draw_models_window();
	};

	struct codol_viewport_dialog::impl : viewport_state
	{
	};

	namespace
	{
		viewport_state* g_impl = nullptr;

		void add_box(std::vector<line_vertex>& out, const float mins[3], const float maxs[3], const DWORD color)
		{
			static constexpr int edges[12][2] = { { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 }, { 0, 2 }, { 1, 3 }, { 4, 6 }, { 5, 7 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };

			for (const auto& e : edges)
			{
				for (const int i : e)
				{
					out.push_back({ (i & 1) ? maxs[0] : mins[0], (i & 2) ? maxs[1] : mins[1], (i & 4) ? maxs[2] : mins[2], color });
				}
			}
		}
	}

	codol_viewport_dialog::codol_viewport_dialog() : m(std::make_unique<impl>())
	{
		g_impl = m.get();
		ui::register_commands();
		editor::edit3d::register_commands();

		// ed_mem [label]: memory of the process and of the device, one line ( a run that opens map after map prints it
		// after each: what grows without bound is a leak )
		command::register_command("ed_mem", [](const std::vector<std::string>& args)
		{
			PROCESS_MEMORY_COUNTERS_EX counters = {};
			counters.cb = sizeof(counters);
			GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters), sizeof(counters));

			const auto& s = editor::get_scene();
			const auto stats = s.renderer.stats();
			const UINT texture_mb = game::glob::d3d9_device ? game::glob::d3d9_device->GetAvailableTextureMem() / (1024u * 1024u) : 0u;

			game::printf_to_console("ed_mem %s: working set %d MB, private %d MB, device texture memory available %u MB, map %s, %d surfaces, %d meshes\n",
				args.size() > 1 ? args[1].c_str() : "-", static_cast<int>(counters.WorkingSetSize / (1024 * 1024)), static_cast<int>(counters.PrivateUsage / (1024 * 1024)),
				texture_mb, s.open ? s.map_name.c_str() : "( none )", stats.world_surface_count, stats.mesh_count);
		});
	}

	codol_viewport_dialog::~codol_viewport_dialog()
	{
		g_impl = nullptr;
	}

	void codol_viewport_dialog::unload()
	{
		ui::close_map(true);
	}

	// "codol_viewport <map>" and whatever else opens a zone by name
	bool codol_viewport_dialog::load_map(const std::string& map_name)
	{
		ui::open_zone(map_name, true);
		this->open();
		return editor::get_scene().open;
	}

	void codol_viewport_dialog::on_device_lost()
	{
		editor::preview::device_lost();
		editor::render_modes::device_lost();

		if (auto& s = editor::get_scene(); s.renderer.initialised())
		{
			s.renderer.on_device_lost();
		}
	}

	void codol_viewport_dialog::on_device_reset()
	{
		if (auto& s = editor::get_scene(); s.renderer.initialised())
		{
			s.renderer.on_device_reset();
		}
	}

	// ---- picking ---------------------------------------------------------------------------------------------

	view::pick_options viewport_state::pick_options() const
	{
		view::pick_options options;
		options.world = world;
		options.static_models = static_models;
		options.entity_models = entity_models;
		options.brush_models = brush_models;
		options.placed_models = placed_models;
		options.meshes = meshes;
		return options;
	}
}

namespace editor::ui
{
	void pick_ray(const float3& origin, const float3& direction, const bool toggle, const bool face)
	{
		auto& s = get_scene();
		auto& doc = s.doc;

		const float len = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		if (len <= 0.0f)
		{
			return;
		}

		const float3 dir{ direction.x / len, direction.y / len, direction.z / len };

		// Patch menu, terrain edge mode: the click turns the diagonal of the terrain quad it hits
		if (!toggle && !face && patch_edge_click(origin, dir))
		{
			return;
		}

		// what the renderer draws
		view::pick_result drawn;
		const bool drawn_hit = s.renderer.loaded() && s.renderer.pick_ray(origin, dir, drawn, ggui::g_impl ? ggui::g_impl->pick_options() : view::pick_options{});

		// what only the document knows: brushes that are shown as wires, boxes of point entities without a model
		const edit::ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };
		edit::pick_options options;
		options.entities = s.show_entity_boxes;

		edit::pick_result wire;
		bool wire_hit = false;

		for (const auto& hit : doc.pick_all(ray, options))
		{
			if (hit.kind == edit::object_kind::entity)
			{
				if (s.has_model(hit.object))
				{
					continue; // its triangles are picked, not its box
				}
			}
			else if (s.show_brushes != brush_display::all && !doc.is_selected(hit.object) && s.imported(hit.object))
			{
				continue; // an invisible collision brush
			}

			wire = hit;
			wire_hit = true;
			break;
		}

		object_id target = edit::no_object;
		int target_face = -1;

		if (wire_hit && (!drawn_hit || wire.distance <= drawn.distance + 0.05))
		{
			target = wire.object;
			target_face = wire.face;
		}
		else if (drawn_hit)
		{
			target = s.object_of(drawn.object);

			if (target != edit::no_object && !doc.is_selectable(target))
			{
				return; // shown, but not to be picked ( Filter window: "don't select .." )
			}

			if (target == edit::no_object)
			{
				// a surface of the world: something to look at, nothing the document owns
				if (!toggle && !face)
				{
					doc.deselect_all();
					s.inspecting = true;
					s.inspected = drawn;
				}

				return;
			}

			if (const auto* geo = doc.geometry(target))
			{
				double distance = 0.0;
				edit::ray_brush(ray, *geo, distance, target_face);
			}
		}

		if (target == edit::no_object)
		{
			if (!toggle && !face)
			{
				deselect();
			}

			return;
		}

		s.inspecting = false;

		if (face)
		{
			if (target_face >= 0 && doc.kind(target) == edit::object_kind::brush)
			{
				doc.select_face({ target, target_face }, edit::select_mode::toggle);
			}

			return;
		}

		doc.select(target, toggle ? edit::select_mode::toggle : edit::select_mode::replace);
	}

	void viewport_click(const float px, const float py, const bool toggle, const bool face)
	{
		auto& s = get_scene();
		if (!ggui::g_impl || ggui::g_impl->view_width <= 0)
		{
			return;
		}

		float3 origin, direction;
		view::screen_ray(s.cam, ggui::g_impl->view_width, ggui::g_impl->view_height, px, py, origin, direction);
		pick_ray(origin, direction, toggle, face);
	}

	// what the camera actions ask of the view: "next_spawn", "overview", "focus" ( done when the view draws next )
	void viewport_request(const std::string_view what)
	{
		if (ggui::g_impl)
		{
			if (what == "next_spawn") { ggui::g_impl->next_spawn = true; }
			else if (what == "overview") { ggui::g_impl->overview = true; }
			else if (what == "focus") { ggui::g_impl->focus = true; }
		}
	}

	void set_material_filter(const std::string& text, const int group)
	{
		if (ggui::g_impl)
		{
			ggui::g_impl->material_filter = text;

			if (group >= 0 && group <= 2)
			{
				ggui::g_impl->material_group = group;
			}
		}
	}
}

namespace ggui
{
	// ---- overlay ---------------------------------------------------------------------------------------------

	void viewport_state::rebuild_static_lines()
	{
		auto& s = editor::get_scene();

		wire_cells.clear();

		// by area, so that what is behind the camera or far away is not sent to the device at all
		constexpr float cell_size = 1024.0f;
		std::unordered_map<std::uint64_t, std::size_t> index;
		const bool all = s.show_brushes == editor::brush_display::all;

		for (const auto& object : s.statics.objects)
		{
			// New and edited brushes are drawn solid. Those of a compiled brush model cannot be ( the zone draws
			// the model as it was compiled ): they are always shown as wires, so that an edit is never invisible.
			// ( Selected brushes are in here as well, under their outline; while one is dragged its wire stays
			// where the drag began. )
			const bool edited_model = object.compiled && !object.imported;

			const bool wire_patch = object.patch && editor::ui::patches_as_wire();		// Patch menu: patches as wireframes

			if (!wire_patch && (!(object.imported || object.compiled) || !(all || edited_model)))
			{
				continue;
			}

			const auto cx = static_cast<std::int32_t>(std::floor((object.mins[0] + object.maxs[0]) * 0.5f / cell_size));
			const auto cy = static_cast<std::int32_t>(std::floor((object.mins[1] + object.maxs[1]) * 0.5f / cell_size));
			const std::uint64_t key = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(cx)) << 32) | static_cast<std::uint32_t>(cy);

			auto it = index.find(key);
			if (it == index.end())
			{
				it = index.emplace(key, wire_cells.size()).first;
				wire_cells.emplace_back();
			}

			auto& cell = wire_cells[it->second];
			const DWORD color = (editor::to_d3d_color(object.color) & 0x00FFFFFFu) | (edited_model ? 0xFF000000u : 0xA0000000u);

			for (int i = 0; i < 3; i++)
			{
				cell.mins[i] = std::min(cell.mins[i], object.mins[i]);
				cell.maxs[i] = std::max(cell.maxs[i], object.maxs[i]);
			}

			for (std::uint32_t i = 0; i < object.count; i++)
			{
				const auto& p = s.statics.points[object.first + i];
				cell.lines.push_back({ p.x, p.y, p.z, color });
			}
		}
	}

	void viewport_state::rebuild_selection_lines()
	{
		auto& s = editor::get_scene();
		auto& doc = s.doc;

		selection_lines.clear();
		model_outline.clear();
		face_fill.clear();
		entity_boxes.clear();
		link_lines.clear();

		// boxes of the point entities without a model ( the selected ones get theirs below ), and the target lines:
		// both move with a drag
		if (s.show_entity_boxes)
		{
			for (const auto& e : s.statics.entities)
			{
				if ((!e.has_model || editor::render_modes::entity_boxes_always()) && !doc.is_selected(e.id))
				{
					add_box(entity_boxes, e.mins, e.maxs, editor::to_d3d_color(e.color));
				}
			}
		}

		if (s.show_connections)
		{
			for (const auto& c : s.connections)
			{
				link_lines.push_back({ c.from.x, c.from.y, c.from.z, 0xFFFF40FF });
				link_lines.push_back({ c.to.x, c.to.y, c.to.z, 0xFFFFC0FF });
			}
		}

		constexpr DWORD brush_color = 0xFFFF3A20;
		constexpr DWORD model_color = 0xFFFFA020;
		constexpr DWORD box_color = 0xFFFFE060;

		for (const auto& object : s.selected.objects)
		{
			for (std::uint32_t i = 0; i < object.count; i++)
			{
				const auto& p = s.selected.points[object.first + i];
				selection_lines.push_back({ p.x, p.y, p.z, brush_color });
			}
		}

		std::vector<view::object_ref> refs;
		std::vector<float3> tris;
		std::size_t budget = 3 * 120000; // triangle corners; more would only be noise ( and slow )

		const auto outline_refs = [&]
		{
			for (const auto& ref : refs)
			{
				tris.clear();

				if (budget > 0 && s.renderer.object_triangles(ref, tris) && tris.size() <= budget)
				{
					budget -= tris.size();

					for (std::size_t i = 0; i + 2 < tris.size(); i += 3)
					{
						for (int e = 0; e < 3; e++)
						{
							const auto& a = tris[i + static_cast<std::size_t>(e)];
							const auto& b = tris[i + static_cast<std::size_t>((e + 1) % 3)];
							model_outline.push_back({ a.x, a.y, a.z, model_color });
							model_outline.push_back({ b.x, b.y, b.z, model_color });
						}
					}
				}
			}
		};

		for (const auto& e : s.selected.entities)
		{
			add_box(selection_lines, e.mins, e.maxs, e.has_model ? box_color : brush_color);

			refs.clear();
			s.refs_of(e.id, refs);
			outline_refs();
		}

		// where a selected light reaches: its radius, the cone of a spot
		{
			const auto& lights = ui::selected_light_lines();
			selection_lines.insert(selection_lines.end(), lights.begin(), lights.end());
		}

		// the brush model the zone draws for a selected brush entity
		for (const object_id id : doc.selected_entities())
		{
			if (const auto* e = doc.find_entity(id); e && !e->children.empty())
			{
				refs.clear();
				s.refs_of(id, refs);
				outline_refs();
			}
		}

		// a world surface that is looked at
		if (s.inspecting)
		{
			refs.assign(1, s.inspected.object);
			outline_refs();
		}

		// selected faces: filled
		for (const auto& f : doc.selected_faces())
		{
			const auto* geo = doc.geometry(f.brush);
			if (!geo || f.face < 0 || static_cast<std::size_t>(f.face) >= geo->face_vertices.size())
			{
				continue;
			}

			const auto& loop = geo->face_vertices[static_cast<std::size_t>(f.face)];

			for (std::size_t i = 1; i + 1 < loop.size(); i++)
			{
				for (const std::size_t k : { static_cast<std::size_t>(0), i, i + 1 })
				{
					const auto& p = geo->vertices[static_cast<std::size_t>(loop[k])];
					face_fill.push_back({ static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z), 0x60FFE020 });
				}
			}

			for (std::size_t i = 0; i < loop.size(); i++)
			{
				const auto& a = geo->vertices[static_cast<std::size_t>(loop[i])];
				const auto& b = geo->vertices[static_cast<std::size_t>(loop[(i + 1) % loop.size()])];
				selection_lines.push_back({ static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z), 0xFFFFE020 });
				selection_lines.push_back({ static_cast<float>(b.x), static_cast<float>(b.y), static_cast<float>(b.z), 0xFFFFE020 });
			}
		}
	}

	void viewport_state::draw_overlay(const view::overlay_context& ctx)
	{
		auto& s = editor::get_scene();
		auto* device = ctx.device;

		const auto draw = [&](const std::vector<line_vertex>& lines)
		{
			constexpr std::size_t chunk = 20000; // lines per call

			for (std::size_t first = 0; first < lines.size(); first += chunk * 2)
			{
				const auto count = std::min(chunk, (lines.size() - first) / 2);
				device->DrawPrimitiveUP(D3DPT_LINELIST, static_cast<UINT>(count), lines.data() + first, sizeof(line_vertex));
			}
		};

		device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// lines that lie exactly on surfaces: pull them towards the camera
		const float bias = -0.00003f;
		device->SetRenderState(D3DRS_DEPTHBIAS, *reinterpret_cast<const DWORD*>(&bias));

		// ---- collision brushes: only the areas in view and near
		if (!wire_cells.empty())
		{
			const auto axes = s.cam.axes();
			const float tan_x = std::tan(s.cam.fov_x * 0.5f * editor::DEG);
			const float tan_y = tan_x * static_cast<float>(ctx.height) / static_cast<float>(std::max(1, ctx.width));
			const float3 right{ -axes.left.x, -axes.left.y, -axes.left.z };

			// the four side planes of the view, normals pointing in
			const float3 normals[4] = {
				{ axes.forward.x * tan_x - right.x, axes.forward.y * tan_x - right.y, axes.forward.z * tan_x - right.z },
				{ axes.forward.x * tan_x + right.x, axes.forward.y * tan_x + right.y, axes.forward.z * tan_x + right.z },
				{ axes.forward.x * tan_y - axes.up.x, axes.forward.y * tan_y - axes.up.y, axes.forward.z * tan_y - axes.up.z },
				{ axes.forward.x * tan_y + axes.up.x, axes.forward.y * tan_y + axes.up.y, axes.forward.z * tan_y + axes.up.z } };

			for (const auto& cell : wire_cells)
			{
				const float c[3] = { (cell.mins[0] + cell.maxs[0]) * 0.5f - s.cam.origin.x, (cell.mins[1] + cell.maxs[1]) * 0.5f - s.cam.origin.y, (cell.mins[2] + cell.maxs[2]) * 0.5f - s.cam.origin.z };
				const float h[3] = { (cell.maxs[0] - cell.mins[0]) * 0.5f, (cell.maxs[1] - cell.mins[1]) * 0.5f, (cell.maxs[2] - cell.mins[2]) * 0.5f };

				// distance from the camera to the box
				float d2 = 0.0f;
				for (int i = 0; i < 3; i++)
				{
					const float d = std::max(0.0f, std::fabs(c[i]) - h[i]);
					d2 += d * d;
				}

				if (d2 > wire_distance * wire_distance)
				{
					continue;
				}

				bool outside = false;
				for (const auto& n : normals)
				{
					if (n.x * c[0] + n.y * c[1] + n.z * c[2] + std::fabs(n.x) * h[0] + std::fabs(n.y) * h[1] + std::fabs(n.z) * h[2] < 0.0f)
					{
						outside = true;
						break;
					}
				}

				if (!outside)
				{
					draw(cell.lines);
				}
			}
		}

		draw(entity_boxes);
		draw(link_lines);
		editor::render_modes::draw_overlay(ctx);
		draw(ui::extra_view_lines(false));		// contents of prefabs, the region, the leak line

		if (show_outline)
		{
			draw(model_outline);
			draw(selection_lines);

			if (!face_fill.empty())
			{
				device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, static_cast<UINT>(face_fill.size() / 3), face_fill.data(), sizeof(line_vertex));
			}

			// once more through everything, faint: a selection behind a wall can be found ( not for half the map )
			if (selection_lines.size() > 200000)
			{
				return;
			}

			device->SetRenderState(D3DRS_ZENABLE, FALSE);
			device->SetRenderState(D3DRS_TEXTUREFACTOR, 0x38FFFFFF);
			device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			draw(selection_lines);
		}
	}

	// classnames next to the boxes of nearby point entities
	void viewport_state::draw_labels(ImDrawList* dl, const ImVec2& origin, const int width, const int height)
	{
		auto& s = editor::get_scene();
		if (!s.show_labels || !s.show_entity_boxes)
		{
			return;
		}

		auto* font = ImGui::GetIO().Fonts->Fonts.Size > 2 ? ImGui::GetIO().Fonts->Fonts[2] : ImGui::GetFont();
		constexpr float reach = 700.0f;
		int budget = 80;

		for (const auto* list : { &s.statics.entities, &s.selected.entities })
		{
			for (const auto& e : *list)
			{
				if (e.has_model || budget <= 0 || (list == &s.statics.entities && s.doc.is_selected(e.id)))
				{
					continue;
				}

				const float dx = e.origin[0] - s.cam.origin.x, dy = e.origin[1] - s.cam.origin.y, dz = e.origin[2] - s.cam.origin.z;
				if (dx * dx + dy * dy + dz * dz > reach * reach)
				{
					continue;
				}

				float xy[2], depth = 0.0f;
				if (!view::project(s.cam, width, height, { (e.mins[0] + e.maxs[0]) * 0.5f, (e.mins[1] + e.maxs[1]) * 0.5f, e.maxs[2] + 4.0f }, xy, depth)
					|| xy[0] < 0.0f || xy[1] < 0.0f || xy[0] > static_cast<float>(width) || xy[1] > static_cast<float>(height))
				{
					continue;
				}

				const ImVec2 size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, e.classname.c_str());
				const ImVec2 at(origin.x + xy[0] - size.x * 0.5f, origin.y + xy[1] - size.y);
				dl->AddText(font, font->FontSize, ImVec2(at.x + 1.0f, at.y + 1.0f), IM_COL32(0, 0, 0, 200), e.classname.c_str());
				dl->AddText(font, font->FontSize, at, e.color, e.classname.c_str());
				budget--;
			}
		}
	}

	// ---- gizmo -----------------------------------------------------------------------------------------------

	// returns true while the gizmo has the mouse ( hovered or dragged )
	bool viewport_state::draw_gizmo(ImDrawList* dl, const ImVec2& origin, const int width, const int height)
	{
		auto& s = editor::get_scene();
		auto& doc = s.doc;
		auto& st = ui::get();
		const auto& io = ImGui::GetIO();

		// vertex / edge tool: the gizmo is on the selected handles; clipper: none
		if (editor::edit3d::replaces_gizmo())
		{
			if (gizmo_active)
			{
				gizmo_active = false;
				s.end_drag();
			}

			return editor::edit3d::handle_gizmo(dl, origin, width, height);
		}

		if (doc.selection().empty())
		{
			if (gizmo_active)
			{
				gizmo_active = false;
				s.end_drag();
			}

			return false;
		}

		// another drag ( a 2D view ) owns the document
		if (s.dragging() && !gizmo_active)
		{
			return false;
		}

		if (!gizmo_active)
		{
			gizmo_pivot = s.selection_pivot();
			edit::vec3 angles;

			// local axes make sense for one entity
			if (!st.gizmo_world && doc.selection().size() == 1)
			{
				if (const auto* e = doc.find_entity(doc.selection()[0]))
				{
					angles = e->angles();
				}
			}

			editor::matrix_of(gizmo_pivot, angles, gizmo_matrix);
		}

		float view_m[16], proj_m[16];
		view::view_matrix(s.cam, view_m);
		view::projection_matrix(s.cam, width, height, proj_m);

		// the renderer's projection has no far plane; the gizmo wants one
		const float n = s.cam.z_near, f = 262144.0f;
		proj_m[10] = f / (f - n);
		proj_m[14] = -n * f / (f - n);
		proj_m[11] = 1.0f;
		proj_m[15] = 0.0f;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(dl);
		ImGuizmo::SetRect(origin.x, origin.y, static_cast<float>(width), static_cast<float>(height));
		ImGuizmo::SetGizmoSizeClipSpace(0.13f);

		const ImGuizmo::OPERATION op = st.gizmo == ui::gizmo_tool::rotate ? ImGuizmo::ROTATE : (st.gizmo == ui::gizmo_tool::scale ? ImGuizmo::SCALE : ImGuizmo::TRANSLATE);

		// Ctrl while dragging: the opposite of the snap setting
		const bool snap = s.snap != io.KeyCtrl;
		const float step = op == ImGuizmo::ROTATE ? s.snap_angle : (op == ImGuizmo::TRANSLATE ? static_cast<float>(s.grid) : 0.125f);
		const float snap_values[3] = { step, step, step };

		dl->PushClipRect(origin, ImVec2(origin.x + static_cast<float>(width), origin.y + static_cast<float>(height)), true);
		const bool changed = ImGuizmo::Manipulate(view_m, proj_m, op, st.gizmo_world && op != ImGuizmo::SCALE ? ImGuizmo::WORLD : ImGuizmo::LOCAL,
			gizmo_matrix, nullptr, snap ? snap_values : nullptr);
		dl->PopClipRect();

		const bool busy = ImGuizmo::IsUsing();

		if (busy && !gizmo_active)
		{
			// mouse down on the gizmo: one undo step from here to the release
			gizmo_active = true;
			std::memcpy(gizmo_start, gizmo_matrix, sizeof(gizmo_start));
			s.begin_drag(op == ImGuizmo::ROTATE ? "Rotate" : (op == ImGuizmo::SCALE ? "Scale" : "Move"));
		}

		if (gizmo_active && changed)
		{
			// the whole movement since the mouse went down, applied to the document as it was then
			s.restart_drag();

			if (op == ImGuizmo::TRANSLATE)
			{
				s.translate({ static_cast<double>(gizmo_matrix[12]) - gizmo_start[12], static_cast<double>(gizmo_matrix[13]) - gizmo_start[13], static_cast<double>(gizmo_matrix[14]) - gizmo_start[14] });
			}
			else if (op == ImGuizmo::ROTATE)
			{
				edit::vec3 axis;
				double degrees = 0.0;

				if (editor::rotation_between(gizmo_start, gizmo_matrix, axis, degrees))
				{
					s.rotate(gizmo_pivot, axis, degrees);
				}
			}
			else
			{
				edit::vec3 factors{ 1.0, 1.0, 1.0 };

				for (int i = 0; i < 3; i++)
				{
					const auto row_length = [i](const float* mat)
					{
						return std::sqrt(static_cast<double>(mat[i * 4]) * mat[i * 4] + static_cast<double>(mat[i * 4 + 1]) * mat[i * 4 + 1] + static_cast<double>(mat[i * 4 + 2]) * mat[i * 4 + 2]);
					};

					const double before = row_length(gizmo_start), now = row_length(gizmo_matrix);
					factors[i] = before > 1e-9 ? std::clamp(now / before, 0.01, 100.0) : 1.0;
				}

				s.scale(gizmo_pivot, factors);
			}
		}

		if (!busy && gizmo_active)
		{
			gizmo_active = false;
			s.end_drag();
		}

		return busy || ImGuizmo::IsOver();
	}

	// ---- Selection window ------------------------------------------------------------------------------------

	void viewport_state::draw_selection_window()
	{
		auto& st = ui::get();
		if (!st.show_selection)
		{
			return;
		}

		auto& s = editor::get_scene();
		auto& doc = s.doc;

		ImGui::SetNextWindowSize(ImVec2(380.0f, 520.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Selection##window", &st.show_selection, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		// ---- a surface of the world: look, do not touch
		if (s.inspecting && doc.selection().empty())
		{
			view::scene_object object;
			s.renderer.object(s.inspected.object, object);

			ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.25f, 1.0f), "%s", editor::kind_name(s.inspected.object.kind));
			ImGui::SameLine();
			ImGui::TextDisabled("#%d", s.inspected.object.id);
			ImGui::TextWrapped("%s", s.inspected.material.c_str());

			if (!s.inspected.material.empty())
			{
				if (auto* thumb = s.renderer.material_thumbnail(s.inspected.material, 128))
				{
					ImGui::Image(thumb, ImVec2(128.0f, 128.0f));
				}

				view::material_info info;
				if (s.renderer.material_details(s.inspected.material, info))
				{
					ImGui::TextDisabled("%s   %d x %d   %s", info.technique_set.c_str(), info.width, info.height, info.zone.c_str());
				}

				if (ImGui::Button("Use this material for new brushes"))
				{
					doc.set_default_material(s.inspected.material);
				}
			}

			ImGui::Spacing();
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled("The surfaces of the world are compiled into the zone. They can be looked at, not edited; the document holds the entities, the models and the collision brushes.");
			ImGui::PopTextWrapPos();
			ImGui::End();
			return;
		}

		const auto sel = doc.selection();

		if (sel.empty())
		{
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled(s.open ? "Nothing selected.\nClick something in a view ( Ctrl or Shift + click adds )." : "No map is open.");
			ImGui::PopTextWrapPos();
			ImGui::End();
			return;
		}

		int brushes = 0, patches = 0, entities = 0;
		for (const object_id id : sel)
		{
			const auto kind = doc.kind(id);
			brushes += kind == edit::object_kind::brush;
			patches += kind == edit::object_kind::patch;
			entities += kind == edit::object_kind::entity;
		}

		std::string summary;
		const auto part = [&](const int count, const char* one, const char* many)
		{
			if (count)
			{
				summary += (summary.empty() ? "" : ", ") + std::to_string(count) + " " + (count == 1 ? one : many);
			}
		};

		part(brushes, "brush", "brushes");
		part(patches, "patch", "patches");
		part(entities, "entity", "entities");
		ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.25f, 1.0f), "%s", summary.c_str());

		const auto box = doc.selection_bounds();
		if (box.valid())
		{
			const auto size = box.size();
			ImGui::TextDisabled("size %.6g x %.6g x %.6g", size.x, size.y, size.z);
		}

		ImGui::Spacing();

		// A field that is dragged or typed into changes the document on every frame; all changes of one activation
		// are one undo step ( the merge key of the transaction ).
		static std::uint64_t activation = 1;
		const auto step = [&](const char* name, const auto& apply)
		{
			doc.begin_transaction(name, 0x73656c00ull + activation);
			apply();
			doc.end_transaction();
		};

		// ---- position of the pivot
		{
			const auto pivot = s.selection_pivot();
			float position[3] = { static_cast<float>(pivot.x), static_cast<float>(pivot.y), static_cast<float>(pivot.z) };

			ImGui::SetNextItemWidth(-70.0f);
			const bool changed = ImGui::DragFloat3("position", position, 1.0f, 0.0f, 0.0f, "%.2f");

			if (ImGui::IsItemActivated())
			{
				activation++;
			}

			if (changed)
			{
				step("Move", [&] { s.translate({ editor::field_value(position[0]) - pivot.x, editor::field_value(position[1]) - pivot.y, editor::field_value(position[2]) - pivot.z }); });
			}
		}

		// ---- one entity: its angles and scale as they are
		if (sel.size() == 1 && entities == 1)
		{
			const object_id id = sel[0];
			const auto* e = doc.find_entity(id);
			const auto a = e->angles();
			float angles[3] = { static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z) };

			ImGui::SetNextItemWidth(-70.0f);
			bool changed = ImGui::DragFloat3("angles", angles, 1.0f, 0.0f, 0.0f, "%.2f");

			if (ImGui::IsItemActivated())
			{
				activation++;
			}

			if (changed)
			{
				step("Rotate", [&] { doc.set_key(id, "angles", edit::format_key_vec3({ editor::field_value(angles[0]), editor::field_value(angles[1]), editor::field_value(angles[2]) })); });
			}

			if (!doc.find_entity(id)->value("model").empty())
			{
				float scale = static_cast<float>(editor::scene::model_scale(*doc.find_entity(id)));

				ImGui::SetNextItemWidth(-70.0f);
				changed = ImGui::DragFloat("scale", &scale, 0.01f, 0.01f, 100.0f, "%.3f");

				if (ImGui::IsItemActivated())
				{
					activation++;
				}

				if (changed)
				{
					step("Scale", [&] { doc.set_key(id, "modelscale", edit::format_key_number(editor::field_value(std::clamp(scale, 0.01f, 100.0f)))); });
				}
			}
		}

		// ---- relative, for anything
		ImGui::Spacing();
		ImGui::Separator();

		static float move_by[3] = { 0.0f, 0.0f, 0.0f };
		static float rotate_by = 90.0f;
		static int rotate_axis = 2;
		static float scale_by[3] = { 1.0f, 1.0f, 1.0f };

		ImGui::SetNextItemWidth(-110.0f);
		ImGui::InputFloat3("##move_by", move_by, "%.2f");
		ImGui::SameLine();

		if (ImGui::Button("Move", ImVec2(-1.0f, 0.0f)))
		{
			ui::report(s.translate({ move_by[0], move_by[1], move_by[2] }), "Move");
		}

		ImGui::SetNextItemWidth(60.0f);
		ImGui::Combo("##rotate_axis", &rotate_axis, "X\0Y\0Z\0");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-110.0f);
		ImGui::InputFloat("##rotate_by", &rotate_by, 0.0f, 0.0f, "%.2f deg");
		ImGui::SameLine();

		if (ImGui::Button("Rotate", ImVec2(-1.0f, 0.0f)))
		{
			edit::vec3 axis;
			axis[rotate_axis] = 1.0;
			ui::report(s.rotate(s.selection_pivot(), axis, rotate_by), "Rotate");
		}

		ImGui::SetNextItemWidth(-110.0f);
		ImGui::InputFloat3("##scale_by", scale_by, "%.3f");
		ImGui::SameLine();

		if (ImGui::Button("Scale", ImVec2(-1.0f, 0.0f)))
		{
			ui::report(s.scale(s.selection_pivot(), { scale_by[0], scale_by[1], scale_by[2] }), "Scale");
		}

		ImGui::Spacing();

		if (ImGui::Button("Go to")) { s.look_at_selection(); }
		ImGui::SameLine();
		if (ImGui::Button("Hide")) { ui::hide_selected(); }
		ImGui::SameLine();
		if (ImGui::Button("Clone")) { ui::clone_selection(); }
		ImGui::SameLine();
		if (ImGui::Button("Delete")) { ui::delete_selection(); }

		// ---- what is selected
		ImGui::Spacing();
		ImGui::Separator();

		if (ImGui::BeginChild("##sel_list", ImVec2(0.0f, 0.0f)))
		{
			const auto current = doc.selection();
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(current.size()));

			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					const object_id id = current[static_cast<std::size_t>(i)];

					if (const auto* e = doc.find_entity(id))
					{
						ImGui::Text("#%u  %s  %s", id, std::string(e->classname()).c_str(), std::string(e->value("model")).c_str());
					}
					else if (const auto* b = doc.find_brush(id))
					{
						const auto* owner = doc.find_entity(b->owner);
						ImGui::Text("#%u  brush  %s  %s%s", id, b->faces.empty() ? "" : b->faces[0].texture.material.c_str(),
							owner && !owner->is_worldspawn() ? std::string(owner->classname()).c_str() : "", s.imported(id) ? "" : "  ( edited )");
					}
					else
					{
						ImGui::Text("#%u  patch", id);
					}
				}
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}

	// ---- Models ----------------------------------------------------------------------------------------------

	void viewport_state::draw_models_window()
	{
		auto& st = ui::get();
		if (!st.show_models)
		{
			return;
		}

		auto& s = editor::get_scene();

		ImGui::SetNextWindowSize(ImVec2(380.0f, 600.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Models##window", &st.show_models, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return;
		}

		if (!s.renderer.loaded())
		{
			ImGui::PushTextWrapPos(0.0f);
			ImGui::TextDisabled("Open a map first: the models of its zones and of the shared zones are listed here.");
			ImGui::PopTextWrapPos();
			ImGui::End();
			return;
		}

		const auto& names = s.renderer.model_names();

		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputTextWithHint("##model_filter", "filter", &model_filter);

		// what the new entity is: a script_model is spawned by the server from the entity override; a misc_model
		// only exists in a compiled map
		ImGui::SetNextItemWidth(160.0f);
		if (ImGui::BeginCombo("##model_class", s.model_class.c_str()))
		{
			for (const char* name : { "script_model", "misc_model" })
			{
				if (ImGui::Selectable(name, s.model_class == name))
				{
					s.model_class = name;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::SameLine();
		ImGui::TextDisabled("%d models. Click: preview. Double click: new entity where the camera looks.", static_cast<int>(names.size()));

		// a model of the map's own project ( assets.hpp ); a row of its own: the window is often narrow
		if (ImGui::Button("Import model .."))
		{
			editor::assets::open_model_dialog();
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("A model of your own from xmodel_export files ( one per LOD ): it is built into the project's zone and listed here.");
		}

		ImGui::SameLine();
		if (ImGui::Button("Assets"))
		{
			st.show_assets = true;
			ImGui::SetWindowFocus("Assets##window");
		}

		static std::vector<int> filtered;
		static std::string filtered_for = "\x01";
		static std::size_t filtered_count = 0;
		static const void* filtered_list = nullptr;

		if (filtered_for != model_filter || filtered_count != names.size() || filtered_list != names.data())
		{
			filtered_for = model_filter;
			filtered_count = names.size();
			filtered_list = names.data();
			filtered.clear();

			const auto needle = codol::script::to_lower(model_filter);
			for (std::size_t i = 0; i < names.size(); i++)
			{
				if (needle.empty() || names[i].find(needle) != std::string::npos)
				{
					filtered.push_back(static_cast<int>(i));
				}
			}
		}

		if (ImGui::BeginChild("##model_list", ImVec2(0.0f, 0.0f), true))
		{
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(filtered.size()));

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					const auto& name = names[static_cast<std::size_t>(filtered[static_cast<std::size_t>(row)])];

					const bool clicked = ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);

					if (clicked && !ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						// a look at it before it is placed
						editor::preview::show_model(name);
					}

					if (clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						// on what the camera looks at, else a few steps ahead; facing the camera
						const auto target = edit::snap_point(s.ray_target(s.cam.origin, s.cam.axes().forward), s.snap ? s.grid : 0.0);
						const double yaw = std::fmod(std::floor(s.cam.yaw / 15.0f + 0.5f) * 15.0 + 180.0, 360.0);

						if (!s.create_model_entity(name, target, yaw))
						{
							ImGui::Toast(ImGuiToastType_Error, "Models", s.doc.last_error().c_str());
						}
					}
				}
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}

	// ---- the view --------------------------------------------------------------------------------------------

	bool codol_viewport_dialog::gui()
	{
		auto& s = editor::get_scene();
		auto& doc = s.doc;
		auto& st = ui::get();
		auto& cam = s.cam;

		ImGuizmo::BeginFrame();

		const auto side_windows = [&]
		{
			m->draw_selection_window();
			m->draw_models_window();
			editor::textures::draw_materials_window(m->material_filter, m->material_group);
		};

		ImGui::SetNextWindowSizeConstraints(ImVec2(480.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(ImVec2(1100.0f, 700.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("CoDOL Viewport##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImGui::End();
			side_windows();
			return false;
		}

		const bool loaded = s.renderer.loaded();

		// ---- toolbar
		const auto tool = [&](const char* label, const ui::gizmo_tool which, const char* tip)
		{
			const bool on = st.gizmo == which;
			if (on)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			}

			if (ImGui::Button(label))
			{
				st.gizmo = which;
			}

			if (on)
			{
				ImGui::PopStyleColor();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", tip);
			}

			ImGui::SameLine();
		};

		tool("Move", ui::gizmo_tool::move, "Move the selection ( W )");
		tool("Rotate", ui::gizmo_tool::rotate, "Rotate the selection about its centre ( E )");
		tool("Scale", ui::gizmo_tool::scale, "Scale the selection about its centre ( R )");

		ImGui::Checkbox("World axes", &st.gizmo_world);
		ImGui::SameLine();
		ImGui::Checkbox("Snap", &s.snap);

		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Move in steps of the grid, rotate in steps of %.0f degrees.\nHold Ctrl while dragging for the opposite.", s.snap_angle);
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(84.0f);

		{
			char grid_label[32];
			std::snprintf(grid_label, sizeof(grid_label), "grid %g", s.grid);

			if (ImGui::BeginCombo("##vp_grid", grid_label))
			{
				for (int g = 1; g <= 512; g *= 2)
				{
					if (ImGui::Selectable(std::to_string(g).c_str(), s.grid == g))
					{
						s.set_grid(g);
					}
				}

				ImGui::EndCombo();
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Grid size ( [ halves, ] doubles )");
			}
		}

		ImGui::SameLine(0.0f, 16.0f);
		ImGui::SetNextItemWidth(150.0f);

		{
			int mode = static_cast<int>(s.show_brushes);
			if (ImGui::Combo("##vp_brushes", &mode, "Brushes: off\0Brushes: selected\0Brushes: all\0"))
			{
				s.show_brushes = static_cast<editor::brush_display>(mode);
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("The collision brushes of the zone, as wires. They are not what the game draws.\nNew and edited brushes are always drawn, solid.");
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Show"))
		{
			ImGui::OpenPopup("##vp_show");
		}

		if (ImGui::BeginPopup("##vp_show"))
		{
			ImGui::Checkbox("World", &m->world);
			ImGui::Checkbox("Static models", &m->static_models);
			ImGui::Checkbox("Entity models", &m->entity_models);
			ImGui::Checkbox("Brush models", &m->brush_models);
			ImGui::Checkbox("New models", &m->placed_models);
			ImGui::Checkbox("New and edited brushes", &m->meshes);
			ImGui::Separator();

			bool rebuild = false;
			rebuild |= ImGui::Checkbox("Entity boxes", &s.show_entity_boxes);
			ImGui::Checkbox("Entity names", &s.show_labels);
			rebuild |= ImGui::Checkbox("Target lines", &s.show_connections);
			ImGui::SetNextItemWidth(160.0f);
			ImGui::SliderFloat("Brush wire distance", &m->wire_distance, 500.0f, 40000.0f, "%.0f", ImGuiSliderFlags_Logarithmic);

			if (rebuild)
			{
				m->built_selected = ~0ull;
			}

			ImGui::Separator();
			ImGui::Checkbox("Sun shadows", &m->sun_shadows);
			ImGui::Checkbox("Model LODs", &m->model_lods);
			ImGui::Checkbox("Wireframe", &m->wireframe);
			ImGui::Checkbox("Selection outline", &m->show_outline);
			ImGui::Separator();

			if (editor::menu::item("Show everything hidden", editor::menu::key("view.show_all"), false, s.open))
			{
				ui::show_all();
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Camera"))
		{
			ImGui::OpenPopup("##vp_camera");
		}

		if (ImGui::BeginPopup("##vp_camera"))
		{
			ImGui::SetNextItemWidth(220.0f);
			ImGui::SliderFloat("Fly speed", &m->fly_speed, 50.0f, 20000.0f, "%.0f units/s", ImGuiSliderFlags_Logarithmic);
			ImGui::SetNextItemWidth(220.0f);
			ImGui::SliderFloat("Field of view", &m->fov, 40.0f, 120.0f, "%.0f deg");
			ImGui::SetNextItemWidth(220.0f);
			ImGui::DragFloat("Angle snap", &s.snap_angle, 1.0f, 1.0f, 90.0f, "%.0f deg");
			ImGui::Separator();

			if (editor::menu::item("Next spawn point", editor::menu::key("camera.next_spawn"), false, loaded)) { m->next_spawn = true; }
			if (editor::menu::item("Whole map from above", editor::menu::key("camera.overview"), false, loaded)) { m->overview = true; }
			if (editor::menu::item("Go to the selection", editor::menu::key("camera.focus"), false, loaded && (!doc.selection().empty() || s.inspecting))) { m->focus = true; }
			ImGui::EndPopup();
		}

		// ---- second row: the editing tools of this view ( edit3d.cpp )
		editor::edit3d::toolbar();

		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const float status_h = ImGui::GetTextLineHeightWithSpacing();
		const int width = static_cast<int>(avail.x);
		const int height = static_cast<int>(avail.y - status_h);

		if (!loaded || width < 16 || height < 16)
		{
			ImGui::Dummy(ImVec2(0.0f, 16.0f));
			ImGui::Indent(16.0f);
			ImGui::PushTextWrapPos(0.0f);

			if (s.open)
			{
				ImGui::TextDisabled("This map is not tied to a zone of the game ( no \"codol_map\" key ): there is nothing to draw here. Use a 2D view.");
			}
			else
			{
				ImGui::TextDisabled("Open a map: double click one in the Maps window, or File > Open map.");
			}

			ImGui::PopTextWrapPos();
			ImGui::Unindent(16.0f);
			ImGui::End();

			side_windows();
			return true;
		}

		m->view_width = width;
		m->view_height = height;

		// ---- camera requests
		editor::prefs::sync_camera(m->fov, m->fly_speed, m->wire_distance);
		cam.fov_x = m->fov;

		if (m->next_spawn)
		{
			m->next_spawn = false;

			if (s.stage)
			{
				// the renderer only knows the stage: the spawn points of the document
				if (!s.document_spawn_camera(++s.spawn_index))
				{
					s.spawn_index = 0;
					s.document_spawn_camera(0);
				}
			}
			else if (!s.renderer.spawn_camera(++s.spawn_index, cam))
			{
				s.spawn_index = 0;
				s.renderer.spawn_camera(0, cam);
			}
		}

		if (m->overview)
		{
			m->overview = false;

			float3 center, half;
			edit::bounds box;
			bool have = false;

			if (s.stage)
			{
				if (s.document_bounds(box))
				{
					center = { static_cast<float>((box.mins.x + box.maxs.x) * 0.5), static_cast<float>((box.mins.y + box.maxs.y) * 0.5), static_cast<float>((box.mins.z + box.maxs.z) * 0.5) };
					half = { static_cast<float>((box.maxs.x - box.mins.x) * 0.5), static_cast<float>((box.maxs.y - box.mins.y) * 0.5), static_cast<float>((box.maxs.z - box.mins.z) * 0.5) };
					have = true;
				}
			}
			else
			{
				have = s.renderer.world_bounds(center, half);
			}

			if (have)
			{
				const float reach = std::max(half.x, half.y) / std::tan(m->fov * 0.5f * editor::DEG);
				cam.origin = { center.x, center.y, center.z + half.z + reach };
				cam.pitch = 89.0f;
				cam.yaw = 90.0f;
				cam.roll = 0.0f;
			}
		}

		if (m->focus)
		{
			m->focus = false;
			s.look_at_selection();
		}

		// ---- line buffers follow the caches of the scene
		const auto lines_start = std::chrono::steady_clock::now();

		if (m->built_statics != s.statics_serial || m->built_mode != s.show_brushes)
		{
			m->built_statics = s.statics_serial;
			m->built_mode = s.show_brushes;
			m->rebuild_static_lines();
		}

		if (m->built_selected != s.selected_serial || m->built_inspect != s.inspecting || (s.inspecting && m->built_inspect_id != s.inspected.object.id))
		{
			m->built_selected = s.selected_serial;
			m->built_inspect = s.inspecting;
			m->built_inspect_id = s.inspected.object.id;
			m->rebuild_selection_lines();
		}

		s.slowest.view_lines = std::max(s.slowest.view_lines, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - lines_start).count());

		// ---- draw
		view::frame_options frame;
		frame.game_time = static_cast<float>(ImGui::GetTime());
		frame.draw_world = m->world;
		frame.draw_static_models = m->static_models;
		frame.draw_entity_models = m->entity_models;
		frame.draw_brush_models = m->brush_models;
		frame.draw_placed_models = m->placed_models;
		frame.draw_meshes = m->meshes;
		frame.sun_shadows = m->sun_shadows;
		frame.model_lods = m->model_lods;
		frame.wireframe = m->wireframe;
		frame.clear_color = 0xFF202020;
		frame.overlay = [this](const view::overlay_context& ctx) { m->draw_overlay(ctx); };

		// the render modes of the Renderer menu ( they own wireframe, sun shadows and the outline of the selection )
		{
			auto& modes = editor::render_modes::get();
			static bool seen_wire = m->wireframe, seen_shadows = m->sun_shadows, seen_outline = m->show_outline;
			if (m->wireframe != seen_wire) { modes.wireframe = m->wireframe; }
			if (m->sun_shadows != seen_shadows) { modes.sun_shadows = m->sun_shadows; }
			if (m->show_outline != seen_outline) { modes.outline_selected = m->show_outline; }
			seen_wire = m->wireframe = modes.wireframe;
			seen_shadows = m->sun_shadows = modes.sun_shadows;
			seen_outline = m->show_outline = modes.outline_selected;
			editor::render_modes::apply(frame);
		}

		IDirect3DTexture9* texture = s.renderer.render(cam, width, height, frame);
		texture = editor::render_modes::post_process(texture, width, height);
		const ImVec2 image_pos = ImGui::GetCursorScreenPos();
		auto* dl = ImGui::GetWindowDrawList();

		if (texture)
		{
			// the alpha channel of the frame is whatever the game's shaders left there: draw it opaque
			dl->AddCallback([](const ImDrawList*, const ImDrawCmd*)
			{
				if (game::glob::d3d9_device)
				{
					game::glob::d3d9_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				}
			}, nullptr);

			ImGui::Image(texture, ImVec2(static_cast<float>(width), static_cast<float>(height)));
			dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
		}
		else
		{
			ImGui::Dummy(ImVec2(static_cast<float>(width), static_cast<float>(height)));
			dl->AddText(ImVec2(image_pos.x + 16.0f, image_pos.y + 16.0f), IM_COL32(245, 115, 100, 255), ("render failed: " + s.renderer.last_error()).c_str());
		}

		dl->PushClipRect(image_pos, ImVec2(image_pos.x + static_cast<float>(width), image_pos.y + static_cast<float>(height)), true);
		editor::fx::draw_in_view(dl, image_pos, width, height);
		m->draw_labels(dl, image_pos, width, height);
		dl->PopClipRect();

		// a tool that paints takes the left button before the view does ( tools.cpp )
		editor::map_tools::viewport_input(image_pos, width, height);

		// ---- input area
		// ImGuizmo only takes a click while no item is hovered or active. So while the mouse is on the gizmo the
		// view is not a button ( which would take the click and drive the camera ) but a plain rectangle.
		ImGui::SetCursorScreenPos(image_pos);

		if (m->gizmo_hover || m->gizmo_active)
		{
			ImGui::Dummy(ImVec2(static_cast<float>(width), static_cast<float>(height)));
		}
		else
		{
			ImGui::InvisibleButton("##vp_nav", ImVec2(static_cast<float>(width), static_cast<float>(height)),
				ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
		}

		const bool hovered = ImGui::IsItemHovered();
		const bool active = ImGui::IsItemActive();
		const auto& io = ImGui::GetIO();

		if (hovered)
		{
			st.next_hovered_3d = true;
		}

		// a material dragged out of the Materials window: on the face it is dropped on ( Shift: the whole brush )
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CODOL_MATERIAL"); payload && payload->Data)
			{
				editor::edit3d::drop_material(static_cast<const char*>(payload->Data), io.MousePos.x - image_pos.x, io.MousePos.y - image_pos.y, io.KeyShift);
			}

			ImGui::EndDragDropTarget();
		}

		// ---- gizmo on the selection of the document: it wins over driving the camera
		const bool gizmo_busy = m->draw_gizmo(dl, image_pos, width, height);
		m->gizmo_hover = gizmo_busy;

		// ---- the editing tools ( edit3d.cpp ): they get the left button first. A tool that takes the press owns it
		// until the release; one that does not leaves the click and the drag to the view, as before.
		bool tool_press = false;

		{
			const float mx = io.MousePos.x - image_pos.x, my = io.MousePos.y - image_pos.y;

			if (editor::edit3d::scripted())
			{
				// a press from the console ( ed3d press ): it ends with `ed3d release`
			}
			else if (editor::edit3d::busy())
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{
					editor::edit3d::drag(mx, my, false);
				}
				else
				{
					editor::edit3d::release();
				}
			}
			else if (hovered && !gizmo_busy && s.open && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				tool_press = editor::edit3d::press(mx, my, io.KeyShift, io.KeyCtrl, io.KeyAlt);
			}

			dl->PushClipRect(image_pos, ImVec2(image_pos.x + static_cast<float>(width), image_pos.y + static_cast<float>(height)), true);
			editor::edit3d::draw(dl, image_pos, width, height, hovered && !gizmo_busy);
			dl->PopClipRect();
		}

		const bool tool_busy = tool_press || editor::edit3d::busy();

		// ---- navigation and picking
		{
			const auto axes = cam.axes();
			const float boost = io.KeyShift ? 4.0f : 1.0f;

			const auto move = [&](const float3& dir, const float amount)
			{
				cam.origin.x += dir.x * amount;
				cam.origin.y += dir.y * amount;
				cam.origin.z += dir.z * amount;
			};

			const bool lmb = active && ImGui::IsMouseDown(ImGuiMouseButton_Left);
			const bool rmb = active && ImGui::IsMouseDown(ImGuiMouseButton_Right);
			const bool mmb = active && ImGui::IsMouseDown(ImGuiMouseButton_Middle);
			const float drag = m->fly_speed * boost / 200.0f;

			if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				m->press_pos = io.MousePos;
				m->press_valid = !gizmo_busy && !tool_busy;
				m->press_dragged = false;
			}

			if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				m->right_press_pos = io.MousePos;
				m->right_press_valid = true;
			}

			const bool dragged = m->press_dragged || std::fabs(io.MousePos.x - m->press_pos.x) + std::fabs(io.MousePos.y - m->press_pos.y) > 4.0f;
			if (m->press_valid && dragged) { m->press_dragged = true; }
			const bool right_dragged = std::fabs(io.MousePos.x - m->right_press_pos.x) + std::fabs(io.MousePos.y - m->right_press_pos.y) > 4.0f;

			if (m->right_press_valid && right_dragged)
			{
				m->right_press_valid = false; // looking around, not a click
			}

			// A camera drag holds the mouse: hidden, kept in the view and put back to its middle every frame ( the motion
			// never runs out at an edge ), back where the drag started when it ends ( mouse_capture.hpp ).
			const bool steering = !tool_busy && ((mmb || (lmb && rmb)) || (rmb && !m->right_press_valid) || (lmb && m->press_valid && dragged && !m->gizmo_active));
			const ImVec2 motion = editor::mouse_capture::update(m.get(), steering, image_pos, ImVec2(image_pos.x + static_cast<float>(width), image_pos.y + static_cast<float>(height)));
			const float dx = motion.x, dy = motion.y;

			if (tool_busy)
			{
				// a tool has the left button: the camera stays where it is
			}
			else if (mmb || (lmb && rmb))
			{
				move(axes.left, dx * drag);
				move(axes.up, dy * drag);
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
			}
			else if (rmb && !m->right_press_valid)
			{
				cam.yaw -= dx * 0.15f;
				cam.pitch = std::clamp(cam.pitch + dy * 0.15f * editor::prefs::look_sign_y(), -89.0f, 89.0f);

				const float step = m->fly_speed * boost * io.DeltaTime;

				if (ImGui::IsKeyDown(ImGuiKey_W)) { move(axes.forward, step); }
				if (ImGui::IsKeyDown(ImGuiKey_S)) { move(axes.forward, -step); }
				if (ImGui::IsKeyDown(ImGuiKey_A)) { move(axes.left, step); }
				if (ImGui::IsKeyDown(ImGuiKey_D)) { move(axes.left, -step); }
				if (ImGui::IsKeyDown(ImGuiKey_E) || ImGui::IsKeyDown(ImGuiKey_Space)) { move({ 0.0f, 0.0f, 1.0f }, step); }
				if (ImGui::IsKeyDown(ImGuiKey_Q) || ImGui::IsKeyDown(ImGuiKey_C)) { move({ 0.0f, 0.0f, 1.0f }, -step); }

				if (io.MouseWheel != 0.0f)
				{
					m->fly_speed = std::clamp(m->fly_speed * std::pow(1.25f, io.MouseWheel), 50.0f, 20000.0f);
				}

				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
			}
			else if (lmb && m->press_valid && dragged && !m->gizmo_active)
			{
				// drive: up / down moves along the ground, left / right turns
				const float yaw = cam.yaw * editor::DEG;
				move({ std::cos(yaw), std::sin(yaw), 0.0f }, -dy * drag);
				cam.yaw -= dx * 0.15f;
			}
			else if (hovered && !rmb)
			{
				if (io.MouseWheel != 0.0f)
				{
					move(axes.forward, io.MouseWheel * m->fly_speed * boost * 0.25f);
				}

				// the keys of this view ( N, Home, F, W E R ) are actions: hotkeys.cpp, camera.* and gizmo.*
			}

			// a click ( press and release without dragging ) selects
			if (m->press_valid && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				m->press_valid = false;

				if (!dragged && !gizmo_busy && !s.dragging())
				{
					// Ctrl or Shift: add / remove. Both: the face under the mouse ( texture tools ).
					const bool face = io.KeyCtrl && io.KeyShift;
					ui::viewport_click(io.MousePos.x - image_pos.x, io.MousePos.y - image_pos.y, !face && (io.KeyCtrl || io.KeyShift), face);
				}
			}

			// a right click ( without looking around ): create an entity where the mouse points
			if (m->right_press_valid && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				m->right_press_valid = false;

				if (hovered && s.open)
				{
					float3 origin, direction;
					view::screen_ray(cam, width, height, io.MousePos.x - image_pos.x, io.MousePos.y - image_pos.y, origin, direction);
					st.create_position = edit::snap_point(s.ray_target(origin, direction), s.snap ? s.grid : 0.0);
					st.open_create_popup = true;
				}
			}
		}

		// ---- the legend of a render mode ( case textures )
		editor::render_modes::draw_view_legend(dl, image_pos, width, height);

		// ---- hint
		if (hovered && !active && !gizmo_busy && !tool_busy)
		{
			// what the mouse and the keys do in the tool that is on
			const char* hint = editor::edit3d::hint();
			const float wrap = std::max(120.0f, static_cast<float>(width) - 24.0f);	// a narrow view: the lines wrap
			const ImVec2 size = ImGui::CalcTextSize(hint, nullptr, false, wrap);
			const ImVec2 at(image_pos.x + 8.0f, image_pos.y + static_cast<float>(height) - size.y - 10.0f);

			dl->AddRectFilled(ImVec2(at.x - 4.0f, at.y - 3.0f), ImVec2(at.x + size.x + 4.0f, at.y + size.y + 3.0f), IM_COL32(0, 0, 0, 150), 3.0f);
			dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), at, IM_COL32(230, 230, 230, 255), hint, nullptr, wrap);
		}

		// ---- status
		const auto stats = s.renderer.stats();
		ImGui::TextDisabled("%s   %d x %d   %d draws   render %.1f ms   frame %.1f ms   pos %.0f %.0f %.0f   pitch %.0f yaw %.0f   grid %g   %d selected   undo %d",
			s.map_name.c_str(), width, height, stats.draw_calls, stats.cpu_ms, st.frame_ms, cam.origin.x, cam.origin.y, cam.origin.z, cam.pitch, cam.yaw, s.grid,
			static_cast<int>(doc.selection().size()), static_cast<int>(doc.undo_count()));

		ImGui::End();

		side_windows();
		return true;
	}

	REGISTER_GUI(codol_viewport_dialog);
}
