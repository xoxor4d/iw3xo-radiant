#pragma once
#include "common/cxywnd.hpp"

namespace ggui
{
	class menubar_dialog final : public ggui::ggui_module
	{
		float m_height;

	public:
		menubar_dialog()
		{
			set_gui_type(GUI_TYPE_DEF);
			m_height = 8.0f;
		}


		// *
		// public member functions

		void menubar();

		float get_height() const
		{
			return m_height;
		}

		void set_height(const float new_height)
		{
			m_height = new_height;
		}

	private:
		void set_grid_size(const xywnd::E_GRID_SIZES size);
		void set_render_method(const game::RENDER_METHOD_E meth);
		void set_texture_resolution(int picmip);
	};
}
