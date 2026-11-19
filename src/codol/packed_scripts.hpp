#pragma once

// The game's server scripts. The offline release ships no loose scripts: the GSC of the server is packed in
// <game>\main\sh1x_svr_scripts.iwd ( a plain zip, entries "scripts/maps/mp/<map>.gsc", "scripts/maps/createfx/..",
// plus waypoints\ and mp/botclasstable.csv ). The server reads scripts from it ( scriptPath fs:scripts ) and lays
// a loose <game>\main\sh1x_rawfiles\<path> over it file by file ( scriptOverlay ): that folder is where edited
// scripts and the level scripts of built maps belong.
//
//   script folder     codol_script_root; when it is empty it is set to <game>\main\sh1x_rawfiles ( created ).
//   reference copy    every script of the archive, unpacked into <editor>\packed_scripts\ ( read only: a file
//                     there changes nothing in the game ). Refreshed when the archive changes; it is the reference
//                     tree of the script workbench while codol_script_ref_root is empty.
//   game paths        "maps/mp/mp_crash.gsc" ( no "scripts/" in front, either slash ).
//
// Implemented by the standalone editor ( editor/src/packed_scripts.cpp, it links codol_render's zip reader );
// shared code uses it under CODOL_STANDALONE.

#include <filesystem>
#include <string>
#include <string_view>

namespace codol::packed_scripts
{
	std::filesystem::path archive_path();			// <game>\main\sh1x_svr_scripts.iwd; empty without an install folder
	std::filesystem::path default_script_root();	// <game>\main\sh1x_rawfiles; empty without an install folder
	std::filesystem::path reference_root();			// <editor>\packed_scripts

	// the script folder to use: codol_script_root, else the default one ( empty without an install folder )
	std::filesystem::path script_root();

	// Once per frame ( cheap ): fills an empty codol_script_root with the default folder and creates it, and
	// refreshes the reference copy when the archive is new or changed.
	void ensure_defaults();

	// The text of a script by its game path, as the server would load it: the script folder, the reference folder
	// of the settings ( codol_script_ref_root ), then the archive. `from` names where it came from.
	bool read(std::string_view game_path, std::string& out, std::string* from = nullptr);

	// whether the archive has the script
	bool packed(std::string_view game_path);

	enum class extract_result
	{
		exists,			// the script folder has its own copy already ( never overwritten )
		extracted,		// copied out of the archive
		not_packed,		// the archive has no such script
		failed,			// no script folder, or it could not be written
	};

	// Copies one script out of the archive into the script folder, so that an edit of it becomes the override.
	// `file` is the loose file ( for exists and extracted ).
	extract_result extract_for_edit(std::string_view game_path, std::filesystem::path* file = nullptr, std::string* error = nullptr);

	// Unpacks every script of the archive into `dest` ( empty: reference_root() ); returns the number of files,
	// -1 on an error.
	int extract_all(const std::filesystem::path& dest = {}, std::string* error = nullptr);
}
