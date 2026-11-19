#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "codol/packed_scripts.hpp"
#include "components/modules/codol_game.hpp"

#include <codol_render/iwd.hpp>

#include <mutex>
#include <unordered_map>

// The game's packed server scripts ( see src/codol/packed_scripts.hpp ).

namespace codol::packed_scripts
{
	namespace fs = std::filesystem;

	namespace
	{
		constexpr const char* ARCHIVE_NAME = "sh1x_svr_scripts.iwd";
		constexpr const char* SCRIPT_FOLDER = "sh1x_rawfiles";
		constexpr const char* STAMP_NAME = ".packed_from";
		constexpr std::string_view ENTRY_PREFIX = "scripts/";

		std::string lower_slash(std::string_view path)
		{
			std::string out(path);
			for (char& c : out)
			{
				c = c == '\\' ? '/' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}

			while (!out.empty() && out.front() == '/') { out.erase(out.begin()); }
			return out;
		}

		std::string dvar_text(const game::dvar_s* dvar)
		{
			return dvar && dvar->current.string ? dvar->current.string : "";
		}

		// the open archive, by its path and time; reopened when either changes
		struct cache
		{
			std::mutex lock;
			fs::path path;
			fs::file_time_type time = {};
			std::unique_ptr<codol::render::iwd_archive> archive;
			std::unordered_map<std::string, const codol::render::iwd_entry*> by_path;		// game path ( lower case ) -> entry
		};

		cache& get_cache()
		{
			static cache c;
			return c;
		}

		// with the lock held
		const codol::render::iwd_archive* open_archive(cache& c)
		{
			const fs::path path = archive_path();
			std::error_code ec;
			const auto time = path.empty() ? fs::file_time_type() : fs::last_write_time(path, ec);

			if (path.empty() || ec)
			{
				c.archive.reset();
				c.by_path.clear();
				c.path.clear();
				return nullptr;
			}

			if (c.archive && c.path == path && c.time == time)
			{
				return c.archive.get();
			}

			c.archive.reset();
			c.by_path.clear();
			c.path = path;
			c.time = time;

			std::string error;
			c.archive = codol::render::iwd_archive::open(path, nullptr, &error);

			if (!c.archive)
			{
				editor::log("^1[scripts] could not open " + editor::to_utf8(path) + ": " + error);
				return nullptr;
			}

			for (const auto& entry : c.archive->entries())
			{
				const std::string name = lower_slash(entry.name);
				if (name.size() > ENTRY_PREFIX.size() && name.compare(0, ENTRY_PREFIX.size(), ENTRY_PREFIX) == 0 && name.back() != '/')
				{
					c.by_path.emplace(name.substr(ENTRY_PREFIX.size()), &entry);
				}
			}

			return c.archive.get();
		}

		bool read_packed(std::string_view game_path, std::string& out)
		{
			auto& c = get_cache();
			std::lock_guard guard(c.lock);

			const auto* archive = open_archive(c);
			if (!archive) { return false; }

			const auto it = c.by_path.find(lower_slash(game_path));
			if (it == c.by_path.end()) { return false; }

			std::vector<std::uint8_t> data;
			std::string error;

			if (!archive->read(*it->second, data, &error))
			{
				editor::log("^1[scripts] " + std::string(game_path) + " of " + ARCHIVE_NAME + ": " + error);
				return false;
			}

			out.assign(reinterpret_cast<const char*>(data.data()), data.size());
			return true;
		}

		bool read_file(const fs::path& file, std::string& out)
		{
			std::error_code ec;
			if (!fs::is_regular_file(file, ec)) { return false; }

			std::ifstream in(file, std::ios::binary);
			if (!in.is_open()) { return false; }

			out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
			return true;
		}

		bool write_new_file(const fs::path& file, const std::string& text, std::string* error)
		{
			std::error_code ec;
			fs::create_directories(file.parent_path(), ec);

			std::ofstream out(file, std::ios::binary | std::ios::trunc);
			if (!out.is_open())
			{
				if (error) { *error = "could not write " + editor::to_utf8(file); }
				return false;
			}

			out.write(text.data(), static_cast<std::streamsize>(text.size()));
			return static_cast<bool>(out);
		}

		std::string stamp_of(const fs::path& archive)
		{
			std::error_code ec;
			const auto size = fs::file_size(archive, ec);
			const auto time = fs::last_write_time(archive, ec);
			return editor::to_utf8(fs::absolute(archive, ec)) + "\n" + std::to_string(size) + "\n" + std::to_string(time.time_since_epoch().count()) + "\n";
		}

		bool same_folder(const fs::path& a, const fs::path& b)
		{
			std::error_code ec;
			if (a.empty() || b.empty()) { return false; }
			if (fs::exists(a, ec) && fs::exists(b, ec)) { return fs::equivalent(a, b, ec); }
			return _wcsicmp(fs::absolute(a, ec).lexically_normal().wstring().c_str(), fs::absolute(b, ec).lexically_normal().wstring().c_str()) == 0;
		}

		std::string g_seen_install = "\x01";		// what ensure_defaults last looked at
		bool g_commands = false;

		void register_commands()
		{
			// scripts_extract [folder]: every script of sh1x_svr_scripts.iwd into the folder ( default: the reference copy )
			command::register_command("scripts_extract", [](const std::vector<std::string>& args)
			{
				std::string error;
				const fs::path dest = args.size() > 1 ? fs::u8path(args[1]) : fs::path();
				const int n = extract_all(dest, &error);

				if (n < 0) { editor::log("^1[scripts] " + error); }
				else { editor::log("[scripts] " + std::to_string(n) + " scripts of " + ARCHIVE_NAME + " -> " + editor::to_utf8(dest.empty() ? reference_root() : dest)); }
			});

			// scripts_unpack <game path>: one script of the archive into the script folder ( to edit it )
			command::register_command("scripts_unpack", [](const std::vector<std::string>& args)
			{
				if (args.size() < 2)
				{
					editor::log("usage: scripts_unpack <maps/mp/<map>.gsc>   copies the packed script into the script folder ( an existing file is kept )");
					return;
				}

				fs::path file;
				std::string error;

				switch (extract_for_edit(args[1], &file, &error))
				{
				case extract_result::exists:     editor::log("[scripts] the script folder has it already: " + editor::to_utf8(file)); break;
				case extract_result::extracted:  editor::log("[scripts] unpacked to " + editor::to_utf8(file)); break;
				case extract_result::not_packed: editor::log("^3[scripts] " + args[1] + " is not in " + ARCHIVE_NAME); break;
				case extract_result::failed:     editor::log("^1[scripts] " + error); break;
				}
			});
		}
	}

	fs::path archive_path()
	{
		const std::string game = components::codol_game::install_path();
		return game.empty() ? fs::path() : fs::u8path(game) / "main" / ARCHIVE_NAME;
	}

	fs::path default_script_root()
	{
		const std::string game = components::codol_game::install_path();
		return game.empty() ? fs::path() : fs::u8path(game) / "main" / SCRIPT_FOLDER;
	}

	fs::path reference_root()
	{
		return fs::u8path(utils::fs::get_basepath()) / "packed_scripts";
	}

	fs::path script_root()
	{
		std::string root = dvar_text(dvars::codol_script_root);
		while (!root.empty() && (root.back() == '\\' || root.back() == '/')) { root.pop_back(); }
		return root.empty() ? default_script_root() : fs::u8path(root);
	}

	void ensure_defaults()
	{
		if (!g_commands)
		{
			g_commands = true;
			register_commands();
		}

		const std::string install = components::codol_game::install_path();
		if (install == g_seen_install) { return; }
		g_seen_install = install;

		if (install.empty()) { return; }

		std::error_code ec;

		// the script folder of the server: <game>\main\sh1x_rawfiles
		if (dvar_text(dvars::codol_script_root).empty() && fs::is_directory(fs::u8path(install) / "main", ec))
		{
			const fs::path root = default_script_root();
			fs::create_directories(root, ec);

			if (fs::is_directory(root, ec))
			{
				dvars::set_string(dvars::codol_script_root, editor::to_utf8(root).c_str());
				editor::log("[scripts] script folder: " + editor::to_utf8(root) + " ( File > Settings, codol_script_root )");
			}
		}

		// the reference copy of the packed scripts
		const fs::path archive = archive_path();
		if (!fs::is_regular_file(archive, ec)) { return; }

		const std::string stamp = stamp_of(archive);
		std::string old;
		read_file(reference_root() / STAMP_NAME, old);

		if (old != stamp)
		{
			std::string error;
			const int n = extract_all({}, &error);

			if (n < 0) { editor::log("^1[scripts] " + error); }
			else { editor::log("[scripts] " + std::to_string(n) + " scripts of " + editor::to_utf8(archive) + " unpacked to " + editor::to_utf8(reference_root()) + " ( reference, read only )"); }
		}
	}

	bool read(std::string_view game_path, std::string& out, std::string* from)
	{
		const fs::path rel = fs::u8path(lower_slash(game_path));

		for (const fs::path& root : { script_root(), fs::u8path(dvar_text(dvars::codol_script_ref_root)) })
		{
			if (!root.empty() && read_file(root / rel, out))
			{
				if (from) { *from = editor::to_utf8(root / rel); }
				return true;
			}
		}

		if (read_packed(game_path, out))
		{
			if (from) { *from = editor::to_utf8(archive_path()) + ":scripts/" + lower_slash(game_path); }
			return true;
		}

		return false;
	}

	bool packed(std::string_view game_path)
	{
		auto& c = get_cache();
		std::lock_guard guard(c.lock);
		return open_archive(c) && c.by_path.contains(lower_slash(game_path));
	}

	extract_result extract_for_edit(std::string_view game_path, fs::path* file, std::string* error)
	{
		const fs::path root = script_root();
		if (root.empty())
		{
			if (error) { *error = "no script folder: set the CoDOL install folder ( File > Settings )"; }
			return extract_result::failed;
		}

		const fs::path target = root / fs::u8path(lower_slash(game_path)).make_preferred();
		if (file) { *file = target; }

		std::error_code ec;
		if (fs::exists(target, ec)) { return extract_result::exists; }

		std::string text;
		if (!read_packed(game_path, text)) { return extract_result::not_packed; }

		return write_new_file(target, text, error) ? extract_result::extracted : extract_result::failed;
	}

	int extract_all(const fs::path& dest_in, std::string* error)
	{
		const bool reference = dest_in.empty();
		const fs::path dest = reference ? reference_root() : dest_in;

		// every loose file of the script folder overrides the packed one: a full copy there would freeze them all
		if (same_folder(dest, script_root()) || same_folder(dest, default_script_root()))
		{
			if (error) { *error = "not into the script folder: every file there overrides the packed script. Pick another folder."; }
			return -1;
		}

		auto& c = get_cache();
		std::lock_guard guard(c.lock);

		const auto* archive = open_archive(c);
		if (!archive)
		{
			if (error) { *error = "no " + std::string(ARCHIVE_NAME) + " in <CoDOL install>\\main"; }
			return -1;
		}

		std::error_code ec;

		// the reference copy belongs to the editor: it is made again from scratch
		if (reference) { fs::remove_all(dest, ec); }
		fs::create_directories(dest, ec);

		int count = 0;
		std::vector<std::uint8_t> data;

		for (const auto& [path, entry] : c.by_path)
		{
			std::string read_error;
			if (!archive->read(*entry, data, &read_error))
			{
				editor::log("^3[scripts] " + path + ": " + read_error);
				continue;
			}

			const fs::path target = dest / fs::u8path(path).make_preferred();
			fs::create_directories(target.parent_path(), ec);

			std::ofstream out(target, std::ios::binary | std::ios::trunc);
			if (!out.is_open())
			{
				if (error) { *error = "could not write " + editor::to_utf8(target); }
				return -1;
			}

			out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
			count++;
		}

		if (reference)
		{
			write_new_file(dest / STAMP_NAME, stamp_of(c.path), nullptr);
			std::ofstream note(dest / "READ_ONLY.txt", std::ios::binary | std::ios::trunc);
			note << "A copy of the scripts in " << editor::to_utf8(c.path) << ", made by CoDOL Radiant to read and search them.\r\n"
				"Changes here do nothing in the game and are lost when the archive changes. To change a script, save it from the\r\n"
				"Script Workbench ( it goes to the script folder, main\\sh1x_rawfiles, which the server lays over the archive )\r\n"
				"or run scripts_unpack <maps/mp/<map>.gsc> on the console.\r\n";
		}

		return count;
	}
}
