#pragma once

namespace ggui
{
	class modelselector_dialog final : public ggui::ggui_module
	{
		ImGuiTextFilter	m_filter;
		bool m_update_scroll_position = false;
		bool m_preferences_open = false;
		bool m_scene_texture_hovered = false;

	public:
		bool m_user_rotation = false;
		bool m_overwrite_selection = false;

		// camera
		float m_camera_fov = 60.0f;
		float m_camera_distance = 0.0f;
		float m_camera_angles[3]{};
		float m_camera_offset[3]{};

		// axis model
		bool m_axis_model_initiated = false;
		int m_axis_model_inst_handle = -1;

		// model preview
		std::string		m_preview_model_name;
		game::XModel*	m_preview_model_ptr = nullptr;
		int				m_preview_model_inst_handle = -1;
		bool			m_bad_model = false;

		float			m_anim_model_yaw = 0.0f;
		bool			m_anim_pause = false;

		// raw/xmodel folder
		int				m_xmodel_filecount = 0;
		const char**	m_xmodel_filelist = nullptr;
		int				m_xmodel_selection = -1;

		modelselector_dialog() { set_gui_type(GUI_TYPE_RTT); } // render-to-texture gui


		// *
		// public member functions

		void gui() override;
		void on_open() override;
		void on_close() override;

		// *
		// asm related


		// *
		// init

		void init();

	private:
		void xmodel_listbox_elem(int index);
	};
}
