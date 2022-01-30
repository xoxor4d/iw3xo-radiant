#pragma once

namespace components
{
	class effects : public component
	{
	public:
		effects();
		~effects();
		const char* get_name() override { return "effects"; };

		static void radiant_init_fx();

	private:
	};
}
