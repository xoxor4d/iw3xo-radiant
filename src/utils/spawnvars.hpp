#pragma once

namespace utils
{
	class spawnvars
	{
	public:
		spawnvars() {};
		spawnvars(const char* string, size_t lenPlusOne) : spawnvars(std::string(string, lenPlusOne - 1)) {}
		spawnvars(std::string buffer) : spawnvars() { this->parse(buffer); };
		spawnvars(const spawnvars&obj) : spawnvars_(obj.spawnvars_) {};

		struct script_model_for_dobj_s
		{
			std::string  model_name;
			game::vec3_t origin;
			game::vec3_t angles;
		};

		void get_script_models_for_dobj(std::vector<spawnvars::script_model_for_dobj_s>& dobjs);
	
	private:
		enum
		{
			PARSE_AWAIT_KEY,
			PARSE_READ_KEY,
			PARSE_AWAIT_VALUE,
			PARSE_READ_VALUE,
		};

		std::vector<std::unordered_map<std::string, std::string>> spawnvars_;
		void parse(std::string buffer);
	};
}
