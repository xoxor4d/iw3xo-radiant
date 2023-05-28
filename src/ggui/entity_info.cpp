#include "std_include.hpp"

namespace ggui
{
	void entity_info::render_hovered()
	{
		if (hovered_entity)
		{
			const auto mins = hovered_entity->def->mins;
			const auto maxs = hovered_entity->def->maxs;

			components::renderer::get()->add_debug_box(game::vec3_origin, mins, maxs, 0.0f, 20.0f, true, false);

			// highlighting an entity this way creates issues when hovering an entity which is currently hidden (either via hidden layer or manual hide)
			// it will stay invisible until manually re-selected

			//game::GfxColor col_packed = {};
			//game::vec4_t col = { 1.0f, 0.0f, 0.0f, 1.0f };
			//game::Byte4PackPixelColor(col, &col_packed);
			//char zero[4] = {};

			//if (hovered_entity->cullFlag)
			//{
			//	// draw brush
			//	utils::hook::call<void(__cdecl)(game::selbrush_def_t* b, float* ident_mtx, int patch_related, int a4, int draw_meth1, int checkhandle, int draw_meth2, game::GfxColor* color, int selection_line_width, int drawFlags, char* a11)>(0x47AFC0)
			//		(hovered_entity, game::vec3_origin, -1, 0, 4, 0, 4, &col_packed, 2, 1, zero);
			//}
		}
	}

	void entity_info::update_entity_list()
	{
		entity_list.clear();
		entity_list.reserve(128);

		FOR_ALL_ENTITY_INSTS(e)
		{
			if (e && e->ent && e->ent->eclass)
			{
				bool class_exists = false;
				for (auto& it : entity_list)
				{
					if (e->ent->eclass->name == it.class_name)
					{
						it.entities.emplace_back(e);
						class_exists = true;
						break;
					}
				}

				if (!class_exists)
				{
					// new entity class
					entity_list.emplace_back(e->ent->eclass->name, e);
				}
			}
		}
	}

	bool entity_info::gui()
	{
		const auto MIN_WINDOW_SIZE = ImVec2(256.0f, 300.0f);
		const auto INITIAL_WINDOW_SIZE = ImVec2(256.0f, 400.0f);

		imgui::SetNextWindowSizeConstraints(MIN_WINDOW_SIZE, ImVec2(FLT_MAX, FLT_MAX));
		imgui::SetNextWindowSize(INITIAL_WINDOW_SIZE, ImGuiCond_FirstUseEver);
		imgui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);

		if (!imgui::Begin("Entity Info##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse))
		{
			imgui::End();
			return false;
		}

		// #

		SPACING(0.0f, 2.0f);
		imgui::Indent(8.0f);

		if (m_update_on_unselect && !game::is_any_brush_selected())
		{
			m_update_on_unselect = false;
			update_entity_list();
		}

		if (!entity_list.empty())
		{
			bool any_item_hovered = false;
			int node_count = 0;

			for (const auto& it : entity_list)
			{
				imgui::PushID(node_count);
				if (imgui::TreeNode(it.class_name.c_str()))
				{
					int ent_count = 0;

					for (const auto e : it.entities) // cant be a const& because 'Select_Deselect' can invalidate the pointer
					{
						imgui::PushID(ent_count);
						if (imgui::Selectable(it.class_name.c_str()))
						{
							// calling this destroys invalidates 'e' when e is a const& (because lambda func?)
							game::Select_Deselect(true);

							auto onext = e->def.ownerNext;

							for (const auto i = &e->def; onext != i; onext = onext->ownerNext)
							{
								game::Brush_Select(onext, true, false, true);
								//components::command::execute("center_camera_on_selection");
							}
						}

						if (imgui::IsItemHovered(ImGuiHoveredFlags_None))
						{
							hovered_entity = e->def.ownerNext;
							any_item_hovered = true;
						}
						imgui::PopID();
						ent_count++;

					}

					imgui::TreePop();
				}

				imgui::PopID();
				node_count++;
			}

			if (!any_item_hovered)
			{
				hovered_entity = nullptr;
			}
		}

		imgui::End();
		return true;
	}

	void entity_info::on_init()
	{ }

	void entity_info::on_open()
	{
		update_entity_list();
	}

	void entity_info::on_close()
	{ }

	REGISTER_GUI(entity_info);
}
