#pragma once

namespace components
{
	class time_wasted : public component
	{
	public:
		time_wasted();
		~time_wasted();
		const char* get_name() override { return "time_wasted"; };

		static time_wasted* p_this;
		static time_wasted* get() { return p_this; }

		struct wasted_entry
		{
			std::string name;
			int time;
		};

		std::string get_map_string();
		void write_entries_to_file();
		void load_entries_from_file();

		wasted_entry* get_entry(const std::string& name);
		std::vector<wasted_entry> m_entries;
	};
}
