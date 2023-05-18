#include "std_include.hpp"
#include <comdef.h>
#include <commctrl.h>

namespace ggui
{

	// CLayersDlg__vtable
	// CLayer_COMMANDMAP
	// CLayerFunctions

	std::string get_layer_name(const WCHAR* data)
	{
		USES_CONVERSION;
		return W2A(data);
	}

	bool layer_context_open = false;
	bool layer_context_pending = false;
	const clayer_s* context_layer = nullptr;
	const clayer_s* context_layer_last_hovered = nullptr;

	void layer_dialog::do_context_menu()
	{
		if (layer_context_open || layer_context_pending)
		{
			ggui::context_menu_style_begin();

			if (!imgui::IsKeyPressed(ImGuiKey_Escape) && imgui::BeginPopupContextItem("context_menu##layerdlg"))
			{
				// select layer in og layerwindow
				TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, context_layer);

				layer_context_open = true;
				layer_context_pending = false;

				if (imgui::MenuItem("Add new layer as child"))
				{
					// CLayers::NewLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C690)(game::layer_dlg);
				}

				if (imgui::MenuItem("Set as active layer"))
				{
					// CLayers::SetActiveLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C610)(game::layer_dlg);
				}

				imgui::Separator();

				// same logic as CXYWnd::OnSelectionAddToActiveLayer (grid context menu)
				if (imgui::MenuItem("Add selection to layer"))
				{
					// CLayersDlg::AsignSelectionToLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C850)(game::layer_dlg);
				}

				if (imgui::MenuItem("Select everything within layer"))
				{
					// CLayersDlg::SelectAllBrushesInLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C960)(game::layer_dlg);
				}

				imgui::Separator();

				if (imgui::MenuItem("Reset layer and children"))
				{
					// CLayersDlg::OnShowLayerAndChildren
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CAF0)(game::layer_dlg);
				}

				imgui::Separator();

				if (imgui::MenuItem("Rename layer"))
				{
					// CLayersDlg::RenameLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CC60)(game::layer_dlg);
				}

				if (imgui::MenuItem("Delete layer"))
				{
					// CLayersDlg::DeleteLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CBE0)(game::layer_dlg);
				}

				imgui::EndPopup();
			}
			else
			{
				layer_context_open = false;
				layer_context_pending = false;
			}

			ggui::context_menu_style_end();
		}
	}

	void handle_context_menu(const clayer_s* layer)
	{
		context_layer_last_hovered = layer;

		if (imgui::IsMouseReleased(ImGuiMouseButton_Right) && !layer_context_pending)
		{
			layer_context_pending = true;
			context_layer = layer;
		}
	}

	void do_layer(const clayer_s* layer, bool is_sibling = false)
	{
		if (!layer)
		{
			return;
		}

		imgui::TableNextRow();
		imgui::TableSetColumnIndex(0);
		imgui::AlignTextToFramePadding(); // adds height to rows

		int pushed_colors = 0;
		const bool is_active_layer = layer == game::layer_dlg->active_layer;
		const bool is_hidden = layer->enabled == 32;
		const bool is_frozen = layer->enabled == 48;

		if (is_active_layer)
		{
			//imgui::PushFontFromIndex(E_FONT::BOLD_18PX);
			imgui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.6f, 1.0f)); pushed_colors++;
		}
		else if (is_hidden)
		{
			imgui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); pushed_colors++;
		}
		else if (is_frozen)
		{
			imgui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f)); pushed_colors++;
		}

		const std::string layer_string_std = get_layer_name(layer->layer_string);
		const char* layer_string = is_active_layer
					? utils::va("%s   %s", layer_string_std.c_str(), ICON_FA_LAYER_GROUP)
					: utils::va("%s   ", layer_string_std.c_str()); // add a bit of padding to the furthest layer

		// do not show arrow on layers with no children
		const bool is_last_node = !layer->child_layers;

		bool is_hovered = false;
		bool is_pressed = false;

		const auto do_hide_and_freeze = [&]() -> void
		{
			if (is_active_layer)
			{
				imgui::PopStyleColor(pushed_colors);
				pushed_colors = 0;
			}

			// ----

			imgui::TableSetColumnIndex(1);
			imgui::SetCursorPosX(imgui::GetCursorPosX() + 12.0f);

			imgui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 5.0f));
			const ImVec2 btn_size = ImVec2(imgui::GetFrameHeight() + 6.0f, imgui::GetFrameHeight());

			if (imgui::Button(is_hidden ? ICON_FA_EYE_SLASH : ICON_FA_EYE, btn_size))
			{
				if (is_hidden)
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					// CLayersDlg::OnShowLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CAC0)(game::layer_dlg);
				}
				else
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					if (is_last_node)
					{
						// CLayersDlg::OnHideLayer
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CA60)(game::layer_dlg);
					}
					else
					{
						// CLayersDlg::OnHideLayerAndChildren
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CA90)(game::layer_dlg);
					}

				}
			}

			if (imgui::IsItemHovered(ImGuiHoveredFlags_None))
			{
				handle_context_menu(layer);
			}

			imgui::SameLine(0.0f, 6.0f);

			if (imgui::Button(ICON_FA_SNOWFLAKE, btn_size))
			{
				if (is_frozen)
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					// CLayersDlg::OnShowLayer
					utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CAC0)(game::layer_dlg);
				}
				else
				{
					TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

					if (is_last_node)
					{
						// CLayersDlg::OnFreezeLayer
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CB20)(game::layer_dlg);
					}
					else
					{
						// CLayersDlg::OnFreezeLayerAndChildren
						utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41CB50)(game::layer_dlg);
					}
				}
			}

			if (imgui::IsItemHovered(ImGuiHoveredFlags_None))
			{
				handle_context_menu(layer);
			}

			imgui::PopStyleVar(); // ImGuiStyleVar_FramePadding

			if (pushed_colors)
			{
				imgui::PopStyleColor(pushed_colors);
				pushed_colors = 0;
			}
		};

		// current layer (custom TreeNodeEx that returns hovered and pressed state - for context menu and bridging to og layerwindow)
		if (imgui::TreeNodeEx(layer_string, &is_hovered, &is_pressed, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (is_last_node ? (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet) : 0)))
		{
			do_hide_and_freeze();

			// ----

			// do all its child layers child siblings
			if (layer->child_layers)
			{
				do_layer(layer->child_layers, false);
			}

			imgui::TreePop();
		}
		else // if node is collapsed
		{
			do_hide_and_freeze();
		}

		if (pushed_colors)
		{
			imgui::PopStyleColor(pushed_colors);
			pushed_colors = 0;
		}

		if (is_pressed)
		{
			// select layer in og layerwindow
			TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);
		}

		// IsItemHovered does not work here because do_layers is recursive
		if (is_hovered)
		{
			handle_context_menu(layer);
			//game::printf_to_console("Hovered %s", get_layer_name(layer->layer_string).c_str());

			if (imgui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				TreeView_SelectItem(game::layer_dlg->tree.m_hWnd, layer);

				// CLayers::SetActiveLayer
				utils::hook::call<void(__fastcall)(CLayerDlg*)>(0x41C610)(game::layer_dlg);
			}
		}
		
		// do all siblings for a child
		if (!is_sibling)
		{
			for (auto sibling = layer->sibling_layer_same_level; sibling; sibling = sibling->sibling_layer_same_level)
			{
				do_layer(sibling, true);
			}
		}
	}

	void layer_dialog::do_table()
	{
		if (imgui::BeginTable("split", 2, ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoSavedSettings /*| ImGuiTableFlags_Borders*/))
		{
			imgui::TableSetupColumn("##cell_layer_desc", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 240.0f);
			imgui::TableSetupColumn("##cell_buttons", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, 90.0f);

			imgui::TableNextRow();
			imgui::TableSetColumnIndex(0);
			//imgui::AlignTextToFramePadding();

			//if (imgui::TreeNodeEx(get_layer_name(game::layer_dlg->root_node_the_map->layer_string).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth))
			{
				// do all siblings of the first child (same level as the first use-able layer)
				for (auto child = game::layer_dlg->root_node_the_map->child_layers; child; child = child->sibling_layer_same_level)
				{
					do_layer(child, true);
				}

				//imgui::TreePop();
			}

			imgui::EndTable();
		}
	}

	bool layer_dialog::gui()
	{
		if (dvars::gui_toolbox_integrate_layers && dvars::gui_toolbox_integrate_layers->current.enabled)
		{
			return false;
		}

		imgui::SetNextWindowSize(ImVec2(400.0f, 600.0f), ImGuiCond_FirstUseEver);
		imgui::SetNextWindowPos(ggui::get_initial_window_pos(), ImGuiCond_FirstUseEver);
		//imgui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

		if (!imgui::Begin("Layers##window", this->get_p_open(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar))
		{
			//imgui::PopStyleColor();
			imgui::End();
			return false;
		}

		SPACING(0.0f, 2.0f);

		do_table();
		do_context_menu();

		//imgui::PopStyleColor();
		imgui::End();
		return true;
	}

	// CMainFrame::OnLayersDlg
	void layer_dialog::on_layerdialog_command()
	{
		if (dvars::gui_toolbox_integrate_layers && dvars::gui_toolbox_integrate_layers->current.enabled)
		{
			const auto tb = GET_GUI(ggui::toolbox_dialog);

			tb->set_bring_to_front(true);
			tb->focus_child(toolbox_dialog::TB_CHILD::LAYERS);
			tb->open();
		}
		else
		{
			const auto gui = GET_GUI(ggui::layer_dialog);
			if (gui->is_inactive_tab() && gui->is_active())
			{
				gui->set_bring_to_front(true);
				return;
			}

			gui->toggle();
		}
	}

	void layer_dialog::on_open()
	{ }

	void layer_dialog::on_close()
	{ }

	void layer_dialog::hooks()
	{
		// detour CMainFrame::OnLayersDlg (hotkey to open the original dialog)
		utils::hook::detour(0x42BD10, layer_dialog::on_layerdialog_command, HK_JUMP);
	}

	REGISTER_GUI(layer_dialog);
}
