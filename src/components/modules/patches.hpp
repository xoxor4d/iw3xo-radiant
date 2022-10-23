#pragma once

namespace components
{
	class patches : public component
	{
	public:
		patches();
		~patches();
		const char* get_name() override { return "patches"; };

		static patches* p_this;
		static patches* get() { return p_this; }

	private:
	};
}
