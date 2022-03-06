#include "std_include.hpp"

#define Assert()	if(IsDebuggerPresent()) __debugbreak();	else {	\
					game::Com_Error("Line %d :: %s\n%s ", __LINE__, __func__, __FILE__); }

namespace fx_system
{
	void R_ProcessCmd_UpdateFxSpotLight(FxCmd* cmd)
	{
		FX_UpdateSpotLight(cmd);
	}

	void R_ProcessCmd_UpdateFxNonDependent(FxCmd* cmd)
	{
		if (game::Dvar_FindVar("fx_enable")->current.enabled)
		{
			FX_Update(cmd->system, true);
		}
	}

	// checked
	void R_ProcessCmd_UpdateFxRemaining(FxCmd* cmd)
	{
		FxGenerateVertsCmd genVertsCmd = {};

		FX_UpdateRemaining(cmd);
		FX_EndUpdate(cmd->localClientNum);
		Sys_DoWorkerCmd(WRKCMD_GENERATE_MARK_VERTS, cmd);
		FX_AddNonSpriteDrawSurfs(cmd); // part of R_DrawEffects
		FX_FillGenerateVertsCmd(cmd->localClientNum, &genVertsCmd);
		Sys_DoWorkerCmd(WRKCMD_GENERATE_FX_VERTS, &genVertsCmd);
	}

	void Sys_DoWorkerCmd(WorkerCmdType type, void* cmd)
	{
		switch (type)
		{
		case WRKCMD_UPDATE_FX_SPOT_LIGHT:

			R_ProcessCmd_UpdateFxSpotLight(static_cast<FxCmd*>(cmd));
			break;


		case WRKCMD_UPDATE_FX_NON_DEPENDENT:

			R_ProcessCmd_UpdateFxNonDependent(static_cast<FxCmd*>(cmd));
			break;


		case WRKCMD_UPDATE_FX_REMAINING:

			R_ProcessCmd_UpdateFxRemaining(static_cast<FxCmd*>(cmd));
			break;


		case WRKCMD_DPVS_CELL_STATIC:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4EC670)(cmd); //	R_AddCellStaticSurfacesInFrustumCmd((DpvsStaticCellCmd *)cmd);
#endif

			break;


		case WRKCMD_DPVS_CELL_SCENE_ENT:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4FDD70)(cmd); // R_AddCellSceneEntSurfacesInFrustumCmd((GfxWorldDpvsPlanes *)cmd);
#endif

			break;


		case WRKCMD_DPVS_CELL_DYN_MODEL:
			return;


		case WRKCMD_DPVS_CELL_DYN_BRUSH:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4C0100)(cmd); // R_AddCellDynBrushSurfacesInFrustumCmd((DpvsDynamicCellCmd *)cmd);
#endif

			break;


		case WRKCMD_DPVS_ENTITY:

#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4FDC00)(cmd); // R_AddEntitySurfacesInFrustumCmd((DpvsEntityCmd *)cmd);
#endif

			break;


		case WRKCMD_ADD_SCENE_ENT:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4BDFA0)(cmd); // R_AddAllSceneEntSurfacesCamera(*(GfxViewInfo **)cmd);
#endif

			break;


		case WRKCMD_SPOT_SHADOW_ENT:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4DD400)(cmd); // R_AddSpotShadowEntCmd((GfxSpotShadowEntCmd *)cmd);
#endif

			break;


		case WRKCMD_SHADOW_COOKIE:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4E2FD0)(cmd);// R_GenerateShadowCookiesCmd((ShadowCookieCmd *)cmd);
#endif

			break;


		case WRKCMD_BOUNDS_ENT_DELAYED:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4EB580)(cmd); // R_UpdateGfxEntityBoundsCmd(cmd);
#endif

			break;


		case WRKCMD_SKIN_ENT_DELAYED:
#ifdef FXEDITOR
			// #ENV_DEPENDENT - not effect related
			utils::hook::call<void(__cdecl)(void*)>(0x4EAA20)(cmd); // R_SkinGfxEntityCmd(cmd);
#endif

			break;


		case WRKCMD_GENERATE_FX_VERTS:

			if (!game::dx->deviceLost)
			{
				FX_GenerateVerts((FxGenerateVertsCmd *)cmd);

				// #ENV_DEPENDENT
				//utils::hook::call<void(__cdecl)(void*)>(0x48E7F0)(cmd); // FX_GenerateVerts
			}
			break;


		case WRKCMD_GENERATE_MARK_VERTS:
			return;

			/*if (!game::dx.deviceLost)
			{
				// #MARKS
				// FX_GenerateMarkVertsForWorld(*((_DWORD *)cmd + 1)); // (localclientnum)

				// #ENV_DEPENDENT
				// utils::hook::call<void(__cdecl)(int)>(0x4839E0)(0); // FX_GenerateMarkVertsForWorld

				
			}
			break;*/

		case WRKCMD_SKIN_CACHED_STATICMODEL:
#ifdef FXEDITOR
			// #ENV_DEPENDENT
			utils::hook::call<void(__cdecl)(int)>(0x4FD230)(0); // R_SkinXModelCmd(cmd);
#endif

			break;


		default:
			Assert();
			break;
		}
	}
}