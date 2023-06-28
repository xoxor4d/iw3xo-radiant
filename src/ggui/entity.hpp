#pragma once

namespace ggui
{
	class entity_dialog final : public ggui::ggui_module
	{
		enum EPAIR_VALUETYPE
		{
			TEXT = 0,
			FLOAT = 1,
			COLOR = 2,
			ORIGIN = 3,
			ANGLES = 4,
			MODEL = 5,
			FX = 6
		};

		struct epair_override_type
		{
			EPAIR_VALUETYPE type;
			std::string key;
		};

		std::vector<epair_override_type> overriden_valuetypes;

		struct epair_wrapper
		{
			game::entity_s_def* entity;
			game::eclass_t* eclass;
			game::epair_t* epair;
			EPAIR_VALUETYPE type;
			float v_speed = 0.1f;
			float v_min = 0.0f;
			float v_max = FLT_MAX;
			bool  is_fixedsize;
		};

		std::vector<epair_wrapper> eprop_sorted;

		// *
		// key value pair vars

		const static int EPROP_MAX_ROWS = 32; // you are doing something wrong if you hit this limit D:
		const static int EPROP_INPUTLEN = 256;

		struct new_kvp_helper
		{
			bool _in_use;

			char key_buffer[EPROP_INPUTLEN];
			bool key_active;
			bool key_set_focus;
			bool key_valid;

			char value_buffer[EPROP_INPUTLEN];
			bool value_active;
			bool value_set_focus;
			bool value_valid;

			void reset()
			{
				memset(this, 0, sizeof(new_kvp_helper));
			}
		};

		new_kvp_helper kvp_helper = {};

		static inline char _edit_buf_key[EPROP_INPUTLEN][EPROP_MAX_ROWS] = {};
		static inline bool _edit_buf_key_dirty[EPROP_MAX_ROWS] = {};

		static inline char _edit_buf_value[EPROP_INPUTLEN][EPROP_MAX_ROWS] = {};
		static inline bool _edit_buf_value_dirty[EPROP_MAX_ROWS] = {};


	public:
		struct template_kvp
		{
			const char* key;
			const char* val;
		};

		enum ECLASS_TYPE : int
		{
			CLASS_LIGHT = 0x1,
			CLASS_WORLDSPAWN = 0x2,
			CLASS_PATH = 0x4,
			CLASS_MODEL = 0x8,
			CLASS_PREFAB = 0x10,
			CLASS_NODE = 0x20,
			CLASS_TRIGGER_RADIUS = 0x40,
			CLASS_TRIGGER_DISC = 0x80,
			CLASS_REFLECTION_PROBE = 0x100,
		};

		struct addprop_helper_s
		{
			bool is_origin;
			bool is_angle;
			bool is_generic_slider;
			bool is_color;
			bool add_undo;
		};

		std::vector<game::eclass_t*> m_classlist;
		game::eclass_t* m_sel_list_ent;
		game::eclass_t* m_edit_entity_class;
		bool m_edit_entity_changed;
		bool m_edit_entity_changed_should_scroll;
		bool m_checkboxflags_states[12] = {};
		bool m_update_worldspawn = true;

		entity_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			m_sel_list_ent = nullptr;
			m_edit_entity_class = nullptr;
			m_edit_entity_changed = false;
			m_edit_entity_changed_should_scroll = false;
			memset(m_checkboxflags_states, 0, sizeof(m_checkboxflags_states));
		}

		// *
		// public member functions

		bool gui() override;
		void	add_prop(const char* key, const char* value, addprop_helper_s* helper = nullptr);
		void	del_prop(const char* key, bool overwrite_classname_check = false);
		char*	get_value_for_key_from_epairs(game::epair_t*& e, const char* key);
		bool	get_value_for_key_from_entity(game::entity_s* ent, float* value, const char* keyname);
		bool	get_vec3_for_key_from_entity(game::entity_s* ent, float* vec3, const char* keyname);
		bool	has_key_value_pair(game::entity_s_def* ent, const char* key);
		bool	has_key_with_value(game::entity_s_def* ent, const char* key, const char* value);

		void	draw_classlist();
		void	draw_comments(const float start_indent = 0.0f);
		void	draw_checkboxes();
		void	draw_entprops(const float max_width = 0.0f, const float start_indent = 0.0f);

		// *
		// asm related

		static void on_mapload_intercept(); // not a callable method
		static void on_update_selection_intercept(); // not a callable method
		static void fill_classlist_intercept(); // not a callable method
		static void init_classlist_stub(); // not a callable method
		static void on_update_selection_stub(); // not a callable method
		static void on_mapload_stub(); // not a callable method
		static void on_viewentity_command(); // not a callable method

		// *
		// init

		static void	register_dvars();
		static void	hooks();

	private:
		void	get_eclass_template(const template_kvp*& tkvp, int* size_out);
		void	separator_for_treenode();
		bool	drag_float_helper_undo(addprop_helper_s* helper, const char* label, float* v, float v_speed, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
		bool	color_edit3_helper_undo(addprop_helper_s* helper, const char* label, float col[3], ImGuiColorEditFlags flags = 0, const ImVec2& color_button_size = ImVec2(0, 0));

		
		void	gui_entprop_new_keyvalue_pair();
		void	gui_entprop_add_key(game::epair_t* epair, int row);
		void	gui_entprop_add_value_text(const epair_wrapper& epw, int row);
		void	gui_entprop_effect_fileprompt(const epair_wrapper& epw, int row);
		void	gui_entprop_model_fileprompt(const epair_wrapper& epw, int row);
		void	gui_entprop_add_value_slider(const epair_wrapper& epw);
		void	gui_entprop_add_value_color(const epair_wrapper& epw);

		void	brush_moveto(game::brush_t_with_custom_def* b, const float* new_origin, const float* old_origin, bool snap = true);
		void	brush_moveto_center(game::brush_t_with_custom_def* b, const float* new_origin, bool snap = true);

		void	gui_entprop_add_value_vec3(const epair_wrapper& epw, float* vec_in, int row = 0);
		void	gui_entprop_add_value_vec3(const epair_wrapper& epw, int row);

		
	};
}

