#include "std_include.hpp"

namespace utils::fs
{
	/**
	 * @brief			write to a file within the home-path (bin)  
	 * @param sub_dir	sub directory within home-path (bin) 
	 * @param file_name	the file name
	 * @param print		print generic error message when we fail to open a handle
	 * @param file		in-out file handle
	 * @return			file handle state (valid or not)
	 */
	bool write_file_homepath(const std::string& sub_dir, const std::string& file_name, bool print, std::ofstream& file)
	{
		// path to radiant
		if (dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
			dvars::fs_homepath)
		{
			std::string	file_path = dvars::fs_homepath->current.string;
						file_path += "\\" + sub_dir + "\\" + file_name;

			file.open(file_path);
			if (!file.is_open())
			{
				if (print)
				{
					game::printf_to_console("[ERR] Could not write file '%s'.\nAborting!", file_path.c_str());
				}

				return false;
			}

			return true;
		}

		return false;
	}

	/**
	 * @brief			open handle to a file within the home-path (bin)
	 * @param sub_dir	sub directory within home-path (bin)
	 * @param file_name	the file name
	 * @param print		print generic error message when we fail to open a handle
	 * @param file		in-out file handle
	 * @return			file handle state (valid or not)
	 */
	bool open_file_homepath(const std::string& sub_dir, const std::string& file_name, bool print, std::ifstream& file)
	{
		// path to radiant
		if (dvars::fs_homepath = game::Dvar_FindVar("fs_homepath");
			dvars::fs_homepath)
		{
			std::string	file_path = dvars::fs_homepath->current.string;
						file_path += "\\" + sub_dir + "\\" + file_name;

			file.open(file_path);
			if (!file.is_open())
			{
				if (print)
				{
					game::printf_to_console("[ERR] Could not open file '%s'.\nAborting!", file_path.c_str());
				}
				
				return false;
			}

			return true;
		}

		return false;
	}
}
