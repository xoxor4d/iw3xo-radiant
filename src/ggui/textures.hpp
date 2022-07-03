#pragma once

namespace ggui
{
	class texture_dialog final : public ggui::ggui_module
	{
		int m_selected_favourite = 0;
		std::string m_selected_favourite_str;
		std::string m_add_selected_to_list_str;

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

		[[nodiscard]] int get_favourite_index() const
		{
			return m_selected_favourite;
		}

		void set_favourite_index(int i)
		{
			m_selected_favourite = i;
		}

		[[nodiscard]] const std::string& get_favourite_str() const
		{
			return m_selected_favourite_str;
		}

		void set_favourite_str(const std::string& str)
		{
			m_selected_favourite_str = str;
		}


	private:
		ImGuiTextFilter	imgui_filter;
		int				imgui_filter_last_len = 0;

		void toolbar_new();
		void toolbar(const ImVec2& screenpos);
	};
}
