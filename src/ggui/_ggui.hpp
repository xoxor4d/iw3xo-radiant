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
		float position[2];
		float size[2];
	};

	struct imgui_context_cam
	{
		bool context_initialized;
		ImGuiContext* context;
		game::GfxWindowTarget* dx_window;
		imgui_context_menu m_demo;
	};

	struct imgui_context_cxy
	{
		bool context_initialized;
		ImGuiContext* context;
		game::GfxWindowTarget* dx_window;
		imgui_context_menu m_toolbar;
		imgui_context_menu m_toolbar_edit;
		imgui_context_menu m_colors;
		imgui_context_menu m_cmdbinds;
		imgui_context_menu m_cmdbinds_helper;
		imgui_context_menu m_demo;
	};

	struct imgui_context_layer
	{
		bool context_initialized;
		ImGuiContext* context;
		game::GfxWindowTarget* dx_window;
		imgui_context_menu m_toolbar;
		imgui_context_menu m_toolbar_edit;
		imgui_context_menu m_colors;
		imgui_context_menu m_cmdbinds;
		imgui_context_menu m_cmdbinds_helper;
		imgui_context_menu m_demo;
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
		imgui_context_menu m_demo;
	};

	struct imgui_state_t
	{
		imgui_context_cam ccamerawnd;
		imgui_context_cxy cxywnd;
		imgui_context_layer clayeredwnd;
		imgui_context_cz czwnd;
		//bool cxywnd_menubar_state;
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

	struct rtt_camera_window_s
	{
		IDirect3DTexture9* scene_texture;
		ImVec2 scene_size_imgui;
		ImVec2 cursor_pos;
		CPoint cursor_pos_pt;
		bool window_hovered;
	};

	struct rtt_grid_window_s
	{
		IDirect3DTexture9* scene_texture;
		ImVec2 scene_size_imgui;
		ImVec2 cursor_pos;
		CPoint cursor_pos_pt;
		bool window_hovered;
	};

	// *
	// | -------------------- Variables ------------------------
	// *

    extern imgui_state_t state;
	
	extern ImGuiID		dockspace_sidebar_id;
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

	extern rtt_camera_window_s rtt_camerawnd;
	extern rtt_grid_window_s rtt_gridwnd;
	
	
	// *
	// | -------------------- Functions ------------------------
	// *

	extern ImVec2 get_initial_window_pos();

	extern bool camera_context_ready();
	extern bool cxy_context_ready();
	extern bool layered_context_ready();
	extern bool cz_context_ready();
	extern bool all_contexts_ready();
	
}