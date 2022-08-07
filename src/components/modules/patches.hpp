#pragma once

namespace components
{
	class patches : public component
	{
	public:
		patches();
		~patches();
		const char* get_name() override { return "patches"; };
	
	private:
	};
}
