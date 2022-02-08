#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	void	R_ProcessCmd_UpdateFxSpotLight(FxCmd* cmd);
	void	R_ProcessCmd_UpdateFxNonDependent(FxCmd* cmd);
	void	R_ProcessCmd_UpdateFxRemaining(FxCmd* cmd);

	void	Sys_DoWorkerCmd(WorkerCmdType type, void* cmd);

}