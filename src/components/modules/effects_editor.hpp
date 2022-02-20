#pragma once

namespace components
{
	class effects_editor : public component
	{
	public:
		effects_editor();
		~effects_editor();
		const char* get_name() override { return "effects_editor"; };

		static bool editor_can_add_segment();
		static bool editor_can_delete_segment();

		static void editor_add_new_segment();
		static void editor_delete_segment(int index);

		static bool		is_editor_active();

	private:

	};
}
