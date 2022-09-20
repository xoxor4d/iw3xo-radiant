#pragma once

class cfxwnd
{
private:
	cfxwnd (cfxwnd const&) = delete;
	void operator =(cfxwnd const&) = delete;

public:
	cfxwnd();

	static inline cfxwnd* p_this = nullptr;
	static cfxwnd*	get() { return p_this; }

	HWND			m_frame_hwnd = nullptr;
	int				m_width = 0;
	int				m_height = 0;
	game::vec3_t	m_origin = {};
	game::vec3_t	m_angles = {};
	game::vec3_t	m_forward = {};
	game::vec3_t	m_right = {};
	game::vec3_t	m_up = {};
	game::vec3_t	m_vup = {};
	game::vec3_t	m_vpn = {};
	game::vec3_t	m_vright = {};

	game::XModel*	m_xmodel_ptr_test = nullptr;
	int				m_xmodel_inst = 0;

	int				m_tickcount_playback = 0;
	int				m_tickcount_repeat = 0;
	void			tick_playback();
	void			setup_fx();

	static void		create_fxwnd();
	static void		create_content_window();
	static void		precreate_window();
	static void		on_paint();

	static LRESULT __stdcall windowproc_frame(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static LRESULT __stdcall windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};