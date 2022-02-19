#pragma once

namespace components
{
	class discord : public component
	{
	public:
		discord();
		~discord();
		const char* get_name() override { return "discord"; };

		static void		update_discord();
		static void		init();
		static void		shutdown();

		static inline bool g_enable_discord_rpc = true;

	private:

	};
}
