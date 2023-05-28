#pragma once
#include "_ggui.hpp"

//namespace ggui::surface_inspector
//{
//	struct patch_texdef_t
//	{
//		game::MaterialDef def;
//		int unk3;
//		float sample_size;
//	};
//
//	struct texedit_helper
//	{
//		void reset_values()
//		{
//			size_horz = 0.0f;
//			size_vert = 0.0f;
//			shift_horz = 0.0f;
//			shift_vert = 0.0f;
//			rotation = 0.0f;
//			//repeatx = 0.0f;
//			//repeaty = 0.0f;
//		}
//
//		float amount_size = 1.0f;
//		float amount_shift = 1.0f;
//		float amount_rotate = 15.0f;
//		float amount_sample = 4.0f;
//		float amount_repeatx = 0.5f;
//		float amount_repeaty = 0.5f;
//
//		float size_horz;
//		float size_vert;
//		float shift_horz;
//		float shift_vert;
//		float rotation;
//		float repeatx = 1.0f;
//		float repeaty = 1.0f;
//
//		int	 scalar_direction;
//		bool specific_mode;
//	};
//
//	void	controls();
//	void	menu(ggui::imgui_context_menu& menu);
//	
//
//}

namespace ggui
{
	class surface_dialog final : public ggui::ggui_module
	{
	public:
		struct patch_texdef_t
		{
			game::MaterialDef def;
			int unk3;
			float sample_size;
		};

		class texedit_helper
		{
		public:
			float amount_size = 1.0f;
			float amount_shift = 1.0f;
			float amount_rotate = 15.0f;
			float amount_sample = 4.0f;
			float amount_repeatx = 0.5f;
			float amount_repeaty = 0.5f;

			float size_horz = 1.0f;
			float size_vert = 1.0f;
			float shift_horz = 1.0f;
			float shift_vert = 1.0f;
			float rotation = 1.0f;
			float repeatx = 1.0f;
			float repeaty = 1.0f;

			int	 scalar_direction = 0;
			bool specific_mode = false;
			bool original_logic = false;

			texedit_helper() = default;

			void reset_values()
			{
				size_horz = 0.0f;
				size_vert = 0.0f;
				shift_horz = 0.0f;
				shift_vert = 0.0f;
				rotation = 0.0f;
			}
		};

		surface_dialog() { set_gui_type(GUI_TYPE_DEF); }


		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;

		void inspector_controls(bool is_toolbox = false, float max_width = 0.0f);

		// *
		// asm related

		static void on_surfaceinspector_command();

		// *
		// init

		static void	hooks();
		static void	register_dvars();

	private:
		typedef int TEXMODE;
		enum TEXMODE_ : int
		{
			TEXMODE_NONE = 0,
			TEXMODE_SIZE_HORZ = 1 << 0,
			TEXMODE_SIZE_VERT = 1 << 1,
			TEXMODE_SHIFT_HORZ = 1 << 2,
			TEXMODE_SHIFT_VERT = 1 << 3,
			TEXMODE_ROTATE = 1 << 4
		};

		void edit_texture_info(game::texdef_sub_t* texdef, texedit_helper* helper, TEXMODE mode = TEXMODE_NONE, int dir = 0);
	};
}
