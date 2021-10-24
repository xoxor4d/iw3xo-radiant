#include "std_include.hpp"

ccamwnd* ccamwnd::activewnd;
IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define RTT_CAMERA_USE_PADDING

void mtx4x4_mul(game::GfxMatrix* mtx_out, game::GfxMatrix* a, game::GfxMatrix* b)
{
	mtx_out->m[0][0] = a->m[0][0] * b->m[0][0] + a->m[0][1] * b->m[1][0] + b->m[2][0] * a->m[0][2] + b->m[3][0] * a->m[0][3];
	mtx_out->m[0][1] = b->m[1][1] * a->m[0][1] + a->m[0][0] * b->m[0][1] + b->m[2][1] * a->m[0][2] + a->m[0][3] * b->m[3][1];
	mtx_out->m[0][2] = b->m[1][2] * a->m[0][1] + b->m[0][2] * a->m[0][0] + a->m[0][2] * b->m[2][2] + a->m[0][3] * b->m[3][2];
	mtx_out->m[0][3] = b->m[1][3] * a->m[0][1] + a->m[0][0] * b->m[0][3] + b->m[2][3] * a->m[0][2] + a->m[0][3] * b->m[3][3];
	mtx_out->m[1][0] = a->m[1][1] * b->m[1][0] + a->m[1][0] * b->m[0][0] + b->m[2][0] * a->m[1][2] + b->m[3][0] * a->m[1][3];
	mtx_out->m[1][1] = a->m[1][0] * b->m[0][1] + b->m[1][1] * a->m[1][1] + b->m[2][1] * a->m[1][2] + a->m[1][3] * b->m[3][1];
	mtx_out->m[1][2] = a->m[1][1] * b->m[1][2] + a->m[1][0] * b->m[0][2] + a->m[1][2] * b->m[2][2] + b->m[3][2] * a->m[1][3];
	mtx_out->m[1][3] = a->m[1][0] * b->m[0][3] + b->m[1][3] * a->m[1][1] + b->m[2][3] * a->m[1][2] + a->m[1][3] * b->m[3][3];
	mtx_out->m[2][0] = a->m[2][1] * b->m[1][0] + a->m[2][0] * b->m[0][0] + b->m[2][0] * a->m[2][2] + b->m[3][0] * a->m[2][3];
	mtx_out->m[2][1] = a->m[2][0] * b->m[0][1] + b->m[1][1] * a->m[2][1] + b->m[2][1] * a->m[2][2] + a->m[2][3] * b->m[3][1];
	mtx_out->m[2][2] = a->m[2][1] * b->m[1][2] + a->m[2][0] * b->m[0][2] + a->m[2][2] * b->m[2][2] + b->m[3][2] * a->m[2][3];
	mtx_out->m[2][3] = a->m[2][0] * b->m[0][3] + b->m[1][3] * a->m[2][1] + b->m[2][3] * a->m[2][2] + a->m[2][3] * b->m[3][3];
	mtx_out->m[3][0] = b->m[0][0] * a->m[3][0] + a->m[3][1] * b->m[1][0] + a->m[3][2] * b->m[2][0] + b->m[3][0] * a->m[3][3];
	mtx_out->m[3][1] = b->m[1][1] * a->m[3][1] + a->m[3][0] * b->m[0][1] + a->m[3][2] * b->m[2][1] + a->m[3][3] * b->m[3][1];
	mtx_out->m[3][2] = b->m[0][2] * a->m[3][0] + b->m[1][2] * a->m[3][1] + a->m[3][2] * b->m[2][2] + b->m[3][2] * a->m[3][3];
	mtx_out->m[3][3] = b->m[1][3] * a->m[3][1] + a->m[3][0] * b->m[0][3] + a->m[3][2] * b->m[2][3] + a->m[3][3] * b->m[3][3];
}

// render to texture - camera window
void ccamwnd::rtt_camera_window()
{
	int p_styles = 0;
	int p_colors = 0;

	const auto IO = ImGui::GetIO();
	const auto camera_size = ImVec2(static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.width), static_cast<float>(cmainframe::activewnd->m_pCamWnd->camera.height));
	ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 320.0f), ImVec2(FLT_MAX, FLT_MAX));

	float	window_padding = 0.0f;
	if(dvars::gui_rtt_padding_enabled && dvars::gui_rtt_padding_enabled->current.enabled) {
			window_padding = dvars::gui_rtt_padding_size ? dvars::gui_rtt_padding_size->current.integer : 6.0f;
	}
	
	const ImVec2 window_padding_both = ImVec2(window_padding * 2.0f, window_padding * 2.0f);
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(window_padding, window_padding)); p_styles++;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f)); p_styles++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, 0); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, 0); p_colors++;
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TabUnfocusedActive)); p_colors++;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ToImVec4(dvars::gui_rtt_padding_color->current.vector)); p_colors++;
	
	auto camerawnd = ggui::get_rtt_camerawnd();
	if (camerawnd->should_set_focus)
	{
		ImGui::SetNextWindowFocus();
		camerawnd->should_set_focus = false;
	}

	ImGui::Begin("Camera Window##rtt", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	if (camerawnd->scene_texture)
	{
		bool tabbar_visible = true;
		const auto wnd = ImGui::GetCurrentWindow();

		if(ImGui::IsWindowDocked() && wnd)
		{
			if(wnd->DockNode && wnd->DockNode->IsHiddenTabBar())
			{
				tabbar_visible = false;
			}
		}

		const float frame_height = tabbar_visible ? ImGui::GetFrameHeightWithSpacing() : 0.0f;
		camerawnd->scene_size_imgui = ImGui::GetWindowSize() - ImVec2(0.0f, frame_height) - window_padding_both;

		// hack to disable left mouse window movement
		ImGui::BeginChild("scene_child", ImVec2(camera_size.x, camera_size.y + frame_height) + window_padding_both, false, ImGuiWindowFlags_NoMove);
		{
			const auto screenpos = ImGui::GetCursorScreenPos();
			SetWindowPos(cmainframe::activewnd->m_pCamWnd->GetWindow(), HWND_BOTTOM, (int)screenpos.x, (int)screenpos.y, (int)camerawnd->scene_size_imgui.x, (int)camerawnd->scene_size_imgui.y, SWP_NOZORDER);

			const auto pre_image_cursor = ImGui::GetCursorPos();

			ImGui::Image(camerawnd->scene_texture, camera_size);
			camerawnd->window_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
			
			// pop ItemSpacing
			ImGui::PopStyleVar(); p_styles--;

			ImGui::SetCursorPos(pre_image_cursor);
			const auto cursor_screen_pos = ImGui::GetCursorScreenPos();

			camerawnd->cursor_pos = ImVec2(IO.MousePos.x - cursor_screen_pos.x, IO.MousePos.y - cursor_screen_pos.y);
			camerawnd->cursor_pos_pt = CPoint((LONG)camerawnd->cursor_pos.x, (LONG)camerawnd->cursor_pos.y);

			ggui::FixDockingTabbarTriangle(wnd, camerawnd);

			
			// *
			// ImGuizmo

			ImGui::Indent(8.0f);

			// setup the view and projection matrices
			
			float axis[9];
			axis[0] = cmainframe::activewnd->m_pCamWnd->camera.vpn[0];
			axis[1] = cmainframe::activewnd->m_pCamWnd->camera.vpn[1];
			axis[2] = cmainframe::activewnd->m_pCamWnd->camera.vpn[2];
			axis[3] = -cmainframe::activewnd->m_pCamWnd->camera.vright[0];
			axis[4] = -cmainframe::activewnd->m_pCamWnd->camera.vright[1];
			axis[5] = -cmainframe::activewnd->m_pCamWnd->camera.vright[2];
			axis[6] = cmainframe::activewnd->m_pCamWnd->camera.vup[0];
			axis[7] = cmainframe::activewnd->m_pCamWnd->camera.vup[1];
			axis[8] = cmainframe::activewnd->m_pCamWnd->camera.vup[2];

			//MatrixForViewer
			game::GfxMatrix view = {};
			utils::hook::call<void(__cdecl)(float(*mtx)[4], const float* origin, const float*)>(0x4A7A70)(view.m, cmainframe::activewnd->m_pCamWnd->camera.origin, axis);

			
			float tanHalfFovY = tan(game::g_PrefsDlg()->camera_fov * 0.01745329238474369f * 0.5f) * 0.75f;
			float tanHalfFovX = tanHalfFovY * ((float)cmainframe::activewnd->m_pCamWnd->camera.width / (float)cmainframe::activewnd->m_pCamWnd->camera.height);

			// R_SetupProjection
			game::GfxMatrix projection = {};
			utils::hook::call<void(__cdecl)(game::GfxMatrix*, float halfx, float halfy, float znear)>(0x4A78E0)(&projection, tanHalfFovX, tanHalfFovY, 0.0099999998f);
			
			// ----------
			
			// LH to RH conversion (not sure what I've done here .. but it works :e)

			game::GfxMatrix inv_view_mtx = {
				-1.f, 0.f, 0.f, 0.f,
				0.f, -1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };

			mtx4x4_mul(&view, &inv_view_mtx, &view);
			mtx4x4_mul(&view, &view, &inv_view_mtx);

			view.m[0][0] *= -1;
			view.m[0][1] *= -1;
			view.m[0][2] *= -1;
			view.m[1][0] *= -1;
			view.m[1][1] *= -1;
			view.m[1][2] *= -1;

			// ----------
			
			game::GfxMatrix inv_proj_mtx = {
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, -1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };
			
			mtx4x4_mul(&projection, &inv_proj_mtx, &projection);
			mtx4x4_mul(&projection, &projection, &inv_proj_mtx);
			
			// ----------
			
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(screenpos.x, screenpos.y, camera_size.x, camera_size.y);

			/*if (ImGuizmo::IsOver())
			{
				if(ImGuizmo::IsUsing())
				{
					game::printf_to_console("is using guizmo");
				}
				
				camerawnd->window_hovered = false;
			}*/

			float mtx_scale[3] = { 1.0f, 1.0f, 1.0f };
			float angles[3] = { 0.0f, 0.0f, 0.0f };
			
			if (const auto edit_entity = game::g_edit_entity();
				edit_entity && edit_entity->epairs)
			{
				if(_stricmp(edit_entity->eclass->name, "worldspawn"))
				{
					// pass mouse input to imgui if guizmo is hovered (TODO: only pass left click)
					if (ImGuizmo::IsOver()) 
					{
						//camerawnd->window_hovered = false;
						camerawnd->capture_left_mousebutton = true;
					}

					for (auto epair = edit_entity->epairs; epair; epair = epair->next)
					{
						std::string key = utils::str_to_lower(epair->key);
						if (key == "angles")
						{
							// switch axis
							if (sscanf(epair->value, "%f %f %f", &angles[1], &angles[2], &angles[0]) == 3)
							{ }

							break;
						}
					}

					float tmp_matrix[16];
					ImGuizmo::RecomposeMatrixFromComponents(edit_entity->origin, angles, mtx_scale, tmp_matrix);

					const auto guizmo_mode = game::g_bRotateMode ? ImGuizmo::OPERATION::ROTATE : ImGuizmo::OPERATION::TRANSLATE;
					if(ImGuizmo::Manipulate(&view.m[0][0], &projection.m[0][0], guizmo_mode, ImGuizmo::MODE::WORLD, tmp_matrix))
					{
						//ImGui::TextUnformatted("Manipulating ...");

						if (ImGuizmo::IsOver())
						{
							float t_origin[3], t_angles[3];
							ImGuizmo::DecomposeMatrixToComponents(tmp_matrix, t_origin, t_angles, mtx_scale);

							char org_str[64] = {};
							ggui::entity::addprop_helper_s helper = {};

							
							helper.add_undo = false; // needs logic for on first edit
							
							if (sprintf_s(org_str, "%.5f %.5f %.5f", t_origin[0], t_origin[1], t_origin[2])) {
								helper.is_origin = true;
								ggui::entity::AddProp("origin", org_str, &helper);
							}

							// switch axis
							if (sprintf_s(org_str, "%.5f %.5f %.5f", t_angles[1], t_angles[2], t_angles[0])) {
								helper.is_angle = true;
								ggui::entity::AddProp("angles", org_str, &helper);
							}

							//ImGui::TextUnformatted(org_str);
						}
						
					}
				}
			}
			
			ImGui::EndChild();
		}
	}
	ImGui::PopStyleColor(p_colors);
	ImGui::PopStyleVar(p_styles);
	ImGui::End();
}

// CTRL + SHIFT + RMButton
void cam_rotate()
{
	auto cam = cmainframe::activewnd->m_pCamWnd;
	auto prefs = game::g_PrefsDlg();
	
	CPoint point;
	GetCursorPos(&point);

	if (point.x != cam->m_ptCursor.x || point.y != cam->m_ptCursor.y)
	{
		// use anglespeed
		cam->camera.angles[1] = cam->camera.angles[1] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
		cam->camera.angles[0] = cam->camera.angles[0] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.y - cam->m_ptCursor.y);

		// org
		//cam->camera.angles[1] = cam->camera.angles[1] - prefs->m_nMoveSpeed / 500.0 * (point.x - cam->m_ptCursor.x);
		//cam->camera.angles[0] = cam->camera.angles[0] - prefs->m_nMoveSpeed / 500.0 * (point.y - cam->m_ptCursor.y);

		SetCursorPos(cam->m_ptCursor.x, cam->m_ptCursor.y);
		cam->x48 = 0;

		ShowCursor(0);
	}
}

void cam_positiondrag()
{
	auto cam = cmainframe::activewnd->m_pCamWnd;
	auto prefs = game::g_PrefsDlg();

	CPoint point;
	GetCursorPos(&point);
	
	if (point.x != cam->m_ptCursor.x || point.y != cam->m_ptCursor.y)
	{
		// use anglespeed
		cam->camera.angles[1] = cam->camera.angles[1] - static_cast<float>(prefs->m_nAngleSpeed) / 500.0f * static_cast<float>(point.x - cam->m_ptCursor.x);
		
		// org
		//cam->camera.angles[1] = cam->camera.angles[1] - prefs->m_nMoveSpeed / 500.0 * (point.x - cam->m_ptCursor.x);

		const float delta = static_cast<float>(prefs->m_nMoveSpeed) / -250.0f * static_cast<float>(point.y - cam->m_ptCursor.y);
		utils::vector::normalize(cam->camera.vpn);
		
		cam->camera.origin[0] = cam->camera.vpn[0] * delta + cam->camera.origin[0];
		cam->camera.origin[1] = cam->camera.vpn[1] * delta + cam->camera.origin[1];
		//cam->camera.origin[2] = cam->camera.origin[2];

		SetCursorPos(cam->m_ptCursor.x, cam->m_ptCursor.y);
		cam->x48 = 0;
		
		ShowCursor(0);
	}
}

void ccamwnd::mouse_control(float dtime)
{
	const static uint32_t Cam_MouseControl_Func = 0x403950;
	__asm
	{
		push	esi;
		mov		esi, ecx; // esi = this
		push	dtime;
		call	Cam_MouseControl_Func; // __userpurge :: automatically fixes the stack
		pop		esi;
	}

	return;
}

void ccamwnd::mouse_up(ccamwnd* cam, int flags)
{
	cam->m_nCambuttonstate = 0;
	game::Drag_MouseUp(flags);
	cam->prob_some_cursor = 0;
	cam->x47 = 0;
	cam->x48 = 1; // prob. int16

	int sw_cur;
	do {
		sw_cur = ShowCursor(1);
	} while (sw_cur < 0);
}

void ccamwnd::mouse_moved(ccamwnd* wnd, int buttons, int x, int y)
{
	const static uint32_t CCamWnd__Cam_MouseMoved_Func = 0x404FC0;
	__asm
	{
		pushad;

		mov		eax, buttons;
		push	y;
		push	x;
		mov		ecx, wnd;
		call	CCamWnd__Cam_MouseMoved_Func; // cleans the stack

		popad;
	}
}


// *
// | -------------------- MSG typedefs ------------------------
// *


typedef void(__thiscall* on_ccamwnd_msg)(ccamwnd*, UINT, CPoint);

	// mouse scrolling handled in cmainframe
    on_ccamwnd_msg __on_lbutton_down;
    on_ccamwnd_msg __on_lbutton_up;
    on_ccamwnd_msg __on_rbutton_down;
    on_ccamwnd_msg __on_rbutton_up;
	on_ccamwnd_msg __on_mouse_move;


typedef void(__stdcall* on_ccamwnd_key)(UINT nChar, UINT nRepCnt, UINT nFlags);

	on_ccamwnd_key __on_keydown_cam;
	on_ccamwnd_key __on_keyup_cam;


// *
// | ----------------- Left Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_lbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_lbutton_down(pThis, nFlags, point); // not in use?
}

void __fastcall ccamwnd::on_lbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	//__on_lbutton_up(pThis, nFlags, point);

	czwnd::on_lbutton_up(cmainframe::activewnd->m_pZWnd, nullptr, nFlags, point); // redirect
}


// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall ccamwnd::on_rbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_rbutton_down(pThis, nFlags, point); // not in use?
}

void __fastcall ccamwnd::on_rbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	//__on_rbutton_up(pThis, nFlags, point);

	czwnd::on_rbutton_up(cmainframe::activewnd->m_pZWnd, nullptr, nFlags, point); // redirect
}


// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall ccamwnd::on_mouse_move(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// original function
	__on_mouse_move(pThis, nFlags, point);
}


// *
// | ----------------- Keys Up/Down ---------------------
// *

void __stdcall ccamwnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyDown
	__on_keydown_cam(nChar, nRepCnt, nFlags);
}

void __stdcall ccamwnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// original function :: CMainFrame::OnKeyUp
	__on_keyup_cam(nChar, nRepCnt, nFlags);
}


// *
// | ----------------- Windowproc ---------------------
// *

BOOL WINAPI ccamwnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// handle text input when the imgui camera window is active (selected) 
	// but the user is within a textbox in some other imgui window
	if (Msg == WM_CHAR || Msg == WM_KEYDOWN || Msg == WM_KEYUP)
	{
		if (ggui::cz_context_ready())
		{
			// set cz context (in-case we use multiple imgui context's)
			IMGUI_BEGIN_CZWND;

			if (!ggui::get_rtt_camerawnd()->window_hovered && ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
	}
	
	// => og CamWndProc
    return utils::hook::call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x402D90)(hWnd, Msg, wParam, lParam);
}


void __declspec(naked) camwnd_set_child_window_style()
{
	const static uint32_t retn_pt = 0x402EF7;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD; //WS_VISIBLE;
		jmp		retn_pt;
	}
}

void __declspec(naked) camwnd_set_detatched_child_window_style()
{
	const static uint32_t retn_pt = 0x402ED1;
	__asm
	{
		mov		dword ptr[eax + 20h], WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_POPUP;
		jmp		retn_pt;
	}
}


// *
// *

void ccamwnd::hooks()
{
	// hook ccamwnd message handler
    utils::hook::set(0x402E86 + 4, ccamwnd::windowproc);

#if 1
	// ccamwnd::precreatewindow -> change window style for child windows (split view, not detatched)
	utils::hook::nop(0x402EEE, 2);
	utils::hook::nop(0x402EF0, 7);
		 utils::hook(0x402EF0, camwnd_set_child_window_style, HOOK_JUMP).install()->quick();

	// ccamwnd::precreatewindow -> change window style for detatched windows (split view, detatched)
	utils::hook::nop(0x402ECA, 7);
		 utils::hook(0x402ECA, camwnd_set_detatched_child_window_style, HOOK_JUMP).install()->quick();
#endif

	utils::hook(0x405346, cam_rotate, HOOK_CALL).install()->quick();
	utils::hook(0x405321, cam_positiondrag, HOOK_CALL).install()->quick();

	//utils::hook::nop(0x404AD1, 23);
	
	__on_lbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403160, ccamwnd::on_lbutton_down, HK_JUMP));
    __on_lbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4031D0, ccamwnd::on_lbutton_up, HK_JUMP));

    __on_rbutton_down   = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x4032B0, ccamwnd::on_rbutton_down, HK_JUMP));
    __on_rbutton_up     = reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403310, ccamwnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_ccamwnd_msg>(utils::hook::detour(0x403100, ccamwnd::on_mouse_move, HK_JUMP));

	__on_keydown_cam	= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x402F60, ccamwnd::on_keydown, HK_JUMP));
	__on_keyup_cam		= reinterpret_cast<on_ccamwnd_key>(utils::hook::detour(0x408B70, ccamwnd::on_keyup, HK_JUMP));
}