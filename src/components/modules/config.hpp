#pragma once

namespace components
{
	class config : public component
	{
	public:
		config();
		~config();
		const char* get_name() override { return "config"; };

		static void		load_dvars();
		static void		write_dvars();
	};
}
