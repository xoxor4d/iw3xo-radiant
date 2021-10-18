#include "std_include.hpp"

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
DWORD WINAPI remote_net_search_server_thread(LPVOID)
{
	WSADATA wsaData;

	game::printf_to_console("[LiveRadiant]: Initiating ...");
	
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		game::printf_to_console("[LiveRadiant]: WSAStartup ERROR!\n");
		return false;
	}

	while (true)
	{
		// wait for config to load
		if (!game::glob::radiant_config_loaded)
		{
			// no config file found, registered default dvars 
			if (game::glob::radiant_config_not_found && dvars::radiant_live && dvars::radiant_live->current.enabled)
			{
				goto INIT_DEFAULT;
			}

			Sleep(200);
			continue;
		}
		else
		{
			// wait till the user enables live-link
			if (dvars::radiant_live && !dvars::radiant_live->current.enabled)
			{
				Sleep(1000);
				continue;
			}
		}

		
	INIT_DEFAULT:
		game::printf_to_console("[LiveRadiant]: Listening for game server on localhost:%d ...\n", dvars::radiant_livePort ? dvars::radiant_livePort->current.integer : 3700);
		
		Sleep(200);

		// connect to the remote game TCP server (doing this in the while loop too if radiant is running and the server is not )
		g_RemoteSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (g_RemoteSocket == INVALID_SOCKET)
		{
			game::printf_to_console("[LiveRadiant]: Failed to initialize client TCP socket!\n");
			return 0;
		}

		// Loop indefinitely until we successfully connect
		while (true)
		{
			g_RemoteSocketStatus = INVALID_SOCKET;

			sockaddr_in remoteAddr;
			remoteAddr.sin_family = AF_INET;

			if (dvars::radiant_livePort)
			{
				remoteAddr.sin_port = htons((u_short)dvars::radiant_livePort->current.integer);
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
		game::printf_to_console("[LiveRadiant]: Game connected!\n");
		g_RemoteSocketStatus = 1;

		game::glob::live_connected = true;

		// Loop indefinitely until the game disconnects or live-link gets disabled
		while (true)
		{
			if (dvars::radiant_live)
			{
				// Check for a socket error or if live-link was disabled
				if (g_RemoteSocket == INVALID_SOCKET || !dvars::radiant_live->current.enabled)
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
			int len   = sizeof(int);

			if (int retval = getsockopt(g_RemoteSocket, SOL_SOCKET, SO_ERROR, (char*)&error, &len); 
				    retval == SOCKET_ERROR || error != 0)
			{
				break;
			}
				
			Sleep(50);
		}

		game::glob::live_connected = false;

		closesocket(g_RemoteSocket);
		g_RemoteSocketStatus = INVALID_SOCKET;

		game::printf_to_console("[LiveRadiant]: Game disconnected!\n");
	}

	return 0;
}

// *
// Receive and process commands send from the server if radiant is connected to a server 
// Will also close the socket and inform the game that radiant disconnected

game::ServerCommand recv_commands[256];

DWORD WINAPI remote_net_receive_packet_thread(LPVOID)
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
			if (!game::glob::radiant_initiated)
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
			//game::ServerCommand recv_commands[31]; // 16
			memset(recv_commands, 0, sizeof(recv_commands));

			// locking the thread
			const int recv_size = recv(g_RemoteSocket, (char *)&recv_commands, sizeof(recv_commands), 0);

			// skip everything if there's no data
			if (recv_size == SOCKET_ERROR)
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
			const size_t command_count = recv_size / sizeof(game::ServerCommand);

			// command processing
			for (size_t i = 0; i < command_count; i++)
			{
				if (recv_commands[i].type == game::SERVER_STRING_MSG)
				{
					if (recv_commands[i].strCommand[0])
					{
						printf(utils::va("[SERVER-MSG]: %s\n", recv_commands[i].strCommand));
					}
				}
				else if (recv_commands[i].type == game::SERVER_CAMERA_UPDATE)
				{
					if (recv_commands[i].strCommand[0])
					{
						components::remote_net::cmd_process_camera(recv_commands[i].strCommand);
					}
				}

				// TODO!
				// trace brushes in-game -> hightlight (selected), export single brush cmd to radiant

				//else if (recv_commands[i].type == game::SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX)
				//{
				//	if (recv_commands[i].strCommand)
				//	{
				//		//printf(utils::va("[SERVER-MSG]: %s\n", recv_commands[i].strCommand));
				//		components::remote_net::Server_ProcessBrushSideIndex_Command(recv_commands[i].strCommand);
				//	}
				//}

				//else if (recv_commands[i].type == game::SERVER_EXPORT_SINGLE_BRUSH_FACE)
				//{
				//	if (recv_commands[i].strCommand)
				//	{
				//		//printf(utils::va("[SERVER-MSG]: %s\n", recv_commands[i].strCommand));
				//		components::remote_net::Server_ProcessBrushSide_Command(recv_commands[i].strCommand);
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

namespace components
{
	// -----------------
	// Receiving packets 

	// *
	// Change radiants camera
	void remote_net::cmd_process_camera(const char *strCommand)
	{
		if (strCommand)
		{
			if (ccamwnd::activewnd)
			{
				if (!sscanf(strCommand, "%f %f %f %f %f %f",
					&ccamwnd::activewnd->camera.origin[0], &ccamwnd::activewnd->camera.origin[1], &ccamwnd::activewnd->camera.origin[2],
					&ccamwnd::activewnd->camera.angles[0], &ccamwnd::activewnd->camera.angles[1], &ccamwnd::activewnd->camera.angles[2]))
				{
					printf("[!]: sscanf failed to read command of type: SERVER_CAMERA_UPDATE\n");
				}

				if (dvars::radiant_liveDebug->current.enabled)
				{
					printf(utils::va("[SERVER-CMD]: SERVER_CAMERA_UPDATE (Origin: (%.1f %.1f %.1f) Angles: (%.1f %.1f %.1f))\n",
						ccamwnd::activewnd->camera.origin[0], ccamwnd::activewnd->camera.origin[1], ccamwnd::activewnd->camera.origin[2],
						ccamwnd::activewnd->camera.angles[0], ccamwnd::activewnd->camera.angles[1], ccamwnd::activewnd->camera.angles[2]));
				}
			}
			else
			{
				printf("[!] ccamwnd::activewnd was NULL\n");
				return;
			}

			game::glob::m_pCamWnd_ref->RedrawWindow(NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW);

			// Debug entries not working as the backend is not active ..
			/*auto gfxFront = GET_FRONTENTDATA;

			float mins[3] = { -48.0f, 224.0f, 16.0f };
			float maxs[3] = { 48.0f, 304.0f, 96.0f };
			float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

			game::R_AddDebugBox(gfxFront->debugGlobals, mins, maxs, color);

			int y = 1;*/
		}

		else
		{
			printf("[!][SERVER-CMD]: failed to parse command (SERVER_CAMERA_UPDATE)!\n");
		}
	}

	// not in use rn
	void remote_net::Cmd_ProcessBrushSideIndex(const char *strCommand)
	{
		if (strCommand)
		{
			if (!sscanf(strCommand, "%d", &game::glob::cServerCmd.brush_currentSideIndex))
			{
				printf("[!]: sscanf failed to read command of type: SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX\n");
			}

			// uh for now .. because i can
			if (game::glob::cServerCmd.brush_currentSideIndex > 5)
			{
				game::glob::cServerCmd.brush_currentSideIndex = 5;
			}

			if(dvars::radiant_liveDebug->current.enabled)
				printf(utils::va("[SERVER-CMD]: SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX (%d)\n", game::glob::cServerCmd.brush_currentSideIndex));
		}

		else
		{
			printf("[!][SERVER-CMD]: failed to parse command (SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX)!\n");
		}
	}


	// not in use rn
	void remote_net::cmd_process_brushside(const char *strCommand)
	{
		game::selbrush_t *selectedBrushes = SELECTED_BRUSHES_GET_ARRAY;

		// we need to have a brush selected to be able to change its sides (for now)
		if (!selectedBrushes->currSelection)
		{
			if (dvars::radiant_liveDebug->current.enabled)
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

				game::face_t *face = &selectedBrushes->currSelection->brush_faces[game::glob::cServerCmd.brush_currentSideIndex];

				if (!sscanf(strCommand, "%f %f %f  %f %f %f  %f %f %f",
					&face->planepts0[0], &face->planepts0[1], &face->planepts0[2],
					&face->planepts1[0], &face->planepts1[1], &face->planepts1[2],
					&face->planepts2[0], &face->planepts2[1], &face->planepts2[2]))
				{
					printf("[!]: sscanf failed to read command of type: SERVER_EXPORT_SINGLE_BRUSH_FACE\n");
				}

				if (dvars::radiant_liveDebug->current.enabled)
					printf(utils::va("[SERVER-CMD]: SERVER_EXPORT_SINGLE_BRUSH_FACE:\n< ( %f %f %f ) ( %f %f %f ) ( %f %f %f ) >\n\n", 
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
	auto _commands_send = 0u;

	// *
	// Send commands to the server
	void remote_net::send_packet(game::RadiantCommand *Command)
	{
		// check if radiant is connected to the server
		if (g_RemoteSocket == INVALID_SOCKET || g_RemoteSocketStatus == INVALID_SOCKET)
		{
			return;
		}

		if (int ret = send(g_RemoteSocket, (const char*)Command, sizeof(game::RadiantCommand), 0); 
				ret == SOCKET_ERROR)
		{
			printf("[LiveRadiant]: Socket ERROR");
			
			closesocket(g_RemoteSocket);
			g_RemoteSocket = INVALID_SOCKET;
			g_RemoteSocketStatus = INVALID_SOCKET;

			game::glob::live_connected = false;
		}

		if (RADIANT_DEBUG_CMDS)
		{
			_commands_send++;

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

			printf(utils::va("Send command num: [%d] of type: [%s]\n", _commands_send, cmdType));
		}
	}

	void remote_net::Cmd_SendDvar(const char* KVCommand)
	{
		game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(game::RadiantCommand));

		cmd.type = game::RADIANT_COMMAND_SET_DVAR;
		strcpy_s(cmd.strCommand, KVCommand);

		remote_net::send_packet(&cmd);
	}

	// unused
	void remote_net::cmd_send_select(const char *KVCommand)
	{
		game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(game::RadiantCommand));

		cmd.type = game::RADIANT_COMMAND_SELECT;
		strcpy_s(cmd.strCommand, KVCommand);

		remote_net::send_packet(&cmd);
	}

	// unused
	void remote_net::cmd_send_selected_update(const char *KVCommand)
	{
		game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(game::RadiantCommand));

		cmd.type = game::RADIANT_COMMAND_UPDATE_SELECTED;
		strcpy_s(cmd.strCommand, KVCommand);

		remote_net::send_packet(&cmd);
	}

	// ~ cmainframe::update_windows
	// Construct a camera command
	void remote_net::cmd_send_camera_update(float *Origin, float *Angles)
	{
		game::RadiantCommand cmd;
		memset(&cmd, 0, sizeof(game::RadiantCommand));

		cmd.type = game::RADIANT_COMMAND_CAMERA;

		sprintf_s(cmd.strCommand, "{\n\"origin\" \"%.1f %.1f %.1f\"\n\"angles\" \"%.1f %.1f %.1f\"\n}",
			Origin[0], Origin[1], Origin[2], Angles[0], Angles[1], Angles[2]);

		remote_net::send_packet(&cmd);
	}


	// *
	// Check if input brush is a valid brush
	bool selection_is_brush(game::brush_t *brush)
	{
		if (brush && brush->owner && brush->owner->eclass)
		{
			return utils::Q_stricmp(brush->owner->eclass->name, "worldspawn") == 0;
		}
		
		return false;
	}

	// *
	// Parse a brush and send it to the server :: returns false if brush is invalid
	bool remote_net::cmd_send_single_brush(game::brush_t *brush, int brushNum)
	{
		// check if it is a valid brush
		if (!selection_is_brush(brush))
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
			printf(utils::va("[BRUSH]: Parsing only (%d) of (%d) faces for the current brush!\n", tempFaceCount, brush->faceCount));
#endif
			
			tempFaceCount = 16;
		}

		game::RadiantCommand cmd;

		// *
		// CMD :: BRUSH CURRENT INDEX (so the server knows where to place the data)
		memset(&cmd, 0, sizeof(game::RadiantCommand));

		cmd.type = game::RADIANT_COMMAND_BRUSH_CURRENT_NUM;
		sprintf_s(cmd.strCommand, "{\n\"brushnum\" \"%d\"\n}", brushNum);

		remote_net::send_packet(&cmd);

#if RADIANT_DEBUG_BRUSH 
		printf(utils::va("[RM-CMD]: Brush num: [%d]\n", brushNum));
#endif


		// *
		// CMD :: BRUSH FACE COUNT (send the amount of brush-faces so that the server can map the winding to the correct face)
		memset(&cmd, 0, sizeof(game::RadiantCommand));

		cmd.type = game::RADIANT_COMMAND_BRUSH_FACE_COUNT;
		sprintf_s(cmd.strCommand, "{\n\"brushfacecount\" \"%d\"\n}", tempFaceCount);

		remote_net::send_packet(&cmd);

#if RADIANT_DEBUG_BRUSH
		printf(utils::va("[RM-CMD]: Brush face count: [%d]\n", tempFaceCount));
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
				printf(utils::va("[BRUSH]: Skipping brush-face with no winding points.\n"));
#endif
				// brush was valid
				return true;
			}

			// *
			// CMD :: BRUSH FACE NORMAL / DISTANCE
			memset(&cmd, 0, sizeof(game::RadiantCommand));

			// convert worldspace plane distances to local space (in relation to the brushes origin)
			// all 3 points are on the same plane so just choose one to calculate the distance from the origin to the plane
			/*float distances[3];
			distances[0] = brush->brush_faces[face].plane.normal[0] * (origin[0] - brush->brush_faces[face].planepts0[0]);
			distances[1] = brush->brush_faces[face].plane.normal[1] * (origin[1] - brush->brush_faces[face].planepts0[1]);
			distances[2] = brush->brush_faces[face].plane.normal[2] * (origin[2] - brush->brush_faces[face].planepts0[2]);*/

			float distance = ( brush->brush_faces[face].plane.normal[0] * (origin[0] - brush->brush_faces[face].planepts0[0]))
							+ (brush->brush_faces[face].plane.normal[1] * (origin[1] - brush->brush_faces[face].planepts0[1]))
							+ (brush->brush_faces[face].plane.normal[2] * (origin[2] - brush->brush_faces[face].planepts0[2]));

			cmd.type = game::RADIANT_COMMAND_BRUSH_FACE_NORMALS;
			sprintf_s(cmd.strCommand, "{\n\"normal\" \"%f %f %f\"\n\"dist\" \"%f\"}",
				brush->brush_faces[face].plane.normal[0],
				brush->brush_faces[face].plane.normal[1],
				brush->brush_faces[face].plane.normal[2],
				distance); // adding all distances together gets us the final distance

			remote_net::send_packet(&cmd);

#if RADIANT_DEBUG_BRUSH 
			printf(utils::va("[RM-CMD]: Brush face[%d] normal: ( %.3f %.3f %.3f ) | dist: (%.3f)\n", 
				face, brush->brush_faces[face].plane.normal[0], brush->brush_faces[face].plane.normal[1], brush->brush_faces[face].plane.normal[2], distance));
#endif

			// skipping a face would result in odd behavior when drawing debug polys
			int windingPtCount = brush->brush_faces[face].face_winding->numPoints;
			if (windingPtCount > 16)
			{
				windingPtCount = 16;

#if RADIANT_DEBUG_BRUSH 
				printf(utils::va("[BRUSH]: Parsing (16) of (%d) points for the current brush-face.\n", windingPtCount));
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
			memset(&cmd, 0, sizeof(game::RadiantCommand));
			cmd.type = game::RADIANT_COMMAND_BRUSH_FACE;

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

				printf(utils::va("\n[RM-CMD]: WindingPoints for Face [%d] of [%d]: \n%s\n", face + 1, tempFaceCount, cmd.strCommand));
				printf("\n---------------------------------------------\n");
#endif

			remote_net::send_packet(&cmd);
		}

#if RADIANT_DEBUG_BRUSH
			printf(utils::va("\n[RM-CMD]: Parsed brush with [%d] faces.\n", brush->faceCount));
			printf("\n++ [REMOTE :: BRUSH END] ++\n\n");
#endif

		return true;
	}

	// *
	// Triggers on brush manipulation :: gather all selected brushes and call command construction functions
	void remote_net::cmd_send_brush_select_deselect(bool select)
	{
		if (!game::glob::live_connected)
		{
			return;
		}
		
		game::RadiantCommand cmd;

		// selected_brushes array
		game::selbrush_t *selectedBrushes = SELECTED_BRUSHES_GET_ARRAY;

		// deselected
		if (!select)
		{
			// CMD :: BRUSH SELECT
			memset(&cmd, 0, sizeof(game::RadiantCommand));

			cmd.type = game::RADIANT_COMMAND_BRUSH_SELECT;
			sprintf_s(cmd.strCommand, "{\n\"brushselect\" \"%d\"\n}", 0);

			remote_net::send_packet(&cmd);

#if RADIANT_DEBUG_BRUSH
			printf("[RM-CMD]: Brush Select (FALSE)\n");
#endif
		}

		// nothing selected
		if (!selectedBrushes->currSelection)
		{

#if RADIANT_DEBUG_BRUSH
			printf("[!] No valid data @ remote_net::cmd_send_brush_select_deselect\n");
#endif
			return;
		}

		// *
		// CMD :: BRUSH SELECT (send one select command to clear all saved brush-data in the game)
		memset(&cmd, 0, sizeof(game::RadiantCommand));

		cmd.type = game::RADIANT_COMMAND_BRUSH_SELECT;
		sprintf_s(cmd.strCommand, "{\n\"brushselect\" \"%d\"\n}", 1);

		remote_net::send_packet(&cmd);

#if RADIANT_DEBUG_BRUSH
		printf("[RM-CMD]: Brush Select (TRUE)\n");
#endif


		// parse brush by brush until there is no valid brush in the selected_brushes array (max. REMOTE_MAX_SEL_BRUSHES) 
		// older selections :: [selectedBrushes->prev->currSelection != null]
		// even older selections :: [selectedBrushes->prev->prev->currSelection != null] and so on ..

		// get the last, "most active" selected brush
		if (remote_net::cmd_send_single_brush(selectedBrushes->currSelection, 0))
		{
			// atleast 1 valid brush
			int selbrushAmount = 1;

			game::selbrush_t* prev = selectedBrushes->prev;

			// for each selected valid previous brush till REMOTE_MAX_SEL_BRUSHES
			while (prev->currSelection && selbrushAmount < REMOTE_MAX_SEL_BRUSHES)
			{
				if (remote_net::cmd_send_single_brush(prev->currSelection, selbrushAmount))
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
			memset(&cmd, 0, sizeof(game::RadiantCommand));

			cmd.type = game::RADIANT_COMMAND_BRUSH_COUNT;
			sprintf_s(cmd.strCommand, "{\n\"brushcount\" \"%d\"\n}", selbrushAmount);

			remote_net::send_packet(&cmd);

#if RADIANT_DEBUG_BRUSH
			printf(utils::va("[RM-CMD]: Brush Count: [%d]\n", selbrushAmount));
#endif
		}
	}


	// --------
	// Wrappers

	__declspec(naked) void on_brush_select_stub()
	{
		const static uint32_t stockFuncAdr = 0x458590;
		const static uint32_t retnPt = 0x47660F;
		__asm
		{
			pushad;
			push	1; // select = true
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			call	stockFuncAdr;
			jmp		retnPt;
		}
	}

	__declspec(naked) void on_brush_deselect_stub()
	{
		const static uint32_t stockFuncAdr = 0x458590;
		const static uint32_t retnPt = 0x4766D0;
		__asm
		{
			pushad;
			push	0; // select = false
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			call	stockFuncAdr;
			jmp		retnPt;
		}
	}

	__declspec(naked) void on_brush_deselect_esc_stub()
	{
		const static uint32_t retnPt = 0x48E995;
		__asm
		{
			pushad;
			push	0; // select = false
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			mov		[game::g_nUpdateBitsPtr], -1;
			jmp		retnPt;
		}
	}

	__declspec(naked) void on_brush_new_brush_drag_stub()
	{
		const static uint32_t stockFuncAdr = 0x40A480;
		const static uint32_t retnPt = 0x4681EF;
		__asm
		{
			pushad;
			push	1; // select = true
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			call	stockFuncAdr;
			jmp		retnPt;
		}
	}

	__declspec(naked) void on_brush_move_selection_drag_and_edge_stub()
	{
		const static uint32_t retnPt = 0x47FEF2;
		__asm
		{
			pushad;
			push	1; // select = true
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			mov		[esp + 0C0h], ecx;
			jmp		retnPt;
		}
	}

	__declspec(naked) void on_brush_move_selection_vertex_stub()
	{
		const static uint32_t stockFuncAdr = 0x402810;
		const static uint32_t retnPt = 0x47F3FD;
		__asm
		{
			pushad;
			push	1; // select = true
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			call	stockFuncAdr;
			jmp		retnPt;
		}
	}

	__declspec(naked) void on_brush_move_selection_rotation_stub()
	{
		const static uint32_t stockFuncAdr = 0x40A480;
		const static uint32_t retnPt = 0x47FD16;
		__asm
		{
			pushad;
			push	1; // select = true
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			call	stockFuncAdr;
			jmp		retnPt;
		}
	}

	__declspec(naked) void on_brush_hotkey_rotate_z_stub()
	{
		const static uint32_t stockFuncAdr = 0x45EA20;
		const static uint32_t retnPt = 0x4252A5;
		__asm
		{
			pushad;
			push	1; // select = true
			call	remote_net::cmd_send_brush_select_deselect;
			add		esp, 4;
			popad;

			call	stockFuncAdr;
			jmp		retnPt;
		}
	}


	// *
	// Function that gets called right before radiant closes
	void cframewnd_on_close()
	{
		closesocket(g_RemoteSocket);
		g_RemoteSocketStatus = INVALID_SOCKET;

		// export current dvars
		config::write_dvars();
	}

	__declspec(naked) void cframewnd_on_close_stub()
	{
		const static uint32_t CWinApp_HideApplication = 0x5AC436;
		const static uint32_t retn_pt = 0x59E458;
		__asm
		{
			pushad;
			call	cframewnd_on_close;
			popad;

			call	CWinApp_HideApplication;
			jmp		retn_pt;
		}
	}

	void remote_net::register_dvars()
	{
		dvars::radiant_live = dvars::register_bool(
			/* name		*/ "radiant_live",
			/* default	*/ true,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "enables radiant <-> game link.");

		dvars::radiant_livePort = dvars::register_int(
			/* name		*/	"radiant_livePort",
			/* default	*/	3700,
			/* mins		*/	0,
			/* maxs		*/	99999,
			/* flags	*/	game::dvar_flags::saved,
			/* desc		*/	"port to be used for live-link.");

		dvars::radiant_liveDebug = dvars::register_bool(
			/* name		*/ "radiant_liveDebug",
			/* default	*/ false,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "enables debug prints.");
	}

	remote_net::remote_net()
	{
		utils::hook(0x59E453, cframewnd_on_close_stub, HOOK_JUMP).install()->quick();

		// selected_brushes includes reflectionProbes and all sorts of helper boxes so always check currSelection->owner->eclass->name = "worldspawn" 
		// if not selected any brush :: selected_brushes->currSelection == null
		// selected_brushes->currSelection = always the latest selection
		// selected_brushes_next->currSelection = always the first brush that was selected

		// hook @ end of Brush_AddToList to detect Brush Selection after selected_brushes was set
		utils::hook(0x47660A, on_brush_select_stub, HOOK_JUMP).install()->quick();

		// hook @ end of Brush_RemoveFromList to detect Brush Deselection 
		utils::hook(0x4766CB, on_brush_deselect_stub, HOOK_JUMP).install()->quick();

		// hook @ end of Select_Deselect to detect Brush Deselection with ESC
		utils::hook::nop(0x48E98B, 10);
			 utils::hook(0x48E98B, on_brush_deselect_esc_stub, HOOK_JUMP).install()->quick();

		// hook @ end of XYWnd::NewBrushDrag to detect new brush dragging
		utils::hook(0x4681EA, on_brush_new_brush_drag_stub, HOOK_JUMP).install()->quick();

		// ... sending brush info to the game before the brush even snapped to the next grid point ... slow af
		//utils::hook(0x480238, onBrush_MoveSelection_AllEdits_stub, HOOK_JUMP).install()->quick();

		// hook @ middle of MoveSelection to detect brush moving / resizing / edge dragging (not vertex edits) (after brush was rebuild)
		utils::hook::nop(0x47FEEB, 7);
			 utils::hook(0x47FEEB, on_brush_move_selection_drag_and_edge_stub, HOOK_JUMP).install()->quick();

		// hook @ end of MoveSelection to detect brush vertex editing
		utils::hook(0x47F3F8, on_brush_move_selection_vertex_stub, HOOK_JUMP).install()->quick();

		// hook @ middle of MoveSelection to detect brush rotation
		utils::hook(0x47FD11, on_brush_move_selection_rotation_stub, HOOK_JUMP).install()->quick();

		// hook @ end of "Hotkey_Brush_RotateZ" to detect brush rotation by Shift + D
		utils::hook(0x4252A0, on_brush_hotkey_rotate_z_stub, HOOK_JUMP).install()->quick();
	}

	remote_net::~remote_net()
	{ }
}
