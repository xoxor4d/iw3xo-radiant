#include "std_include.hpp"

// tests
//#define HK_RESIZE_WND


IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * (cxywnd member functions are only used if the xy subwindow is focused)
 * - Directly clicking onto an imgui window will not focus the subwindow behind it
 * - IO will instead be handled by cmainframe member functions
 * + Mouse scrolling handled by cmainframe::on_mscroll
 * + Char events handled by cxywnd::wndproc / ccamwnd::wndproc or cmainframe::windowproc (depends on focused window)
 */

typedef void(__thiscall* on_cxywnd_msg)(cxywnd*, UINT, CPoint);
	on_cxywnd_msg __on_lbutton_down;
	on_cxywnd_msg __on_lbutton_up;
	on_cxywnd_msg __on_rbutton_down;
	on_cxywnd_msg __on_rbutton_up;
	on_cxywnd_msg __on_mouse_move;


typedef void(__stdcall* on_cxywnd_key)(UINT nChar, UINT nRepCnt, UINT nFlags);
	on_cxywnd_key __on_keydown;
	on_cxywnd_key __on_keyup;


// *
// | ----------------- Left Mouse Button ---------------------
// *

void __fastcall cxywnd::on_lbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	// set cxy context
	IMGUI_BEGIN_CXYWND;

	// check if mouse cursor is above any cxy imgui window
	if (ImGui::GetIO().WantCaptureMouse)
	{
		// handle input, don't pass input to the xywindow
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONDOWN);
	}
	else // pass input to the xywindow
    {
        return __on_lbutton_down(pThis, nFlags, point);
    }
}

void __fastcall cxywnd::on_lbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_LBUTTONUP);
	}
	else
    {
        return __on_lbutton_up(pThis, nFlags, point);
    }
}


// *
// | ----------------- Right Mouse Button ---------------------
// *

void __fastcall cxywnd::on_rbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONDOWN);
	}
	else
    {
        return __on_rbutton_down(pThis, nFlags, point);
    }
}

void __fastcall cxywnd::on_rbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(pThis->GetWindow(), WM_RBUTTONUP);
	}
	else
    {
        return __on_rbutton_up(pThis, nFlags, point);
    }
}



// *
// | ----------------- Mouse Move ---------------------
// *

void __fastcall cxywnd::on_mouse_move(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point)
{
	IMGUI_BEGIN_CXYWND;

	// block xywindow input if mouse cursor is above any cxy imgui window
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		return __on_mouse_move(pThis, nFlags, point);
	}
}



// *
// | ----------------- Keys Up/Down ---------------------
// *

void __stdcall cxywnd::on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(cmainframe::activewnd->m_pXYWnd->GetWindow(), WM_KEYDOWN, 0, nChar);
	}
	else
	{
		// cmainframe::OnKeyDown
		return __on_keydown(nChar, nRepCnt, nFlags);
	}
}

void __stdcall cxywnd::on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	IMGUI_BEGIN_CXYWND;
	
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui::HandleKeyIO(cmainframe::activewnd->m_pXYWnd->GetWindow(), WM_KEYUP, 0, nChar);
	}
	else
	{
		// cmainframe::OnKeyUp
		return __on_keyup(nChar, nRepCnt, nFlags);
	}
}


//void cxywnd::on_endframe()
//{
//	game::R_EndFrame();
//}


// *
// | ----------------- Windowproc ---------------------
// *

LRESULT WINAPI cxywnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (components::gui::all_contexts_ready())
	{
		// we only need the char event
		if (Msg == WM_CHAR)
		{
			// handle char inputs when xy window is focused
			IMGUI_BEGIN_CXYWND;
			if (ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}

			// handle char inputs if xy window is focused but cursor is within the camera window, over an imgui menu
			IMGUI_BEGIN_CCAMERAWND;
			if (ImGui::GetIO().WantCaptureMouse)
			{
				ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam);
				return true;
			}
		}
	}
	
	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

void __declspec(naked) windowproc_stub()
{
	const static uint32_t AfxRegisterClass_Func = 0x58A0A1;
	const static uint32_t retn_pt = 0x463A09;
	__asm
	{
		mov     dword ptr[esp + 10h], offset cxywnd::windowproc;
		call	AfxRegisterClass_Func;

		jmp		retn_pt;
	}
}

#ifdef HK_RESIZE_WND
//void R_SetupAntiAliasing(game::GfxWindowParms* wnd, int window_count)
//{
//	int wnd_count = window_count;
//	
//	int aa_samples = wnd->aaSamples;
//	if (aa_samples <= 1)
//	{
//	LABEL_9:
//		game::dx->multiSampleType = 0;
//		game::dx->multiSampleQuality = 0;
//	}
//	else
//	{
//		while (1)
//		{
//			game::dx->multiSampleType = aa_samples;
//			if (game::dx->d3d9->CheckDeviceMultiSampleType(0, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, !wnd->fullscreen, (D3DMULTISAMPLE_TYPE)aa_samples, (DWORD*)&wnd_count) >= 0)
//			{
//				break;
//			}
//			if (--aa_samples <= 1)
//			{
//				goto LABEL_9;
//			}
//		}
//		printf("Using %ix anti-aliasing\n", aa_samples);
//		game::dx->multiSampleQuality = aa_samples - 1;
//	}
//}

void R_SetD3DPresentParameters(_D3DPRESENT_PARAMETERS_* d3dpp, game::GfxWindowParms* wnd, int window_count)
{
	ASSERT_MSG(d3dpp, "invalid D3DPRESENT_PARAMETERS d3dpp");
	ASSERT_MSG(wnd, "invalid GfxWindowParms wnd");
	ASSERT_MSG(wnd->hwnd, "invalid HWND wnd->hwnd");
	
	//R_SetupAntiAliasing(wnd, window_count);
	memset(d3dpp, 0, sizeof(_D3DPRESENT_PARAMETERS_));
	d3dpp->BackBufferWidth = wnd->displayWidth;
	d3dpp->BackBufferHeight = wnd->displayHeight;
	d3dpp->BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp->BackBufferCount = 1;
	d3dpp->MultiSampleType = _D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE; // (D3DMULTISAMPLE_TYPE)game::dx->multiSampleType;
	d3dpp->MultiSampleQuality = 0; // game::dx->multiSampleQuality
	d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp->EnableAutoDepthStencil = 0;
	d3dpp->AutoDepthStencilFormat = static_cast<D3DFORMAT>(game::dx->depthStencilFormat);
	d3dpp->PresentationInterval = 1; //r_vsync->current.enabled ? 1 : 0x80000000;
	d3dpp->hDeviceWindow = wnd->hwnd;
	d3dpp->Flags = 0;

	if (wnd->fullscreen)
	{
		d3dpp->Windowed = 0;
		d3dpp->FullScreen_RefreshRateInHz = wnd->hz;
	}
	else
	{
		d3dpp->FullScreen_RefreshRateInHz = 0;
		d3dpp->Windowed = 1;
	}
}

// R_Hwnd_Resize called from CXYWnd::OnSize
void cxywnd::on_resize(HWND__* hwnd, int width, int height)
{
	ASSERT_MSG(hwnd, "invalid hwnd");

	_D3DPRESENT_PARAMETERS_ d3dpp {};
	game::GfxWindowParms wnd {};
	
	if(width && height)
	{
		int wnd_count = 0;
		if (game::dx->windowCount > 0)
		{
			for (auto i = game::dx->windows; i->hwnd != hwnd; ++i)
			{
				if (++wnd_count >= game::dx->windowCount)
				{
					return;
				}
			}
			
			wnd.hwnd = hwnd;
			wnd.fullscreen = false;
			wnd.displayWidth = width;
			wnd.displayHeight = height;
			wnd.sceneWidth = width;
			wnd.sceneHeight = height;
			wnd.aaSamples = 1;

			R_SetD3DPresentParameters(&d3dpp, &wnd, game::dx->windowCount);

			auto swapchain = &game::dx->windows[wnd_count].swapChain;
			auto old_swapchain = *swapchain;
			if(*swapchain == nullptr)
			{
				ASSERT_MSG(1, "var");
			}
			*swapchain == nullptr;

			if(old_swapchain->lpVtbl->Release((IDirect3DSwapChain9*)old_swapchain))
			{
				ASSERT_MSG(0, "release failed, leaks ...");
			}

			if(auto hr = game::dx->device->CreateAdditionalSwapChain(&d3dpp, (IDirect3DSwapChain9**) swapchain); 
				hr < 0)
			{
				// g_disableRendering;
				ASSERT_MSG(0, "CreateAdditionalSwapChain failed ...");
			}

			game::dx->windows[wnd_count].width = width;
			game::dx->windows[wnd_count].height = height;

			//cmainframe::activewnd->m_pXYWnd->m_nHeight = height;
		}
	}
}
#endif

// *
// | ----------------- Main ---------------------
// *

void cxywnd::main()
{
	// custom windowproc
	utils::hook::nop(0x463A00, 9);
		 utils::hook(0x463A00, windowproc_stub, HOOK_JUMP).install()->quick();

#ifdef HK_RESIZE_WND
	utils::hook(0x46DBF2, cxywnd::on_resize, HOOK_CALL).install()->quick();
#endif
	
	// endframe hook
	//utils::hook(0x465C0E, cxywnd::on_endframe, HOOK_CALL).install()->quick();

	__on_lbutton_down	= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x463F70, cxywnd::on_lbutton_down, HK_JUMP));
	__on_lbutton_up		= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x464860, cxywnd::on_lbutton_up, HK_JUMP));

	__on_rbutton_down	= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x4647B0, cxywnd::on_rbutton_down, HK_JUMP));
	__on_rbutton_up		= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x464990, cxywnd::on_rbutton_up, HK_JUMP));

	__on_mouse_move		= reinterpret_cast<on_cxywnd_msg>(utils::hook::detour(0x464B10, cxywnd::on_mouse_move, HK_JUMP));

	__on_keydown		= reinterpret_cast<on_cxywnd_key>(utils::hook::detour(0x465C90, cxywnd::on_keydown, HK_JUMP));
	__on_keyup			= reinterpret_cast<on_cxywnd_key>(utils::hook::detour(0x46E510, cxywnd::on_keyup, HK_JUMP));
}