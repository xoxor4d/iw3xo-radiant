#include "std_include.hpp"

// CDC
// Do not allow this class to be instanced
CDC::CDC()
{ }

void CDC::SetTextColor(COLORREF crColor)
{
	((SetTextColor_t)0x593FC1)(this, crColor);
}

COLORREF CDC::SetBkColor(COLORREF crColor)
{
	return ((SetBkColor_t)0x593EDA)(this, crColor);
}

COLORREF CDC::SetBkMode(int mode)
{
	return ((SetBkMode_t)0x593F09)(this, mode);
}

// Do not allow this class to be instanced
CWnd::CWnd()
{ }

class CGdiObject
{
private:
	DWORD unk;
	DWORD m_hObject;

	// Do not allow this class to be instanced
	CGdiObject() = delete;
	~CGdiObject() = delete;

public:
	inline void* GetSafeHandle(void)
	{
		if (this)
		{
			return (void*)&this->m_hObject;
		}
		else
		{
			return NULL;
		}
	}
};

HWND CWnd::GetWindow()
{
	if(this && this->m_hWnd)
	{
		return this->m_hWnd;
	}

	return nullptr;
}

BOOL CWnd::RedrawWindow(LPCRECT lpRectUpdate, struct CRgn* prgnUpdate, UINT flags)
{
	HRGN hrgnUpdate = (HRGN)reinterpret_cast<CGdiObject*>(prgnUpdate)->GetSafeHandle();
	return ::RedrawWindow(this->m_hWnd, lpRectUpdate, hrgnUpdate, flags);
}

namespace afx
{
	// *
	// -------------------------------------- function typedefs -------------------------------------------
	// *
	
	CSplitterWnd__GetPane_t CSplitterWnd__GetPane = reinterpret_cast<CSplitterWnd__GetPane_t>(0x5A5409);
	CSplitterWnd__SetRowInfo_t CSplitterWnd__SetRowInfo = reinterpret_cast<CSplitterWnd__SetRowInfo_t>(0x5A544B);

	CWnd_SetFocus_t CWnd_SetFocus = reinterpret_cast<CWnd_SetFocus_t>(0x58EAAC);
	CWnd_FromHandle_t CWnd_FromHandle = reinterpret_cast<CWnd_FromHandle_t>(0x5871BD);
	
}