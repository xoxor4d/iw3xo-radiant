#include "std_include.hpp"

namespace ggui
{
	void effects_browser::effect_listbox_elem(int index)
	{
		//const auto m_selector = ggui::get_rtt_modelselector();

		const bool is_selected = (this->m_effect_selection == index);
		if (ImGui::Selectable(this->m_effect_filelist[index], is_selected))
		{
			this->m_effect_selection_old = m_effect_selection;
			this->m_effect_selection = index;

			cfxwnd::get()->stop_effect();
			cfxwnd::get()->load_effect(this->m_effect_filelist[index]);

			//this->m_preview_model_name = this->m_effect_filelist[index];
		} TT("Drag me to the grid or camera window");

		if (is_selected) 
		{
			ImGui::SetItemDefaultFocus();
		}

		/*if (is_selected && m_update_scroll_position)
		{
			ImGui::SetScrollHereY();
			m_update_scroll_position = false;
		}*/

		// target => grid_dialog::drag_drop_target()
		// target => camera_dialog::drag_drop_target()
		// target => entity_dialog::gui_entprop_add_value_text()
		// target => effects_editor_dialog::tab_visuals()
		/*if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("MODEL_SELECTOR_ITEM", nullptr, 0, ImGuiCond_Once);

			this->m_effect_selection = index;
			this->m_preview_model_name = this->m_effect_filelist[index];

			ImGui::Text("Model: %s", this->m_effect_filelist[index]);
			ImGui::EndDragDropSource();
		}*/
	}

	bool effects_browser::gui()
	{
		const auto io = ImGui::GetIO();

		const auto MIN_WINDOW_SIZE = ImVec2(500.0f, 400.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(550.0f, 600.0f);

		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));

		if (!ImGui::Begin("Effects Browser##rtt", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			ImGui::End();
			return false;
		}

		const auto window_size = ImGui::GetWindowSize();
		const bool split_horizontal = window_size.x >= 700.0f;
		const auto listbox_width = split_horizontal ? ImClamp(window_size.x * 0.25f, 200.0f, FLT_MAX) : ImGui::GetContentRegionAvail().x;

		ImGui::BeginGroup();
		{
			// filter widget
			const auto screenpos_prefilter = ImGui::GetCursorScreenPos();
			this->m_filter.Draw("##xmodel_filter", listbox_width - 32.0f);
			const auto screenpos_postfilter = ImGui::GetCursorScreenPos();

			ImGui::SameLine(listbox_width - 27.0f);
			if (ImGui::ButtonEx("x##clear_filter"))
			{
				this->m_filter.Clear();
			}

			if (!this->m_filter.IsActive())
			{
				ImGui::SetCursorScreenPos(ImVec2(screenpos_prefilter.x + 12.0f, screenpos_prefilter.y + 4.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
				ImGui::TextUnformatted("Filter ..");
				ImGui::PopStyleColor();
				ImGui::SetCursorScreenPos(ImVec2(screenpos_postfilter.x, screenpos_postfilter.y));
			}

			// xmodel listbox
			if (ImGui::BeginListBox("##effect_list", ImVec2(listbox_width, split_horizontal ? ImGui::GetContentRegionAvail().y : ImGui::GetContentRegionAvail().y * 0.5f)))
			{
				if (this->m_filter.IsActive())
				{
					for (int i = 0; i < this->m_effect_filecount; i++)
					{
						if (!this->m_filter.PassFilter(this->m_effect_filelist[i]))
						{
							continue;
						}

						effect_listbox_elem(i);
					}
				}
				else
				{
					ImGuiListClipper clipper;
					clipper.Begin(this->m_effect_filecount);

					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						{
							effect_listbox_elem(i);
						}
					}
					clipper.End();
				}

				ImGui::EndListBox();
			}
		}
		ImGui::EndGroup();

		if (split_horizontal)
		{
			ImGui::SameLine();
		}

		ImGui::BeginChild("##pref_child", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			const auto child_size = ImGui::GetContentRegionAvail();
			SetWindowPos(components::renderer::get_window(components::renderer::CFXWND)->hwnd, HWND_BOTTOM, 0, 0, static_cast<int>(child_size.x), static_cast<int>(child_size.y), SWP_NOZORDER);
			this->rtt_set_size(child_size);

			if (this->rtt_get_texture())
			{
				ImGui::Image(this->rtt_get_texture(), child_size);
				this->m_scene_texture_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);

				if (this->m_scene_texture_hovered)
				{
					// zoom
					if (io.MouseWheel != 0.0f)
					{
						this->m_camera_distance += (io.MouseWheel * 10.0f);
					}
				}
			}
		}

		ImGui::EndChild();
		ImGui::End();

		return true;
	}

	void effects_browser::on_open()
	{
		//if (!this->is_initiated())
		{
			this->m_effect_filelist = game::FS_ListFilteredFilesWrapper("fx", nullptr, &this->m_effect_filecount);
			//this->set_initiated();
		}
	}

	void effects_browser::on_close()
	{ }

	REGISTER_GUI(effects_browser);
}
