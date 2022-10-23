#include "std_include.hpp"

namespace components
{
	time_wasted* time_wasted::p_this = nullptr;

	/**
	 * @return	current map string without ext.
	 */
	std::string time_wasted::get_map_string()
	{
		std::string mapname = std::string(game::current_map_filepath);

		utils::replace(mapname, "/", "\\");
		mapname = mapname.substr(mapname.find_last_of("\\") + 1);

		utils::erase_substring(mapname, ".map");

		return mapname;
	}

	void time_wasted::write_entries_to_file()
	{
		std::ofstream file;
		if (utils::fs::write_file_homepath("IW3xRadiant", "time_wasted.ini", false, file))
		{
			for (const auto& entry : m_entries)
			{
				file << entry.name << "," << entry.time << std::endl;
			}

			file.close();
		}
	}

	void time_wasted::load_entries_from_file()
	{
		m_entries.clear();

		std::ifstream file;
		if (utils::fs::open_file_homepath("IW3xRadiant", "time_wasted.ini", false, file))
		{
			std::string input;
			std::vector<std::string> args;

			// read line by line
			while (std::getline(file, input))
			{
				// split string ','
				args = utils::split(input, ',');

				if (args.size() == 2)
				{
					const auto time = utils::try_stoi(args[1], true);
					m_entries.emplace_back(args[0], time);
				}
			}

			file.close();
		}
	}

	/**
	 * @brief		get wasted entry based on given name
	 * @param name	map name
	 * @return		pointer to wasted entry
	 */
	time_wasted::wasted_entry* time_wasted::get_entry(const std::string& name)
	{
		for (size_t e = 0u; e < m_entries.size(); e++)
		{
			if (m_entries[e].name == name)
			{
				return &m_entries[e];
			}
		}

		return nullptr;
	}

	time_wasted::time_wasted()
	{
		time_wasted::p_this = this;

		// initial load (once)
		components::exec::on_gui_once([]
		{
			time_wasted::get()->load_entries_from_file();
		});

		// yes
		components::exec::on_gui_recurring([]
		{
			const auto tw = time_wasted::get();

			if (!game::glob::is_loading_map)
			{
				std::string mapname = tw->get_map_string();

				if (const auto& entry = tw->get_entry(mapname);
					entry)
				{
					entry->time += 1;
				}
				else
				{
					tw->m_entries.emplace_back(mapname, 1);
				}

				tw->write_entries_to_file();
			}

		}, 60s);
	}

	time_wasted::~time_wasted()
	{ }
}
