#include "STDInclude.hpp"

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

BOOL CWnd::RedrawWindow(LPCRECT lpRectUpdate, struct CRgn* prgnUpdate, UINT flags)
{
	HRGN hrgnUpdate = (HRGN)reinterpret_cast<CGdiObject*>(prgnUpdate)->GetSafeHandle();
	return ::RedrawWindow(this->m_hWnd, lpRectUpdate, hrgnUpdate, flags);
}
