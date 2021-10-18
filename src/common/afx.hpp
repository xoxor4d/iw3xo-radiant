#pragma once
//#include <afxwin.h>

struct AFX_MODULE_STATE;

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

struct CMenu : CObject
{
	HMENU__* m_hMenu;
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
	AFX_MODULE_STATE* m_pModuleState; // AFX_MODULE_STATE
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

struct /*VFT*/ CFrameWnd_vtbl
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
	int(__thiscall* Create_02)(CFrameWnd*, const char*, const char*, unsigned int, const tagRECT*, CWnd*, const char*, unsigned int, CCreateContext*);
	int(__thiscall* LoadFrame)(CFrameWnd*, unsigned int, unsigned int, CWnd*, CCreateContext*);
	void* (__thiscall* GetActiveDocument)(CFrameWnd*); // CDocument*
	CFrameWnd* (__thiscall* GetActiveFrame)(CFrameWnd*);
	void(__thiscall* GetMessageString)(CFrameWnd*, unsigned int, void* missing_arg);
	void(__thiscall* RecalcLayout)(CFrameWnd*, int);
	void(__thiscall* ActivateFrame)(CFrameWnd*, int);
	void(__thiscall* OnSetPreviewMode)(CFrameWnd*, int, void*); // CPrintPreviewState*
	CWnd* (__thiscall* GetMessageBar)(CFrameWnd*);
	int(__thiscall* NegotiateBorderSpace)(CFrameWnd*, unsigned int, tagRECT*);
	int(__thiscall* OnCreateClient)(CFrameWnd*, tagCREATESTRUCTA*, CCreateContext*);
	void(__thiscall* OnUpdateFrameTitle)(CFrameWnd*, int);
	void(__thiscall* OnUpdateFrameMenu)(CFrameWnd*, HMENU__*);
	HACCEL__* (__thiscall* GetDefaultAccelerator)(CFrameWnd*);
	void(__thiscall* DelayUpdateFrameMenu)(CFrameWnd*, HMENU__*);
	void(__thiscall* ExitHelpMode)(CFrameWnd*);
};

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

struct CToolBar;

struct CToolBar_vtbl
{
	CRuntimeClass* (__thiscall* GetRuntimeClass)(CObject*);
	void(__thiscall* CObject_destructor)(CObject*);
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
	int pad;
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
	int(__thiscall* Create2)(CToolBar*, CWnd*, unsigned int, unsigned int);
	int(__thiscall* CreateEx2)(CToolBar*, CWnd*, unsigned int, unsigned int, CRect, unsigned int);
	void(__thiscall* GetItemRect)(CToolBar*, int, tagRECT*);
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

struct settings_prefab_s
{
	void* unk;
};

struct __declspec(align(4)) CPrefsDlg : public CDialog
{
	int x0;
	int x1;
	int x2;
	int x3;
	int x4;
	int x5;
	int x6;
	int x7;
	int x8;
	int x9;
	int x10;
	int x11;
	int x12;
	int x13;
	int x14;
	int x15;
	int x16;
	int x17;
	int x18;
	int x19;
	int x20;
	int x21;
	int x22;
	int x23;
	int x24;
	int x25;
	int x26;
	int x27;
	int x28;
	int x29;
	int x30;
	int x31;
	int x32;
	int x33;
	int x34;
	int x35;
	int x36;
	int x37;
	int x38;
	int x39;
	int x40;
	int x41;
	int x42;
	int x43;
	int x44;
	int x45;
	int x46;
	int x47;
	int x48;
	int x49;
	int x50;
	int x51;
	int x52;
	int x53;
	int x54;
	int x55;
	int x56;
	int x57;
	int x58;
	int x59;
	int x60;
	int x61;
	int x62;
	int x63;
	int x64;
	int x65;
	int x66;
	int x67;
	int x68;
	int x69;
	int x70;
	int x71;
	int x72;
	int x73;
	int x74;
	int x75;
	int x76;
	int x77;
	int x78;
	int x79;
	int x80;
	int x81;
	int x82;
	int _0x1C0;
	bool m_nMouse_unsure;
	char zz_pad_0x01C5[3];
	int m_nView;
	bool m_bTextureLock;
	char zz_pad_0x01CD[3];
	bool m_bLightmapLock;
	char zz_pad_0x01D1[3];
	bool m_bLoadLast;
	char zz_pad_0x01D5[3];
	bool m_bRunBefore;
	char zz_pad_0x01D9[3];
	int camera_mode;
	bool camera_masked;
	char zz_pad_0x01E1[3];
	int entities_off;
	int sky_brush_off;
	int draw_toggle;
	const char* m_strLastProject;
	const char* m_strLastMap;
	bool m_bFace;
	char zz_pad_0x01F9[3];
	bool m_bRightClick;
	char zz_pad_0x01FD[3];
	bool m_bAutoSave;
	char zz_pad_0x0201[3];
	int m_bNewApplyHandling;
	int part_of_autosave;
	bool m_bLoadLastMap;
	char zz_pad_0x020D[3];
	bool m_bTextureWindowSearch;
	char zz_pad_0x0211[3];
	bool m_bSnapShots;
	char zz_pad_0x0215[3];
	bool loose_changes;
	char zz_pad_0x0219[3];
	float m_fTinySize;
	int m_dropHeight;
	bool m_bCleanTinyBrushes;
	char zz_pad_0x0225[3];
	int m_nStatusSize;
	bool m_bCamXYUpdate;
	char zz_pad_0x022D[7];
	settings_prefab_s* which_game;
	bool m_bALTEdge;
	char zz_pad_0x0239[3];
	bool m_bTextureBar;
	char zz_pad_0x023D[3];
	bool m_bSnapTToGrid;
	char zz_pad_0x0241[3];
	bool linking_keeps_selection;
	char zz_pad_0x0245[3];
	bool m_bXZVis;
	char zz_pad_0x0249[3];
	bool m_bYZVis;
	char zz_pad_0x024D[3];
	bool m_bZVis;
	char zz_pad_0x251[3];
	bool m_bSizePaint;
	char zz_pad_0x0255[3];
	bool b_mCullSky;
	char zz_pad_0x0259[3];
	bool m_bRotateLock;
	char zz_pad_0x025D[3];
	bool m_bNoClamp;
	char zz_pad_0x0261[3];
	bool m_bDropModel;
	char zz_pad_0x0265[3];
	bool m_bOrientModel;
	char zz_pad_0x0269[3];
	const char* m_strUserIniPath;
	const char* m_strUserFilterPath;
	int m_nRotation;
	bool m_bChaseMouse;
	char zz_pad_0x0279[3];
	bool m_bTextureScrollbar;
	char zz_pad_0x027D[3];
	bool thick_selection_lines;
	char zz_pad_0x0281[3];
	bool m_bColoredEnts;
	char zz_pad_0x0285[3];
	bool m_bTolerantWeld;
	char zz_pad_0x0289[3];
	bool m_bVertSnapModel;
	char zz_pad_0x028D[3];
	bool m_bVertSnapBrush;
	char zz_pad_0x0291[3];
	bool m_bVertSnapPrefab;
	char zz_pad_0x0295[3];
	bool m_bSelectableModels;
	char zz_pad_0x0299[3];
	bool texture_brush_2d;
	char zz_pad_0x029D[3];
	bool texture_mesh_2d;
	char zz_pad_0x02A1[3];
	bool fast_2d_view_dragging;
	char zz_pad_viewdrag[3];
	bool detatch_windows;
	char zz_pad_0x02A9[3];
	bool transparent_background;
	char zz_pad_0x02AD[3];
	int farplane;
	int tolerant_weld;
	int vehicle_arrow_time;
	int vehicle_arrow_size;
	int splay;
	int m_nUndoLevels;
	int scale_base;
	int scale_range;
	float camera_fov;
	bool m_bForceZeroDropHeight;
	char zz_pad_0x02D5[3];
	float model_origin_size;
	float prefab_origin_size;
	int m_nMouseButtons;
	int m_nAngleSpeed;
	int m_nMoveSpeed;
	int m_nAutoSave;
	bool m_bCubicClipping;
	char zz_pad_0x02F1[3];
	int m_nCubicScale;
	bool m_bSelectCurves;
	char zz_pad_0x02F9[3];
	int m_nEntityShowState;
	int m_nTextureWindowScale;
	char zz_pad_0x0304[4];
	int m_bSwitchClip;
	bool m_bSelectWholeEntities;
	char zz_pad_0x030D[3];
	bool enable_light_preview;
	char zz_pad_0x0311[3];
	bool preview_sun_aswell;
	char zz_pad_0x0315[3];
	//bool patch_wireframe;
	//char pad_0x0319[3];
	int g_nPatchAsWireframe;
	bool g_bPatchWeld;
	bool patch_drill_down;
	char zz_pad_0x031D[2];
	bool camera_use_wheel;
	char zz_pad_0x0321[3];
	const char* ScriptGroupKey;
	const char* ScriptGroupTokenKey;
	const char* ScriptColorTeamKey;
	char ScriptColorKey[8];
	const char* ScriptSubKey_key;
	int ScriptSubValue_key;
	int xxx2;
	int xxx3;
	int xxx4;
	int xxx5;
	int xxx6;
};

struct CListBox : public CWnd
{};

struct CCheckListBox : public CListBox
{
	int m_cyText;
	unsigned int m_nStyle;
};

struct CFilterWnd : public CDialog
{
	CCheckListBox geometry_filters;
	CCheckListBox entity_filters;
	CCheckListBox trigger_filters;
	CCheckListBox other_filters;
};

struct CNoTrackObject;

struct /*VFT*/ CNoTrackObject_vtbl
{
	void(__thiscall* CNoTrackObject_Destructor)(CNoTrackObject*);
};

struct CNoTrackObject
{
	CNoTrackObject_vtbl* __vftable /*VFT*/;
};

struct CWinApp : CCmdTarget {};

struct AFX_MODULE_STATE : CNoTrackObject
{
	CWinApp* m_pCurrentWinApp;
	HINSTANCE__* m_hCurrentInstanceHandle;
	HINSTANCE__* m_hCurrentResourceHandle;
	const char* m_lpszCurrentAppName;
	unsigned __int8 m_bDLL;
	unsigned __int8 m_bSystem;
	unsigned __int8 m_bReserved[2];
	unsigned int m_fRegisteredClasses;
	//CRuntimeClass *m_pClassInit;
	//CTypedSimpleList<CRuntimeClass *> m_classList;
	//COleObjectFactory *m_pFactoryInit;
	//CTypedSimpleList<COleObjectFactory *> m_factoryList;
	//int m_nObjectCount;
	//int m_bUserCtrl;
	//char m_szUnregisterList[4096];
	//int (__stdcall *m_pfnAfxWndProc)(HWND__ *, unsigned int, unsigned int, int);
	//unsigned int m_dwVersion;
	//void (__stdcall *m_pfnFilterToolTipMessage)(tagMSG *, CWnd *);
	//CTypedSimpleList<CDynLinkLibrary *> m_libraryList;
	//HINSTANCE__ *m_appLangDLL;
	//COccManager *m_pOccManager;
	//CTypedSimpleList<COleControlLock *> m_lockList;
	//_AFX_DAO_STATE *m_pDaoState;
	//CTypeLibCache m_typeLibCache;
	//CTypeLibCacheMap *m_pTypeLibCacheMap;
	//CThreadLocal<AFX_MODULE_THREAD_STATE> m_thread;
};


namespace afx
{
	// *
	// -------------------------------------- function typedefs -------------------------------------------
	// *
	
	typedef CWnd* (__thiscall* CSplitterWnd__GetPane_t)(CSplitterWnd*, int row, int col); extern CSplitterWnd__GetPane_t CSplitterWnd__GetPane;
	typedef void(__thiscall* CSplitterWnd__SetRowInfo_t)(CSplitterWnd*, int row, int, int); extern CSplitterWnd__SetRowInfo_t CSplitterWnd__SetRowInfo;
	typedef void(__thiscall* CWnd_SetFocus_t)(CWnd*); extern CWnd_SetFocus_t CWnd_SetFocus;
	typedef void(__stdcall* CWnd_FromHandle_t)(HWND); extern CWnd_FromHandle_t CWnd_FromHandle;
	typedef AFX_MODULE_STATE* (__stdcall* get_module_state_t)();  extern get_module_state_t get_module_state;
	
}