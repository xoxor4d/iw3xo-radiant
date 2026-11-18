#pragma once

// A map read straight from its 1.1.1.13 fastfile with the native zone reader ( codol_zone ):
// the entity string, every collision brush with the material of each side and the sub model that owns it,
// and every static model of the render world. No external tool, no intermediate files.

#include <filesystem>

namespace codol::mapgeo
{
	struct zone_map
	{
		map_geometry geometry;
		std::string entity_text;	// exactly as the zone stores it
		int trigger_models = -1;
		std::string world_name;		// "maps/mp/<map>.d3dbsp"
	};

	// `client_exe`: codoMP_client_shipRetail.exe of the install ( the fastfile key is read from it )
	bool read_zone_map(const std::filesystem::path& fastfile, const std::filesystem::path& client_exe, zone_map& out, std::string& error);
}
