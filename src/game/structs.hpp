#pragma once
#pragma warning(push)
#pragma warning(disable: 4324)
namespace game
{
	struct MaterialTechnique;
	typedef float vec_t;
	typedef vec_t vec2_t[2];
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];

	enum RadiantCommandType
	{
		RADIANT_COMMAND_SELECT = 0,
		RADIANT_COMMAND_DESELECT = 1,
		RADIANT_COMMAND_UPDATE_SELECTED = 2,
		RADIANT_COMMAND_UPDATE = 3,
		RADIANT_COMMAND_CREATE = 4,
		RADIANT_COMMAND_DELETE = 5,
		RADIANT_COMMAND_CAMERA = 6,
		RADIANT_COMMAND_BRUSH_SELECT = 7,		// on each new brush select / deselect
		RADIANT_COMMAND_BRUSH_COUNT = 8,		// amount of selected brushes
		RADIANT_COMMAND_BRUSH_CURRENT_NUM = 9,	// current processing brush num
		RADIANT_COMMAND_BRUSH_FACE = 10,		// parse a new face on the server
		RADIANT_COMMAND_BRUSH_FACE_COUNT = 11,	// total amount of faces per brush
		RADIANT_COMMAND_BRUSH_FACE_NORMALS = 12,// send brushface 3 & 4 to determin its angles 

		RADIANT_COMMAND_SET_DVAR = 30,
	};

	struct RadiantCommand
	{
		RadiantCommandType type;
		int liveUpdateId;
		char strCommand[512];
	};

	enum ServerCommandType
	{
		SERVER_STRING_MSG = 0,
		SERVER_CAMERA_UPDATE = 10,
		SERVER_EXPORT_SINGLE_BRUSH_FACE_INDEX = 20, // current index of face we are sending next
		SERVER_EXPORT_SINGLE_BRUSH_FACE = 21, // receive a single face
	};

	enum RENDER_METHOD_E : int
	{
		RM_WIREFRAME,
		RM_FULLBRIGHT,
		RM_NORMALFAKELIGHT,
		RM_VIEWFAKELIGHT,
		RM_CASETEXTURES,
	};
	
	struct ServerCommand
	{
		ServerCommandType type;
		int liveUpdateId;
		char strCommand[512];
	};

	struct ProcessServerCommands
	{
		int brush_currentSideIndex;
	};

	struct TrackWorldspawn
	{
		bool initiated;
		float sundirection[3];
		float suncolor[3];
		float sunlight;
	};

	union DvarLimits
	{
		struct
		{
			int stringCount;
			const char **strings;
		} enumeration;

		struct
		{
			int min;
			int max;
		} integer;

		struct
		{
			float min;
			float max;
		} value;

		struct
		{
			float min;
			float max;
		} vector;
	};

	union DvarValue
	{
		bool enabled;
		int integer;
		unsigned int unsignedInt;
		float value;
		float vector[4];
		const char *string;
		char color[4];
	};

	enum DvarType
	{
		DVAR_TYPE_BOOL = 0x0,
		DVAR_TYPE_FLOAT = 0x1,
		DVAR_TYPE_FLOAT_2 = 0x2,
		DVAR_TYPE_FLOAT_3 = 0x3,
		DVAR_TYPE_FLOAT_4 = 0x4,
		DVAR_TYPE_INT = 0x5,
		DVAR_TYPE_ENUM = 0x6,
		DVAR_TYPE_STRING = 0x7,
		DVAR_TYPE_COLOR = 0x8,
		DVAR_TYPE_COUNT = 0x9,
	};

	enum class dvar_type : std::int8_t
	{
		boolean = 0,
		value = 1,
		vec2 = 2,
		vec3 = 3,
		vec4 = 4,
		integer = 5,
		enumeration = 6,
		string = 7,
		color = 8,
		rgb = 9 // Color without alpha
	};

	enum dvar_flags : std::uint16_t
	{
		none = 0,
		saved = 1,
		write_protected = 0x10,
		latched = 0x20,
		read_only = 0x40,
		cheat_protected = 0x80,
		external = 0x4000,
	};

	struct dvar_s
	{
		const char *name;
		const char *description;
		//unsigned __int16 flags;
		dvar_flags flags;
		//char type;
		dvar_type type;
		bool modified;
		DvarValue current;
		DvarValue latched;
		DvarValue reset;
		DvarLimits domain;
		bool(__cdecl *domainFunc)(dvar_s *, DvarValue);
		dvar_s *hashNext;
	};

	enum MapType
	{
		MAPTYPE_NONE = 0x0,
		MAPTYPE_INVALID1 = 0x1,
		MAPTYPE_INVALID2 = 0x2,
		MAPTYPE_2D = 0x3,
		MAPTYPE_3D = 0x4,
		MAPTYPE_CUBE = 0x5,
		MAPTYPE_COUNT = 0x6,
	};

	struct GfxImageLoadDef
	{
		char levelCount;
		char flags;
		__int16 dimensions[3];
		int format;
		int resourceSize;
		char data[1];
	};

	union GfxTexture
	{
		GfxImageLoadDef* loadDef;
		void* data;
	};

	struct Picmip
	{
		char platform[2];
	};

	struct CardMemory
	{
		int platform[2];
	};

	struct GfxImage
	{
		MapType mapType;
		GfxTexture texture;
		Picmip picmip;
		bool noPicmip;
		char semantic;
		char track;
		CardMemory cardMemory;
		unsigned __int16 width;
		unsigned __int16 height;
		unsigned __int16 depth;
		char category;
		bool delayLoadPixels;
		const char* name;
	};

	struct texdef_t
	{
		char pad_0x0000[4];
		char *name;
		char pad_0x0008[12];
		int width;
		int height;
		char pad_0x001C[20];
	};

	enum E_PROP_CLASSTYPE
	{
		ECLASS_RADIANT_NODE = 0x2,
		ECLASS_LIGHT = 0x3,
		ECLASS_UNK = 0x10,
		ECLASS_MISC_MODEL = 0xA,
		ECLASS_PREFAB = 0x12,
		ECLASS_NODE = 0x22,
		ECLASS_TRIGGER_RADIUS = 0x42,
		ECLASS_TRIGGER_DISC = 0x82,
		ECLASS_REFLECTION_PROBE = 0x102,
	};
	
	struct __declspec(align(4)) eclass_t
	{
		eclass_t *next;
		const char *name;
		bool fixedsize;
		char pad_0x0009[3];
		vec3_t mins;
		vec3_t maxs;
		vec3_t color;
		float unk;
		char pad_0x0034[4];
		void *textureTableOrSth;
		char *comments;
		char flagnames[8][32];
		char xx0_pad[32];
		int xx1;
		const char* default_model_name;
		int xx3;
		int xx4;
		int xx5;
		int xx6;
		int xx7;
		int xx8;
		E_PROP_CLASSTYPE classtype;
		int xx10;
		int xx11;
		int xx12;
		char *commands;
	};
	STATIC_ASSERT_OFFSET(eclass_t, commands, 0x190);

	struct epair_t
	{
		epair_t *next;
		char *key;
		char *value;
	};

	struct winding_t
	{
		int numPoints;
		vec3_t points[1024];
	};

	struct plane_t
	{
		vec3_t normal;
		int type;
		double dist; // correct
		//float unk;
	};

	struct texdef_sub_t
	{
		float size[2];
		float shift[2];
		float rotate;
		int unk3;
		float sample_size;
	};

	struct qtexture_s
	{
		qtexture_s* next;
		const char* name;
		int unk_flags1;
		int unk_flags2;
		int tex_num;
		int width;
		int height;
		int color;
		int in_use;
		qtexture_s* prev;
		char pad_0x0028[8];
	};

	struct __declspec(align(4)) LayerMaterialDef
	{
		int xx001;
		int xx002;
		int xx003;
		int xx004;
		int xx005;
		int xx006;
		int xx007;
		int xx008;
		int xx009;
		int xx010;
		int xx011;
		int xx012;
		int xx013;
		int xx014;
		int xx015;
		int xx016;
		int xx017;
		int layerCount;
		int current_layer;
		int xx020;
		int layers_ptr;
		int xx022;
		int xx023;
		int xx024;
		int xx025;
		int xx026;
		int xx027;
		int xx028;
		int xx029;
		int xx030;
	};

	struct MaterialDef
	{
		LayerMaterialDef* lyrMtl;
		game::qtexture_s* radMtl;
		texdef_sub_t mat_texDef[1];
		//int value1;
		//game::qtexture_s* lightmapMtl;
		//texdef_sub_t lightmap_texDef;
		//int value2;
		//game::qtexture_s* smoothingMtl;
		//texdef_sub_t smoothing_texDef;
	}; STATIC_ASSERT_SIZE(MaterialDef, 0x24);//0x6C);

	struct face_t_new
	{
		vec3_t planepts0;
		vec3_t planepts1;
		vec3_t planepts2;
		//MaterialDef mtldef;
		MaterialDef mtldef[4];
		//int mat_unkown_value1;
		//qtexture_s* mat_unkown;
		//texdef_sub_t mat_unknown_texdef;
		int unk01;
		int unk02;
		int unk03;
		plane_t plane;
		int unk04;
		int unk05;
		winding_t* w;
		int minus1;
	}; STATIC_ASSERT_SIZE(face_t_new, 0xE8);

	struct face_t
	{
		vec3_t planepts0;
		vec3_t planepts1;
		vec3_t planepts2;
		int contents;
		texdef_t *texdef;
		float texScaleX;
		float texScaleY;
		float texOrgX;
		float texOrgY;
		float texRot;
		char pad_0x0040[12];
		texdef_t *mat_lightMap;
		float lightmapScaleX;
		float lightmapScaleY;
		float lightmapOrgX;
		float lightmapOrgY;
		float lightmapRot;
		char pad_0x0064[12];
		texdef_t *mat_smoothing;
		float smoothingScaleX;
		float smoothingScaleY;
		float smoothingOrgX;
		float smoothingOrgY;
		float smoothingRot;
		char pad_0x0088[12];
		texdef_t *mat_unusedLM;
		float unkTexScaleX;
		float unkTexScaleY;
		float unkTexOrgX;
		float unkTexOrgY;
		float unkTexRot;
		char pad_0x00AC[12];
		int endOfMats;
		int unk01;
		plane_t plane;
		int unk_val3;
		int unk03;
		winding_t *face_winding;
		int minus1;
	};

	struct entity_s;

	struct brush_t
	{
		brush_t *prev;
		brush_t *next;
		entity_s *owner;
		char pad_0x000C[4];
		int editAmount;
		char pad_0x0014[12];
		vec3_t mins;
		vec3_t maxs;
		int contentFlag;
		int materialLayer;
		int faceCount;
		face_t *brush_faces;
		char *mapLayer;
		char pad_0x004C[16];
	}; // brush_t is missing 8*4 bytes, see undo_s
	
	struct entity_s
	{
		entity_s *prev;
		entity_s *next;
		entity_s* firstActive;
		brush_t *firstBrush; // <- brush substruct, no ptr
		char pad_0x0010[52];
		void* modelInst;
		int prefab;
		int version;
		char* mapLayer;
		int someCount;
		bool bModelFailed;
		bool patchBrush;
		bool hiddenBrush;
		bool terrainBrush;
		char pPatch[4];
		eclass_t *eclass;
		char pad_0x0064[4];
		vec3_t origin;
		epair_t *epairs;
		int movedAmount;
		int epairEdits;
		char pad_0x0080[8];
		int refCount;
		char pad_0x008C[4];
	};
	STATIC_ASSERT_OFFSET(entity_s, modelInst, 0x44);
	STATIC_ASSERT_OFFSET(entity_s, eclass, 0x60);
	STATIC_ASSERT_OFFSET(entity_s, refCount, 0x88);

	struct brush_t_def
	{
		brush_t* oprev;
		brush_t* onext;
		entity_s* owner;
		entity_s* ownerNext;
		entity_s* ownerPrev;
		brush_t* def;
		int unk1;
		int refCount;
		vec3_t mins;
		vec3_t maxs;
	}; // brush_t is missing 8*4 bytes, see undo_s

	struct entity_s_def
	{
		entity_s* prev;
		entity_s* next;
		entity_s* def;
		brush_t_def brushes;
		int modelInst;
		int prefab;
		face_t* brush_faces;
		char* mapLayer;
		int someCount;
		bool bModelFailed;
		bool patchBrush;
		bool hiddenBrush;
		bool terrainBrush;
		char pPatch[4];
		eclass_t* eclass;
		void* modelClass;
		vec3_t origin;
		epair_t* epairs;
		int version;
		int epairEdits;
		char pad_0x0080[8];
		int refCount;
		char pad_0x008C[4];
	};
	STATIC_ASSERT_OFFSET(entity_s_def, modelInst, 0x44);
	STATIC_ASSERT_OFFSET(entity_s_def, epairs, 0x74);


	struct __declspec(align(4)) BrushPt_t
	{
		float xyz[3];
		int sideIndex[3];
	};

	struct drawVert_t
	{
		vec3_t xyz;
		float st;
		float lightmap;
		float smoothing;
		vec3_t normal;
		char pad_0x0030[4];
		float idk1;
		vec3_t idk2;
		char pad_0x0044[12];
	};

	struct patchMesh_t
	{
		int width;
		int height;
		int contents;
		int flags;
		int type;
		int subDivType;
		char pad_0x0018[4];
		qtexture_s* d_texture;
		char pad_0x0020[4];
		texdef_t* mat_lightmap;
		char pad_0x0028[4];
		texdef_t* mat_smoothing;
		char pad_0x0030[4];
		texdef_t* mat_unk;
		drawVert_t ctrl[16][16];
		int xx00;
		int xx0;
		int xx1;
		int xx2;
		int xx3;
		int xx4;
		int xx5;
		int xx6;
		int xx7;
		int xx8;
		int xx9;
		int xx10;
		char pad_0x4468[3028];
		brush_t* pSymbiot;
		int xx20;
		int xx21;
		int size_of_struct_0x504C;
	}; STATIC_ASSERT_SIZE(patchMesh_t, 0x504C);

	struct patch_def_t
	{
		void* unk;
	};


	struct patch_t
	{
		//patch_def_t* def;
		patchMesh_t* def;
	};

	

	struct brush_t_with_custom_def
	{
		brush_t_with_custom_def* oprev;
		brush_t_with_custom_def* onext;
		entity_s* owner;
		entity_s* ownerNext;
		entity_s* ownerPrev;
		brush_t_def* def;
		int unk1;
		int refCount;
		vec3_t mins;
		vec3_t maxs;
		int xx1;
		int xx2;
		int facecount;
		//face_t* brush_faces;
		face_t_new* brush_faces;
		char* layerstr;
		std::int16_t unk01;
		std::int16_t version;
		patchMesh_t* patch;
		int total_size_0x58;
	};
	STATIC_ASSERT_OFFSET(brush_t_with_custom_def, mins, 0x20);
	STATIC_ASSERT_OFFSET(brush_t_with_custom_def, version, 0x4E);

	
	// somethings really off with those structs
	// selected_brushes should be brush_t but that doesnt match up with real brush_t structs in memory? 
	//struct selbrush_t
	//{
	//	selbrush_t *prev; // prev selected brush @prev->currSelection
	//	selbrush_t *next; // next always empty i guess
	//	entity_s *owner;	// not sure
	//	brush_t *ownerNext; // not sure
	//	char pad[4];		// not sure
	//	brush_t *currSelection;	// selected brush info
	//	char pad2[4];
	//	vec3_t* faces;
	//	patch_t* patch;
	//	std::int16_t version;
	//}; STATIC_ASSERT_OFFSET(selbrush_t, version, 0x24);

	struct selbrush_def_t
	{
		selbrush_def_t* prev; // prev selected brush @prev->def
		selbrush_def_t* next; // next always empty i guess
		entity_s* owner;
		brush_t* ownerNext;
		int xx0;		
		brush_t_with_custom_def* def;
		int facecount;
		vec3_t* faces;
		patch_t* patch;
		std::int16_t version;
		std::int16_t unk;
		int xx1;
		int xx2;
		int xx3;
		int brushflags;
	};
	STATIC_ASSERT_OFFSET(selbrush_def_t, def, 0x14);
	STATIC_ASSERT_OFFSET(selbrush_def_t, version, 0x24);

	struct selbrush_ptr
	{
		selbrush_def_t*p;
	};

	struct selface_t
	{
		game::selbrush_def_t* brush;
		//game::face_t* face;
		game::face_t_new* face;
		int index;
	};

	union GfxColor
	{
		unsigned int packed;
		char array[4];
	};

	struct GfxFog
	{
		int startTime;
		int finishTime;
		GfxColor color;
		float fogStart;
		float density;
	};

	struct GfxMatrix
	{
		float m[4][4];
	};

	struct GfxViewParms
	{
		GfxMatrix viewMatrix;
		GfxMatrix projectionMatrix;
		GfxMatrix viewProjectionMatrix;
		GfxMatrix inverseViewProjectionMatrix;
		float origin[4];
		float axis[3][3];
		float depthHackNearClip;
		float zNear;
		int pad;
	};

	struct FxCodeMeshData
	{
		unsigned int triCount;
		unsigned __int16 *indices;
		unsigned __int16 argOffset;
		unsigned __int16 argCount;
		unsigned int pad;
	};

	struct GfxCmdHeader
	{
		unsigned __int16 id;
		unsigned __int16 byteCount;
	};

	struct GfxCmdArray
	{
		char *cmds;
		int usedTotal;
		int usedCritical;
		GfxCmdHeader *lastCmd;
	};

	struct GfxSceneDef
	{
		int time;
		float floatTime;
		float viewOffset[3];
	};

	struct GfxViewport
	{
		int x;
		int y;
		int width;
		int height;
	};

	enum ShadowType
	{
		SHADOW_NONE = 0x0,
		SHADOW_COOKIE = 0x1,
		SHADOW_MAP = 0x2,
	};

	enum MaterialTechniqueType
	{
		TECHNIQUE_DEPTH_PREPASS = 0x0,
		TECHNIQUE_BUILD_FLOAT_Z = 0x1,
		TECHNIQUE_BUILD_SHADOWMAP_DEPTH = 0x2,
		TECHNIQUE_BUILD_SHADOWMAP_COLOR = 0x3,
		TECHNIQUE_UNLIT = 0x4,
		TECHNIQUE_EMISSIVE = 0x5,
		TECHNIQUE_EMISSIVE_SHADOW = 0x6,
		TECHNIQUE_LIT_BEGIN = 0x7,
		TECHNIQUE_LIT = 0x7,
		TECHNIQUE_LIT_SUN = 0x8,
		TECHNIQUE_LIT_SUN_SHADOW = 0x9,
		TECHNIQUE_LIT_SPOT = 0xA,
		TECHNIQUE_LIT_SPOT_SHADOW = 0xB,
		TECHNIQUE_LIT_OMNI = 0xC,
		TECHNIQUE_LIT_OMNI_SHADOW = 0xD,
		TECHNIQUE_LIT_INSTANCED = 0xE,
		TECHNIQUE_LIT_INSTANCED_SUN = 0xF,
		TECHNIQUE_LIT_INSTANCED_SUN_SHADOW = 0x10,
		TECHNIQUE_LIT_INSTANCED_SPOT = 0x11,
		TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW = 0x12,
		TECHNIQUE_LIT_INSTANCED_OMNI = 0x13,
		TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW = 0x14,
		TECHNIQUE_LIT_END = 0x15,
		TECHNIQUE_LIGHT_SPOT = 0x15,
		TECHNIQUE_LIGHT_OMNI = 0x16,
		TECHNIQUE_LIGHT_SPOT_SHADOW = 0x17,
		TECHNIQUE_FAKELIGHT_NORMAL = 0x18,
		TECHNIQUE_FAKELIGHT_VIEW = 0x19,
		TECHNIQUE_SUNLIGHT_PREVIEW = 0x1A,
		TECHNIQUE_CASE_TEXTURE = 0x1B,
		TECHNIQUE_WIREFRAME_SOLID = 0x1C,
		TECHNIQUE_WIREFRAME_SHADED = 0x1D,
		TECHNIQUE_SHADOWCOOKIE_CASTER = 0x1E,
		TECHNIQUE_SHADOWCOOKIE_RECEIVER = 0x1F,
		TECHNIQUE_DEBUG_BUMPMAP = 0x20,
		TECHNIQUE_DEBUG_BUMPMAP_INSTANCED = 0x21,
	};

	struct __declspec(align(4)) GfxLightImage
	{
		void *image; // GfxImage
		char samplerState;
	};

	struct GfxLightDef
	{
		const char *name;
		GfxLightImage attenuation;
		int lmapLookupStart;
	};

	struct GfxLight
	{
		char type;
		char canUseShadowMap;
		char unused[2];
		float color[3];
		float dir[3];
		float origin[3];
		float radius;
		float cosHalfFovOuter;
		float cosHalfFovInner;
		int exponent;
		unsigned int spotShadowIndex;
		GfxLightDef *def;
	};

	struct GfxViewInfo;

	struct GfxDrawSurfListInfo
	{
		void *drawSurfs; // GfxDrawSurf
		unsigned int drawSurfCount;
		MaterialTechniqueType baseTechType;
		GfxViewInfo *viewInfo;
		float viewOrigin[4];
		GfxLight *light;
		int cameraView;
	};

	//struct __declspec(align(16)) ShadowCookie
	//{
	//	GfxMatrix shadowLookupMatrix;
	//	float boxMin[3];
	//	float boxMax[3];
	//	GfxViewParms *shadowViewParms;
	//	float fade;
	//	unsigned int sceneEntIndex;
	//	__unaligned __declspec(align(1)) GfxDrawSurfListInfo casterInfo;
	//	__unaligned __declspec(align(1)) GfxDrawSurfListInfo receiverInfo;
	//};

	//struct __declspec(align(16)) ShadowCookieList
	//{
	//	ShadowCookie cookies[24];
	//	unsigned int cookieCount;
	//};

	struct PointLightPartition
	{
		GfxLight light;
		GfxDrawSurfListInfo info;
	};

	struct GfxVertexBufferState
	{
		volatile int used;
		int total;
		int IDirect3DVertexBuffer9;
		char *verts;
	};

	struct GfxMeshData
	{
		unsigned int indexCount;
		unsigned int totalIndexCount;
		unsigned __int16 *indices;
		GfxVertexBufferState vb;
		unsigned int vertSize;
	};

	struct GfxDepthOfField
	{
		float viewModelStart;
		float viewModelEnd;
		float nearStart;
		float nearEnd;
		float farStart;
		float farEnd;
		float nearBlur;
		float farBlur;
	};

	struct GfxFilm
	{
		bool enabled;
		float brightness;
		float contrast;
		float desaturation;
		bool invert;
		float tintDark[3];
		float tintLight[3];
	};

	struct GfxGlow
	{
		bool enabled;
		float bloomCutoff;
		float bloomDesaturation;
		float bloomIntensity;
		float radius;
	};

	struct GfxSunShadowProjection
	{
		float viewMatrix[4][4];
		float switchPartition[4];
		float shadowmapScale[4];
	};

	struct GfxSunShadowBoundingPoly
	{
		float snapDelta[2];
		int pointCount;
		float points[9][2];
		int pointIsNear[9];
	};

	struct GfxSunShadowPartition
	{
		GfxViewParms shadowViewParms;
		int partitionIndex;
		GfxViewport viewport;
		__declspec(align(1)) GfxDrawSurfListInfo info;
		GfxSunShadowBoundingPoly boundingPoly;
	};

	struct GfxSunShadow
	{
		GfxMatrix lookupMatrix;
		GfxSunShadowProjection sunProj;
		GfxSunShadowPartition partition[2];
	};

	enum GfxRenderTargetId
	{
		R_RENDERTARGET_SAVED_SCREEN = 0x0,
		R_RENDERTARGET_FRAME_BUFFER = 0x1,
		R_RENDERTARGET_SCENE = 0x2,
		R_RENDERTARGET_RESOLVED_POST_SUN = 0x3,
		R_RENDERTARGET_RESOLVED_SCENE = 0x4,
		R_RENDERTARGET_FLOAT_Z = 0x5,
		R_RENDERTARGET_DYNAMICSHADOWS = 0x6,
		R_RENDERTARGET_PINGPONG_0 = 0x7,
		R_RENDERTARGET_PINGPONG_1 = 0x8,
		R_RENDERTARGET_SHADOWCOOKIE = 0x9,
		R_RENDERTARGET_SHADOWCOOKIE_BLUR = 0xA,
		R_RENDERTARGET_POST_EFFECT_0 = 0xB,
		R_RENDERTARGET_POST_EFFECT_1 = 0xC,
		R_RENDERTARGET_SHADOWMAP_SUN = 0xD,
		R_RENDERTARGET_SHADOWMAP_SPOT = 0xE,
		R_RENDERTARGET_COUNT = 0xF,
		R_RENDERTARGET_NONE = 0x10,
	};

	struct GfxSpotShadow
	{
		GfxViewParms shadowViewParms;
		GfxMatrix lookupMatrix;
		char shadowableLightIndex;
		char pad[3];
		GfxLight *light;
		float fade;
		__declspec(align(1)) GfxDrawSurfListInfo info;
		GfxViewport viewport;
		void *image; // GfxImage
		GfxRenderTargetId renderTargetId;
		float pixelAdjust[4];
		int clearScreen;
		GfxMeshData *clearMesh;
	};

	struct GfxQuadMeshData
	{
		float x;
		float y;
		float width;
		float height;
		GfxMeshData meshData;
	};

	struct GfxBackEndData;

	struct __declspec(align(8)) GfxCmdBufInput
	{
		float consts[58][4];
		void *codeImages[27]; // GfxImage
		char codeImageSamplerStates[27];
		GfxBackEndData *data;
		char pad;
	};
	
	struct GfxCmdSetViewport
	{
		GfxCmdHeader header;
		GfxViewport viewport;
	};
	
	struct GfxViewInfo
	{
		GfxViewParms viewParms;
		GfxSceneDef sceneDef;
		GfxViewport sceneViewport;
		GfxViewport displayViewport;
		GfxViewport scissorViewport;
		ShadowType dynamicShadowType;
		//__declspec(align(16)) ShadowCookieList shadowCookieList;
		char shadowCookieList[1210];
		int localClientNum;
		int isRenderingFullScreen;
		bool needsFloatZ;
		GfxLight shadowableLights[255];
		unsigned int shadowableLightCount;
		PointLightPartition pointLightPartitions[4];
		GfxMeshData pointLightMeshData[4];
		int pointLightCount;
		unsigned int emissiveSpotLightIndex;
		GfxLight emissiveSpotLight;
		int emissiveSpotDrawSurfCount;
		void *emissiveSpotDrawSurfs; // GfxDrawSurf
		unsigned int emissiveSpotLightCount;
		float blurRadius;
		float frustumPlanes[4][4];
		GfxDepthOfField dof;
		GfxFilm film;
		GfxGlow glow;
		const void *cmds;
		GfxSunShadow sunShadow;
		unsigned int spotShadowCount;
		GfxSpotShadow spotShadows[4];
		GfxQuadMeshData *fullSceneViewMesh;
		GfxDrawSurfListInfo litInfo;
		GfxDrawSurfListInfo decalInfo;
		GfxDrawSurfListInfo emissiveInfo;
		GfxCmdBufInput input;
	};

	struct GfxDebugPoly
	{
		float color[4];
		int firstVert;
		int vertCount;
	};

	struct trDebugString_t
	{
		float xyz[3];
		float color[4];
		float scale;
		char text[96];
	};

	struct trDebugLine_t
	{
		float start[3];
		float end[3];
		float color[4];
		int depthTest;
	};

	struct GfxDebugPlume
	{
		float origin[3];
		float color[4];
		int score;
		int startTime;
		int duration;
	};

	struct DebugGlobals
	{
		float(*verts)[3];
		int vertCount;
		int vertLimit;
		GfxDebugPoly *polys;
		int polyCount;
		int polyLimit;
		trDebugString_t *strings;
		int stringCount;
		int stringLimit;
		trDebugString_t *externStrings;
		int externStringCount;
		int externMaxStringCount;
		trDebugLine_t *lines;
		int lineCount;
		int lineLimit;
		trDebugLine_t *externLines;
		int externLineCount;
		int externMaxLineCount;
		GfxDebugPlume *plumes;
		int plumeCount;
		int plumeLimit;
	};

	struct GfxBackEndData
	{
		char surfsBuffer[131072];
		FxCodeMeshData codeMeshes[2048];
		int primDrawSurfsBuf[65536];
		GfxViewParms viewParms[1];
		char pad_soemtpy[4096][118];
		char pad_whatever[88];
		int viewParmCount;
		GfxFog fogSettings[1];
		GfxCmdArray *commands;
		int viewInfoIndex;
		int viewInfoCount;
		GfxViewInfo *viewInfo;
		char pad_0xDE1C0[4];
		GfxLight sunLight[1];
		int hasApproxSunDirChanged;
		int primDrawSurfPos;
		int *staticModelLit;
		DebugGlobals debugGlobals[1];
		int drawType;
	};

	/*struct IDirect3DSwapChain9Vtbl
	{
		HRESULT(__stdcall* QueryInterface)(IDirect3DSwapChain9* This, const IID* const riid, void** ppvObj);
		ULONG(__stdcall* AddRef)(IDirect3DSwapChain9* This);
		ULONG(__stdcall* Release)(IDirect3DSwapChain9* This);
		HRESULT(__stdcall* Present)(IDirect3DSwapChain9* This, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags);
		HRESULT(__stdcall* GetFrontBufferData)(IDirect3DSwapChain9* This, IDirect3DSurface9* pDestSurface);
		HRESULT(__stdcall* GetBackBuffer)(IDirect3DSwapChain9* This, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
		HRESULT(__stdcall* GetRasterStatus)(IDirect3DSwapChain9* This, D3DRASTER_STATUS* pRasterStatus);
		HRESULT(__stdcall* GetDisplayMode)(IDirect3DSwapChain9* This, D3DDISPLAYMODE* pMode);
		HRESULT(__stdcall* GetDevice)(IDirect3DSwapChain9* This, IDirect3DDevice9** ppDevice);
		HRESULT(__stdcall* GetPresentParameters)(IDirect3DSwapChain9* This, D3DPRESENT_PARAMETERS* pPresentationParameters);
		D3DPRESENT_PARAMETERS PresentParameters;
		D3DDISPLAYMODE DisplayMode;
	};

	struct IDirect3DSwapChain9
	{
		IDirect3DSwapChain9Vtbl* lpVtbl;
	};*/

	struct GfxWindowTarget
	{
		HWND__* hwnd;
		IDirect3DSwapChain9* swapChain;
		int width;
		int height;
	};

	struct GfxWindowParms
	{
		HWND hwnd;
		int hz;
		bool fullscreen;
		int x;
		int y;
		int sceneWidth;
		int sceneHeight;
		int displayWidth;
		int displayHeight;
		int aaSamples;
	};

	struct __declspec(align(8)) DxGlobals
	{
		IDirect3DQuery9* query;
		HINSTANCE__* hinst;
		IDirect3D9* d3d9;
		IDirect3DDevice9* device;
		unsigned int adapterIndex;
		bool adapterNativeIsValid;
		int adapterNativeWidth;
		int adapterNativeHeight;
		int adapterFullscreenWidth;
		int adapterFullscreenHeight;
		int depthStencilFormat;
		unsigned int displayModeCount;
		_D3DDISPLAYMODE displayModes[256];
		const char* resolutionNameTable[257];
		const char* refreshRateNameTable[257];
		char modeText[5120];
		IDirect3DQuery9* fencePool[8];
		unsigned int nextFence;
		int gpuSync;
		int multiSampleType;
		unsigned int multiSampleQuality;
		IDirect3DSurface9* singleSampleDepthStencilSurface;
		bool deviceLost;
		bool inScene;
		int targetWindowIndex;
		int windowCount;
		GfxWindowTarget windows[5];
		int flushGpuQueryCount;
		IDirect3DQuery9* flushGpuQuery;
		unsigned __int64 gpuSyncDelay;
		unsigned __int64 gpuSyncStart;
		unsigned __int64 gpuSyncEnd;
		int pad02;
		bool flushGpuQueryIssued;
		int linearNonMippedMinFilter;
		int linearNonMippedMagFilter;
		int linearMippedMinFilter;
		int linearMippedMagFilter;
		int anisotropicMinFilter;
		int anisotropicMagFilter;
		int linearMippedAnisotropy;
		int anisotropyFor2x;
		int anisotropyFor4x;
		int mipFilterMode;
		unsigned int mipBias;
		IDirect3DQuery9* swapFence;
	};

	struct pedge_t
	{
		int p1;
		int p2;
		face_t* f1;
		face_t* f2;
	};

	struct terrainVert_t
	{
		float height;
		float scale;
	};

	struct LPMRUMENU
	{
		WORD wNbItemFill;
		WORD wNbLruShow;
		WORD wNbLruMenu;
		WORD wMaxSizeLruItem;
		WORD wIdMru;
		LPSTR lpMRU;
	};
	
	enum SavedInfo_Colors
	{
		COLOR_TEXTUREBACK = 0x0,
		COLOR_GRIDBACK = 0x1,
		COLOR_GRIDMINOR = 0x2,
		COLOR_GRIDMAJOR = 0x3,
		COLOR_CAMERABACK = 0x4,
		COLOR_ENTITY = 0x5,
		COLOR_ENTITYUNK = 0x6,
		COLOR_GRIDBLOCK = 0x7,
		COLOR_GRIDTEXT = 0x8,
		COLOR_BRUSHES = 0x9,
		COLOR_SELBRUSHES = 0xA,
		COLOR_SELBRUSHES_CAMERA = 0xB,
		COLOR_CLIPPER = 0xC,
		COLOR_VIEWNAME = 0xD,
		COLOR_DETAIL_BRUSH = 0xE,
		COLOR_DRAW_TOGGLESUFS = 0xF,
		COLOR_SELFACE_CAMERA = 0x10,
		COLOR_FUNC_GROUP = 0x11,
		COLOR_FUNC_CULLGROUP = 0x12,
		COLOR_WEAPON_CLIP = 0x13,
		COLOR_SIZE_INFO = 0x14,
		COLOR_MODEL = 0x15,
		COLOR_NONCOLLIDING = 0x16,
		COLOR_WIREFRAME = 0x17,
		COLOR_FROZEN_LAYERS = 0x18,
		COLOR_UNKOWN2 = 0x19,
		COLOR_UNKOWN3 = 0x1A,
	};

	struct SavedInfo_t
	{
		int iSize;
		int iTextMenu;
		char szProject[256];
		vec4_t colors[27];
		int d_xyShowFlags;
		float d_gridsize;
		int d_picmip;
	}; STATIC_ASSERT_SIZE(SavedInfo_t, 0x2C4);

	enum select_t
	{
		sel_brush = 0x0,
		sel_vertex = 0x1,
		sel_edge = 0x2,
		sel_singlevertex = 0x3,
		sel_curvepoint = 0x4,
		sel_area = 0x5,
		sel_terrainpoint = 0x6,
		sel_terraintexture = 0x7,
		sel_addpoint = 0x8,
		sel_cycle_edge_direction_quad = 0x9,
		sel_editpoint = 0xA,
	};

	struct select_info_t
	{
		int numBrushesAndPatches;
		int numEntWithFlag;
		int numPatches;
		int numBrushes;
		int numFixedSize;
	};

	struct GfxDrawSurfFields
	{
		unsigned __int64 objectId : 16;
		unsigned __int64 reflectionProbeIndex : 8;
		unsigned __int64 customIndex : 5;
		unsigned __int64 materialSortedIndex : 11;
		unsigned __int64 prepass : 2;
		unsigned __int64 primaryLightIndex : 8;
		unsigned __int64 surfType : 4;
		unsigned __int64 primarySortKey : 6;
		unsigned __int64 unused : 4;
	};

	union GfxDrawSurf
	{
		GfxDrawSurfFields fields;
		unsigned __int64 packed;
	};

	struct MaterialInfo
	{
		const char* name;
		char gameFlags;
		char sortKey;
		char textureAtlasRowCount;
		char textureAtlasColumnCount;
		GfxDrawSurf drawSurf;
		unsigned int surfaceTypeBits;
		unsigned __int16 hashIndex;
	};

	struct GfxStateBits
	{
		unsigned int loadBits[2];
	};

	struct WaterWritable
	{
		float floatTime;
	};

	struct complex_s
	{
		float real;
		float imag;
	};

	struct water_t
	{
		WaterWritable writable;
		complex_s* H0;
		float* wTerm;
		int M;
		int N;
		float Lx;
		float Lz;
		float gravity;
		float windvel;
		float winddir[2];
		float amplitude;
		float codeConstant[4];
		GfxImage* image;
	};

	union MaterialTextureDefInfo
	{
		GfxImage* image;
		water_t* water;
	};

	struct MaterialTextureDef
	{
		unsigned int nameHash;
		char nameStart;
		char nameEnd;
		char samplerState;
		char semantic;
		MaterialTextureDefInfo u;
	};

	struct MaterialConstantDef
	{
		int nameHash;
		char name[12];
		vec4_t literal;
	};

	enum MaterialWorldVertexFormat : __int8
	{
		MTL_WORLDVERT_TEX_1_NRM_1 = 0x0,
		MTL_WORLDVERT_TEX_2_NRM_1 = 0x1,
		MTL_WORLDVERT_TEX_2_NRM_2 = 0x2,
		MTL_WORLDVERT_TEX_3_NRM_1 = 0x3,
		MTL_WORLDVERT_TEX_3_NRM_2 = 0x4,
		MTL_WORLDVERT_TEX_3_NRM_3 = 0x5,
		MTL_WORLDVERT_TEX_4_NRM_1 = 0x6,
		MTL_WORLDVERT_TEX_4_NRM_2 = 0x7,
		MTL_WORLDVERT_TEX_4_NRM_3 = 0x8,
		MTL_WORLDVERT_TEX_5_NRM_1 = 0x9,
		MTL_WORLDVERT_TEX_5_NRM_2 = 0xA,
		MTL_WORLDVERT_TEX_5_NRM_3 = 0xB,
	};

	struct MaterialTechniqueSet
	{
		char* name;
		MaterialWorldVertexFormat worldVertFormat;
		MaterialTechnique* techniques[34];
	};
	
	struct Material
	{
		MaterialInfo info;
		char pad[32];
		char stateBitsEntry[34];
		char textureCount;
		char constantCount;
		char stateBitsCount;
		char stateFlags;
		char cameraRegion;
		MaterialTechniqueSet* techniqueSet;
		MaterialTextureDef* textureTable;
		MaterialConstantDef* constantTable;
		GfxStateBits* stateBitsTable;
	};

	struct Glyph
	{
		unsigned __int16 letter;
		char x0;
		char y0;
		char dx;
		char pixelWidth;
		char pixelHeight;
		float s0;
		float t0;
		float s1;
		float t1;
	};

	struct Font_s
	{
		const char* fontName;
		int pixelHeight;
		int glyphCount;
		Material* material;
		Material* glowMaterial;
		Glyph* glyphs;
	};

	struct GfxCodeMatrices
	{
		GfxMatrix matrix[32];
	};

	struct GfxPlacement
	{
		float quat[4];
		float origin[3];
	};

	struct GfxScaledPlacement
	{
		GfxPlacement base;
		float scale;
	};

	enum GfxViewMode
	{
		VIEW_MODE_NONE = 0x0,
		VIEW_MODE_3D = 0x1,
		VIEW_MODE_2D = 0x2,
		VIEW_MODE_IDENTITY = 0x3,
	};

	enum GfxViewportBehavior
	{
		GFX_USE_VIEWPORT_FOR_VIEW = 0x0,
		GFX_USE_VIEWPORT_FULL = 0x1,
	};

	struct __declspec(align(16)) GfxCmdBufSourceState
	{
		GfxCodeMatrices matrices;
		GfxCmdBufInput input;
		GfxViewParms viewParms;
		GfxMatrix shadowLookupMatrix;
		unsigned __int16 constVersions[90];
		unsigned __int16 matrixVersions[8];
		float eyeOffset[4];
		unsigned int shadowableLightForShadowLookupMatrix;
		GfxScaledPlacement* objectPlacement;
		GfxViewParms* viewParms3D;
		unsigned int depthHackFlags;
		GfxScaledPlacement skinnedPlacement;
		int cameraView;
		GfxViewMode viewMode;
		GfxSceneDef sceneDef;
		GfxViewport sceneViewport;
		float materialTime;
		GfxViewportBehavior viewportBehavior;
		int renderTargetWidth;
		int renderTargetHeight;
		bool viewportIsDirty;
		unsigned int shadowableLightIndex;
	};

	struct GfxCmdDrawText3D
	{
		game::GfxCmdHeader header;
		float org[3];
		game::Font_s* font;
		float xPixelStep[3];
		float yPixelStep[3];
		game::GfxColor color;
		char text[4];
	}; STATIC_ASSERT_SIZE(GfxCmdDrawText3D, 0x34);

	enum FILTER_TYPE1
	{
		CONTENTS = 0,
		SURFACE_PROPERTY = 1, // info-> contents_ptr == texture pointer (union)
		SPECIFIC_MATERIAL = 4, // material_ptr = Material or MaterialInfo struct at offs 0x8
	};

	enum FILTER_TYPE2
	{
		NONE = 0,
		FDHIDE = 1,
		FDSHOW = 2,
		FACE = 4,
	};

	union filter_contents_s0
	{
		int contents;
		const char* name;
		const char* key;
	};

	union filter_contents_s1
	{
		int contents;
		const char* name;
		const char* value;
	};

	struct filter_contents_s
	{
		filter_contents_s0 u0;
		filter_contents_s1 u1;
	};

	struct filter_info_s
	{
		int surfaceFlags; //0x0000 
		char z_pad_0x0004[0x4]; //0x0004
		filter_contents_s* contents_ptr; //0x0008 
	}; //Size=0x0010

	struct filter_entry_s
	{
		FILTER_TYPE2 filter_type_enum; //0x0000 
		bool isShown; // enabled //0x0004 
		char z_pad_0x0005[0x3]; //0x0005
		MaterialInfo* material_ptr; //0x0008 
		const char* name; //0x000C 
		filter_info_s* info; //0x0010 
		filter_entry_s* next_filter; //0x0014 
	};
	
	struct __declspec(align(8)) qeglobals_t
	{
		bool d_showgrid;
		int d_gridsize;
		int d_num_entities;
		entity_s* d_project_entity;
		float d_new_brush_bottom_x;
		float d_new_brush_bottom_y;
		float d_new_brush_bottom_z;
		float d_new_brush_top_x;
		float d_new_brush_top_y;
		float d_new_brush_top_z;
		HINSTANCE d_hInstance;
		HWND d_hwndMain;
		HWND d_hwndCamera;
		HWND d_hwndEdit;
		HWND d_hwndEntity;
		HWND d_hwndTexture;
		HWND d_hwndXY;
		HWND d_hwndZ;
		HWND d_hwndStatus;
		HWND d_hwndGroup;
		HWND d_hwndMedia;
		vec3_t d_points[2048];
		int d_numpoints;
		pedge_t d_edges[512];
		int d_numedges;
		int d_num_move_points;
		float* d_move_points[4096];
		int d_numterrapoints;
		terrainVert_t* d_terrapoints[4096];
		vec3_t d_select_translate_unk;
		float unkown_pmesh_float1;
		int pad_01;
		char patch_verts_array01[196600];
		int patch_verts_array01_count;
		float unkown_pmesh_float2;
		float unkown_pmesh_float3;
		char patch_verts_array02[196600];
		int patch_verts_array02_count;
		int pad_02;
		int current_edit_layer;
		void* d_activeLayer;
		char random_texture_stuff[6300];
		LPMRUMENU* d_lpMruMenu;
		SavedInfo_t d_savedinfo;
		int d_workcount;
		int d_select_count;
		brush_t* d_select_order[2];
		vec3_t d_select_translate;
		select_t d_select_mode;
		select_info_t d_select_info;
		int surfInsp_nIDButton;
		int surfInsp_unk01;
		int surfInsp_unk02;
		Font_s* d_font_list;
		Material* d_white;
		Material* d_opague;
		Material* d_additive;
		int d_parsed_brushes;
		int pad_d_parsed_brushes;
		int some_cam_xy_count1;
		int some_cam_xy_count2;
		int some_cam_xy_count3;
		int some_cam_xy_count4;
		void* camera_fov_setup;
		bool use_ini;
		char pad_use_ini[3];
		char use_ini_registry[64];
		bool dontDrawSelectedOutlines;
		char pad_dontDrawSelectedOutlines[3];
		bool dontDrawSelectedTint;
		char pad_dontDrawSelectedTint[3];
		bool preview_at_max_intensity;
		char pad_draw_toggle_unk01[3];
		bool toggle_unk02;
		bool toggle_unk03_mousedrag_state1;
		bool toggle_unk04_mousedrag_state2;
		bool bLockPatchVerts;
		bool bUnlockPatchVerts;
		bool toggle_unk05;
		char pad3[6];
		double g_oldtime;
		double g_time;
		int g_filtersUpdated;
		int g_layerCount_maybe;
		__int16 w_cyclePreviewMode;
		char pad_cyclePreviewMode[2];
		filter_entry_s* d_filterGlobals_geometryFilters;
		filter_entry_s* d_filterGlobals_entityFilters;
		filter_entry_s* d_filterGlobals_triggerFilters;
		filter_entry_s* d_filterGlobals_otherFilters;
		filter_entry_s* d_filterGlobals_layerFilters;
	}; STATIC_ASSERT_SIZE(qeglobals_t, 0x71CE0);
	STATIC_ASSERT_OFFSET(qeglobals_t, d_select_mode, 0x71C04);

	struct GfxRenderTargetSurface
	{
		IDirect3DSurface9* color;
		IDirect3DSurface9* depthStencil;
	};

	struct GfxRenderTarget
	{
		GfxImage* image;
		GfxRenderTargetSurface surface;
		unsigned int width;
		unsigned int height;
	};

	struct GfxLodRamp
	{
		float scale;
		float bias;
	};

	struct __declspec(align(4)) GfxLodParms
	{
		float origin[3];
		GfxLodRamp ramp[2];
		bool valid;
	};

	struct __declspec(align(8)) r_globals_t
	{
		GfxViewParms identityViewParms;
		bool inFrame;
		bool registered;
		bool forbidDelayLoadImages;
		bool ignorePrecacheErrors;
		float viewOrg[3];
		float viewDir[3];
		unsigned int frontEndFrameCount;
		int totalImageMemory;
		void* Material_materialHashTable[4096];
		char pad[40];
		GfxLodParms lodParms;
		GfxLodParms correctedLodParms;
	};

	struct r_global_permanent_t
	{
		Material* sortedMaterials[4096];
		int needSortMaterials;
		int materialCount;
		GfxImage* whiteImage;
		GfxImage* blackImage;
		GfxImage* grayImage;
		GfxImage* identityNormalMapImage;
		GfxImage* specularityImage;
		GfxImage* outdoorImage;
		GfxImage* pixelCostColorCodeImage;
		GfxLightDef* dlightDef;
		Material* defaultMaterial;
		Material* whiteMaterial;
		Material* additiveMaterial;
		Material* pointMaterial;
		Material* lineMaterial;
		Material* lineMaterialNoDepth;
		Material* clearAlphaStencilMaterial;
		Material* shadowClearMaterial;
		Material* shadowCookieOverlayMaterial;
		Material* shadowCookieBlurMaterial;
		Material* shadowCasterMaterial;
		Material* shadowOverlayMaterial;
		Material* depthPrepassMaterial;
		Material* glareBlindMaterial;
		Material* pixelCostAddDepthAlwaysMaterial;
		Material* pixelCostAddDepthDisableMaterial;
		Material* pixelCostAddDepthEqualMaterial;
		Material* pixelCostAddDepthLessMaterial;
		Material* pixelCostAddDepthWriteMaterial;
		Material* pixelCostAddNoDepthWriteMaterial;
		Material* pixelCostColorCodeMaterial;
		Material* stencilShadowMaterial;
		Material* stencilDisplayMaterial;
		Material* floatZDisplayMaterial;
		Material* colorChannelMixerMaterial;
		Material* frameColorDebugMaterial;
		Material* frameAlphaDebugMaterial;
		GfxImage* rawImage;
		void* world; //GfxWorld*
		int caseTextures_count;
		GfxImage* caseTextures[64];
		Material* feedbackReplaceMaterial;
		Material* feedbackBlendMaterial;
		Material* feedbackFilmBlendMaterial;
		Material* postFxColorMaterial;
		Material* postFxMaterial;
		void* symmetricFilter_0;
		Material* symmetricFilterMaterial[8];
		Material* shellShockBlurredMaterial;
		Material* shellShockFlashedMaterial;
		Material* glowConsistentSetupMaterial;
		Material* glowApplyBloomMaterial;
	};

	struct XModelLodInfo
	{
		float dist; //0x0000 
		std::int16_t numsurfs; //0x0004 
		std::int16_t surfIndex; //0x0006 
		int partBits[4]; //0x0008 
	}; //Size=0x0018

	struct DObjAnimMat
	{
		float quat[4];
		float trans[3];
		float transWeight;
	};

	struct XSurfaceVertexInfo
	{
		__int16 vertCount[4];
		unsigned __int16* vertsBlend;
	};

	union PackedTexCoords
	{
		unsigned int packed;
	};
	
	union PackedUnitVec
	{
		unsigned int packed;
		char array[4];
	};
	
	struct GfxPackedVertex
	{
		float xyz[3];
		float binormalSign;
		GfxColor color;
		PackedTexCoords texCoord;
		PackedUnitVec normal;
		PackedUnitVec tangent;
	};

	struct XRigidVertList
	{
		unsigned __int16 boneOffset;
		unsigned __int16 vertCount;
		unsigned __int16 triOffset;
		unsigned __int16 triCount;
		void* collisionTree; // XSurfaceCollisionTree
	};
	
	struct XSurface
	{
		char tileMode;
		bool deformed;
		unsigned __int16 vertCount;
		unsigned __int16 triCount;
		char zoneHandle;
		unsigned __int16 baseTriIndex;
		unsigned __int16 baseVertIndex;
		unsigned __int16* triIndices;
		XSurfaceVertexInfo vertInfo;
		GfxPackedVertex* verts0;
		unsigned int vertListCount;
		XRigidVertList* vertList;
		int partBits[4];
	};

	struct XModelCollTri_s
	{
		float plane[4];
		float svec[4];
		float tvec[4];
	};
	
	struct XModelCollSurf_s
	{
		XModelCollTri_s* collTris;
		int numCollTris;
		float mins[3];
		float maxs[3];
		int boneIdx;
		int contents;
		int surfFlags;
	};

	struct XBoneInfo
	{
		float bounds[2][3];
		float offset[3];
		float radiusSquared;
	};
	
	struct XModel
	{
		const char* name; //0x0000 
		std::int8_t numBones; //0x0004 
		std::int8_t numRootBones; //0x0005 
		std::int8_t numsurfs; //0x0006 
		std::int8_t lodRampType; //0x0007 
		unsigned __int16* boneNames; //0x0008 
		char* parentList; //0x000C 
		__int16* quats; //0x0010 
		float* trans; //0x0014 
		char* partClassification; //0x0018 
		DObjAnimMat* baseMat; //0x001C 
		XSurface* surfs; //0x0020 XSurface
		Material** materialHandles; //0x0024  Material **
		XModelLodInfo lodInfo[4]; //0x0028 XModelLodInfo
		XModelCollSurf_s* collSurfs; //0x0088 XModelCollSurf_s
		int numCollSurfs; //0x008C 
		int contents; //0x0090 
		XBoneInfo* boneInfo; //0x0094 XBoneInfo
		float radius; //0x0098 
		vec3_t mins; //0x009C 
		vec3_t maxs; //0x00A8 
		std::int16_t numLods; //0x00B4 
		std::int16_t collLod; //0x00B6 
		void* streamInfo; //0x00B8 
		int memUsage; //0x00BC 
		std::int8_t flags; //0x00C0 
		bool bad; //0x00C1 C1
		char pad_0x00C2[0x2]; //0x00C2
	}; STATIC_ASSERT_SIZE(XModel, 0xC4); //Size=0x00C4

	struct GfxVisibleLight
	{
		int drawSurfCount;
		GfxDrawSurf drawSurfs[1024];
	};

	struct DpvsPlane
	{
		float coeffs[4];
		char side[3];
		char pad;
	};

	struct GfxShadowCookie
	{
		DpvsPlane planes[5];
		volatile int drawSurfCount;
		GfxDrawSurf drawSurfs[256];
	};

	struct GfxSkinnedXModelSurfs
	{
		void* firstSurf;
	};

	struct GfxSceneEntityCull
	{
		volatile unsigned int state;
		float mins[3];
		float maxs[3];
		char lods[32];
		GfxSkinnedXModelSurfs skinnedSurfs;
	};

	struct GfxSkinCacheEntry
	{
		unsigned int frameCount;
		int skinnedCachedOffset;
		unsigned __int16 numSkinnedVerts;
		unsigned __int16 ageCount;
	};

	struct clientControllers_t
	{
		float angles[6][3];
		float tag_origin_angles[3];
		float tag_origin_offset[3];
	};

	/* 1082 */
	struct __declspec(align(4)) CEntPlayerInfo
	{
		clientControllers_t* control;
		char tag[6];
	};

	/* 1083 */
	struct CEntTurretAngles
	{
		float pitch;
		float yaw;
	};

	/* 1084 */
	union $7879FE2059B0C294B0A710F7E3BFC2F0
	{
		CEntTurretAngles angles;
		const float* viewAngles;
	};

	/* 1085 */
	struct CEntTurretInfo
	{
		$7879FE2059B0C294B0A710F7E3BFC2F0 ___u0;
		float barrelPitch;
		bool playerUsing;
		char tag_aim;
		char tag_aim_animated;
		char tag_flash;
	};

#pragma warning( push )
#pragma warning( disable : 4359 )
	struct __declspec(align(2)) CEntVehicleInfo
	{
		__int16 pitch;
		__int16 yaw;
		__int16 roll;
		__int16 barrelPitch;
		float barrelRoll;
		__int16 steerYaw;
		float time;
		unsigned __int16 wheelFraction[4];
		char wheelBoneIndex[4];
		char tag_body;
		char tag_turret;
		char tag_barrel;
	};
#pragma warning( pop )

	struct FxBoltAndSortOrder
	{
		unsigned __int32 dobjHandle : 12;
		unsigned __int32 temporalBits : 2;
		unsigned __int32 boneIndex : 10;
		unsigned __int32 sortOrder : 8;
	};

	struct FxSpatialFrame
	{
		float quat[4];
		float origin[3];
	};

	struct FxEffect
	{
		void* FxEffectDef_def;
		volatile int status;
		unsigned __int16 firstElemHandle[3];
		unsigned __int16 firstSortedElemHandle;
		unsigned __int16 firstTrailHandle;
		unsigned __int16 randomSeed;
		unsigned __int16 owner;
		unsigned __int16 packedLighting;
		FxBoltAndSortOrder boltAndSortOrder;
		volatile int frameCount;
		int msecBegin;
		int msecLastUpdate;
		FxSpatialFrame frameAtSpawn;
		FxSpatialFrame frameNow;
		FxSpatialFrame framePrev;
		float distanceTraveled;
	};

	/* 1090 */
	struct CEntFx
	{
		int triggerTime;
		FxEffect* effect;
	};

	union $EAE81CC4C8A7A2F7E95AA09AC9F9F9C0
	{
		CEntPlayerInfo player;
		CEntTurretInfo turret;
		CEntVehicleInfo vehicle;
		CEntFx fx;
	};

	struct cpose_t
	{
		unsigned __int16 lightingHandle;
		char eType;
		char eTypeUnion;
		char localClientNum;
		int cullIn;
		char isRagdoll;
		int ragdollHandle;
		int killcamRagdollHandle;
		int physObjId;
		float origin[3];
		float angles[3];
		GfxSkinCacheEntry skinCacheEntry;
		$EAE81CC4C8A7A2F7E95AA09AC9F9F9C0 ___u12;
	};

	union GfxSceneEntityInfo
	{
		cpose_t* pose;
		unsigned __int16* cachedLightingHandle;
	};

	struct __declspec(align(4)) GfxSceneEntity
	{
		float lightingOrigin[3];
		GfxScaledPlacement placement;
		GfxSceneEntityCull cull;
		unsigned __int16 gfxEntIndex;
		unsigned __int16 entnum;
		void* DObj_s_obj;
		GfxSceneEntityInfo info;
		char reflectionProbeIndex;
	};

	struct XModelDrawInfo
	{
		unsigned __int16 lod;
		unsigned __int16 surfId;
	};

	struct __declspec(align(4)) GfxSceneModel
	{
		XModelDrawInfo info;
		XModel* model;
		void* DObj_s_obj;
		GfxScaledPlacement placement;
		unsigned __int16 gfxEntIndex;
		unsigned __int16 entnum;
		float radius;
		unsigned __int16* cachedLightingHandle;
		float lightingOrigin[3];
		char reflectionProbeIndex;
	};

	struct BModelDrawInfo
	{
		unsigned __int16 surfId;
	};

	struct GfxBrushModelWritable
	{
		float mins[3];
		float maxs[3];
	};

	struct __declspec(align(4)) GfxBrushModel
	{
		GfxBrushModelWritable writable;
		float bounds[2][3];
		unsigned __int16 surfaceCount;
		unsigned __int16 startSurfIndex;
		unsigned __int16 surfaceCountNoDecal;
	};

	struct __declspec(align(4)) GfxSceneBrush
	{
		BModelDrawInfo info;
		unsigned __int16 entnum;
		GfxBrushModel* bmodel;
		GfxPlacement placement;
		char reflectionProbeIndex;
	};

	union GfxEntCellRefInfo
	{
		float radius;
		GfxBrushModel* bmodel;
	};

	struct GfxSceneDpvs
	{
		unsigned int localClientNum;
		char* entVisData[7];
		unsigned __int16* sceneXModelIndex;
		unsigned __int16* sceneDObjIndex;
		GfxEntCellRefInfo* entInfo[4];
	};

#pragma warning( push )
#pragma warning( disable : 4324 )
	struct __declspec(align(64)) GfxScene
	{
		GfxDrawSurf bspDrawSurfs[8192];
		GfxDrawSurf smodelDrawSurfsLight[8192];
		GfxDrawSurf entDrawSurfsLight[8192];
		GfxDrawSurf bspDrawSurfsDecal[512];
		GfxDrawSurf smodelDrawSurfsDecal[512];
		GfxDrawSurf entDrawSurfsDecal[512];
		GfxDrawSurf bspDrawSurfsEmissive[8192];
		GfxDrawSurf smodelDrawSurfsEmissive[8192];
		GfxDrawSurf entDrawSurfsEmissive[8192];
		GfxDrawSurf fxDrawSurfsEmissive[8192];
		GfxDrawSurf fxDrawSurfsEmissiveAuto[8192];
		GfxDrawSurf fxDrawSurfsEmissiveDecal[8192];
		GfxDrawSurf bspSunShadowDrawSurfs0[4096];
		GfxDrawSurf smodelSunShadowDrawSurfs0[4096];
		GfxDrawSurf entSunShadowDrawSurfs0[4096];
		GfxDrawSurf bspSunShadowDrawSurfs1[8192];
		GfxDrawSurf smodelSunShadowDrawSurfs1[8192];
		GfxDrawSurf entSunShadowDrawSurfs1[8192];
		GfxDrawSurf bspSpotShadowDrawSurfs0[256];
		GfxDrawSurf smodelSpotShadowDrawSurfs0[256];
		GfxDrawSurf entSpotShadowDrawSurfs0[512];
		GfxDrawSurf bspSpotShadowDrawSurfs1[256];
		GfxDrawSurf smodelSpotShadowDrawSurfs1[256];
		GfxDrawSurf entSpotShadowDrawSurfs1[512];
		GfxDrawSurf bspSpotShadowDrawSurfs2[256];
		GfxDrawSurf smodelSpotShadowDrawSurfs2[256];
		GfxDrawSurf entSpotShadowDrawSurfs2[512];
		GfxDrawSurf bspSpotShadowDrawSurfs3[256];
		GfxDrawSurf smodelSpotShadowDrawSurfs3[256];
		GfxDrawSurf entSpotShadowDrawSurfs3[512];
		GfxDrawSurf shadowDrawSurfs[512];
		unsigned int shadowableLightIsUsed[32];
		int maxDrawSurfCount[34];
		volatile int drawSurfCount[34];
		GfxDrawSurf* drawSurfs[34];
		GfxDrawSurf fxDrawSurfsLight[8192];
		GfxDrawSurf fxDrawSurfsLightAuto[8192];
		GfxDrawSurf fxDrawSurfsLightDecal[8192];
		GfxSceneDef def;
		int addedLightCount;
		GfxLight addedLight[32];
		bool isAddedLightCulled[32];
		float dynamicSpotLightNearPlaneOffset;
		GfxVisibleLight visLight[4];
		GfxVisibleLight visLightShadow[1];
		GfxShadowCookie cookie[24];
		unsigned int* entOverflowedDrawBuf;
		volatile int sceneDObjCount;
		GfxSceneEntity sceneDObj[512];
		char sceneDObjVisData[7][512];
		volatile int sceneModelCount;
		GfxSceneModel sceneModel[1024];
		char sceneModelVisData[7][1024];
		volatile int sceneBrushCount;
		GfxSceneBrush sceneBrush[512];
		char sceneBrushVisData[3][512];
		unsigned int sceneDynModelCount;
		unsigned int sceneDynBrushCount;
		DpvsPlane shadowFarPlane[2];
		DpvsPlane shadowNearPlane[2];
		GfxSceneDpvs dpvs;
	}; STATIC_ASSERT_SIZE(GfxScene, 0x154D00);
#pragma warning( pop )

	struct model_inst
	{
		vec4_t angles; //0x0000 
		vec3_t origin; //0x0010 
		float modelscale; //0x001C 
		char pad_0x0020[0x4]; //0x0020
		XModel* model; //0x0024 
		int random_one; //0x0028 
	}; STATIC_ASSERT_SIZE(model_inst, 0x2C); //Size=0x002C

	enum MaterialVertexDeclType
	{
		VERTDECL_GENERIC = 0x0,
		VERTDECL_PACKED = 0x1,
		VERTDECL_WORLD = 0x2,
		VERTDECL_WORLD_T1N0 = 0x3,
		VERTDECL_WORLD_T1N1 = 0x4,
		VERTDECL_WORLD_T2N0 = 0x5,
		VERTDECL_WORLD_T2N1 = 0x6,
		VERTDECL_WORLD_T2N2 = 0x7,
		VERTDECL_WORLD_T3N0 = 0x8,
		VERTDECL_WORLD_T3N1 = 0x9,
		VERTDECL_WORLD_T3N2 = 0xA,
		VERTDECL_WORLD_T4N0 = 0xB,
		VERTDECL_WORLD_T4N1 = 0xC,
		VERTDECL_WORLD_T4N2 = 0xD,
		VERTDECL_POS_TEX = 0xE,
		VERTDECL_STATICMODELCACHE = 0xF,
		VERTDECL_COUNT = 0x10,
	};

	struct gfxVertexSteamsUnk
	{
		unsigned int stride;
		int* vb;
		unsigned int offset;
	};

	struct GfxCmdBufPrimState
	{
		IDirect3DDevice9* device;
		IDirect3DIndexBuffer9* indexBuffer;
		MaterialVertexDeclType vertDeclType;
		gfxVertexSteamsUnk streams[2];
		IDirect3DVertexDeclaration9* vertexDecl;
	}; STATIC_ASSERT_SIZE(GfxCmdBufPrimState, 0x28);

	struct MaterialStreamRouting
	{
		char source;
		char dest;
	};

	struct MaterialVertexStreamRouting
	{
		MaterialStreamRouting data[16];
		void* decl[16];
	};

	struct MaterialVertexDeclaration
	{
		char streamCount;
		bool hasOptionalSource;
		bool isLoaded;
		MaterialVertexStreamRouting routing;
	};
	
	struct GfxVertexShaderLoadDef
	{
		unsigned int* program;
		unsigned __int16 programSize;
		unsigned __int16 loadForRenderer;
	};

	struct MaterialVertexShaderProgram
	{
		void* vs;
		GfxVertexShaderLoadDef loadDef;
	};

	struct MaterialVertexShader
	{
		const char* name;
		MaterialVertexShaderProgram prog;
	};

	struct GfxPixelShaderLoadDef
	{
		unsigned int* program;
		unsigned __int16 programSize;
		unsigned __int16 loadForRenderer;
	};

	struct MaterialPixelShaderProgram
	{
		void* ps;
		GfxPixelShaderLoadDef loadDef;
	};

	struct MaterialPixelShader
	{
		const char* name;
		MaterialPixelShaderProgram prog;
	};

	enum GfxRenderCommand
	{
		RC_END_OF_LIST = 0x0,
		RC_SET_MATERIAL_COLOR = 0x1,
		RC_SET_LIGHT_COLOR = 0x2,
		RC_SAVE_SCREEN = 0x3,
		RC_SAVE_SCREEN_SECTION = 0x4,
		RC_CLEAR_SCREEN = 0x5,
		RC_BEGIN_VIEW = 0x6,
		RC_SET_VIEWPORT = 0x7,
		RC_FIRST_NONCRITICAL = 0x8,
		RC_STRETCH_PIC = 0x8,
		RC_STRETCH_PIC_FLIP_ST = 0x9,
		RC_STRETCH_PIC_ROTATE_XY = 0xA,
		RC_STRETCH_PIC_ROTATE_ST = 0xB,
		RC_STRETCH_RAW = 0xC,
		RC_DRAW_QUAD_PIC = 0xD,
		RC_DRAW_FULLSCREEN_COLORED_QUAD = 0xE,
		RC_DRAW_TEXT_2D = 0xF,
		RC_DRAW_TEXT_3D = 0x10,
		RC_BLEND_SAVED_SCREEN_BLURRED = 0x11,
		RC_BLEND_SAVED_SCREEN_FLASHED = 0x12,
		RC_DRAW_POINTS = 0x13,
		RC_DRAW_LINES = 0x14,
		RC_DRAW_TRIANGLES = 0x15,
		RC_DRAW_EDITOR_SKINNEDCACHED = 0x16,
		RC_SET_CUSTOM_CONSTANT = 0x17,
		RC_PROJECTION_SET = 0x18,
		RC_COUNT = 0x18,
	};

	enum GfxProjectionTypes
	{
		GFX_PROJECTION_2D = 0x0,
		GFX_PROJECTION_3D = 0x1,
	};

	struct GfxCmdProjectionSet
	{
		GfxCmdHeader header;
		GfxProjectionTypes projection;
	}; STATIC_ASSERT_SIZE(GfxCmdProjectionSet, 8);

	struct GfxCmdSetCustomConstant
	{
		GfxCmdHeader header;
		unsigned int type;
		float vec[4];
	}; STATIC_ASSERT_SIZE(GfxCmdSetCustomConstant, 24);

	struct SunLightParseParams
	{
		char name[64];
		float ambientScale;
		float ambientColor[3];
		float diffuseFraction;
		float sunLight;
		float sunColor[3];
		float diffuseColor[3];
		bool diffuseColorHasBeenSet;
		float angles[3];
	};

	enum ShaderCodeConstants : unsigned __int16
	{
		CONST_SRC_CODE_MAYBE_DIRTY_PS_BEGIN = 0x0,
		CONST_SRC_CODE_LIGHT_POSITION = 0x0,
		CONST_SRC_CODE_LIGHT_DIFFUSE = 0x1,
		CONST_SRC_CODE_LIGHT_SPECULAR = 0x2,
		CONST_SRC_CODE_LIGHT_SPOTDIR = 0x3,
		CONST_SRC_CODE_LIGHT_SPOTFACTORS = 0x4,
		CONST_SRC_CODE_NEARPLANE_ORG = 0x5,
		CONST_SRC_CODE_NEARPLANE_DX = 0x6,
		CONST_SRC_CODE_NEARPLANE_DY = 0x7,
		CONST_SRC_CODE_SHADOW_PARMS = 0x8,
		CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET = 0x9,
		CONST_SRC_CODE_RENDER_TARGET_SIZE = 0xA,
		CONST_SRC_CODE_LIGHT_FALLOFF_PLACEMENT = 0xB,
		CONST_SRC_CODE_DOF_EQUATION_VIEWMODEL_AND_FAR_BLUR = 0xC,
		CONST_SRC_CODE_DOF_EQUATION_SCENE = 0xD,
		CONST_SRC_CODE_DOF_LERP_SCALE = 0xE,
		CONST_SRC_CODE_DOF_LERP_BIAS = 0xF,
		CONST_SRC_CODE_DOF_ROW_DELTA = 0x10,
		CONST_SRC_CODE_PARTICLE_CLOUD_COLOR = 0x11,
		CONST_SRC_CODE_GAMETIME = 0x12,
		CONST_SRC_CODE_MAYBE_DIRTY_PS_END = 0x13,
		CONST_SRC_CODE_ALWAYS_DIRTY_PS_BEGIN = 0x13,
		CONST_SRC_CODE_PIXEL_COST_FRACS = 0x13,
		CONST_SRC_CODE_PIXEL_COST_DECODE = 0x14,
		CONST_SRC_CODE_FILTER_TAP_0 = 0x15,
		CONST_SRC_CODE_FILTER_TAP_1 = 0x16,
		CONST_SRC_CODE_FILTER_TAP_2 = 0x17,
		CONST_SRC_CODE_FILTER_TAP_3 = 0x18,
		CONST_SRC_CODE_FILTER_TAP_4 = 0x19,
		CONST_SRC_CODE_FILTER_TAP_5 = 0x1A,
		CONST_SRC_CODE_FILTER_TAP_6 = 0x1B,
		CONST_SRC_CODE_FILTER_TAP_7 = 0x1C,
		CONST_SRC_CODE_COLOR_MATRIX_R = 0x1D,
		CONST_SRC_CODE_COLOR_MATRIX_G = 0x1E,
		CONST_SRC_CODE_COLOR_MATRIX_B = 0x1F,
		CONST_SRC_CODE_ALWAYS_DIRTY_PS_END = 0x20,
		CONST_SRC_CODE_NEVER_DIRTY_PS_BEGIN = 0x20,
		CONST_SRC_CODE_SHADOWMAP_SWITCH_PARTITION = 0x20,
		CONST_SRC_CODE_SHADOWMAP_SCALE = 0x21,
		CONST_SRC_CODE_ZNEAR = 0x22,
		CONST_SRC_CODE_SUN_POSITION = 0x23,
		CONST_SRC_CODE_SUN_DIFFUSE = 0x24,
		CONST_SRC_CODE_SUN_SPECULAR = 0x25,
		CONST_SRC_CODE_LIGHTING_LOOKUP_SCALE = 0x26,
		CONST_SRC_CODE_DEBUG_BUMPMAP = 0x27,
		CONST_SRC_CODE_MATERIAL_COLOR = 0x28,
		CONST_SRC_CODE_FOG = 0x29,
		CONST_SRC_CODE_FOG_COLOR = 0x2A,
		CONST_SRC_CODE_GLOW_SETUP = 0x2B,
		CONST_SRC_CODE_GLOW_APPLY = 0x2C,
		CONST_SRC_CODE_COLOR_BIAS = 0x2D,
		CONST_SRC_CODE_COLOR_TINT_BASE = 0x2E,
		CONST_SRC_CODE_COLOR_TINT_DELTA = 0x2F,
		CONST_SRC_CODE_OUTDOOR_FEATHER_PARMS = 0x30,
		CONST_SRC_CODE_ENVMAP_PARMS = 0x31,
		CONST_SRC_CODE_SPOT_SHADOWMAP_PIXEL_ADJUST = 0x32,
		CONST_SRC_CODE_CLIP_SPACE_LOOKUP_SCALE = 0x33,
		CONST_SRC_CODE_CLIP_SPACE_LOOKUP_OFFSET = 0x34,
		CONST_SRC_CODE_PARTICLE_CLOUD_MATRIX = 0x35,
		CONST_SRC_CODE_DEPTH_FROM_CLIP = 0x36,
		CONST_SRC_CODE_CODE_MESH_ARG_0 = 0x37,
		CONST_SRC_CODE_CODE_MESH_ARG_1 = 0x38,
		CONST_SRC_CODE_CODE_MESH_ARG_LAST = 0x38,
		CONST_SRC_CODE_BASE_LIGHTING_COORDS = 0x39,
		CONST_SRC_CODE_NEVER_DIRTY_PS_END = 0x3A,
		CONST_SRC_CODE_COUNT_FLOAT4 = 0x3A,
		CONST_SRC_FIRST_CODE_MATRIX = 0x3A,
		CONST_SRC_CODE_WORLD_MATRIX = 0x3A,
		CONST_SRC_CODE_INVERSE_WORLD_MATRIX = 0x3B,
		CONST_SRC_CODE_TRANSPOSE_WORLD_MATRIX = 0x3C,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_MATRIX = 0x3D,
		CONST_SRC_CODE_VIEW_MATRIX = 0x3E,
		CONST_SRC_CODE_INVERSE_VIEW_MATRIX = 0x3F,
		CONST_SRC_CODE_TRANSPOSE_VIEW_MATRIX = 0x40,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_VIEW_MATRIX = 0x41,
		CONST_SRC_CODE_PROJECTION_MATRIX = 0x42,
		CONST_SRC_CODE_INVERSE_PROJECTION_MATRIX = 0x43,
		CONST_SRC_CODE_TRANSPOSE_PROJECTION_MATRIX = 0x44,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_PROJECTION_MATRIX = 0x45,
		CONST_SRC_CODE_WORLD_VIEW_MATRIX = 0x46,
		CONST_SRC_CODE_INVERSE_WORLD_VIEW_MATRIX = 0x47,
		CONST_SRC_CODE_TRANSPOSE_WORLD_VIEW_MATRIX = 0x48,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX = 0x49,
		CONST_SRC_CODE_VIEW_PROJECTION_MATRIX = 0x4A,
		CONST_SRC_CODE_INVERSE_VIEW_PROJECTION_MATRIX = 0x4B,
		CONST_SRC_CODE_TRANSPOSE_VIEW_PROJECTION_MATRIX = 0x4C,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_VIEW_PROJECTION_MATRIX = 0x4D,
		CONST_SRC_CODE_WORLD_VIEW_PROJECTION_MATRIX = 0x4E,
		CONST_SRC_CODE_INVERSE_WORLD_VIEW_PROJECTION_MATRIX = 0x4F,
		CONST_SRC_CODE_TRANSPOSE_WORLD_VIEW_PROJECTION_MATRIX = 0x50,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_VIEW_PROJECTION_MATRIX = 0x51,
		CONST_SRC_CODE_SHADOW_LOOKUP_MATRIX = 0x52,
		CONST_SRC_CODE_INVERSE_SHADOW_LOOKUP_MATRIX = 0x53,
		CONST_SRC_CODE_TRANSPOSE_SHADOW_LOOKUP_MATRIX = 0x54,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_SHADOW_LOOKUP_MATRIX = 0x55,
		CONST_SRC_CODE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x56,
		CONST_SRC_CODE_INVERSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x57,
		CONST_SRC_CODE_TRANSPOSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x58,
		CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 0x59,
		CONST_SRC_TOTAL_COUNT = 0x5A,
		CONST_SRC_NONE = 0x5B,
	};

	struct MaterialArgumentCodeConst
	{
		ShaderCodeConstants index; //unsigned __int16 index;
		char firstRow;
		char rowCount;
	};

	union MaterialArgumentDef
	{
		const float* literalConst;
		MaterialArgumentCodeConst codeConst;
		unsigned int codeSampler;
		unsigned int nameHash;
	};

	struct MaterialShaderArgument
	{
		unsigned __int16 type;
		unsigned __int16 dest;
		MaterialArgumentDef u;
	};

	struct MaterialPass
	{
		MaterialVertexDeclaration* vertexDecl;
		MaterialVertexShader* vertexShader;
		MaterialPixelShader* pixelShader;
		char perPrimArgCount;
		char perObjArgCount;
		char stableArgCount;
		char customSamplerFlags;
		MaterialShaderArgument* args;
	};

	struct MaterialTechnique
	{
		const char* name;
		unsigned __int16 flags;
		unsigned __int16 passCount;
		MaterialPass passArray[1];
	};

	enum GfxDepthRangeType
	{
		GFX_DEPTH_RANGE_SCENE = 0x0,
		GFX_DEPTH_RANGE_VIEWMODEL = 0x2,
		GFX_DEPTH_RANGE_FULL = 0xFFFFFFFF,
	};

	struct GfxCmdBufState
	{
		char refSamplerState[16];
		unsigned int samplerState[16];
		GfxTexture* samplerTexture[16];
		GfxCmdBufPrimState prim;
		Material* material;
		MaterialTechniqueType techType;
		MaterialTechnique* technique;
		MaterialPass* pass;
		unsigned int passIndex;
		GfxDepthRangeType depthRangeType;
		float depthRangeNear;
		float depthRangeFar;
		unsigned __int64 vertexShaderConstState[32];
		unsigned __int64 pixelShaderConstState[256];
		char alphaRef;
		unsigned int refStateBits[2];
		unsigned int activeStateBits[2];
		MaterialPixelShader* pixelShader;
		MaterialVertexShader* vertexShader;
		GfxViewport viewport;
		GfxRenderTargetId renderTargetId;
		Material* origMaterial;
		MaterialTechniqueType origTechType;
	}; STATIC_ASSERT_SIZE(GfxCmdBufState, 0xA10);

	struct GfxEntity
	{
		unsigned int renderFxFlags;
		float materialTime;
	};

	struct GfxLightingInfo
	{
		char primaryLightIndex;
		char reflectionProbeIndex;
	};

	struct modelLightGlobals_s
	{
		float invImageHeight;
		unsigned int xmodel;
		unsigned int totalEntryLimit;
		unsigned int entryBitsY;
		unsigned int imageHeight;
		GfxEntity* entities;
		unsigned int modFrameCount;
		GfxImage* lightImages[2];
		GfxImage* image;
		unsigned int xmodelEntryLimit;
		GfxLightingInfo* lightingInfo;
		float(*lightingOrigins)[3];
		int allocModelFail;
		unsigned int* pixelFreeBits[4];
		unsigned int* prevPrevPixelFreeBits;
		unsigned int* prevPixelFreeBits;
		unsigned int* currPixelFreeBits;
		unsigned int pixelFreeBitsSize;
		unsigned int pixelFreeBitsWordCount;
		unsigned int pixelFreeRover;
		_D3DLOCKED_BOX lockedBox;
	};

	
	struct filter_material_t
	{
		const char* name;
		int index;
	};

	struct undo_s
	{
		double time;
		int id;
		int done;
		char* operation;
		brush_t brushlist;
		entity_s ent;
		undo_s* prev;
		undo_s* next;
	};

	struct SCommandInfo
	{
		const char* m_strCommand;
		unsigned int m_nKey;
		unsigned int m_nModifiers;
		unsigned int m_nCommand;
	};

	struct SCommandInfoHotkey
	{
		std::string m_strCommand;
		unsigned int m_nKey;
		unsigned int m_nModifiers;
	};

	struct trace_t
	{
		brush_t* brush;
		face_t* face;
		int xx1;
		int xx2;
		int xx3;
		int xx4;
		int xx5;
		int xx6;
		int xx7;
		int xx8;
		int xx9;
		int xx10;
		int xx11;
		int xx12;
		int xx13;
		int xx14;
		int xx15;
		float dist;
		bool selected;
		vec3_t some_point;
	};

	struct orientation_t
	{
		float origin[3];
		float axis[3][3];
	};

	
	enum ENTITY_DEFS
	{
		ENTITY_WIREFRAME		= 0x1,
		ENTITY_SKIN_MODEL		= 0x10,
		ENTITY_SELECTED_ONLY	= 0x100,
		ENTITY_BOXED			= 0x1000,
		ENTITY_SKINNED			= 0x10000,
	};

	enum E_ENTITYWND_HWNDS
	{
		ENTWND_LIST,
		ENTWND_COMMENT,
		ENTWND_CHECK1,
		ENTWND_CHECK2,
		ENTWND_CHECK3,
		ENTWND_CHECK4,
		ENTWND_CHECK5,
		ENTWND_CHECK6,
		ENTWND_CHECK7,
		ENTWND_CHECK8,
		ENTWND_CHECK_EASY,
		ENTWND_CHECK_MEDIUM,
		ENTWND_CHECK_HARD,
		ENTWND_CHECK_DEATHMATCH,
		ENTWND_PROBS_VALUEPAIR,
		ENTWND_DIR_0,
		ENTWND_DIR_45,
		ENTWND_DIR_90,
		ENTWND_DIR_135,
		ENTWND_DIR_180,
		ENTWND_DIR_225,
		ENTWND_DIR_270,
		ENTWND_DIR_315,
		ENTWND_DIR_UP,
		ENTWND_DIR_DOWN,
		ENTWND_DELETE,
		ENTWND_KEYLABEL,
		ENTWND_KEYFIELD,
		ENTWND_VALUELABEL,
		ENTWND_VALUEFIELD,
		ENTWND_ASSIGNMODEL,
		ENTWND_TAB,
	};
	
	/*
	struct entitywnd_hwnd_list_s
	{
		HWND entlist_listbox;
		HWND comment;
		HWND entcheck_1;
		HWND entcheck_2;
		HWND entcheck_3;
		HWND entcheck_4;
		HWND entcheck_5;
		HWND entcheck_6;
		HWND entcheck_7;
		HWND entcheck_8;
		HWND autocheckbox_easy;
		HWND autocheckbox_medium;
		HWND autocheckbox_hard;
		HWND autocheckbox_deathmatch;
		HWND entprobs_valuepair;
		HWND entdir0;
		HWND entdir45;
		HWND entdir90;
		HWND entdir135;
		HWND entdir180;
		HWND entdir225;
		HWND entdir270;
		HWND entdir315;
		HWND entdirup;
		HWND entdirdown;
		HWND entdelete;
		HWND entkeylabel;
		HWND entkeyfield;
		HWND entvaluelabel;
		HWND entvaluefield;
		HWND entassignmodel;
		HWND enttab;
	};*/

	enum WindowMessages : UINT
	{
		_WM_CREATE = 0x1,
		_WM_DESTROY = 0x2,
		_WM_MOVE = 0x3,
		_WM_SIZE = 0x5,
		_WM_ACTIVATE = 0x6,
		_WM_SETFOCUS = 0x7,
		_WM_KILLFOCUS = 0x8,
		_WM_ENABLE = 0xA,
		_WM_SETREDRAW = 0xB,
		_WM_SETTEXT = 0xC,
		_WM_GETTEXT = 0xD,
		_WM_GETTEXTLENGTH = 0xE,
		_WM_PAINT = 0xF,
		_WM_CLOSE = 0x10,
		_WM_QUERYENDSESSION = 0x11,
		_WM_QUERYOPEN = 0x13,
		_WM_ENDSESSION = 0x16,
		_WM_QUIT = 0x12,
		_WM_ERASEBKGND = 0x14,
		_WM_SYSCOLORCHANGE = 0x15,
		_WM_SHOWWINDOW = 0x18,
		_WM_WININICHANGE = 0x1A,
		_WM_DEVMODECHANGE = 0x1B,
		_WM_ACTIVATEAPP = 0x1C,
		_WM_FONTCHANGE = 0x1D,
		_WM_TIMECHANGE = 0x1E,
		_WM_CANCELMODE = 0x1F,
		_WM_SETCURSOR = 0x20,
		_WM_MOUSEACTIVATE = 0x21,
		_WM_CHILDACTIVATE = 0x22,
		_WM_QUEUESYNC = 0x23,
		_WM_GETMINMAXINFO = 0x24,
		_WM_PAINTICON = 0x26,
		_WM_ICONERASEBKGND = 0x27,
		_WM_NEXTDLGCTL = 0x28,
		_WM_SPOOLERSTATUS = 0x2A,
		_WM_DRAWITEM = 0x2B,
		_WM_MEASUREITEM = 0x2C,
		_WM_DELETEITEM = 0x2D,
		_WM_VKEYTOITEM = 0x2E,
		_WM_CHARTOITEM = 0x2F,
		_WM_SETFONT = 0x30,
		_WM_GETFONT = 0x31,
		_WM_SETHOTKEY = 0x32,
		_WM_GETHOTKEY = 0x33,
		_WM_QUERYDRAGICON = 0x37,
		_WM_COMPAREITEM = 0x39,
		_WM_GETOBJECT = 0x3D,
		_WM_COMPACTING = 0x41,
		_WM_COMMNOTIFY = 0x44,
		_WM_WINDOWPOSCHANGING = 0x46,
		_WM_WINDOWPOSCHANGED = 0x47,
		_WM_POWER = 0x48,
		_WM_COPYDATA = 0x4A,
		_WM_CANCELJOURNAL = 0x4B,
		_WM_NOTIFY = 0x4E,
		_WM_INPUTLANGCHANGEREQUEST = 0x50,
		_WM_INPUTLANGCHANGE = 0x51,
		_WM_TCARD = 0x52,
		_WM_HELP = 0x53,
		_WM_USERCHANGED = 0x54,
		_WM_NOTIFYFORMAT = 0x55,
		_WM_CONTEXTMENU = 0x7B,
		_WM_STYLECHANGING = 0x7C,
		_WM_STYLECHANGED = 0x7D,
		_WM_DISPLAYCHANGE = 0x7E,
		_WM_GETICON = 0x7F,
		_WM_SETICON = 0x80,
		_WM_NCCREATE = 0x81,
		_WM_NCDESTROY = 0x82,
		_WM_NCCALCSIZE = 0x83,
		_WM_NCHITTEST = 0x84,
		_WM_NCPAINT = 0x85,
		_WM_NCACTIVATE = 0x86,
		_WM_GETDLGCODE = 0x87,
		_WM_SYNCPAINT = 0x88,
		_WM_NCMOUSEMOVE = 0xA0,
		_WM_NCLBUTTONDOWN = 0xA1,
		_WM_NCLBUTTONUP = 0xA2,
		_WM_NCLBUTTONDBLCLK = 0xA3,
		_WM_NCRBUTTONDOWN = 0xA4,
		_WM_NCRBUTTONUP = 0xA5,
		_WM_NCRBUTTONDBLCLK = 0xA6,
		_WM_NCMBUTTONDOWN = 0xA7,
		_WM_NCMBUTTONUP = 0xA8,
		_WM_NCMBUTTONDBLCLK = 0xA9,
		_WM_NCXBUTTONDOWN = 0xAB,
		_WM_NCXBUTTONUP = 0xAC,
		_WM_NCXBUTTONDBLCLK = 0xAD,
		_WM_INPUT_DEVICE_CHANGE = 0xFE,
		_WM_INPUT = 0xFF,
		_WM_KEYFIRST = 0x100,
		_WM_KEYDOWN = 0x100,
		_WM_KEYUP = 0x101,
		_WM_CHAR = 0x102,
		_WM_DEADCHAR = 0x103,
		_WM_SYSKEYDOWN = 0x104,
		_WM_SYSKEYUP = 0x105,
		_WM_SYSCHAR = 0x106,
		_WM_SYSDEADCHAR = 0x107,
		_WM_UNICHAR = 0x109,
		_WM_KEYLAST = 0x109,
		_WM_IME_STARTCOMPOSITION = 0x10D,
		_WM_IME_ENDCOMPOSITION = 0x10E,
		_WM_IME_COMPOSITION = 0x10F,
		_WM_IME_KEYLAST = 0x10F,
		_WM_INITDIALOG = 0x110,
		_WM_COMMAND = 0x111,
		_WM_SYSCOMMAND = 0x112,
		_WM_TIMER = 0x113,
		_WM_HSCROLL = 0x114,
		_WM_VSCROLL = 0x115,
		_WM_INITMENU = 0x116,
		_WM_INITMENUPOPUP = 0x117,
		_WM_MENUSELECT = 0x11F,
		_WM_MENUCHAR = 0x120,
		_WM_ENTERIDLE = 0x121,
		_WM_MENURBUTTONUP = 0x122,
		_WM_MENUDRAG = 0x123,
		_WM_MENUGETOBJECT = 0x124,
		_WM_UNINITMENUPOPUP = 0x125,
		_WM_MENUCOMMAND = 0x126,
		_WM_CHANGEUISTATE = 0x127,
		_WM_UPDATEUISTATE = 0x128,
		_WM_QUERYUISTATE = 0x129,
		_WM_CTLCOLORMSGBOX = 0x132,
		_WM_CTLCOLOREDIT = 0x133,
		_WM_CTLCOLORLISTBOX = 0x134,
		_WM_CTLCOLORBTN = 0x135,
		_WM_CTLCOLORDLG = 0x136,
		_WM_CTLCOLORSCROLLBAR = 0x137,
		_WM_CTLCOLORSTATIC = 0x138,
		_MN_GETHMENU = 0x1E1,
		_WM_MOUSEFIRST = 0x200,
		_WM_MOUSEMOVE = 0x200,
		_WM_LBUTTONDOWN = 0x201,
		_WM_LBUTTONUP = 0x202,
		_WM_LBUTTONDBLCLK = 0x203,
		_WM_RBUTTONDOWN = 0x204,
		_WM_RBUTTONUP = 0x205,
		_WM_RBUTTONDBLCLK = 0x206,
		_WM_MBUTTONDOWN = 0x207,
		_WM_MBUTTONUP = 0x208,
		_WM_MBUTTONDBLCLK = 0x209,
		_WM_MOUSEWHEEL = 0x20A,
		_WM_XBUTTONDOWN = 0x20B,
		_WM_XBUTTONUP = 0x20C,
		_WM_XBUTTONDBLCLK = 0x20D,
		_WM_MOUSEHWHEEL = 0x20E,
		_WM_PARENTNOTIFY = 0x210,
		_WM_ENTERMENULOOP = 0x211,
		_WM_EXITMENULOOP = 0x212,
		_WM_NEXTMENU = 0x213,
		_WM_SIZING = 0x214,
		_WM_CAPTURECHANGED = 0x215,
		_WM_MOVING = 0x216,
		_WM_POWERBROADCAST = 0x218,
		_WM_DEVICECHANGE = 0x219,
		_WM_MDICREATE = 0x220,
		_WM_MDIDESTROY = 0x221,
		_WM_MDIACTIVATE = 0x222,
		_WM_MDIRESTORE = 0x223,
		_WM_MDINEXT = 0x224,
		_WM_MDIMAXIMIZE = 0x225,
		_WM_MDITILE = 0x226,
		_WM_MDICASCADE = 0x227,
		_WM_MDIICONARRANGE = 0x228,
		_WM_MDIGETACTIVE = 0x229,
		_WM_MDISETMENU = 0x230,
		_WM_ENTERSIZEMOVE = 0x231,
		_WM_EXITSIZEMOVE = 0x232,
		_WM_DROPFILES = 0x233,
		_WM_MDIREFRESHMENU = 0x234,
		_WM_IME_SETCONTEXT = 0x281,
		_WM_IME_NOTIFY = 0x282,
		_WM_IME_CONTROL = 0x283,
		_WM_IME_COMPOSITIONFULL = 0x284,
		_WM_IME_SELECT = 0x285,
		_WM_IME_CHAR = 0x286,
		_WM_IME_REQUEST = 0x288,
		_WM_IME_KEYDOWN = 0x290,
		_WM_IME_KEYUP = 0x291,
		_WM_MOUSEHOVER = 0x2A1,
		_WM_MOUSELEAVE = 0x2A3,
		_WM_NCMOUSEHOVER = 0x2A0,
		_WM_NCMOUSELEAVE = 0x2A2,
		_WM_WTSSESSION_CHANGE = 0x2B1,
		_WM_TABLET_FIRST = 0x2C0,
		_WM_TABLET_LAST = 0x2DF,
		_WM_CUT = 0x300,
		_WM_COPY = 0x301,
		_WM_PASTE = 0x302,
		_WM_CLEAR = 0x303,
		_WM_UNDO = 0x304,
		_WM_RENDERFORMAT = 0x305,
		_WM_RENDERALLFORMATS = 0x306,
		_WM_DESTROYCLIPBOARD = 0x307,
		_WM_DRAWCLIPBOARD = 0x308,
		_WM_PAINTCLIPBOARD = 0x309,
		_WM_VSCROLLCLIPBOARD = 0x30A,
		_WM_SIZECLIPBOARD = 0x30B,
		_WM_ASKCBFORMATNAME = 0x30C,
		_WM_CHANGECBCHAIN = 0x30D,
		_WM_HSCROLLCLIPBOARD = 0x30E,
		_WM_QUERYNEWPALETTE = 0x30F,
		_WM_PALETTEISCHANGING = 0x310,
		_WM_PALETTECHANGED = 0x311,
		_WM_HOTKEY = 0x312,
		_WM_PRINT = 0x317,
		_WM_PRINTCLIENT = 0x318,
		_WM_APPCOMMAND = 0x319,
		_WM_THEMECHANGED = 0x31A,
		_WM_CLIPBOARDUPDATE = 0x31D,
		_WM_DWMCOMPOSITIONCHANGED = 0x31E,
		_WM_DWMNCRENDERINGCHANGED = 0x31F,
		_WM_DWMCOLORIZATIONCOLORCHANGED = 0x320,
		_WM_DWMWINDOWMAXIMIZEDCHANGE = 0x321,
		_WM_GETTITLEBARINFOEX = 0x33F,
		_WM_HANDHELDFIRST = 0x358,
		_WM_HANDHELDLAST = 0x35F,
		_WM_AFXFIRST = 0x360,
		_WM_AFXLAST = 0x37F,
		_WM_PENWINFIRST = 0x380,
		_WM_PENWINLAST = 0x38F,
		_WM_APP = 0x8000,
		_WM_USER = 0x400,
	};
}

#pragma warning(pop)