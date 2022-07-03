#include "std_include.hpp"
#include <discord_rpc.h>

namespace components
{
	DiscordRichPresence discord_presence;
	bool discord_presence_initialized = false;

	void discord::update_discord()
	{
		Discord_RunCallbacks();

		static bool time_init = false;
		static std::string last_map_name = "";

	
		std::string mapname = std::string(game::current_map_filepath);
		const bool is_editing_prefab = utils::string_contains(mapname, "prefabs");

		const bool fx_editor_active = GET_GUI(ggui::effects_editor_dialog)->is_active();

		utils::replace(mapname, "/", "\\");
		mapname = mapname.substr(mapname.find_last_of("\\") + 1);

		if(!time_init || !is_editing_prefab && last_map_name != mapname)
		{
			time_init = true;
			last_map_name = mapname;

			discord_presence.startTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
		}

		discord_presence.state = "";

		if (fx_editor_active)
		{
			if (const auto	editor_effect = fx_system::get_editor_effect();
							editor_effect)
			{
				discord_presence.state = utils::va("Editing effect: %s", editor_effect->name);
			}
		}
		else if (is_editing_prefab)
		{
			discord_presence.state = utils::va("Editing prefab: %s", mapname.c_str());
		}

		discord_presence.details = last_map_name.c_str();
		discord_presence.partySize = 0;
		discord_presence.partyMax = 0;

		discord_presence.largeImageKey = "icon";

		Discord_UpdatePresence(&discord_presence);
	}

	static void ready([[maybe_unused]] const DiscordUser* request)
	{
		ZeroMemory(&discord_presence, sizeof(discord_presence));

		discord_presence.instance = 1;

		Discord_UpdatePresence(&discord_presence);
	}

	static void errored(const int error_code, const char* message)
	{
		game::printf_to_console("Discord: (%i) %s", error_code, message);
	}

	void discord::init()
	{
		if (g_enable_discord_rpc && !discord_presence_initialized)
		{
			DiscordEventHandlers handlers;
			ZeroMemory(&handlers, sizeof(handlers));
			handlers.ready = ready;
			handlers.errored = errored;
			handlers.disconnected = errored;
			handlers.joinGame = nullptr;
			handlers.spectateGame = nullptr;
			handlers.joinRequest = nullptr;

			Discord_Initialize("944578423001612328", &handlers, 1, nullptr);
			discord_presence_initialized = true;
		}
	}

	void discord::shutdown()
	{
		if (discord_presence_initialized)
		{
			Discord_Shutdown();
			discord_presence_initialized = false;
		}
	}

	discord::discord()
	{
		discord::init();
	}

	discord::~discord()
	{
		discord::shutdown();
	}
}
