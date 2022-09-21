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

	fx_system::FxEditorEffectDef m_raw_effect = {};
	fx_system::FxEffect* m_active_effect = nullptr;
	bool			m_effect_is_playing = false;

	game::XModel*	m_xmodel_ptr_test = nullptr;
	int				m_xmodel_inst = 0;

	int				m_tickcount_playback = 0;
	int				m_tickcount_repeat = 0;

	void			stop_effect();
	bool			load_effect(const char* effect_name);
	void			setup_and_spawn_fx();
	void			tick_playback();
	void			update_fx();

	static void		create_fxwnd();
	static void		create_content_window();
	static void		precreate_window();
	static void		on_paint();

	static LRESULT __stdcall windowproc_frame(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static LRESULT __stdcall windowproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};