#pragma once
#include <Windows.h>
#include "CamWnd.hpp"
#include "TexWnd.hpp"
#include "XYWnd.hpp"
#include "ZWnd.hpp"

class CMainFrame
{
	friend BOOL RadiantMod_Init();

private:
	// Do not allow this class to be instanced
	CMainFrame() = delete;
	~CMainFrame() = delete;

	int gap01;
	BYTE gap02[1976];
	CXYWnd *m_pXYWnd;
	CCamWnd *m_pCamWnd;
	CTexWnd *m_pTexWnd;
	CZWnd *m_pZWnd;
	void *m_pFilterWnd;
	BYTE gap03[72];
	bool m_bCamPreview;
	bool m_bDoLoop;
	bool unkBoolIsOne;

	static void hk_RoutineProcessing(void);

public:
	void RoutineProcessing(void);
	void UpdateWindows(int nBits);
};
