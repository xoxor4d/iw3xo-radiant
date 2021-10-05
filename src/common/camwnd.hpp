#pragma once

struct camera_s
{
	int	width;
	int height;
	bool timing;
	char pad_timing[3];
	game::vec3_t origin;
	game::vec3_t angles;
	int draw_mode;
	game::vec3_t color;
	game::vec3_t forward;
	game::vec3_t right;
	game::vec3_t up;
	game::vec3_t vup;
	game::vec3_t vpn;
	game::vec3_t vright;
};

class ccamwnd : public CWnd
{
private:
	// Do not allow this class to be instanced
	ccamwnd() = delete;
	~ccamwnd() = delete;

public:
	void* m_pUnkown;
	camera_s camera;
	int m_nCambuttonstate;
	CPoint m_ptButton;
	int x32;
	CPoint m_ptCursor;
	CPoint m_ptLastCursor;
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
	int prob_some_cursor;
	int x47;
	int x48;

	//
	static ccamwnd *activewnd;

	static void				rtt_camera_window();
	
	void					mouse_control(float dtime);
	static void				mouse_up(ccamwnd* cam, int flags);
	static void				mouse_moved(ccamwnd* wnd, int buttons, int x, int y);
	
	static void				hooks();
	static BOOL WINAPI		windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	static void __fastcall	on_lbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_lbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_rbutton_down(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_rbutton_up(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_mouse_move(ccamwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __stdcall	on_keydown(UINT nChar, UINT nRepCnt, UINT nFlags);
	static void __stdcall	on_keyup(UINT nChar, UINT nRepCnt, UINT nFlags);
};
STATIC_ASSERT_OFFSET(ccamwnd, camera.origin, 0x64);
STATIC_ASSERT_OFFSET(ccamwnd, camera.angles, 0x70);
STATIC_ASSERT_OFFSET(ccamwnd, m_nCambuttonstate, 0xD4);