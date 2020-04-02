#pragma once

#define SELECTED_BRUSHES_GET_ARRAY (Game::selbrush_t*) *(DWORD*)(Game::currSelectedBrushes)
#define GET_FRONTENTDATA (Game::GfxBackEndData*) *(DWORD*)(Game::frontEndDataOut_ptr)
#define GET_WORLDENTITY (Game::entity_s*) *(DWORD*)(Game::worldEntity_ptr)

DWORD WINAPI RemoteNet_SearchServerThread(LPVOID);
DWORD WINAPI RemoteNet_ReceivePacket_Thread(LPVOID);

namespace Components
{
	class RemNet : public Component 
	{
	public:
		RemNet();
		~RemNet();
		const char* getName() override { return "RemNet"; };

		static void		Cmd_ProcessCamera(const char *strCommand);
		static void		Cmd_ProcessBrushSideIndex(const char *strCommand);
		static void		Cmd_ProcessBrushSide(const char *strCommand);

		static void		SendPacket(Game::RadiantCommand *Command);
		static void		Cmd_SendDvar(const char* KVCommand);
		static void		Cmd_SendSelect(const char *KVCommand);
		static void		Cmd_SendSelectedUpdate(const char *KVCommand);
		static void		Cmd_SendCameraUpdate(float *Origin, float *Angles);
		static bool		Cmd_SendSingleBrush(Game::brush_t *brush, int brushNum);
		static void		Cmd_SendBrushSelectDeselect(bool select);
		
	private:
	};
}