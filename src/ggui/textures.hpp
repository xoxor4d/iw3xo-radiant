#pragma once

namespace ggui
{
	class texture_dialog final : public ggui::ggui_module
	{
	public:
		texture_dialog() { set_gui_type(GUI_TYPE_RTT); }
		bool gui() override;

		auto& get_filter()
		{
			return imgui_filter;
		}

		[[nodiscard]] int get_filter_length() const
		{
			return imgui_filter_last_len;
		}

	private:
		ImGuiTextFilter	imgui_filter;
		int				imgui_filter_last_len = 0;

		void toolbar_new();
		void toolbar(const ImVec2& screenpos);
	};
}
