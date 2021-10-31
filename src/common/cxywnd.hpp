#pragma once

class cmainframe;

class cxywnd : public CWnd
{
private:
	// Do not allow this class to be instanced
	cxywnd() = delete;
	~cxywnd() = delete;

public:
	bool m_bActive;
	char pad_bActive[3];
	int m_nUpdateBits;
	int m_nWidth;
	int m_nHeight;
	float m_fScale;
	float m_TopClip;
	float m_BottomClip;
	BYTE gap68[4];
	float m_vOrigin[3];
	CPoint m_ptCursor;
	bool m_bRButtonDown;
	char pad01[3];
	int m_nButtonstate;
	int m_nPressx;
	int m_nPressy;
	game::vec3_t m_vPressdelta;
	bool m_bPress_selection;
	bool xx2;
	bool xx3;
	bool xx4;
	int x84;
	int x85;
	int x86;
	int x87;
	int x88;
	int x89;
	int x90;
	int x91;
	int x92;
	int x93;
	int x94;
	int x95;
	int x96;
	int x97;
	int x98;
	int x99;
	int x100;
	int x101;
	int x102;
	int x103;
	int x104;
	int x105;
	int x106;
	game::brush_t* g_brUndo_01;
	DWORD dword108;
	game::brush_t* g_brUndo_02;
	DWORD dword110;
	DWORD dword114;
	DWORD dword118;
	DWORD dword11C;
	int m_nViewType;
	unsigned int m_nTimerID;
	int m_nScrollFlags;
	CPoint m_ptDrag;
	CPoint m_ptDragAdj;
	CPoint m_ptDragTotal;
	CPoint m_ptDown;

	//
	static void				hooks();
	static void				register_dvars();
	static LRESULT WINAPI	windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static void				rtt_grid_window();
	
	//static void __cdecl	on_resize(HWND__* hwnd, int width, int height);
	//static void			on_endframe();

	static void __fastcall	on_lbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_lbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_rbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_rbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_mbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_mbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	
	static void __fastcall	on_mouse_move(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __stdcall	on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags);
	static void __stdcall	on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags);

	static BOOL __stdcall	on_scroll(UINT nFlags, std::int16_t zDelta, CPoint point);
	
	static void				on_view_zoomin(cmainframe* pThis, CPoint point);
	static void				on_view_zoomout(cmainframe* pThis, CPoint point);
};

namespace xywnd
{
	typedef void(__thiscall* on_cxywnd_msg)(cxywnd*, UINT, CPoint);
	extern on_cxywnd_msg __on_lbutton_down;
	extern on_cxywnd_msg __on_lbutton_up;
	extern on_cxywnd_msg __on_rbutton_down;
	extern on_cxywnd_msg __on_rbutton_up;
	extern on_cxywnd_msg __on_mbutton_down;
	extern on_cxywnd_msg __on_mbutton_up;

	typedef void(__thiscall* on_cxywnd_msg)(cxywnd*, UINT, CPoint);
	extern on_cxywnd_msg __on_mouse_move;

	typedef void(__stdcall* on_cxywnd_key)(UINT nChar, UINT nRepCnt, UINT nFlags);
	extern on_cxywnd_key __on_keydown;
	extern on_cxywnd_key __on_keyup;

	typedef BOOL(__stdcall* on_cxywnd_scroll)(UINT nFlags, std::int16_t zDelta, CPoint point);
	extern on_cxywnd_scroll __on_scroll;

	const float GRID_SIZES[11] =
	{ 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f, 256.0f, 512.0f };

	enum E_GRID_SIZES : int
	{
		GRID_05,
		GRID_1,
		GRID_2,
		GRID_4,
		GRID_8,
		GRID_16,
		GRID_32,
		GRID_64,
		GRID_128,
		GRID_256,
		GRID_512,
	};
	
}