#pragma once

namespace ggui
{
	class template_dialog final : public ggui::ggui_module
	{
	public:
		template_dialog() { set_gui_type(GUI_TYPE_RTT); } // render-to-texture gui


		// *
		// public member functions

		void gui() override;


		// *
		// asm related



		// *
		// init

		
	};
}
