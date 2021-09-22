#pragma once

struct zwnd_s
{
	int		cursor_y; // not sure
	int		cursor_x;
	int		width, height;
	bool	timing;
	float	origin[3];			// at center of window
	float	scale;
};

extern zwnd_s* zwnd;

class czwnd : public CWnd
{
private:
	// Do not allow this class to be instanced
	czwnd() = delete;
	~czwnd() = delete;

public:

	static void				main();
	static LRESULT WINAPI	windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	static void __fastcall	on_lbutton_down(czwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_lbutton_up(czwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_rbutton_down(czwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_rbutton_up(czwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_mbutton_down(czwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
	static void __fastcall	on_mbutton_up(czwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);

	static void __fastcall	on_mouse_move(czwnd* pThis, [[maybe_unused]] void* edx, UINT nFlags, CPoint point);
};