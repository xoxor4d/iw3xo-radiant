#pragma once
#include <Windows.h>
#include "CamWnd.hpp"
#include "TexWnd.hpp"
#include "XYWnd.hpp"
#include "ZWnd.hpp"

//#include <afxext.h>

class CMainFrame : public CFrameWnd
{
	// why?? D:
	//friend BOOL Init();

private:
	// Do not allow this class to be instanced
	CMainFrame() = delete;
	~CMainFrame() = delete;

public:
	CStatusBar m_wndStatusBar;
	CLstToolBar m_wndToolBar;
	CDialogBar m_wndHelpBar;
	CTextureBar m_wndTextureBar;
	//BYTE gap02[0x294]; //0x4F4 //0x598 //0x650 //1764 //1892 //1976
	CSplitterWnd m_wndSplit;
	CSplitterWnd m_wndSplit2;
	CSplitterWnd m_wndSplit3;
	CXYWnd *m_pXYWnd;
	CCamWnd *m_pCamWnd;
	CTexWnd *m_pTexWnd;
	CZWnd *m_pZWnd;
	CGroupDlg* m_pFilterWnd;
	void* m_pEditWnd;
	int m_nCurrentStyle;
	char* m_strStatus[15];
	CXYWnd* m_pActiveXY;
	bool m_bCamPreview;
	bool m_bDoLoop;
	bool m_bSplittersOK;

	//
	static void	hk_RoutineProcessing(void);
	static void	hk_DefWindowProc(void);


	static void	main();

	static CMainFrame* ActiveWindow;
	
	static LRESULT __fastcall	windowproc(CMainFrame* pThis, [[maybe_unused]] void* edx, UINT Msg, WPARAM wParam, LPARAM lParam);
	static void __fastcall		on_mscroll(CMainFrame* pThis, [[maybe_unused]] void* edx, UINT nFlags, SHORT zDelta, CPoint point);
	static void __fastcall		on_keydown(CMainFrame* pThis, [[maybe_unused]] void* edx, UINT nChar, UINT nRepCnt, UINT nFlags);
	static void __stdcall		on_keyup(CMainFrame* pThis, UINT nChar);
	
	void						RoutineProcessing(void);
	void						UpdateWindows(int nBits);
};
STATIC_ASSERT_OFFSET(CMainFrame, m_wndStatusBar, 0xD4);
STATIC_ASSERT_OFFSET(CMainFrame, m_wndToolBar, 0x16C);
STATIC_ASSERT_OFFSET(CMainFrame, m_wndHelpBar, 0x224);
STATIC_ASSERT_OFFSET(CMainFrame, m_wndTextureBar, 0x2C8);
STATIC_ASSERT_OFFSET(CMainFrame, m_wndSplit, 0x528);
STATIC_ASSERT_OFFSET(CMainFrame, m_wndSplit2, 0x604);
STATIC_ASSERT_OFFSET(CMainFrame, m_wndSplit3, 0x6E0);
STATIC_ASSERT_OFFSET(CMainFrame, m_pXYWnd, 0x7BC);
/* ... */
STATIC_ASSERT_OFFSET(CMainFrame, m_bDoLoop, 0x819);