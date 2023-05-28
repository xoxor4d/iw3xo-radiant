#pragma once

namespace components
{
	extern game::Material* g_invalid_material;

	class renderer : public component
	{
	public:
		float	m_cam_msec = 0.0f;
		double	m_cam_time = 0.0;

	public:
		renderer();
		~renderer();

		static renderer* p_this;
		static renderer* get() { return p_this; }

		const char*		get_name() override { return "renderer"; };
		static void		register_dvars();

		static void		RB_Draw3D();
		static void		R_SetAndClearSceneTarget(bool clear);

		static void		R_AddCmdSetCustomShaderConstant(game::ShaderCodeConstants constant, float x, float y, float z, float w);
		static void		R_ConvertColorToBytes(float* from, game::GfxColor* gfx_col);
		static void		R_AddCmdDrawTextAtPosition(const char* text, game::Font_s* font, float* origin, float* pixel_step_x, float* pixel_step_y, float* color);

		static void		R_InitDrawSurfListInfo(game::GfxDrawSurfListInfo* list);

		static void		add_debug_box(const float* origin, const float* mins, const float* maxs, float yaw, float size_offset, bool depth_test_override = false, bool depth_test_value = false);
		static void		R_AddLineCmd(const std::uint16_t count, const char width, const char dimension, const game::GfxPointVertex* verts);
		static void		R_AddPointCmd(const std::uint16_t count, const char width, const char dimension, const game::GfxPointVertex* verts);

		static inline int effect_drawsurf_count_ = 0;

		void set_line_depthtesting(const bool state) { g_line_depth_testing = state; }
		bool get_line_depthtesting() { return g_line_depth_testing; }
		bool g_line_depth_testing = true;

		// ------

		static uint8_t constexpr  GFX_TARGETWINDOW_COUNT = 10;
		static inline game::GfxWindowTarget windows[GFX_TARGETWINDOW_COUNT] = {};

		enum GFXWND_ : int
		{
			CCAMERAWND = 0,
			CXYWND = 1,
			CZWND = 2,
			CTEXWND = 3,
			LAYERED = 4,
			CFXWND = 5,
		};

		static void	copy_scene_to_texture(GFXWND_ GFXWND, IDirect3DTexture9*& dest, bool no_release = false);

		static game::GfxWindowTarget* get_window(const GFXWND_ GFXWND)
		{
			if (!renderer::windows[GFXWND].hwnd)
			{
				AssertS(utils::va("Invalid window [%d]", GFXWND));
			}

			return &renderer::windows[GFXWND];
		}

		static bool is_rendering_camerawnd()	{ return game::dx->targetWindowIndex == CCAMERAWND; }
		static bool is_rendering_gridwnd()		{ return game::dx->targetWindowIndex == CXYWND; }
		static bool is_rendering_zwnd()			{ return game::dx->targetWindowIndex == CZWND; }
		static bool is_rendering_texturewnd()	{ return game::dx->targetWindowIndex == CTEXWND; }
		static bool is_rendering_layeredwnd()	{ return game::dx->targetWindowIndex == LAYERED; }
		static bool is_rendering_effectswnd()	{ return game::dx->targetWindowIndex == CFXWND; }

		class postfx
		{
		private:
			static inline int	disble_duration_in_frames = 120;
			static inline int	frame_counter = 0;
			static inline bool	any_active = false;
			static inline bool	disable_all_postfx = false;

		public:
			static void set_disable_duration(float seconds)
			{
				disble_duration_in_frames = static_cast<int>(game::glob::frames_per_second * seconds);
			}

			// is any post effect active?
			static bool is_any_active()
			{
				return any_active;
			}

			// are post effects disabled?
			static bool is_disabled()
			{
				return disable_all_postfx;
			}

			// enable/disable post effect logic
			static void set_state(bool state)
			{
				any_active = state;
			}

			// disable all post effects and reset counter
			static void disable()
			{
				disable_all_postfx = true;
				frame_counter = 0;
			}

			// call every frame
			static void frame()
			{
				if(disable_all_postfx)
				{
					if (frame_counter < disble_duration_in_frames)
					{
						frame_counter++;
					}
					else
					{
						disble_duration_in_frames = 120;
						disable_all_postfx = false;
						frame_counter = 0;
					}
				}
			}
		};
	};

}
