#pragma once

namespace ggui
{
	class curve_patch_dialog final : public ggui::ggui_module
	{
	public:
		curve_patch_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;
	};

	// #
	// ---------------

	class terrain_patch_dialog final : public ggui::ggui_module
	{
	public:
		terrain_patch_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;
	};

	// #
	// ---------------

	class thicken_patch_dialog final : public ggui::ggui_module
	{
	public:
		thicken_patch_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;
	};

	// #
	// ---------------

	class cap_patch_dialog final : public ggui::ggui_module
	{
	public:
		cap_patch_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;
	};
}
