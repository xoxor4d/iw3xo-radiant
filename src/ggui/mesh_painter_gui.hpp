#pragma once

namespace ggui
{
	class mesh_painter_dialog final : public ggui::ggui_module
	{
	public:
		mesh_painter_dialog() { set_gui_type(GUI_TYPE_DEF); }

		bool gui() override;
		void on_init() override;
		void on_open() override;
		void on_close() override;
	};
}
