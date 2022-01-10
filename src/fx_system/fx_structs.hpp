#pragma once

namespace fx_system
{
	constexpr auto FX_HANDLE_NONE = 0xFFFF;

	constexpr auto FX_EFFECT_HANDLE_SCALE = 4;
	constexpr auto FX_EFFECT_LIMIT = 1024; // size 0x80
	constexpr auto FX_EFFECT_HANDLE_NONE = FX_EFFECT_LIMIT - 1;

	constexpr auto FX_ELEM_HANDLE_SCALE = 4;
	constexpr auto FX_ELEM_LIMIT = 2048; // size 0x28

	constexpr auto FX_TRAIL_HANDLE_SCALE = 4;
	constexpr auto FX_TRAIL_LIMIT = 128; // size 0x8

	constexpr auto FX_TRAILELEM_HANDLE_SCALE = 4;
	constexpr auto FX_TRAILELEM_LIMIT = 2048; // size 0x20

	constexpr auto FX_SPOT_LIGHT_LIMIT = 1;


	constexpr auto FX_SPAWN_DOBJ_HANDLES = 4095;
	constexpr auto FX_SPAWN_BONE_INDEX = 1023;
	constexpr auto FX_SPAWN_MARK_ENTNUM = 2175;


	struct FxEffectDef;

	enum $D91DF7250D497AE680597430665E65B1
	{
		FX_STATUS_REF_COUNT_MASK = 0xFFFF,
		FX_STATUS_HAS_PENDING_LOOP_ELEMS = 0x10000,
		FX_STATUS_OWNED_EFFECTS_SHIFT = 0x11,
		FX_STATUS_OWNED_EFFECTS_MASK = 0x7FE0000,
		FX_STATUS_DEFER_UPDATE = 0x8000000,
		FX_STATUS_SELF_OWNED = 0x10000000,
		FX_STATUS_IS_LOCKED = 0x20000000,
		FX_STATUS_IS_LOCKED_MASK = 0x60000000,
	};

	enum FxUpdateResult
	{
		FX_UPDATE_REMOVE = 0x0,
		FX_UPDATE_KEEP = 0x1,
	};

	enum $7C8B8C292E4C5EC841521618A6E64E01
	{
		FX_ELEM_SPAWN_RELATIVE_TO_EFFECT = 0x2,
		FX_ELEM_SPAWN_FRUSTUM_CULL = 0x4,
		FX_ELEM_RUNNER_USES_RAND_ROT = 0x8,
		FX_ELEM_SPAWN_OFFSET_NONE = 0x0,
		FX_ELEM_SPAWN_OFFSET_SPHERE = 0x10,
		FX_ELEM_SPAWN_OFFSET_CYLINDER = 0x20,
		FX_ELEM_SPAWN_OFFSET_MASK = 0x30,
		FX_ELEM_RUN_RELATIVE_TO_WORLD = 0x0,
		FX_ELEM_RUN_RELATIVE_TO_SPAWN = 0x40,
		FX_ELEM_RUN_RELATIVE_TO_EFFECT = 0x80,
		FX_ELEM_RUN_RELATIVE_TO_OFFSET = 0xC0,
		FX_ELEM_RUN_MASK = 0xC0,
		FX_ELEM_USE_COLLISION = 0x100,
		FX_ELEM_DIE_ON_TOUCH = 0x200,
		FX_ELEM_DRAW_PAST_FOG = 0x400,
		FX_ELEM_DRAW_WITH_VIEWMODEL = 0x800,
		FX_ELEM_BLOCK_SIGHT = 0x1000,
		FX_ELEM_HAS_VELOCITY_GRAPH_LOCAL = 0x1000000,
		FX_ELEM_HAS_VELOCITY_GRAPH_WORLD = 0x2000000,
		FX_ELEM_HAS_GRAVITY = 0x4000000,
		FX_ELEM_USE_MODEL_PHYSICS = 0x8000000,
		FX_ELEM_NONUNIFORM_SCALE = 0x10000000,
	};

	enum $39199BFA0D60C62DCFD9209B26DED22C
	{
		FX_ELEM_TYPE_SPRITE_BILLBOARD = 0x0,
		FX_ELEM_TYPE_SPRITE_ORIENTED = 0x1,
		FX_ELEM_TYPE_TAIL = 0x2,
		FX_ELEM_TYPE_TRAIL = 0x3,
		FX_ELEM_TYPE_CLOUD = 0x4,
		FX_ELEM_TYPE_MODEL = 0x5,
		FX_ELEM_TYPE_OMNI_LIGHT = 0x6,
		FX_ELEM_TYPE_SPOT_LIGHT = 0x7,
		FX_ELEM_TYPE_SOUND = 0x8,
		FX_ELEM_TYPE_DECAL = 0x9,
		FX_ELEM_TYPE_RUNNER = 0xA,
		FX_ELEM_TYPE_COUNT = 0xB,
		FX_ELEM_TYPE_LAST_SPRITE = 0x3,
		FX_ELEM_TYPE_LAST_DRAWN = 0x7,
	};

	enum $7E23918555002659587F63136D232798
	{
		FX_ELEM_CLASS_SPRITE = 0x0,
		FX_ELEM_CLASS_NONSPRITE = 0x1,
		FX_ELEM_CLASS_CLOUD = 0x2,
		FX_ELEM_CLASS_COUNT = 0x3,
	};

	enum TraceHitType
	{
		TRACE_HITTYPE_NONE = 0x0,
		TRACE_HITTYPE_ENTITY = 0x1,
		TRACE_HITTYPE_DYNENT_MODEL = 0x2,
		TRACE_HITTYPE_DYNENT_BRUSH = 0x3,
	};

	struct FxInsertSortElem
	{
		int defSortOrder;
		float distToCamSq;
		int msecBegin;
		int defIndex;
		char elemType;
	}; STATIC_ASSERT_SIZE(FxInsertSortElem, 0x14);

	struct FxCamera
	{
		float origin[3];
		volatile int isValid;
		float frustum[6][4];
		float axis[3][3];
		unsigned int frustumPlaneCount;
		float viewOffset[3];
		unsigned int pad[3];
	};

	struct r_double_index_t
	{
		unsigned __int16 value[2];
	};

	struct FxSpriteInfo
	{
		r_double_index_t* indices;
		unsigned int indexCount;
		game::Material* material;
		const char* name;
	};

	struct FxSpawnDefLooping
	{
		int intervalMsec;
		int count;
	};

	struct FxIntRange
	{
		int base;
		int amplitude;
	};

	struct FxSpawnDefOneShot
	{
		FxIntRange count;
	};

	union FxSpawnDef
	{
		FxSpawnDefLooping looping;
		FxSpawnDefOneShot oneShot;
	};

	struct FxFloatRange
	{
		float base;
		float amplitude;
	};

	struct FxElemAtlas
	{
		char behavior;
		char index;
		char fps;
		char loopCount;
		char colIndexBits;
		char rowIndexBits;
		__int16 entryCount;
	};

	struct FxElemVec3Range
	{
		float base[3];
		float amplitude[3];
	};

	struct FxElemVelStateInFrame
	{
		FxElemVec3Range velocity;
		FxElemVec3Range totalDelta;
	};

	struct FxElemVelStateSample
	{
		FxElemVelStateInFrame local;
		FxElemVelStateInFrame world;
	};

	struct FxElemVisualState
	{
		char color[4];
		float rotationDelta;
		float rotationTotal;
		float size[2];
		float scale;
	};

	struct FxElemVisStateSample
	{
		FxElemVisualState base;
		FxElemVisualState amplitude;
	};

	union FxEffectDefRef
	{
		FxEffectDef* handle;
		const char* name;
	};

	union FxElemVisuals
	{
		const void* anonymous;
		game::Material* material;
		game::XModel* model;
		FxEffectDefRef effectDef;
		const char* soundName;
	};

	struct FxElemMarkVisuals
	{
		game::Material* materials[2];
	};

	union FxElemDefVisuals
	{
		FxElemMarkVisuals* markArray;
		FxElemVisuals* array;
		FxElemVisuals instance;
	};

	struct FxTrailVertex
	{
		float pos[2];
		float normal[2];
		float texCoord;
	};

	struct FxTrailDef
	{
		int scrollTimeMsec;
		int repeatDist;
		int splitDist;
		int vertCount;
		FxTrailVertex* verts;
		int indCount;
		unsigned __int16* inds;
	};

	struct FxElemDef
	{
		int flags;
		FxSpawnDef spawn;
		FxFloatRange spawnRange;
		FxFloatRange fadeInRange;
		FxFloatRange fadeOutRange;
		float spawnFrustumCullRadius;
		FxIntRange spawnDelayMsec;
		FxIntRange lifeSpanMsec;
		FxFloatRange spawnOrigin[3];
		FxFloatRange spawnOffsetRadius;
		FxFloatRange spawnOffsetHeight;
		FxFloatRange spawnAngles[3];
		FxFloatRange angularVelocity[3];
		FxFloatRange initialRotation;
		FxFloatRange gravity;
		FxFloatRange reflectionFactor;
		FxElemAtlas atlas;
		char elemType;
		char visualCount;
		char velIntervalCount;
		char visStateIntervalCount;
		FxElemVelStateSample* velSamples;
		FxElemVisStateSample* visSamples;
		FxElemDefVisuals visuals;
		float collMins[3];
		float collMaxs[3];
		FxEffectDefRef effectOnImpact;
		FxEffectDefRef effectOnDeath;
		FxEffectDefRef effectEmitted;
		FxFloatRange emitDist;
		FxFloatRange emitDistVariance;
		FxTrailDef* trailDef;
		char sortOrder;
		char lightingFrac;
		char useItemClip;
		char unused[1];
	};

	struct FxEffectDef
	{
		const char* name;
		int flags;
		int totalSize;
		int msecLoopingLife;
		int elemDefCountLooping;
		int elemDefCountOneShot;
		int elemDefCountEmission;
		FxElemDef* elemDefs;
	};

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
		FxEffectDef* def;
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

	union $A58BA6DA60295001BBA5E9F807131CF1
	{
		int physObjId;
		float origin[3];
	};

	union $CAB802B49D819BF84CE5061C1E209C1A
	{
		float trailTexCoord;
		unsigned __int16 lightingHandle;
	};

	struct FxElem
	{
		char defIndex;
		char sequence;
		char atRestFraction;
		char emitResidual;
		unsigned __int16 nextElemHandleInEffect;
		unsigned __int16 prevElemHandleInEffect;
		int msecBegin;
		float baseVel[3];
		$A58BA6DA60295001BBA5E9F807131CF1 ___u8;
		$CAB802B49D819BF84CE5061C1E209C1A u;
	};

	union $BEF49F010BBA87A46BC346A1DC05E6F3
	{
		int nextFree;
		FxElem item;
	};

	struct FxPool_FxElem
	{
		$BEF49F010BBA87A46BC346A1DC05E6F3 ___u0;
	};

	struct FxTrail
	{
		unsigned __int16 nextTrailHandle;
		unsigned __int16 firstElemHandle;
		unsigned __int16 lastElemHandle;
		char defIndex;
		char sequence;
	};

	union $B23B48F46E0D94E30C16D62557B4B3AE
	{
		int nextFree;
		FxTrail item;
	};

	struct FxPool_FxTrail
	{
		$B23B48F46E0D94E30C16D62557B4B3AE ___u0;
	};

	struct FxTrailElem
	{
		float origin[3];
		float spawnDist;
		int msecBegin;
		unsigned __int16 nextTrailElemHandle;
		__int16 baseVelZ;
		char basis[2][3];
		char sequence;
		char unused;
	};

	union $49681455E9ABE6DA006916B24A5CA67F
	{
		int nextFree;
		FxTrailElem item;
	};

	struct FxPool_FxTrailElem
	{
		$49681455E9ABE6DA006916B24A5CA67F ___u0;
	};

	struct FxVisBlocker
	{
		float origin[3];
		unsigned __int16 radius;
		unsigned __int16 visibility;
	};

	struct FxVisState
	{
		FxVisBlocker blocker[256];
		volatile int blockerCount;
		unsigned int pad[3];
	};

	struct FxSystem
	{
		FxCamera camera;
		FxCamera cameraPrev;
		FxSpriteInfo sprite;
		FxEffect* effects;
		FxPool_FxElem* elems;
		FxPool_FxTrail* trails;
		FxPool_FxTrailElem* trailElems;
		unsigned __int16* deferredElems;
		volatile int firstFreeElem;
		volatile int firstFreeTrailElem;
		volatile int firstFreeTrail;
		volatile int deferredElemCount;
		volatile int activeElemCount;
		volatile int activeTrailElemCount;
		volatile int activeTrailCount;
		volatile int gfxCloudCount;
		FxVisState* visState;
		const FxVisState* visStateBufferRead;
		FxVisState* visStateBufferWrite;
		volatile int firstActiveEffect;
		volatile int firstNewEffect;
		volatile int firstFreeEffect;
		unsigned __int16 allEffectHandles[1024];
		volatile int activeSpotLightEffectCount;
		volatile int activeSpotLightElemCount;
		unsigned __int16 activeSpotLightEffectHandle;
		unsigned __int16 activeSpotLightElemHandle;
		__int16 activeSpotLightBoltDobj;
		volatile int iteratorCount;
		int msecNow;
		volatile int msecDraw;
		int frameCount;
		bool isInitialized;
		bool needsGarbageCollection;
		bool isArchiving;
		unsigned __int8 localClientNum;
		unsigned int restartList[32];
	};

	struct trace_t
	{
		float fraction;
		float normal[3];
		int surfaceFlags;
		int contents;
		const char* material;
		TraceHitType hitType;
		unsigned __int16 hitId;
		unsigned __int16 modelIndex;
		unsigned __int16 partName;
		unsigned __int16 partGroup;
		bool allsolid;
		bool startsolid;
		bool walkable;
		bool pad;
	}; STATIC_ASSERT_SIZE(trace_t, 0x2C);

	struct FxUpdateElem
	{
		FxEffect* effect;
		int elemIndex;
		int atRestFraction;
		game::orientation_t orient;
		int randomSeed;
		int sequence;
		float msecLifeSpan;
		int msecElemBegin;
		int msecElemEnd;
		int msecUpdateBegin;
		int msecUpdateEnd;
		float msecElapsed;
		float normTimeUpdateEnd;
		float* elemOrigin;
		float* elemBaseVel;
		float posWorld[3];
		bool onGround;
		int physObjId;
	};
}