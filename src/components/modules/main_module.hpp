#pragma once

namespace components
{
	class main_module : public component
	{
	public:
		main_module();
		~main_module();
		const char* get_name() override { return "main_module"; };
	
	private:
	};
}
