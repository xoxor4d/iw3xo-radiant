#pragma once

namespace Components
{
	class Command : public Component
	{
	public:
		Command();
		~Command();
		const char* getName() override { return "Command"; };

		static void CommandThread();
        static void RegisterCommand(const std::string &name, std::function<void(std::vector<std::string>)> cb);
		static void ExecuteCommand(std::vector<std::string> args);

	private:
        static std::map<std::string, std::function<void(std::vector<std::string>)>> cmd;
	};
}
