// codol_mapc: compiles an edited map into the fast file it was imported from.
//
//   codol_mapc --base <map.ff> --exe <codoMP_client_shipRetail.exe> --map <edited.map> [--baseline <map.codol_base.map>] --out <new.ff>
//              [--report <file>] [--zone-out <file>] [--donor <zone.ff>]... [--level 0..9]
//              [--assets <project zone.ff>[,<type> ..]]... [--rawfile <name>=<file>]... [--append-rawfile <name>=<file>]...
//              [--no-entities] [--no-models] [--no-brushes] [--no-surfaces]
//   codol_mapc --check <map.ff> --exe <codoMP_client_shipRetail.exe>
//
// --check: reads the collision data of a fast file the way a dedicated server does ( codol_mapc/server_check.hpp )
// and says whether the server would take it.
//
// Exit code 0: compiled / accepted; 1: failed ( message on stderr ); 2: usage.

#include "codol_mapc/mapc.hpp"
#include "codol_mapc/server_check.hpp"
#include "codol_zone/fastfile.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>

int main(const int argc, char** argv)
{
	codol::mapc::options opts;
	std::filesystem::path report_file, check_file;
	for (int i = 1; i < argc; i++)
	{
		const std::string arg = argv[i];
		const auto value = [&]() -> std::string { return i + 1 < argc ? argv[++i] : std::string(); };
		if (arg == "--base") { opts.base = value(); }
		else if (arg == "--exe") { opts.exe = value(); }
		else if (arg == "--map") { opts.map = value(); }
		else if (arg == "--baseline") { opts.baseline = value(); }
		else if (arg == "--out") { opts.out = value(); }
		else if (arg == "--zone-out") { opts.zone_out = value(); }
		else if (arg == "--report") { report_file = value(); }
		else if (arg == "--check") { check_file = value(); }
		else if (arg == "--donor") { opts.donors.emplace_back(value()); }
		else if (arg == "--assets")
		{
			// <zone.ff>[,<type> ..]
			const std::string text = value();
			codol::mapc::options::asset_source source;
			std::size_t at = text.find(',');
			source.zone = text.substr(0, at);
			while (at != std::string::npos)
			{
				const std::size_t next = text.find(',', at + 1);
				source.types.push_back(text.substr(at + 1, next == std::string::npos ? std::string::npos : next - at - 1));
				at = next;
			}
			opts.assets.push_back(std::move(source));
		}
		else if (arg == "--rawfile" || arg == "--append-rawfile")
		{
			// <name>=<file>
			const std::string text = value();
			const std::size_t at = text.find('=');
			if (at == std::string::npos)
			{
				std::fprintf(stderr, "%s wants <name>=<file>\n", arg.c_str());
				return 1;
			}
			(arg == "--rawfile" ? opts.rawfiles : opts.rawfiles_appended).emplace_back(text.substr(0, at), text.substr(at + 1));
		}
		else if (arg == "--level") { opts.level = std::atoi(value().c_str()); }
		else if (arg == "--no-entities") { opts.entities = false; }
		else if (arg == "--no-models") { opts.models = false; }
		else if (arg == "--no-brushes") { opts.brushes = false; }
		else if (arg == "--no-surfaces") { opts.surfaces = false; }
		else
		{
			std::fprintf(stderr, "unknown argument %s\n", arg.c_str());
			return 2;
		}
	}
	if (!check_file.empty() && !opts.exe.empty())
	{
		codol::zone::ClientKey key;
		std::vector<std::uint8_t> zone;
		if (const codol::zone::Status s = key.load_from_exe(opts.exe); !s)
		{
			std::fprintf(stderr, "client key: %s\n", s.message.c_str());
			return 1;
		}
		if (const codol::zone::Status s = codol::zone::read_fastfile(check_file, key, zone); !s)
		{
			std::fprintf(stderr, "%s\n", s.message.c_str());
			return 1;
		}
		codol::mapc::server_collision_info info;
		std::string why;
		if (!codol::mapc::check_server_collision(zone, why, &info))
		{
			std::fprintf(stderr, "a dedicated server would refuse the collision data: %s\n", why.c_str());
			return 1;
		}
		std::printf("%s: collision data as a dedicated server reads it: %u brushes ( %u sides, %u edge bytes ), %u planes, %u leaf brush nodes, %u + %u leaf brushes, "
			"%u leafs, %u brush models, %u static models\n", info.map.c_str(), info.brushes, info.brush_sides, info.brush_edges, info.planes, info.leaf_brush_nodes,
			info.leaf_brushes, info.inline_leaf_brushes, info.leafs, info.sub_models, info.static_models);
		std::uint64_t hash = 1469598103934665603ull;
		std::size_t splits = 0;
		for (const codol::mapc::server_leaf_brush_node& n : info.nodes)
		{
			if (n.count > 0 || !n.child[0])
			{
				continue;
			}
			splits++;
			std::uint32_t words[3] = { static_cast<std::uint32_t>(n.axis) | (n.count < 0 ? 256u : 0u), 0, 0 };
			std::memcpy(&words[1], &n.dist, 4);
			std::memcpy(&words[2], &n.range, 4);
			for (const std::uint32_t w : words)
			{
				hash = (hash ^ w) * 1099511628211ull;
			}
		}
		std::printf("%zu splitting leaf brush nodes, hash of their axis / dist / range %016llx\n", splits, static_cast<unsigned long long>(hash));
		return 0;
	}
	if (opts.base.empty() || opts.exe.empty() || opts.map.empty() || opts.out.empty())
	{
		std::fprintf(stderr,
			"codol_mapc --base <map.ff> --exe <codoMP_client_shipRetail.exe> --map <edited.map> [--baseline <map.codol_base.map>] --out <new.ff>\n"
			"           [--report <file>] [--zone-out <file>] [--donor <zone.ff>]... [--level 0..9]\n"
			"           [--assets <project zone.ff>[,<type> ..]]... [--rawfile <name>=<file>]... [--append-rawfile <name>=<file>]...\n"
			"           [--no-entities] [--no-models] [--no-brushes] [--no-surfaces]\n"
			"codol_mapc --check <map.ff> --exe <codoMP_client_shipRetail.exe>\n");
		return 2;
	}
	codol::mapc::report report;
	std::string error;
	const bool ok = codol::mapc::compile(opts, report, error);
	std::string text = ok ? report.text() : "failed: " + error + "\n";
	if (ok)
	{
		text += "wrote " + opts.out.string() + " ( IWffu100 )\n";
	}
	std::fputs(text.c_str(), ok ? stdout : stderr);
	if (!report_file.empty())
	{
		std::ofstream(report_file, std::ios::binary) << text;
	}
	return ok ? 0 : 1;
}
