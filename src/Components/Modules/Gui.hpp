#pragma once

namespace Components
{
	class Gui : public Component
	{
	public:
		Gui();
		~Gui();

		const char* getName() override { return "Gui"; };

		//static void imgui_init_ccamerawnd();
		static void shutdown();
		static bool all_contexts_ready();
		
		static void begin_frame();
		static void end_frame(); 

		static void render_loop();
		static void toggle(ggui::imgui_context_menu& menu, int keycatcher, bool onCommand);

		static void register_dvars();
		//static bool any_open_menus();

	private:
	};
}
