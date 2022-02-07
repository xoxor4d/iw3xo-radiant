#pragma once

namespace ggui
{
	enum E_FONT
	{
		BOLD_18PX = 0,
		REGULAR_12PX = 1,
		REGULAR_18PX = 2,
	};
	
	enum E_CALLTYPE
	{
		MAINFRAME_CDECL = 0,
		MAINFRAME_THIS = 1,
		MAINFRAME_STD = 2,
		CDECLCALL = 3,
	};
	
	#define mainframe_thiscall(return_val, addr)	\
		utils::hook::call<return_val(__fastcall)(cmainframe*)>(addr)(cmainframe::activewnd)
	
	#define mainframe_cdeclcall(return_val, addr)	\
		utils::hook::call<return_val(__cdecl)(cmainframe*)>(addr)(cmainframe::activewnd)
	
	#define mainframe_stdcall(return_val, addr)	\
		utils::hook::call<return_val(__stdcall)(cmainframe*)>(addr)(cmainframe::activewnd)
	
	#define cdeclcall(return_val, addr)	\
		utils::hook::call<return_val(__cdecl)()>(addr)()


	// left label menu widget, eg. "dragfloat"
	#define IMGUI_MENU_WIDGET_SINGLE(label, func)                                                       \
		ImGui::Text(label); ImGui::SameLine();                                                          \
		const ImGuiMenuColumns* offsets = &ImGui::GetCurrentWindow()->DC.MenuColumns;                   \
		ImGui::SetCursorPosX(static_cast<float>(offsets->OffsetShortcut + 5));                          \
		ImGui::PushItemWidth(static_cast<float>(offsets->Widths[2] + offsets->Widths[3] + 5));          \
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));   \
		func; ImGui::PopItemWidth(); ImGui::PopStyleVar();
	
	
	// show tooltip after x seconds
	#define TTDELAY 0.5f 
	
	// tooltip with delay
	#define TT(tooltip) if (ImGui::IsItemHoveredDelay(TTDELAY)) ImGui::SetTooltip(tooltip)
	
	// spacing dummy
	#define SPACING(x, y) ImGui::Dummy(ImVec2(x, y)) 
	
	// seperator with spacing
	#define SEPERATORV(spacing) SPACING(0.0f, spacing); ImGui::Separator(); SPACING(0.0f, spacing) 

	
	// *
	// | -------------------- Structs ------------------------
	// *

    enum e_gfxwindow
	{
		CCAMERAWND = 0,
		CXYWND = 1,
    	CZWND = 2,
    	CTEXWND = 3,
    	LAYERED = 4,
	};

	struct imgui_context_menu
	{
		bool menustate;
		bool was_open;
		bool one_time_init;
		bool inactive_tab;
		bool bring_tab_to_front;
		float position[2];
		float size[2];
	};

	struct render_to_texture_window_s
	{
		IDirect3DTexture9* scene_texture;
		ImVec2 scene_size_imgui;
		ImVec2 scene_pos_imgui;
		ImVec2 cursor_pos;
		CPoint cursor_pos_pt;
		const char* window_name;
		bool window_hovered;
		bool should_set_focus;
		bool capture_left_mousebutton;
		bool menustate;
		bool inactive_tab;
		bool bring_tab_to_front;
		bool was_open;
		bool one_time_init;
	};

	struct model_selector_s
	{
		IDirect3DTexture9* scene_texture;
		
		float camera_fov;
		float camera_distance;
		float camera_angles[3];
		float camera_offset[3];
		CPoint m_ptCursor;
		bool user_rotation;
		bool overwrite_selection;

		// axis model
		bool axis_model_initiated;
		int axis_model_inst_handle;
		
		// model preview
		std::string preview_model_name;
		game::XModel* preview_model_ptr;
		int preview_model_inst_handle;
		bool bad_model;

		// raw/xmodel folder
		int  xmodel_filecount;
		const char** xmodel_filelist = nullptr;
		int xmodel_selection = -1;
		
		ImVec2 scene_size_imgui;
		CPoint cursor_pos_pt;

		bool scene_hovered;
		bool window_hovered;
		bool menustate;
		bool inactive_tab;
		bool bring_tab_to_front;
		bool was_open;
		bool one_time_init;
	};
	
	struct imgui_context_cz
	{
		bool context_initialized;
		ImGuiContext* context;
		game::GfxWindowTarget* dx_window;
		imgui_context_menu m_toolbar;
		imgui_context_menu m_toolbar_edit;
		imgui_context_menu m_colors;
		imgui_context_menu m_cmdbinds;
		imgui_context_menu m_cmdbinds_helper;
		imgui_context_menu m_console;
		imgui_context_menu m_filter;
		imgui_context_menu m_entity;
		imgui_context_menu m_surface_inspector;
		imgui_context_menu m_preferences;
		imgui_context_menu m_about;
		imgui_context_menu m_camera_settings;
		imgui_context_menu m_demo;
		render_to_texture_window_s rtt_camerawnd;
		render_to_texture_window_s rtt_gridwnd;
		render_to_texture_window_s rtt_texwnd;
		model_selector_s rtt_model_selector;
	};

	struct imgui_state_t
	{
		imgui_context_cz czwnd;
	};

	struct commandbinds
	{
		std::string cmd_name;
		std::string keys;
		int modifier_alt;
		int modifier_ctrl;
		int modifier_shift;
		std::string modifier_key;
	};

	// *
	// | -------------------- Variables ------------------------
	// *

    extern imgui_state_t state;
	extern bool			saved_states_init;
	
	extern ImGuiID		dockspace_outer_left_node;
	extern bool			reset_dockspace;
	extern bool			mainframe_menubar_enabled;
	extern float		menubar_height;
	
	extern ImVec2		toolbar_pos;
	extern ImVec2		toolbar_size;
	extern ImGuiAxis	toolbar_axis;
	extern bool			toolbar_reset;
	extern ImGuiID		toolbar_dock_top;
	extern ImGuiID		toolbar_dock_left;

	extern std::vector<commandbinds> cmd_hotkeys;
	extern std::vector<game::SCommandInfo> cmd_addon_hotkeys_builtin;
	extern std::vector<game::SCommandInfoHotkey> cmd_addon_hotkeys; //std::vector<game::SCommandInfo> cmd_addon_hotkeys;

	//extern model_selector_s rtt_model_preview;
	
	// *
	// | -------------------- Functions ------------------------
	// *

	extern ImVec2 get_initial_window_pos();
	extern void set_next_window_initial_pos_and_constraints(ImVec2 mins, ImVec2 initial_size, ImVec2 overwrite_pos = ImVec2(0.0f, 0.0f));
	extern bool cz_context_ready();

	extern bool rtt_handle_windowfocus_overlaywidget(ggui::render_to_texture_window_s* wnd);
	extern void	FixDockingTabbarTriangle(ImGuiWindow* wnd, ggui::render_to_texture_window_s* rtt);

	extern void dragdrop_overwrite_leftmouse_capture();
	extern void dragdrop_reset_leftmouse_capture();
	
	inline render_to_texture_window_s* get_rtt_gridwnd()
	{
		return &state.czwnd.rtt_gridwnd;
	}
	
	inline render_to_texture_window_s* get_rtt_camerawnd()
	{
		return &state.czwnd.rtt_camerawnd;
	}

	inline render_to_texture_window_s* get_rtt_texturewnd()
	{
		return &state.czwnd.rtt_texwnd;
	}

	inline model_selector_s* get_rtt_modelselector()
	{
		return &state.czwnd.rtt_model_selector;
	}
}
