#include "std_include.hpp"

// windows.h macros
#undef min
#undef max

#include "host.hpp"
#include "ui.hpp"
#include "assets.hpp"

#include "codol_assets/fx.hpp"
#include "codol_assets/link.hpp"
#include "codol_render/texture.hpp"

#include <atomic>

// The project's assets: sources on disk -> a zone of native assets, on a worker thread. See assets.hpp.

namespace editor::assets
{
	namespace ca = ::codol::assets;
	namespace fs = std::filesystem;

	namespace
	{
		// ---- small things ----------------------------------------------------------------------------------

		std::string lower(std::string text)
		{
			for (char& c : text)
			{
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}

			return text;
		}

		// an asset name: lower case letters, digits, '_' ( and '/' in material names )
		std::string clean_name(const std::string& text, const bool slash = false)
		{
			std::string out;

			for (const char c : lower(text))
			{
				const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '$' || (slash && c == '/');
				out.push_back(ok ? c : '_');
			}

			return out;
		}

		bool read_text(const fs::path& file, std::string& out)
		{
			std::ifstream in(file, std::ios::binary);
			if (!in)
			{
				return false;
			}

			out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
			return true;
		}

		bool write_text(const fs::path& file, const std::string& text)
		{
			std::error_code ec;
			fs::create_directories(file.parent_path(), ec);
			std::ofstream out(file, std::ios::binary | std::ios::trunc);
			out << text;
			return static_cast<bool>(out);
		}

		// size and time of a file: what tells that it changed
		std::string file_signature(const fs::path& file)
		{
			std::error_code ec;
			const auto size = fs::file_size(file, ec);
			if (ec)
			{
				return "-";
			}

			const auto time = fs::last_write_time(file, ec).time_since_epoch().count();
			return std::to_string(size) + ":" + std::to_string(static_cast<long long>(time));
		}

		bool is_image_file(const fs::path& file)
		{
			const std::string ext = lower(file.extension().string());
			return ext == ".png" || ext == ".tga" || ext == ".dds";
		}

		bool same_file_contents(const fs::path& a, const fs::path& b)
		{
			std::string x, y;
			return read_text(a, x) && read_text(b, y) && x == y;
		}

		// words, "quoted words", { } and // comments
		std::vector<std::string> tokenize(const std::string& text)
		{
			std::vector<std::string> out;
			std::size_t i = 0;

			while (i < text.size())
			{
				const char c = text[i];

				if (std::isspace(static_cast<unsigned char>(c)))
				{
					i++;
				}
				else if (c == '/' && i + 1 < text.size() && text[i + 1] == '/')
				{
					while (i < text.size() && text[i] != '\n') { i++; }
				}
				else if (c == '"')
				{
					const std::size_t end = text.find('"', i + 1);
					out.push_back(text.substr(i + 1, end == std::string::npos ? std::string::npos : end - i - 1));
					i = end == std::string::npos ? text.size() : end + 1;
				}
				else if (c == '{' || c == '}')
				{
					out.emplace_back(1, c);
					i++;
				}
				else
				{
					std::size_t end = i;
					while (end < text.size() && !std::isspace(static_cast<unsigned char>(text[end])) && text[end] != '{' && text[end] != '}' && text[end] != '"') { end++; }
					out.push_back(text.substr(i, end - i));
					i = end;
				}
			}

			return out;
		}

		// ---- what is on disk -------------------------------------------------------------------------------

		struct model_def
		{
			std::string name;
			std::vector<model_lod> lods;			// files relative to the models folder, or absolute
			int collision_lod = -1;
			int contents = 1;
			std::map<std::string, std::string> materials;
		};

		// distances of the LODs of a model that names none: the last one is drawn as far as shipped props are
		float default_lod_distance(const std::size_t lod, const std::size_t count)
		{
			return lod + 1 >= count ? 1000000.0f : 600.0f * static_cast<float>(1u << lod);
		}

		bool parse_model_file(const fs::path& file, model_def& out, std::string& error)
		{
			std::string text;
			if (!read_text(file, text))
			{
				error = "cannot read " + to_utf8(file);
				return false;
			}

			const auto words = tokenize(text);
			std::size_t k = 0;

			if (words.size() < 4 || words[0] != "model" || words[2] != "{")
			{
				error = "expected: model \"name\" { .. }";
				return false;
			}

			out = {};
			out.name = clean_name(words[1]);
			k = 3;

			while (k < words.size() && words[k] != "}")
			{
				const std::string& key = words[k++];

				if (key == "lod" && k < words.size())
				{
					model_lod lod;
					lod.file = fs::u8path(words[k++]);
					lod.distance = -1.0f;

					if (k < words.size() && !words[k].empty() && (std::isdigit(static_cast<unsigned char>(words[k][0])) || words[k][0] == '.'))
					{
						lod.distance = static_cast<float>(std::atof(words[k++].c_str()));
					}

					out.lods.push_back(lod);
				}
				else if (key == "collision" && k < words.size())
				{
					out.collision_lod = words[k] == "none" ? -1 : std::atoi(words[k].c_str());
					k++;
				}
				else if (key == "contents" && k < words.size())
				{
					out.contents = static_cast<int>(std::strtol(words[k++].c_str(), nullptr, 0));
				}
				else if (key == "material" && k + 1 < words.size())
				{
					out.materials[words[k]] = words[k + 1];
					k += 2;
				}
				else
				{
					error = "unknown key '" + key + "' ( lod, collision, contents, material )";
					return false;
				}
			}

			if (out.lods.empty() || out.lods.size() > 4)
			{
				error = "a model has 1 to 4 lod lines";
				return false;
			}

			for (std::size_t i = 0; i < out.lods.size(); i++)
			{
				if (out.lods[i].distance < 0.0f)
				{
					out.lods[i].distance = default_lod_distance(i, out.lods.size());
				}
			}

			return true;
		}

		std::string write_model_text(const model_def& def)
		{
			std::string out = "// a model of the project: LOD files ( nearest first ) with the distance each is drawn up to,\n// the LOD the collision is made from ( none: no collision ), material names of the export -> materials\n";
			out += "model \"" + def.name + "\"\n{\n";
			char line[64];

			for (const auto& lod : def.lods)
			{
				std::snprintf(line, sizeof(line), "%g", lod.distance);
				out += "\tlod \"" + to_utf8(lod.file) + "\" " + line + "\n";
			}

			out += "\tcollision " + (def.collision_lod < 0 ? std::string("none") : std::to_string(def.collision_lod)) + "\n";

			if (def.contents != 1)
			{
				out += "\tcontents " + std::to_string(def.contents) + "\n";
			}

			for (const auto& [from, to] : def.materials)
			{
				out += "\tmaterial \"" + from + "\" \"" + to + "\"\n";
			}

			return out + "}\n";
		}

		// an animation of the project: anims\<name>.anim, or an .xanim_export alone ( no options )
		struct anim_def
		{
			std::string name;
			fs::path file;					// the xanim_export, relative to the anims folder, or absolute
			std::string model;				// the skeleton: a model of the project, or an .xmodel_export file; empty: none
			std::string type = "relative";	// relative | delta | delta3d
			bool loop = false;
			std::string tree;				// the animtree; empty: the project's
		};

		bool parse_anim_file(const fs::path& file, anim_def& out, std::string& error)
		{
			std::string text;
			if (!read_text(file, text))
			{
				error = "cannot read " + to_utf8(file);
				return false;
			}

			const auto words = tokenize(text);

			if (words.size() < 4 || words[0] != "anim" || words[2] != "{")
			{
				error = "expected: anim \"name\" { .. }";
				return false;
			}

			out = {};
			out.name = clean_name(words[1]);
			std::size_t k = 3;

			while (k < words.size() && words[k] != "}")
			{
				const std::string& key = words[k++];

				if (key == "file" && k < words.size())
				{
					out.file = fs::u8path(words[k++]);
				}
				else if (key == "model" && k < words.size())
				{
					out.model = words[k++];
				}
				else if (key == "type" && k < words.size())
				{
					out.type = lower(words[k++]);

					if (out.type != "relative" && out.type != "delta" && out.type != "delta3d")
					{
						error = "type is relative, delta or delta3d";
						return false;
					}
				}
				else if (key == "loop")
				{
					out.loop = true;
				}
				else if (key == "tree" && k < words.size())
				{
					out.tree = clean_name(words[k++]);
				}
				else
				{
					error = "unknown key '" + key + "' ( file, model, type, loop, tree )";
					return false;
				}
			}

			if (out.file.empty())
			{
				error = "an animation needs a file line ( the .xanim_export )";
				return false;
			}

			return true;
		}

		std::string write_anim_text(const anim_def& def)
		{
			std::string out = "// an animation of the project: the xanim_export, the model it is for ( a model of the project or an\n"
				"// .xmodel_export file: the bone hierarchy ), type relative | delta | delta3d, loop, the animtree a script names\n";
			out += "anim \"" + def.name + "\"\n{\n\tfile \"" + to_utf8(def.file) + "\"\n";

			if (!def.model.empty()) { out += "\tmodel \"" + def.model + "\"\n"; }
			if (def.type != "relative") { out += "\ttype " + def.type + "\n"; }
			if (def.loop) { out += "\tloop\n"; }
			if (!def.tree.empty()) { out += "\ttree \"" + def.tree + "\"\n"; }

			return out + "}\n";
		}

		// everything the project folder holds, read without compiling anything
		struct source_set
		{
			std::vector<item> items;
			std::map<std::string, ca::MaterialDef> materials;	// by material name
			std::map<std::string, model_def> models;			// by model name
			std::map<std::string, fs::path> images;				// image name -> file
			std::map<std::string, anim_def> anims;				// by animation name
			std::map<std::string, fs::path> sounds;				// soundaliases file name ( no extension ) -> the .csv
			std::map<std::string, fs::path> effects;			// effect name ( "misc/x" ) -> the .efx
			std::map<std::string, fs::path> scripts;			// "maps/mp/x.gsc" -> the file in the scripts folder
		};

		// the file of an animation's skeleton; empty when `model` names nothing that exists
		fs::path skeleton_file(const fs::path& dir, const source_set& sources, const std::string& model)
		{
			std::error_code ec;

			if (const auto known = sources.models.find(clean_name(model)); known != sources.models.end() && !known->second.lods.empty())
			{
				const fs::path& lod = known->second.lods[0].file;
				return lod.is_absolute() ? lod : dir / "models" / lod;
			}

			const fs::path given = fs::u8path(model);

			for (const fs::path& candidate : { given, dir / "anims" / given, dir / "models" / given })
			{
				if ((candidate.is_absolute() || candidate != given) && fs::is_regular_file(candidate, ec))
				{
					return candidate;
				}
			}

			return {};
		}

		// regular files below a folder, with their path relative to it ( '/' ), sorted
		std::vector<std::pair<std::string, fs::path>> files_below(const fs::path& dir)
		{
			std::vector<std::pair<std::string, fs::path>> out;
			std::error_code ec;

			for (fs::recursive_directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec))
			{
				if (it->is_regular_file(ec))
				{
					std::string name = to_utf8(fs::relative(it->path(), dir, ec));
					std::replace(name.begin(), name.end(), '\\', '/');
					out.emplace_back(std::move(name), it->path());
				}
			}

			std::sort(out.begin(), out.end());
			return out;
		}

		// one word for the state of many files: changes when one of them does
		std::string tree_signature(const std::vector<std::pair<std::string, fs::path>>& files)
		{
			std::string all;
			for (const auto& [name, file] : files)
			{
				all += name + "=" + file_signature(file) + ";";
			}

			return std::to_string(files.size()) + "f" + std::to_string(std::hash<std::string>{}(all));
		}

		std::vector<fs::path> files_of(const fs::path& dir)
		{
			std::vector<fs::path> out;
			std::error_code ec;

			for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec))
			{
				if (it->is_regular_file(ec))
				{
					out.push_back(it->path());
				}
			}

			std::sort(out.begin(), out.end());
			return out;
		}

		void scan_sources(const fs::path& dir, source_set& out)
		{
			out = {};

			// ---- images
			for (const auto& file : files_of(dir / "images"))
			{
				if (is_image_file(file))
				{
					out.images.emplace(clean_name(file.stem().string()), file);		// the first of two files with one name
				}
			}

			// ---- materials
			std::unordered_set<std::string> used_images;

			for (const auto& file : files_of(dir / "materials"))
			{
				if (lower(file.extension().string()) != ".mtl")
				{
					continue;
				}

				std::string text;
				std::vector<ca::MaterialDef> defs;
				const bool readable = read_text(file, text);
				const ca::Status parsed = readable ? ca::parse_material_text(text, defs) : ca::Status::error("cannot read the file");

				if (!parsed)
				{
					item bad;
					bad.type = kind::material;
					bad.name = "( " + to_utf8(file.filename()) + " )";
					bad.file = file;
					bad.state = status::failed;
					bad.message = parsed.message;
					out.items.push_back(std::move(bad));
					continue;
				}

				for (auto& def : defs)
				{
					item entry;
					entry.type = kind::material;
					entry.name = def.name;
					entry.file = file;
					entry.signature = file_signature(file);

					for (const auto& texture : def.textures)
					{
						used_images.insert(texture.image);

						if (const auto image = out.images.find(texture.image); image != out.images.end())
						{
							entry.signature += "|" + file_signature(image->second);
						}
					}

					if (out.materials.count(def.name))
					{
						entry.state = status::failed;
						entry.message = "a material of this name is defined twice ( also in " + to_utf8(out.items.back().file.filename()) + " )";
						entry.name += " ( again )";
					}
					else
					{
						out.materials.emplace(def.name, def);
					}

					out.items.push_back(std::move(entry));
				}
			}

			for (const auto& [name, file] : out.images)
			{
				item entry;
				entry.type = kind::image;
				entry.name = name;
				entry.file = file;
				entry.signature = file_signature(file);
				entry.state = used_images.count(name) ? status::fresh : status::unused;
				out.items.push_back(std::move(entry));
			}

			// ---- models: .model files, then the export files none of them names
			const fs::path model_dir = dir / "models";
			std::unordered_set<std::string> named_exports;
			const auto model_files = files_of(model_dir);

			const auto lod_path = [&](const fs::path& file) { return file.is_absolute() ? file : model_dir / file; };

			const auto add_model = [&](const model_def& def, const fs::path& file, const std::string& error)
			{
				item entry;
				entry.type = kind::model;
				entry.name = def.name;
				entry.file = file;
				entry.signature = file_signature(file);

				for (const auto& lod : def.lods)
				{
					entry.signature += "|" + file_signature(lod_path(lod.file));
				}

				if (!error.empty())
				{
					entry.state = status::failed;
					entry.message = error;
				}
				else if (out.models.count(def.name))
				{
					entry.state = status::failed;
					entry.message = "a model of this name is defined twice";
					entry.name += " ( again )";
				}
				else
				{
					out.models.emplace(def.name, def);
				}

				out.items.push_back(std::move(entry));
			};

			for (const auto& file : model_files)
			{
				if (lower(file.extension().string()) != ".model")
				{
					continue;
				}

				model_def def;
				std::string error;

				if (!parse_model_file(file, def, error))
				{
					def.name = "( " + to_utf8(file.filename()) + " )";
					def.lods.clear();
				}

				for (const auto& lod : def.lods)
				{
					named_exports.insert(lower(to_utf8(lod_path(lod.file).filename())));
				}

				add_model(def, file, error);
			}

			// <name>_lod<n>.xmodel_export files belong together; any other file is a model of one LOD
			std::map<std::string, std::map<int, fs::path>> loose;

			for (const auto& file : model_files)
			{
				if (lower(file.extension().string()) != ".xmodel_export" || named_exports.count(lower(to_utf8(file.filename()))))
				{
					continue;
				}

				std::string name = clean_name(file.stem().string());
				int lod = 0;

				if (name.size() > 5 && name.compare(name.size() - 5, 4, "_lod") == 0 && name.back() >= '0' && name.back() <= '3')
				{
					lod = name.back() - '0';
					name.resize(name.size() - 5);
				}

				loose[name].emplace(lod, file);
			}

			for (const auto& [name, lods] : loose)
			{
				model_def def;
				def.name = name;

				for (const auto& [index, file] : lods)
				{
					def.lods.push_back({ file.filename(), 0.0f });
				}

				for (std::size_t i = 0; i < def.lods.size(); i++)
				{
					def.lods[i].distance = default_lod_distance(i, def.lods.size());
				}

				add_model(def, lods.begin()->second, {});
			}

			// ---- animations: .anim files, then the export files none of them names
			const fs::path anim_dir = dir / "anims";
			const auto anim_files = files_of(anim_dir);
			std::unordered_set<std::string> named_anims;

			const auto add_anim = [&](const anim_def& def, const fs::path& file, const std::string& error)
			{
				item entry;
				entry.type = kind::anim;
				entry.name = def.name;
				entry.file = file;
				entry.signature = file_signature(file);

				if (error.empty())
				{
					entry.signature += "|" + file_signature(def.file.is_absolute() ? def.file : anim_dir / def.file) + "|" + def.model;

					if (const fs::path skeleton = def.model.empty() ? fs::path() : skeleton_file(dir, out, def.model); !skeleton.empty())
					{
						entry.signature += "|" + file_signature(skeleton);
					}
				}

				if (!error.empty())
				{
					entry.state = status::failed;
					entry.message = error;
				}
				else if (out.anims.count(def.name))
				{
					entry.state = status::failed;
					entry.message = "an animation of this name is defined twice";
					entry.name += " ( again )";
				}
				else
				{
					out.anims.emplace(def.name, def);
				}

				out.items.push_back(std::move(entry));
			};

			for (const auto& file : anim_files)
			{
				if (lower(file.extension().string()) != ".anim")
				{
					continue;
				}

				anim_def def;
				std::string error;

				if (!parse_anim_file(file, def, error))
				{
					def.name = "( " + to_utf8(file.filename()) + " )";
				}
				else
				{
					named_anims.insert(lower(to_utf8(def.file.filename())));
				}

				add_anim(def, file, error);
			}

			for (const auto& file : anim_files)
			{
				if (lower(file.extension().string()) != ".xanim_export" || named_anims.count(lower(to_utf8(file.filename()))))
				{
					continue;
				}

				anim_def def;
				def.name = clean_name(file.stem().string());
				def.file = file.filename();
				add_anim(def, file, {});
			}

			// ---- sounds: every soundaliases\*.csv with the tree it reads from ( sound\, curves, speaker maps )
			{
				const auto alias_files = files_of(dir / "soundaliases");
				std::vector<std::pair<std::string, fs::path>> tree = files_below(dir / "sound");

				for (const auto& file : alias_files)
				{
					const std::string ext = lower(file.extension().string());
					if (ext == ".vfcurve" || ext == ".spkrmap") { tree.emplace_back("soundaliases/" + to_utf8(file.filename()), file); }
				}

				const std::string tree_state = tree_signature(tree);

				for (const auto& file : alias_files)
				{
					if (lower(file.extension().string()) != ".csv")
					{
						continue;
					}

					item entry;
					entry.type = kind::sound;
					entry.name = clean_name(file.stem().string());
					entry.file = file;
					entry.signature = file_signature(file) + "|" + tree_state;

					if (!out.sounds.emplace(entry.name, file).second)
					{
						entry.state = status::failed;
						entry.message = "two alias files of this name";
						entry.name += " ( again )";
					}

					out.items.push_back(std::move(entry));
				}
			}

			// ---- effects: fx\<folder>\<name>.efx is the effect <folder>/<name>
			for (const auto& [relative, file] : files_below(dir / "fx"))
			{
				if (relative.size() < 5 || lower(relative.substr(relative.size() - 4)) != ".efx")
				{
					continue;
				}

				item entry;
				entry.type = kind::fx;
				entry.name = lower(relative.substr(0, relative.size() - 4));
				entry.file = file;
				entry.signature = file_signature(file);

				if (!out.effects.emplace(entry.name, file).second)
				{
					entry.state = status::failed;
					entry.message = "two effects of this name";
					entry.name += " ( again )";
				}

				out.items.push_back(std::move(entry));
			}

			// ---- scripts: not compiled; the built map takes them in front of what the compiler generates
			for (const auto& [relative, file] : files_below(dir / "scripts"))
			{
				item entry;
				entry.type = kind::script;
				entry.name = relative;
				entry.file = file;
				entry.signature = file_signature(file);
				entry.state = status::raw;
				entry.detail = "not compiled: File > Build map puts it into the map zone ( rawfile " + relative + " ) and into the package's scripts, in front of a generated file of that name";
				out.scripts.emplace(lower(relative), file);
				out.items.push_back(std::move(entry));
			}
		}

		// ---- state -----------------------------------------------------------------------------------------

		struct build_result
		{
			std::vector<item> items;
			std::vector<std::string> log;
			std::string error;				// the whole build failed
			bool wrote = false;
			int assets = 0;
			double seconds = 0.0;
		};

		struct state_t
		{
			std::string project;
			std::string project_map;			// the map the project was chosen for
			std::string registered;				// zone name the renderer has from us
			std::vector<item> items;
			source_set sources;

			std::atomic_bool busy = false;
			bool rebuild_wanted = false;
			std::string step;
			std::vector<std::string> log;
			std::vector<std::string> compile_problems;

			std::unordered_set<std::string> world_materials;	// of the open map's world surfaces
			std::uint64_t world_serial = ~0ull;
		};

		state_t g;

		fs::path mod_dir()
		{
			return fs::u8path(utils::fs::get_basepath()) / "mod";
		}

		const char* status_word(const status s)
		{
			switch (s)
			{
			case status::fresh: return "new";
			case status::changed: return "changed";
			case status::built: return "built";
			case status::failed: return "failed";
			case status::raw: return "as is";
			default: return "unused";
			}
		}

		const char* kind_word(const kind k)
		{
			switch (k)
			{
			case kind::image: return "image";
			case kind::material: return "material";
			case kind::model: return "model";
			case kind::anim: return "anim";
			case kind::sound: return "sound";
			case kind::fx: return "fx";
			default: return "script";
			}
		}

		std::string one_line(std::string text)
		{
			for (char& c : text)
			{
				if (c == '\t' || c == '\n' || c == '\r') { c = ' '; }
			}

			return text;
		}

		void save_build_state(const fs::path& dir, const std::vector<item>& items)
		{
			std::string text = "// what the last build made of each source of the project ( CoDOL Radiant )\n";

			for (const auto& entry : items)
			{
				if (entry.state != status::built && entry.state != status::failed)
				{
					continue;
				}

				std::string notes;
				for (const auto& note : entry.notes)
				{
					notes += (notes.empty() ? "" : " | ") + one_line(note);
				}

				text += std::string(kind_word(entry.type)) + "\t" + entry.name + "\t" + entry.signature + "\t" + status_word(entry.state) + "\t" + one_line(entry.detail) + "\t"
					+ one_line(entry.message) + "\t" + notes + "\n";
			}

			write_text(dir / "build_state.txt", text);
		}

		void apply_build_state(const fs::path& dir, std::vector<item>& items, const bool have_zone)
		{
			std::string text;
			if (!read_text(dir / "build_state.txt", text))
			{
				return;
			}

			std::map<std::string, std::vector<std::string>> records;

			for (const auto& line : utils::split(text, '\n'))
			{
				std::vector<std::string> fields;
				std::size_t at = 0;

				while (true)
				{
					const auto tab = line.find('\t', at);
					fields.push_back(line.substr(at, tab == std::string::npos ? std::string::npos : tab - at));
					if (tab == std::string::npos) { break; }
					at = tab + 1;
				}

				if (fields.size() >= 7)
				{
					while (!fields[6].empty() && fields[6].back() == '\r') { fields[6].pop_back(); }
					records[fields[0] + "\t" + fields[1]] = fields;
				}
			}

			for (auto& entry : items)
			{
				if (entry.state == status::unused || entry.state == status::failed || entry.state == status::raw)
				{
					continue;		// failed here: the source could not even be read
				}

				const auto record = records.find(std::string(kind_word(entry.type)) + "\t" + entry.name);
				if (record == records.end())
				{
					continue;
				}

				const auto& f = record->second;
				const bool was_built = f[3] == "built";

				if (f[2] != entry.signature || (was_built && !have_zone))
				{
					entry.state = status::changed;
					continue;
				}

				entry.state = was_built ? status::built : status::failed;
				entry.detail = f[4];
				entry.message = f[5];
				entry.notes.clear();

				std::size_t at = 0;
				while (at < f[6].size())
				{
					const auto bar = f[6].find(" | ", at);
					entry.notes.push_back(f[6].substr(at, bar == std::string::npos ? std::string::npos : bar - at));
					if (bar == std::string::npos) { break; }
					at = bar + 3;
				}
			}
		}

		// ---- the renderer ----------------------------------------------------------------------------------

		void register_with_renderer(const bool refresh)
		{
			auto& s = get_scene();

			if (!s.renderer.initialised() || g.project.empty())
			{
				return;
			}

			if (!g.registered.empty() && g.registered != zone_name())
			{
				s.renderer.unregister_zone(g.registered);
				g.registered.clear();
			}

			if (!zone_exists())
			{
				if (!g.registered.empty())
				{
					s.renderer.unregister_zone(g.registered);
					g.registered.clear();

					if (refresh) { s.refresh_assets(); }
				}

				return;
			}

			std::string error;
			if (!s.renderer.register_zone(zone_file(), &error))
			{
				game::printf_to_console("^1[assets] the renderer does not take %s: %s\n", to_utf8(zone_file().filename()).c_str(), error.c_str());
				return;
			}

			g.registered = zone_name();

			if (refresh)
			{
				s.refresh_assets();
			}
		}

		void drop_from_renderer()
		{
			auto& s = get_scene();

			if (!g.registered.empty() && s.renderer.initialised())
			{
				s.renderer.unregister_zone(g.registered);

				if (s.renderer.loaded())
				{
					s.refresh_assets();
				}
			}

			g.registered.clear();
		}

		const std::unordered_set<std::string>& world_materials()
		{
			auto& s = get_scene();

			if (g.world_serial != s.open_serial)
			{
				g.world_serial = s.open_serial;
				g.world_materials.clear();

				if (s.renderer.loaded())
				{
					for (const auto& object : s.renderer.objects())
					{
						if (object.kind == view::object_kind::world_surface && !object.name.empty())
						{
							g.world_materials.insert(object.name);
						}
					}
				}
			}

			return g.world_materials;
		}

		// ---- the build, on the worker thread -----------------------------------------------------------------

		struct build_job
		{
			fs::path dir, install, exe, matlib, fxatlas, out;
			std::string zone, project;
			bool have_shipped = false;
			std::unordered_set<std::string> shipped_materials;		// of the zones of the open map, without the project's
		};

		void set_step(const std::string& text)
		{
			components::codol_game::post_to_gui([text] { g.step = text; });
		}

		ca::ImageUse use_of_sampler(const std::string& sampler)
		{
			return sampler == "normalMap" ? ca::ImageUse::normal : sampler == "specularMap" ? ca::ImageUse::specular : sampler == "detailMap" ? ca::ImageUse::detail : ca::ImageUse::color;
		}

		build_result run_build(const build_job& job)
		{
			build_result result;
			const auto started = std::chrono::steady_clock::now();
			const auto say = [&](const std::string& line) { result.log.push_back(line); };

			source_set sources;
			scan_sources(job.dir, sources);
			result.items = sources.items;

			const auto item_of = [&](const kind type, const std::string& name) -> item*
			{
				for (auto& entry : result.items)
				{
					if (entry.type == type && entry.name == name) { return &entry; }
				}

				return nullptr;
			};

			const auto fail_item = [&](item& entry, const std::string& why)
			{
				entry.state = status::failed;
				entry.message = why;
				entry.detail.clear();
				say(std::string(kind_word(entry.type)) + " " + entry.name + ": FAILED: " + why);
			};

			std::error_code ec;
			if (!fs::exists(job.matlib, ec))
			{
				set_step("reading the materials of the install ( once, about 20 s ) ..");
				say("material library: scanning the install ( once )");
			}
			else
			{
				set_step("material library ..");
			}

			ca::LinkOptions link;
			link.install = job.install;
			link.exe = job.exe;
			link.libraryCache = job.matlib;
			link.zoneName = job.zone;

			ca::AssetLinker linker;
			if (const ca::Status s = linker.begin(link); !s)
			{
				result.error = s.message;
				return result;
			}

			// ---- what each image is used for: the materials say
			std::map<std::string, ca::ImageUse> uses;
			for (const auto& [name, def] : sources.materials)
			{
				for (const auto& texture : def.textures)
				{
					uses.emplace(texture.image, (def.type == "2d" || def.techset == "2d") ? ca::ImageUse::ui : use_of_sampler(texture.sampler));
				}
			}

			// ---- images
			std::unordered_set<std::string> images_ok;

			for (const auto& [name, file] : sources.images)
			{
				item* entry = item_of(kind::image, name);
				const auto use = uses.find(name);

				if (!entry || use == uses.end())
				{
					continue;		// no material names it
				}

				set_step("image " + name + " ..");

				ca::SourceImage source;
				if (const ca::Status s = ca::load_image(file, source); !s)
				{
					fail_item(*entry, s.message);
					continue;
				}

				ca::ImageOptions options;
				options.use = use->second;

				ca::CompiledImage compiled;
				if (const ca::Status s = ca::compile_image(name, source, options, compiled); !s)
				{
					fail_item(*entry, s.message);
					continue;
				}

				if (const ca::Status s = linker.add_image(compiled); !s)
				{
					fail_item(*entry, s.message);
					continue;
				}

				char text[160];
				std::snprintf(text, sizeof(text), "%u x %u %s, %d levels, %s map", compiled.width, compiled.height,
					::codol::render::iwi_format_name(static_cast<::codol::render::iwi_format>(compiled.iwiFormat)), compiled.levelCount,
					options.use == ca::ImageUse::normal ? "normal" : options.use == ca::ImageUse::specular ? "specular" : options.use == ca::ImageUse::detail ? "detail"
					: options.use == ca::ImageUse::ui ? "2d" : "colour");
				entry->state = status::built;
				entry->detail = text;
				entry->message.clear();
				images_ok.insert(name);
				result.assets++;
				say("image " + name + ": " + text);
			}

			// ---- materials
			std::unordered_set<std::string> materials_ok;
			std::map<std::string, std::int32_t> surface_flags;		// material -> surface flags of its collision triangles

			for (const auto& [name, def] : sources.materials)
			{
				item* entry = item_of(kind::material, name);
				if (!entry || entry->state == status::failed)
				{
					continue;
				}

				set_step("material " + name + " ..");
				std::string problem;
				std::vector<std::string> notes;

				for (const auto& texture : def.textures)
				{
					if (sources.images.count(texture.image))
					{
						if (!images_ok.count(texture.image))
						{
							const item* image = item_of(kind::image, texture.image);
							problem = "its image " + texture.image + " failed" + (image && !image->message.empty() ? ": " + image->message : std::string());
						}
					}
					else
					{
						notes.push_back("image " + texture.image + " is not in the project: named only, a loaded zone of the game has to have it");
					}
				}

				ca::CompiledMaterial compiled;

				if (problem.empty())
				{
					if (const ca::Status s = ca::compile_material(def, linker.library(), compiled); !s)
					{
						problem = s.message;
					}
					else if (const ca::Status added = linker.add_material(compiled); !added)
					{
						problem = added.message;
					}
				}

				if (!problem.empty())
				{
					fail_item(*entry, problem);
					continue;
				}

				for (int type = 0; type < 32; type++)
				{
					if (compiled.header.surfaceTypeBits & (1u << type))
					{
						surface_flags[compiled.name] = (type + 1) << 20;		// as the shipped collision surfaces have it
					}
				}

				if (!compiled.stateShipped)
				{
					notes.push_back("state derived: no shipped material of " + compiled.techset + " has this combination ( a guess, see codol_assets README )");
				}

				notes.insert(notes.end(), compiled.notes.begin(), compiled.notes.end());

				entry->state = status::built;
				entry->detail = "technique set " + compiled.techset + ", sort " + std::to_string(compiled.header.sortKey) + ", " + std::to_string(compiled.textures.size()) + " textures";
				entry->message.clear();
				entry->notes = notes;
				materials_ok.insert(name);
				result.assets++;
				say("material " + name + ": " + entry->detail + ( compiled.stateShipped ? "" : ", state derived" ));
			}

			// ---- models
			const auto material_known = [&](const std::string& name)
			{
				return materials_ok.count(name) || (job.have_shipped && job.shipped_materials.count(name));
			};

			for (const auto& [name, def] : sources.models)
			{
				item* entry = item_of(kind::model, name);
				if (!entry || entry->state == status::failed)
				{
					continue;
				}

				set_step("model " + name + " ..");

				ca::ModelOptions options;
				options.name = name;
				options.collisionLod = def.collision_lod;
				options.contents = def.contents;
				options.materialNames = def.materials;
				options.collisionFlagsByMaterial = surface_flags;
				std::string problem;

				for (const auto& lod : def.lods)
				{
					ca::ModelLodSource source;
					source.distance = lod.distance;
					const fs::path file = lod.file.is_absolute() ? lod.file : job.dir / "models" / lod.file;

					if (const ca::Status s = ca::load_xmodel_export(file, source.mesh); !s)
					{
						problem = s.message;
						break;
					}

					options.lods.push_back(std::move(source));
				}

				if (problem.empty() && def.collision_lod >= static_cast<int>(options.lods.size()))
				{
					problem = "collision LOD " + std::to_string(def.collision_lod) + " does not exist ( " + std::to_string(options.lods.size()) + " LODs )";
				}

				// material names of the export -> materials that exist
				std::vector<std::string> notes;

				for (std::size_t l = 0; problem.empty() && l < options.lods.size(); l++)
				{
					for (const std::string& export_name : options.lods[l].mesh.materials)
					{
						std::string target = export_name;

						if (const auto mapped = options.materialNames.find(export_name); mapped != options.materialNames.end())
						{
							target = mapped->second;
						}
						else if (!material_known(export_name) && export_name.rfind("codol_", 0) == 0)
						{
							// CODOL-AssetExplorer writes "mc/mtl_x" as "codol_mc_mtl_x"
							target = export_name.substr(6);
							if (const auto under = target.find('_'); under != std::string::npos) { target[under] = '/'; }
							options.materialNames[export_name] = target;
						}

						if (!material_known(target))
						{
							if (job.have_shipped)
							{
								problem = "material '" + target + "'" + (target != export_name ? " ( of the export's '" + export_name + "' )" : "")
									+ " does not exist: make it ( New material, kind model ) or name another one in the .model file";
								break;
							}

							notes.push_back("material " + target + " was not checked: no map was open");
						}
					}
				}

				ca::CompiledModel compiled;

				if (problem.empty())
				{
					if (const ca::Status s = ca::compile_model(options, compiled); !s)
					{
						problem = s.message;
					}
					else if (const ca::Status added = linker.add_model(compiled); !added)
					{
						problem = added.message;
					}
				}

				if (!problem.empty())
				{
					fail_item(*entry, problem);
					continue;
				}

				std::size_t vertices = 0, triangles = 0, surfaces = 0;
				for (const auto& lod : compiled.lods)
				{
					for (const auto& surface : lod.surfaces)
					{
						vertices += surface.vertices.size();
						triangles += surface.indices.size() / 3;
						surfaces++;
					}
				}

				notes.insert(notes.end(), compiled.notes.begin(), compiled.notes.end());

				char text[200];
				std::snprintf(text, sizeof(text), "%zu bones, %zu LODs, %zu surfaces, %zu vertices, %zu triangles, %s", compiled.boneNames.size(), compiled.lods.size(), surfaces,
					vertices, triangles, compiled.collision.empty() ? "no collision" : (std::to_string(compiled.collision.size()) + " collision surfaces").c_str());
				entry->state = status::built;
				entry->detail = text;
				entry->message.clear();
				entry->notes = notes;
				result.assets++;
				say("model " + name + ": " + text);
			}

			// ---- animations
			std::map<std::string, std::vector<std::string>> trees;		// animtree -> its animations of the project

			for (const auto& [name, def] : sources.anims)
			{
				item* entry = item_of(kind::anim, name);
				if (!entry || entry->state == status::failed)
				{
					continue;
				}

				set_step("animation " + name + " ..");

				ca::ExportAnim source;
				ca::ExportModel skeleton;
				ca::AnimOptions options;
				options.name = name;
				options.looping = def.loop;
				options.type = def.type == "delta" ? ca::AnimType::delta : def.type == "delta3d" ? ca::AnimType::delta3d : ca::AnimType::relative;
				std::string problem;
				std::vector<std::string> notes;

				if (const ca::Status s = ca::load_xanim_export(def.file.is_absolute() ? def.file : job.dir / "anims" / def.file, source); !s)
				{
					problem = s.message;
				}
				else if (!def.model.empty())
				{
					const fs::path file = skeleton_file(job.dir, sources, def.model);

					if (file.empty())
					{
						problem = "its model '" + def.model + "' is neither a model of the project nor an .xmodel_export file in anims\\ or models\\";
					}
					else if (const ca::Status loaded = ca::load_xmodel_export(file, skeleton); !loaded)
					{
						problem = loaded.message;
					}
					else
					{
						options.skeleton = &skeleton;
					}
				}

				ca::CompiledAnim compiled;

				if (problem.empty())
				{
					if (const ca::Status s = ca::compile_anim(source, options, compiled); !s)
					{
						problem = s.message;
					}
					else if (const ca::Status added = linker.add_anim(compiled); !added)
					{
						problem = added.message;
					}
				}

				if (!problem.empty())
				{
					fail_item(*entry, problem);
					continue;
				}

				const std::string tree = def.tree.empty() ? job.project : def.tree;
				trees[tree].push_back(name);
				notes.insert(notes.end(), compiled.notes.begin(), compiled.notes.end());

				// an animtree of the modder's own is used instead of the generated one: it has to name the animation
				if (std::string own; read_text(job.dir / "scripts" / "animtrees" / (tree + ".atr"), own))
				{
					const auto lines = tokenize(own);
					if (std::find(lines.begin(), lines.end(), name) == lines.end())
					{
						notes.push_back("scripts\\animtrees\\" + tree + ".atr of the project is used for the animtree and does not list " + name + ": add a line with the name");
					}
				}

				const auto& h = compiled.raw.header;
				char text[240];
				std::snprintf(text, sizeof(text), "%u parts, %u frames at %g / s%s%s, %u notifies, animtree %s", static_cast<unsigned>(h.boneCount[9]), h.numframes + 1u, h.framerate,
					(h.flags & ca::ANIM_FLAG_LOOP) ? ", looping" : "", (h.flags & ca::ANIM_FLAG_DELTA) ? ", delta" : (h.flags & ca::ANIM_FLAG_DELTA_3D) ? ", delta 3d" : "",
					static_cast<unsigned>(h.notifyCount), tree.c_str());
				entry->state = status::built;
				entry->detail = text;
				entry->message.clear();
				entry->notes = notes;
				result.assets++;
				say("anim " + name + ": " + text);
			}

			// what a map needs besides the animations ( codol_assets README ): the animtree a script is compiled
			// against and the map's animlist. Written as files; the map package makes rawfiles of them.
			{
				const fs::path generated = job.dir / "generated";

				for (const auto& old : files_of(generated / "animtrees"))
				{
					if (lower(old.extension().string()) == ".atr") { fs::remove(old, ec); }
				}

				fs::remove(generated / "animlists" / "map.animlist", ec);
				std::string list;

				for (const auto& [tree, names] : trees)
				{
					std::string text;
					list += "=" + tree + "\n";

					for (const auto& name : names)
					{
						text += name + "\n";
						list += name + "\n";
					}

					write_text(generated / "animtrees" / (tree + ".atr"), text);
					say("animtree " + tree + ": " + std::to_string(names.size()) + " animation(s), generated\\animtrees\\" + tree + ".atr ( #using_animtree( \"" + tree + "\" ) )");
				}

				if (!list.empty())
				{
					write_text(generated / "animlists" / "map.animlist", list);
				}
			}

			// ---- sounds: every alias file with the loaded sounds and curves it names
			std::vector<std::pair<std::string, std::vector<std::uint8_t>>> streamed;

			for (const auto& [name, file] : sources.sounds)
			{
				item* entry = item_of(kind::sound, name);
				if (!entry || entry->state == status::failed)
				{
					continue;
				}

				set_step("sounds " + name + " ..");
				ca::SoundBuild sounds;

				if (const ca::Status s = ca::build_sound_aliases(file, job.dir, linker.editor(), sounds); !s)
				{
					fail_item(*entry, s.message);
					continue;
				}

				std::vector<std::string> notes;

				for (const auto& line : sounds.log)
				{
					say("  " + line);

					// what was assumed, not what was made
					if (line.rfind("loaded sound ", 0) != 0 && line.rfind("sound curve ", 0) != 0)
					{
						notes.push_back(line);
					}
				}

				for (auto& packed : sounds.streamedFiles)
				{
					if (std::none_of(streamed.begin(), streamed.end(), [&](const auto& other) { return other.first == packed.first; }))
					{
						streamed.push_back(std::move(packed));
					}
				}

				char text[200];
				std::snprintf(text, sizeof(text), "%u alias lists ( %u aliases ), %u loaded sounds, %u curves, %zu streamed files", sounds.lists, sounds.aliases, sounds.loadedSounds,
					sounds.curves, sounds.streamedFiles.size());

				if (!sounds.streamedFiles.empty())
				{
					notes.push_back("streamed files are not in the zone: they go into zz_" + job.project + "_sounds.iwd, which belongs into the game's main folder");
				}

				entry->state = status::built;
				entry->detail = text;
				entry->message.clear();
				entry->notes = notes;
				result.assets += static_cast<int>(sounds.lists + sounds.loadedSounds + sounds.curves);
				say("sound " + name + ": " + text);
			}

			// ---- effects: after the materials and models they draw
			{
				ca::FxAtlasLibrary atlas;
				bool atlas_open = false;
				std::string atlas_problem;

				for (const auto& [name, file] : sources.effects)
				{
					item* entry = item_of(kind::fx, name);
					if (!entry || entry->state == status::failed)
					{
						continue;
					}

					if (!atlas_open && atlas_problem.empty())
					{
						if (!fs::exists(job.fxatlas, ec))
						{
							set_step("reading the texture atlases of the install ( once, about 30 s ) ..");
							say("effect atlas table: scanning the install ( once )");
						}

						if (const ca::Status s = atlas.open(job.install, linker.key(), job.fxatlas); !s)
						{
							atlas_problem = "the atlas table of the install: " + s.message;
						}

						atlas_open = atlas_problem.empty();
					}

					if (!atlas_open)
					{
						fail_item(*entry, atlas_problem);
						continue;
					}

					set_step("effect " + name + " ..");

					ca::EditorEffect effect;
					ca::CompiledFx fx;
					std::string problem;
					std::vector<std::string> notes;

					if (const ca::Status s = ca::load_efx(file, effect); !s)
					{
						problem = s.message;
					}
					else if (const ca::Status compiled = ca::compile_fx(name, effect, fx, ca::fx_compile_options(linker.editor(), atlas, job.dir / "fx")); !compiled)
					{
						problem = compiled.message;
					}
					else
					{
						notes = fx.notes;

						if (const ca::Status linked = ca::link_fx(linker.editor(), fx, &notes); !linked)
						{
							problem = linked.message;
						}
					}

					if (!problem.empty())
					{
						fail_item(*entry, problem);
						continue;
					}

					char text[200];
					std::snprintf(text, sizeof(text), "%zu elements ( %d looping, %d one shot, %d emission ), %d bytes", fx.elems.size(), fx.countLooping, fx.countOneShot,
						fx.countEmission, fx.totalSize);
					entry->state = status::built;
					entry->detail = text;
					entry->message.clear();
					entry->notes = notes;
					result.assets++;
					say("fx " + name + ": " + text);
				}
			}

			for (const auto& line : linker.log())
			{
				say("  " + line);
			}

			// the streamed sound files: an archive next to the zone, or none
			const fs::path sound_archive = job.dir / ("zz_" + job.project + "_sounds.iwd");

			if (streamed.empty())
			{
				fs::remove(sound_archive, ec);
			}
			else if (const ca::Status s = ca::write_iwd(sound_archive, streamed); !s)
			{
				say("sound archive: FAILED: " + s.message);
			}
			else
			{
				say("wrote " + to_utf8(sound_archive.filename()) + ": " + std::to_string(streamed.size()) + " streamed sound file(s)");
			}

			if (!result.assets)
			{
				say("nothing was built: no zone written");
				result.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
				return result;
			}

			// ---- the zone: written by the game's loaders; what they do not accept is not written
			set_step("writing " + job.zone + ".ff ..");

			std::vector<std::uint8_t> zone_bytes;
			if (const ca::Status s = linker.write_zone(zone_bytes); !s)
			{
				result.error = "the game's loaders do not accept the zone: " + s.message;
				return result;
			}

			fs::path temp = job.out;
			temp += ".tmp";

			if (const ca::Status s = ::codol::zone::write_fastfile(temp, zone_bytes, linker.key()); !s)
			{
				result.error = s.message;
				return result;
			}

			fs::rename(temp, job.out, ec);
			if (ec)
			{
				fs::remove(job.out, ec);
				fs::rename(temp, job.out, ec);
			}

			if (ec)
			{
				result.error = "cannot replace " + to_utf8(job.out) + ": " + ec.message();
				return result;
			}

			result.wrote = true;
			result.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

			char text[200];
			std::snprintf(text, sizeof(text), "wrote %s: %zu assets, zone %zu bytes ( %.1f s )", to_utf8(job.out.filename()).c_str(), linker.editor().asset_count(), zone_bytes.size(),
				result.seconds);
			say(text);
			return result;
		}

		void build_finished(build_result result, const fs::path& dir, const std::string& project_name)
		{
			g.busy = false;
			g.step.clear();
			g.log = result.log;

			for (const auto& line : result.log)
			{
				game::printf_to_console("[assets] %s\n", line.c_str());
			}

			if (!result.error.empty())
			{
				game::printf_to_console("^1[assets] build of %s failed: %s\n", project_name.c_str(), result.error.c_str());
				g.log.push_back("FAILED: " + result.error);

				// every source that was to be built failed with the zone
				for (auto& entry : result.items)
				{
					if (entry.state != status::unused && entry.state != status::failed && entry.state != status::raw)
					{
						entry.state = status::failed;
						entry.detail.clear();
						entry.message = "the zone was not written: " + result.error;
					}
				}
			}

			save_build_state(dir, result.items);

			if (project_name != g.project)
			{
				return;		// another project was chosen meanwhile
			}

			rescan();

			int failed = 0, built = 0;
			std::string first_failure;

			for (const auto& entry : g.items)
			{
				built += entry.state == status::built;

				if (entry.state == status::failed)
				{
					if (!failed++) { first_failure = entry.name + ": " + entry.message; }
				}
			}

			if (result.wrote)
			{
				register_with_renderer(true);
			}

			if (failed)
			{
				const std::string text = std::to_string(failed) + " of " + std::to_string(failed + built) + " failed. " + first_failure;
				ImGui::Toast(ImGuiToastType_Error, "Assets", text.c_str(), 8000);
			}
			else if (result.wrote)
			{
				const std::string text = std::to_string(built) + " assets built into " + zone_name() + ".ff";
				ImGui::Toast(ImGuiToastType_Success, "Assets", text.c_str(), 4000);
			}

			if (g.rebuild_wanted)
			{
				g.rebuild_wanted = false;
				std::string error;
				build(error);
			}
		}

		// "<stem>.<ext>" of a source file inside the project's image folder; copies when it is not there yet
		bool take_image(const fs::path& source, const bool overwrite, std::string& name, std::string& error)
		{
			std::error_code ec;
			if (!fs::is_regular_file(source, ec))
			{
				error = "no such file: " + to_utf8(source);
				return false;
			}

			if (!is_image_file(source))
			{
				error = to_utf8(source.filename()) + " is not a .png, .tga or .dds";
				return false;
			}

			name = clean_name(source.stem().string());
			const fs::path dir = project_dir() / "images";
			const fs::path target = dir / (name + lower(source.extension().string()));
			fs::create_directories(dir, ec);

			if (fs::equivalent(source, target, ec))
			{
				return true;
			}

			// one image per name, whatever its format
			for (const auto& other : files_of(dir))
			{
				if (is_image_file(other) && clean_name(other.stem().string()) == name)
				{
					if (same_file_contents(source, other))
					{
						return true;
					}

					if (!overwrite)
					{
						error = "the project already has another image named " + name + " ( " + to_utf8(other.filename()) + " ): rename the file, or overwrite";
						return false;
					}

					fs::remove(other, ec);
				}
			}

			fs::copy_file(source, target, fs::copy_options::overwrite_existing, ec);
			if (ec)
			{
				error = "cannot copy " + to_utf8(source) + ": " + ec.message();
				return false;
			}

			return true;
		}

		std::string material_file_name(const std::string& material)
		{
			std::string out = material;
			for (char& c : out)
			{
				if (c == '/') { c = '_'; }
			}

			return out + ".mtl";
		}
	}

	// ---- catalog of what "New material" makes --------------------------------------------------------------

	const std::vector<material_kind>& material_kinds()
	{
		static const std::vector<material_kind> kinds = {
			{ "world", "World ( brushes, terrain )", "wc/", "world phong", "" },
			{ "world_alpha", "World, alpha tested ( fences, grates )", "wc/", "world phong", "\talphaTest ge128\n\tcullFace none\n" },
			{ "world_blend", "World, blended ( glass )", "wc/", "world phong", "\tblendFunc blend\n" },
			{ "decal", "Decal ( blended, on top of a surface )", "wc/", "world phong", "\tblendFunc blend\n\tpolygonOffset decal\n" },
			{ "world_unlit", "World, unlit", "wc/", "world unlit", "" },
			{ "model", "Model", "mc/", "model phong", "" },
			{ "model_alpha", "Model, alpha tested ( foliage )", "mc/", "model phong", "\talphaTest ge128\n\tcullFace none\n" },
			{ "model_blend", "Model, blended", "mc/", "model phong", "\tblendFunc blend\n" },
			{ "model_unlit", "Model, unlit", "mc/", "model unlit", "" },
			{ "2d", "2D ( hud, menu )", "", "2d", "" },
		};

		return kinds;
	}

	const std::vector<std::string>& surface_types()
	{
		static const std::vector<std::string> types = { "", "asphalt", "bark", "brick", "carpet", "ceramic", "cloth", "concrete", "cushion", "dirt", "flesh", "foliage", "fruit",
			"glass", "grass", "gravel", "ice", "metal", "mud", "paintedmetal", "paper", "plaster", "plastic", "rock", "rubber", "sand", "snow", "water", "wood" };
		return types;
	}

	// ---- project ---------------------------------------------------------------------------------------------

	const std::string& project() { return g.project; }

	fs::path project_dir() { return mod_dir() / fs::u8path(g.project); }

	std::string zone_name() { return g.project + "_assets"; }

	fs::path zone_file() { return project_dir() / fs::u8path(zone_name() + ".ff"); }

	bool zone_exists()
	{
		std::error_code ec;
		return !g.project.empty() && fs::is_regular_file(zone_file(), ec);
	}

	const std::vector<item>& items() { return g.items; }

	const item* find(const kind type, const std::string& name)
	{
		for (const auto& entry : g.items)
		{
			if (entry.type == type && entry.name == name) { return &entry; }
		}

		return nullptr;
	}

	void rescan()
	{
		g.items.clear();
		g.sources = {};

		if (g.project.empty())
		{
			return;
		}

		scan_sources(project_dir(), g.sources);
		g.items = g.sources.items;
		apply_build_state(project_dir(), g.items, zone_exists());
	}

	void set_project(const std::string& name)
	{
		auto& s = get_scene();
		const std::string wanted = clean_name(name.empty() ? s.map_name : name);

		if (g.busy)
		{
			game::printf_to_console("^1[assets] a build is running: the project stays %s\n", g.project.c_str());
			return;
		}

		if (wanted == g.project)
		{
			return;
		}

		if (!g.registered.empty() && g.registered != wanted + "_assets")
		{
			drop_from_renderer();
		}

		g.project = wanted;
		g.compile_problems.clear();
		g.log.clear();
		rescan();

		if (!g.project.empty())
		{
			register_with_renderer(s.renderer.loaded());
			game::printf_to_console("[assets] project %s: %s ( %d sources%s )\n", g.project.c_str(), to_utf8(project_dir()).c_str(), static_cast<int>(g.items.size()),
				zone_exists() ? ", zone built" : "");
		}
	}

	void map_opening(const std::string& map)
	{
		// before the renderer loads the map: the zone of the map's project is then one of its extra zones
		if (g.busy)
		{
			return;
		}

		const std::string wanted = clean_name(map);

		// the same map again keeps the project that was chosen for it ( asset_project )
		if (wanted != g.project && (map != g.project_map || g.project.empty()))
		{
			auto& s = get_scene();

			if (!g.registered.empty() && s.renderer.initialised())
			{
				s.renderer.unregister_zone(g.registered);
			}

			g.registered.clear();
			g.project = wanted;
			g.compile_problems.clear();
			g.log.clear();
			rescan();
		}

		g.project_map = map;
		register_with_renderer(false);
	}

	// ---- New material ----------------------------------------------------------------------------------------

	bool new_material(const material_request& request, std::string& error, std::string* made)
	{
		if (g.project.empty())
		{
			error = "open a map first: the project is named after it";
			return false;
		}

		const material_kind* which = nullptr;
		for (const auto& candidate : material_kinds())
		{
			if (request.kind == candidate.id) { which = &candidate; }
		}

		if (!which)
		{
			error = "unknown kind '" + request.kind + "'";
			return false;
		}

		std::string leaf = clean_name(request.name);

		// "wc/x" typed with the prefix of the kind is fine
		if (const std::string typed = clean_name(request.name, true); *which->prefix && typed.rfind(which->prefix, 0) == 0)
		{
			leaf = clean_name(typed.substr(std::strlen(which->prefix)));
		}

		if (leaf.empty())
		{
			error = "the material needs a name";
			return false;
		}

		if (request.color.empty())
		{
			error = "the material needs a colour map";
			return false;
		}

		if (!request.surface_type.empty() && !ca::surface_type_bits(request.surface_type))
		{
			error = "unknown surface type '" + request.surface_type + "'";
			return false;
		}

		const std::string name = which->prefix + leaf;
		const fs::path file = project_dir() / "materials" / material_file_name(name);
		std::error_code ec;

		if (!request.overwrite)
		{
			if (fs::exists(file, ec) || g.sources.materials.count(name))
			{
				error = "the project already has a material " + name + " ( overwrite, or pick another name )";
				return false;
			}

			if (material_exists(name))
			{
				error = "the game already has a material " + name + ": pick another name";
				return false;
			}
		}

		std::string color, normal, specular;

		if (!take_image(request.color, request.overwrite, color, error)
			|| (!request.normal.empty() && !take_image(request.normal, request.overwrite, normal, error))
			|| (!request.specular.empty() && !take_image(request.specular, request.overwrite, specular, error)))
		{
			return false;
		}

		std::string text = "// made by CoDOL Radiant ( New material ); the format: codol_assets/README.md, \".mtl\"\n";
		text += "material \"" + name + "\"\n{\n\ttype \"" + which->type + "\"\n\tcolorMap " + color + "\n";

		if (!normal.empty()) { text += "\tnormalMap " + normal + "\n"; }
		if (!specular.empty()) { text += "\tspecularMap " + specular + "\n"; }

		text += which->extra;

		if (!request.surface_type.empty()) { text += "\tsurfaceType " + request.surface_type + "\n"; }

		text += "}\n";

		// what was written has to read back
		std::vector<ca::MaterialDef> check;
		if (const ca::Status s = ca::parse_material_text(text, check); !s || check.size() != 1)
		{
			error = "the definition does not parse: " + s.message;
			return false;
		}

		if (!write_text(file, text))
		{
			error = "cannot write " + to_utf8(file);
			return false;
		}

		game::printf_to_console("[assets] material %s: %s ( technique set %s )\n", name.c_str(), to_utf8(file).c_str(), ca::techset_for(check[0]).c_str());

		if (made)
		{
			*made = name;
		}

		rescan();

		std::string build_error;
		if (!build(build_error))
		{
			game::printf_to_console("^1[assets] %s\n", build_error.c_str());
		}

		return true;
	}

	// ---- Import model ----------------------------------------------------------------------------------------

	bool read_export_materials(const fs::path& file, std::vector<std::string>& out, std::string& error)
	{
		ca::ExportModel model;
		if (const ca::Status s = ca::load_xmodel_export(file, model); !s)
		{
			error = s.message;
			return false;
		}

		for (const auto& name : model.materials)
		{
			if (std::find(out.begin(), out.end(), name) == out.end())
			{
				out.push_back(name);
			}
		}

		return true;
	}

	bool material_exists(const std::string& name)
	{
		if (g.sources.materials.count(name))
		{
			return true;
		}

		auto& s = get_scene();
		if (!s.renderer.loaded())
		{
			return false;
		}

		const auto& names = s.renderer.material_names();
		return std::binary_search(names.begin(), names.end(), name);
	}

	std::string default_material_for(const std::string& export_name)
	{
		if (material_exists(export_name))
		{
			return export_name;
		}

		for (const char* prefix : { "mc/", "wc/" })
		{
			if (material_exists(prefix + export_name)) { return prefix + export_name; }
		}

		if (export_name.rfind("codol_", 0) == 0)
		{
			std::string native = export_name.substr(6);
			if (const auto under = native.find('_'); under != std::string::npos) { native[under] = '/'; }
			return native;
		}

		return export_name;
	}

	bool import_model(const model_request& request, std::string& error)
	{
		if (g.project.empty())
		{
			error = "open a map first: the project is named after it";
			return false;
		}

		const std::string name = clean_name(request.name);

		if (name.empty())
		{
			error = "the model needs a name";
			return false;
		}

		if (request.lods.empty() || request.lods.size() > 4)
		{
			error = "a model has 1 to 4 LOD files";
			return false;
		}

		if (request.collision_lod >= static_cast<int>(request.lods.size()))
		{
			error = "collision LOD " + std::to_string(request.collision_lod) + " does not exist";
			return false;
		}

		const fs::path dir = project_dir() / "models";
		const fs::path file = dir / (name + ".model");
		std::error_code ec;

		if (!request.overwrite)
		{
			if (fs::exists(file, ec) || g.sources.models.count(name))
			{
				error = "the project already has a model " + name + " ( overwrite, or pick another name )";
				return false;
			}

			if (auto& s = get_scene(); s.renderer.loaded())
			{
				const auto& names = s.renderer.model_names();
				if (std::binary_search(names.begin(), names.end(), name))
				{
					error = "the game already has a model " + name + ": pick another name";
					return false;
				}
			}
		}

		// every LOD has to read before anything is copied
		for (const auto& lod : request.lods)
		{
			ca::ExportModel mesh;
			if (const ca::Status s = ca::load_xmodel_export(lod.file, mesh); !s)
			{
				error = s.message;
				return false;
			}
		}

		fs::create_directories(dir, ec);

		model_def def;
		def.name = name;
		def.collision_lod = request.collision_lod;
		def.materials = request.materials;

		for (std::size_t i = 0; i < request.lods.size(); i++)
		{
			const fs::path target = dir / (name + "_lod" + std::to_string(i) + ".xmodel_export");

			if (!fs::equivalent(request.lods[i].file, target, ec))
			{
				ec.clear();
				fs::copy_file(request.lods[i].file, target, fs::copy_options::overwrite_existing, ec);

				if (ec)
				{
					error = "cannot copy " + to_utf8(request.lods[i].file) + ": " + ec.message();
					return false;
				}
			}

			def.lods.push_back({ target.filename(), request.lods[i].distance > 0.0f ? request.lods[i].distance : default_lod_distance(i, request.lods.size()) });
		}

		if (!write_text(file, write_model_text(def)))
		{
			error = "cannot write " + to_utf8(file);
			return false;
		}

		game::printf_to_console("[assets] model %s: %s ( %d LODs, collision %s )\n", name.c_str(), to_utf8(file).c_str(), static_cast<int>(def.lods.size()),
			def.collision_lod < 0 ? "none" : ("from LOD " + std::to_string(def.collision_lod)).c_str());

		rescan();

		std::string build_error;
		if (!build(build_error))
		{
			game::printf_to_console("^1[assets] %s\n", build_error.c_str());
		}

		return true;
	}

	// ---- animations, sounds, effects, scripts ---------------------------------------------------------------------

	namespace
	{
		// copies a file into the project; an existing other file of that name stays unless `overwrite`
		bool take_file(const fs::path& source, const fs::path& target, const bool overwrite, std::string& error)
		{
			std::error_code ec;

			if (!fs::is_regular_file(source, ec))
			{
				error = "no such file: " + to_utf8(source);
				return false;
			}

			if (fs::equivalent(source, target, ec))
			{
				return true;
			}

			if (fs::exists(target, ec) && !overwrite && !same_file_contents(source, target))
			{
				error = "the project already has another " + to_utf8(target.filename()) + " ( overwrite, or pick another name )";
				return false;
			}

			fs::create_directories(target.parent_path(), ec);
			ec.clear();
			fs::copy_file(source, target, fs::copy_options::overwrite_existing, ec);

			if (ec)
			{
				error = "cannot copy " + to_utf8(source) + ": " + ec.message();
				return false;
			}

			return true;
		}

		void build_after_import()
		{
			rescan();

			std::string build_error;
			if (!build(build_error))
			{
				// while a build runs this is no failure: the next one takes the new source
				game::printf_to_console("%s[assets] %s\n", g.busy ? "" : "^1", build_error.c_str());
			}
		}
	}

	std::string default_anim_tree() { return g.project; }

	bool model_files(const std::string& name, preview_files& out)
	{
		const auto known = g.sources.models.find(clean_name(name));
		if (known == g.sources.models.end() || known->second.lods.empty())
		{
			return false;
		}

		const fs::path& lod = known->second.lods[0].file;
		out.mesh = lod.is_absolute() ? lod : project_dir() / "models" / lod;
		out.materials = known->second.materials;
		return true;
	}

	bool anim_files(const std::string& name, preview_files& out)
	{
		const auto known = g.sources.anims.find(clean_name(name));
		if (known == g.sources.anims.end())
		{
			return false;
		}

		const auto& def = known->second;
		out = {};
		out.xanim = def.file.is_absolute() ? def.file : project_dir() / "anims" / def.file;
		out.loop = def.loop;

		if (!def.model.empty())
		{
			preview_files model;

			if (model_files(def.model, model))
			{
				out.model = clean_name(def.model);
				out.mesh = model.mesh;
				out.materials = model.materials;
			}
			else
			{
				out.mesh = skeleton_file(project_dir(), g.sources, def.model);
			}
		}

		return true;
	}

	std::vector<std::string> project_models()
	{
		std::vector<std::string> out;
		for (const auto& [name, def] : g.sources.models) { out.push_back(name); }
		return out;
	}

	bool import_anim(const anim_request& request, std::string& error)
	{
		if (g.project.empty())
		{
			error = "open a map first: the project is named after it";
			return false;
		}

		const std::string name = clean_name(request.name.empty() ? request.file.stem().string() : request.name);

		if (name.empty())
		{
			error = "the animation needs a name";
			return false;
		}

		const std::string type = lower(request.type.empty() ? std::string("relative") : request.type);

		if (type != "relative" && type != "delta" && type != "delta3d")
		{
			error = "type is relative, delta or delta3d";
			return false;
		}

		const fs::path dir = project_dir() / "anims";
		const fs::path file = dir / (name + ".anim");
		std::error_code ec;

		if (!request.overwrite && (fs::exists(file, ec) || g.sources.anims.count(name)))
		{
			error = "the project already has an animation " + name + " ( overwrite, or pick another name )";
			return false;
		}

		// it has to read before anything is copied
		ca::ExportAnim source;
		if (const ca::Status s = ca::load_xanim_export(request.file, source); !s)
		{
			error = s.message;
			return false;
		}

		anim_def def;
		def.name = name;
		def.file = fs::u8path(name + ".xanim_export");
		def.type = type;
		def.loop = request.loop;
		def.tree = clean_name(request.tree);

		if (def.tree == g.project) { def.tree.clear(); }

		// the skeleton: a model of the project by its name, a file of the project by its path, any other file is copied
		if (!request.model.empty())
		{
			const fs::path given = fs::u8path(request.model);

			if (g.sources.models.count(clean_name(request.model)) && !fs::is_regular_file(given, ec))
			{
				def.model = clean_name(request.model);
			}
			else
			{
				ca::ExportModel skeleton;
				if (const ca::Status s = ca::load_xmodel_export(given, skeleton); !s)
				{
					error = "the model of the animation: " + s.message;
					return false;
				}

				const fs::path copy = dir / (name + "_skeleton.xmodel_export");
				if (!take_file(given, copy, true, error))
				{
					return false;
				}

				def.model = to_utf8(copy.filename());
			}
		}

		if (!take_file(request.file, dir / def.file, true, error))
		{
			return false;
		}

		if (!write_text(file, write_anim_text(def)))
		{
			error = "cannot write " + to_utf8(file);
			return false;
		}

		game::printf_to_console("[assets] anim %s: %s ( %zu parts, %zu frames, %s%s, animtree %s, %s )\n", name.c_str(), to_utf8(file).c_str(), source.parts.size(), source.frames.size(),
			type.c_str(), def.loop ? ", looping" : "", def.tree.empty() ? g.project.c_str() : def.tree.c_str(), def.model.empty() ? "no model: every part is a root" : ("model " + def.model).c_str());

		build_after_import();
		return true;
	}

	bool import_sounds(const fs::path& csv, const fs::path& root, const bool overwrite, std::string& error)
	{
		if (g.project.empty())
		{
			error = "open a map first: the project is named after it";
			return false;
		}

		std::string text;
		if (!read_text(csv, text))
		{
			error = "cannot read " + to_utf8(csv);
			return false;
		}

		std::vector<ca::SoundAliasDef> rows;
		if (const ca::Status s = ca::parse_sound_aliases(text, rows); !s)
		{
			error = to_utf8(csv.filename()) + ": " + s.message;
			return false;
		}

		if (rows.empty())
		{
			error = to_utf8(csv.filename()) + " has no alias";
			return false;
		}

		const fs::path from = root.empty() ? ca::sound_source_root(csv) : root;
		const fs::path dir = project_dir();
		std::error_code ec;

		// what the rows name: sound files, curves, speaker maps. A file that is not there is not an error - the
		// alias then names a sound, curve or map of the game.
		std::map<fs::path, fs::path> copies;		// source -> target
		std::unordered_set<std::string> missing;

		for (const auto& row : rows)
		{
			if (!row.file.empty())
			{
				const std::string name = ca::sound_file_name(row.file);
				if (const fs::path source = from / "sound" / fs::u8path(name); fs::is_regular_file(source, ec)) { copies[source] = dir / "sound" / fs::u8path(name); }
				else { missing.insert(name); }
			}

			for (const auto& [word, ext] : { std::pair<const std::string&, const char*>{ row.volumeFalloffCurve, ".vfcurve" }, { row.speakerMap, ".spkrmap" } })
			{
				if (word.empty()) { continue; }
				if (const fs::path source = from / "soundaliases" / fs::u8path(word + ext); fs::is_regular_file(source, ec)) { copies[source] = dir / "soundaliases" / source.filename(); }
			}
		}

		if (!take_file(csv, dir / "soundaliases" / csv.filename(), overwrite, error))
		{
			return false;
		}

		for (const auto& [source, target] : copies)
		{
			if (!take_file(source, target, overwrite, error))
			{
				return false;
			}
		}

		game::printf_to_console("[assets] sounds %s: %zu rows, %zu files copied from %s%s\n", to_utf8(csv.filename()).c_str(), rows.size(), copies.size(), to_utf8(from).c_str(),
			missing.empty() ? "" : ( ", " + std::to_string(missing.size()) + " sound file(s) not found there: named only ( sounds of the game )" ).c_str());

		build_after_import();
		return true;
	}

	bool import_fx(const fs::path& file, const std::string& wanted, const bool overwrite, std::string& error)
	{
		if (g.project.empty())
		{
			error = "open a map first: the project is named after it";
			return false;
		}

		ca::EditorEffect effect;
		if (const ca::Status s = ca::load_efx(file, effect); !s)
		{
			error = s.message;
			return false;
		}

		// the name loadfx takes: what follows the fx folder in the path of the file
		std::string name = clean_name(wanted, true);

		if (name.empty())
		{
			std::vector<std::string> parts;
			for (const auto& part : file.parent_path()) { parts.push_back(lower(to_utf8(part))); }

			const auto fx = std::find(parts.rbegin(), parts.rend(), "fx");
			std::string folder;

			for (auto it = fx == parts.rend() ? parts.end() : fx.base(); it != parts.end(); ++it)
			{
				folder += clean_name(*it) + "/";
			}

			name = (folder.empty() ? g.project + "/" : folder) + clean_name(file.stem().string());
		}

		while (!name.empty() && name.front() == '/') { name.erase(name.begin()); }

		if (name.empty() || name.back() == '/')
		{
			error = "the effect needs a name ( folder/name, as loadfx takes it )";
			return false;
		}

		const fs::path target = (project_dir() / "fx" / fs::u8path(name + ".efx")).make_preferred();
		std::error_code ec;

		if (!overwrite && (g.sources.effects.count(name) || fs::exists(target, ec)) && !same_file_contents(file, target))
		{
			error = "the project already has an effect " + name + " ( overwrite, or pick another name )";
			return false;
		}

		if (!take_file(file, target, true, error))
		{
			return false;
		}

		game::printf_to_console("[assets] fx %s: %s ( loadfx( \"%s\" ); effects it emits are read from fx\\<name>.efx of the project )\n", name.c_str(), to_utf8(target).c_str(), name.c_str());

		build_after_import();
		return true;
	}

	bool level_script(fs::path& file, std::string& error)
	{
		auto& s = get_scene();

		if (g.project.empty() || !s.open)
		{
			error = "open a map first: the level script is maps\\mp\\<map>.gsc";
			return false;
		}

		const std::string map = s.map_file.empty() ? s.map_name : to_utf8(s.map_file.stem());
		file = project_dir() / "scripts" / "maps" / "mp" / fs::u8path(map + ".gsc");
		std::error_code ec;

		if (fs::exists(file, ec))
		{
			return true;
		}

		// what codol_bsp generates ( compile.cpp write_scripts ), to start from
		const std::string text = "// the level script of " + map + ". File > Build map takes this file in front of the one the compiler generates\n"
			"// ( mod\\" + g.project + "\\scripts\\ is laid over the generated scripts; the two effect scripts stay generated ).\n"
			"// Animations of the project: #using_animtree( \"" + g.project + "\" ); in front of main(), then %<animation>.\n"
			"#include maps\\mp\\_utility;\n\nmain()\n{\n\tmaps\\mp\\" + map + "_fx::main();\n\n\tmaps\\mp\\_load::main();\n\n"
			"\tmaps\\mp\\_compass::setupMiniMap( \"compass_map_" + map + "\" );\n\n\tgame[ \"attackers\" ] = \"allies\";\n\tgame[ \"defenders\" ] = \"axis\";\n}\n";

		if (!write_text(file, text))
		{
			error = "cannot write " + to_utf8(file);
			return false;
		}

		game::printf_to_console("[assets] level script: %s\n", to_utf8(file).c_str());
		rescan();
		return true;
	}

	// ---- GDT -------------------------------------------------------------------------------------------------

	bool read_gdt(const fs::path& gdt, const fs::path& root, std::vector<gdt_entry>& out, std::string& error)
	{
		out.clear();

		std::error_code absolute_error;
		const fs::path absolute = fs::absolute(gdt, absolute_error);
		const fs::path file = absolute_error ? gdt : absolute;

		std::string text;
		if (!read_text(file, text))
		{
			error = "cannot read " + to_utf8(file);
			return false;
		}

		std::vector<ca::MaterialDef> defs;
		if (const ca::Status s = ca::parse_gdt_materials(text, defs); !s)
		{
			error = s.message;
			return false;
		}

		// image paths of a GDT are relative to the root of the mod tools ( the GDT is in source_data\ below it )
		std::vector<fs::path> roots;
		if (!root.empty())
		{
			roots.push_back(root);
		}

		fs::path up = file.parent_path();
		for (int i = 0; i < 3 && !up.empty(); i++)
		{
			roots.push_back(up);
			if (up == up.parent_path()) { break; }
			up = up.parent_path();
		}

		for (auto& def : defs)
		{
			gdt_entry entry;
			def.name = clean_name(def.name, true);

			// CoD4 names world materials without a prefix; here they are wc/<name>, model materials mc/<name>
			if (def.name.find('/') == std::string::npos)
			{
				if (def.type.rfind("world", 0) == 0) { def.name = "wc/" + def.name; }
				else if (def.type.rfind("model", 0) == 0) { def.name = "mc/" + def.name; }
			}

			entry.name = def.name;
			entry.type = def.type;

			for (auto& texture : def.textures)
			{
				texture.image = clean_name(texture.image);
				const auto source = def.imageFiles.find(texture.sampler);

				if (source == def.imageFiles.end())
				{
					continue;
				}

				std::string path = source->second;
				for (char& c : path)
				{
					if (c == '\\') { c = '/'; }
				}

				fs::path found;
				std::error_code ec;

				for (const auto& base : roots)
				{
					if (const fs::path candidate = base / fs::u8path(path); fs::is_regular_file(candidate, ec))
					{
						found = candidate;
						break;
					}
				}

				entry.images.emplace_back(texture.sampler, source->second);
				entry.found.push_back(found);
			}

			entry.text = ca::write_material_text(def);
			out.push_back(std::move(entry));
		}

		return true;
	}

	int convert_gdt(const std::vector<gdt_entry>& entries, const bool overwrite, std::vector<std::string>& log)
	{
		int converted = 0;

		if (g.project.empty())
		{
			log.push_back("open a map first: the project is named after it");
			return 0;
		}

		for (const auto& entry : entries)
		{
			const fs::path file = project_dir() / "materials" / material_file_name(entry.name);
			std::error_code ec;
			std::string problem;

			if (!overwrite && (fs::exists(file, ec) || g.sources.materials.count(entry.name)))
			{
				problem = "the project already has this material";
			}

			for (std::size_t i = 0; problem.empty() && i < entry.images.size(); i++)
			{
				if (entry.found[i].empty())
				{
					problem = entry.images[i].first + " " + entry.images[i].second + " was not found ( set the folder the image paths start from )";
				}
			}

			for (std::size_t i = 0; problem.empty() && i < entry.images.size(); i++)
			{
				std::string name;
				take_image(entry.found[i], overwrite, name, problem);
			}

			if (problem.empty() && !write_text(file, "// converted from a GDT by CoDOL Radiant\n" + entry.text))
			{
				problem = "cannot write " + to_utf8(file);
			}

			if (problem.empty())
			{
				converted++;
				log.push_back(entry.name + ": " + to_utf8(file.filename()));
			}
			else
			{
				log.push_back(entry.name + ": NOT converted: " + problem);
			}
		}

		for (const auto& line : log)
		{
			game::printf_to_console("[assets] gdt: %s\n", line.c_str());
		}

		rescan();

		if (converted)
		{
			std::string error;
			if (!build(error))
			{
				game::printf_to_console("^1[assets] %s\n", error.c_str());
			}
		}

		return converted;
	}

	// ---- build -----------------------------------------------------------------------------------------------

	bool busy() { return g.busy; }

	const std::string& build_step() { return g.step; }

	const std::vector<std::string>& build_log() { return g.log; }

	bool build(std::string& error)
	{
		if (g.project.empty())
		{
			error = "open a map first: the project is named after it";
			return false;
		}

		if (g.busy)
		{
			// what was added meanwhile is built right after
			g.rebuild_wanted = true;
			error = "a build is running: another one follows it";
			return false;
		}

		const std::string install = components::codol_game::install_path();
		if (install.empty())
		{
			error = "set the CoDOL install folder in File > Settings";
			return false;
		}

		rescan();

		bool anything = false;
		for (const auto& entry : g.items)
		{
			anything |= entry.type != kind::image && entry.type != kind::script;
		}

		if (!anything)
		{
			error = "the project has nothing to build yet ( no material, model, animation, sound alias file or effect )";
			return false;
		}

		build_job job;
		job.dir = project_dir();
		job.install = fs::u8path(install);
		job.exe = job.install / "codoMP_client_shipRetail.exe";
		job.matlib = mod_dir() / "codol_matlib.txt";
		job.fxatlas = mod_dir() / "codol_fxatlas.txt";
		job.out = zone_file();
		job.zone = zone_name();
		job.project = g.project;

		std::error_code ec;
		if (!fs::is_regular_file(job.exe, ec))
		{
			job.exe = job.install / "codoMP_client_shipRetail.exe";
		}

		// what a model may name: the materials of the open map's zones, without what the project made before
		if (auto& s = get_scene(); s.renderer.loaded())
		{
			job.have_shipped = true;

			for (const auto& name : s.renderer.material_names())
			{
				if (!g.sources.materials.count(name))
				{
					job.shipped_materials.insert(name);
				}
			}
		}

		// a material the project no longer defines but its old zone still has is not a shipped one either
		{
			std::string text;
			if (read_text(job.dir / "build_state.txt", text))
			{
				for (const auto& line : utils::split(text, '\n'))
				{
					if (line.rfind("material\t", 0) == 0)
					{
						job.shipped_materials.erase(line.substr(9, line.find('\t', 9) - 9));
					}
				}
			}
		}

		g.busy = true;
		g.step = "starting ..";
		g.log.clear();
		game::printf_to_console("[assets] building %s ..\n", job.zone.c_str());

		const std::string project_name = g.project;

		std::thread([job, project_name]
		{
			build_result result;

			try
			{
				result = run_build(job);
			}
			catch (const std::exception& e)
			{
				result.error = std::string("exception: ") + e.what();
			}

			components::codol_game::post_to_gui([result = std::move(result), dir = job.dir, project_name]() mutable
			{
				build_finished(std::move(result), dir, project_name);
			});
		}).detach();

		return true;
	}

	// ---- dropped files ---------------------------------------------------------------------------------------

	void files_dropped(const std::vector<fs::path>& files)
	{
		std::vector<fs::path> images, exports;
		int copied = 0;

		for (const auto& file : files)
		{
			const std::string ext = lower(file.extension().string());

			if (is_image_file(file))
			{
				images.push_back(file);
			}
			else if (ext == ".xmodel_export")
			{
				exports.push_back(file);
			}
			else if (ext == ".gdt")
			{
				open_gdt_dialog(file);
			}
			else if (ext == ".xanim_export")
			{
				open_anim_dialog(file);
			}
			else if (ext == ".csv" || ext == ".efx")
			{
				std::string error;
				if (!(ext == ".csv" ? import_sounds(file, {}, false, error) : import_fx(file, {}, false, error)))
				{
					game::printf_to_console("^1[assets] %s: %s\n", to_utf8(file.filename()).c_str(), error.c_str());
					ImGui::Toast(ImGuiToastType_Error, "Assets", error.c_str(), 8000);
				}
			}
			else if (ext == ".mtl" && !g.project.empty())
			{
				std::error_code ec;
				const fs::path target = project_dir() / "materials" / file.filename();
				fs::create_directories(target.parent_path(), ec);

				if (fs::exists(target, ec))
				{
					game::printf_to_console("^1[assets] the project already has %s\n", to_utf8(file.filename()).c_str());
				}
				else if (fs::copy_file(file, target, ec))
				{
					copied++;
				}
			}
			else
			{
				game::printf_to_console("[assets] %s: not an image ( png, tga, dds ), .xmodel_export, .xanim_export, soundaliases .csv, .efx, .gdt or .mtl\n",
					to_utf8(file.filename()).c_str());
			}
		}

		if (!images.empty())
		{
			open_material_dialog(images);
		}

		if (!exports.empty())
		{
			open_model_dialog(exports);
		}

		if (copied)
		{
			rescan();
			std::string error;
			build(error);
		}
	}

	// ---- the map compile -------------------------------------------------------------------------------------

	bool undrawn_in_game(const std::string& material)
	{
		// a map of your own ( drawn in the stage ) is built whole by File > Build map: every material goes into its zone
		if (material.rfind("wc/", 0) != 0 || !get_scene().renderer.loaded() || get_scene().stage)
		{
			return false;
		}

		return !world_materials().count(material);
	}

	std::string package_arguments()
	{
		std::error_code ec;
		return !g.project.empty() && fs::is_directory(project_dir(), ec) ? " -Project \"" + to_utf8(project_dir()) + "\"" : std::string();
	}

	std::vector<std::string> package_problems()
	{
		std::vector<std::string> lines;

		if (g.project.empty())
		{
			return lines;
		}

		if (!g.busy)
		{
			rescan();
		}

		int waiting = 0, failed = 0;
		std::string first;

		for (const auto& entry : g.items)
		{
			const bool late = entry.state == status::fresh || entry.state == status::changed;

			if (late || entry.state == status::failed)
			{
				(late ? waiting : failed)++;
				if (first.empty()) { first = std::string(kind_word(entry.type)) + " " + entry.name + " ( " + status_word(entry.state) + " )"; }
			}
		}

		if (g.busy)
		{
			lines.push_back("the assets of " + g.project + " are being built right now: the map takes the zone as it is on disk");
		}

		if (waiting || failed)
		{
			lines.push_back(std::to_string(waiting + failed) + " source(s) of the asset project " + g.project + " are not in its zone ( " + std::to_string(waiting) + " new or changed, "
				+ std::to_string(failed) + " failed; first: " + first + " ): the map gets what the last asset build made - Assets > Build first");
		}

		return lines;
	}

	// What codol_mapc gets of the project for an edited shipped map - the same things File > Build map puts into a
	// map of one's own ( package_map.ps1 ):
	//   --assets <zone>                           animations, sounds, effects, models, materials are carried into the
	//                                             map zone ( the zone is a donor for entity models as well )
	//   --append-rawfile animlists/<map>.animlist the project's animations behind the map's own list
	//   --rawfile animtrees/<tree>.atr            the animtrees of the project's animations
	//   --rawfile <path>                          every file of the project's scripts\, in front of the above
	std::string compile_arguments()
	{
		if (!zone_exists())
		{
			return {};
		}

		std::error_code ec;
		const fs::path dir = project_dir();
		const std::string map = get_scene().map_name;
		std::string out = " --assets \"" + to_utf8(zone_file()) + "\"";
		std::map<std::string, fs::path> raw;		// rawfile name ( lower case ) -> file; the project's scripts win

		for (const auto& entry : fs::directory_iterator(dir / "generated" / "animtrees", ec))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".atr")
			{
				raw["animtrees/" + lower(to_utf8(entry.path().filename()))] = entry.path();
			}
		}

		bool own_animlist = false;

		for (const auto& entry : fs::recursive_directory_iterator(dir / "scripts", ec))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}

			std::string name = lower(to_utf8(fs::relative(entry.path(), dir / "scripts", ec)));
			std::replace(name.begin(), name.end(), '\\', '/');

			if (name.find_first_of("=\"") != std::string::npos)
			{
				continue;
			}

			own_animlist |= name == "animlists/" + lower(map) + ".animlist";
			raw[name] = entry.path();
		}

		if (const fs::path list = dir / "generated" / "animlists" / "map.animlist"; !own_animlist && !map.empty() && fs::is_regular_file(list, ec))
		{
			out += " --append-rawfile \"animlists/" + map + ".animlist=" + to_utf8(list) + "\"";
		}

		for (const auto& [name, file] : raw)
		{
			out += " --rawfile \"" + name + "=" + to_utf8(file) + "\"";
		}

		return out;
	}

	// After a compile of an edited shipped map: what of the project is not inside the fastfile and where it goes.
	// The project's scripts are copied to compiled\<map>.scripts\ ( the dedicated server reads level scripts from its
	// script folder, never from a zone ); the streamed sounds need their archive in main\.
	std::vector<std::string> compile_extras(const fs::path& out_dir, const std::string& map)
	{
		std::vector<std::string> lines;
		std::error_code ec;

		if (g.project.empty() || !zone_exists())
		{
			return lines;
		}

		const fs::path dir = project_dir();
		const fs::path scripts_to = out_dir / fs::u8path(map + ".scripts");
		int copied = 0;

		for (const auto& entry : fs::recursive_directory_iterator(dir / "scripts", ec))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}

			const fs::path to = scripts_to / fs::relative(entry.path(), dir / "scripts", ec);
			fs::create_directories(to.parent_path(), ec);
			fs::copy_file(entry.path(), to, fs::copy_options::overwrite_existing, ec);
			copied += !ec;
		}

		lines.push_back("the project's assets ( " + to_utf8(zone_file().filename()) + " ) were carried into the map zone: animations, sounds, effects, models, materials");

		if (copied)
		{
			lines.push_back(std::to_string(copied) + " script file( s ) of the project: compiled\\" + map + ".scripts\\ - copy them into the server's script folder ( a level script maps\\mp\\"
				+ map + ".gsc there replaces the map's own: it has to hold what that one does )");
		}

		for (const auto& entry : fs::directory_iterator(dir, ec))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".iwd")
			{
				lines.push_back("streamed sounds: " + to_utf8(entry.path().filename()) + " has to become an archive of the game, for client and server: codol_assetc --install <game> --iwd <game>\\main\\iw_sound_23.iwd --iwd-add \""
					+ to_utf8(entry.path()) + "\"  ( a free number from 04 to 23 )");
			}
		}

		return lines;
	}

	const std::vector<std::string>& compile_problems() { return g.compile_problems; }

	namespace
	{
		// project models the entities of the map use: name -> ( static instances, other entities )
		std::map<std::string, std::pair<int, int>> used_project_models()
		{
			std::map<std::string, std::pair<int, int>> out;
			auto& doc = get_scene().doc;

			for (const object_id id : doc.entities())
			{
				const auto* e = doc.find_entity(id);
				const std::string model(e->value("model"));

				if (model.empty() || !g.sources.models.count(model))
				{
					continue;
				}

				const bool is_static = e->classname() == "misc_model" && e->find("codol_static");
				auto& count = out[model];
				(is_static ? count.first : count.second)++;
			}

			return out;
		}
	}

	std::vector<std::string> compile_precheck()
	{
		std::vector<std::string> lines;
		auto& s = get_scene();
		auto& doc = s.doc;

		if (g.project.empty() || !s.open)
		{
			return lines;
		}

		rescan();

		// ---- faces of new and edited world brushes whose material the world of the base map does not use
		std::map<std::string, std::pair<int, int>> undrawn;		// material -> faces, brushes
		// the words codol_mapc knows a tool material by ( codol_mapc diff.cpp is_tool_material ): no render face
		static const char* const tools[] = { "clip", "hint", "mantle", "nosight", "portal", "ladder", "trigger", "caulk", "nodraw", "skip", "lightgrid", "aitrig" };

		if (const auto* world = doc.find_entity(doc.worldspawn()))
		{
			for (const object_id child : world->children)
			{
				const auto* brush = doc.find_brush(child);

				if (!brush || brush->opaque || s.imported(child))
				{
					continue;
				}

				std::unordered_set<std::string> seen;

				for (const auto& face : brush->faces)
				{
					const std::string& material = face.texture.material;
					const std::string name = material.rfind("wc/", 0) == 0 ? material : "wc/" + material;
					const bool tool = std::any_of(std::begin(tools), std::end(tools), [&](const char* word) { return material.find(word) != std::string::npos; });

					if (tool || world_materials().count(name))
					{
						continue;
					}

					auto& count = undrawn[name];
					count.first++;
					count.second += seen.insert(name).second ? 1 : 0;
				}
			}
		}

		for (const auto& [material, count] : undrawn)
		{
			const bool ours = g.sources.materials.count(material) != 0;
			lines.push_back(std::to_string(count.first) + " face(s) of " + std::to_string(count.second) + " new / edited brush(es) with " + material
				+ " will NOT be drawn in the game ( they collide ): " + (ours ? "a material of the project is not part of the map zone - " : "")
				+ "the incremental compiler only draws materials that the world of " + s.map_name + " already uses");
		}

		// ---- models of the project
		for (const auto& [model, count] : used_project_models())
		{
			const item* entry = find(kind::model, model);
			const std::string users = std::to_string(count.first + count.second) + " entit" + (count.first + count.second == 1 ? "y" : "ies");

			if (!entry || entry->state != status::built || !zone_exists())
			{
				lines.push_back(users + " with model " + model + " will NOT have a model in the game: the model is not built ( " + (entry ? status_word(entry->state) : "missing")
					+ (entry && !entry->message.empty() ? ": " + entry->message : "") + " ) - build the assets first");
			}
		}

		return lines;
	}

	void compile_finished(const bool ok, const std::string& compiler_output)
	{
		std::vector<std::string> problems = g.compile_problems;		// the precheck of this compile

		if (ok)
		{
			// what the compiler says will be missing
			for (const auto& raw : utils::split(compiler_output, '\n'))
			{
				std::string line = raw;
				while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) { line.pop_back(); }

				const auto warning = line.find("warning: ");
				if (warning == std::string::npos)
				{
					continue;
				}

				const std::string text = line.substr(warning + 9);
				const bool about_missing = text.find("not drawn") != std::string::npos || text.find("left out") != std::string::npos
					|| text.find("could not be taken") != std::string::npos || text.find("no model in the game") != std::string::npos
					|| text.find("donor zone") != std::string::npos;

				// what the precheck already said about a material is not said twice
				bool said = false;
				if (const auto quote = text.find("uses material '"); quote != std::string::npos)
				{
					const std::string material = text.substr(quote + 15, text.find('\'', quote + 15) - quote - 15);
					said = std::any_of(problems.begin(), problems.end(), [&](const std::string& line) { return line.find(" with " + material + " will NOT") != std::string::npos; });
				}

				if (about_missing && !said)
				{
					problems.push_back("codol_mapc: " + text);
				}
			}

			// models of the project that the compiler did not take into the map zone
			for (const auto& [model, count] : used_project_models())
			{
				if (compiler_output.find("xmodel '" + model + "' taken from zone") != std::string::npos
					|| compiler_output.find("'" + model + "' could not be taken") != std::string::npos)
				{
					continue;
				}

				const item* entry = find(kind::model, model);
				if (entry && entry->state == status::built && zone_exists())
				{
					problems.push_back("model " + model + " ( " + std::to_string(count.first + count.second)
						+ " entities ) was NOT copied into the map zone: the compiler did not report taking it ( it copies a model only when the map zone does not have it already )");
				}
			}
		}

		g.compile_problems = problems;

		if (problems.empty())
		{
			return;
		}

		game::printf_to_console("^1[CoDOL] compile: %d thing(s) of the map will not appear in the game as they do in the editor:\n", static_cast<int>(problems.size()));

		std::string toast;
		for (std::size_t i = 0; i < problems.size(); i++)
		{
			game::printf_to_console("^1  %s\n", problems[i].c_str());

			if (i < 3)
			{
				toast += (toast.empty() ? "" : "\n") + problems[i].substr(0, 150) + (problems[i].size() > 150 ? " .." : "");
			}
		}

		if (problems.size() > 3)
		{
			toast += "\n.. and " + std::to_string(problems.size() - 3) + " more: see the console or the Assets window";
		}

		ImGui::Toast(ImGuiToastType_Warning, "Not in the game", toast.c_str(), 12000);
	}

	void begin_compile()
	{
		g.compile_problems = compile_precheck();
	}

	// ---- per frame -------------------------------------------------------------------------------------------

	void frame()
	{
		auto& s = get_scene();

		// the project follows the map ( scene::load_renderer calls map_opening for a map that is being opened )
		if (!s.open && !g.project.empty() && !g.busy)
		{
			drop_from_renderer();
			g.project.clear();
			g.project_map.clear();
			g.items.clear();
			g.sources = {};
			g.compile_problems.clear();
			close_dialogs();
		}
		else if (s.open && !g.busy && g.project_map != s.map_name)
		{
			// a map was opened while a build of another project ran: its project is chosen now
			g.project_map = s.map_name;
			set_project({});
		}

		draw_windows();
	}

	// ---- console commands ------------------------------------------------------------------------------------

	void register_commands()
	{
		using args_t = const std::vector<std::string>&;

		const auto option = [](args_t args, const std::string& key, std::string& value)
		{
			for (std::size_t i = 1; i < args.size(); i++)
			{
				if (args[i].rfind(key + "=", 0) == 0)
				{
					value = args[i].substr(key.size() + 1);
					return true;
				}
			}

			return false;
		};

		const auto flag = [](args_t args, const char* word)
		{
			return std::find(args.begin() + 1, args.end(), word) != args.end();
		};

		// asset_project [name]: the project ( folder mod\<name> ); without a name: which one it is
		command::register_command("asset_project", [](args_t args)
		{
			if (args.size() > 1)
			{
				if (!get_scene().open) { game::printf_to_console("^1no map is open\n"); return; }
				set_project(args[1] == "default" ? std::string() : args[1]);
			}

			if (g.project.empty())
			{
				game::printf_to_console("no project: open a map ( its project is mod\\<map> next to the editor )\n");
				return;
			}

			game::printf_to_console("project %s: %s; zone %s%s\n", g.project.c_str(), to_utf8(project_dir()).c_str(), to_utf8(zone_file().filename()).c_str(),
				zone_exists() ? "" : " ( not built )");
		});

		command::register_command("asset_list", [](args_t)
		{
			rescan();
			game::printf_to_console("project %s: %d sources, zone %s, %s\n", g.project.c_str(), static_cast<int>(g.items.size()), zone_exists() ? "built" : "not built",
				g.busy ? ("building: " + g.step).c_str() : "idle");

			for (const auto& entry : g.items)
			{
				game::printf_to_console("%s  %-8s %-34s %-8s %s\n", entry.state == status::failed ? "^1" : "", kind_word(entry.type), entry.name.c_str(), status_word(entry.state),
					entry.state == status::failed ? entry.message.c_str() : entry.detail.c_str());

				for (const auto& note : entry.notes)
				{
					game::printf_to_console("             note: %s\n", note.c_str());
				}

				if (entry.type == kind::material && entry.state != status::failed && undrawn_in_game(entry.name))
				{
					game::printf_to_console("             not in the game: new faces with it collide but are not drawn ( the incremental compiler only draws materials the world of %s already uses )\n",
						get_scene().map_name.c_str());
				}
			}
		});

		command::register_command("asset_rescan", [](args_t) { rescan(); game::printf_to_console("%d sources\n", static_cast<int>(g.items.size())); });

		command::register_command("asset_build", [](args_t)
		{
			std::string error;
			if (!build(error)) { game::printf_to_console("^1[assets] %s\n", error.c_str()); }
		});

		command::register_command("asset_status", [](args_t)
		{
			game::printf_to_console("assets: %s; zone %s\n", g.busy ? ("building: " + g.step).c_str() : "idle", zone_exists() ? "built" : "not built");
		});

		// asset_new_material <name> <kind> <colour map> [normal=<file>] [specular=<file>] [surface=<type>] [overwrite]
		command::register_command("asset_new_material", [option, flag](args_t args)
		{
			if (args.size() < 4)
			{
				std::string kinds;
				for (const auto& k : material_kinds()) { kinds += std::string(kinds.empty() ? "" : " | ") + k.id; }
				game::printf_to_console("usage: asset_new_material <name> <kind> <colour map file> [normal=<file>] [specular=<file>] [surface=<type>] [overwrite]\n  kinds: %s\n", kinds.c_str());
				return;
			}

			material_request request;
			request.name = args[1];
			request.kind = args[2];
			request.color = fs::u8path(args[3]);
			std::string value;
			if (option(args, "normal", value)) { request.normal = fs::u8path(value); }
			if (option(args, "specular", value)) { request.specular = fs::u8path(value); }
			option(args, "surface", request.surface_type);
			request.overwrite = flag(args, "overwrite");

			std::string error, made;
			if (!new_material(request, error, &made)) { game::printf_to_console("^1[assets] %s\n", error.c_str()); }
		});

		// asset_import_model <name> <lod0>[@dist] [<lod1>[@dist] ..] [collision=<lod>] [mtl:<export material>=<material>] [overwrite]
		command::register_command("asset_import_model", [option, flag](args_t args)
		{
			if (args.size() < 3)
			{
				game::printf_to_console("usage: asset_import_model <name> <lod0.xmodel_export>[@dist] [<lod1>[@dist] ..] [collision=<lod>] [mtl:<export material>=<material>] [overwrite]\n");
				return;
			}

			model_request request;
			request.name = args[1];
			request.overwrite = flag(args, "overwrite");
			std::string value;
			if (option(args, "collision", value)) { request.collision_lod = value == "none" ? -1 : std::atoi(value.c_str()); }

			for (std::size_t i = 2; i < args.size(); i++)
			{
				const std::string& a = args[i];

				if (a == "overwrite" || a.rfind("collision=", 0) == 0)
				{
					continue;
				}

				if (a.rfind("mtl:", 0) == 0 && a.find('=') != std::string::npos)
				{
					request.materials[a.substr(4, a.find('=') - 4)] = a.substr(a.find('=') + 1);
					continue;
				}

				model_lod lod;
				lod.distance = 0.0f;
				std::string file = a;

				if (const auto at = a.rfind('@'); at != std::string::npos)
				{
					lod.distance = static_cast<float>(std::atof(a.c_str() + at + 1));
					file = a.substr(0, at);
				}

				lod.file = fs::u8path(file);
				request.lods.push_back(lod);
			}

			std::string error;
			if (!import_model(request, error)) { game::printf_to_console("^1[assets] %s\n", error.c_str()); }
		});

		// asset_import_anim <file.xanim_export> [name=<name>] [model=<project model | file.xmodel_export>] [type=relative|delta|delta3d] [loop] [tree=<animtree>] [overwrite]
		command::register_command("asset_import_anim", [option, flag](args_t args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: asset_import_anim <file.xanim_export> [name=<name>] [model=<model of the project | file.xmodel_export>] [type=relative|delta|delta3d] [loop] [tree=<animtree>] [overwrite]\n");
				return;
			}

			anim_request request;
			request.file = fs::u8path(args[1]);
			option(args, "name", request.name);
			option(args, "model", request.model);
			option(args, "type", request.type);
			option(args, "tree", request.tree);
			request.loop = flag(args, "loop");
			request.overwrite = flag(args, "overwrite");

			std::string error;
			if (!import_anim(request, error)) { game::printf_to_console("^1[assets] %s\n", error.c_str()); }
		});

		// asset_import_sounds <file.csv> [root=<folder with sound\ and soundaliases\>] [overwrite]
		command::register_command("asset_import_sounds", [option, flag](args_t args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: asset_import_sounds <soundaliases file.csv> [root=<folder that holds sound\\ and soundaliases\\>] [overwrite]\n");
				return;
			}

			std::string root, error;
			option(args, "root", root);
			if (!import_sounds(fs::u8path(args[1]), fs::u8path(root), flag(args, "overwrite"), error)) { game::printf_to_console("^1[assets] %s\n", error.c_str()); }
		});

		// asset_import_fx <file.efx> [name=<folder/name>] [overwrite]
		command::register_command("asset_import_fx", [option, flag](args_t args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: asset_import_fx <file.efx> [name=<folder/name as loadfx takes it>] [overwrite]\n");
				return;
			}

			std::string name, error;
			option(args, "name", name);
			if (!import_fx(fs::u8path(args[1]), name, flag(args, "overwrite"), error)) { game::printf_to_console("^1[assets] %s\n", error.c_str()); }
		});

		// asset_level_script [<file.gsc>]: scripts\maps\mp\<map>.gsc of the project - a copy of the file, or a start
		command::register_command("asset_level_script", [](args_t args)
		{
			fs::path file;
			std::string error;
			bool ok = true;

			if (args.size() > 1)
			{
				auto& s = get_scene();
				if (g.project.empty() || !s.open) { game::printf_to_console("^1no map is open\n"); return; }

				file = project_dir() / "scripts" / "maps" / "mp" / fs::u8path((s.map_file.empty() ? s.map_name : to_utf8(s.map_file.stem())) + ".gsc");
				ok = take_file(fs::u8path(args[1]), file, true, error);
				rescan();
			}
			else
			{
				ok = level_script(file, error);
			}

			if (!ok) { game::printf_to_console("^1[assets] %s\n", error.c_str()); return; }
			game::printf_to_console("level script: %s ( File > Build map takes it in front of the generated one )\n", to_utf8(file).c_str());
		});

		// asset_gdt <file.gdt> [root=<folder of the image paths>] [overwrite] [<entry> .. | all]: without entries: the list
		command::register_command("asset_gdt", [option, flag](args_t args)
		{
			if (args.size() < 2)
			{
				game::printf_to_console("usage: asset_gdt <file.gdt> [root=<folder the image paths start from>] [overwrite] [all | <material> ..]\n");
				return;
			}

			std::string root, error;
			option(args, "root", root);
			std::vector<gdt_entry> entries;

			if (!read_gdt(fs::u8path(args[1]), fs::u8path(root), entries, error))
			{
				game::printf_to_console("^1[assets] %s\n", error.c_str());
				return;
			}

			std::vector<gdt_entry> chosen;
			bool any_choice = false;

			for (std::size_t i = 2; i < args.size(); i++)
			{
				if (args[i] == "overwrite" || args[i].rfind("root=", 0) == 0)
				{
					continue;
				}

				any_choice = true;

				for (const auto& entry : entries)
				{
					if (args[i] == "all" || entry.name == args[i] || entry.name == "wc/" + args[i] || entry.name == "mc/" + args[i])
					{
						chosen.push_back(entry);
					}
				}
			}

			if (!any_choice)
			{
				game::printf_to_console("%s: %d material entries\n", args[1].c_str(), static_cast<int>(entries.size()));

				for (const auto& entry : entries)
				{
					std::string images;
					for (std::size_t i = 0; i < entry.images.size(); i++)
					{
						images += " " + entry.images[i].first + "=" + entry.images[i].second + (entry.found[i].empty() ? " ( NOT FOUND )" : "");
					}

					game::printf_to_console("  %-34s %-14s%s\n", entry.name.c_str(), entry.type.c_str(), images.c_str());
				}

				return;
			}

			std::vector<std::string> log;
			const int converted = convert_gdt(chosen, flag(args, "overwrite"), log);
			game::printf_to_console("%d of %d entries converted\n", converted, static_cast<int>(chosen.size()));
		});

		// asset_drop <file> ..: what dropping the files on the window does
		command::register_command("asset_drop", [](args_t args)
		{
			std::vector<fs::path> files;
			for (std::size_t i = 1; i < args.size(); i++) { files.push_back(fs::u8path(args[i])); }
			files_dropped(files);
		});

		// asset_dialog <material | model | anim | sounds | fx | gdt | close> [file]: the windows that make assets
		command::register_command("asset_dialog", [](args_t args)
		{
			const std::string which = args.size() > 1 ? args[1] : "";
			if (which == "material") { open_material_dialog(); }
			else if (which == "model") { open_model_dialog(); }
			else if (which == "anim") { open_anim_dialog(args.size() > 2 ? fs::u8path(args[2]) : fs::path()); }
			else if (which == "gdt") { open_gdt_dialog(args.size() > 2 ? fs::u8path(args[2]) : fs::path()); }
			else if (which == "sounds") { open_sounds_dialog(args.size() > 2 ? fs::u8path(args[2]) : fs::path()); }
			else if (which == "fx") { open_fx_dialog(args.size() > 2 ? fs::u8path(args[2]) : fs::path()); }
			else { close_dialogs(); }
		});

		// asset_verify: codol_assetc --verify on the project's zone ( the game's loaders read it, it is written again )
		command::register_command("asset_verify", [](args_t)
		{
			if (!zone_exists()) { game::printf_to_console("^1[assets] the project has no zone yet\n"); return; }

			const fs::path base = fs::u8path(utils::fs::get_basepath());
			const std::string line = "\"" + to_utf8(base / "codol_assetc.exe") + "\" --verify \"" + to_utf8(zone_file()) + "\" --install \"" + components::codol_game::install_path() + "\"";

			std::thread([line, base]
			{
				std::string output;
				const int code = components::codol_game::run_command(line, to_utf8(base), output);

				components::codol_game::post_to_gui([code, output]
				{
					game::printf_to_console("%s\n%s[assets] verify: exit code %d\n", output.c_str(), code ? "^1" : "", code);
				});
			}).detach();
		});
	}
}
