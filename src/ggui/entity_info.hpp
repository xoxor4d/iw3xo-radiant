#pragma once

namespace ggui
{
	class entity_info final : public ggui::ggui_module
	{
	public:
		entity_info() { set_gui_type(GUI_TYPE_DEF); }


		// *
		// public member functions

		bool gui() override;
		void on_init() override;
		void on_open() override;
		void on_close() override;

		void update_entity_list();
		void render_hovered();

		struct entity_list_s
		{
			std::string class_name;
			std::vector<game::entity_inst_s*> entities;

			entity_list_s(const std::string& name, game::entity_inst_s* ent)
			{
				class_name = name;
				entities.emplace_back(ent);
			}
		};

		bool m_update_on_unselect = false;
		game::selbrush_def_t* hovered_entity = nullptr;
		std::vector<entity_list_s> entity_list;
	};
}
