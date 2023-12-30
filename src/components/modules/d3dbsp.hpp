#pragma once

namespace components
{
	class d3dbsp : public component
	{
	public:

		// *
		// * Entity Handling
		// *

		struct radiant_dobj_s
		{
			game::DObj_s obj;
			game::cpose_t pose;
		};

		static int radiant_dobj_count;
		static radiant_dobj_s radiant_dobj[512];

		static void dobj_clear_list();



		// *
		// * BSP Loading
		// *

		enum LumpType
		{
			LUMP_MATERIALS = 0x0,
			LUMP_LIGHTBYTES = 0x1,
			LUMP_LIGHTGRIDENTRIES = 0x2,
			LUMP_LIGHTGRIDCOLORS = 0x3,
			LUMP_PLANES = 0x4,
			LUMP_BRUSHSIDES = 0x5,
			LUMP_BRUSHSIDEEDGECOUNTS = 0x6,
			LUMP_BRUSHEDGES = 0x7,
			LUMP_BRUSHES = 0x8,
			LUMP_TRIANGLES = 0x9,
			LUMP_DRAWVERTS = 0xA,
			LUMP_DRAWINDICES = 0xB,
			LUMP_CULLGROUPS = 0xC,
			LUMP_CULLGROUPINDICES = 0xD,
			LUMP_OBSOLETE_1 = 0xE,
			LUMP_OBSOLETE_2 = 0xF,
			LUMP_OBSOLETE_3 = 0x10,
			LUMP_OBSOLETE_4 = 0x11,
			LUMP_OBSOLETE_5 = 0x12,
			LUMP_PORTALVERTS = 0x13,
			LUMP_OBSOLETE_6 = 0x14,
			LUMP_OBSOLETE_7 = 0x15,
			LUMP_OBSOLETE_8 = 0x16,
			LUMP_OBSOLETE_9 = 0x17,
			LUMP_AABBTREES = 0x18,
			LUMP_CELLS = 0x19,
			LUMP_PORTALS = 0x1A,
			LUMP_NODES = 0x1B,
			LUMP_LEAFS = 0x1C,
			LUMP_LEAFBRUSHES = 0x1D,
			LUMP_LEAFSURFACES = 0x1E,
			LUMP_COLLISIONVERTS = 0x1F,
			LUMP_COLLISIONTRIS = 0x20,
			LUMP_COLLISIONEDGEWALKABLE = 0x21,
			LUMP_COLLISIONBORDERS = 0x22,
			LUMP_COLLISIONPARTITIONS = 0x23,
			LUMP_COLLISIONAABBS = 0x24,
			LUMP_MODELS = 0x25,
			LUMP_VISIBILITY = 0x26,
			LUMP_ENTITIES = 0x27,
			LUMP_PATHCONNECTIONS = 0x28,
			LUMP_REFLECTION_PROBES = 0x29,
			LUMP_VERTEX_LAYER_DATA = 0x2A,
			LUMP_PRIMARY_LIGHTS = 0x2B,
			LUMP_LIGHTGRIDHEADER = 0x2C,
			LUMP_LIGHTGRIDROWS = 0x2D,
			LUMP_OBSOLETE_10 = 0x2E,
			LUMP_UNLAYERED_TRIANGLES = 0x2F,
			LUMP_UNLAYERED_DRAWVERTS = 0x30,
			LUMP_UNLAYERED_DRAWINDICES = 0x31,
			LUMP_UNLAYERED_CULLGROUPS = 0x32,
			LUMP_UNLAYERED_AABBTREES = 0x33,
			LUMP_LIGHTREGIONS = 0x34,
			LUMP_LIGHTREGION_HULLS = 0x35,
			LUMP_LIGHTREGION_AXES = 0x36,
		};

		struct BspChunk
		{
			LumpType type;
			unsigned int length;
		};

		struct BspHeader
		{
			unsigned int ident;
			unsigned int version;
			unsigned int chunkCount;
			BspChunk chunks[100];
		};

		struct bspGlob_s
		{
			char name[64];
			BspHeader* header;
			unsigned int fileSize;
			unsigned int checksum;
			LumpType loadedLumpType;
			const void* loadedLumpData;
		};

		static std::string loaded_bsp_path;
		static inline std::string last_compiled_map;
		static bspGlob_s comBspGlob;
		static game::clipMap_t cm;
		static game::GfxLight scene_lights[255];

		static void add_entities_to_scene();

		static game::cplane_s* CM_GetPlanes();
		static int CM_GetPlaneCount();

		static bool Com_IsBspLoaded();
		static std::uint32_t Com_GetBspVersion();
		static char* Com_ValidateBspLumpData(LumpType type, unsigned int offset, unsigned int length, unsigned int elemSize, unsigned int* count);

		static void Com_SaveLump(LumpType type, const void* newLump, unsigned int size);

		static const void* Com_GetBspLump(LumpType type, unsigned int elemSize, unsigned int* count);
		static bool Com_GetBspLumpBool(LumpType type);
		static bool Com_LoadBsp(const char* filename);

		static void shutdown_bsp();

		static bool radiant_load_bsp(const char* bsppath, bool reload = false);
		static void reload_bsp();

		static void toggle_radiant_bsp_view(bool show_bsp);

		static void compile_fastfile(const std::string& bsp_name);
		static void compile_bsp(const std::string& bsp_name, bool generate_createfx = false);
		static void compile_current_map();
		
		static void force_dvars();
		static void register_dvars();

		d3dbsp();
		~d3dbsp();
		const char* get_name() override { return "d3dbsp"; };

	};
}
