#pragma once

namespace utils::fs
{
	bool write_file_homepath(const std::string& sub_dir, const std::string& file_name, bool print, std::ofstream& file);
	bool open_file_homepath(const std::string& sub_dir, const std::string& file_name, bool print, std::ifstream& file);
}
