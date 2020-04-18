#include "STDInclude.hpp"

#define RADIANT_DEBUG_BRUSH false
#define RADIANT_DEBUG_CMDS false

// maximum amount of selected brushes to send to the game
#define REMOTE_MAX_SEL_BRUSHES 16

SOCKET g_RemoteSocket;
int g_RemoteSocketStatus = -1;

// g_qeglobals_d_select_mode 
// 0 :: normal draging / skewing
// 1 :: vertex
// 2 :: edge
// 3 ::
// 4 ::
// 5 ::

// *
// Constantly search for a server radiant can connect to
DWORD WINAPI RemoteNet_SearchServerThread(LPVOID)
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		printf("[LiveRadiant]: WSAStartup ERROR!\n");
		return false;
	}
		
	while (true)
	{
		// wait for config to load
		if (!Game::Globals::radiant_config_loaded)
		{
			// no config file found, registered default dvars 
			if (Game::Globals::radiant_config_not_found && Dvars::radiant_live && Dvars::radiant_live->current.enabled)
			{
				goto INIT_DEFAULT;
			}

			Sleep(200);
			continue;
		}
		else
		{
			// wait till the user enables live-link
			if (Dvars::radiant_live && !Dvars::radiant_live->current.enabled)
			{
				Sleep(1000);
				continue;
			}
		}

	INIT_DEFAULT:

		Sleep(200);

		// connect to the remote game TCP server (doing this in the while loop too if radiant is running and the server is not )
		g_RemoteSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (g_RemoteSocket == INVALID_SOCKET)
		{
			printf("[LiveRadiant]: Failed to initialize client TCP socket!\n");
			return 0;
		}

		if (Dvars::radiant_livePort)
		{
			printf(Utils::VA("[LiveRadiant]: Listening for game server on 127.0.0.1:%d ...\n", Dvars::radiant_livePort->current.integer));
		}	
		else
		{
			printf("[LiveRadiant]: Listening for game server on 127.0.0.1:3700 ...\n");
		}
			

		// Loop indefinitely until we successfully connect
		while (true)
		{
			g_RemoteSocketStatus = INVALID_SOCKET;

			sockaddr_in remoteAddr;
			remoteAddr.sin_family = AF_INET;

			if (Dvars::radiant_livePort)
			{
				remoteAddr.sin_port = htons((u_short)Dvars::radiant_livePort->current.integer);
			}
			else
			{
				remoteAddr.sin_port = htons(3700);
			}

			remoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

			// check if the remote socket was created
			if (g_RemoteSocket == INVALID_SOCKET)
			{
				g_RemoteSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			}

			// if connected sucessfully
			if (connect(g_RemoteSocket, (sockaddr *)&remoteAddr, sizeof(sockaddr)) != SOCKET_ERROR)
			{
				break;
			}

			Sleep(200);
		}

		// Send updates until the game exits/connection is terminated
		printf("[LiveRadiant]: Game connected!\n");
		g_RemoteSocketStatus = 1;

		Game::Globals::live_connected = true;

		// Loop indefinitely until the game disconnects or live-link gets disabled
		while (true)
		{
			if (Dvars::radiant_live)
			{
				// Check for a socket error or if live-link was disabled
				if (g_RemoteSocket == INVALID_SOCKET || !Dvars::radiant_live->current.enabled)
				{
					break;
				}
			}
			else
			{
				// Check for a socket error
				if (g_RemoteSocket == INVALID_SOCKET)
				{
					break;
				}
			}
			
			int error = 0;
			int len = sizeof(int);
			int retval = getsockopt(g_RemoteSocket, SOL_SOCKET, SO_ERROR, (char *)&error, &len);

			if (retval == SOCKET_ERROR || error != 0)
			{
				break;
			}
				
			Sleep(50);
		}

		Game::Globals::live_connected = false;

		closesocket(g_RemoteSocket);
		g_RemoteSocketStatus = INVALID_SOCKET;

		printf("[LiveRadiant]: Game disconnected!\n");
	}

	return 0;
}

// *
// Receive and process commands send from the server if radiant is connected to a server 
// Will also close the socket and inform the game that radiant disconnected

Game::ServerCommand recvCommands[256];

DWORD WINAPI RemoteNet_ReceivePacket_Thread(LPVOID)
{
	while (true)
	{
		// check if we are connected to a server
		while (true)
		{
			// check if radiant is connected to the server
			if (g_RemoteSocket == INVALID_SOCKET || g_RemoteSocketStatus == INVALID_SOCKET)
			{
				Sleep(200);
			}
			// start processing commands if we are
			else
			{
				break;
			}
		}

		// read server commands as long as the socket is valid
		while (true)
		{
			// stop commands from processing until radiant is fully initialized
			if (!Game::Globals::radiant_initiated)
			{
				Sleep(100);
				continue;
			}

			// socket is invalid
			if (g_RemoteSocket == INVALID_SOCKET || g_RemoteSocketStatus == INVALID_SOCKET)
			{
				break;
			}

			// non-blocking read
			//Game::ServerCommand recvCommands[31]; // 16
			memset(recvCommands, 0, sizeof(recvCommands));

			// locking the thread
			int recvSize = recv(g_RemoteSocket, (char *)&recvCommands, sizeof(recvCommands), 0);

			// skip everything if there's no data
			if (recvSize == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					break;
				}

				// some other problem occurred and now the socket is bad
				shutdown(g_RemoteSocket, 2 /*SD_BOTH*/);
				closesocket(g_RemoteSocket);

				g_RemoteSocket = INVALID_SOCKET;
				printf("[LiveRadiant]: Failed to process server command! Shutting down socket ...\n");

				g_RemoteSocketStatus = INVALID_SOCKET;
				break;
			}


			// determine the number of commands sent, then tell the game
			size_t commandCount = recvSize / sizeof(Game::ServerCommand);

			// command processing
			for (size_t i = 0; i < commandCount; i++)
			{
				if (recvCommands[i].type == Game::SERVER_STRING_MSG)
				{
					if (recvCommands[i].strCommand)
					{
						printf(Utils::VA("[SERVER-MSG]: %s\n", recvCommands[i].strCommand));
					}
				}
				else if (recvCommands[i].type == Game::SERVER_CAMERA_UPDATE)
				{
					if (recvCommands[i].strCommand)
					{
						Components::RemNet::Cmd_ProcessCamera(recvCommands[i].strCommand);
					}
				}

				// idea ::
				// trace brushes in-game -> hightlight (selected), export single brush cmd to radiant

				//else if (recvCommands[i].type == Game::SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX)
				//{
				//	if (recvCommands[i].strCommand)
				//	{
				//		//printf(Utils::VA("[SERVER-MSG]: %s\n", recvCommands[i].strCommand));
				//		Components::RemNet::Server_ProcessBrushSideIndex_Command(recvCommands[i].strCommand);
				//	}
				//}

				//else if (recvCommands[i].type == Game::SERVER_EXPORT_SINGLE_BRUSH_FACE)
				//{
				//	if (recvCommands[i].strCommand)
				//	{
				//		//printf(Utils::VA("[SERVER-MSG]: %s\n", recvCommands[i].strCommand));
				//		Components::RemNet::Server_ProcessBrushSide_Command(recvCommands[i].strCommand);
				//	}
				//}

				else
				{
					printf("[SERVER-MSG]: received unkown command-type!\n");
				}
			}
		}
	}
}

// ------------------------------------------

namespace Components
{
	// -----------------
	// Receiving packets 

	// *
	// Change radiants camera
	void RemNet::Cmd_ProcessCamera(const char *strCommand)
	{
		if (strCommand)
		{
			if (CCamWnd::ActiveWindow)
			{
				if (!sscanf(strCommand, "%f %f %f %f %f %f",
					&CCamWnd::ActiveWindow->cameraOrigin[0], &CCamWnd::ActiveWindow->cameraOrigin[1], &CCamWnd::ActiveWindow->cameraOrigin[2],
					&CCamWnd::ActiveWindow->cameraAngles[0], &CCamWnd::ActiveWindow->cameraAngles[1], &CCamWnd::ActiveWindow->cameraAngles[2]))
				{
					printf("[!]: sscanf failed to read command of type: SERVER_CAMERA_UPDATE\n");
				}

				if (Dvars::radiant_liveDebug->current.enabled)
				{
					printf(Utils::VA("[SERVER-CMD]: SERVER_CAMERA_UPDATE (Origin: (%.1f %.1f %.1f) Angles: (%.1f %.1f %.1f))\n",
						CCamWnd::ActiveWindow->cameraOrigin[0], CCamWnd::ActiveWindow->cameraOrigin[1], CCamWnd::ActiveWindow->cameraOrigin[2],
						CCamWnd::ActiveWindow->cameraAngles[0], CCamWnd::ActiveWindow->cameraAngles[1], CCamWnd::ActiveWindow->cameraAngles[2]));
				}
			}
			else
			{
				printf("[!] CCamWnd::ActiveWindow was NULL\n");
				return;
			}

			Game::Globals::m_pCamWnd_ref->RedrawWindow(NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW);

			// Debug entries not working as the backend is not active ..
			/*auto gfxFront = GET_FRONTENTDATA;

			float mins[3] = { -48.0f, 224.0f, 16.0f };
			float maxs[3] = { 48.0f, 304.0f, 96.0f };
			float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

			Game::R_AddDebugBox(gfxFront->debugGlobals, mins, maxs, color);

			int y = 1;*/
		}

		else
		{
			printf("[!][SERVER-CMD]: failed to parse command (SERVER_CAMERA_UPDATE)!\n");
		}
	}

	// not in use rn
	void RemNet::Cmd_ProcessBrushSideIndex(const char *strCommand)
	{
		if (strCommand)
		{
			if (!sscanf(strCommand, "%d", &Game::Globals::cServerCmd.brush_currentSideIndex))
			{
				printf("[!]: sscanf failed to read command of type: SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX\n");
			}

			// uh for now .. because i can
			if (Game::Globals::cServerCmd.brush_currentSideIndex > 5)
			{
				Game::Globals::cServerCmd.brush_currentSideIndex = 5;
			}

			if(Dvars::radiant_liveDebug->current.enabled)
				printf(Utils::VA("[SERVER-CMD]: SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX (%d)\n", Game::Globals::cServerCmd.brush_currentSideIndex));
		}

		else
		{
			printf("[!][SERVER-CMD]: failed to parse command (SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX)!\n");
		}
	}


	// not in use rn
	void RemNet::Cmd_ProcessBrushSide(const char *strCommand)
	{
		Game::selbrush_t *selectedBrushes = SELECTED_BRUSHES_GET_ARRAY;

		// we need to have a brush selected to be able to change its sides (for now)
		if (!selectedBrushes->currSelection)
		{
			if (Dvars::radiant_liveDebug->current.enabled)
				printf("[!][SERVER-CMD]: failed to execute command of type (SERVER_EXPORT_SINGLE_BRUSH_FACE). Select a brush to modify first!\n");

			return;
		}

		else
		{
			if (strCommand)
			{
				if (!selectedBrushes->currSelection->brush_faces)
				{
					printf("[!]: selectedBrushes->currSelection->brush_faces == NULL\n");
					return;
				}

				Game::face_t *face = &selectedBrushes->currSelection->brush_faces[Game::Globals::cServerCmd.brush_currentSideIndex];

				if (!sscanf(strCommand, "%f %f %f  %f %f %f  %f %f %f",
					&face->planepts0[0], &face->planepts0[1], &face->planepts0[2],
					&face->planepts1[0], &face->planepts1[1], &face->planepts1[2],
					&face->planepts2[0], &face->planepts2[1], &face->planepts2[2]))
				{
					printf("[!]: sscanf failed to read command of type: SERVER_EXPORT_SINGLE_BRUSH_FACE\n");
				}

				if (Dvars::radiant_liveDebug->current.enabled)
					printf(Utils::VA("[SERVER-CMD]: SERVER_EXPORT_SINGLE_BRUSH_FACE:\n< ( %f %f %f ) ( %f %f %f ) ( %f %f %f ) >\n\n", 
						face->planepts0[0], face->planepts0[1], face->planepts0[2],
						face->planepts1[0], face->planepts1[1], face->planepts1[2],
						face->planepts2[0], face->planepts2[1], face->planepts2[2]));
			}

			else
			{
				printf("[!][SERVER-CMD]: failed to parse command (SERVER_EXPORT_SINGLE_BRUSH_FACE)!\n");
			}
		}
	}
	

	// ---------------
	// Sending packets 

	// command index for printing
	auto commandsSend = 0u;

	// *
	// Send commands to the server
	void RemNet::SendPacket(Game::RadiantCommand *Command)
	{
		// check if radiant is connected to the server
		if (g_RemoteSocket == INVALID_SOCKET || g_RemoteSocketStatus == INVALID_SOCKET)
		{
			return;
		}
			
		int ret = send(g_RemoteSocket, (const char *)Command, sizeof(Game::RadiantCommand), 0);

		if (ret == SOCKET_ERROR)
		{
			printf("[LiveRadiant]: Socket ERROR");
			
			closesocket(g_RemoteSocket);
			g_RemoteSocket = INVALID_SOCKET;
			g_RemoteSocketStatus = INVALID_SOCKET;

			Game::Globals::live_connected = false;
		}

		if (RADIANT_DEBUG_CMDS)
		{
			commandsSend++;

			const char* cmdType = "";

			switch (Command->type)
			{
			case 0:
				cmdType = "RADIANT_COMMAND_SELECT";
				break;
			case 1:
				cmdType = "RADIANT_COMMAND_DESELECT";
				break;
			case 2:
				cmdType = "RADIANT_COMMAND_UPDATE_SELECTED";
				break;
			case 3:
				cmdType = "RADIANT_COMMAND_UPDATE";
				break;
			case 4:
				cmdType = "RADIANT_COMMAND_CREATE";
				break;
			case 5:
				cmdType = "RADIANT_COMMAND_DELETE";
				break;
			case 6:
				cmdType = "RADIANT_COMMAND_CAMERA";
				break;
			case 7:
				cmdType = "RADIANT_COMMAND_BRUSH_SELECT";
				break;
			case 8:
				cmdType = "RADIANT_COMMAND_BRUSH_COUNT";
				break;
			case 9:
				cmdType = "RADIANT_COMMAND_BRUSH_CURRENT_NUM";
				break;
			case 10:
				cmdType = "RADIANT_COMMAND_BRUSH_FACE";
				break;
			case 11:
				cmdType = "RADIANT_COMMAND_BRUSH_FACE_COUNT";
				break;
			case 12:
				cmdType = "RADIANT_COMMAND_BRUSH_FACE_NORMALS";
				break;

			case 30:
				cmdType = "RADIANT_COMMAND_SET_DVAR";
				break;

			default:
				cmdType = "COMMAND_TYPE_UNKOWN";
			}

			printf(Utils::VA("Send command num: [%d] of type: [%s]\n", commandsSend, cmdType));
		}
	}

	void RemNet::Cmd_SendDvar(const char* KVCommand)
	{
		Game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(Game::RadiantCommand));

		cmd.type = Game::RADIANT_COMMAND_SET_DVAR;
		strcpy_s(cmd.strCommand, KVCommand);

		RemNet::SendPacket(&cmd);
	}

	// unused
	void RemNet::Cmd_SendSelect(const char *KVCommand)
	{
		Game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(Game::RadiantCommand));

		cmd.type = Game::RADIANT_COMMAND_SELECT;
		strcpy_s(cmd.strCommand, KVCommand);

		RemNet::SendPacket(&cmd);
	}

	// unused
	void RemNet::Cmd_SendSelectedUpdate(const char *KVCommand)
	{
		Game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(Game::RadiantCommand));

		cmd.type = Game::RADIANT_COMMAND_UPDATE_SELECTED;
		strcpy_s(cmd.strCommand, KVCommand);

		RemNet::SendPacket(&cmd);
	}

	// ~ CMainFrame::UpdateWindows
	// Construct a camera command
	void RemNet::Cmd_SendCameraUpdate(float *Origin, float *Angles)
	{
		Game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(Game::RadiantCommand));

		cmd.type = Game::RADIANT_COMMAND_CAMERA;

		sprintf_s(cmd.strCommand, "{\n\"origin\" \"%.1f %.1f %.1f\"\n\"angles\" \"%.1f %.1f %.1f\"\n}",
			Origin[0], Origin[1], Origin[2], Angles[0], Angles[1], Angles[2]);

		RemNet::SendPacket(&cmd);
	}


	// *
	// Check if input brush is a valid brush
	bool Selection_isBrush(Game::brush_t *brush)
	{
		if (brush && brush->owner && brush->owner->eclass)
		{
			return Utils::Q_stricmp(brush->owner->eclass->name, "worldspawn") == 0;
		}
		
		return false;
	}

	// *
	// Parse a brush and send it to the server :: returns false if brush is invalid
	bool RemNet::Cmd_SendSingleBrush(Game::brush_t *brush, int brushNum)
	{
		// check if it is a valid brush
		if (!Selection_isBrush(brush))
		{
			return false;
		}

#if RADIANT_DEBUG_BRUSH 
		printf("\n++ [REMOTE :: NEW BRUSH START] ++\n\n");
#endif

		int tempFaceCount = brush->faceCount;

		// brushes send to the server only support up to 16 faces for now
		if (tempFaceCount > 16)
		{

#if RADIANT_DEBUG_BRUSH
			printf(Utils::VA("[BRUSH]: Parsing only (%d) of (%d) faces for the current brush!\n", tempFaceCount, brush->faceCount));
#endif
			tempFaceCount = 16;
		}

		Game::RadiantCommand cmd;

		// *
		// CMD :: BRUSH CURRENT INDEX (so the server knows where to place the data)
		memset(&cmd, 0, sizeof(Game::RadiantCommand));

		cmd.type = Game::RADIANT_COMMAND_BRUSH_CURRENT_NUM;
		sprintf_s(cmd.strCommand, "{\n\"brushnum\" \"%d\"\n}", brushNum);

		RemNet::SendPacket(&cmd);

#if RADIANT_DEBUG_BRUSH 
		printf(Utils::VA("[RM-CMD]: Brush num: [%d]\n", brushNum));
#endif


		// *
		// CMD :: BRUSH FACE COUNT (send the amount of brush-faces so that the server can map the winding to the correct face)
		memset(&cmd, 0, sizeof(Game::RadiantCommand));

		cmd.type = Game::RADIANT_COMMAND_BRUSH_FACE_COUNT;
		sprintf_s(cmd.strCommand, "{\n\"brushfacecount\" \"%d\"\n}", tempFaceCount);

		RemNet::SendPacket(&cmd);

#if RADIANT_DEBUG_BRUSH
		printf(Utils::VA("[RM-CMD]: Brush face count: [%d]\n", tempFaceCount));
#endif

		// *
		// calculate the brush origin (to calculate plane distances)
		float origin[3];
		origin[0] = (brush->mins[0] + brush->maxs[0]) * 0.5f;
		origin[1] = (brush->mins[1] + brush->maxs[1]) * 0.5f;
		origin[2] = (brush->mins[2] + brush->maxs[2]) * 0.5f;


		// *
		// CMD :: BRUSH FACE (1 command for each brush-face)
		for (auto face = 0; face < tempFaceCount; face++)
		{
			// check if there is a winding
			if (!brush->brush_faces[face].face_winding)
			{

#if RADIANT_DEBUG_BRUSH 
				printf(Utils::VA("[BRUSH]: Skipping brush-face with no winding points.\n"));
#endif
				// brush was valid
				return true;
			}

			// *
			// CMD :: BRUSH FACE NORMAL / DISTANCE
			memset(&cmd, 0, sizeof(Game::RadiantCommand));

			// convert worldspace plane distances to local space (in relation to the brushes origin)
			// all 3 points are on the same plane so just choose one to calculate the distance from the origin to the plane
			/*float distances[3];
			distances[0] = brush->brush_faces[face].plane.normal[0] * (origin[0] - brush->brush_faces[face].planepts0[0]);
			distances[1] = brush->brush_faces[face].plane.normal[1] * (origin[1] - brush->brush_faces[face].planepts0[1]);
			distances[2] = brush->brush_faces[face].plane.normal[2] * (origin[2] - brush->brush_faces[face].planepts0[2]);*/

			float distance = ( brush->brush_faces[face].plane.normal[0] * (origin[0] - brush->brush_faces[face].planepts0[0]))
							+ (brush->brush_faces[face].plane.normal[1] * (origin[1] - brush->brush_faces[face].planepts0[1]))
							+ (brush->brush_faces[face].plane.normal[2] * (origin[2] - brush->brush_faces[face].planepts0[2]));

			cmd.type = Game::RADIANT_COMMAND_BRUSH_FACE_NORMALS;
			sprintf_s(cmd.strCommand, "{\n\"normal\" \"%f %f %f\"\n\"dist\" \"%f\"}",
				brush->brush_faces[face].plane.normal[0],
				brush->brush_faces[face].plane.normal[1],
				brush->brush_faces[face].plane.normal[2],
				distance); // adding all distances together gets us the final distance

			RemNet::SendPacket(&cmd);

#if RADIANT_DEBUG_BRUSH 
			printf(Utils::VA("[RM-CMD]: Brush face[%d] normal: ( %.3f %.3f %.3f ) | dist: (%.3f)\n", 
				face, brush->brush_faces[face].plane.normal[0], brush->brush_faces[face].plane.normal[1], brush->brush_faces[face].plane.normal[2], distance));
#endif

			// skipping a face would result in odd behavior when drawing debug polys
			int windingPtCount = brush->brush_faces[face].face_winding->numPoints;
			if (windingPtCount > 16)
			{
				windingPtCount = 16;

#if RADIANT_DEBUG_BRUSH 
				printf(Utils::VA("[BRUSH]: Parsing (16) of (%d) points for the current brush-face.\n", windingPtCount));
#endif
			}

			std::vector<float>toAdd(3, 0.0f);
			std::vector<std::vector<float>> windingPtsList(16, toAdd); // windingPtCount

			// for each winding point
			for (auto pt = 0; pt < windingPtCount; pt++)
			{
				windingPtsList[pt][0] = brush->brush_faces[face].face_winding->points[pt][0];
				windingPtsList[pt][1] = brush->brush_faces[face].face_winding->points[pt][1];
				windingPtsList[pt][2] = brush->brush_faces[face].face_winding->points[pt][2];
			}

			// *
			// CMD :: BRUSH FACE WINDINGCOUNT / WINDINGPOINTS
			memset(&cmd, 0, sizeof(Game::RadiantCommand));
			cmd.type = Game::RADIANT_COMMAND_BRUSH_FACE;

			// create the cmd string :: max 16 windings (vec3) // was %.4f
			sprintf_s(cmd.strCommand, "{\n\"windingcount\" \"%d\"\n\"windingpoints\" \"%.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f\"\n}",
				windingPtCount,
				windingPtsList[0][0], windingPtsList[0][1], windingPtsList[0][2],
				windingPtsList[1][0], windingPtsList[1][1], windingPtsList[1][2],
				windingPtsList[2][0], windingPtsList[2][1], windingPtsList[2][2],
				windingPtsList[3][0], windingPtsList[3][1], windingPtsList[3][2],
				windingPtsList[4][0], windingPtsList[4][1], windingPtsList[4][2],
				windingPtsList[5][0], windingPtsList[5][1], windingPtsList[5][2],
				windingPtsList[6][0], windingPtsList[6][1], windingPtsList[6][2],
				windingPtsList[7][0], windingPtsList[7][1], windingPtsList[7][2],
				windingPtsList[8][0], windingPtsList[8][1], windingPtsList[8][2],
				windingPtsList[9][0], windingPtsList[9][1], windingPtsList[9][2],
				windingPtsList[10][0], windingPtsList[10][1], windingPtsList[10][2],
				windingPtsList[11][0], windingPtsList[11][1], windingPtsList[11][2],
				windingPtsList[12][0], windingPtsList[12][1], windingPtsList[12][2],
				windingPtsList[13][0], windingPtsList[13][1], windingPtsList[13][2],
				windingPtsList[14][0], windingPtsList[14][1], windingPtsList[14][2],
				windingPtsList[15][0], windingPtsList[15][1], windingPtsList[15][2]);

#if RADIANT_DEBUG_BRUSH

				printf(Utils::VA("\n[RM-CMD]: WindingPoints for Face [%d] of [%d]: \n%s\n", face + 1, tempFaceCount, cmd.strCommand));
				printf("\n---------------------------------------------\n");
#endif

			RemNet::SendPacket(&cmd);
		}

#if RADIANT_DEBUG_BRUSH
			printf(Utils::VA("\n[RM-CMD]: Parsed brush with [%d] faces.\n", brush->faceCount));
			printf("\n++ [REMOTE :: BRUSH END] ++\n\n");
#endif

		return true;
	}

	// *
	// Triggers on brush manipulation :: gather all selected brushes and call command construction functions
	void RemNet::Cmd_SendBrushSelectDeselect(bool select)
	{
		Game::RadiantCommand cmd;

		// selected_brushes array
		Game::selbrush_t *selectedBrushes = SELECTED_BRUSHES_GET_ARRAY;

		// deselected
		if (!select)
		{
			// CMD :: BRUSH SELECT
			memset(&cmd, 0, sizeof(Game::RadiantCommand));

			cmd.type = Game::RADIANT_COMMAND_BRUSH_SELECT;
			sprintf_s(cmd.strCommand, "{\n\"brushselect\" \"%d\"\n}", 0);

			RemNet::SendPacket(&cmd);

#if RADIANT_DEBUG_BRUSH
			printf("[RM-CMD]: Brush Select (FALSE)\n");
#endif
		}

		// nothing selected
		if (!selectedBrushes->currSelection)
		{

#if RADIANT_DEBUG_BRUSH
			printf("[!] No valid data @ RemNet::Cmd_SendBrushSelectDeselect\n");
#endif
			return;
		}

		// *
		// CMD :: BRUSH SELECT (send one select command to clear all saved brush-data in the game)
		memset(&cmd, 0, sizeof(Game::RadiantCommand));

		cmd.type = Game::RADIANT_COMMAND_BRUSH_SELECT;
		sprintf_s(cmd.strCommand, "{\n\"brushselect\" \"%d\"\n}", 1);

		RemNet::SendPacket(&cmd);

#if RADIANT_DEBUG_BRUSH
		printf("[RM-CMD]: Brush Select (TRUE)\n");
#endif


		// parse brush by brush until there is no valid brush in the selected_brushes array (max. REMOTE_MAX_SEL_BRUSHES) 
		// older selections :: [selectedBrushes->prev->currSelection != null]
		// even older selections :: [selectedBrushes->prev->prev->currSelection != null] and so on ..

		// get the last, "most active" selected brush
		if (RemNet::Cmd_SendSingleBrush(selectedBrushes->currSelection, 0))
		{
			// atleast 1 valid brush
			int selbrushAmount = 1;

			Game::selbrush_t* prev = selectedBrushes->prev;

			// for each selected valid previous brush till REMOTE_MAX_SEL_BRUSHES
			while (prev->currSelection && selbrushAmount < REMOTE_MAX_SEL_BRUSHES)
			{
				if (RemNet::Cmd_SendSingleBrush(prev->currSelection, selbrushAmount))
				{
					selbrushAmount++;
					prev = prev->prev;
				}
				// not a valid brush
				else
				{
					break;
				}
			}

			// *
			// CMD :: BRUSH COUNT (amount of selected brushes so the game knows how many brushes to draw)
			memset(&cmd, 0, sizeof(Game::RadiantCommand));

			cmd.type = Game::RADIANT_COMMAND_BRUSH_COUNT;
			sprintf_s(cmd.strCommand, "{\n\"brushcount\" \"%d\"\n}", selbrushAmount);

			RemNet::SendPacket(&cmd);

#if RADIANT_DEBUG_BRUSH
			printf(Utils::VA("[RM-CMD]: Brush Count: [%d]\n", selbrushAmount));
#endif
		}
	}


	// --------
	// Wrappers

	__declspec(naked) void onBrush_Select_stub()
	{
		const static uint32_t stockFuncAdr = 0x458590;
		const static uint32_t retnPt = 0x47660F;
		
		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	1 // select = true
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm Call	stockFuncAdr
		__asm jmp	retnPt
	}

	__declspec(naked) void onBrush_Deselect_stub()
	{
		const static uint32_t stockFuncAdr = 0x458590;
		const static uint32_t retnPt = 0x4766D0;
		
		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	0 // select = false
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm Call	stockFuncAdr
		__asm jmp	retnPt
	}

	__declspec(naked) void onBrush_DeselectESC_stub()
	{
		const static uint32_t retnPt = 0x48E995;
		
		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	0 // select = false
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm mov	[Game::g_nUpdateBitsPtr], -1
		__asm jmp	retnPt
	}

	__declspec(naked) void onBrush_NewBrushDrag_stub()
	{
		const static uint32_t stockFuncAdr = 0x40A480; //VA
		const static uint32_t retnPt = 0x4681EF;
		
		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	1 // select = true
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm Call	stockFuncAdr
		__asm jmp	retnPt
	}

	__declspec(naked) void onBrush_MoveSelection_DragAndEdge_stub()
	{
		const static uint32_t retnPt = 0x47FEF2;
		
		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	1 // select = true
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm mov	[esp + 0C0h], ecx
		__asm jmp	retnPt
	}

	__declspec(naked) void onBrush_MoveSelection_Vertex_stub()
	{
		const static uint32_t stockFuncAdr = 0x402810;
		const static uint32_t retnPt = 0x47F3FD;

		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	1 // select = true
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm Call	stockFuncAdr
		__asm jmp	retnPt
	}

	__declspec(naked) void onBrush_MoveSelection_Rotation_stub()
	{
		const static uint32_t stockFuncAdr = 0x40A480; //VA
		const static uint32_t retnPt = 0x47FD16;

		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	1 // select = true
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm Call	stockFuncAdr
		__asm jmp	retnPt
	}

	__declspec(naked) void onBrush_Hotkey_RotateZ_stub()
	{
		const static uint32_t stockFuncAdr = 0x45EA20; // what ever that is
		const static uint32_t retnPt = 0x4252A5; // next op

		__asm pushad
		if (Game::Globals::live_connected)
		{
			__asm
			{
				push	1 // select = true
				Call	RemNet::Cmd_SendBrushSelectDeselect
				add		esp, 4
			}
		}
		__asm popad

		__asm Call	stockFuncAdr
		__asm jmp	retnPt
	}


	// *
	// Function that gets called right before radiant closes
	void CFrameWnd_OnClose()
	{
		closesocket(g_RemoteSocket);
		g_RemoteSocketStatus = INVALID_SOCKET;

		// export current dvars
		Config::WriteDvars();
	}

	__declspec(naked) void CFrameWnd_OnClose_stub()
	{
		const static uint32_t stockFuncAdr = 0x5AC436; // HideApplication
		const static uint32_t retnPt = 0x59E458; // next op
		__asm
		{
			pushad
			Call	CFrameWnd_OnClose
			popad

			Call	stockFuncAdr
			jmp		retnPt
		}
	}

	RemNet::RemNet()
	{
		// Hook CFrameWnd::OnClose 
		Utils::Hook(0x59E453, CFrameWnd_OnClose_stub, HOOK_JUMP).install()->quick();

		// selected_brushes includes reflectionProbes and all sorts of helper boxes so always check currSelection->owner->eclass->name = "worldspawn" 
		// if not selected any brush :: selected_brushes->currSelection == null
		// selected_brushes->currSelection = always the latest selection
		// selected_brushes_next->currSelection = always the first brush that was selected

		// Hook @ end of Brush_AddToList to detect Brush Selection after selected_brushes was set
		Utils::Hook(0x47660A, onBrush_Select_stub, HOOK_JUMP).install()->quick();

		// Hook @ end of Brush_RemoveFromList to detect Brush Deselection 
		Utils::Hook(0x4766CB, onBrush_Deselect_stub, HOOK_JUMP).install()->quick();

		// Hook @ end of Select_Deselect to detect Brush Deselection with ESC
		Utils::Hook::Nop(0x48E98B, 10); Utils::Hook(0x48E98B, onBrush_DeselectESC_stub, HOOK_JUMP).install()->quick();

		// Hook @ end of XYWnd::NewBrushDrag to detect new brush dragging
		Utils::Hook(0x4681EA, onBrush_NewBrushDrag_stub, HOOK_JUMP).install()->quick();

		// ... sending brush info to the game before the brush even snapped to the next grid point ... slow af
		//Utils::Hook(0x480238, onBrush_MoveSelection_AllEdits_stub, HOOK_JUMP).install()->quick();

		// Hook @ middle of MoveSelection to detect brush moving / resizing / edge dragging (not vertex edits) (after brush was rebuild)
		Utils::Hook::Nop(0x47FEEB, 7); Utils::Hook(0x47FEEB, onBrush_MoveSelection_DragAndEdge_stub, HOOK_JUMP).install()->quick();

		// Hook @ end of MoveSelection to detect brush vertex editing
		Utils::Hook(0x47F3F8, onBrush_MoveSelection_Vertex_stub, HOOK_JUMP).install()->quick();

		// Hook @ middle of MoveSelection to detect brush rotation
		Utils::Hook(0x47FD11, onBrush_MoveSelection_Rotation_stub, HOOK_JUMP).install()->quick();

		// Hook @ end of "Hotkey_Brush_RotateZ" to detect brush rotation by Shift + D
		Utils::Hook(0x4252A0, onBrush_Hotkey_RotateZ_stub, HOOK_JUMP).install()->quick();
	}

	RemNet::~RemNet()
	{ }
}
