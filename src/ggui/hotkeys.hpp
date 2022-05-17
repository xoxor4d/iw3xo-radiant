#pragma once

namespace ggui
{
	class hotkey_helper_dialog final : public ggui::ggui_module
	{
	public:
		hotkey_helper_dialog() { set_gui_type(GUI_TYPE_DEF); }


		// *
		// public member functions

		void gui() override;
	};


	// * ---------------------


	class hotkey_dialog final : public ggui::ggui_module
	{
	private:
		ImGuiTextFilter	m_filter;

	public:
		hotkey_dialog() { set_gui_type(GUI_TYPE_DEF); }


		// *
		// public member functions

		void gui() override;
		void on_open() override;
		void on_close() override;

		static int cmdbinds_key_to_ascii(std::string key);
		static std::string cmdbinds_ascii_to_keystr(int key);
		static bool cmdbinds_load_from_file(std::string file);
		static std::string get_hotkey_for_command(const char* command);

		// *
		// asm related

		static void load_commandmap();
		static void load_default_commandmap();

		// *
		// init

		void hooks();

	private:
		bool cmdbinds_check_dupe(commandbinds& bind, std::string& o_dupebind);
		void do_row(int index, int& row_counter);
	};
}
