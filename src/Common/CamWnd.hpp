#pragma once

//struct CPoint
//{
//	int x;
//	int y;
//};

struct camera_s
{
	int	width;
	int height;
	bool timing;
	char pad_timing[3];
	Game::vec3_t origin;
	Game::vec3_t angles;
	int draw_mode;
	Game::vec3_t color;
	Game::vec3_t forward;
	Game::vec3_t right;
	Game::vec3_t up;
	Game::vec3_t vup;
	Game::vec3_t vpn;
	Game::vec3_t vright;
};

class CCamWnd : public CWnd
{
private:
	// Do not allow this class to be instanced
	CCamWnd() = delete;
	~CCamWnd() = delete;

public:
	void* m_pUnkown;
	camera_s camera;
	int m_nCambuttonstate;
	CPoint m_ptButton;
	int unkown01;
	CPoint m_ptCursor;
	CPoint m_ptLastCursor;

	//
	static CCamWnd *ActiveWindow;

	void Cam_MouseControl(float dtime);

	static void				main();
	static BOOL WINAPI		wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static void				on_endframe();

	static void __fastcall	on_lbutton_down(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_lbutton_up(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_rbutton_down(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_rbutton_up(CCamWnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
};
STATIC_ASSERT_OFFSET(CCamWnd, camera.origin, 0x64); // diff. offset then bo1
STATIC_ASSERT_OFFSET(CCamWnd, camera.angles, 0x70); // ^
STATIC_ASSERT_OFFSET(CCamWnd, m_nCambuttonstate, 0xD4);