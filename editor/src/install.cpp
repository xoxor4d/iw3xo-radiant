#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "codol/packed_scripts.hpp"
#include "ui.hpp"
#include "codol/path_picker.hpp"

#include <codol_zone/zone_editor.hpp>
#include <codol_zone/assets.hpp>
#include <codol_assets/image.hpp>
#include <codol_render/iwd.hpp>

#include <set>

// Install / uninstall: the package of File > Build map ( compiled\<map>\ ) goes where the game reads it, and
// comes out again.
//
//   zone\*.ff                    -> <game>\usermaps\<map>\             ( or a zone folder given by hand )
//   main\zz_*_sounds.iwd         -> <game>\main\iw_sound_NN.iwd        ( written again under a name the game opens )
//   scripts\..                   -> the server's script folder
//   presence\ ( the row, the arena entry, the name )
//                                -> the create-room list of <game>: only when asked for
//
// Rules. A file that is there and differs is kept once as <file>.pre-<map> before it is replaced; a file that is
// the same already is left alone, so that installing twice changes nothing. Everything written is noted in
// compiled\<map>\work\installed.txt, and uninstall undoes exactly that: files that were new are deleted, files
// that were replaced get their backup back, the entries come out of the list.
//
// The create-room list ( README of the install's usermaps\build_usermaps_patch.py ): the map pool is the string
// table mp/comaplist.csv, the lobby environment the rawfile mp/basemaps.arena, the name a localize entry. The
// game takes the first asset of a name it links, and the zones of client_shipRetail\patch\ are linked first: so
// the list is whichever zone of that folder holds mp/comaplist.csv. That zone is read with codol_zone, the row,
// the arena entry and the name are added and it is written again; without such a zone usermaps_patch.ff is made
// from the stock table and arena list of client_shipRetail\chinese\( patch_ )code_post_gfx_mp.ff.

namespace editor::ui
{
	namespace fs = std::filesystem;
	namespace cz = codol::zone;

	namespace
	{
		constexpr const char* MAPLIST = "mp/comaplist.csv";
		constexpr const char* ARENA = "mp/basemaps.arena";
		constexpr int COLS = 31, C_NUMBER = 0, C_NAME = 1, C_ID = 4, C_TYPE = 5, C_SEQUENCE = 30;
		constexpr int ARENA_MAX = 128;		// what the client reads

		std::string lower(std::string text)
		{
			for (char& c : text)
			{
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}

			return text;
		}

		bool read_text(const fs::path& file, std::string& out)
		{
			std::ifstream in(file, std::ios::binary);
			if (!in)
			{
				return false;
			}

			out.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

			if (out.rfind("\xEF\xBB\xBF", 0) == 0)
			{
				out.erase(0, 3);
			}

			return true;
		}

		bool same_file(const fs::path& a, const fs::path& b)
		{
			std::error_code ec;
			if (fs::file_size(a, ec) != fs::file_size(b, ec) || ec)
			{
				return false;
			}

			std::string x, y;
			std::ifstream fa(a, std::ios::binary), fb(b, std::ios::binary);
			x.assign((std::istreambuf_iterator<char>(fa)), std::istreambuf_iterator<char>());
			y.assign((std::istreambuf_iterator<char>(fb)), std::istreambuf_iterator<char>());
			return x == y;
		}

		void say(const char* color, const std::string& text)
		{
			game::printf_to_console("%s[CoDOL] install: %s\n", color, text.c_str());
		}

		// ---- what an install wrote -----------------------------------------------------------------------------
		struct record
		{
			std::string kind;		// new | replaced | list | listnew
			std::string path;
			std::string extra;		// replaced, list: the backup; listnew: the number of stock rows
		};

		struct manifest
		{
			fs::path file;
			std::vector<record> records;

			record* find(const std::string& path)
			{
				for (auto& r : records)
				{
					if (lower(r.path) == lower(path))
					{
						return &r;
					}
				}

				return nullptr;
			}

			void load(const fs::path& from)
			{
				file = from;
				records.clear();
				std::string text;

				if (!read_text(from, text))
				{
					return;
				}

				std::istringstream lines(text);
				std::string line;

				while (std::getline(lines, line))
				{
					if (!line.empty() && line.back() == '\r') { line.pop_back(); }

					const auto a = line.find('\t');
					if (a == std::string::npos) { continue; }
					const auto b = line.find('\t', a + 1);
					record r;
					r.kind = line.substr(0, a);
					r.path = line.substr(a + 1, b == std::string::npos ? std::string::npos : b - a - 1);
					r.extra = b == std::string::npos ? std::string() : line.substr(b + 1);
					records.push_back(std::move(r));
				}
			}

			bool save() const
			{
				std::error_code ec;

				if (records.empty())
				{
					fs::remove(file, ec);
					return true;
				}

				fs::create_directories(file.parent_path(), ec);
				std::ofstream out(file, std::ios::binary | std::ios::trunc);

				for (const auto& r : records)
				{
					out << r.kind << '\t' << r.path << '\t' << r.extra << "\r\n";
				}

				return static_cast<bool>(out);
			}
		};

		struct counts
		{
			int written = 0, unchanged = 0, failed = 0;
		};

		// a file of the package to its place; the first time a different file is in the way it is kept as .pre-<map>
		void put(const fs::path& from, const fs::path& to, const std::string& name, manifest& m, counts& n)
		{
			std::error_code ec;
			const std::string target = to_utf8(to);
			record* known = m.find(target);

			if (fs::is_regular_file(to, ec))
			{
				if (same_file(from, to))
				{
					n.unchanged++;
					say("", target + "  ( unchanged )");

					if (!known)
					{
						// the same file was there before this install knew of it: it stays when the map is uninstalled
						m.records.push_back({ "kept", target, {} });
					}

					return;
				}

				if (!known || known->kind == "kept")
				{
					const fs::path backup = fs::u8path(target + ".pre-" + name);

					if (!fs::exists(backup, ec))
					{
						fs::copy_file(to, backup, ec);

						if (ec)
						{
							n.failed++;
							say("^1", "no backup of " + target + ": " + ec.message() + " - not replaced");
							return;
						}
					}

					say("", "backup " + to_utf8(backup));

					if (known) { *known = { "replaced", target, to_utf8(backup) }; }
					else { m.records.push_back({ "replaced", target, to_utf8(backup) }); }
				}
			}
			else if (!known)
			{
				m.records.push_back({ "new", target, {} });
			}

			fs::create_directories(to.parent_path(), ec);
			fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);

			if (ec)
			{
				n.failed++;
				say("^1", target + ": " + ec.message());
			}
			else
			{
				n.written++;
				say("", target);
			}
		}

		// ---- the create-room list ------------------------------------------------------------------------------
		cz::zaddr find_asset_ci(const cz::Zone& zone, const std::int32_t type, const std::string& name)
		{
			for (const auto& entry : zone.assets())
			{
				if (entry.type == type && lower(std::string(zone.asset_name(type, entry.header))) == name)
				{
					return entry.header;
				}
			}

			return 0;
		}

		struct arena_block
		{
			std::size_t begin = 0, end = 0;		// of "{ .. }" in the text
			std::string map, longname;
		};

		std::string quoted_after(const std::string& block, const char* key)
		{
			const std::string low = lower(block);
			std::size_t at = 0;

			while ((at = low.find(key, at)) != std::string::npos)
			{
				const bool starts = at == 0 || std::isspace(static_cast<unsigned char>(low[at - 1])) || low[at - 1] == '{';
				std::size_t next = at + std::strlen(key);

				if (starts && next < low.size() && std::isspace(static_cast<unsigned char>(low[next])))
				{
					while (next < low.size() && std::isspace(static_cast<unsigned char>(low[next]))) { next++; }

					if (next < low.size() && low[next] == '"')
					{
						const auto close = block.find('"', next + 1);
						return close == std::string::npos ? std::string() : block.substr(next + 1, close - next - 1);
					}
				}

				at += std::strlen(key);
			}

			return {};
		}

		std::vector<arena_block> arena_blocks(const std::string& text)
		{
			std::vector<arena_block> out;
			std::size_t at = 0;

			while ((at = text.find('{', at)) != std::string::npos)
			{
				const auto close = text.find('}', at);
				if (close == std::string::npos) { break; }

				arena_block b;
				b.begin = at;
				b.end = close + 1;
				const std::string body = text.substr(at, close - at);
				b.map = lower(quoted_after(body, "map"));
				b.longname = quoted_after(body, "longname");
				out.push_back(std::move(b));
				at = close + 1;
			}

			return out;
		}

		std::vector<std::string> split_row(const std::string& line)
		{
			std::vector<std::string> cells;
			std::string cell;

			for (const char c : line)
			{
				if (c == ',') { cells.push_back(cell); cell.clear(); }
				else if (c != '\r' && c != '\n') { cell.push_back(c); }
			}

			cells.push_back(cell);
			return cells;
		}

		// text of presence\localize.txt -> what the game stores ( GBK, the game's text encoding )
		std::string to_game_text(const std::string& utf8)
		{
			const int wide = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
			std::wstring w(static_cast<std::size_t>(std::max(wide, 0)), L'\0');
			MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), w.data(), wide);
			const int bytes = WideCharToMultiByte(936, 0, w.c_str(), wide, nullptr, 0, nullptr, nullptr);
			std::string out(static_cast<std::size_t>(std::max(bytes, 0)), '\0');
			WideCharToMultiByte(936, 0, w.c_str(), wide, out.data(), bytes, nullptr, nullptr);
			return out.empty() ? utf8 : out;
		}

		bool load_key(const fs::path& game, cz::ClientKey& key, std::string& error)
		{
			std::error_code ec;

			for (const fs::path& exe : { game / "codoMP_client_shipRetail.exe", fs::u8path(components::codol_game::install_path()) / "codoMP_client_shipRetail.exe" })
			{
				if (fs::is_regular_file(exe, ec) && key.load_from_exe(exe))
				{
					return true;
				}
			}

			error = "no client exe with the fastfile key: neither in " + to_utf8(game) + " nor in the install of File > Settings";
			return false;
		}

		// Adds ( remove: takes out ) the map's row, arena entry and name. True when the list is as wanted afterwards;
		// `changed` says whether a zone was written.
		bool edit_list(const fs::path& game, const std::string& name, const fs::path& package, const bool remove, manifest& m, bool& changed, std::string& error)
		{
			changed = false;
			std::error_code ec;
			cz::ClientKey key;

			if (!load_key(game, key, error))
			{
				return false;
			}

			// ---- what goes in
			std::string row_text, arena_entry, localize_text;
			const std::string list_key = "USERMAPS_" + [&]
			{
				std::string up;
				for (const char c : name) { up.push_back(std::isalnum(static_cast<unsigned char>(c)) ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : '_'); }
				return up;
			}();

			std::vector<std::string> row;
			std::string display = name;

			if (!remove)
			{
				if (!read_text(package / "presence" / "comaplist_row.csv", row_text) || !read_text(package / "presence" / "basemaps_arena_entry.txt", arena_entry))
				{
					error = "compiled\\" + name + "\\presence\\ has no comaplist_row.csv / basemaps_arena_entry.txt: File > Build map first";
					return false;
				}

				row = split_row(row_text.substr(0, row_text.find('\n')));

				if (row.size() != static_cast<std::size_t>(COLS) || lower(row[C_ID]) != lower(name))
				{
					error = "presence\\comaplist_row.csv is not a row of " + std::to_string(COLS) + " cells for " + name;
					return false;
				}

				if (read_text(package / "presence" / "localize.txt", localize_text))
				{
					const auto tab = localize_text.find('\t');
					if (tab != std::string::npos)
					{
						display = localize_text.substr(tab + 1);
						while (!display.empty() && (display.back() == '\r' || display.back() == '\n')) { display.pop_back(); }
					}
				}
			}

			// ---- the zone that is the list
			const fs::path patch = game / "client_shipRetail" / "patch";
			std::vector<fs::path> candidates;

			for (const auto& entry : fs::directory_iterator(patch, ec))
			{
				if (entry.is_regular_file() && lower(to_utf8(entry.path().extension())) == ".ff")
				{
					candidates.push_back(entry.path());
				}
			}

			std::sort(candidates.begin(), candidates.end());
			fs::path list_file;
			int holders = 0;

			for (const auto& file : candidates)
			{
				cz::Zone zone;
				if (zone.open(file, key) && find_asset_ci(zone, codol::c1::ASSET_TYPE_STRINGTABLE, MAPLIST))
				{
					if (list_file.empty()) { list_file = file; }
					holders++;
				}
			}

			if (holders > 1)
			{
				say("^3", "more than one zone of " + to_utf8(patch) + " holds " + MAPLIST + ": the game uses the one it links first; " + to_utf8(list_file.filename()) + " is edited");
			}

			cz::ZoneEditor editor;
			cz::StringTableData table;
			std::string arena;
			bool created = false;
			int stock_rows = 0;

			const auto take_table = [&](const cz::Zone& zone) -> bool
			{
				const auto header = find_asset_ci(zone, codol::c1::ASSET_TYPE_STRINGTABLE, MAPLIST);
				cz::StringTableView view;

				if (!header || !cz::read_stringtable(zone, header, view) || view.raw.columnCount != COLS)
				{
					return false;
				}

				table.columns = view.raw.columnCount;
				table.rows = view.raw.rowCount;
				table.cells.clear();
				for (const auto cell : view.cells) { table.cells.emplace_back(cell); }
				return table.cells.size() == static_cast<std::size_t>(table.rows) * static_cast<std::size_t>(table.columns);
			};

			const auto take_arena = [&](const cz::Zone& zone) -> bool
			{
				const auto header = find_asset_ci(zone, codol::c1::ASSET_TYPE_RAWFILE, ARENA);
				cz::RawFileView view;
				std::vector<std::uint8_t> bytes;

				if (!header || !cz::read_rawfile(zone, header, view) || !cz::rawfile_contents(view, bytes))
				{
					return false;
				}

				arena.assign(bytes.begin(), bytes.end());
				return true;
			};

			const auto stock = [&](const std::function<bool(const cz::Zone&)>& take) -> bool
			{
				for (const char* zone_name : { "patch_code_post_gfx_mp", "code_post_gfx_mp" })
				{
					cz::Zone zone;
					if (zone.open(game / "client_shipRetail" / "chinese" / (std::string(zone_name) + ".ff"), key) && take(zone))
					{
						return true;
					}
				}

				return false;
			};

			if (!list_file.empty())
			{
				if (const auto status = editor.open(list_file, key); !status)
				{
					error = to_utf8(list_file.filename()) + ": " + status.message;
					return false;
				}

				if (!take_table(editor.zone()))
				{
					error = to_utf8(list_file.filename()) + ": " + MAPLIST + " does not have the " + std::to_string(COLS) + " columns of 1.1.1.13";
					return false;
				}

				// the arena list of the same zone, else the stock one: the game reads one rawfile of that name
				if (!take_arena(editor.zone()) && !stock(take_arena))
				{
					error = std::string(ARENA) + " is neither in " + to_utf8(list_file.filename()) + " nor in client_shipRetail\\chinese\\( patch_ )code_post_gfx_mp.ff of " + to_utf8(game);
					return false;
				}
			}
			else
			{
				if (remove)
				{
					say("", "no zone of " + to_utf8(patch) + " holds the create-room list: nothing to take out");
					return true;
				}

				if (!stock(take_table) || !stock(take_arena))
				{
					error = "the create-room list was not found: no zone of " + to_utf8(patch) + " holds " + MAPLIST + ", and the stock list ( " + MAPLIST + ", " + ARENA
						+ " ) could not be read from client_shipRetail\\chinese\\patch_code_post_gfx_mp.ff or code_post_gfx_mp.ff of " + to_utf8(game);
					return false;
				}

				if (const auto status = editor.create("usermaps_patch"); !status)
				{
					error = status.message;
					return false;
				}

				list_file = patch / "usermaps_patch.ff";
				created = true;
				stock_rows = table.rows;
			}

			// ---- the row
			const auto cell = [&](const int r, const int c) -> std::string& { return table.cells[static_cast<std::size_t>(r) * COLS + static_cast<std::size_t>(c)]; };
			int ours = -1, foreign = -1, number = 0, sequence = 0;

			for (int r = 0; r < table.rows; r++)
			{
				if (lower(cell(r, C_ID)) == lower(name))
				{
					(cell(r, C_NAME) == list_key ? ours : foreign) = r;
				}

				const std::string& n = cell(r, C_NUMBER);
				const std::string& s = cell(r, C_SEQUENCE);
				if (!n.empty() && std::all_of(n.begin(), n.end(), ::isdigit)) { number = std::max(number, std::atoi(n.c_str())); }
				if (cell(r, C_TYPE) == "pvp" && !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit)) { sequence = std::max(sequence, std::atoi(s.c_str())); }
			}

			bool table_changed = false;

			if (remove)
			{
				if (ours >= 0)
				{
					table.cells.erase(table.cells.begin() + static_cast<std::ptrdiff_t>(ours) * COLS, table.cells.begin() + static_cast<std::ptrdiff_t>(ours + 1) * COLS);
					table.rows--;
					table_changed = true;
				}
			}
			else if (ours >= 0)
			{
				// there already: the cells follow the package, number and place in the list stay
				for (int c = 0; c < COLS; c++)
				{
					if (c != C_NUMBER && c != C_SEQUENCE && cell(ours, c) != row[static_cast<std::size_t>(c)])
					{
						cell(ours, c) = row[static_cast<std::size_t>(c)];
						table_changed = true;
					}
				}
			}
			else if (foreign >= 0)
			{
				say("^3", std::string(MAPLIST) + " already has a row for " + name + " that this editor did not write ( " + cell(foreign, C_NAME) + " ): it is left as it is");
			}
			else
			{
				row[C_NUMBER] = std::to_string(number + 1);
				row[C_SEQUENCE] = std::to_string(sequence + 1);
				table.cells.insert(table.cells.end(), row.begin(), row.end());
				table.rows++;
				table_changed = true;
			}

			// ---- the arena entry
			bool arena_changed = false;
			{
				const auto blocks = arena_blocks(arena);
				const arena_block* have = nullptr;

				for (const auto& b : blocks)
				{
					if (b.map == lower(name)) { have = &b; }
				}

				if (remove)
				{
					if (have && have->longname == list_key)
					{
						std::size_t end = have->end;
						while (end < arena.size() && (arena[end] == '\r' || arena[end] == '\n')) { end++; }
						arena.erase(have->begin, end - have->begin);
						arena_changed = true;
					}
				}
				else if (!have)
				{
					if (static_cast<int>(blocks.size()) + 1 > ARENA_MAX)
					{
						error = std::string(ARENA) + " would hold " + std::to_string(blocks.size() + 1) + " entries; the client reads " + std::to_string(ARENA_MAX);
						return false;
					}

					const std::string nl = arena.find("\r\n") != std::string::npos ? "\r\n" : "\n";
					std::string entry;

					for (const char c : arena_entry)
					{
						if (c == '\r') { continue; }
						entry += c == '\n' ? nl : std::string(1, c);
					}

					while (!entry.empty() && (entry.back() == '\r' || entry.back() == '\n')) { entry.pop_back(); }

					if (!arena.empty() && arena.back() != '\n') { arena += nl; }
					arena += entry + nl;
					arena_changed = true;
				}
			}

			// ---- the name: it belongs to a row of ours
			const std::size_t name_index = created ? cz::ZoneEditor::npos : editor.find_asset(codol::c1::ASSET_TYPE_LOCALIZE_ENTRY, list_key);
			const bool name_present = name_index != cz::ZoneEditor::npos;
			const bool add_name = !remove && !name_present && (ours >= 0 || table_changed);
			const bool drop_name = remove && name_present;

			if (!table_changed && !arena_changed && !add_name && !drop_name)
			{
				say("", "create-room list ( " + to_utf8(created ? fs::path("the stock list") : list_file.filename()) + " ): " + (remove ? "has nothing of " + name : "already has " + name)
					+ "  ( unchanged )");
				return true;
			}

			if (drop_name)
			{
				if (const auto status = editor.remove_asset(name_index); !status)
				{
					error = "the name " + list_key + " cannot be taken out: " + status.message;
					return false;
				}
			}

			if (table_changed || created)
			{
				if (const auto status = editor.set_string_table(MAPLIST, table); !status)
				{
					error = std::string(MAPLIST) + ": " + status.message;
					return false;
				}
			}

			if (arena_changed || created)
			{
				const std::span<const std::uint8_t> bytes(reinterpret_cast<const std::uint8_t*>(arena.data()), arena.size());

				if (const auto status = editor.set_rawfile(ARENA, bytes, true); !status)
				{
					error = std::string(ARENA) + ": " + status.message;
					return false;
				}
			}

			if (add_name)
			{
				// LocalizeEntry { value, name }: both follow in the stream
				const std::string value = to_game_text(display);
				std::vector<cz::ZoneNewPiece> pieces(3);
				pieces[0].bytes.assign(8, 0xFF);
				pieces[1].bytes.assign(value.begin(), value.end());
				pieces[1].bytes.push_back(0);
				pieces[1].string = true;
				pieces[2].bytes.assign(list_key.begin(), list_key.end());
				pieces[2].bytes.push_back(0);
				pieces[2].string = true;

				if (const auto status = editor.add_new_asset(codol::c1::ASSET_TYPE_LOCALIZE_ENTRY, list_key, pieces); !status)
				{
					error = "the name " + list_key + ": " + status.message;
					return false;
				}
			}

			// ---- write, read back, put in place
			std::vector<std::uint8_t> bytes;

			if (const auto status = editor.write(bytes); !status)
			{
				error = to_utf8(list_file.filename()) + " cannot be written again: " + status.message;
				return false;
			}

			const fs::path tmp = fs::u8path(to_utf8(list_file) + ".codol_tmp");

			if (const auto status = cz::write_fastfile(tmp, bytes, key); !status)
			{
				error = to_utf8(tmp) + ": " + status.message;
				return false;
			}

			{
				cz::Zone check;
				cz::StringTableView view;
				const auto status = check.open(tmp, key);
				const auto header = status ? find_asset_ci(check, codol::c1::ASSET_TYPE_STRINGTABLE, MAPLIST) : 0;
				bool ok = header && cz::read_stringtable(check, header, view) && view.raw.rowCount == table.rows && view.raw.columnCount == COLS;

				for (std::size_t i = 0; ok && i < table.cells.size(); i++)
				{
					ok = view.cells[i] == table.cells[i];
				}

				cz::LocalizeView text;
				const auto loc = status ? find_asset_ci(check, codol::c1::ASSET_TYPE_LOCALIZE_ENTRY, lower(list_key)) : 0;
				ok = ok && (remove ? loc == 0 : (!(add_name || name_present) || (loc != 0 && cz::read_localize(check, loc, text))));

				if (!ok)
				{
					fs::remove(tmp, ec);
					error = "the written list does not read back as it was made ( " + (status ? std::string("its table or name differs") : status.message) + " ): nothing was changed";
					return false;
				}
			}

			const std::string target = to_utf8(list_file);
			record* known = m.find(target);

			if (fs::is_regular_file(list_file, ec))
			{
				const fs::path backup = fs::u8path(target + ".pre-" + name);

				if (!remove && !fs::exists(backup, ec))
				{
					fs::copy_file(list_file, backup, ec);

					if (ec)
					{
						fs::remove(tmp, ec);
						error = "no backup of " + target + ": " + ec.message() + " - the list was not changed";
						return false;
					}

					say("", "backup " + to_utf8(backup));
				}

				if (!known && !remove)
				{
					m.records.push_back({ "list", target, to_utf8(backup) });
				}
			}
			else if (!known && !remove)
			{
				m.records.push_back({ "listnew", target, std::to_string(stock_rows) });
			}

			// a list this editor made that holds the stock rows again is not needed any more
			if (remove && known && known->kind == "listnew" && table.rows == std::atoi(known->extra.c_str()))
			{
				fs::remove(tmp, ec);
				fs::remove(list_file, ec);
				say("", "removed " + target + " ( it held nothing but the stock list )");
				changed = true;
				return true;
			}

			if (!MoveFileExW(tmp.c_str(), list_file.c_str(), MOVEFILE_REPLACE_EXISTING))
			{
				fs::remove(tmp, ec);
				error = target + " could not be replaced ( is the game running? )";
				return false;
			}

			changed = true;
			say("", "create-room list ( " + to_utf8(list_file.filename()) + " ): " + (remove ? name + " taken out" : name + " - row " + std::to_string(ours >= 0 ? ours + 1 : table.rows)
				+ " of " + MAPLIST + ", an entry of " + ARENA + ", the name " + list_key + " = \"" + display + "\"") + ( created ? "  ( the zone is new: made from the stock list )" : "" ));
			return true;
		}

		// two fastfiles hold the same assets ( whatever their bytes are: a zone written again differs in its container )
		bool same_zone(const fs::path& a, const fs::path& b, const cz::ClientKey& key)
		{
			cz::ZoneEditor x, y;

			if (!x.open(a, key) || !y.open(b, key) || x.asset_count() != y.asset_count())
			{
				return false;
			}

			for (std::size_t i = 0; i < x.asset_count(); i++)
			{
				const auto p = x.asset(i), q = y.asset(i);

				if (p.type != q.type || p.name != q.name)
				{
					return false;
				}

				// the two assets an install writes again are compared by what they say, the others by what they are
				if (p.type == codol::c1::ASSET_TYPE_STRINGTABLE && lower(p.name) == MAPLIST)
				{
					cz::StringTableView u, v;
					if (!cz::read_stringtable(x.zone(), find_asset_ci(x.zone(), p.type, MAPLIST), u) || !cz::read_stringtable(y.zone(), find_asset_ci(y.zone(), q.type, MAPLIST), v)
						|| u.raw.columnCount != v.raw.columnCount || u.cells != v.cells)
					{
						return false;
					}
				}
				else if (p.type == codol::c1::ASSET_TYPE_RAWFILE && lower(p.name) == ARENA)
				{
					cz::RawFileView u, v;
					std::vector<std::uint8_t> s, t;
					if (!cz::read_rawfile(x.zone(), find_asset_ci(x.zone(), p.type, ARENA), u) || !cz::read_rawfile(y.zone(), find_asset_ci(y.zone(), q.type, ARENA), v)
						|| !cz::rawfile_contents(u, s) || !cz::rawfile_contents(v, t) || s != t)
					{
						return false;
					}
				}
				else if (x.asset_fingerprint(i) != y.asset_fingerprint(i))
				{
					return false;
				}
			}

			return true;
		}

		fs::path package_of(const std::string& name)
		{
			return fs::u8path(utils::fs::get_basepath()) / "compiled" / name;
		}

		// The streamed sound files of a package ( main\zz_*_sounds.iwd, plain archives ) as ONE archive of the game:
		// <game>\main\iw_sound_NN.iwd. The game only opens archives named like its own ( iw_sound_NN, NN up to 23 ),
		// decrypts those, and plays a streamed file only out of an archive the server has as well - the same main\
		// folder where client and server run from one install. The number is the one an earlier install of this map
		// took, else the first free one from 23 down; the name is part of the encryption, so the archive is written
		// for it ( into work\ ) and then put in place like every other file.
		void install_sound_archive(const std::vector<fs::path>& archives, const fs::path& game, const std::string& name, const fs::path& package, manifest& m, counts& n)
		{
			namespace ca = codol::assets;
			namespace cr = codol::render;
			std::error_code ec;
			std::vector<std::pair<std::string, std::vector<std::uint8_t>>> files;

			for (const auto& from : archives)
			{
				std::string why;
				const auto archive = cr::iwd_archive::open(from, nullptr, &why);

				if (!archive)
				{
					n.failed++;
					say("^1", to_utf8(from) + ": " + why);
					return;
				}

				for (const auto& entry : archive->entries())
				{
					std::vector<std::uint8_t> bytes;

					if (!archive->read(entry, bytes, &why))
					{
						n.failed++;
						say("^1", to_utf8(from) + ": " + entry.name + ": " + why);
						return;
					}

					files.emplace_back(entry.name, std::move(bytes));
				}
			}

			if (files.empty())
			{
				return;
			}

			fs::path target;

			for (const auto& r : m.records)
			{
				const fs::path known = fs::u8path(r.path);

				if ((r.kind == "new" || r.kind == "replaced") && lower(to_utf8(known.parent_path())) == lower(to_utf8(game / "main")) && lower(to_utf8(known.filename())).rfind("iw_sound_", 0) == 0)
				{
					target = known;
				}
			}

			if (target.empty())
			{
				target = ca::free_sound_iwd(game / "main");
			}

			if (target.empty())
			{
				n.failed++;
				say("^1", "streamed sounds: " + to_utf8(game / "main") + " has no free iw_sound_NN.iwd ( 04 .. 23 are taken )");
				return;
			}

			cr::iwd_key key{};
			std::string why;
			bool have_key = false;

			for (const fs::path& exe : { game / "codoMP_client_shipRetail.exe", fs::u8path(components::codol_game::install_path()) / "codoMP_client_shipRetail.exe" })
			{
				if (!have_key && fs::is_regular_file(exe, ec) && cr::derive_iwd_key(exe, key, &why))
				{
					have_key = true;
				}
			}

			if (!have_key)
			{
				n.failed++;
				say("^1", "streamed sounds: the archive key could not be read from codoMP_client_shipRetail.exe of " + to_utf8(game) + " ( " + why + " )");
				return;
			}

			const fs::path made = package / "work" / target.filename();

			if (const auto status = ca::write_iwd(made, files, &key); !status)
			{
				n.failed++;
				say("^1", "streamed sounds: " + status.message);
				return;
			}

			say("", "streamed sounds: " + std::to_string(files.size()) + " file( s ) as " + to_utf8(target.filename()) + " ( the server needs the same archive in its main\\ )");
			put(made, target, name, m, n);
		}
	}

	// Install: the package of the last build goes where the game reads it. `request.game` gives the usermaps folder,
	// main\ and - with `list` - the create-room list; `zone_dir` a zone folder by hand ( a server on another path ).
	void install_map(const install_request& request)
	{
		auto& s = get_scene();

		if (!s.open || s.map_file.empty())
		{
			ImGui::Toast(ImGuiToastType_Warning, "Install", "Open the map and build it first.");
			return;
		}

		const std::string name = to_utf8(s.map_file.stem());
		const auto package = package_of(name);
		std::error_code ec;

		if (!fs::is_regular_file(package / "zone" / (name + ".ff"), ec))
		{
			ImGui::Toast(ImGuiToastType_Warning, "Install", ("compiled\\" + name + "\\zone\\" + name + ".ff is not there: File > Build map first.").c_str(), 6000);
			return;
		}

		const fs::path game = fs::u8path(request.game);
		const fs::path zone_dir = !request.zone_dir.empty() ? fs::u8path(request.zone_dir) : request.game.empty() ? fs::path() : game / "usermaps" / name;
		// no script folder named: the game's ( <game>\main\sh1x_rawfiles, the overlay of sh1x_svr_scripts.iwd ), else the one of the settings
		std::error_code dir_ec;
		const fs::path script_dir = !request.script_dir.empty() ? fs::u8path(request.script_dir)
			: !request.game.empty() && fs::is_directory(game / "main", dir_ec) ? game / "main" / "sh1x_rawfiles"
			: codol::packed_scripts::script_root();

		if (zone_dir.empty())
		{
			ImGui::Toast(ImGuiToastType_Error, "Install", "No game folder: set the CoDOL install folder ( File > Settings ) or name the folders.", 6000);
			return;
		}

		manifest m;
		m.load(package / "work" / "installed.txt");
		counts n;

		for (const auto& entry : fs::directory_iterator(package / "zone", ec))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".ff")
			{
				put(entry.path(), zone_dir / entry.path().filename(), name, m, n);
			}
		}

		// the streamed sounds of the project: next to the game's archives, under a name the game opens. A zone
		// folder given by hand says nothing about where main\ is
		{
			std::vector<fs::path> archives;

			for (const auto& entry : fs::directory_iterator(package / "main", ec))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".iwd")
				{
					archives.push_back(entry.path());
				}
			}

			std::sort(archives.begin(), archives.end());

			if (!archives.empty() && request.game.empty())
			{
				say("^3", "compiled\\" + name + "\\main\\ ( streamed sounds ) belongs into the game's main folder: not installed, no game folder was given");
			}
			else if (!archives.empty())
			{
				install_sound_archive(archives, game, name, package, m, n);
			}
		}

		if (!script_dir.empty())
		{
			const auto from = package / "scripts";

			for (const auto& entry : fs::recursive_directory_iterator(from, ec))
			{
				if (entry.is_regular_file())
				{
					put(entry.path(), script_dir / fs::relative(entry.path(), from, ec), name, m, n);
				}
			}
		}
		else
		{
			say("^3", "no script folder ( File > Settings, codol_script_root ): the level scripts in compiled\\" + name + "\\scripts were not copied");
		}

		bool list_ok = true;

		if (request.list)
		{
			if (request.game.empty())
			{
				say("^1", "the create-room list needs the game folder");
				list_ok = false;
			}
			else
			{
				bool changed = false;
				std::string error;
				list_ok = edit_list(game, name, package, false, m, changed, error);

				if (!list_ok)
				{
					say("^1", "create-room list: " + error);
				}
				else
				{
					(changed ? n.written : n.unchanged)++;

					// The lobby emulator ( alpha0022, handoff\map_handoff.py installed_maps ) starts the map of a room
					// only when <map>.ff is in ITS zone folder - the install's client_shipRetail\chinese, or the one
					// folder CODOL_ZONE_DIR names; it never looks into usermaps\. Any other map of a room is replaced
					// by its default map. Read from its sources; said here because the list alone does not start a map.
					if (lower(to_utf8(zone_dir.lexically_normal())) != lower(to_utf8((game / "client_shipRetail" / "chinese").lexically_normal())))
					{
						say("^3", "the lobby only starts a room's map whose zone is in its zone folder ( <game>\\client_shipRetail\\chinese, or CODOL_ZONE_DIR ): " + name
							+ ".ff went to " + to_utf8(zone_dir) + " - install with zones=<game>\\client_shipRetail\\chinese, or the lobby plays its default map instead");
					}
				}
			}
		}
		else
		{
			say("", "to play, start the lobby with CODOL_FORCE_MAP=" + name + "; the create-room list was not touched ( File > Install, \"Add to the create-room list\", or map_install .. list )");
		}

		if (!m.save())
		{
			say("^3", "compiled\\" + name + "\\work\\installed.txt could not be written: uninstall will not know these files");
		}

		const std::string summary = std::to_string(n.written) + " written, " + std::to_string(n.unchanged) + " unchanged" + (n.failed || !list_ok ? ", " + std::to_string(n.failed + (list_ok ? 0 : 1)) + " failed" : "");
		say(n.failed || !list_ok ? "^1" : "", name + ": " + summary);

		if (n.failed || !list_ok)
		{
			ImGui::Toast(ImGuiToastType_Error, "Install", (summary + " - see the console.").c_str(), 8000);
		}
		else
		{
			ImGui::Toast(ImGuiToastType_Success, "Install", ("Installed " + name + ": " + summary + ".").c_str(), 6000);
		}
	}

	// Uninstall: everything compiled\<map>\work\installed.txt names goes back to what it was.
	void uninstall_map(const std::string& map)
	{
		auto& s = get_scene();
		const std::string name = !map.empty() ? map : s.open && !s.map_file.empty() ? to_utf8(s.map_file.stem()) : std::string();

		if (name.empty())
		{
			ImGui::Toast(ImGuiToastType_Warning, "Uninstall", "Open the map, or name it: map_uninstall <map>.");
			return;
		}

		const auto package = package_of(name);
		manifest m;
		m.load(package / "work" / "installed.txt");

		if (m.records.empty())
		{
			say("", name + ": compiled\\" + name + "\\work\\installed.txt names nothing - the map was not installed from here");
			ImGui::Toast(ImGuiToastType_Info, "Uninstall", (name + " was not installed from here.").c_str(), 5000);
			return;
		}

		std::error_code ec;
		int undone = 0, failed = 0;
		std::vector<record> left;
		std::set<fs::path> folders;

		for (const auto& r : m.records)
		{
			const fs::path file = fs::u8path(r.path);

			if (r.kind == "new")
			{
				if (!fs::exists(file, ec) || fs::remove(file, ec))
				{
					undone++;
					folders.insert(file.parent_path());
					say("", "removed " + r.path);
				}
				else
				{
					failed++;
					left.push_back(r);
					say("^1", r.path + " could not be removed: " + ec.message());
				}
			}
			else if (r.kind == "replaced")
			{
				const fs::path backup = fs::u8path(r.extra);

				if (fs::is_regular_file(backup, ec) && MoveFileExW(backup.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING))
				{
					undone++;
					say("", "restored " + r.path);
				}
				else
				{
					failed++;
					left.push_back(r);
					say("^1", r.path + ": its backup " + r.extra + " could not be put back");
				}
			}
			else if (r.kind == "list" || r.kind == "listnew")
			{
				// .. client_shipRetail\patch\<zone>.ff: the game folder is two above
				const fs::path game = file.parent_path().parent_path().parent_path();
				bool changed = false;
				std::string error;
				manifest scratch;
				scratch.records.push_back(r);

				if (edit_list(game, name, package, true, scratch, changed, error))
				{
					undone++;

					// The backup is the list before this map; other maps may have come since, so the entries were taken
					// out of the list as it is now. When that leaves exactly what the backup holds, the backup itself
					// goes back - the file is then byte for byte what it was.
					if (r.kind == "list" && !r.extra.empty() && fs::is_regular_file(fs::u8path(r.extra), ec))
					{
						cz::ClientKey key;
						std::string key_error;

						if (load_key(game, key, key_error) && same_zone(fs::u8path(r.extra), file, key) && MoveFileExW(fs::u8path(r.extra).c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING))
						{
							say("", "restored " + r.path + " ( exactly as it was )");
						}
						else
						{
							say("", "the list as it was before " + name + " stays as " + r.extra + " ( the list has changed since )");
						}
					}
				}
				else
				{
					failed++;
					left.push_back(r);
					say("^1", "create-room list: " + error);
				}
			}
			// "kept": a file that was there and the same before the install - it stays
		}

		// folders this left empty ( usermaps\<map> ): only when empty, never with what is in them
		for (const auto& folder : folders)
		{
			if (fs::is_directory(folder, ec) && fs::is_empty(folder, ec))
			{
				fs::remove(folder, ec);
			}
		}

		m.records = left;
		m.save();

		const std::string summary = std::to_string(undone) + " undone" + (failed ? ", " + std::to_string(failed) + " failed" : "");
		say(failed ? "^1" : "", name + " uninstalled: " + summary);
		ImGui::Toast(failed ? ImGuiToastType_Error : ImGuiToastType_Success, "Uninstall", (name + ": " + summary + ".").c_str(), 6000);
	}

	// map_list: what the create-room list of a game folder holds - which zone it is, its last rows, the arena entries
	void print_map_list(const std::string& game_folder)
	{
		const fs::path game = fs::u8path(game_folder.empty() ? components::codol_game::install_path() : game_folder);
		cz::ClientKey key;
		std::string error;
		std::error_code ec;

		if (!load_key(game, key, error))
		{
			say("^1", error);
			return;
		}

		std::vector<fs::path> files;
		for (const auto& entry : fs::directory_iterator(game / "client_shipRetail" / "patch", ec))
		{
			if (entry.is_regular_file() && lower(to_utf8(entry.path().extension())) == ".ff") { files.push_back(entry.path()); }
		}

		std::sort(files.begin(), files.end());
		bool any = false;

		for (const auto& file : files)
		{
			cz::Zone zone;
			cz::StringTableView view;
			const auto header = zone.open(file, key) ? find_asset_ci(zone, codol::c1::ASSET_TYPE_STRINGTABLE, MAPLIST) : 0;

			if (!header || !cz::read_stringtable(zone, header, view))
			{
				continue;
			}

			any = true;
			int names = 0;
			for (const auto& entry : zone.assets())
			{
				names += entry.type == codol::c1::ASSET_TYPE_LOCALIZE_ENTRY && std::string(zone.asset_name(entry.type, entry.header)).rfind("USERMAPS_", 0) == 0;
			}

			std::string arena_text;
			cz::RawFileView raw;
			std::vector<std::uint8_t> bytes;
			if (const auto arena = find_asset_ci(zone, codol::c1::ASSET_TYPE_RAWFILE, ARENA); arena && cz::read_rawfile(zone, arena, raw) && cz::rawfile_contents(raw, bytes))
			{
				arena_text.assign(bytes.begin(), bytes.end());
			}

			game::printf_to_console("[CoDOL] create-room list: %s - %s with %d rows of %d columns, %d arena entries, %d USERMAPS_ names\n", to_utf8(file).c_str(), MAPLIST,
				view.raw.rowCount, view.raw.columnCount, static_cast<int>(arena_blocks(arena_text).size()), names);

			for (int r = std::max(0, view.raw.rowCount - 4); r < view.raw.rowCount; r++)
			{
				game::printf_to_console("  row %d: number %s, name %s, id %s, type %s, sequence %s\n", r + 1, std::string(view.cell(r, C_NUMBER)).c_str(), std::string(view.cell(r, C_NAME)).c_str(),
					std::string(view.cell(r, C_ID)).c_str(), std::string(view.cell(r, C_TYPE)).c_str(), std::string(view.cell(r, C_SEQUENCE)).c_str());
			}
		}

		if (!any)
		{
			say("", "no zone of " + to_utf8(game / "client_shipRetail" / "patch") + " holds " + MAPLIST + ": the game shows the stock list");
		}
	}

	// the folders File > Settings gives: the old "install" with one click
	void install_map(const std::string& zone_to, const std::string& scripts_to)
	{
		install_request request;
		request.zone_dir = zone_to;
		request.script_dir = scripts_to;

		if (zone_to.empty())
		{
			request.game = components::codol_game::install_path();
		}

		if (scripts_to.empty())
		{
			request.script_dir = to_utf8(codol::packed_scripts::script_root());
		}

		install_map(request);
	}

	// ---- File > Install ..: where to, and whether the create-room list is part of it --------------------------
	namespace
	{
		struct install_window
		{
			bool open = false;
			bool appearing = false;
			std::string game, scripts;
			bool list = false;
		};

		install_window g_install;
	}

	void open_install_window()
	{
		auto& w = g_install;

		if (!w.open)
		{
			w.game = components::codol_game::install_path();
			w.scripts = to_utf8(codol::packed_scripts::script_root());
			w.list = false;
		}

		w.open = w.appearing = true;
	}

	void draw_install_window()
	{
		auto& w = g_install;
		if (!w.open)
		{
			return;
		}

		auto& s = get_scene();
		const std::string name = s.open && !s.map_file.empty() ? to_utf8(s.map_file.stem()) : std::string();

		ImGui::SetNextWindowSize(ImVec2(700.0f, 0.0f), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (w.appearing)
		{
			ImGui::SetNextWindowFocus();
			w.appearing = false;
		}

		if (ImGui::Begin("Install the built map##install", &w.open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Game folder");
			ImGui::SameLine(130.0f);
			codol::ui::path_input("##game", w.game, codol::ui::path_kind::folder, nullptr, "the Call of Duty Online 1.1.1.13 folder", 540.0f);

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Script folder");
			ImGui::SameLine(130.0f);
			codol::ui::path_input("##scripts", w.scripts, codol::ui::path_kind::folder, nullptr, "the server's script folder ( empty: <game>\\main\\sh1x_rawfiles )", 540.0f);

			ImGui::Checkbox("Add to the create-room list", &w.list);

			ImGui::PushTextWrapPos(660.0f);
			ImGui::TextDisabled("compiled\\%s\\zone\\*.ff go to <game>\\usermaps\\%s\\, streamed sounds to <game>\\main\\ as a free iw_sound_NN.iwd, scripts\\ to the script folder. "
				"The create-room list is the zone of <game>\\client_shipRetail\\patch\\ that holds mp/comaplist.csv: the map's row, its arena entry and its name are added to it. "
				"A file that is replaced is kept as <file>.pre-%s; Uninstall puts everything back. Without the list a map is played with CODOL_FORCE_MAP=%s on the lobby.",
				name.empty() ? "<map>" : name.c_str(), name.empty() ? "<map>" : name.c_str(), name.empty() ? "<map>" : name.c_str(), name.empty() ? "<map>" : name.c_str());
			ImGui::PopTextWrapPos();
			ImGui::Spacing();

			ImGui::BeginDisabled(name.empty() || w.game.empty());

			if (ImGui::Button("Install", ImVec2(150.0f, 0.0f)))
			{
				install_request request;
				request.game = w.game;
				request.script_dir = w.scripts;
				request.list = w.list;
				install_map(request);
			}

			ImGui::SameLine();

			if (ImGui::Button("Uninstall", ImVec2(150.0f, 0.0f)))
			{
				uninstall_map(name);
			}

			ImGui::EndDisabled();
			ImGui::SameLine();

			if (ImGui::Button("Close", ImVec2(110.0f, 0.0f)))
			{
				w.open = false;
			}

			if (name.empty())
			{
				ImGui::TextColored(ImVec4(0.95f, 0.80f, 0.35f, 1.0f), "Open the map and build it first.");
			}
		}

		ImGui::End();
	}
}
