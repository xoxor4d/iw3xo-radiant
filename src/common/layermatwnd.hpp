#pragma once

struct layermatwnd_s
{
	HWND m_LayeredWnd_hWnd;
	HWND m_LayeredWnd_Toolbar_hWnd;
	HWND m_hWnd;
	int unkown01;
	int active_layer_material;
	int selected_layer_index;
};

extern layermatwnd_s* layermatwnd_struct;

class clayermatwnd
{
private:
	// Do not allow this class to be instanced
	clayermatwnd() = delete;
	~clayermatwnd() = delete;

public:
    static void     main();
	static void		create_layermatwnd();
	static void		create_layerlist();
	static void		precreate_window();
	static void		on_paint();
	
	static LRESULT __stdcall windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};