#include "std_include.hpp"

namespace components
{
    std::map<std::string, std::function<void(std::vector<std::string>)>> command::cmd;

    void command::register_command(const std::string &name, std::function<void(std::vector<std::string>)> cb)
    {
        command::cmd[name] = cb;
    }

    void command::execute_command(std::vector<std::string> args)
    {
    	if (command::cmd.find(args[0]) != command::cmd.end())
    	{
            command::cmd[args[0]](args);
    	}
    	else 
    	{
    		game::console_error(utils::va("Unknown command \"%s\"", args[0].data()));
    	}
    }

    void command::command_thread()
    {
    	while (true)
    	{
    		std::string input;
    		std::getline(std::cin, input);

    		std::vector<std::string> args;

    		if (input.find(' ') != std::string::npos) 
    		{
    			args = utils::split(input, ' ');
    		}
    		else 
    		{
    			args.push_back(input);
    		}

            command::execute_command(args);
    	}
    }


	command::command()
	{
	}

	command::~command()
	{ 
        /* for (auto command : command::Functions)
		{
			delete command;
		}

		command::Functions.clear(); */
    }
}
