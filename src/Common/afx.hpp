#pragma once

typedef void(__thiscall * SetTextColor_t)(void *, COLORREF);
typedef COLORREF(__thiscall * SetBkColor_t)(void *, COLORREF);
typedef COLORREF(__thiscall* SetBkMode_t)(void*, int mode);

class CDC
{
private:
	// Do not allow this class to be instanced
	CDC();

public:
	
	void SetTextColor(COLORREF crColor);
	COLORREF SetBkColor(COLORREF crColor);
	COLORREF SetBkMode(int mode);
};

class CWnd
{
private:
	// Do not allow this class to be instanced
	CWnd();

	// pure virtual dummy func
	// force this class to have a vtable
	virtual void vftable() = 0;

	void* baseclass[7];
	HWND m_hWnd;
	BYTE unk[0x30];

public:
	BOOL RedrawWindow(	LPCRECT lpRectUpdate = NULL,
						struct CRgn* prgnUpdate = NULL,
						UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
};
STATIC_ASSERT_SIZE(CWnd, 0x54);