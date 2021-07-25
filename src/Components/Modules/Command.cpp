#include "STDInclude.hpp"

namespace Components
{
    std::map<std::string, std::function<void(std::vector<std::string>)>> Command::cmd;

    void Command::RegisterCommand(const std::string &name, std::function<void(std::vector<std::string>)> cb)
    {
        Command::cmd[name] = cb;
    }

    void Command::ExecuteCommand(std::vector<std::string> args)
    {
    	if (Command::cmd.find(args[0]) != Command::cmd.end())
    	{
            Command::cmd[args[0]](args);
    	}
    	else 
    	{
    		Game::ConsoleError(Utils::VA("Unknown command \"%s\"", args[0].data()));
    	}
    }

    void Command::CommandThread()
    {
    	while (true)
    	{
    		std::string input;
    		std::getline(std::cin, input);

    		std::vector<std::string> args;

    		if (input.find(' ') != std::string::npos) 
    		{
    			args = Utils::split(input, ' ');
    		}
    		else 
    		{
    			args.push_back(input);
    		}

            Command::ExecuteCommand(args);
    	}
    }


	Command::Command()
	{
	}

	Command::~Command()
	{ 
        /* for (auto command : Command::Functions)
		{
			delete command;
		}

		Command::Functions.clear(); */
    }
}
