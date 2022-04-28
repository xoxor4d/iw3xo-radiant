#pragma once

namespace ggui
{
	class grid_dialog final : public ggui::ggui_module
	{
		struct eclass_context_ent
		{
			std::string token;
			std::string ent_str;
		};

		struct eclass_context_group
		{
			std::string group_name;
			std::string path_including_group;
			std::vector<eclass_context_group> childs;
			std::vector<eclass_context_ent> ents;
		};

		eclass_context_group* eclass_ctr_curr;
		eclass_context_group* eclass_ctr_prev;

	public:
		std::vector<eclass_context_group> eclass_context_groups; // grouped entities
		std::vector<eclass_context_ent> eclass_context_ents; // single ungrouped entities

		grid_dialog()
		{
			set_gui_type(GUI_TYPE_RTT);
			this->open(); // done so that on_open fires

			eclass_ctr_curr = nullptr;
			eclass_ctr_prev = nullptr;
		} // render-to-texture gui


		// *
		// public member functions

		void grid_gui();

		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related

		// *
		// init

		void	hooks();

	private:
		void build_eclass_context_new();
		void context_menu();
		void drag_drop_target();
	};
}
