#pragma once

namespace Components
{
	class Gui : public Component
	{
	public:
		Gui();
		~Gui();

		const char* getName() override { return "Gui"; };

		static void imgui_init();
		static void reset();
		
		static void begin_frame();
		static void end_frame(); 

		static void render_loop();
		static void toggle(Game::gui_menus_t& menu, int keycatcher, bool onCommand);

		static void set_menu_layout(Game::gui_menus_t& menu, const float x, const float y, const float width, const float height, const int horzAlign, const int vertAlign);

		static void register_dvars();
		static void load_settings();
		static void save_settings();
		static bool any_open_menus();

		static Game::gui_menus_t&	GetMenu(Game::GUI_MENUS id);

	private:
	};
}
