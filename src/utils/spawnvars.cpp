#include "std_include.hpp"

namespace utils
{
	void spawnvars::get_script_models_for_dobj(std::vector<spawnvars::script_model_for_dobj_s>& dobjs)
	{
		dobjs.reserve(32);

		for (auto& entity : this->spawnvars_)
		{
			std::string classname = entity["classname"];

			if (   !entity["classname"].empty() &&  utils::string_contains(entity["classname"], "script_model")
				&&  entity.contains("model")
				&& !entity.contains("script_exploder") // exclude script_exploder
				&& !entity.contains("script_gameobjectname")) // exclude script_gameobjectname
			{
				if (!entity["model"].empty())
				{
					spawnvars::script_model_for_dobj_s obj = {};
					obj.model_name = entity["model"];

					if(!entity["origin"].empty())
					{
						sscanf_s(entity["origin"].c_str(), "%f %f %f", &obj.origin[0], &obj.origin[1], &obj.origin[2]);
					}

					if (!entity["angles"].empty())
					{
						sscanf_s(entity["angles"].c_str(), "%f %f %f", &obj.angles[0], &obj.angles[1], &obj.angles[2]);
					}
					
					dobjs.push_back(obj);
				}
			}
		}
	}

	void spawnvars::parse(std::string buffer)
	{
		int parse_state= 0;

		std::string key;
		std::string value;

		std::unordered_map<std::string, std::string> entity;

		for (unsigned int i = 0; i < buffer.size(); ++i)
		{
			const char character = buffer[i];
			if (character == '{')
			{
				entity.clear();
			}

			switch (character)
			{
				case '{':
				{
					entity.clear();
					break;
				}

				case '}':
				{
					this->spawnvars_.push_back(entity);
					entity.clear();
					break;
				}

				case '"':
				{
					if (parse_state == PARSE_AWAIT_KEY)
					{
						key.clear();
						parse_state = PARSE_READ_KEY;
					}
					else if (parse_state == PARSE_READ_KEY)
					{
						parse_state = PARSE_AWAIT_VALUE;
					}
					else if (parse_state == PARSE_AWAIT_VALUE)
					{
						value.clear();
						parse_state = PARSE_READ_VALUE;
					}
					else if (parse_state == PARSE_READ_VALUE)
					{
						entity[utils::str_to_lower(key)] = value;
						parse_state = PARSE_AWAIT_KEY;
					}
					else
					{
						throw std::runtime_error("Parsing error!");
					}
					break;
				}

				default:
				{
					if (parse_state == PARSE_READ_KEY)
					{
						key.push_back(character);
					}
					else if (parse_state == PARSE_READ_VALUE)
					{
						value.push_back(character);
					}

					break;
				}
			}
		}
	}
}
