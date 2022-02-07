#pragma once

namespace components
{
	class effects : public component
	{
	public:
		effects();
		~effects();

		const char* get_name() override { return "effects"; };

		static bool load_test_effect(const char* effect_name);

		static void editor_on_effect_play_repeat();
		static void editor_trigger_effect(int msecBegin);

		static bool effect_is_paused();
		static bool effect_is_playing();
		static bool effect_is_repeating();
		static bool effect_can_play();

		static bool is_fx_origin_selected();
		static void fx_origin_frame();

		static void tick_playback();
		static void tick_repeat();

		static void on_effect_stop();
		static void reset_editor_effect();
		static void radiant_init_fx();

		static void generate_createfx();

		static inline bool			is_fx_origin_selected_;
		static inline std::string	last_fx_name_;

	private:
	};
}
