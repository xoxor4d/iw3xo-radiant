#include "std_include.hpp"

namespace ggui
{
	void filter_dialog::build_radiant_filterlists()
	{
		// *
		if (!_geofilters_initialized)
		{
			for (auto	filter = game::g_qeglobals->d_filterGlobals_geometryFilters;
						filter;
						filter = filter->next_filter)
			{
				if (!filter->name) 
				{
					break;
				}

				_geofilters.push_back(filter);
			}

			std::ranges::reverse(_geofilters.begin(), _geofilters.end());
			_geofilters_initialized = true;
		}

		// *
		if (!_entityfilters_initialized)
		{
			for (auto	filter = game::g_qeglobals->d_filterGlobals_entityFilters;
						filter;
						filter = filter->next_filter)
			{
				if (!filter->name) 
				{
					break;
				}

				_entityfilters.push_back(filter);
			}

			std::ranges::reverse(_entityfilters.begin(), _entityfilters.end());
			_entityfilters_initialized = true;
		}

		// *
		if (!_triggerfilters_initialized)
		{
			for (auto	filter = game::g_qeglobals->d_filterGlobals_triggerFilters;
						filter;
						filter = filter->next_filter)
			{
				if (!filter->name) 
				{
					break;
				}

				_triggerfilters.push_back(filter);
			}

			std::ranges::reverse(_triggerfilters.begin(), _triggerfilters.end());
			_triggerfilters_initialized = true;
		}

		// *
		if (!_otherfilters_initialized)
		{
			for (auto	filter = game::g_qeglobals->d_filterGlobals_otherFilters;
						filter;
						filter = filter->next_filter)
			{
				if (!filter->name) 
				{
					break;
				}

				_otherfilters.push_back(filter);
			}

			std::ranges::reverse(_otherfilters.begin(), _otherfilters.end());
			_otherfilters_initialized = true;
		}
	}

	// *
	// bridges imgui and original radiant filter window
	void filter_dialog::handle_radiant_filter(CCheckListBox* checklist, game::filter_entry_s* filter, int index)
	{
		const int checked = afx::CCheckListBox__GetCheck(checklist, index);
		if (imgui::RadioButton(filter->name, checked))
		{
			if (checked)
			{
				afx::CCheckListBox__SetCheck(checklist, index, 0);
				filter->isShown = false;

				if (!((filter->filter_type_enum & game::FILTER_TYPE2::FACE) == 0))
				{
					// RadiantFilters05_large
					utils::hook::call<void(__cdecl)(game::filter_entry_s*)>(0x411BE0)(filter);
				}
			}
			else
			{
				afx::CCheckListBox__SetCheck(checklist, index, 1);
				filter->isShown = true;

				if (!((filter->filter_type_enum & game::FILTER_TYPE2::FACE) == 0))
				{
					// RadiantFilters06_large
					utils::hook::call<void(__cdecl)(game::filter_entry_s*)>(0x411E30)(filter);
				}
			}

			game::g_qeglobals->g_filtersUpdated++;
		}
	}

	// retuns original enabled/disabled state before the filter was toggled either way
	bool filter_dialog::toggle_by_name(const char* name, E_FILTERS filter_type, bool on_off)
	{
		build_radiant_filterlists();

		bool return_val = false;

		CCheckListBox* checklistbox;
		size_t filter_list_count = 0;

		std::string filter_name = name;
		filter_name = utils::str_to_lower(filter_name);

		switch (filter_type)
		{
		case GEOMETRY:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->geometry_filters;
			filter_list_count = _geofilters.size();
			break;

		case ENTITY:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->entity_filters;
			filter_list_count = _entityfilters.size();
			break;

		case TRIGGER:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->trigger_filters;
			filter_list_count = _triggerfilters.size();
			break;

		case OTHER:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->other_filters;
			filter_list_count = _otherfilters.size();
			break;

		default:
			return false;
		}

		for (size_t i = 0; i < filter_list_count; i++)
		{
			game::filter_entry_s* filter = nullptr;

			switch (filter_type)
			{
			case GEOMETRY:
				filter = _geofilters[i];
				break;

			case ENTITY:
				filter = _entityfilters[i];
				break;

			case TRIGGER:
				filter = _triggerfilters[i];
				break;

			case OTHER:
				filter = _otherfilters[i];
				break;

			default:
				return false;
			}

			if (filter && filter->name)
			{
				if (utils::str_to_lower(filter->name) == filter_name)
				{
					return_val = filter->isShown;

					if (!on_off && filter->isShown)
					{
						afx::CCheckListBox__SetCheck(checklistbox, i, 0);
						filter->isShown = false;

						if (!((filter->filter_type_enum & game::FILTER_TYPE2::FACE) == 0))
						{
							// RadiantFilters05_large
							utils::hook::call<void(__cdecl)(game::filter_entry_s*)>(0x411BE0)(filter);
						}
					}
					else if (on_off && !filter->isShown)
					{
						afx::CCheckListBox__SetCheck(checklistbox, i, 1);
						filter->isShown = true;

						if (!((filter->filter_type_enum & game::FILTER_TYPE2::FACE) == 0))
						{
							// RadiantFilters06_large
							utils::hook::call<void(__cdecl)(game::filter_entry_s*)>(0x411E30)(filter);
						}
					}

					game::g_qeglobals->g_filtersUpdated++;
				}
			}
		}

		return return_val;
	}

	void filter_dialog::geometry_filters(float max_width)
	{
		static float geo_child_height = 200.0f;

		const float column_width = 140.0f;
		int column_count = static_cast<int>(max_width / column_width);
		column_count = column_count < 2 ? 2 : column_count;
		column_count = column_count > 6 ? 6 : column_count;

		imgui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 1.0f));
		imgui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));

		auto min = imgui::GetCursorScreenPos();
		min.y += imgui::GetFrameHeight();

		const auto max = ImVec2(min.x + max_width, min.y + geo_child_height - imgui::GetFrameHeight());
		imgui::GetWindowDrawList()->AddRectFilled(min, max, imgui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 0.3f)), 12.0f, ImDrawFlags_RoundCornersBottom);

		//imgui::BeginChild("##geo_child", ImVec2(0.0f, geo_child_height), false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		{
			imgui::BeginGroup();
			{
				if (imgui::ButtonEx("Toggle All Geo", ImVec2(max_width, imgui::GetFrameHeight()), ImGuiButtonFlags_AlignTextBaseLine))
				{
					components::gameview::p_this->toggle_all_geo(!components::gameview::p_this->get_all_geo_state());
				}

				imgui::Indent(6.0f);

				if (imgui::BeginTable("##geofilter_table", column_count, ImGuiTableFlags_SizingStretchSame))
				{
					for (size_t i = 0; i < _geofilters.size(); i++)
					{
						if (const auto	filter = _geofilters[i];
							filter->name)
						{
							if (imgui_filter.IsActive())
							{
								if (!imgui_filter.PassFilter(filter->name))
								{
									continue;
								}
							}

							imgui::TableNextColumn();

							imgui::PushStyleCompact();
							imgui::PushID("geofilter");

							handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->geometry_filters, filter, i);

							imgui::PopID();
							imgui::PopStyleCompact();
						}
					}

					imgui::EndTable();
				}

				imgui::EndGroup();
				geo_child_height = imgui::GetItemRectSize().y + 8.0f;
			}
		}
		//imgui::EndChild();
		imgui::PopStyleColor();
		imgui::PopStyleVar();
	}

	void filter_dialog::entity_filters(float max_width)
	{
		static float ent_child_height = 200.0f;

		const float column_width = 140.0f;
		int column_count = static_cast<int>(max_width / column_width);
		column_count = column_count < 2 ? 2 : column_count;
		column_count = column_count > 6 ? 6 : column_count;

		imgui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 1.0f));
		imgui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));

		auto min = imgui::GetCursorScreenPos();
		min.y += imgui::GetFrameHeight();

		const auto max = ImVec2(min.x + max_width, min.y + ent_child_height - imgui::GetFrameHeight());
		imgui::GetWindowDrawList()->AddRectFilled(min, max, imgui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 0.3f)), 12.0f, ImDrawFlags_RoundCornersBottom);

		//imgui::BeginChild("##geo_child", ImVec2(0.0f, ent_child_height), false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		{
			imgui::BeginGroup();
			{
				if (imgui::ButtonEx("Toggle All Entities", ImVec2(max_width, imgui::GetFrameHeight()), ImGuiButtonFlags_AlignTextBaseLine))
				{
					components::gameview::p_this->toggle_all_entities(!components::gameview::p_this->get_all_ents_state());
				}

				imgui::Indent(6.0f);

				if (imgui::BeginTable("##entityfilter_table", column_count, ImGuiTableFlags_SizingStretchSame))
				{
					for (size_t i = 0; i < _entityfilters.size(); i++)
					{
						if (const auto	filter = _entityfilters[i];
							filter->name)
						{
							if (imgui_filter.IsActive())
							{
								if (!imgui_filter.PassFilter(filter->name))
								{
									continue;
								}
							}

							imgui::TableNextColumn();

							imgui::PushStyleCompact();
							imgui::PushID("entityfilter");

							handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->entity_filters, filter, i);

							imgui::PopID();
							imgui::PopStyleCompact();
						}
					}
					imgui::EndTable();
				}
				imgui::EndGroup();
				ent_child_height = imgui::GetItemRectSize().y + 8.0f;
			}
		}
		//imgui::EndChild();
		imgui::PopStyleColor();
		imgui::PopStyleVar();
	}

	void filter_dialog::trigger_filters(float max_width)
	{
		static float trigger_child_height = 200.0f;

		const float column_width = 140.0f;
		int column_count = static_cast<int>(max_width / column_width);
		column_count = column_count < 2 ? 2 : column_count;
		column_count = column_count > 6 ? 6 : column_count;

		imgui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 1.0f));
		imgui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));

		auto min = imgui::GetCursorScreenPos();
		min.y += imgui::GetFrameHeight();

		const auto max = ImVec2(min.x + max_width, min.y + trigger_child_height - imgui::GetFrameHeight());
		imgui::GetWindowDrawList()->AddRectFilled(min, max, imgui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 0.3f)), 12.0f, ImDrawFlags_RoundCornersBottom);

		//imgui::BeginChild("##geo_child", ImVec2(0.0f, trigger_child_height), false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		{
			imgui::BeginGroup();
			{
				if (imgui::ButtonEx("Toggle All Triggers", ImVec2(max_width, imgui::GetFrameHeight()), ImGuiButtonFlags_AlignTextBaseLine))
				{
					components::gameview::p_this->toggle_all_triggers(!components::gameview::p_this->get_all_triggers_state());
				}

				imgui::Indent(6.0f);

				if (imgui::BeginTable("##triggerfilter_table", column_count, ImGuiTableFlags_SizingStretchSame))
				{
					for (size_t i = 0; i < _triggerfilters.size(); i++)
					{
						if (const auto	filter = _triggerfilters[i];
							filter->name)
						{
							if (imgui_filter.IsActive())
							{
								if (!imgui_filter.PassFilter(filter->name))
								{
									continue;
								}
							}

							imgui::TableNextColumn();

							imgui::PushStyleCompact();
							imgui::PushID("triggerfilter");

							handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->trigger_filters, filter, i);

							imgui::PopID();
							imgui::PopStyleCompact();
						}
					}
					imgui::EndTable();
				}
				imgui::EndGroup();
				trigger_child_height = imgui::GetItemRectSize().y + 8.0f;
			}
		}
		//imgui::EndChild();
		imgui::PopStyleColor();
		imgui::PopStyleVar();
	}

	void filter_dialog::other_filters(float max_width)
	{
		static float other_child_height = 200.0f;

		const float column_width = 140.0f;
		int column_count = static_cast<int>(max_width / column_width);
		column_count = column_count < 2 ? 2 : column_count;
		column_count = column_count > 6 ? 6 : column_count;

		imgui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 1.0f));
		imgui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));

		auto min = imgui::GetCursorScreenPos();
		min.y += imgui::GetFrameHeight();

		const auto max = ImVec2(min.x + max_width, min.y + other_child_height - imgui::GetFrameHeight());
		imgui::GetWindowDrawList()->AddRectFilled(min, max, imgui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 0.3f)), 12.0f, ImDrawFlags_RoundCornersBottom);

		//imgui::BeginChild("##geo_child", ImVec2(0.0f, other_child_height), false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		{
			imgui::BeginGroup();
			{
				if (imgui::ButtonEx("Toggle All Others", ImVec2(max_width, imgui::GetFrameHeight()), ImGuiButtonFlags_AlignTextBaseLine))
				{
					components::gameview::p_this->toggle_all_others(!components::gameview::p_this->get_all_others_state());
				}

				imgui::Indent(6.0f);

				if (imgui::BeginTable("##otherfilter_table", column_count, ImGuiTableFlags_SizingStretchSame))
				{
					for (size_t i = 0; i < _otherfilters.size(); i++)
					{
						if (const auto	filter = _otherfilters[i];
							filter->name)
						{
							if (imgui_filter.IsActive())
							{
								if (!imgui_filter.PassFilter(filter->name))
								{
									continue;
								}
							}

							imgui::TableNextColumn();

							imgui::PushStyleCompact();
							imgui::PushID("otherfilter");

							handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->other_filters, filter, i);

							imgui::PopID();
							imgui::PopStyleCompact();
						}
					}
					imgui::EndTable();
				}
				imgui::EndGroup();
				other_child_height = imgui::GetItemRectSize().y + 8.0f;
			}
		}
		//imgui::EndChild();
		imgui::PopStyleColor();
		imgui::PopStyleVar();
	}

	bool filter_dialog::gui()
	{
		if (dvars::gui_toolbox_integrate_filter && dvars::gui_toolbox_integrate_filter->current.enabled)
		{
			return false;
		}

		const auto MIN_WINDOW_SIZE = ImVec2(360.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		imgui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		imgui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		imgui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!imgui::Begin("Filters##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			imgui::End();
			return false;
		}

		window_hovered = imgui::IsWindowHovered();

		const auto pre_filter_pos = imgui::GetCursorScreenPos();

		imgui_filter.Draw("#filter_filter", imgui::GetContentRegionAvail().x);
		input_focused = imgui::IsItemFocused();

		const auto post_filter_pos = imgui::GetCursorScreenPos();

		if (!imgui_filter.IsActive())
		{
			imgui::SetCursorScreenPos(ImVec2(pre_filter_pos.x + (imgui::GetContentRegionAvail().x * 0.48f), pre_filter_pos.y + 4.0f));
			imgui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
			imgui::TextUnformatted("Filter");
			imgui::PopStyleColor();
			imgui::SetCursorScreenPos(post_filter_pos);
		}

		SPACING(0.0f, 4.0f);

		int sflags = 0;
		imgui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f); sflags++;
		imgui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f); sflags++;

		const auto button_width = imgui::GetContentRegionAvail().x;

		if (imgui::TreeNodeEx("Geometry Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			filter_dialog::geometry_filters(button_width);
			imgui::TreePop();
		}

		SPACING(0.0f, 4.0f);

		if (imgui::TreeNodeEx("Entity Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			filter_dialog::entity_filters(button_width);
			imgui::TreePop();
		}

		SPACING(0.0f, 4.0f);

		if (imgui::TreeNodeEx("Trigger Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			filter_dialog::trigger_filters(button_width);
			imgui::TreePop();
		}

		SPACING(0.0f, 4.0f);

		if (imgui::TreeNodeEx("Other Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			filter_dialog::other_filters(button_width);
			imgui::TreePop();
		}

		SPACING(0.0f, 4.0f);

		imgui::PopStyleVar(sflags);
		imgui::End();
		return true;
	}

	// CMainFrame::OnFilterDlg
	void filter_dialog::on_filterdialog_command()
	{
		if (dvars::gui_toolbox_integrate_filter && dvars::gui_toolbox_integrate_filter->current.enabled)
		{
			const auto tb = GET_GUI(ggui::toolbox_dialog);

			tb->set_bring_to_front(true);
			tb->focus_child(toolbox_dialog::TB_CHILD::FILTER);
			tb->open();
		}
		else
		{
			const auto gui = GET_GUI(ggui::filter_dialog);
			gui->needs_window_hovered_once = true;

			if (gui->is_inactive_tab() && gui->is_active())
			{
				gui->set_bring_to_front(true);
				return;
			}

			gui->toggle();
		}
	}

	void filter_dialog::on_init()
	{
		build_radiant_filterlists();
	}

	void filter_dialog::hooks()
	{
		// disable filter tab insertion in entitywnd :: CTabCtrl::InsertItem(&g_wndTabsEntWnd, 1u, 4u, "&Filters", 0, 0);
		utils::hook::nop(0x4966A5, 23);

		// hide filter window on startup
		utils::hook::set<BYTE>(0x422596 + 1, 0x0);

		// no parent filterwnd
		utils::hook::nop(0x422583, 2);
		utils::hook::nop(0x42258A, 6);

		// detour CMainFrame::OnFilterDlg (hotkey to open the original dialog)
		utils::hook::detour(0x42B7A0, filter_dialog::on_filterdialog_command, HK_JUMP);
	}

	REGISTER_GUI(filter_dialog);
}
