#pragma once

namespace components
{
	class renderer : public component
	{
	public:
		renderer();
		~renderer();
		const char* get_name() override { return "renderer"; };

		static void		R_ConvertColorToBytes(float* from, game::GfxColor* gfx_col);
		static void		R_AddCmdDrawTextAtPosition(const char* text, game::Font_s* font, float* origin, float* pixel_step_x, float* pixel_step_y, float* color);
		static void		copy_scene_to_texture(ggui::e_gfxwindow wnd, IDirect3DTexture9*& dest);
	};
}
