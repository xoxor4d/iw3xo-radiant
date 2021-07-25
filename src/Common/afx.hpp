#pragma once
//#include <afxwin.h>

struct IAccessibleProxy;
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

//class CWnd
//{
//private:
//	// Do not allow this class to be instanced
//	CWnd();
//
//	// pure virtual dummy func
//	// force this class to have a vtable
//	//virtual void vftable() = 0;
//	void* vftable;
//
//	void* baseclass[7];
//	HWND m_hWnd;
//	BYTE unk[0x30];
//
//public:
//	HWND GetWindow();
//	BOOL RedrawWindow(	LPCRECT lpRectUpdate = NULL,
//						struct CRgn* prgnUpdate = NULL,
//						UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
//};
//STATIC_ASSERT_SIZE(CWnd, 0x54);

typedef tagRECT CRect;
typedef tagPOINT CPoint;
typedef tagSIZE CSize;

struct CRuntimeClass;
class CObject;
struct CDumpContext;

struct CTypeLibCache {};
struct AFX_CMDHANDLERINFO {};
struct AFX_MSGMAP {};
struct AFX_OLECMDMAP {};
struct AFX_DISPMAP {};
struct AFX_CONNECTIONMAP {};
struct AFX_INTERFACEMAP {};
struct AFX_EVENTSINKMAP {};
struct IConnectionPoint {};
struct CCreateContext {};
struct tagTOOLINFOA {};
struct CScrollBar {};
struct IAccessible {};
struct IAccessibleProxy {};
struct CDataExchange {};
struct COleControlSite {};
struct COleControlContainer {};
struct _AFX_OCC_DIALOG_INFO {};
struct AFX_SIZEPARENTPARAMS {};

struct /*VFT*/ CObject_vtbl
{
	CRuntimeClass* (__thiscall* GetRuntimeClass)(CObject*);
	void(__thiscall * CObject_Destructor)(CObject*);
	void(__thiscall* Serialize)(CObject*, void*); // CArchive
	void(__thiscall* AssertValid)(CObject*);
	void(__thiscall* Dump)(CObject*, CDumpContext*); // CDumpContext
};


class CObject
{
private:
	//CObject();

public:
	//CObject() = delete;
	CObject_vtbl* __vftable /*VFT*/;
};


struct CCmdTarget__XDispatch
{
	unsigned int m_vtbl;
};


struct CCmdTarget__XConnPtContainer
{
	unsigned int m_vtbl;
};


class CCmdTarget : public CObject
{
private:

public:
	int m_dwRef;
	void* m_pOuterUnknown;
	unsigned int m_xInnerUnknown;
	CCmdTarget__XDispatch m_xDispatch;
	int m_bResultExpected;
	CCmdTarget__XConnPtContainer m_xConnPtContainer;
	void* m_pModuleState; // AFX_MODULE_STATE
};


struct CWnd__XAccessible;
struct /*VFT*/ CWnd__XAccessible_vtbl
{
	unsigned int(__stdcall* AddRef)(CWnd__XAccessible*);
	unsigned int(__stdcall* Release)(CWnd__XAccessible*);
	HRESULT(__stdcall* QueryInterface)(CWnd__XAccessible*, const _GUID*, void**);
	HRESULT(__stdcall* Invoke)(CWnd__XAccessible*, int, const _GUID*, unsigned int, unsigned __int16, tagDISPPARAMS*, tagVARIANT*, tagEXCEPINFO*, unsigned int*);
	HRESULT(__stdcall* GetIDsOfNames)(CWnd__XAccessible*, const _GUID*, wchar_t**, unsigned int, unsigned int, int*);
	HRESULT(__stdcall* GetTypeInfoCount)(CWnd__XAccessible*, unsigned int*);
	HRESULT(__stdcall* GetTypeInfo)(CWnd__XAccessible*, unsigned int, unsigned int, ITypeInfo**);
	HRESULT(__stdcall* get_accParent)(CWnd__XAccessible*, IDispatch**);
	HRESULT(__stdcall* get_accChildCount)(CWnd__XAccessible*, int*);
	HRESULT(__stdcall* get_accChild)(CWnd__XAccessible*, tagVARIANT, IDispatch**);
	HRESULT(__stdcall* get_accName)(CWnd__XAccessible*, tagVARIANT, wchar_t**);
	HRESULT(__stdcall* get_accValue)(CWnd__XAccessible*, tagVARIANT, wchar_t**);
	HRESULT(__stdcall* get_accDescription)(CWnd__XAccessible*, tagVARIANT, wchar_t**);
	HRESULT(__stdcall* get_accRole)(CWnd__XAccessible*, tagVARIANT, tagVARIANT*);
	HRESULT(__stdcall* get_accState)(CWnd__XAccessible*, tagVARIANT, tagVARIANT*);
	HRESULT(__stdcall* get_accHelp)(CWnd__XAccessible*, tagVARIANT, wchar_t**);
	HRESULT(__stdcall* get_accHelpTopic)(CWnd__XAccessible*, wchar_t**, tagVARIANT, int*);
	HRESULT(__stdcall* get_accKeyboardShortcut)(CWnd__XAccessible*, tagVARIANT, wchar_t**);
	HRESULT(__stdcall* get_accFocus)(CWnd__XAccessible*, tagVARIANT*);
	HRESULT(__stdcall* get_accSelection)(CWnd__XAccessible*, tagVARIANT*);
	HRESULT(__stdcall* get_accDefaultAction)(CWnd__XAccessible*, tagVARIANT, wchar_t**);
	HRESULT(__stdcall* accSelect)(CWnd__XAccessible*, int, tagVARIANT);
	HRESULT(__stdcall* accLocation)(CWnd__XAccessible*, int*, int*, int*, int*, tagVARIANT);
	HRESULT(__stdcall* accNavigate)(CWnd__XAccessible*, int, tagVARIANT, tagVARIANT*);
	HRESULT(__stdcall* accHitTest)(CWnd__XAccessible*, int, int, tagVARIANT*);
	HRESULT(__stdcall* accDoDefaultAction)(CWnd__XAccessible*, tagVARIANT);
	HRESULT(__stdcall* put_accName)(CWnd__XAccessible*, tagVARIANT, wchar_t*);
	HRESULT(__stdcall* put_accValue)(CWnd__XAccessible*, tagVARIANT, wchar_t*);
};


struct CWnd__XAccessible
{
	CWnd__XAccessible_vtbl* __vftable /*VFT*/;
};

struct CWnd__XAccessibleServer;
struct /*VFT*/ CWnd__XAccessibleServer_vtbl
{
	unsigned int(__stdcall* AddRef)(CWnd__XAccessibleServer*);
	unsigned int(__stdcall* Release)(CWnd__XAccessibleServer*);
	HRESULT(__stdcall* QueryInterface)(CWnd__XAccessibleServer*, const _GUID*, void**);
	HRESULT(__stdcall* SetProxy)(CWnd__XAccessibleServer*, IAccessibleProxy*);
	HRESULT(__stdcall* GetHWND)(CWnd__XAccessibleServer*, HWND__**);
	HRESULT(__stdcall* GetEnumVariant)(CWnd__XAccessibleServer*, IEnumVARIANT**);
};

struct CWnd__XAccessibleServer
{
	CWnd__XAccessibleServer_vtbl* __vftable /*VFT*/;
};

class CWnd : public CCmdTarget
{
private:
	// Do not allow this class to be instanced
	CWnd();

public:
	HWND m_hWnd;
	bool m_bEnableActiveAccessibility;
	__declspec(align(4)) IAccessible* m_pStdObject;
	IAccessibleProxy* m_pProxy;
	CWnd__XAccessible m_xAccessible;
	CWnd__XAccessibleServer m_xAccessibleServer;
	HWND__* m_hWndOwner;
	unsigned int m_nFlags;
	int(__stdcall* m_pfnSuper)(HWND__*, unsigned int, unsigned int, int);
	int m_nModalResult;
	void* m_pDropTarget; // COleDropTarget
	COleControlContainer* m_pCtrlCont;
	COleControlSite* m_pCtrlSite;


	HWND GetWindow();
	BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL,
		struct CRgn* prgnUpdate = NULL,
		UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
};
STATIC_ASSERT_OFFSET(CWnd, m_hWnd, 0x20);
STATIC_ASSERT_SIZE(CWnd, 0x54);

struct CPtrArray;

struct CWnd_vtbl
{
	CRuntimeClass* (__thiscall* GetRuntimeClass)(CObject*);
	void(__thiscall* CObject_Destructor)(CObject);
	void(__thiscall* Dump)(CObject*, CDumpContext*);
	int(__thiscall* OnCmdMsg)(CCmdTarget*, unsigned int, int, void*, AFX_CMDHANDLERINFO*);
	void(__thiscall* OnFinalRelease)(CCmdTarget*);
	int(__thiscall* IsInvokeAllowed)(CCmdTarget*, int);
	int(__thiscall* GetDispatchIID)(CCmdTarget*, _GUID*);
	unsigned int(__thiscall* GetTypeInfoCount)(CCmdTarget*);
	CTypeLibCache* (__thiscall* GetTypeLibCache)(CCmdTarget*);
	HRESULT(__thiscall* GetTypeLib)(CCmdTarget*, unsigned int, ITypeLib**);
	const AFX_MSGMAP* (__thiscall* GetMessageMap)(CCmdTarget*);
	const AFX_OLECMDMAP* (__thiscall* GetCommandMap)(CCmdTarget*);
	const AFX_DISPMAP* (__thiscall* GetDispatchMap)(CCmdTarget*);
	const AFX_CONNECTIONMAP* (__thiscall* GetConnectionMap)(CCmdTarget*);
	const AFX_INTERFACEMAP* (__thiscall* GetInterfaceMap)(CCmdTarget*);
	const AFX_EVENTSINKMAP* (__thiscall* GetEventSinkMap)(CCmdTarget*);
	int(__thiscall* OnCreateAggregates)(CCmdTarget*);
	IUnknown* (__thiscall* GetInterfaceHook)(CCmdTarget*, const void*);
	int(__thiscall* GetExtraConnectionPoints)(CCmdTarget*, CPtrArray*);
	IConnectionPoint* (__thiscall* GetConnectionHook)(CCmdTarget*, const _GUID*);
	void(__thiscall* PreSubclassWindow)(CWnd*);
	int(__thiscall* Create)(CWnd*, const char*, const char*, unsigned int, const tagRECT*, CWnd*, unsigned int, CCreateContext*);
	BYTE gap60[4];
	int(__thiscall* CreateEx)(CWnd*, unsigned int, const char*, const char*, unsigned int, int, int, int, int, HWND__*, HMENU__*, void*);
	int(__thiscall* DestroyWindow)(CWnd*);
	int(__thiscall* PreCreateWindow)(CWnd*, tagCREATESTRUCTA*);
	void(__thiscall* CalcWindowRect)(CWnd*, tagRECT*, unsigned int);
	int(__thiscall* OnToolHitTest)(CWnd*, CPoint, tagTOOLINFOA*);
	CScrollBar* (__thiscall* GetScrollBarCtrl)(CWnd*, int);
	void(__thiscall* WinHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* HtmlHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* WinHelpInternal)(CWnd*, unsigned int, unsigned int);
	int(__thiscall* ContinueModal)(CWnd*);
	void(__thiscall* EndModalLoop)(CWnd*, int);
	HRESULT(__thiscall* EnsureStdObj)(CWnd*);
	HRESULT(__thiscall* get_accParent)(CWnd*, IDispatch**);
	HRESULT(__thiscall* get_accChildCount)(CWnd*, int*);
	HRESULT(__thiscall* get_accChild)(CWnd*, tagVARIANT, IDispatch**);
	HRESULT(__thiscall* get_accName)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accValue)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accDescription)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accRole)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accState)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accHelp)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accHelpTopic)(CWnd*, wchar_t**, tagVARIANT, int*);
	HRESULT(__thiscall* get_accKeyboardShortcut)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accFocus)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accSelection)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accDefaultAction)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* accSelect)(CWnd*, int, tagVARIANT);
	HRESULT(__thiscall* accLocation)(CWnd*, int*, int*, int*, int*, tagVARIANT);
	HRESULT(__thiscall* accNavigate)(CWnd*, int, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* accHitTest)(CWnd*, int, int, tagVARIANT*);
	HRESULT(__thiscall* accDoDefaultAction)(CWnd*, tagVARIANT);
	HRESULT(__thiscall* put_accName)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* put_accValue)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* SetProxy)(CWnd*, IAccessibleProxy*);
	HRESULT(__thiscall* CreateAccessibleProxy)(CWnd*, unsigned int, int, int*);
	int(__thiscall* OnCommand)(CWnd*, unsigned int, int);
	int(__thiscall* OnNotify)(CWnd*, unsigned int, int, int*);
	int(__stdcall** (__thiscall* GetSuperWndProcAddr)(CWnd*))(HWND__*, unsigned int, unsigned int, int);
	void(__thiscall* DoDataExchange)(CWnd*, CDataExchange*);
	void(__thiscall* BeginModalState)(CWnd*);
	void(__thiscall* EndModalState)(CWnd*);
	int(__thiscall* PreTranslateMessage)(CWnd*, tagMSG*);
	int(__thiscall* OnAmbientProperty)(CWnd*, COleControlSite*, int, tagVARIANT*);
	int(__thiscall* WindowProc)(CWnd*, unsigned int, unsigned int, int);
	int(__thiscall* OnWndMsg)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* DefWindowProcA)(CWnd*, unsigned int, unsigned int, int);
	void(__thiscall* PostNcDestroy)(CWnd*);
	int(__thiscall* OnChildNotify)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* CheckAutoCenter)(CWnd*);
	int(__thiscall* IsFrameWnd)(CWnd*);
	int(__thiscall* CreateControlContainer)(CWnd*, COleControlContainer**);
	int(__thiscall* CreateControlSite)(CWnd*, COleControlContainer*, COleControlSite**, unsigned int, const _GUID*);
	int(__thiscall* SetOccDialogInfo)(CWnd*, _AFX_OCC_DIALOG_INFO*);
};




struct CPtrList__CNode
{
	CPtrList__CNode* pNext;
	CPtrList__CNode* pPrev;
	void* data;
};

struct CPlex
{
	CPlex* pNext;
};

struct CPtrList : CObject
{
	CPtrList__CNode* m_pNodeHead;
	CPtrList__CNode* m_pNodeTail;
	int m_nCount;
	CPtrList__CNode* m_pNodeFree;
	CPlex* m_pBlocks;
	int m_nBlockSize;
};

struct CView : CWnd
{
	void* m_pDocument; // CDocument
};

struct CSimpleStringT
{
	char* m_pszData;
};

struct CStringT : CSimpleStringT
{
};

struct CArchive
{
};

struct CFile : CObject
{
	void* m_hFile;
	int m_bCloseOnDelete;
	CStringT m_strFileName;
};

struct CDumpContext
{
	int m_nDepth;
	CFile* m_pFile;
};

struct AFX_CLASSINIT
{
};

struct CRuntimeClass
{
	const char* m_lpszClassName;
	int m_nObjectSize;
	unsigned int m_wSchema;
	CObject* (__stdcall* m_pfnCreateObject)();
	CRuntimeClass* (__stdcall* m_pfnGetBaseClass)();
	CRuntimeClass* m_pNextClass;
	const AFX_CLASSINIT* m_pClassInit;
};

struct __declspec(align(4)) CFrameWnd : CWnd
{
	int m_bAutoMenuEnable;
	int m_nWindow;
	HMENU__* m_hMenuDefault;
	HACCEL__* m_hAccelTable;
	unsigned int m_dwPromptContext;
	int m_bHelpMode;
	CFrameWnd* m_pNextFrameWnd;
	CRect m_rectBorder;
	void* m_pNotifyHook; // COleFrameHook*
	CPtrList m_listControlBars;
	int m_nShowDelay;
	unsigned int m_nIDHelp;
	unsigned int m_nIDTracking;
	unsigned int m_nIDLastMessage;
	CView* m_pViewActive;
	int(__stdcall* m_lpfnCloseProc)(CFrameWnd*);
	unsigned int m_cModalStack;
	HWND** m_phWndDisable;
	HMENU__* m_hMenuAlt;
	CStringT m_strTitle;
	int m_bInRecalcLayout;
	CRuntimeClass* m_pFloatingFrameClass;
	unsigned int m_nIdleFlags;
};
STATIC_ASSERT_SIZE(CFrameWnd, 0xD4);

/* -------------------------------- */

class CDockBar;
struct CDockContext;
class CControlBar;

struct /*VFT*/ CControlBar_vtbl
{
	CRuntimeClass* (__thiscall* GetRuntimeClass)(CObject*);
	void(__thiscall * CObject_Destructor)(CObject*);
	void(__thiscall* Serialize)(CObject*, CArchive*);
	void(__thiscall* AssertValid)(CObject*);
	void(__thiscall* Dump)(CObject*, CDumpContext*);
	int(__thiscall* OnCmdMsg)(CCmdTarget*, unsigned int, int, void*, AFX_CMDHANDLERINFO*);
	void(__thiscall* OnFinalRelease)(CCmdTarget*);
	int(__thiscall* IsInvokeAllowed)(CCmdTarget*, int);
	int(__thiscall* GetDispatchIID)(CCmdTarget*, _GUID*);
	unsigned int(__thiscall* GetTypeInfoCount)(CCmdTarget*);
	CTypeLibCache* (__thiscall* GetTypeLibCache)(CCmdTarget*);
	HRESULT(__thiscall* GetTypeLib)(CCmdTarget*, unsigned int, ITypeLib**);
	const AFX_MSGMAP* (__thiscall* GetMessageMap)(CCmdTarget*);
	const AFX_OLECMDMAP* (__thiscall* GetCommandMap)(CCmdTarget*);
	const AFX_DISPMAP* (__thiscall* GetDispatchMap)(CCmdTarget*);
	const AFX_CONNECTIONMAP* (__thiscall* GetConnectionMap)(CCmdTarget*);
	const AFX_INTERFACEMAP* (__thiscall* GetInterfaceMap)(CCmdTarget*);
	const AFX_EVENTSINKMAP* (__thiscall* GetEventSinkMap)(CCmdTarget*);
	int(__thiscall* OnCreateAggregates)(CCmdTarget*);
	IUnknown* (__thiscall* GetInterfaceHook)(CCmdTarget*, const void*);
	int(__thiscall* GetExtraConnectionPoints)(CCmdTarget*, void*); // CPtrArray
	IConnectionPoint* (__thiscall* GetConnectionHook)(CCmdTarget*, const _GUID*);
	void(__thiscall* PreSubclassWindow)(CWnd*);
	int(__thiscall* Create)(CWnd*, const char*, const char*, unsigned int, const tagRECT*, CWnd*, unsigned int, CCreateContext*);
	BYTE gap60[4];
	int(__thiscall* CreateEx)(CWnd*, unsigned int, const char*, const char*, unsigned int, int, int, int, int, HWND__*, HMENU__*, void*);
	int(__thiscall* DestroyWindow)(CWnd*);
	int(__thiscall* PreCreateWindow)(CWnd*, tagCREATESTRUCTA*);
	void(__thiscall* CalcWindowRect)(CWnd*, tagRECT*, unsigned int);
	int(__thiscall* OnToolHitTest)(CWnd*, CPoint, tagTOOLINFOA*);
	CScrollBar* (__thiscall* GetScrollBarCtrl)(CWnd*, int);
	void(__thiscall* WinHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* HtmlHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* WinHelpInternal)(CWnd*, unsigned int, unsigned int);
	int(__thiscall* ContinueModal)(CWnd*);
	void(__thiscall* EndModalLoop)(CWnd*, int);
	HRESULT(__thiscall* EnsureStdObj)(CWnd*);
	HRESULT(__thiscall* get_accParent)(CWnd*, IDispatch**);
	HRESULT(__thiscall* get_accChildCount)(CWnd*, int*);
	HRESULT(__thiscall* get_accChild)(CWnd*, tagVARIANT, IDispatch**);
	HRESULT(__thiscall* get_accName)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accValue)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accDescription)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accRole)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accState)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accHelp)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accHelpTopic)(CWnd*, wchar_t**, tagVARIANT, int*);
	HRESULT(__thiscall* get_accKeyboardShortcut)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accFocus)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accSelection)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accDefaultAction)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* accSelect)(CWnd*, int, tagVARIANT);
	HRESULT(__thiscall* accLocation)(CWnd*, int*, int*, int*, int*, tagVARIANT);
	HRESULT(__thiscall* accNavigate)(CWnd*, int, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* accHitTest)(CWnd*, int, int, tagVARIANT*);
	HRESULT(__thiscall* accDoDefaultAction)(CWnd*, tagVARIANT);
	HRESULT(__thiscall* put_accName)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* put_accValue)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* SetProxy)(CWnd*, IAccessibleProxy*);
	HRESULT(__thiscall* CreateAccessibleProxy)(CWnd*, unsigned int, int, int*);
	int(__thiscall* OnCommand)(CWnd*, unsigned int, int);
	int(__thiscall* OnNotify)(CWnd*, unsigned int, int, int*);
	int(__stdcall** (__thiscall* GetSuperWndProcAddr)(CWnd*))(HWND__*, unsigned int, unsigned int, int);
	void(__thiscall* DoDataExchange)(CWnd*, CDataExchange*);
	void(__thiscall* BeginModalState)(CWnd*);
	void(__thiscall* EndModalState)(CWnd*);
	int(__thiscall* PreTranslateMessage)(CWnd*, tagMSG*);
	int(__thiscall* OnAmbientProperty)(CWnd*, COleControlSite*, int, tagVARIANT*);
	int(__thiscall* WindowProc)(CWnd*, unsigned int, unsigned int, int);
	int(__thiscall* OnWndMsg)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* DefWindowProcA)(CWnd*, unsigned int, unsigned int, int);
	void(__thiscall* PostNcDestroy)(CWnd*);
	int(__thiscall* OnChildNotify)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* CheckAutoCenter)(CWnd*);
	int(__thiscall* IsFrameWnd)(CWnd*);
	int(__thiscall* CreateControlContainer)(CWnd*, COleControlContainer**);
	int(__thiscall* CreateControlSite)(CWnd*, COleControlContainer*, COleControlSite**, unsigned int, const _GUID*);
	int(__thiscall* SetOccDialogInfo)(CWnd*, _AFX_OCC_DIALOG_INFO*);
	CSize* (__thiscall* CalcFixedLayout)(CControlBar*, CSize* result, int, int);
	CSize* (__thiscall* CalcDynamicLayout)(CControlBar*, CSize* result, int, unsigned int);
	void(__thiscall* OnUpdateCmdUI)(CControlBar*, CFrameWnd*, int);
	void(__thiscall* CalcInsideRect)(CControlBar*, CRect*, int);
	void(__thiscall* DoPaint)(CControlBar*, CDC*);
	void(__thiscall* DrawBorders)(CControlBar*, CDC*, CRect*);
	void(__thiscall* DrawGripper)(CControlBar*, CDC*, const CRect*);
	void(__thiscall* DelayShow)(CControlBar*, int);
	int(__thiscall* IsVisible)(CControlBar*);
	unsigned int(__thiscall* RecalcDelayShow)(CControlBar*, AFX_SIZEPARENTPARAMS*);
	int(__thiscall* IsDockBar)(CControlBar*);
	void(__thiscall* OnBarStyleChange)(CControlBar*, unsigned int, unsigned int);
	int(__thiscall* SetStatusText)(CControlBar*, int);
};
STATIC_ASSERT_OFFSET(CControlBar_vtbl, SetStatusText, 0x168);
//STATIC_ASSERT_SIZE(CWnd, 0x54);


class CControlBar : public CWnd
{
private:
	// Do not allow this class to be instanced
	//CControlBar() = delete;
	//~CStatusBar() = delete;

public:
	CControlBar() = delete;
	CWnd* m_pInPlaceOwner;
	int m_bAutoDelete;
	int m_cxLeftBorder;
	int m_cxRightBorder;
	int m_cyTopBorder;
	int m_cyBottomBorder;
	int m_cxDefaultGap;
	unsigned int m_nMRUWidth;
	int m_nCount;
	void* m_pData;
	unsigned int m_nStateFlags;
	unsigned int m_dwStyle;
	unsigned int m_dwDockStyle;
	CFrameWnd* m_pDockSite;
	CDockBar* m_pDockBar;
	CDockContext* m_pDockContext;
};
STATIC_ASSERT_SIZE(CControlBar, 0x94);


struct CPtrArray : CObject
{
	void** m_pData;
	int m_nSize;
	int m_nMaxSize;
	int m_nGrowBy;
};

//struct CDockBar : CControlBar
//{
//	int m_bFloating;
//	CPtrArray m_arrBars;
//	int m_bLayoutQuery;
//	CRect m_rectLayout;
//};

class CDockBar : public CControlBar
{
private:
	// Do not allow this class to be instanced
	CDockBar() = delete;
	//~CStatusBar() = delete;

public:
	int m_bFloating;
	CPtrArray m_arrBars;
	int m_bLayoutQuery;
	CRect m_rectLayout;
};

struct CDockContext;

struct CDockContext_vtbl
{
	void(__thiscall* StartDrag)(CDockContext*, CPoint);
	void(__thiscall* StartResize)(CDockContext*, int, CPoint);
	void(__thiscall* ToggleDocking)(CDockContext*);
};

struct CDC_new : CObject
{
	HDC__* m_hDC;
	HDC__* m_hAttribDC;
	int m_bPrinting;
};

struct CDockContext
{
	CDockContext_vtbl* x_vftable;
	CPoint m_ptLast;
	CRect m_rectLast;
	CSize m_sizeLast;
	int m_bDitherLast;
	CRect m_rectDragHorz;
	CRect m_rectDragVert;
	CRect m_rectFrameDragHorz;
	CRect m_rectFrameDragVert;
	CControlBar* m_pBar;
	CFrameWnd* m_pDockSite;
	unsigned int m_dwDockStyle;
	unsigned int m_dwOverDockStyle;
	unsigned int m_dwStyle;
	int m_bFlip;
	int m_bForceFrame;
	CDC_new* m_pDC;
	int m_bDragging;
	int m_nHitTest;
	unsigned int m_uMRUDockID;
	CRect m_rectMRUDockPos;
	unsigned int m_dwMRUFloatStyle;
	CPoint m_ptMRUFloatPos;
};

class CStatusBar;

//struct CStatusBar_vtbl
//{
//	CRuntimeClass* (__thiscall* GetRuntimeClass)(CObject*);
//	//void(__thiscall * ~CObject)(CObject*);
//	void(__thiscall * CObject_destructor)(CObject*);
//	void(__thiscall* Serialize)(CObject*, CArchive*);
//	void(__thiscall* AssertValid)(CObject*);
//	void(__thiscall* Dump)(CObject*, CDumpContext*);
//	int(__thiscall* OnCmdMsg)(CCmdTarget*, unsigned int, int, void*, AFX_CMDHANDLERINFO*);
//	void(__thiscall* OnFinalRelease)(CCmdTarget*);
//	int(__thiscall* IsInvokeAllowed)(CCmdTarget*, int);
//	int(__thiscall* GetDispatchIID)(CCmdTarget*, _GUID*);
//	unsigned int(__thiscall* GetTypeInfoCount)(CCmdTarget*);
//	CTypeLibCache* (__thiscall* GetTypeLibCache)(CCmdTarget*);
//	HRESULT(__thiscall* GetTypeLib)(CCmdTarget*, unsigned int, ITypeLib**);
//	const AFX_MSGMAP* (__thiscall* GetMessageMap)(CCmdTarget*);
//	const AFX_OLECMDMAP* (__thiscall* GetCommandMap)(CCmdTarget*);
//	const AFX_DISPMAP* (__thiscall* GetDispatchMap)(CCmdTarget*);
//	const AFX_CONNECTIONMAP* (__thiscall* GetConnectionMap)(CCmdTarget*);
//	const AFX_INTERFACEMAP* (__thiscall* GetInterfaceMap)(CCmdTarget*);
//	const AFX_EVENTSINKMAP* (__thiscall* GetEventSinkMap)(CCmdTarget*);
//	int(__thiscall* OnCreateAggregates)(CCmdTarget*);
//	IUnknown* (__thiscall* GetInterfaceHook)(CCmdTarget*, const void*);
//	int(__thiscall* GetExtraConnectionPoints)(CCmdTarget*, CPtrArray*);
//	IConnectionPoint* (__thiscall* GetConnectionHook)(CCmdTarget*, const _GUID*);
//	void(__thiscall* PreSubclassWindow)(CWnd*);
//	int(__thiscall* Create)(CWnd*, const char*, const char*, unsigned int, const tagRECT*, CWnd*, unsigned int, CCreateContext*);
//	char gap60[4];
//	int(__thiscall* CreateEx)(CWnd*, unsigned int, const char*, const char*, unsigned int, int, int, int, int, HWND__*, HMENU__*, void*);
//	int(__thiscall* DestroyWindow)(CWnd*);
//	int(__thiscall* PreCreateWindow)(CWnd*, tagCREATESTRUCTA*);
//	void(__thiscall* CalcWindowRect)(CWnd*, tagRECT*, unsigned int);
//	int(__thiscall* OnToolHitTest)(CWnd*, CPoint, tagTOOLINFOA*);
//	CScrollBar* (__thiscall* GetScrollBarCtrl)(CWnd*, int);
//	void(__thiscall* WinHelpA)(CWnd*, unsigned int, unsigned int);
//	void(__thiscall* HtmlHelpA)(CWnd*, unsigned int, unsigned int);
//	void(__thiscall* WinHelpInternal)(CWnd*, unsigned int, unsigned int);
//	int(__thiscall* ContinueModal)(CWnd*);
//	void(__thiscall* EndModalLoop)(CWnd*, int);
//	HRESULT(__thiscall* EnsureStdObj)(CWnd*);
//	HRESULT(__thiscall* get_accParent)(CWnd*, IDispatch**);
//	HRESULT(__thiscall* get_accChildCount)(CWnd*, int*);
//	HRESULT(__thiscall* get_accChild)(CWnd*, tagVARIANT, IDispatch**);
//	HRESULT(__thiscall* get_accName)(CWnd*, tagVARIANT, wchar_t**);
//	HRESULT(__thiscall* get_accValue)(CWnd*, tagVARIANT, wchar_t**);
//	HRESULT(__thiscall* get_accDescription)(CWnd*, tagVARIANT, wchar_t**);
//	HRESULT(__thiscall* get_accRole)(CWnd*, tagVARIANT, tagVARIANT*);
//	HRESULT(__thiscall* get_accState)(CWnd*, tagVARIANT, tagVARIANT*);
//	HRESULT(__thiscall* get_accHelp)(CWnd*, tagVARIANT, wchar_t**);
//	HRESULT(__thiscall* get_accHelpTopic)(CWnd*, wchar_t**, tagVARIANT, int*);
//	HRESULT(__thiscall* get_accKeyboardShortcut)(CWnd*, tagVARIANT, wchar_t**);
//	HRESULT(__thiscall* get_accFocus)(CWnd*, tagVARIANT*);
//	HRESULT(__thiscall* get_accSelection)(CWnd*, tagVARIANT*);
//	HRESULT(__thiscall* get_accDefaultAction)(CWnd*, tagVARIANT, wchar_t**);
//	HRESULT(__thiscall* accSelect)(CWnd*, int, tagVARIANT);
//	HRESULT(__thiscall* accLocation)(CWnd*, int*, int*, int*, int*, tagVARIANT);
//	HRESULT(__thiscall* accNavigate)(CWnd*, int, tagVARIANT, tagVARIANT*);
//	HRESULT(__thiscall* accHitTest)(CWnd*, int, int, tagVARIANT*);
//	HRESULT(__thiscall* accDoDefaultAction)(CWnd*, tagVARIANT);
//	HRESULT(__thiscall* put_accName)(CWnd*, tagVARIANT, wchar_t*);
//	HRESULT(__thiscall* put_accValue)(CWnd*, tagVARIANT, wchar_t*);
//	HRESULT(__thiscall* SetProxy)(CWnd*, IAccessibleProxy*);
//	HRESULT(__thiscall* CreateAccessibleProxy)(CWnd*, unsigned int, int, int*);
//	int(__thiscall* OnCommand)(CWnd*, unsigned int, int);
//	int(__thiscall* OnNotify)(CWnd*, unsigned int, int, int*);
//	int(__stdcall** (__thiscall* GetSuperWndProcAddr)(CWnd* ))(HWND__*, unsigned int, unsigned int, int);
//	void(__thiscall* DoDataExchange)(CWnd*, CDataExchange*);
//	void(__thiscall* BeginModalState)(CWnd*);
//	void(__thiscall* EndModalState)(CWnd*);
//	int(__thiscall* PreTranslateMessage)(CWnd*, tagMSG*);
//	int(__thiscall* OnAmbientProperty)(CWnd*, COleControlSite*, int, tagVARIANT*);
//	int(__thiscall* WindowProc)(CWnd*, unsigned int, unsigned int, int);
//	int(__thiscall* OnWndMsg)(CWnd*, unsigned int, unsigned int, int, int*);
//	int(__thiscall* DefWindowProcA)(CWnd*, unsigned int, unsigned int, int);
//	void(__thiscall* PostNcDestroy)(CWnd*);
//	int(__thiscall* OnChildNotify)(CWnd*, unsigned int, unsigned int, int, int*);
//	int(__thiscall* CheckAutoCenter)(CWnd*);
//	int(__thiscall* IsFrameWnd)(CWnd*);
//	int(__thiscall* CreateControlContainer)(CWnd*, COleControlContainer**);
//	int(__thiscall* CreateControlSite)(CWnd*, COleControlContainer*, COleControlSite**, unsigned int, const _GUID*);
//	int(__thiscall* SetOccDialogInfo)(CWnd*, _AFX_OCC_DIALOG_INFO*);
//	CSize* (__thiscall* CalcFixedLayout)(CControlBar*, CSize* result, int, int);
//	CSize* (__thiscall* CalcDynamicLayout)(CControlBar*, CSize* result, int, unsigned int);
//	void(__thiscall* OnUpdateCmdUI)(CControlBar*, CFrameWnd*, int);
//	void(__thiscall* CalcInsideRect)(CControlBar*, CRect*, int);
//	void(__thiscall* DoPaint)(CControlBar*, CDC*);
//	void(__thiscall* DrawBorders)(CControlBar*, CDC*, CRect*);
//	void(__thiscall* DrawGripper)(CControlBar*, CDC*, const CRect*);
//	void(__thiscall* DelayShow)(CControlBar*, int);
//	int(__thiscall* IsVisible)(CControlBar*);
//	unsigned int(__thiscall* RecalcDelayShow)(CControlBar*, AFX_SIZEPARENTPARAMS*);
//	int(__thiscall* IsDockBar)(CControlBar*);
//	void(__thiscall* OnBarStyleChange)(CControlBar*, unsigned int, unsigned int);
//	int(__thiscall* SetStatusText)(CControlBar*, int);
//	int(__thiscall* Create_x)(CStatusBar*, CWnd*, unsigned int, unsigned int);
//	int(__thiscall* CreateEx_x)(CStatusBar*, CWnd*, unsigned int, unsigned int, unsigned int);
//	void(__thiscall* DrawItem)(CStatusBar*, tagDRAWITEMSTRUCT*);
//};
//STATIC_ASSERT_OFFSET(CStatusBar_vtbl, SetStatusText, 0x168);
//STATIC_ASSERT_SIZE(CStatusBar_vtbl, 0x178);

struct CStatusBar_vtbl
{
	CRuntimeClass* (__thiscall* GetRuntimeClass)(CObject*);
	void(__thiscall* CObject_destructor)(CObject*);
	int gap01;
	int(__thiscall* OnCmdMsg)(CCmdTarget*, unsigned int, int, void*, AFX_CMDHANDLERINFO*);
	void(__thiscall* OnFinalRelease)(CCmdTarget*);
	int(__thiscall* IsInvokeAllowed)(CCmdTarget*, int);
	int(__thiscall* GetDispatchIID)(CCmdTarget*, _GUID*);
	unsigned int(__thiscall* GetTypeInfoCount)(CCmdTarget*);
	CTypeLibCache* (__thiscall* GetTypeLibCache)(CCmdTarget*);
	HRESULT(__thiscall* GetTypeLib)(CCmdTarget*, unsigned int, ITypeLib**);
	const AFX_MSGMAP* (__thiscall* GetMessageMap)(CCmdTarget*);
	const AFX_OLECMDMAP* (__thiscall* GetCommandMap)(CCmdTarget*);
	const AFX_DISPMAP* (__thiscall* GetDispatchMap)(CCmdTarget*);
	const AFX_CONNECTIONMAP* (__thiscall* GetConnectionMap)(CCmdTarget*);
	const AFX_INTERFACEMAP* (__thiscall* GetInterfaceMap)(CCmdTarget*);
	const AFX_EVENTSINKMAP* (__thiscall* GetEventSinkMap)(CCmdTarget*);
	int(__thiscall* OnCreateAggregates)(CCmdTarget*);
	IUnknown* (__thiscall* GetInterfaceHook)(CCmdTarget*, const void*);
	int(__thiscall* GetExtraConnectionPoints)(CCmdTarget*, CPtrArray*);
	IConnectionPoint* (__thiscall* GetConnectionHook)(CCmdTarget*, const _GUID*);
	void(__thiscall* PreSubclassWindow)(CWnd*);
	int(__thiscall* Create)(CWnd*, const char*, const char*, unsigned int, const tagRECT*, CWnd*, unsigned int, CCreateContext*);
	char gap60[4];
	int(__thiscall* CreateEx)(CWnd*, unsigned int, const char*, const char*, unsigned int, int, int, int, int, HWND__*, HMENU__*, void*);
	int(__thiscall* DestroyWindow)(CWnd*);
	int(__thiscall* PreCreateWindow)(CWnd*, tagCREATESTRUCTA*);
	void(__thiscall* CalcWindowRect)(CWnd*, tagRECT*, unsigned int);
	int(__thiscall* OnToolHitTest)(CWnd*, CPoint, tagTOOLINFOA*);
	CScrollBar* (__thiscall* GetScrollBarCtrl)(CWnd*, int);
	void(__thiscall* WinHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* HtmlHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* WinHelpInternal)(CWnd*, unsigned int, unsigned int);
	int(__thiscall* ContinueModal)(CWnd*);
	void(__thiscall* EndModalLoop)(CWnd*, int);
	HRESULT(__thiscall* EnsureStdObj)(CWnd*); // 
	HRESULT(__thiscall* get_accParent)(CWnd*, IDispatch**);
	HRESULT(__thiscall* get_accChildCount)(CWnd*, int*);
	HRESULT(__thiscall* get_accChild)(CWnd*, tagVARIANT, IDispatch**);
	HRESULT(__thiscall* get_accName)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accValue)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accDescription)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accRole)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accState)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accHelp)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accHelpTopic)(CWnd*, wchar_t**, tagVARIANT, int*);
	HRESULT(__thiscall* get_accKeyboardShortcut)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accFocus)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accSelection)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accDefaultAction)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* accSelect)(CWnd*, int, tagVARIANT);
	HRESULT(__thiscall* accLocation)(CWnd*, int*, int*, int*, int*, tagVARIANT);
	HRESULT(__thiscall* accNavigate)(CWnd*, int, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* accHitTest)(CWnd*, int, int, tagVARIANT*); //
	HRESULT(__thiscall* accDoDefaultAction)(CWnd*, tagVARIANT);
	HRESULT(__thiscall* put_accName)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* put_accValue)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* SetProxy)(CWnd*, IAccessibleProxy*);
	HRESULT(__thiscall* CreateAccessibleProxy)(CWnd*, unsigned int, int, int*);
	int(__thiscall* OnCommand)(CWnd*, unsigned int, int);
	int(__thiscall* OnNotify)(CWnd*, unsigned int, int, int*);
	int(__stdcall** (__thiscall* GetSuperWndProcAddr)(CWnd*))(HWND__*, unsigned int, unsigned int, int);
	void(__thiscall* DoDataExchange)(CWnd*, CDataExchange*);
	void(__thiscall* BeginModalState)(CWnd*);
	void(__thiscall* EndModalState)(CWnd*);
	int(__thiscall* PreTranslateMessage)(CWnd*, tagMSG*);
	int(__thiscall* OnAmbientProperty)(CWnd*, COleControlSite*, int, tagVARIANT*);
	int(__thiscall* WindowProc)(CWnd*, unsigned int, unsigned int, int);
	int(__thiscall* OnWndMsg)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* DefWindowProcA)(CWnd*, unsigned int, unsigned int, int);
	void(__thiscall* PostNcDestroy)(CWnd*);
	int(__thiscall* OnChildNotify)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* CheckAutoCenter)(CWnd*);
	int(__thiscall* IsFrameWnd)(CWnd*);
	int(__thiscall* CreateControlContainer)(CWnd*, COleControlContainer**);
	int(__thiscall* CreateControlSite)(CWnd*, COleControlContainer*, COleControlSite**, unsigned int, const _GUID*);
	int(__thiscall* SetOccDialogInfo)(CWnd*, _AFX_OCC_DIALOG_INFO*);
	int gap02;
	CSize* (__thiscall* CalcFixedLayout)(CControlBar*, CSize* result, int, int);
	CSize* (__thiscall* CalcDynamicLayout)(CControlBar*, CSize* result, int, unsigned int);
	void(__thiscall* OnUpdateCmdUI)(CControlBar*, CFrameWnd*, int);
	void(__thiscall* CalcInsideRect)(CControlBar*, CRect*, int);
	void(__thiscall* DoPaint)(CControlBar*, CDC*);
	void(__thiscall* DrawBorders)(CControlBar*, CDC*, CRect*);
	void(__thiscall* DrawGripper)(CControlBar*, CDC*, const CRect*);
	void(__thiscall* DelayShow)(CControlBar*, int);
	int(__thiscall* IsVisible)(CControlBar*);
	unsigned int(__thiscall* RecalcDelayShow)(CControlBar*, AFX_SIZEPARENTPARAMS*);
	int(__thiscall* IsDockBar)(CControlBar*);
	void(__thiscall* OnBarStyleChange)(CControlBar*, unsigned int, unsigned int);
	int(__thiscall* SetStatusText)(CControlBar*, int);
	int(__thiscall* Create_x)(CStatusBar*, CWnd*, unsigned int, unsigned int);
	int(__thiscall* CreateEx_x)(CStatusBar*, CWnd*, unsigned int, unsigned int, unsigned int);
	void(__thiscall* DrawItem)(CStatusBar*, tagDRAWITEMSTRUCT*);
};
STATIC_ASSERT_OFFSET(CStatusBar_vtbl, SetStatusText, 0x164);

class CStatusBar : public CControlBar
{
private:
	// Do not allow this class to be instanced
	//CStatusBar() = delete;
	//~CStatusBar() = delete;
	
public:
	CStatusBar() = delete;
	int m_nMinHeight;
};

struct CToolBar : CControlBar
{
	HRSRC__* m_hRsrcImageWell;
	HINSTANCE__* m_hInstImageWell;
	HBITMAP__* m_hbmImageWell;
	int m_bDelayedButtonLayout;
	CSize m_sizeImage;
	CSize m_sizeButton;
	void* m_pStringMap; // CMapStringToPtr
};


struct CLstToolBar : CToolBar
{
};

struct __declspec(align(4)) CDialogBar : public CControlBar
{
	CSize m_sizeDefault;
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
	const char* m_lpszTemplateName;
};

struct CSpinButtonCtrl : CWnd
{
};

struct CTextureBar : public CDialogBar
{
	CSpinButtonCtrl m_spinRotate;
	CSpinButtonCtrl m_spinVScale;
	CSpinButtonCtrl m_spinVShift;
	CSpinButtonCtrl m_spinHScale;
	CSpinButtonCtrl m_spinHShift;
	int m_nHShift;
	int m_nHScale;
	int m_nRotate;
	int m_nVShift;
	int m_nVScale;
	int m_nRotateAmt;
};

struct CSplitterWnd__CRowColInfo
{
	int nMinSize;
	int nIdealSize;
	int nCurSize;
};

struct CSplitterWnd : public CWnd
{
	CRuntimeClass* m_pDynamicViewClass;
	int m_nMaxRows;
	int m_nMaxCols;
	int m_cxSplitter;
	int m_cySplitter;
	int m_cxBorderShare;
	int m_cyBorderShare;
	int m_cxSplitterGap;
	int m_cySplitterGap;
	int m_cxBorder;
	int m_cyBorder;
	int m_nRows;
	int m_nCols;
	int m_bHasHScroll;
	int m_bHasVScroll;
	CSplitterWnd__CRowColInfo* m_pColInfo;
	CSplitterWnd__CRowColInfo* m_pRowInfo;
	int m_bTracking;
	int m_bTracking2;
	CPoint m_ptTrackOffset;
	CRect m_rectLimit;
	CRect m_rectTracker;
	CRect m_rectTracker2;
	int m_htTrack;
};

enum CSplitterWnd__ESplitType : __int32
{
	splitBox = 0x0,
	splitBar = 0x1,
	splitIntersection = 0x2,
	splitBorder = 0x3,
};

struct CSplitterWnd_vtbl
{
	CRuntimeClass* (__thiscall* GetRuntimeClass)(CObject*);
	void(__thiscall* CObject_Destructor)(CObject*);
	void(__thiscall* Dump)(CObject*, CDumpContext*);
	int(__thiscall* OnCmdMsg)(CCmdTarget*, unsigned int, int, void*, AFX_CMDHANDLERINFO*);
	void(__thiscall* OnFinalRelease)(CCmdTarget*);
	int(__thiscall* IsInvokeAllowed)(CCmdTarget*, int);
	int(__thiscall* GetDispatchIID)(CCmdTarget*, _GUID*);
	unsigned int(__thiscall* GetTypeInfoCount)(CCmdTarget*);
	CTypeLibCache* (__thiscall* GetTypeLibCache)(CCmdTarget*);
	HRESULT(__thiscall* GetTypeLib)(CCmdTarget*, unsigned int, ITypeLib**);
	const AFX_MSGMAP* (__thiscall* GetMessageMap)(CCmdTarget*);
	const AFX_OLECMDMAP* (__thiscall* GetCommandMap)(CCmdTarget*);
	const AFX_DISPMAP* (__thiscall* GetDispatchMap)(CCmdTarget*);
	const AFX_CONNECTIONMAP* (__thiscall* GetConnectionMap)(CCmdTarget*);
	const AFX_INTERFACEMAP* (__thiscall* GetInterfaceMap)(CCmdTarget*);
	const AFX_EVENTSINKMAP* (__thiscall* GetEventSinkMap)(CCmdTarget*);
	int(__thiscall* OnCreateAggregates)(CCmdTarget*);
	IUnknown* (__thiscall* GetInterfaceHook)(CCmdTarget*, const void*);
	int(__thiscall* GetExtraConnectionPoints)(CCmdTarget*, CPtrArray*);
	IConnectionPoint* (__thiscall* GetConnectionHook)(CCmdTarget*, const _GUID*);
	void(__thiscall* PreSubclassWindow)(CWnd*);
	int(__thiscall* Create)(CWnd*, const char*, const char*, unsigned int, const tagRECT*, CWnd*, unsigned int, CCreateContext*);
	int gap01;
	int(__thiscall* CreateEx)(CWnd*, unsigned int, const char*, const char*, unsigned int, int, int, int, int, HWND__*, HMENU__*, void*);
	int(__thiscall* DestroyWindow)(CWnd*);
	int(__thiscall* PreCreateWindow)(CWnd*, tagCREATESTRUCTA*);
	void(__thiscall* CalcWindowRect)(CWnd*, tagRECT*, unsigned int);
	int(__thiscall* OnToolHitTest)(CWnd*, CPoint, tagTOOLINFOA*);
	CScrollBar* (__thiscall* GetScrollBarCtrl)(CWnd*, int);
	void(__thiscall* WinHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* HtmlHelpA)(CWnd*, unsigned int, unsigned int);
	void(__thiscall* WinHelpInternal)(CWnd*, unsigned int, unsigned int);
	int(__thiscall* ContinueModal)(CWnd*);
	void(__thiscall* EndModalLoop)(CWnd*, int);
	HRESULT(__thiscall* EnsureStdObj)(CWnd*);
	HRESULT(__thiscall* get_accParent)(CWnd*, IDispatch**);
	HRESULT(__thiscall* get_accChildCount)(CWnd*, int*);
	HRESULT(__thiscall* get_accChild)(CWnd*, tagVARIANT, IDispatch**);
	HRESULT(__thiscall* get_accName)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accValue)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accDescription)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accRole)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accState)(CWnd*, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* get_accHelp)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accHelpTopic)(CWnd*, wchar_t**, tagVARIANT, int*);
	HRESULT(__thiscall* get_accKeyboardShortcut)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* get_accFocus)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accSelection)(CWnd*, tagVARIANT*);
	HRESULT(__thiscall* get_accDefaultAction)(CWnd*, tagVARIANT, wchar_t**);
	HRESULT(__thiscall* accSelect)(CWnd*, int, tagVARIANT);
	HRESULT(__thiscall* accLocation)(CWnd*, int*, int*, int*, int*, tagVARIANT);
	HRESULT(__thiscall* accNavigate)(CWnd*, int, tagVARIANT, tagVARIANT*);
	HRESULT(__thiscall* accHitTest)(CWnd*, int, int, tagVARIANT*);
	HRESULT(__thiscall* accDoDefaultAction)(CWnd*, tagVARIANT);
	HRESULT(__thiscall* put_accName)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* put_accValue)(CWnd*, tagVARIANT, wchar_t*);
	HRESULT(__thiscall* SetProxy)(CWnd*, IAccessibleProxy*);
	HRESULT(__thiscall* CreateAccessibleProxy)(CWnd*, unsigned int, int, int*);
	int(__thiscall* OnCommand)(CWnd*, unsigned int, int);
	int(__thiscall* OnNotify)(CWnd*, unsigned int, int, int*);
	int(__stdcall** (__thiscall* GetSuperWndProcAddr)(CWnd*))(HWND__*, unsigned int, unsigned int, int);
	void(__thiscall* DoDataExchange)(CWnd*, CDataExchange*);
	void(__thiscall* BeginModalState)(CWnd*);
	void(__thiscall* EndModalState)(CWnd*);
	int(__thiscall* PreTranslateMessage)(CWnd*, tagMSG*);
	int(__thiscall* OnAmbientProperty)(CWnd*, COleControlSite*, int, tagVARIANT*);
	int(__thiscall* WindowProc)(CWnd*, unsigned int, unsigned int, int);
	int(__thiscall* OnWndMsg)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* DefWindowProcA)(CWnd*, unsigned int, unsigned int, int);
	void(__thiscall* PostNcDestroy)(CWnd*);
	int(__thiscall* OnChildNotify)(CWnd*, unsigned int, unsigned int, int, int*);
	int(__thiscall* CheckAutoCenter)(CWnd*);
	int(__thiscall* IsFrameWnd)(CWnd*);
	int(__thiscall* CreateControlContainer)(CWnd*, COleControlContainer**);
	int(__thiscall* CreateControlSite)(CWnd*, COleControlContainer*, COleControlSite**, unsigned int, const _GUID*);
	int(__thiscall* SetOccDialogInfo)(CWnd*, _AFX_OCC_DIALOG_INFO*);
	int gap02;
	int(__thiscall* Create_02)(CSplitterWnd*, CWnd*, int, int, tagSIZE, CCreateContext*, unsigned int, unsigned int);
	int(__thiscall* CreateStatic)(CSplitterWnd*, CWnd*, int, int, unsigned int, unsigned int);
	int(__thiscall* CreateView)(CSplitterWnd*, int, int, CRuntimeClass*, tagSIZE, CCreateContext*);
	void(__thiscall* RecalcLayout)(CSplitterWnd*);
	void(__thiscall* OnDrawSplitter)(CSplitterWnd*, CDC*, CSplitterWnd__ESplitType, const CRect*);
	void(__thiscall* OnInvertTracker)(CSplitterWnd*, const CRect*);
	int(__thiscall* CreateScrollBarCtrl)(CSplitterWnd*, unsigned int, unsigned int);
	void(__thiscall* DeleteView)(CSplitterWnd*, int, int);
	int(__thiscall* SplitRow)(CSplitterWnd*, int);
	int(__thiscall* SplitColumn)(CSplitterWnd*, int);
	void(__thiscall* DeleteRow)(CSplitterWnd*, int);
	void(__thiscall* DeleteColumn)(CSplitterWnd*, int);
	CWnd* (__thiscall* GetActivePane)(CSplitterWnd*, int*, int*);
	void(__thiscall* SetActivePane)(CSplitterWnd*, int, int, CWnd*);
	int(__thiscall* CanActivateNext)(CSplitterWnd*, int);
	void(__thiscall* ActivateNext)(CSplitterWnd*, int);
	int(__thiscall* DoKeyboardSplit)(CSplitterWnd*);
	int(__thiscall* DoScroll)(CSplitterWnd*, CView*, unsigned int, int);
	int(__thiscall* DoScrollBy)(CSplitterWnd*, CView*, CSize, int);
	int(__thiscall* HitTest)(CSplitterWnd*, CPoint);
	void(__thiscall* GetInsideRect)(CSplitterWnd*, CRect*);
	void(__thiscall* GetHitRect)(CSplitterWnd*, int, CRect*);
	void(__thiscall* TrackRowSize)(CSplitterWnd*, int, int);
	void(__thiscall* TrackColumnSize)(CSplitterWnd*, int, int);
	void(__thiscall* DrawAllSplitBars)(CSplitterWnd*, CDC*, int, int);
	void(__thiscall* SetSplitCursor)(CSplitterWnd*, int);
	void(__thiscall* StartTracking)(CSplitterWnd*, int);
	void(__thiscall* StopTracking)(CSplitterWnd*, int);
};


struct  __declspec(align(4)) CDialog : CWnd
{
	unsigned int m_nIDHelp;
	const char* m_lpszTemplateName;
	void* m_hDialogTemplate;
	const DLGTEMPLATE* m_lpDialogTemplate;
	void* m_lpDialogInit;
	CWnd* m_pParentWnd;
	HWND__* m_hWndTop;
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
};

struct CGroupDlg : public CDialog
{
};

namespace afx
{
	// *
	// -------------------------------------- function typedefs -------------------------------------------
	// *
	
	typedef CWnd* (__thiscall* CSplitterWnd__GetPane_t)(CSplitterWnd*, int row, int col);
	extern CSplitterWnd__GetPane_t CSplitterWnd__GetPane;

	typedef void(__thiscall* CSplitterWnd__SetRowInfo_t)(CSplitterWnd*, int row, int, int);
	extern CSplitterWnd__SetRowInfo_t CSplitterWnd__SetRowInfo;
	
}