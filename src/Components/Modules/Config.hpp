#pragma once

namespace Components
{
	class Config : public Component
	{
	public:
		Config();
		~Config();
		const char* getName() override { return "Config"; };

		static void		LoadDvars();
		static void		WriteDvars();

	private:
	};
}
