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

	enum FILE_DIALOG_HANDLER
	{
		MAP_LOAD = 0,
		MAP_SAVE = 1,
		MAP_EXPORT = 2,
		MISC_MODEL = 3,
		MISC_MODEL_CHANGE = 4,
		MISC_PREFAB = 5,
		MISC_PREFAB_CHANGE = 6,
		MISC_PREFAB_CREATE = 7,
		FX_CHANGE = 10,
		FX_EDITOR_DEF = 11,
		FX_EDITOR_MATERIAL = 12,
		FX_EDITOR_MODEL = 13,
		D3DBSP_LOAD = 20,
	};
	
	//#define mainframe_thiscall(return_val, addr)	\
	//	utils::hook::call<return_val(__fastcall)(cmainframe*)>(addr)(cmainframe::activewnd)
	//
	//#define mainframe_cdeclcall(return_val, addr)	\
	//	utils::hook::call<return_val(__cdecl)(cmainframe*)>(addr)(cmainframe::activewnd)
	//
	//#define mainframe_stdcall(return_val, addr)	\
	//	utils::hook::call<return_val(__stdcall)(cmainframe*)>(addr)(cmainframe::activewnd)
	//
	//#define cdeclcall(return_val, addr)	\
	//	utils::hook::call<return_val(__cdecl)()>(addr)()


	// left label menu widget, eg. "dragfloat"
	#define IMGUI_MENU_WIDGET_SINGLE(label, func) {                                                     \
		ImGui::Text(label); ImGui::SameLine();                                                          \
		const ImGuiMenuColumns* offsets = &ImGui::GetCurrentWindow()->DC.MenuColumns;                   \
		ImGui::SetCursorPosX(static_cast<float>(offsets->OffsetShortcut + 5));                          \
		ImGui::PushItemWidth(ImMax(40.0f, static_cast<float>(offsets->Widths[2] + offsets->Widths[3] + 5))); \
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0));   \
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.23f, 0.23f, 0.23f, 0.55f));					\
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.43f, 0.43f, 0.43f, 0.44f));				\
		func; ImGui::PopItemWidth(); ImGui::PopStyleVar(); ImGui::PopStyleColor(2); }
	
	
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

#define GUI_CHECK_RTT	ASSERT_MSG(GUI_TYPE == GUI_TYPE_RTT, "GUI Class not of type RTT")

	enum GUI_TYPE_
	{
		GUI_TYPE_DEF = 0,
		GUI_TYPE_RTT = 1,
	};

	class ggui_module
	{
		struct default_context_s
		{
			bool _active;
			bool _was_active;
			bool _one_time_init;
			bool _inactive_tab;
			bool _bring_tab_to_front;
			float _position[2];
			float _size[2];
		};

		struct render_to_texture_context_s
		{
			bool _active;
			bool _was_active;
			bool _one_time_init;
			bool _inactive_tab;
			bool _bring_tab_to_front;

			ImVec2 _scene_pos_imgui;
			ImVec2 _scene_size_imgui;

			IDirect3DTexture9* _scene_texture;
			ImVec2 _cursor_pos;
			CPoint _cursor_pos_pt;
			const char* _window_name;
			bool _window_hovered;
			bool _should_set_focus;
			bool _capture_left_mousebutton;
		};

		union vars_u
		{
			default_context_s def;
			render_to_texture_context_s rtt;
		};

		vars_u vars = {};
		GUI_TYPE_ GUI_TYPE = GUI_TYPE_DEF;

	public:
		ggui_module() = default;
		virtual ~ggui_module() = default;
		virtual void gui() {}
		virtual void on_open() {}
		virtual void on_close() {}

		void set_gui_type(GUI_TYPE_ type)
		{
			GUI_TYPE = type;
		}

		// called each frame :: components::gui::render_loop()
		void frame()
		{
			if (is_active())
			{
				if (this->is_bring_to_front_pending())
				{
					set_bring_to_front(false);
					ImGui::SetNextWindowFocus();
				}

				if(!was_active())
				{
					on_open();
				}

				gui();
				set_was_active(true);
			}
			else if (was_active())
			{
				on_close();
				set_was_active(false);
			}
		}

		// returns true if gui is open / in use
		[[nodiscard]] bool is_active() const
		{
			switch(GUI_TYPE)
			{
			case GUI_TYPE_DEF: return vars.def._active;
			case GUI_TYPE_RTT: return vars.rtt._active;
			}

			return false;
		}

		void toggle(bool manually = false, bool n_state = false)
		{
			if(!manually)
			{
				switch (GUI_TYPE)
				{
				case GUI_TYPE_DEF: vars.def._active = !vars.def._active; break;
				case GUI_TYPE_RTT: vars.rtt._active = !vars.rtt._active; break;
				}
			}
			else
			{
				switch (GUI_TYPE)
				{
				case GUI_TYPE_DEF: vars.def._active = n_state; break;
				case GUI_TYPE_RTT: vars.rtt._active = n_state; break;
				}
			}
			
		}

		// has to be called last
		void open()
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: vars.def._active = true; break;
			case GUI_TYPE_RTT: vars.rtt._active = true; break;
			}
		}

		void close()
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: vars.def._active = false; break;
			case GUI_TYPE_RTT: vars.rtt._active = false; break;
			}
		}

		// returns ptr to gui window-state bool used by ImGui::Begin 
		bool* get_p_open()
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: return &vars.def._active;
			case GUI_TYPE_RTT: return &vars.rtt._active;
			}

			return nullptr;
		}

		[[nodiscard]] auto*& rtt_get_texture()
		{
			GUI_CHECK_RTT;
			return vars.rtt._scene_texture;
		}

		// *
		// no desc
		[[nodiscard]] bool is_inactive_tab() const
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: return vars.def._inactive_tab;
			case GUI_TYPE_RTT: return vars.rtt._inactive_tab;
			}

			return false;
		}

		// 
		void set_inactive_tab(const bool n_state)
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: vars.def._inactive_tab = n_state; break;
			case GUI_TYPE_RTT: vars.rtt._inactive_tab = n_state; break;
			}
		}


		// *
		// internal on-init bool
		[[nodiscard]] bool is_initiated() const
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: return vars.def._one_time_init;
			case GUI_TYPE_RTT: return vars.rtt._one_time_init;
			}

			return false;
		}

		// 
		void set_initiated(bool reset = false)
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: vars.def._one_time_init = reset ? false : true; break;
			case GUI_TYPE_RTT: vars.rtt._one_time_init = reset ? false : true; break;
			}
		}


		// *
		// is pending bring-to-front within next frame (docked windows)
		[[nodiscard]] bool is_bring_to_front_pending() const
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: return vars.def._bring_tab_to_front;
			case GUI_TYPE_RTT: return vars.rtt._bring_tab_to_front;
			}

			return false;
		}

		// bring gui to the front within next frame (docked windows)
		void set_bring_to_front(const bool n_state)
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: vars.def._bring_tab_to_front = n_state; break;
			case GUI_TYPE_RTT: vars.rtt._bring_tab_to_front = n_state; break;
			}
		}


		// *
		// get render-to-texture size
		auto& rtt_get_size()
		{
			GUI_CHECK_RTT;
			return vars.rtt._scene_size_imgui;
		}

		// update render-to-texture size (does not resize the gui, used for legacy functions)
		void rtt_set_size(const ImVec2& new_size)
		{
			GUI_CHECK_RTT;
			vars.rtt._scene_size_imgui = new_size;
		}


		// *
		// get gui position
		auto& rtt_get_position()
		{
			GUI_CHECK_RTT;
			return vars.rtt._scene_pos_imgui;
		}

		// update gui position (does not move the gui, used for legacy functions and mouse offset calculations)
		void rtt_set_position(const ImVec2& new_pos)
		{
			GUI_CHECK_RTT;
			vars.rtt._scene_pos_imgui = new_pos;
		}


		// *
		// [ImVec2] get cursor position in relation to gui position AS 
		auto& rtt_get_cursor_pos()
		{
			GUI_CHECK_RTT;
			return vars.rtt._cursor_pos;
		}

		// [ImVec2] update cursor position (incl. CPoint)
		void rtt_set_cursor_pos(const ImVec2& new_pos)
		{
			GUI_CHECK_RTT;
			vars.rtt._cursor_pos = new_pos;
			vars.rtt._cursor_pos_pt.x = static_cast<LONG>(new_pos.x);
			vars.rtt._cursor_pos_pt.y = static_cast<LONG>(new_pos.y);
		}


		// *
		// [CPoint] get cursor position in relation to gui position AS 
		auto& rtt_get_cursor_pos_cpoint()
		{
			GUI_CHECK_RTT;

			// update the CPoint variant
			vars.rtt._cursor_pos_pt.x = static_cast<LONG>(vars.rtt._cursor_pos.x);
			vars.rtt._cursor_pos_pt.y = static_cast<LONG>(vars.rtt._cursor_pos.y);

			return vars.rtt._cursor_pos_pt;
		}

		// [CPoint] update cursor position
		void rtt_set_cursor_pos_cpoint(const CPoint& new_pos)
		{
			GUI_CHECK_RTT;
			vars.rtt._cursor_pos_pt = new_pos;
			vars.rtt._cursor_pos.x = static_cast<float>(new_pos.x);
			vars.rtt._cursor_pos.y = static_cast<float>(new_pos.y);
		}


		// *
		// is gui hovered
		[[nodiscard]] bool rtt_is_hovered() const
		{
			GUI_CHECK_RTT;
			return vars.rtt._window_hovered;
		}

		// get ptr to hovered state bool
		auto* rtt_get_hovered_state()
		{
			GUI_CHECK_RTT;
			return &vars.rtt._window_hovered;
		}

		// update hovered state
		void rtt_set_hovered_state(const bool n_state)
		{
			GUI_CHECK_RTT;
			vars.rtt._window_hovered = n_state;
		}


		// *
		// is pending focus within next frame
		[[nodiscard]] bool rtt_is_focus_pending() const
		{
			GUI_CHECK_RTT;
			return vars.rtt._should_set_focus;
		}

		// focus gui within the next frame
		void rtt_set_focus_state(const bool n_state)
		{
			GUI_CHECK_RTT;
			vars.rtt._should_set_focus = n_state;
		}


		// *
		// is imgui capturing the left mouse button
		[[nodiscard]] bool rtt_is_capturing_lmb() const
		{
			GUI_CHECK_RTT;
			return vars.rtt._capture_left_mousebutton;
		}

		// imgui: capture the left mouse button (for the current / next frame)
		void rtt_set_lmb_capturing(const bool n_state)
		{
			GUI_CHECK_RTT;
			vars.rtt._capture_left_mousebutton = n_state;
		}

		[[nodiscard]] bool was_active() const
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: return vars.def._was_active;
			case GUI_TYPE_RTT: return vars.rtt._was_active;
			}

			return false;
		}

		void set_was_active(const bool s)
		{
			switch (GUI_TYPE)
			{
			case GUI_TYPE_DEF: vars.def._was_active = s; break;
			case GUI_TYPE_RTT: vars.rtt._was_active = s; break;
			}
		}
	};

	class loader final
	{
	public:
		
		template <typename T>
		class installer final
		{
			static_assert(std::is_base_of_v<ggui_module, T>, "Module has invalid base class");

		public:
			installer()
			{
				register_gui(std::make_unique<T>());
			}
		};

		template <typename T>
		static T* get()
		{
			for (const auto& module_ : *modules_)
			{
				if (typeid(*module_.get()) == typeid(T))
				{
					return reinterpret_cast<T*>(module_.get());
				}
			}

			ASSERT_MSG(false, "GET_GUI nullptr");
			return nullptr;
		}

		static const std::vector<std::unique_ptr<ggui_module>>& get_modules()
		{
			return *modules_;
		}

		static void register_gui(std::unique_ptr<ggui_module>&& module_);

	private:
		static std::vector<std::unique_ptr<ggui_module>>* modules_;
		static void destroy_modules();
	};


#define GET_GUI(name) ggui::loader::get<name>()
#define REGISTER_GUI(name)							\
namespace											\
{													\
	static ggui::loader::installer<name> $_##name;	\
}

	// *
	// | -------------------- Variables ------------------------
	// *

	extern bool			m_init_saved_states;

	extern bool			m_ggui_initialized;
	extern ImGuiContext* m_ggui_context;

	extern ImGuiID		m_dockspace_outer_left_node;
	extern bool			m_dockspace_initiated;
	extern bool			m_dockspace_reset;
	extern bool			mainframe_menubar_enabled;

	extern bool			m_demo_menu_state;

	// -----------

	struct commandbinds
	{
		std::string cmd_name;
		std::string keys;
		int modifier_alt;
		int modifier_ctrl;
		int modifier_shift;
		std::string modifier_key;
	};

	extern std::vector<commandbinds> cmd_hotkeys;
	extern std::vector<game::SCommandInfo> cmd_addon_hotkeys_builtin;
	extern std::vector<game::SCommandInfoHotkey> cmd_addon_hotkeys;
	
	// *
	// | -------------------- Functions ------------------------
	// *

	extern ImVec2 get_initial_window_pos();
	extern void set_next_window_initial_pos_and_constraints(ImVec2 mins, ImVec2 initial_size, ImVec2 overwrite_pos = ImVec2(0.0f, 0.0f));
	extern bool is_ggui_initialized();

	extern bool rtt_handle_windowfocus_overlaywidget(bool* gui_hover_state);
	extern void redraw_undocking_triangle(ImGuiWindow* wnd, bool* gui_hover_state);

	extern void dragdrop_overwrite_leftmouse_capture();
	extern void dragdrop_reset_leftmouse_capture();

	extern void file_dialog_frame();
}
