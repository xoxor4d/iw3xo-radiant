#pragma once
#include "ode/ode.h"
#include "amotor.h"
#include "ball.h"
#include "hinge.h"

namespace physics
{
	enum PhysWorld : int
	{
		PHYS_WORLD_DYNENT = 0x0,
		PHYS_WORLD_FX = 0x1,
		PHYS_WORLD_RAGDOLL = 0x2,
		PHYS_WORLD_COUNT = 0x3,
	};

	enum physStuckState_t
	{
		PHYS_OBJ_STATE_POSSIBLY_STUCK = 0x0,
		PHYS_OBJ_STATE_STUCK = 0x1,
		PHYS_OBJ_STATE_FREE = 0x2,
	};

	enum $3E40264DEA36A90C0882F99B0BCF0AEC
	{
		GEOM_CLASS_BRUSHMODEL = 14,
		GEOM_CLASS_BRUSH = 15,
		GEOM_CLASS_CYLINDER = 16,
		GEOM_CLASS_CAPSULE = 17,
		GEOM_CLASS_WORLD = 18,
	};

	enum PhysicsGeomType
	{
		PHYS_GEOM_NONE = 0x0,
		PHYS_GEOM_BOX = 0x1,
		PHYS_GEOM_BRUSHMODEL = 0x2,
		PHYS_GEOM_BRUSH = 0x3,
		PHYS_GEOM_CYLINDER = 0x4,
		PHYS_GEOM_CAPSULE = 0x5,
		PHYS_GEOM_COUNT = 0x6,
	};

	struct pooldata_t
	{
	  void *firstFree;
	  int activeCount;
	};

	struct Jitter
	{
		float origin[3];
		float innerRadiusSq;
		float outerRadiusSq;
		float innerRadius;
		float outerRadius;
		float minDisplacement;
		float maxDisplacement;
	};

	struct PhysWorldData
	{
		int timeLastSnapshot;
		int timeLastUpdate;
		float timeNowLerpFrac;
		void(__cdecl* collisionCallback)();
		int numJitterRegions;
		bool useContactCentroids;
		Jitter jitterRegions[5];
	};

	struct __declspec(align(4)) PhysObjUserData
	{
		float translation[3];
		dxBody* body;
		float savedPos[3];
		float savedRot[3][3];
		int sndClass;
		float friction;
		float bounce;
		physStuckState_t state;
		float contactCentroid[3];
		int timeLastAsleep;
		float awakeTooLongLastPos[3];
		bool hasDisplayedAwakeTooLongWarning;
		bool debugContacts;
	};

	struct BodyState_t
	{
		float position[3];
		float rotation[3][3];
		float velocity[3];
		float angVelocity[3];
		float centerOfMassOffset[3];
		float mass;
		float friction;
		float bounce;
		physStuckState_t state;
		int timeLastAsleep;
		int type;
		bool unk;
	};

	struct GeomStateCylinder
	{
		int direction;
		float radius;
		float halfHeight;
	};

	struct PhysTriMeshInfo
	{
		float* verts;
		int vertStride;
		int vertCount;
		unsigned int* indices;
		unsigned int indexCount;
	};

	struct PhysStaticArray_dxJointHinge
	{
		int freeEntry;
		dxJointHinge entries[192];
	};

	struct PhysStaticArray_dxJointBall
	{
		int freeEntry;
		dxJointBall entries[160];
	};

	struct PhysStaticArray_dxJointAMotor
	{
		int freeEntry;
		dxJointAMotor entries[160];
	};

	struct GeomStateBox
	{
		float extent[3];
	};

	union $08D63B10D51D8DC51437D6552A72A4AA
	{
		unsigned __int16 brushModel;
		game::cbrush_t* brush;
	};

	struct GeomStateBrush
	{
		$08D63B10D51D8DC51437D6552A72A4AA u;
		float momentsOfInertia[3];
		float productsOfInertia[3];
	};

	union $5B13BEF7809C87B40AA0A8398E702F02
	{
		GeomStateCylinder cylinderState;
		GeomStateBox boxState;
		GeomStateBrush brushState;
	};

	struct GeomState
	{
		PhysicsGeomType type;
		bool isOriented;
		float orientation[3][3];
		$5B13BEF7809C87B40AA0A8398E702F02 u;
	};

	/*struct BrushWrapper
	{
		float mins[3];
		int contents;
		float maxs[3];
		unsigned int numsides;
		game::cbrushside_t* sides;
		__int16 axialMaterialNum[2][3];
		char* baseAdjacentSide;
		__int16 firstAdjacentSideOffsets[2][3];
		char edgeCount[2][3];
		int totalEdgeCount;
		game::cplane_s* planes;
	};*/

	struct dContactGeomExt
	{
		dContactGeom contact;
		int surfFlags;
	};

	struct ContactList
	{
		dContactGeomExt contacts[128];
		int contactCount;
	};

	struct PhysGlob
	{
		dxWorld* world[3];
		float world_seconds[3]; // ode version of IW has seconds inside dxWorld
		PhysWorldData worldData[3];
		dxSpace* space[3];
		dxJointGroup* contactgroup[3];
		PhysObjUserData userData[512];
		pooldata_t userDataPool;
		PhysTriMeshInfo triMeshInfo;
		dxTriMeshData* triMeshDataID;
		bool dumpContacts;
		dxGeom* visTrisGeom;
		dxGeom* worldGeom;
		int debugActiveObjCount;

		int hinge_freeEntry;
		dxJointHinge* hinge_entries; // 192

		int ball_freeEntry;
		dxJointBall* ball_entries; // 160

		int motor_freeEntry;
		dxJointAMotor* motor_entries; // 160

		//PhysStaticArray_dxJointHinge hingeArray;
		//PhysStaticArray_dxJointBall ballArray;
		//PhysStaticArray_dxJointAMotor aMotorArray;

		float gravityDirection[3];
		unsigned int physPreviousFrameTimes[10];
		unsigned int physPerformanceFrame;
		float performanceAverage;
		unsigned int performanceMintime;
		unsigned int performanceMaxtime;
	};

	struct odeGlob_t
	{
		dxWorld world[3];
		//dxSimpleSpace space[3]; // dxSimpleSpace - not even used?
		dxJointGroup contactsGroup[3]; // used
		//__declspec(align(16)) dxBody bodies[512]; // not used?
		pooldata_t bodyPool; // used
		//char geoms[425984]; // not used?
		pooldata_t geomPool; // used
		//dxUserGeom worldGeom; // dxUserGeom // not used?
	};


	// *
	// dupes

	struct dupe_dxGeom : dBase
	{
		int type;		// geom type number, set by subclass constructor
		int gflags;		// flags used by geom and space
		void* data;		// user-defined data pointer
		dBodyID body;		// dynamics body associated with this object (if any)
		dxGeom* body_next;	// next geom in body's linked list of associated geoms
		dxPosR* final_posr;	// final position of the geom in world coordinates
		dxPosR* offset_posr;	// offset from body in local coordinates

		// information used by spaces
		dxGeom* next;		// next geom in linked list of geoms
		dxGeom** tome;	// linked list backpointer
		dxGeom* next_ex;	// next geom in extra linked list of geoms (for higher level structures)
		dxGeom** tome_ex;	// extra linked list backpointer (for higher level structures)
		dxSpace* parent_space;// the space this geom is contained in, 0 if none
		dReal aabb[6];	// cached AABB for this space
		unsigned long category_bits, collide_bits;
	};

	struct dupe_dxGeomTransform : dupe_dxGeom
	{
		dxGeom* obj;		// object that is being transformed
		int cleanup;		// 1 to destroy obj when destroyed
		int infomode;		// 1 to put Tx geom in dContactGeom g1

		// cached final object transform (body tx + relative tx). this is set by
		// computeAABB(), and it is valid while the AABB is valid.
		dxPosR transform_posr;
	};
}