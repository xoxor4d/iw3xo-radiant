#include "std_include.hpp"

namespace components
{
    std::map<std::string, std::function<void(std::vector<std::string>)>> command::cmd;
	std::vector<std::string> command::cmd_names_autocomplete;
	
    void command::register_command(const std::string &name, std::function<void(std::vector<std::string>)> cb)
    {
        command::cmd[name] = cb;
		command::cmd_names_autocomplete.push_back(name);
    }

	void command::register_command_with_hotkey(const std::string& name, std::function<void(std::vector<std::string>)> cb)
	{
		command::cmd[name] = cb;
		command::cmd_names_autocomplete.push_back(name);
		ggui::cmd_addon_hotkeys.emplace_back(game::SCommandInfoHotkey { name, 0, 0 });
	}

	void command::execute(std::string cmd_name)
	{
		if (command::cmd.find(cmd_name) != command::cmd.end())
		{
			std::vector<std::string> null;
			command::cmd[cmd_name](null);
		}
		else
		{
			game::console_error(utils::va("Unknown command \"%s\"", cmd_name.c_str()));
		}
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
    		if(!game::glob::command_thread_running)
    		{
				break;
    		}
    		
    		std::string input;
    		std::getline(std::cin, input);

    		if(input != "")
    		{
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
			else
			{
				Sleep(100);
			}
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
