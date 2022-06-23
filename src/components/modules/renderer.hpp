#pragma once

namespace components
{
	extern game::Material* g_invalid_material;

	class renderer : public component
	{
	public:
		renderer();
		~renderer();
		const char*		get_name() override { return "renderer"; };
		static void		register_dvars();

		static void		RB_Draw3D();
		static void		R_SetAndClearSceneTarget(bool clear);

		static void		R_AddCmdSetCustomShaderConstant(game::ShaderCodeConstants constant, float x, float y, float z, float w);
		static void		R_ConvertColorToBytes(float* from, game::GfxColor* gfx_col);
		static void		R_AddCmdDrawTextAtPosition(const char* text, game::Font_s* font, float* origin, float* pixel_step_x, float* pixel_step_y, float* color);
		static void		copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest, bool no_release = false);
		static void		R_AddLineCmd(const std::uint16_t count, const char width, const char dimension, const game::GfxPointVertex* verts);

		static inline int effect_drawsurf_count_ = 0;

		static bool is_rendering_camerawnd()
		{
			return game::dx->targetWindowIndex == ggui::CCAMERAWND;
		}
		static bool is_rendering_gridwnd()
		{
			return game::dx->targetWindowIndex == ggui::CXYWND;
		}
		static bool is_rendering_texturewnd()
		{
			return game::dx->targetWindowIndex == ggui::CTEXWND;
		}
		
		static bool is_rendering_layeredwnd()
		{
			return game::dx->targetWindowIndex == ggui::LAYERED;
		}

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
