#include "std_include.hpp"

namespace ggui::filter
{
	typedef void(__thiscall* CCheckListBox__SetCheck_t)(CCheckListBox*, int nIndex, int nCheck);
	auto CCheckListBox__SetCheck = reinterpret_cast<CCheckListBox__SetCheck_t>(0x599D88);

	typedef int(__thiscall* CCheckListBox__GetCheck_t)(CCheckListBox*, int nIndex);
	auto CCheckListBox__GetCheck = reinterpret_cast<CCheckListBox__GetCheck_t>(0x598A0F);

	std::vector<game::filter_entry_s*> _geofilters, _entityfilters, _triggerfilters, _otherfilters;
	bool _geofilters_initialized, _entityfilters_initialized, _triggerfilters_initialized, _otherfilters_initialized;

	ImGuiTextFilter	imgui_filter;
	
	void build_radiant_filterlists()
	{
		// *
		if (!_geofilters_initialized)
		{
			for (auto filter = game::g_qeglobals->d_filterGlobals_geometryFilters;
					  filter;
					  filter = filter->next_filter)
			{
				if (!filter->name) {
					break;
				}

				_geofilters.push_back(filter);
			}

			std::reverse(_geofilters.begin(), _geofilters.end());
			_geofilters_initialized = true;
		}

		// *
		if(!_entityfilters_initialized)
		{
			for (auto filter = game::g_qeglobals->d_filterGlobals_entityFilters;
					  filter;
					  filter = filter->next_filter)
			{
				if (!filter->name) {
					break;
				}

				_entityfilters.push_back(filter);
			}

			std::reverse(_entityfilters.begin(), _entityfilters.end());
			_entityfilters_initialized = true;
		}

		// *
		if (!_triggerfilters_initialized)
		{
			for (auto filter = game::g_qeglobals->d_filterGlobals_triggerFilters;
					  filter;
					  filter = filter->next_filter)
			{
				if (!filter->name) {
					break;
				}

				_triggerfilters.push_back(filter);
			}

			std::reverse(_triggerfilters.begin(), _triggerfilters.end());
			_triggerfilters_initialized = true;
		}

		// *
		if (!_otherfilters_initialized)
		{
			for (auto filter = game::g_qeglobals->d_filterGlobals_otherFilters;
					  filter;
					  filter = filter->next_filter)
			{
				if (!filter->name) {
					break;
				}

				_otherfilters.push_back(filter);
			}

			std::reverse(_otherfilters.begin(), _otherfilters.end());
			_otherfilters_initialized = true;
		}
	}

	// *
	// bridges imgui and original radiant filter window
	void handle_radiant_filter(CCheckListBox* checklist, game::filter_entry_s* filter, int index)
	{
		const int checked = CCheckListBox__GetCheck(checklist, index);
		if (ImGui::RadioButton(filter->name, checked))
		{
			if (checked)
			{
				CCheckListBox__SetCheck(checklist, index, 0);
				filter->isShown = 0;
				
				if (!((filter->filter_type_enum & game::FILTER_TYPE2::FACE) == 0))
				{
					// RadiantFilters05_large
					utils::hook::call<void(__cdecl)(game::filter_entry_s*)>(0x411BE0)(filter);
				}
			}
			else
			{
				CCheckListBox__SetCheck(checklist, index, 1);
				filter->isShown = 1;
				
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
	bool toggle_by_name(const char* name, E_FILTERS filter_type, bool on_off)
	{
		build_radiant_filterlists();

		bool return_val = false;

		CCheckListBox* checklistbox;
		size_t filter_list_count = 0;
		
		std::string filter_name = name;
					filter_name = utils::str_to_lower(filter_name);
		
		switch(filter_type)
		{
		case GEOMETRY:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->geometry_filters;
			filter_list_count = ggui::filter::_geofilters.size();
			break;
			
		case ENTITY:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->entity_filters;
			filter_list_count = ggui::filter::_entityfilters.size();
			break;
			
		case TRIGGER:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->trigger_filters;
			filter_list_count = ggui::filter::_triggerfilters.size();
			break;

		case OTHER:
			checklistbox = &cmainframe::activewnd->m_pFilterWnd->other_filters;
			filter_list_count = ggui::filter::_otherfilters.size();
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
				filter = ggui::filter::_geofilters[i];
				break;

			case ENTITY:
				filter = ggui::filter::_entityfilters[i];
				break;

			case TRIGGER:
				filter = ggui::filter::_triggerfilters[i];
				break;

			case OTHER:
				filter = ggui::filter::_otherfilters[i];
				break;

			default:
				return false;
			}
			
			if (filter && filter->name)
			{
				
				if(utils::str_to_lower(filter->name) == filter_name)
				{
					return_val = filter->isShown;
					
					if (!on_off && filter->isShown)
					{
						CCheckListBox__SetCheck(checklistbox, i, 0);
						filter->isShown = false;

						if (!((filter->filter_type_enum & game::FILTER_TYPE2::FACE) == 0))
						{
							// RadiantFilters05_large
							utils::hook::call<void(__cdecl)(game::filter_entry_s*)>(0x411BE0)(filter);
						}
					}
					else if(on_off && !filter->isShown)
					{
						CCheckListBox__SetCheck(checklistbox, i, 1);
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

	bool input_focused = false;
	bool window_hovered = false;

	bool needs_window_hovered_once = false;
	
	// *
	// imgui filter window
	void menu(ggui::imgui_context_menu& menu)
	{
		build_radiant_filterlists();
		
		const auto MIN_WINDOW_SIZE = ImVec2(360.0f, 200.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(400.0f, 800.0f);

		ImGui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		
		ImGui::Begin("Filters##window", &menu.menustate, ImGuiWindowFlags_NoCollapse);
		window_hovered = ImGui::IsWindowHovered();
		
		const auto pre_filter_pos = ImGui::GetCursorScreenPos();

		imgui_filter.Draw("#filter_filter", ImGui::GetContentRegionAvailWidth());
		input_focused = ImGui::IsItemFocused();

		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0) && !ImGui::IsWindowAppearing())
		{
			if(needs_window_hovered_once && window_hovered)
			{
				needs_window_hovered_once = false;
			}
			else if(window_hovered && !needs_window_hovered_once)
			{
				ImGui::SetKeyboardFocusHere(-1);
			}
		}
		
		const auto post_filter_pos = ImGui::GetCursorScreenPos();
		
		if(!imgui_filter.IsActive())
		{
			ImGui::SetCursorScreenPos(ImVec2(pre_filter_pos.x + (ImGui::GetContentRegionAvailWidth() * 0.48f), pre_filter_pos.y + 4.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 0.6f));
			ImGui::TextUnformatted("Filter");
			ImGui::PopStyleColor();
			ImGui::SetCursorScreenPos(post_filter_pos);
		}
		
		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersOuter;
	
		const float column_width = 140.0f;
		int column_count = static_cast<int>(ImGui::GetWindowContentRegionMax().x / column_width);
			column_count = column_count < 2 ? 2 : column_count;
			column_count = column_count > 6 ? 6 : column_count;

		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
		if(ImGui::TreeNodeEx("Geometry Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("##geofilter_table", column_count, flags))
			{
				for (size_t i = 0; i < _geofilters.size(); i++)
				{
					if (const auto filter = _geofilters[i];
						filter->name)
					{
						if (imgui_filter.IsActive())
						{
							if (!imgui_filter.PassFilter(filter->name)) {
								continue;
							}
						}
						
						ImGui::TableNextColumn();

						ImGui::PushStyleCompact();
						ImGui::PushID("geofilter");
						
						handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->geometry_filters, filter, i);
						
						ImGui::PopID();
						ImGui::PopStyleCompact();
					}
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Entity Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("##entityfilter_table", column_count, flags))
			{
				for (size_t i = 0; i < _entityfilters.size(); i++)
				{
					if (const auto filter = _entityfilters[i];
						filter->name)
					{
						if (imgui_filter.IsActive())
						{
							if (!imgui_filter.PassFilter(filter->name)) {
								continue;
							}
						}

						ImGui::TableNextColumn();
						
						ImGui::PushStyleCompact();
						ImGui::PushID("entityfilter");
						
						handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->entity_filters, filter, i);
						
						ImGui::PopID();
						ImGui::PopStyleCompact();
					}
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Trigger Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("##triggerfilter_table", column_count, flags))
			{
				for (size_t i = 0; i < _triggerfilters.size(); i++)
				{
					if (const auto filter = _triggerfilters[i];
						filter->name)
					{
						if (imgui_filter.IsActive())
						{
							if (!imgui_filter.PassFilter(filter->name)) {
								continue;
							}
						}

						ImGui::TableNextColumn();
						
						ImGui::PushStyleCompact();
						ImGui::PushID("triggerfilter");

						handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->trigger_filters, filter, i);

						ImGui::PopID();
						ImGui::PopStyleCompact();
					}
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Other Filters", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginTable("##otherfilter_table", column_count, flags))
			{
				for (size_t i = 0; i < _otherfilters.size(); i++)
				{
					if (const auto filter = _otherfilters[i];
						filter->name)
					{
						if (imgui_filter.IsActive())
						{
							if (!imgui_filter.PassFilter(filter->name)) {
								continue;
							}
						}

						ImGui::TableNextColumn();
						
						ImGui::PushStyleCompact();
						ImGui::PushID("otherfilter");

						handle_radiant_filter(&cmainframe::activewnd->m_pFilterWnd->other_filters, filter, i);

						ImGui::PopID();
						ImGui::PopStyleCompact();
					}
				}
				ImGui::EndTable();
			}
			ImGui::TreePop();
		}
		
		ImGui::PopStyleVar();
		ImGui::End();
	}

	// *
	// *

	// CMainFrame::OnFilterDlg
	void on_filterdialog_command()
	{
		needs_window_hovered_once = true;
		components::gui::toggle(ggui::state.czwnd.m_filter, 0, true);
	}

	void hooks()
	{
		// disable filter tab insertion in entitywnd :: CTabCtrl::InsertItem(&g_wndTabsEntWnd, 1u, 4u, "&Filters", 0, 0);
		utils::hook::nop(0x4966A5, 23);

		// hide filter window on startup
		utils::hook::set<BYTE>(0x422596 + 1, 0x0);
		
		// no parent filterwnd
		utils::hook::nop(0x422583, 2);
		utils::hook::nop(0x42258A, 6);

		// detour CMainFrame::OnFilterDlg (hotkey to open the original dialog)
		utils::hook::detour(0x42B7A0, on_filterdialog_command, HK_JUMP);
	}
}
