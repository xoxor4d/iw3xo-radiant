#include "STDInclude.hpp"

// Taken directly from q3radiant
// https://github.com/id-Software/Quake-III-Arena
#define W_CAMERA		0x0001
#define W_XY			0x0002
#define W_XY_OVERLAY	0x0004
#define W_Z				0x0008
#define W_TEXTURE		0x0010
#define W_Z_OVERLAY		0x0020
#define W_CONSOLE		0x0040
#define W_ENTITY		0x0080
#define W_CAMERA_IFON	0x0100
#define W_XZ			0x0200  //--| only used for patch vertex manip stuff
#define W_YZ			0x0400  //--|
#define W_GROUP			0x0800 
#define W_MEDIA			0x1000 
#define W_ALL			0xFFFFFFFF

void __declspec(naked) CMainFrame::hk_RoutineProcessing(void)
{
	_asm
	{
		push ecx
		mov ecx, eax
		call CMainFrame::RoutineProcessing
		pop ecx
		retn
	}
}

void CMainFrame::RoutineProcessing()
{
	if (!this->m_bDoLoop)
	{
		return;
	}

	if (0.0 == Game::g_time)
	{
		Game::g_time = 0.0;
	}

	if (0.0 == Game::g_oldtime)
	{
		Game::g_oldtime = 0.0;
	}

	double time = clock() / 1000.0;
	double oldtime = time - Game::g_time;

	Game::g_time = time;

	if (oldtime > 2.0)
	{
		oldtime = 0.1; // 0.1
	}

	Game::g_oldtime = oldtime;

	if (oldtime > 0.2)
	{
		oldtime = 0.2; // 0.2
	}

	if (this->m_pCamWnd)
	{
		float delta = (float)oldtime;
		this->m_pCamWnd->Cam_MouseControl(delta);
	}

	if (Game::g_nUpdateBits)
	{
		int nBits = Game::g_nUpdateBits;
		Game::g_nUpdateBits = 0;
		this->UpdateWindows(nBits);
	}

	Game::Globals::radiant_initiated = true;
}

bool Worldspawn_OnKeyChange(const Game::epair_t* epair, const char* key, float* value, const int &valueSize)
{
	bool changed = false;

	if (!Utils::Q_stricmp(epair->key, key))
	{
		std::vector<std::string> KeyValues = Utils::Explode(epair->value, ' ');

		int count = KeyValues.size();
		if (count > valueSize) count = valueSize;

		for (auto i = 0; i < count; i++)
		{
			float temp = Utils::try_stof(KeyValues[i], true);

			if (value[i] != temp)
			{
				value[i] = temp;
				changed = true;
			}
		}

		if (changed)
		{
			return true;
		}
	}
	
	return false;
}

void TrackWorldspawnSettings()
{
	// trackWorldspawn
	auto world = GET_WORLDENTITY;

	if (world && world->firstActive->eclass->name)
	{
		if (!Utils::Q_stricmp(world->firstActive->eclass->name, "worldspawn"))
		{
			for (auto epair = world->firstActive->epairs; epair; epair = epair->next)
			{
				if (Worldspawn_OnKeyChange(epair, "sundirection", Game::Globals::trackWorldspawn.sundirection, 3))
				{
					if (Game::Globals::trackWorldspawn.initiated)
					{
						Components::RemNet::Cmd_SendDvar(Utils::VA("{\n\"dvarname\" \"%s\"\n\"value\" \"%.1f %.1f %.1f\"\n}", "r_lighttweaksundirection",
							Game::Globals::trackWorldspawn.sundirection[0], Game::Globals::trackWorldspawn.sundirection[1], Game::Globals::trackWorldspawn.sundirection[2]));
					}
				}

				if (Worldspawn_OnKeyChange(epair, "suncolor", Game::Globals::trackWorldspawn.suncolor, 3))
				{
					if (Game::Globals::trackWorldspawn.initiated)
					{
						Components::RemNet::Cmd_SendDvar(Utils::VA("{\n\"dvarname\" \"%s\"\n\"value\" \"%.1f %.1f %.1f\"\n}", "r_lighttweaksuncolor",
							Game::Globals::trackWorldspawn.suncolor[0], Game::Globals::trackWorldspawn.suncolor[1], Game::Globals::trackWorldspawn.suncolor[2]));
					}
				}

				if (Worldspawn_OnKeyChange(epair, "sunlight", &Game::Globals::trackWorldspawn.sunlight, 1))
				{
					if (Game::Globals::trackWorldspawn.initiated)
					{
						Components::RemNet::Cmd_SendDvar(Utils::VA("{\n\"dvarname\" \"%s\"\n\"value\" \"%.1f\"\n}", "r_lighttweaksunlight",
							Game::Globals::trackWorldspawn.sunlight));
					}
				}
			}
		}

		if (!Game::Globals::trackWorldspawn.initiated)
		{
			Game::Globals::trackWorldspawn.initiated = true;
			return;
		}
	}
}

void CMainFrame::UpdateWindows(int nBits)
{
	// grab camera if not using floating windows
	if (!Game::Globals::radiant_floatingWindows && this->m_pCamWnd)
	{
		CCamWnd::ActiveWindow = this->m_pCamWnd;
		Game::Globals::radiant_floatingWindows = true;
	}

	if (!Game::g_bScreenUpdates)
	{
		return;
	}

	TrackWorldspawnSettings();

	if (nBits & (W_XY | W_XY_OVERLAY))
	{
		if (this->m_pXYWnd)
		{
			m_pXYWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	if (nBits & W_CAMERA || ((nBits & W_CAMERA_IFON) && this->m_bCamPreview))
	{
		if (this->m_pCamWnd)
		{
			// Redraw the camera view
			m_pCamWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

			Game::Globals::m_pCamWnd_ref = m_pCamWnd;

			// on update cam window through the 2d grid or something else
			if ((nBits & W_CAMERA_IFON) && this->m_bCamPreview || nBits < 0 || nBits == 3)
			{
			}

			// only update the remote cam when we actually move it, not when we update the cam window by doing something in the gridWnd etc.
			else
			{
				// Attempt to update the remote camera
				if (CCamWnd::ActiveWindow)
				{
					Components::RemNet::Cmd_SendCameraUpdate(CCamWnd::ActiveWindow->cameraOrigin, CCamWnd::ActiveWindow->cameraAngles);
				}
			}
		}
	}

	if (nBits & (W_Z | W_Z_OVERLAY))
	{
		if (this->m_pZWnd)
		{
			m_pZWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	if (nBits & W_TEXTURE)
	{
		if (this->m_pTexWnd) 
		{
			m_pTexWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
}
