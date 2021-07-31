#pragma once

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
	static void				main();
	static LRESULT WINAPI	windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static void				on_endframe();

	static void __fastcall	on_lbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_lbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_rbutton_down(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_rbutton_up(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_mouse_move(cxywnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __stdcall	on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags);
	static void __stdcall	on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags);
};