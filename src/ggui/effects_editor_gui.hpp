#pragma once

namespace ggui::effects_editor_gui
{
	//static inline int selected_editor_elemdef = 0;
	//static inline bool editor_effect_was_modified = false;

	

	//bool		Modal_UnsavedChanges(const char* label);
	//void		menu(ggui::imgui_context_menu& menu);

}

namespace ggui
{
	/*extern int selected_editor_elemdef;
	extern bool m_effect_was_modified;
	extern bool m_pending_close;
	extern bool m_pending_reload;*/

	class effects_editor_dialog final : public ggui::ggui_module
	{
		ImGradient gradient;
		int selected_editor_elemdef = 0;

		const char* s_elemTypeNames[11] =
		{
			"Billboard Sprite",
			"Oriented Sprite",
			"Tail",
			"Geo Trail",
			"Cloud",
			"Model",
			"Light",
			"SpotLight",
			"Sound",
			"Decal",
			"FX Runner"
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_line[] =
		{
			{ { 0.00f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ { 0.00f,  0.87500f}, { .0f, .0f}, 1.0f },
			{ { 0.00f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_triangle[] =
		{
			{ {  0.00f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.75f,  0.43750f}, { .0f, .0f}, 0.666667f },
			{ { -0.75f,  0.43750f}, { .0f, .0f}, 1.33333f },
			{ {  0.00f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_quad[] =
		{
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.87500f,  0.00000f}, { .0f, .0f}, 0.5f },
			{ { -0.00000f,  0.90625f}, { .0f, .0f}, 1.0f },
			{ { -0.87500f, -0.00000f}, { .0f, .0f}, 1.5f },
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_pentagon[] =
		{
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.87500f, -0.28125f}, { .0f, .0f}, 0.4f },
			{ {  0.50000f,  0.71875f}, { .0f, .0f}, 0.8f },
			{ { -0.53125f,  0.71875f}, { .0f, .0f}, 1.2f },
			{ { -0.87500f, -0.28125f}, { .0f, .0f}, 1.6f },
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_hexagon[] =
		{
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.75000f, -0.46875f}, { .0f, .0f}, 0.333333f },
			{ {  0.75000f,  0.43750f}, { .0f, .0f}, 0.666667f },
			{ { -0.00000f,  0.87500f}, { .0f, .0f}, 1.0f },
			{ { -0.75000f,  0.43750f}, { .0f, .0f}, 1.33333f },
			{ { -0.75000f, -0.46875f}, { .0f, .0f}, 1.66667f },
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_septagon[] =
		{
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.68750f, -0.56250f}, { .0f, .0f}, 0.285714f },
			{ {  0.87500f,  0.18750f}, { .0f, .0f}, 0.571429f },
			{ {  0.37500f,  0.78125f}, { .0f, .0f}, 0.857143f },
			{ { -0.37500f,  0.78125f}, { .0f, .0f}, 1.14286f },
			{ { -0.87500f,  0.18750f}, { .0f, .0f}, 1.42857f },
			{ { -0.68750f, -0.56250f}, { .0f, .0f}, 1.71429f },
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_octagon[] =
		{
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.62500f, -0.65625f}, { .0f, .0f}, 0.25f },
			{ {  0.87500f,  0.00000f}, { .0f, .0f}, 0.5f },
			{ {  0.62500f,  0.62500f}, { .0f, .0f}, 0.75f },
			{ { -0.00000f,  0.90625f}, { .0f, .0f}, 1.0f },
			{ { -0.62500f,  0.62500f}, { .0f, .0f}, 1.25f },
			{ { -0.87500f, -0.00000f}, { .0f, .0f}, 1.5f },
			{ { -0.65625f, -0.65625f}, { .0f, .0f}, 1.75f },
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_nonagon[] =
		{
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.56250f, -0.71875f}, { .0f, .0f}, 0.222222f },
			{ {  0.87500f, -0.18705f}, { .0f, .0f}, 0.444444f },
			{ {  0.75000f,  0.43750f}, { .0f, .0f}, 0.666667f },
			{ {  0.31250f,  0.84375f}, { .0f, .0f}, 0.888889f },
			{ { -0.31250f,  0.84375f}, { .0f, .0f}, 1.11111f },
			{ { -0.75000f,  0.43750f}, { .0f, .0f}, 1.33333f },
			{ { -0.87500f, -0.18750f}, { .0f, .0f}, 1.55556f },
			{ { -0.56250f, -0.71875f}, { .0f, .0f}, 1.77778f },
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const fx_system::FxTrailVertex geotrail_shape_decagon[] =
		{
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 0.0f },
			{ {  0.50000f, -0.75000f}, { .0f, .0f}, 0.2f },
			{ {  0.84375f, -0.28125f}, { .0f, .0f}, 0.4f },
			{ {  0.84375f,  0.25000f}, { .0f, .0f}, 0.6f },
			{ {  0.50000f,  0.71875f}, { .0f, .0f}, 0.8f },
			{ { -0.00000f,  0.90625f}, { .0f, .0f}, 1.0f },
			{ { -0.53125f,  0.71875f}, { .0f, .0f}, 1.2f },
			{ { -0.87500f,  0.25000f}, { .0f, .0f}, 1.4f },
			{ { -0.87500f, -0.28125f}, { .0f, .0f}, 1.6f },
			{ { -0.53125f, -0.75000f}, { .0f, .0f}, 1.8f },
			{ {  0.00000f, -0.90625f}, { .0f, .0f}, 2.0f },
		};

		static inline const std::uint16_t indices_list_line[] = { 4, 0, 1, 1, 2 };
		static inline const std::uint16_t indices_list_tri[] = { 6, 0, 1, 1, 2, 2, 3 };
		static inline const std::uint16_t indices_list_quad[] = { 8, 0, 1, 1, 2, 2, 3, 3, 4 };
		static inline const std::uint16_t indices_list_pent[] = { 10, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5 };
		static inline const std::uint16_t indices_list_hex[] = { 12, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6 };
		static inline const std::uint16_t indices_list_sep[] = { 14, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7 };
		static inline const std::uint16_t indices_list_oct[] = { 16, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8 };
		static inline const std::uint16_t indices_list_non[] = { 18, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9 };
		static inline const std::uint16_t indices_list_dec[] = { 20, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10 };

		static inline const std::uint16_t* indices_list[] =
		{
			nullptr,			// 0
			nullptr,			// 1
			nullptr,			// 2
			indices_list_line,	// 3
			indices_list_tri,	// 4
			indices_list_quad,	// 5
			indices_list_pent,	// 6
			indices_list_hex,	// 7
			indices_list_sep,	// 8
			indices_list_oct,	// 9
			indices_list_non,	// 10
			indices_list_dec	// 11
		};

	public:
		bool m_effect_was_modified;
		bool m_pending_close;
		bool m_pending_reload;

		effects_editor_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			m_effect_was_modified = false;
			m_pending_close = false;
			m_pending_reload = false;
		}

		// *
		// public member functions

		bool gui() override;


		// *
		// asm related



		// *
		// init



	private:
		void elemdef_elem(fx_system::FxEditorElemDef* elemdef, int row, int* selected_index);

		fx_system::FxEffectDef* effectdef_fileprompt();
		game::Material* material_fileprompt();
		game::XModel* xmodel_fileprompt();

		void on_modified(bool modified);

		void tab_generation(fx_system::FxEditorElemDef* elem);
		void tab_size(fx_system::FxEditorElemDef* elem);
		void tab_velocity(fx_system::FxEditorElemDef* elem);
		void tab_rotation(fx_system::FxEditorElemDef* elem);
		void tab_physics(fx_system::FxEditorElemDef* elem);
		void tab_color(fx_system::FxEditorElemDef* elem);
		void tab_visuals(fx_system::FxEditorElemDef* elem);

		void effect_elemdef_list();
		bool effect_property_window();

		bool modal_unsaved_changes(const char* label);
	};
}
