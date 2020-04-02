#pragma once

#define DVAR_NOFLAG				0				// 0x0000
#define DVAR_ARCHIVE            (1 << 0)        // 0x0001
#define DVAR_USERINFO           (1 << 1)        // 0x0002
#define DVAR_SERVERINFO         (1 << 2)        // 0x0004
#define DVAR_SYSTEMINFO         (1 << 3)        // 0x0008
#define DVAR_INIT               (1 << 4)        // 0x0010
#define DVAR_LATCH              (1 << 5)        // 0x0020
#define DVAR_ROM                (1 << 6)        // 0x0040
#define DVAR_CHEAT              (1 << 7)        // 0x0080
#define DVAR_DEVELOPER          (1 << 8)        // 0x0100
#define DVAR_SAVED              (1 << 9)        // 0x0200
#define DVAR_NORESTART          (1 << 10)       // 0x0400
#define DVAR_CHANGEABLE_RESET   (1 << 12)       // 0x1000
#define DVAR_EXTERNAL           (1 << 14)       // 0x4000
#define DVAR_AUTOEXEC           (1 << 15)       // 0x8000
#define DVAR_UNRESTRICTED		(1 << 16)		// 0x10000

namespace Game
{
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
		latched = 0x20,
		cheat_protected = 0x80,
		write_protected = 0x10,
		read_only = 0x40,
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

	struct texdef_t
	{
		char pad_0x0000[4];
		char *name;
		char pad_0x0008[12];
		int width;
		int height;
		char pad_0x001C[20];
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
		char *entityDesc;
		char flagname0[32];
		char flagname1[32];
		char flagname2[32];
		char flagname3[32];
		char flagname4[32];
		char flagname5[32];
		char flagname6[32];
		char flagname7[32];
		char flagname8[32];
		char pad_0x0160[48];
		char *commands;
	};

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

	struct __declspec(align(4)) face_t
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
	};

	struct entity_s
	{
		entity_s *prev;
		entity_s *next;
		entity_s* firstActive; //brush_t *lastActive;
		brush_t *firstBrush;
		char pad_0x0010[80];
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

	struct __declspec(align(4)) BrushPt_t
	{
		float xyz[3];
		int sideIndex[3];
	};

	struct qtexture_s
	{
		qtexture_s *next;
		char *name;
		char pad_0x0008[8];
		__int16 texture_number_no;
		__int16 width;
		__int16 height;
		__int16 color;
		bool inuse;
		char pad_0x0021[3];
		qtexture_s *prev;
		char pad_0x0028[8];
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
		__int16 width;
		__int16 height;
		__int16 contents;
		__int16 flags;
		__int16 value;
		__int16 subDivType;
		char pad_0x0018[4];
		qtexture_s *d_texture;
		char pad_0x0020[4];
		texdef_t *mat_lightmap;
		char pad_0x0028[4];
		texdef_t *mat_smoothing;
		char pad_0x0030[4];
		texdef_t *mat_unk;
		drawVert_t ctrl[16];
	};

	// somethings really off with those structs
	// selected_brushes should be brush_t but that doesnt match up with real brush_t structs in memory? 
	struct selbrush_t
	{
		selbrush_t *prev; // prev selected brush @prev->currSelection
		selbrush_t *next; // next always empty i guess
		entity_s *owner;	// not sure
		brush_t *ownerNext; // not sure
		char pad[4];		// not sure
		brush_t *currSelection;	// selected brush info
	};

	struct selbrush_ptr
	{
		selbrush_t *p;
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
}
