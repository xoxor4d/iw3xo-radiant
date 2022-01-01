#pragma once

#define SELECTED_BRUSHES_GET_ARRAY (game::selbrush_t*) *(DWORD*)(game::currSelectedBrushes)
#define GET_FRONTENTDATA (game::GfxBackEndData*) *(DWORD*)(game::frontEndDataOut_ptr)
#define GET_WORLDENTITY (game::entity_s*) *(DWORD*)(game::worldEntity_ptr)

DWORD WINAPI remote_net_search_server_thread(LPVOID);
DWORD WINAPI remote_net_receive_packet_thread(LPVOID);

namespace components
{
	class remote_net : public component 
	{
	public:
		remote_net();
		~remote_net();
		const char*		get_name() override { return "remote_net"; };

		static bool		selection_is_brush(game::brush_t_with_custom_def* brush);
		
		static void		cmd_process_camera(const char *strCommand);
		static void		Cmd_ProcessBrushSideIndex(const char *strCommand);
		static void		cmd_process_brushside(const char *strCommand);

		static void		send_packet(game::RadiantCommand *Command);
		static void		Cmd_SendDvar(const char* KVCommand);
		static void		cmd_send_select(const char *KVCommand);
		static void		cmd_send_selected_update(const char *KVCommand);
		static void		cmd_send_camera_update(float *Origin, float *Angles);
		static bool		cmd_send_single_brush(game::brush_t_with_custom_def *brush, int brushNum);
		static void		cmd_send_brush_select_deselect(bool select);

		static void		track_worldspawn_settings();

		static void		on_shutdown();
		static void		register_dvars();
		
	private:
	};
}