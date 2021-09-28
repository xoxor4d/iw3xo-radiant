#pragma once

namespace components
{
	class quick_patch : public component
	{
	public:
		quick_patch();
		~quick_patch();
		const char* get_name() override { return "quick_patch"; };
	
	private:
	};
}
