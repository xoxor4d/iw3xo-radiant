#include "std_include.hpp"

namespace ggui::toolbar
{
	void menu(ggui::imgui_context_menu& menu)
	{
		int _stylevars = 0; int _stylecolors = 0;
		
		// *
		// create toolbar window

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 4.0f));		_stylevars++;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));	_stylevars++;

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1, 1, 1, 0));					_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(100, 100, 100, 70));	_stylecolors++;
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(100, 100, 100, 70));		_stylecolors++;

		ImGui::Begin("toolbar_xywnd", nullptr,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar
		);


		// *
		// sizes (save current window position and size)

		menu.position[0] = ImGui::GetWindowPos().x;
		menu.position[1] = ImGui::GetWindowPos().y;
		menu.size[0] = ImGui::GetWindowSize().x;
		menu.size[1] = ImGui::GetWindowSize().y;


		// *
		// gui elements

		const ImVec2 toolbar_imagebutton_size = ImVec2(28.0f, 28.0f);

		// lock x
		if (const auto image = game::Image_RegisterHandle("lock_x"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow == 2 || game::g_nScaleHow == 4 || game::g_nScaleHow == 6) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0))
			{
				mainframe_thiscall(LRESULT, 0x428BC0); // cmainframe::OnScalelockX
			} TT("lock grid along the x-axis");
		}


		// lock y
		if (const auto image = game::Image_RegisterHandle("lock_y"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow == 1 || game::g_nScaleHow == 4 || game::g_nScaleHow == 5) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0)) 
			{
				mainframe_thiscall(LRESULT, 0x428B60); // cmainframe::OnScalelockY
			} TT("lock grid along the y-axis");
		}


		// lock z
		if (const auto image = game::Image_RegisterHandle("lock_z"); image)
		{
			ImVec2 uv0 = (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3) ? ImVec2(0.5f, 0.0f) : ImVec2(0.0f, 0.0f);
			ImVec2 uv1 = (game::g_nScaleHow > 0 && game::g_nScaleHow <= 3) ? ImVec2(1.0f, 1.0f) : ImVec2(0.5f, 1.0f);

			ImGui::SameLine();
			if (ImGui::ImageButton(image->texture.data, toolbar_imagebutton_size, uv0, uv1, 0)) 
			{
				mainframe_thiscall(LRESULT, 0x428B90); // cmainframe::OnScalelockZ
			} TT("lock grid along the z-axis");
		}

		ImGui::SameLine();
		if (ImGui::Button("Reload Images")) { game::R_ReloadImages(); }

		// TODO! - remove me
		/*ImGui::SameLine();
		if (ImGui::Button("Lock X")) { mainframe_thiscall(LRESULT, 0x428BC0); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Y")) { mainframe_thiscall(LRESULT, 0x428B60); }

		ImGui::SameLine();
		if (ImGui::Button("Lock Z")) { mainframe_thiscall(LRESULT, 0x428B90); }*/


		// TODO! - remove me
		/*ImGui::SameLine();
		if (ImGui::Button("Switch Console <-> Splitter"))
		{
			const auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);

			const auto pTop = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 0, 0);
			const auto pBottom = afx::CSplitterWnd__GetPane(&cmainframe::activewnd->m_wndSplit, 1, 0);

			if (!pTop || !pBottom)
			{
				goto END_GUI;
			}

			const auto _top = pTop->m_hWnd;
			const auto idTop = GetWindowLongA(_top, GWL_ID);

			const auto _bottom = pBottom->m_hWnd;
			const auto idBottom = GetWindowLongA(_bottom, GWL_ID);

			SetWindowLongA(_top, GWL_ID, idBottom);
			SetWindowLongA(_bottom, GWL_ID, idTop);

			vtable->RecalcLayout(&cmainframe::activewnd->m_wndSplit);
		}

		ImGui::SameLine();
		if (ImGui::Button("Hide Console"))
		{
			const auto vtable = reinterpret_cast<CSplitterWnd_vtbl*>(cmainframe::activewnd->m_wndSplit.__vftable);
			vtable->DeleteRow(&cmainframe::activewnd->m_wndSplit, 1);
		}*/

		// TODO! - remove me
		/*ImGui::SameLine();
		if (ImGui::Button("Set Statustext"))
		{
			const auto vtable = reinterpret_cast<CStatusBar_vtbl*>(cmainframe::activewnd->m_wndStatusBar.__vftable);
			vtable->SetStatusText(&cmainframe::activewnd->m_wndStatusBar, 0x75);
		}*/

		/*ImGui::SameLine();
		if (ImGui::Button("Toggle Toolbar"))
		{
			typedef void(__thiscall* CFrameWnd_ShowControlBar_t)(CFrameWnd*, CControlBar*, BOOL bShow, BOOL bDelay);
			CFrameWnd_ShowControlBar_t CFrameWnd_ShowControlBar = reinterpret_cast<CFrameWnd_ShowControlBar_t>(0x59E9DD);

			auto vtable = reinterpret_cast<CToolBar_vtbl*>(cmainframe::activewnd->m_wndToolBar.__vftable);
			CFrameWnd_ShowControlBar(cmainframe::activewnd, &cmainframe::activewnd->m_wndToolBar, vtable->IsVisible(&cmainframe::activewnd->m_wndToolBar) ? 0 : 1, 1);
		}*/

		/*ImGui::SameLine();
		if (ImGui::Button("Toggle Menubar"))
		{
			if (!ggui::mainframe_menubar_enabled)
			{
				components::command::execute("menubar_show");
			}
			else
			{
				components::command::execute("menubar_hide");
			}

			game::CPrefsDlg_SavePrefs();
		}*/

		//ImGui::SameLine();
		//if (ImGui::Button("Reload Commandmap"))
		//{
		//	// CMainFrame::LoadCommandMap
		//	cdeclcall(void, 0x421230);
		//}

	//END_GUI:
		ImGui::PopStyleColor(_stylecolors);
		ImGui::PopStyleVar(_stylevars);
		ImGui::End();
	}
}
