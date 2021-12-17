#pragma once
#include <Windows.h>
#include "camwnd.hpp"
#include "texwnd.hpp"
#include "cxywnd.hpp"
#include "zwnd.hpp"

//#include <afxext.h>

class cmainframe : public CFrameWnd
{
private:
	// Do not allow this class to be instanced
	cmainframe() = delete;
	~cmainframe() = delete;

public:
	CStatusBar m_wndStatusBar;
	CLstToolBar m_wndToolBar;
	CDialogBar m_wndHelpBar;
	CTextureBar m_wndTextureBar;
	CSplitterWnd m_wndSplit;
	CSplitterWnd m_wndSplit2;
	CSplitterWnd m_wndSplit3;
	cxywnd *m_pXYWnd;
	ccamwnd *m_pCamWnd;
	ctexwnd *m_pTexWnd;
	czwnd *m_pZWnd;
	CFilterWnd* m_pFilterWnd;
	CWnd* m_pEditWnd;
	int m_nCurrentStyle;
	char* m_strStatus[15];
	cxywnd* m_pActiveXY;
	bool m_bCamPreview;
	bool m_bDoLoop;
	bool m_bSplittersOK;

	//
	static cmainframe* activewnd;
	
	static void					hooks();
	static bool					is_combined_view() { return cmainframe::activewnd->m_nCurrentStyle; };
	
	static LRESULT __fastcall	windowproc(cmainframe* pThis, [[maybe_unused]] void* edx, UINT Msg, WPARAM wParam, LPARAM lParam);
	static BOOL __fastcall		on_mscroll(cmainframe* pThis, [[maybe_unused]] void* edx, UINT nFlags, SHORT zDelta, CPoint point);
	static void __fastcall		on_keydown(cmainframe* pThis, [[maybe_unused]] void* edx, UINT nChar, UINT nRepCnt, UINT nFlags);
	static void __stdcall		on_keyup(cmainframe* pThis, UINT nChar);
	static void __fastcall		on_size(cmainframe* pThis, [[maybe_unused]] void* edx, UINT nFlags, int x, int y);
	static void __fastcall		on_destroy(cmainframe* pThis);
	
	void						update_windows(int nBits);
	void						routine_processing(void);
	static void					hk_routine_processing(void);

	static void					register_dvars();
	
};
STATIC_ASSERT_OFFSET(cmainframe, m_wndStatusBar, 0xD4);
STATIC_ASSERT_OFFSET(cmainframe, m_wndToolBar, 0x16C);
STATIC_ASSERT_OFFSET(cmainframe, m_wndHelpBar, 0x224);
STATIC_ASSERT_OFFSET(cmainframe, m_wndTextureBar, 0x2C8);
STATIC_ASSERT_OFFSET(cmainframe, m_wndSplit, 0x528);
STATIC_ASSERT_OFFSET(cmainframe, m_wndSplit2, 0x604);
STATIC_ASSERT_OFFSET(cmainframe, m_wndSplit3, 0x6E0);
STATIC_ASSERT_OFFSET(cmainframe, m_pXYWnd, 0x7BC);
/* ... */
STATIC_ASSERT_OFFSET(cmainframe, m_bDoLoop, 0x819);


namespace mainframe
{
	typedef BOOL(__thiscall* on_cmainframe_scroll)(cmainframe*, UINT, SHORT, CPoint);
		extern on_cmainframe_scroll __on_mscroll;
	
	typedef void(__thiscall* on_cmainframe_keydown)(cmainframe*, UINT, UINT, UINT);
		extern on_cmainframe_keydown __on_keydown;

	typedef void(__stdcall* on_cmainframe_keyup)(cmainframe*, UINT);
		extern on_cmainframe_keyup __on_keyup;
}
