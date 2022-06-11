#pragma once

namespace ggui
{
	class curve_patch_dialog final : public ggui::ggui_module
	{
	public:
		curve_patch_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related

		// *
		// init
		
	};

	// ---------------

	class terrain_patch_dialog final : public ggui::ggui_module
	{
	public:
		terrain_patch_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related

		// *
		// init

	};

	// ---------------

	class thicken_patch_dialog final : public ggui::ggui_module
	{
	public:
		thicken_patch_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related

		// *
		// init

	};
}
