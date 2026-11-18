// codol_assetc: source assets -> a fast file with native 1.1.1.13 assets. Asset Manager + converter + linker_pc.
//
//   codol_assetc --install <dir> --out <zone.ff> [options] [assets]
//     --exe <client exe>             default <install>/codoMP_client_shipRetail.exe
//     --base <zone.ff>               add to an existing zone instead of making a new one
//     --name <zone name>             name of a new zone ( default: the output file name )
//     --matlib <file>                material library cache ( default <out dir>/codol_matlib.txt; made on first use, 20 s )
//     --copy-techsets                copy every technique set into the zone instead of naming those of the common zones
//     --source-root <dir>            where the image files of GDT materials are looked for
//     --iwi-dir <dir>                write images/<name>.iwi of the streamed images there
//     --iwd <file.iwd>               .. and / or pack them into a plain .iwd ( name it without iw_image, see README )
//     --zone-out <file>              also write the inflated zone
//
//     --image <name>=<file>[,use=color|normal|specular|detail|ui|function][,format=dxt1|dxt3|dxt5|rgba|rgb|la|l|a]
//                                 [,streamed][,nomips][,nopicmip][,clamp][,flipy]
//     --material <file.mtl>          every material of the file
//     --material gdt:<file.gdt>[:<entry>]   the material entries of a GDT ( one, or all ); their images are compiled too
//     --xmodel <name>=<lod0.xmodel_export>[@dist][,<lod1>[@dist] ..][,collision=<lod>][,contents=<n>][,mtl:<export name>=<material>]
//                                 [,hitloc:<bone>=<class>]
//     --xanim <name>=<file.xanim_export>[,model=<file.xmodel_export>][,type=relative|delta|delta3d][,loop][,deltapart=<part>]
//                                 [,assettype=<n>][,rottol=<s16 units>][,transtol=<units>]
//     --rawfile <name>=<file>
//     --soundalias <file.csv>        sound aliases; loaded .wav files, curves and speaker maps from the tree around it
//     --sound-root <dir>             the folder that holds sound/ and soundaliases/ ( default: next to / above the csv )
//     --fx <name>=<file.efx>         an effect ( name as loadfx takes it: misc/my_effect ); effects it emits are read from
//                                    <name>.efx files of the same fx folder
//     --fx-atlas <file>              texture atlas table of the shipped materials ( default <out dir>/codol_fxatlas.txt; made
//                                    on first use, half a minute )
//     --iwd-add <archive.iwd>        the files of a plain archive go into the --iwd archive as well. An --iwd whose name
//                                    holds iw_sound, iw_image .. is written encrypted, as the game's own archives are
//     --carry <zone.ff>[,<type> ..]  take assets of another fast file as they are, each with what it points at: by default
//                                    its animations, sounds ( alias lists, loaded sounds, curves ) and effects; types as
//                                    --verify prints them ( xanim, sound, sndcurve, loaded_sound, fx, xmodel, material ..,
//                                    all ). What the zone already has is kept. A project zone -> the zone of a map
//
//   codol_assetc --verify <zone.ff> --install <dir> [--exe <client exe>]
//     loads the file with the game's loaders, writes it again ( must be identical ), lists what is in it

#include "codol_assets/fx.hpp"
#include "codol_assets/link.hpp"
#include "codol_render/iwd.hpp"
#include "codol_render/texture.hpp"
#include "codol_zone/assets.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>

using namespace codol::assets;
namespace fs = std::filesystem;
namespace c1 = codol::c1;
namespace z = codol::zone;

namespace
{
	std::vector<std::string> split(const std::string& text, const char separator)
	{
		std::vector<std::string> out;
		std::size_t at = 0;
		while (true)
		{
			const std::size_t next = text.find(separator, at);
			out.push_back(text.substr(at, next == std::string::npos ? std::string::npos : next - at));
			if (next == std::string::npos)
			{
				break;
			}
			at = next + 1;
		}
		return out;
	}

	int fail(const std::string& message)
	{
		std::fprintf(stderr, "codol_assetc: %s\n", message.c_str());
		return 1;
	}

	struct ImageRequest
	{
		std::string name;
		fs::path file;
		ImageOptions options;
		bool useGiven = false;
	};

	bool parse_image(const std::string& spec, ImageRequest& out, std::string& error)
	{
		const std::size_t equal = spec.find('=');
		if (equal == std::string::npos || equal == 0)
		{
			error = "--image wants <name>=<file>";
			return false;
		}
		out.name = spec.substr(0, equal);
		const std::vector<std::string> parts = split(spec.substr(equal + 1), ',');
		out.file = parts[0];
		for (std::size_t k = 1; k < parts.size(); k++)
		{
			const std::string& p = parts[k];
			const std::string value = p.find('=') == std::string::npos ? std::string() : p.substr(p.find('=') + 1);
			if (p.rfind("use=", 0) == 0)
			{
				out.useGiven = true;
				out.options.use = value == "color" ? ImageUse::color : value == "normal" ? ImageUse::normal : value == "specular" ? ImageUse::specular
					: value == "detail" ? ImageUse::detail : value == "ui" ? ImageUse::ui : ImageUse::function;
				if (value != "color" && value != "normal" && value != "specular" && value != "detail" && value != "ui" && value != "function")
				{
					error = "unknown image use '" + value + "'";
					return false;
				}
			}
			else if (p.rfind("format=", 0) == 0)
			{
				const std::pair<const char*, ImageFormat> formats[] = { { "dxt1", ImageFormat::dxt1 }, { "dxt3", ImageFormat::dxt3 }, { "dxt5", ImageFormat::dxt5 },
					{ "rgba", ImageFormat::rgba32 }, { "rgb", ImageFormat::rgb24 }, { "la", ImageFormat::luminance_alpha8 }, { "l", ImageFormat::luminance8 },
					{ "a", ImageFormat::alpha8 } };
				bool found = false;
				for (const auto& [name, format] : formats)
				{
					if (value == name)
					{
						out.options.format = format;
						found = true;
					}
				}
				if (!found)
				{
					error = "unknown image format '" + value + "'";
					return false;
				}
			}
			else if (p == "streamed")
			{
				out.options.streamed = true;
			}
			else if (p == "nomips")
			{
				out.options.mips = 0;
			}
			else if (p == "nopicmip")
			{
				out.options.noPicmip = 1;
			}
			else if (p == "clamp")
			{
				out.options.clampU = out.options.clampV = true;
			}
			else if (p == "flipy")
			{
				out.options.flipNormalY = true;
			}
			else
			{
				error = "unknown image option '" + p + "'";
				return false;
			}
		}
		return true;
	}

	ImageUse use_of_sampler(const std::string& sampler)
	{
		return sampler == "normalMap" ? ImageUse::normal : sampler == "specularMap" ? ImageUse::specular : sampler == "detailMap" ? ImageUse::detail : ImageUse::color;
	}

	int verify(const fs::path& file, const fs::path& install, fs::path exe)
	{
		exe = exe.empty() ? install / "codoMP_client_shipRetail.exe" : exe;
		z::ClientKey key;
		if (const z::Status s = key.load_from_exe(exe); !s)
		{
			return fail(s.message);
		}
		z::ZoneEditor editor;
		if (const z::Status s = editor.open(file, key); !s)
		{
			return fail(file.string() + ": " + s.message);
		}
		std::vector<std::uint8_t> original, again;
		if (const z::Status s = z::read_fastfile(file, key, original); !s)
		{
			return fail(s.message);
		}
		if (const z::Status s = editor.write(again); !s)
		{
			return fail("write: " + s.message);
		}
		const z::Zone& zone = editor.zone();
		std::printf("%s: the game's loaders consume the zone to the last byte: %zu top level assets, %zu with nested ones, %zu warnings\n", file.filename().string().c_str(),
			zone.assets().size(), zone.all_assets().size(), zone.warnings().size());
		std::printf("read -> write: %s ( %zu bytes )\n", original == again ? "identical" : "DIFFERENT", again.size());
		int bad = original == again ? 0 : 1;
		for (const z::XAssetEntry& e : zone.assets())
		{
			const std::string name(zone.asset_name(e.type, e.header));
			if (e.type == c1::ASSET_TYPE_IMAGE)
			{
				z::ImageView v;
				z::read_image(zone, e.header, v);
				std::string state = v.streamed ? "streamed ( images/" + name + ".iwi )" : "inline";
				if (!v.streamed)
				{
					codol::render::embedded_image image;
					image.d3d_format = v.loadDef.format;
					image.level_count = v.loadDef.levelCount;
					image.flags = v.loadDef.flags;
					image.width = v.raw.width;
					image.height = v.raw.height;
					image.map_type = v.raw.mapType;
					image.data_size = static_cast<std::uint32_t>(v.pixels.size());
					codol::render::texture_upload upload;
					std::string error;
					const bool ok = codol::render::build_embedded_upload(image, v.pixels, upload, &error);
					state += ok ? ", " + std::to_string(upload.levels) + " levels decode" : ", DOES NOT DECODE: " + error;
					bad += ok ? 0 : 1;
				}
				std::printf("  image     %-40s %4u x %-4u semantic %u format 0x%08X %s\n", name.c_str(), v.raw.width, v.raw.height, v.raw.semantic, v.loadDef.format, state.c_str());
			}
			else if (e.type == c1::ASSET_TYPE_MATERIAL)
			{
				z::MaterialView v;
				z::read_material(zone, e.header, v);
				std::string textures;
				for (const z::MaterialTexture& t : v.textures)
				{
					textures += " " + std::string(t.imageName);
				}
				std::printf("  material  %-40s techset %s%s, sort %u, %zu state bits, textures:%s\n", name.c_str(), std::string(v.techsetName).c_str(),
					zone.find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, v.techsetName) ? "" : " ( by name )", v.raw.sortKey, v.stateBits.size(), textures.c_str());
			}
			else if (e.type == c1::ASSET_TYPE_XMODEL)
			{
				z::ModelView v;
				z::read_model(zone, e.header, v, true);
				std::size_t vertices = 0, triangles = 0;
				for (const z::ModelLod& lod : v.lods)
				{
					for (const z::SurfaceView& s : lod.surfs.surfaces)
					{
						vertices += s.vertCount;
						triangles += s.triCount;
					}
				}
				std::printf("  xmodel    %-40s %zu bones, %zu LODs, %zu vertices, %zu triangles, %d collision surfaces, materials:", name.c_str(), v.bones.size(), v.lods.size(),
					vertices, triangles, v.raw.numCollSurfs);
				for (const std::string_view m : v.materialNames)
				{
					std::printf(" %.*s", static_cast<int>(m.size()), m.data());
				}
				std::printf("\n");
			}
			else if (e.type == c1::ASSET_TYPE_SOUND)
			{
				CompiledAliasList list;
				bool ok = extract_sound_alias_list(zone, e.header, list);
				std::string files;
				for (const CompiledAlias& a : list.aliases)
				{
					const std::string channel(sound_channel_name(static_cast<int>((a.raw.flags >> 9) & 0x3F)));
					ok = ok && !channel.empty();
					files += " " + (!a.hasFile ? std::string("( no file )") : a.streamed ? "streamed:" + a.streamDir + "/" + a.streamName
						: a.loadedName + (zone.find_asset(c1::ASSET_TYPE_LOADED_SOUND, a.loadedName) ? "" : " ( by name )")) + " [" + channel + ((a.raw.flags & 1) ? ", looping" : "") + "]";
				}
				bad += ok ? 0 : 1;
				std::printf("  sound     %-40s %zu alias(es):%s%s\n", name.c_str(), list.aliases.size(), files.c_str(), ok ? "" : "  DOES NOT READ");
			}
			else if (e.type == c1::ASSET_TYPE_LOADED_SOUND)
			{
				CompiledLoadedSound sound;
				WaveData pcm;
				const bool ok = extract_loaded_sound(editor, e.header, sound) && (sound.info.format != WAVE_IMA_ADPCM || ima_adpcm_decode(loaded_sound_wave(sound), pcm));
				bad += ok ? 0 : 1;
				std::printf("  loaded_sound %-37s %s, %d channel(s), %u Hz, %u samples, %u bytes%s\n", name.c_str(), sound.info.format == WAVE_IMA_ADPCM ? "IMA ADPCM" : "16 bit PCM",
					sound.info.channels, sound.info.rate, sound.info.samples, sound.info.dataLen, ok ? (sound.info.format == WAVE_IMA_ADPCM ? ", decodes" : "") : "  DOES NOT READ");
			}
			else if (e.type == c1::ASSET_TYPE_SOUND_CURVE)
			{
				SndCurveDef curve;
				extract_snd_curve(zone, e.header, curve);
				std::printf("  sndcurve  %-40s %zu knots\n", name.c_str(), curve.knots.size());
			}
			else if (e.type == c1::ASSET_TYPE_XANIMPARTS)
			{
				RawAnim raw;
				DecodedAnim decoded;
				std::string error;
				Status status;
				if (!read_raw_anim(zone, e.header, raw, &error))
				{
					status = Status::error(error);
				}
				else
				{
					status = decode_anim(raw, decoded);
				}
				const c1::XAnimParts& h = raw.header;
				std::string notes;
				for (const auto& [note, time] : raw.notifies)
				{
					char text[32];
					std::snprintf(text, sizeof(text), "@%.3f", time);
					notes += " " + note + text;
				}
				std::printf("  xanim     %-40s %u parts, %u frames at %g / s, flags %u%s%s, tracks %s, notifies:%s\n", name.c_str(), h.boneCount[9], h.numframes + 1u, h.framerate, h.flags,
					h.flags & ANIM_FLAG_LOOP ? " loop" : "", h.flags & (ANIM_FLAG_DELTA | ANIM_FLAG_DELTA_3D) ? " delta" : "", status ? "decode" : ("DO NOT DECODE: " + status.message).c_str(),
					notes.c_str());
				bad += status ? 0 : 1;
			}
			else if (e.type == c1::ASSET_TYPE_FX)
			{
				CompiledFx fx;
				const bool ok = read_fx(zone, e.header, fx);
				std::set<std::string> visuals;
				for (const FxElem& elem : fx.elems)
				{
					visuals.insert(elem.visuals.begin(), elem.visuals.end());
				}
				std::string list;
				for (const std::string& v : visuals)
				{
					list += " " + v;
				}
				std::printf("  fx        %-40s %zu elements ( %d looping, %d one shot, %d emission ), %d bytes, visuals:%s%s\n", name.c_str(), fx.elems.size(), fx.countLooping,
					fx.countOneShot, fx.countEmission, fx.totalSize, list.c_str(), ok && fx.totalSize == fx_total_size(fx) ? "" : "  DOES NOT READ");
				bad += ok && fx.totalSize == fx_total_size(fx) ? 0 : 1;
			}
			else
			{
				std::printf("  %-9s %s\n", z::asset_type_name(e.type).c_str(), name.c_str());
			}
		}
		return bad;
	}
}

int main(const int argc, char** argv)
{
	LinkOptions link;
	fs::path out, sourceRoot, iwiDir, iwd, zoneOut, verifyFile;
	std::vector<fs::path> iwdAdd;
	std::vector<std::string> images, materials, models, rawfiles;
	std::vector<std::string> anims, soundAliases;
	fs::path soundRoot;
	std::vector<std::string> effects;
	fs::path fxAtlasFile;
	std::vector<std::string> carried;
	bool nameGiven = false;
	for (int k = 1; k < argc; k++)
	{
		const std::string a = argv[k];
		const auto value = [&]() -> std::string { return k + 1 < argc ? argv[++k] : std::string(); };
		if (a == "--install") link.install = value();
		else if (a == "--exe") link.exe = value();
		else if (a == "--out") out = value();
		else if (a == "--base") link.base = value();
		else if (a == "--name") { link.zoneName = value(); nameGiven = true; }
		else if (a == "--matlib") link.libraryCache = value();
		else if (a == "--copy-techsets") link.copyAllTechsets = true;
		else if (a == "--source-root") sourceRoot = value();
		else if (a == "--iwi-dir") iwiDir = value();
		else if (a == "--iwd") iwd = value();
		else if (a == "--iwd-add") iwdAdd.push_back(value());
		else if (a == "--zone-out") zoneOut = value();
		else if (a == "--image") images.push_back(value());
		else if (a == "--material") materials.push_back(value());
		else if (a == "--xmodel") models.push_back(value());
		else if (a == "--rawfile") rawfiles.push_back(value());
		else if (a == "--xanim") anims.push_back(value());
		else if (a == "--soundalias") soundAliases.push_back(value());
		else if (a == "--sound-root") soundRoot = value();
		else if (a == "--fx") effects.push_back(value());
		else if (a == "--fx-atlas") fxAtlasFile = value();
		else if (a == "--carry") carried.push_back(value());
		else if (a == "--verify") verifyFile = value();
		else return fail("unknown option " + a + " ( usage: see the head of codol_assetc.cpp or the README )");
	}
	if (link.install.empty())
	{
		return fail("--install <1.1.1.13 install> is needed");
	}
	if (!verifyFile.empty())
	{
		return verify(verifyFile, link.install, link.exe);
	}
	if (out.empty() && !iwd.empty() && !iwdAdd.empty())
	{
		// only an archive: the files of plain archives under a name the game mounts ( iw_sound_NN.iwd ), no zone
		std::vector<std::pair<std::string, std::vector<std::uint8_t>>> files;
		for (const fs::path& from : iwdAdd)
		{
			std::string why;
			const auto archive = codol::render::iwd_archive::open(from, nullptr, &why);
			if (!archive)
			{
				return fail("--iwd-add " + from.string() + ": " + why);
			}
			for (const codol::render::iwd_entry& entry : archive->entries())
			{
				std::vector<std::uint8_t> bytes;
				if (!archive->read(entry, bytes, &why))
				{
					return fail("--iwd-add " + from.string() + ": " + why);
				}
				files.emplace_back(entry.name, std::move(bytes));
			}
		}
		codol::render::iwd_key key{};
		const bool encrypted = iwd_needs_key(iwd);
		std::string why;
		if (encrypted && !codol::render::derive_iwd_key(link.exe.empty() ? link.install / "codoMP_client_shipRetail.exe" : link.exe, key, &why))
		{
			return fail("--iwd " + iwd.filename().string() + " has a name the game decrypts, and the key could not be read from the client: " + why);
		}
		if (!game_mounts_iwd(iwd))
		{
			std::printf("warning: the game does not open an archive named %s: it takes iw_NN.iwd and iw_sound_NN / iw_image_NN / iw_imagestripped_NN / iw_model_NN / "
				"iw_config_NN.iwd with NN from 00 to 23\n", iwd.filename().string().c_str());
		}
		if (const Status s = write_iwd(iwd, files, encrypted ? &key : nullptr); !s)
		{
			return fail(s.message);
		}
		std::printf("wrote %s%s: %zu files of other archives\n", iwd.string().c_str(), encrypted ? " ( encrypted like the game's archives )" : "", files.size());
		return 0;
	}
	if (out.empty())
	{
		return fail("--out <zone.ff> is needed");
	}
	if (!nameGiven)
	{
		link.zoneName = out.stem().string();
	}
	if (link.libraryCache.empty())
	{
		link.libraryCache = out.parent_path() / "codol_matlib.txt";
	}

	AssetLinker linker;
	if (const Status s = linker.begin(link); !s)
	{
		return fail(s.message);
	}

	// ---- assets of other zones, as they are
	for (const std::string& spec : carried)
	{
		const std::vector<std::string> parts = split(spec, ',');
		std::vector<std::int32_t> types;
		for (std::size_t k = 1; k < parts.size(); k++)
		{
			if (parts[k] == "all")
			{
				for (std::int32_t t = 0; t < c1::ASSET_TYPE_COUNT; t++)
				{
					types.push_back(t);
				}
				continue;
			}
			const std::int32_t type = z::asset_type_from_name(parts[k]);
			if (type < 0)
			{
				return fail("--carry: unknown asset type '" + parts[k] + "'");
			}
			types.push_back(type);
		}
		if (types.empty())
		{
			types = { c1::ASSET_TYPE_XANIMPARTS, c1::ASSET_TYPE_SOUND_CURVE, c1::ASSET_TYPE_LOADED_SOUND, c1::ASSET_TYPE_SOUND, c1::ASSET_TYPE_FX };
		}
		AssetLinker::Carried result;
		if (const Status s = linker.carry(parts[0], types, result); !s)
		{
			return fail(s.message);
		}
		std::string counts;
		for (const auto& [type, count] : result.copied)
		{
			counts += (counts.empty() ? "" : ", ") + std::to_string(count) + " " + z::asset_type_name(type);
		}
		std::printf("carried from %s: %s%s\n", fs::path(parts[0]).filename().string().c_str(), counts.empty() ? "nothing" : counts.c_str(),
			result.dependencies ? ( " ( and " + std::to_string(result.dependencies) + " assets they point at )" ).c_str() : "");
		for (const std::string& line : result.kept)
		{
			std::printf("  %s: the zone has it already, kept\n", line.c_str());
		}
		for (const std::string& line : result.failed)
		{
			std::printf("warning: not carried: %s\n", line.c_str());
		}
	}

	// ---- sounds: loaded sounds and curves, then the alias lists that name them
	SoundBuild sounds;
	for (const std::string& file : soundAliases)
	{
		if (const Status s = build_sound_aliases(file, soundRoot.empty() ? sound_source_root(file) : soundRoot, linker.editor(), sounds); !s)
		{
			return fail(s.message);
		}
	}
	for (const std::string& line : sounds.log)
	{
		std::printf("%s\n", line.c_str());
	}
	if (!soundAliases.empty())
	{
		std::printf("sounds: %u alias lists ( %u aliases ), %u loaded sounds, %u curves, %zu streamed files found\n", sounds.lists, sounds.aliases, sounds.loadedSounds,
			sounds.curves, sounds.streamedFiles.size());
	}

	// ---- material definitions first: they say what the images are used for
	std::vector<MaterialDef> defs;
	for (const std::string& spec : materials)
	{
		std::vector<std::uint8_t> bytes;
		std::vector<MaterialDef> parsed;
		if (spec.rfind("gdt:", 0) == 0)
		{
			// gdt:<file>[:<entry>] - the file may hold a drive colon
			std::string file = spec.substr(4), entry;
			const std::size_t colon = file.rfind(':');
			if (colon != std::string::npos && colon > 1)
			{
				entry = file.substr(colon + 1);
				file = file.substr(0, colon);
			}
			if (const Status s = z::read_file(file, bytes); !s)
			{
				return fail(s.message);
			}
			if (const Status s = parse_gdt_materials({ reinterpret_cast<const char*>(bytes.data()), bytes.size() }, parsed); !s)
			{
				return fail(file + ": " + s.message);
			}
			if (!entry.empty())
			{
				std::erase_if(parsed, [&](const MaterialDef& d) { return d.name != entry; });
				if (parsed.empty())
				{
					return fail(file + " has no material entry '" + entry + "'");
				}
			}
			const fs::path root = sourceRoot.empty() ? fs::path(file).parent_path() : sourceRoot;
			for (const MaterialDef& def : parsed)
			{
				for (const MaterialTextureRef& texture : def.textures)
				{
					const auto source = def.imageFiles.find(texture.sampler);
					if (source != def.imageFiles.end())
					{
						std::string path = source->second;
						for (char& c : path)
						{
							c = c == '\\' ? '/' : c;
						}
						images.push_back(texture.image + "=" + (root / path).string() + ",use="
							+ (texture.sampler == "normalMap" ? "normal" : texture.sampler == "specularMap" ? "specular" : texture.sampler == "detailMap" ? "detail" : "color"));
					}
				}
			}
		}
		else
		{
			if (const Status s = z::read_file(spec, bytes); !s)
			{
				return fail(s.message);
			}
			if (const Status s = parse_material_text({ reinterpret_cast<const char*>(bytes.data()), bytes.size() }, parsed); !s)
			{
				return fail(spec + ": " + s.message);
			}
		}
		defs.insert(defs.end(), parsed.begin(), parsed.end());
	}

	// ---- images
	std::vector<std::pair<std::string, std::vector<std::uint8_t>>> iwiFiles;
	std::set<std::string> done;
	for (const std::string& spec : images)
	{
		ImageRequest request;
		std::string error;
		if (!parse_image(spec, request, error))
		{
			return fail(error);
		}
		if (!done.insert(request.name).second)
		{
			continue;
		}
		if (!request.useGiven)
		{
			for (const MaterialDef& def : defs)
			{
				for (const MaterialTextureRef& texture : def.textures)
				{
					if (texture.image == request.name)
					{
						request.options.use = (def.type == "2d" || def.techset == "2d") ? ImageUse::ui : use_of_sampler(texture.sampler);
					}
				}
			}
		}
		SourceImage source;
		if (const Status s = load_image(request.file, source); !s)
		{
			return fail(s.message);
		}
		CompiledImage compiled;
		if (const Status s = compile_image(request.name, source, request.options, compiled); !s)
		{
			return fail(s.message);
		}
		if (const Status s = linker.add_image(compiled); !s)
		{
			return fail(s.message);
		}
		std::printf("image %s: %u x %u %s, %d levels, %u bytes, %s\n", compiled.name.c_str(), compiled.width, compiled.height,
			codol::render::iwi_format_name(static_cast<codol::render::iwi_format>(compiled.iwiFormat)), compiled.levelCount, compiled.data_bytes(),
			compiled.streamed ? "streamed" : "inline");
		if (compiled.streamed)
		{
			iwiFiles.emplace_back("images/" + compiled.name + ".iwi", write_iwi(compiled));
		}
	}
	if (!iwiDir.empty())
	{
		for (const auto& [name, bytes] : iwiFiles)
		{
			std::error_code ec;
			fs::create_directories((iwiDir / name).parent_path(), ec);
			if (const Status s = z::write_file(iwiDir / name, bytes); !s)
			{
				return fail(s.message);
			}
		}
	}
	// --iwd-add <plain archive.iwd>: its files go into the --iwd archive as well ( a project's sound archive written
	// under a plain name becomes one with a name the game mounts )
	std::vector<std::pair<std::string, std::vector<std::uint8_t>>> addedFiles;
	for (const fs::path& from : iwdAdd)
	{
		std::string why;
		const auto archive = codol::render::iwd_archive::open(from, nullptr, &why);
		if (!archive)
		{
			return fail("--iwd-add " + from.string() + ": " + why);
		}
		for (const codol::render::iwd_entry& entry : archive->entries())
		{
			std::vector<std::uint8_t> bytes;
			if (!archive->read(entry, bytes, &why))
			{
				return fail("--iwd-add " + from.string() + ": " + why);
			}
			addedFiles.emplace_back(entry.name, std::move(bytes));
		}
	}
	if (!iwd.empty() && !(iwiFiles.empty() && sounds.streamedFiles.empty() && addedFiles.empty()))
	{
		auto packed = iwiFiles;
		packed.insert(packed.end(), addedFiles.begin(), addedFiles.end());
		packed.insert(packed.end(), sounds.streamedFiles.begin(), sounds.streamedFiles.end());
		// an archive with one of the game's own names ( iw_sound_NN.iwd .. ) is encrypted like the game's
		codol::render::iwd_key key{};
		const bool encrypted = iwd_needs_key(iwd);
		if (encrypted)
		{
			std::string why;
			if (!codol::render::derive_iwd_key(link.exe.empty() ? link.install / "codoMP_client_shipRetail.exe" : link.exe, key, &why))
			{
				return fail("--iwd " + iwd.filename().string() + " has a name the game decrypts, and the key could not be read from the client: " + why);
			}
		}
		if (const Status s = write_iwd(iwd, packed, encrypted ? &key : nullptr); !s)
		{
			return fail(s.message);
		}
		std::printf("wrote %s%s: %zu images, %zu streamed sound files, %zu files of other archives\n", iwd.string().c_str(),
			encrypted ? " ( encrypted like the game's archives )" : "", iwiFiles.size(), sounds.streamedFiles.size(), addedFiles.size());
	}
	if (iwd.empty() && !sounds.streamedFiles.empty())
	{
		std::printf("warning: %zu streamed sound file(s) and no --iwd: the game will not find them\n", sounds.streamedFiles.size());
	}
	else if (!iwiFiles.empty() && iwiDir.empty())
	{
		std::printf("warning: %zu streamed image(s) and neither --iwi-dir nor --iwd: the game will not find their pixels\n", iwiFiles.size());
	}

	// ---- materials
	std::map<std::string, std::int32_t> surfaceFlags;		// material -> surface flags of its collision triangles
	for (const MaterialDef& def : defs)
	{
		CompiledMaterial compiled;
		if (const Status s = compile_material(def, linker.library(), compiled); !s)
		{
			return fail(s.message);
		}
		if (const Status s = linker.add_material(compiled); !s)
		{
			return fail(s.message);
		}
		for (int type = 0; type < 32; type++)
		{
			if (compiled.header.surfaceTypeBits & (1u << type))
			{
				surfaceFlags[compiled.name] = (type + 1) << 20;		// shipped collision surfaces: surface type number << 20
			}
		}
		std::printf("material %s: techset %s, sort %u, %zu textures, state %s\n", compiled.name.c_str(), compiled.techset.c_str(), compiled.header.sortKey,
			compiled.textures.size(), compiled.stateShipped ? "as shipped materials have it" : "derived");
	}

	// ---- models
	for (const std::string& spec : models)
	{
		const std::size_t equal = spec.find('=');
		if (equal == std::string::npos || equal == 0)
		{
			return fail("--xmodel wants <name>=<file.xmodel_export>");
		}
		ModelOptions options;
		options.name = spec.substr(0, equal);
		options.collisionFlagsByMaterial = surfaceFlags;
		for (const std::string& part : split(spec.substr(equal + 1), ','))
		{
			if (part.rfind("collision=", 0) == 0)
			{
				options.collisionLod = std::atoi(part.c_str() + 10);
			}
			else if (part.rfind("contents=", 0) == 0)
			{
				options.contents = static_cast<std::int32_t>(std::strtol(part.c_str() + 9, nullptr, 0));
			}
			else if (part.rfind("hitloc:", 0) == 0 && part.find('=') != std::string::npos)
			{
				options.partClassification[part.substr(7, part.find('=') - 7)] = static_cast<std::uint8_t>(std::atoi(part.c_str() + part.find('=') + 1));
			}
			else if (part.rfind("mtl:", 0) == 0 && part.find('=') != std::string::npos)
			{
				options.materialNames[part.substr(4, part.find('=') - 4)] = part.substr(part.find('=') + 1);
			}
			else
			{
				ModelLodSource lod;
				std::string file = part;
				if (const std::size_t at = part.rfind('@'); at != std::string::npos)
				{
					lod.distance = static_cast<float>(std::atof(part.c_str() + at + 1));
					file = part.substr(0, at);
				}
				if (const Status s = load_xmodel_export(file, lod.mesh); !s)
				{
					return fail(s.message);
				}
				options.lods.push_back(std::move(lod));
			}
		}
		// export names CODOL-AssetExplorer makes of "mc/mtl_x": "codol_mc_mtl_x"
		for (const ModelLodSource& lod : options.lods)
		{
			for (const std::string& name : lod.mesh.materials)
			{
				if (options.materialNames.count(name) || linker.editor().find_asset(c1::ASSET_TYPE_MATERIAL, name) != z::ZoneEditor::npos || name.rfind("codol_", 0) != 0)
				{
					continue;
				}
				std::string native = name.substr(6);
				if (const std::size_t under = native.find('_'); under != std::string::npos)
				{
					native[under] = '/';
				}
				options.materialNames[name] = native;
			}
		}
		CompiledModel compiled;
		if (const Status s = compile_model(options, compiled); !s)
		{
			return fail(s.message);
		}
		if (const Status s = linker.add_model(compiled); !s)
		{
			return fail(s.message);
		}
		std::size_t vertices = 0, triangles = 0, surfaces = 0;
		for (const CompiledLod& lod : compiled.lods)
		{
			for (const CompiledSurface& s : lod.surfaces)
			{
				vertices += s.vertices.size();
				triangles += s.indices.size() / 3;
				surfaces++;
			}
		}
		std::printf("xmodel %s: %zu bones, %zu LODs, %zu surfaces, %zu vertices, %zu triangles, %zu collision surfaces\n", compiled.name.c_str(), compiled.boneNames.size(),
			compiled.lods.size(), surfaces, vertices, triangles, compiled.collision.size());
	}

	// ---- animations
	for (const std::string& spec : anims)
	{
		const std::size_t equal = spec.find('=');
		if (equal == std::string::npos || equal == 0)
		{
			return fail("--xanim wants <name>=<file.xanim_export>");
		}
		AnimOptions options;
		options.name = spec.substr(0, equal);
		ExportAnim source;
		ExportModel skeleton;
		bool sourceRead = false;
		for (const std::string& part : split(spec.substr(equal + 1), ','))
		{
			if (part.rfind("model=", 0) == 0)
			{
				if (const Status s = load_xmodel_export(part.substr(6), skeleton); !s)
				{
					return fail(s.message);
				}
				options.skeleton = &skeleton;
			}
			else if (part.rfind("type=", 0) == 0)
			{
				const std::string type = part.substr(5);
				if (type != "relative" && type != "delta" && type != "delta3d")
				{
					return fail("--xanim type= is relative, delta or delta3d");
				}
				options.type = type == "delta" ? AnimType::delta : type == "delta3d" ? AnimType::delta3d : AnimType::relative;
			}
			else if (part == "loop") options.looping = true;
			else if (part.rfind("deltapart=", 0) == 0) options.deltaPart = part.substr(10);
			else if (part.rfind("assettype=", 0) == 0) options.assetType = static_cast<std::uint8_t>(std::atoi(part.c_str() + 10));
			else if (part.rfind("rottol=", 0) == 0) options.rotationTolerance = static_cast<float>(std::atof(part.c_str() + 7));
			else if (part.rfind("transtol=", 0) == 0) options.translationTolerance = static_cast<float>(std::atof(part.c_str() + 9));
			else if (!sourceRead)
			{
				if (const Status s = load_xanim_export(part, source); !s)
				{
					return fail(part + ": " + s.message);
				}
				sourceRead = true;
			}
			else
			{
				return fail("--xanim: unknown option '" + part + "'");
			}
		}
		if (!sourceRead)
		{
			return fail("--xanim wants <name>=<file.xanim_export>");
		}
		CompiledAnim compiled;
		if (const Status s = compile_anim(source, options, compiled); !s)
		{
			return fail(s.message);
		}
		if (const Status s = linker.add_anim(compiled); !s)
		{
			return fail(s.message);
		}
		const c1::XAnimParts& h = compiled.raw.header;
		std::printf("xanim %s: %u parts, %u frames at %g / s%s%s, %u notifies, %zu bytes of tracks\n", compiled.raw.name.c_str(), h.boneCount[9], h.numframes + 1u, h.framerate,
			h.flags & ANIM_FLAG_LOOP ? ", looping" : "", h.flags & ANIM_FLAG_DELTA ? ", delta" : h.flags & ANIM_FLAG_DELTA_3D ? ", delta 3d" : "", h.notifyCount,
			compiled.raw.dataByte.size() + compiled.raw.dataShort.size() * 2 + compiled.raw.dataInt.size() * 4 + compiled.raw.randomDataShort.size() * 2
				+ compiled.raw.randomDataByte.size() + compiled.raw.indices.size() * 2);
		for (const std::string& note : compiled.notes)
		{
			std::printf("  xanim %s: %s\n", compiled.raw.name.c_str(), note.c_str());
		}
	}

	// ---- effects: after the materials and models they draw
	FxAtlasLibrary fxAtlas;
	for (const std::string& spec : effects)
	{
		const std::size_t equal = spec.find('=');
		if (equal == std::string::npos || equal == 0)
		{
			return fail("--fx wants <name>=<file.efx>");
		}
		const std::string name = spec.substr(0, equal);
		const fs::path file = spec.substr(equal + 1);
		if (fxAtlas.size() == 0)
		{
			if (const Status s = fxAtlas.open(link.install, linker.key(), fxAtlasFile.empty() ? out.parent_path() / "codol_fxatlas.txt" : fxAtlasFile); !s)
			{
				return fail(s.message);
			}
		}
		EditorEffect effect;
		CompiledFx fx;
		if (const Status s = load_efx(file, effect); !s)
		{
			return fail(s.message);
		}
		if (const Status s = compile_fx(name, effect, fx, fx_compile_options(linker.editor(), fxAtlas, fx_source_root(file, name))); !s)
		{
			return fail(s.message);
		}
		std::vector<std::string> notes = fx.notes;
		if (const Status s = link_fx(linker.editor(), fx, &notes); !s)
		{
			return fail(s.message);
		}
		std::printf("fx %s: %zu elements ( %d looping, %d one shot, %d emission ), %d bytes\n", name.c_str(), fx.elems.size(), fx.countLooping, fx.countOneShot,
			fx.countEmission, fx.totalSize);
		for (const std::string& note : notes)
		{
			std::printf("  %s\n", note.c_str());
		}
	}

	for (const std::string& spec : rawfiles)
	{
		const std::size_t equal = spec.find('=');
		std::vector<std::uint8_t> bytes;
		if (equal == std::string::npos || !z::read_file(spec.substr(equal + 1), bytes))
		{
			return fail("--rawfile wants <name>=<readable file>");
		}
		if (const Status s = linker.add_rawfile(spec.substr(0, equal), bytes); !s)
		{
			return fail(s.message);
		}
	}

	for (const std::string& line : linker.log())
	{
		std::printf("  %s\n", line.c_str());
	}
	std::vector<std::uint8_t> zoneBytes;
	if (const Status s = linker.write_zone(zoneBytes); !s)
	{
		return fail("the game's loaders do not accept the zone: " + s.message);
	}
	if (!zoneOut.empty())
	{
		z::write_file(zoneOut, zoneBytes);
	}
	if (const Status s = z::write_fastfile(out, zoneBytes, linker.key()); !s)
	{
		return fail(s.message);
	}
	std::printf("wrote %s: %zu assets, zone %zu bytes\n", out.string().c_str(), linker.editor().asset_count(), zoneBytes.size());
	return 0;
}
