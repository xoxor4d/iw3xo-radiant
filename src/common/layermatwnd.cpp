#include "std_include.hpp"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
layermatwnd_s* layermatwnd_struct = reinterpret_cast<layermatwnd_s*>(0x181F500);

void clayermatwnd::on_paint()
{
	PAINTSTRUCT Paint;
	BeginPaint(layermatwnd_struct->m_hWnd, &Paint);

	// R_CheckHwnd_or_Device
	if (!utils::hook::call<bool(__cdecl)(HWND)>(0x501A70)(layermatwnd_struct->m_hWnd))
	{
		EndPaint(layermatwnd_struct->m_hWnd, &Paint);
		return;
	}
	
	// R_BeginFrame
	cdeclcall(void, 0x4FCB10);

	// R_Clear
	utils::hook::call<void(__cdecl)(int, const float*, float, char)>(0x4FCC70)(7, game::g_qeglobals->d_savedinfo.colors[0], 1.0f, 0);

	// R_EndFrame
	cdeclcall(void, 0x4FCBC0);

	// R_IssueRenderCommands
	utils::hook::call<void(__cdecl)(int)>(0x4FD630)(-1);

	// still no clue what this is
	cdeclcall(void, 0x4FD910);

	// R_CheckTargetWindow
	utils::hook::call<void(__cdecl)(HWND)>(0x500660)(layermatwnd_struct->m_hWnd);

	int& random_dword01 = *reinterpret_cast<int*>(0x25D5B88);
	int& random_dword02 = *reinterpret_cast<int*>(0x242293C);
	int& random_dword03 = *reinterpret_cast<int*>(0x2422940);
	
	if(!random_dword01)
	{
		__debugbreak();
	}

	random_dword03 = random_dword02;

	// nice meme IW
	EndPaint(layermatwnd_struct->m_hWnd, &Paint);
}


LRESULT __stdcall clayermatwnd::windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_SIZE)
	{
		game::R_Hwnd_Resize(hWnd, LOWORD(lParam), HIWORD(lParam));
		return 0;
	}

	if(Msg == WM_PAINT)
	{
		clayermatwnd::on_paint();
		return 0;
	}

	if (ggui::layered_context_ready())
	{
		IMGUI_BEGIN_LAYERED;

		switch(Msg)
		{
		case WM_LBUTTONDOWN:
			ImGui::HandleKeyIO(layermatwnd_struct->m_hWnd, WM_LBUTTONDOWN);
			break;

		case WM_LBUTTONUP:
			ImGui::HandleKeyIO(layermatwnd_struct->m_hWnd, WM_LBUTTONUP);
			break;

			
		case WM_RBUTTONDOWN:
			ImGui::HandleKeyIO(layermatwnd_struct->m_hWnd, WM_RBUTTONDOWN);
			break;
			
		case WM_RBUTTONUP:
			ImGui::HandleKeyIO(layermatwnd_struct->m_hWnd, WM_RBUTTONUP);
			break;


		case WM_MBUTTONDOWN:
			ImGui::HandleKeyIO(layermatwnd_struct->m_hWnd, WM_MBUTTONDOWN);
			break;
			
		case WM_MBUTTONUP:
			ImGui::HandleKeyIO(layermatwnd_struct->m_hWnd, WM_MBUTTONUP);
			break;
		}

		if (ImGui::GetIO().WantCaptureMouse)
		{
			return 0;
		}
	}

	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}


void clayermatwnd::precreate_window()
{
	WNDCLASSEXA wc;
	memset(&wc, 0, sizeof(wc));
	
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = 0;
	wc.hIcon = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.style = 512;
	wc.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
	wc.lpszClassName = "LayeredMaterialList";
	wc.lpfnWndProc = clayermatwnd::windowproc;
	
	RegisterClassExA(&wc);
}


void clayermatwnd::create_layerlist()
{
	const auto prefs = game::g_PrefsDlg();
	DWORD window_style;

	if(prefs->m_nView == 1 && prefs->detatch_windows)
	{
		window_style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD | WS_POPUP;
	}
	else
	{
		window_style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD;
	}
	
	layermatwnd_struct->m_hWnd = CreateWindowExA(
		0, // WS_EX_COMPOSITED
		"LayeredMaterialList",
		0,
		window_style,
		0, 0,
		400, 400,
		cmainframe::activewnd->GetWindow(), 
		0, 0, 0);

	if (!layermatwnd_struct->m_hWnd)
	{
		game::Com_Error("Couldn't create the material layer list");
	}
}


void clayermatwnd::create_layermatwnd()
{
	memset(layermatwnd_struct, 0, sizeof(layermatwnd_s));
	clayermatwnd::precreate_window();
	clayermatwnd::create_layerlist();
}


// *
// *

void clayermatwnd::hooks()
{
	//utils::hook(0x422694, clayermatwnd::create_layermatwnd, HOOK_CALL).install()->quick();

	// disable layer render stuff in on_paint
	//utils::hook::nop(0x417FC6, 5);
}