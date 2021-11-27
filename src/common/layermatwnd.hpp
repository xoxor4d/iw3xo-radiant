#pragma once

struct layermatwnd_s
{
	HWND m_frame_hwnd;
	HWND m_toolbar_hwnd;
	HWND m_content_hwnd;
	int unkown01;
	int active_layer_material;
	int selected_layer_index;
};

enum GfxProjectionTypes
{
	GFX_PROJECTION_2D = 0x0,
	GFX_PROJECTION_3D = 0x1,
};

struct GfxCmdProjectionSet
{
	game::GfxCmdHeader header;
	GfxProjectionTypes projection;
};

struct GfxPointVertex
{
	float xyz[3];
	game::GfxColor color;
};


extern layermatwnd_s* layermatwnd_struct;

namespace layermatwnd
{
	extern int rendermethod_axis;
	extern int rendermethod_preview;
	extern float fov;
	extern bool rotation_pause;
}

class clayermatwnd
{
private:
	// Do not allow this class to be instanced
	clayermatwnd() = delete;
	~clayermatwnd() = delete;

public:
    static void     hooks();
	static void		create_layermatwnd();
	static void		create_layerlist();
	static void		precreate_window();
	static void		on_paint();

	static LRESULT __stdcall windowproc_frame(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static LRESULT __stdcall windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};