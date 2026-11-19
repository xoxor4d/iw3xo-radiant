#pragma once

// What the windows of the editor share: tool state, the editing actions ( menu, shortcuts and console commands
// all end up in the same functions ), file handling with the "unsaved changes" question.

#include "scene.hpp"

namespace editor::ui
{
	enum class gizmo_tool : int
	{
		move,
		rotate,
		scale,
	};

	// the clipper of the 2D views: two or three points; with two, the plane runs along the view axis
	struct clipper_state
	{
		bool on = false;
		std::vector<edit::vec3> points;
		int axis = 2;				// view axis the points were placed in
		int dragging = -1;
	};

	struct state
	{
		gizmo_tool gizmo = gizmo_tool::move;
		bool gizmo_world = true;

		bool show_selection = true;
		bool show_entity = true;
		bool show_layers = false;
		bool show_models = false;
		bool show_materials = false;
		bool show_surface = false;
		bool show_assets = false;
		bool show_preview = false;

		// which view the mouse was over in the last frame: decides what view relative shortcuts mean
		int hovered_2d = -1;		// index of the 2D view, -1: none
		bool hovered_3d = false;
		int next_hovered_2d = -1;
		bool next_hovered_3d = false;

		clipper_state clipper;
		bool vertex_mode = false;

		// "Create entity" popup: opened by a right click in a view
		bool open_create_popup = false;
		edit::vec3 create_position;
		std::string create_filter;

		// frame time of the editor's own work ( everything but Present ), milliseconds
		double frame_ms = 0.0;
		double frame_ms_max = 0.0;
	};

	state& get();

	// ---- actions ---------------------------------------------------------------------------------------------
	// Each does one thing to the document and tells the user when there was nothing to do.
	void undo();
	void redo();
	void delete_selection();
	void clone_selection();
	void hide_selected();
	void show_all();
	void select_all();
	void invert_selection();
	void deselect();
	void select_whole_entities();
	void snap_to_grid();
	void csg_subtract();
	void csg_hollow();
	void set_detail(bool detail);
	void toggle_texture_lock();
	void connect_selected();
	void move_to_worldspawn();
	void grid_smaller();
	void grid_larger();
	// one grid step along a world axis
	void nudge(int axis, int sign);
	// one grid step, relative to the view the mouse is over: dx right, dy up ( on screen ), dz towards / away
	void nudge_view(int dx, int dy);

	// clipper: keep 0 front, 1 back, 2 both ( split )
	bool apply_clip(int keep);
	void set_clipper(bool on);
	void set_vertex_mode(bool on);

	// toast + console line; returns ok
	bool report(bool ok, const char* action);

	// ---- files -----------------------------------------------------------------------------------------------
	// These ask before unsaved changes are thrown away. `force`: do not ask ( console commands ).
	void open_zone(const std::string& map, bool force = false);
	void open_map_file(const std::filesystem::path& file, bool force = false);
	void open_map_dialog();
	bool save();
	void save_as_dialog();
	void export_entities(bool reload);
	void compile_map();
	void new_map(const std::string& name, bool force = false);
	void build_map(bool fast);
	void add_gameplay_entities();
	// install.cpp: the package of the last build to where the game reads it, and out again
	struct install_request
	{
		std::string game;			// the game folder: usermaps\<map>\, main\, and the create-room list
		std::string zone_dir;		// a zone folder by hand instead of <game>\usermaps\<map>
		std::string script_dir;		// the server's script folder; empty: no scripts
		bool list = false;			// add the map to the create-room list of `game`
	};
	void install_map(const install_request& request);
	void install_map(const std::string& zone_to, const std::string& scripts_to);	// empty: the folders of File > Settings
	void uninstall_map(const std::string& map);										// empty: the open map
	void print_map_list(const std::string& game_folder);							// empty: the install of File > Settings
	void open_install_window();
	void draw_install_window();
	void close_map(bool force = false);
	void exit_editor();

	// ---- per frame -------------------------------------------------------------------------------------------
	void process_shortcuts();
	void draw_file_menu_items();
	void draw_edit_menu();
	void draw_modals();
	void draw_create_entity_popup();
	void draw_entity_window();
	void draw_layers_window();
	void draw_surface_window();

	// ---- light ( lights.cpp ) ----------------------------------------------------------------------------------
	// a `light` entity as the map compiler reads it ( codol_bsp map_input.cpp )
	struct light_info
	{
		float3 origin{};
		float radius = 600.0f;
		float color[3] = { 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;
		int flags = 0;					// spawnflags
		bool scriptable = false;		// spawnflags 4: a primary light a script can change; it stays an entity
		bool primary_omni = false;
		bool primary_spot = false;
		bool spot = false;				// has a cone: a primary spot, or a baked light with fov_outer or a target
		float outer = 0.0f;				// whole angles of the cone, degrees
		float inner = 0.0f;
		int exponent = 0;
		float3 forward{ 0.0f, 0.0f, -1.0f };
		bool has_target = false;		// the key is there ..
		bool target_found = false;		// .. and names an entity
		float3 target{};
		bool can_shadow = true;
	};

	bool is_light_class(std::string_view classname);		// light, light_spot, light_omni
	light_info read_light(const edit::document& doc, const edit::entity& e);
	// lines ( two vertices each ) that show where a light reaches: its radius, the cone of a spot
	void light_shape(const light_info& light, std::vector<line_vertex>& out);
	// the shapes of the selected light entities, world space, colours as Direct3D has them
	const std::vector<line_vertex>& selected_light_lines();
	// the typed fields of the Entity window for `light` entities / the light keys of the worldspawn
	void draw_light_panel(const std::vector<object_id>& entities, bool world);
	// once per frame: closes the undo step of a field that was dragged
	void finish_live_edits();

	// one row of filter + list of the classes of the catalog; returns the picked class
	const edit::class_info* class_picker(std::string& filter, float height, bool brush_classes_enabled);

	void register_commands();

	// Development aid ( like CODOL_STARTUP_CMDS ): the console commands ui_mouse / ui_key / ui_text feed mouse and
	// keyboard input to the gui, so that unattended runs go through the same code as a hand on the mouse. Called
	// between the platform backend's new frame and ImGui::NewFrame().
	void apply_injected_input();

	// whether ui_mouse fed the mouse ( a position or a button ) since the last "ui_mouse off": the views then never
	// capture the real cursor
	bool mouse_injected();

	// the 2D views ( grid_view.cpp )
	void grid_view_command(const std::vector<std::string>& args);
	void draw_grid_views();
	void add_grid_view();
	void cycle_grid_axis();
	void draw_grid_view_menu_items();

	// view axis ( 0 x, 1 y, 2 z ) of a 2D view
	int grid_view_axis(int index);

	// the 3D view ( viewport.cpp ): a click along a ray / at a pixel of the view, as the mouse does it. The nearest of
	// what the renderer draws and what the document shows ( brushes, entity boxes ) is selected; `toggle` adds /
	// removes ( Ctrl or Shift click ), `face` selects the brush face ( Shift + Ctrl click ).
	void pick_ray(const float3& origin, const float3& direction, bool toggle, bool face);
	void viewport_click(float px, float py, bool toggle, bool face);
	void set_material_filter(const std::string& text, int group);
	// "next_spawn", "overview", "focus": the camera actions of the 3D view
	void viewport_request(std::string_view what);

	// ---- the Brush and Patch menus ( menu_brush_patch.cpp ) -----------------------------------------------------
	void draw_brush_menu();
	void draw_patch_menu();
	void draw_brush_patch_windows();			// their dialogs, once per frame
	void register_brush_patch_commands();		// brush_*, patch_*
	bool patches_as_wire();						// patches are drawn as lines only
	// vertex mode of the 2D views: the vertices of a patch that were grabbed become the selected patch vertices
	// ( without "drill down" only the one nearest to the viewer stays in the list ); a drag honours weld and lock
	void patch_vertices_grabbed(object_id patch, std::vector<int>& vertices, int view_axis);
	bool move_patch_vertices(object_id patch, const std::vector<int>& vertices, const edit::vec3& delta);
	// The patch vertex selection - the vertices of one selected patch that the Patch menu works on - is one for every
	// view: the 2D views' vertex mode, the 3D view's vertex tool, the Advanced Edit dialog.
	object_id patch_vertex_selection(std::vector<int>* out = nullptr);
	[[nodiscard]] bool patch_vertex_selected(object_id patch, int index);
	void select_patch_vertices(object_id patch, const std::vector<int>& list, bool add);
	void deselect_patch_vertices(object_id patch, const std::vector<int>& list);
	void clear_patch_vertices();
	[[nodiscard]] bool patch_vertex_locked(object_id patch, int index);
	[[nodiscard]] bool patch_drill_down();
	// the locked vertices of a patch by place, and locked again by place after an operation that renumbers the vertices
	std::vector<edit::vec3> locked_positions(object_id patch);
	void relock_patch_vertices(object_id patch, const std::vector<edit::vec3>& positions);
	// the rotate / scale gizmo on selected patch vertices ( weld and lock honoured )
	bool transform_patch_vertices(object_id patch, const std::vector<int>& vertices, const edit::transform& t);
	// Weld > Snap To Brush / Model Vertices While Dragging: the point a dragged patch vertex at t lands on
	bool patch_drag_snap(const edit::vec3& at, edit::vec3& out);
	bool weld_patches_to_models();
	// calls mark( position, colour ) for the selected and the locked patch vertices
	void patch_vertex_marks(const std::function<void(const edit::vec3&, ImU32)>& mark);
	// 3D view, terrain edge mode: a click along the ray turns the diagonal of the quad it hits; true when it did
	bool patch_edge_click(const float3& origin, const float3& direction);
	// ---- File / Edit / View additions, the Selection, Grid, Misc and Region menus, filters, prefabs
	// ( menu_selection.cpp, filters.cpp, prefabs.cpp; what they share: menu_ext.hpp ) ---------------------------
	void draw_file_menu_extra();		// after the entries of draw_file_menu_items
	void draw_edit_menu_extra();
	void draw_view_menu_extra();
	void draw_extra_menus();			// Selection, Grid, Misc, Region
	void draw_extra_windows();			// once per frame: Filter, Map Info, Entity Info, Prefabs, the dialogs
	void register_extra_commands();
	// lines in world space every view draws: the contents of prefabs, the region, the leak line ( 2D: the grid blocks )
	const std::vector<line_vertex>& extra_view_lines(bool view2d);
	// the box of a misc_prefab entity relative to its origin; false for any other entity
	bool prefab_box(const edit::entity& e, edit::bounds& local_box);
	// the file File > Build map compiles: the map, or what the region shows ( after "Build region" )
	std::filesystem::path build_source_file(const std::filesystem::path& map_file);
}
