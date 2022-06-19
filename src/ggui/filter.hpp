#pragma once

namespace ggui
{
	class filter_dialog final : public ggui::ggui_module
	{
		ImGuiTextFilter	imgui_filter;

		bool input_focused;
		bool window_hovered;
		bool needs_window_hovered_once;

		bool _geofilters_initialized;
		bool _entityfilters_initialized;
		bool _triggerfilters_initialized;
		bool _otherfilters_initialized;


	public:
		enum E_FILTERS
		{
			GEOMETRY,
			ENTITY,
			TRIGGER,
			OTHER,
		};

		std::vector<game::filter_entry_s*> _geofilters, _entityfilters, _triggerfilters, _otherfilters;

		filter_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);

			input_focused = false;
			window_hovered = false;
			needs_window_hovered_once = false;
			_geofilters_initialized = false;
			_entityfilters_initialized = false;
			_triggerfilters_initialized = false;
			_otherfilters_initialized = false;
		}

		// *
		// public member functions

		bool gui() override;
		bool toggle_by_name(const char* name, E_FILTERS filter_type, bool on_off);
		void build_radiant_filterlists();

		// *
		// asm related

		static void on_filterdialog_command(); // not a callable method

		// *
		// init

		void hooks();


	private:
		void handle_radiant_filter(CCheckListBox* checklist, game::filter_entry_s* filter, int index);
	};
}

