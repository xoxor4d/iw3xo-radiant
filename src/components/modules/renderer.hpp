#pragma once

namespace components
{
	class renderer : public component
	{
	public:
		renderer();
		~renderer();
		const char*		get_name() override { return "renderer"; };
		static void		register_dvars();

		static void		R_AddCmdSetCustomShaderConstant(game::ShaderCodeConstants constant, float x, float y, float z, float w);
		static void		R_ConvertColorToBytes(float* from, game::GfxColor* gfx_col);
		static void		R_AddCmdDrawTextAtPosition(const char* text, game::Font_s* font, float* origin, float* pixel_step_x, float* pixel_step_y, float* color);
		static void		copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest);

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
	};
}
