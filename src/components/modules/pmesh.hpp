#pragma once

namespace components
{
	class pmesh : public component
	{
	public:
		pmesh();
		~pmesh();
		const char* get_name() override { return "pmesh"; };

		static void cap_current(int type);
	};
}
