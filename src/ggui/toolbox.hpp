#pragma once

namespace ggui
{
	class toolbox_dialog final : public ggui::ggui_module
	{
	private:
		struct toolbox_child_s
		{
			unsigned int index;
			std::function<void()> callback;
		};

		nlohmann::fifo_map<std::string, toolbox_child_s> _toolbox_childs;

		bool m_update_scroll;
		int m_child_current;
		unsigned int m_child_count;

		const std::string CAT_BRUSH = "brush";
		const std::string CAT_PATCH = "patch";
		const std::string CAT_SURF_INSP = "surface_inspector";
		const std::string CAT_ENTITY_PROPS = "entity_properties";
		const std::string CAT_FILTER = "Filter";
		const std::string CAT_LAYERS = "Layers";

	public:
		enum class TB_CHILD : int
		{
			BRUSH,
			PATCH,
			SURFACE_INSP,
			ENTITY_PROPS,
			FILTER,
			LAYERS
		};

		toolbox_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			m_update_scroll = false;
			m_child_current = 0;
			m_child_count = 0;
		}

		void focus_child(TB_CHILD child)
		{
			m_child_current = static_cast<int>(child);
			m_update_scroll = true;
		}

		// *
		// public member functions
		bool gui() override;
		void on_init() override;

		static bool treenode_begin(const char* name, bool default_open, int& style_colors, int& style_vars);
		static void treenode_end(int style_colors, int style_vars, float end_spacing = 8.0f);
		static void center_horz_begin(const float group_width, float indent = 4.0f);
		static void center_horz_end(float& group_width);

		static void register_dvars();

	private:
		void  register_child(const std::string& _child_name, const std::function<void()>& _callback);

		void child_brush();
		void child_patch();
		void child_surface_inspector();
		void child_entity_properties();
		void child_filter();
		void child_layers();
	};
}
