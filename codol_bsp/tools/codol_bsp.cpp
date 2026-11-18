// codol_bsp: compiles a .map into a map fast file of Call of Duty Online 1.1.1.13.
//
//   codol_bsp --map <file.map> --exe <codoMP_client_shipRetail.exe> --install <install dir> --out <mapname.ff> [--name <mapname>]
//             [--report <file>] [--zone-out <file>] [--cache <dir>] [--donor <zone.ff>]... [--rawfile <name>=<file>]...
//             [--fast | --quality n] [--no-light] [--no-vis] [--no-load] [--level 0..9]
//             [--minimap <picture.tga>] [--scripts <folder for the generated .gsc files>]
//   codol_bsp --check <map.ff> --exe <codoMP_client_shipRetail.exe>
//
// Exit code 0: written; 1: failed, nothing written ( message on stderr ); 2: usage.

#include "codol_bsp/bsp.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>

int main(const int argc, char** argv)
{
	codol::bsp::options opts;
	std::filesystem::path report_file, check_file;
	for (int i = 1; i < argc; i++)
	{
		const std::string arg = argv[i];
		const auto value = [&]() -> std::string { return i + 1 < argc ? argv[++i] : std::string(); };
		if (arg == "--map") { opts.map = value(); }
		else if (arg == "--exe") { opts.exe = value(); }
		else if (arg == "--install") { opts.install = value(); }
		else if (arg == "--out") { opts.out = value(); }
		else if (arg == "--name") { opts.name = value(); }
		else if (arg == "--zone-out") { opts.zone_out = value(); }
		else if (arg == "--cache") { opts.cache = value(); }
		else if (arg == "--report") { report_file = value(); }
		else if (arg == "--check") { check_file = value(); }
		else if (arg == "--donor" || arg == "--assets") { opts.donors.emplace_back(value()); }
		else if (arg == "--rawfile")
		{
			const std::string pair = value();
			const std::size_t eq = pair.find('=');
			if (eq == std::string::npos)
			{
				std::fprintf(stderr, "--rawfile <name in the zone>=<file>\n");
				return 2;
			}
			opts.rawfiles.emplace_back(pair.substr(0, eq), pair.substr(eq + 1));
		}
		else if (arg == "--fast") { opts.quality = 0; }
		else if (arg == "--quality") { opts.quality = std::atoi(value().c_str()); }
		else if (arg == "--no-light") { opts.light = false; }
		else if (arg == "--no-vis") { opts.vis = false; }
		else if (arg == "--no-load") { opts.load_zone = false; }
		else if (arg == "--level") { opts.level = std::atoi(value().c_str()); }
		else if (arg == "--exact-size") { opts.exact_size = std::strtoull(value().c_str(), nullptr, 10); }
		else if (arg == "--verbose") { opts.verbose = true; }
		else if (arg == "--minimap") { opts.minimap = value(); }
		else if (arg == "--scripts") { opts.scripts_out = value(); }
		else if (arg == "--vision") { opts.vision = value(); }
		else if (arg == "--sun") { opts.sun = value(); }
		else if (arg == "--prefabs") { opts.prefab_roots.emplace_back(value()); }
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
		std::vector<std::string> problems;
		std::string summary;
		const bool ok = codol::bsp::validate_zone(zone, problems, &summary);
		std::printf("%s\n", summary.c_str());
		for (const std::string& p : problems)
		{
			std::printf("problem: %s\n", p.c_str());
		}
		std::printf("%s: %s\n", check_file.string().c_str(), ok ? "valid" : "NOT valid");
		return ok ? 0 : 1;
	}
	if (opts.map.empty() || opts.exe.empty() || opts.install.empty() || opts.out.empty())
	{
		std::fprintf(stderr,
			"codol_bsp --map <file.map> --exe <codoMP_client_shipRetail.exe> --install <install dir> --out <mapname.ff> [--name <mapname>]\n"
			"          [--report <file>] [--zone-out <file>] [--cache <dir>] [--donor <zone.ff>]... [--rawfile <name>=<file>]...\n"
			"          [--assets <project zone.ff>]... [--minimap <picture.tga>] [--scripts <folder>] [--vision <file>] [--sun <file>]\n"
			"          [--fast | --quality n] [--no-light] [--no-vis] [--no-load] [--level 0..9] [--exact-size <bytes>]\n"
			"          [--prefabs <folder that holds prefabs\\..>]...\n"
			"codol_bsp --check <map.ff> --exe <codoMP_client_shipRetail.exe>\n");
		return 2;
	}
	codol::bsp::report rep;
	std::string error;
	const bool ok = codol::bsp::compile(opts, rep, error);
	const std::string text = rep.text();
	std::fputs(text.c_str(), stdout);
	if (!report_file.empty())
	{
		std::ofstream out(report_file, std::ios::trunc);
		out << text;
		if (!ok)
		{
			out << "FAILED: " << error << "\n";
		}
	}
	if (!ok)
	{
		std::fprintf(stderr, "codol_bsp: %s\n", error.c_str());
		return 1;
	}
	std::printf("wrote %s ( IWffu100 )\n", opts.out.string().c_str());
	return 0;
}
