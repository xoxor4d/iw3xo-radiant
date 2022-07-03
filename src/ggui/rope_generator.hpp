#pragma once

namespace ggui
{
	class rope_generator_dialog final : public ggui::ggui_module
	{
		int m_thickness = 6;
		int m_slack = 20;
		int m_subdiv = 2;
		bool m_delete_null = false;

	public:
		rope_generator_dialog() { set_gui_type(GUI_TYPE_DEF); }

		// *
		// public member functions

		bool gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related


		// *
		// init

	private:
		void generate_rope(int thickness, int slack, int subdiv, bool delete_null);
	};
}
