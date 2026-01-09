#ifndef __TOOLS_STUBS_H__
#define __TOOLS_STUBS_H__

#define PROC_FILE_EXT "proc"
#define PROC_FILE_ID  "mapProcFile003"

typedef enum {
	MC_BAD,
	MC_OPAQUE,	   // completely fills the triangle, will have black drawn on fillDepthBuffer
	MC_PERFORATED, // may have alpha tested holes
	MC_TRANSLUCENT // blended with background
} materialCoverage_t;

enum SubViewType : uint16_t { SUBVIEW_NONE, SUBVIEW_MIRROR, SUBVIEW_DIRECT_PORTAL };

typedef enum { CT_FRONT_SIDED, CT_BACK_SIDED, CT_TWO_SIDED } cullType_t;


// contents flags, NOTE: make sure to keep the defines in doom_defs.script up to date with these!
typedef enum {
	CONTENTS_SOLID = BIT( 0 ),	 // an eye is never valid in a solid
	CONTENTS_OPAQUE = BIT( 1 ),	 // blocks visibility (for ai)
	CONTENTS_WATER = BIT( 2 ),	 // used for water
	CONTENTS_PLAYERCLIP = BIT( 3 ),	 // solid to players
	CONTENTS_MONSTERCLIP = BIT( 4 ),	 // solid to monsters
	CONTENTS_MOVEABLECLIP = BIT( 5 ),	 // solid to moveable entities
	CONTENTS_IKCLIP = BIT( 6 ),	 // solid to IK
	CONTENTS_BLOOD = BIT( 7 ),	 // used to detect blood decals
	CONTENTS_BODY = BIT( 8 ),	 // used for actors
	CONTENTS_PROJECTILE = BIT( 9 ),	 // used for projectiles
	CONTENTS_CORPSE = BIT( 10 ), // used for dead bodies
	CONTENTS_RENDERMODEL = BIT( 11 ), // used for render models for collision detection
	CONTENTS_TRIGGER = BIT( 12 ), // used for triggers
	CONTENTS_AAS_SOLID = BIT( 13 ), // solid for AAS
	CONTENTS_AAS_OBSTACLE = BIT( 14 ), // used to compile an obstacle into AAS that can be enabled/disabled
	CONTENTS_FLASHLIGHT_TRIGGER = BIT( 15 ), // used for triggers that are activated by the flashlight

	// jmarshall: used by Quake 3 bots
	CONTENTS_SLIME = BIT( 16 ), // used for slime
	CONTENTS_FOG = BIT( 17 ), // used for fog
	CONTENTS_LAVA = BIT( 18 ),
	// jmarshall end

	// contents used by utils
	CONTENTS_AREAPORTAL = BIT( 20 ), // portal separating renderer areas
	CONTENTS_NOCSG = BIT( 21 ), // don't cut this brush with CSG operations in the editor
	CONTENTS_ORIGIN = BIT( 22 ),

	CONTENTS_REMOVE_UTIL = ~( CONTENTS_AREAPORTAL | CONTENTS_NOCSG )
} contentsFlags_t;


// surface types
const int NUM_SURFACE_BITS = 4;
const int MAX_SURFACE_TYPES = 1 << NUM_SURFACE_BITS;

typedef enum {
	SURFTYPE_NONE, // default type
	SURFTYPE_METAL,
	SURFTYPE_STONE,
	SURFTYPE_FLESH,
	SURFTYPE_WOOD,
	SURFTYPE_CARDBOARD,
	SURFTYPE_LIQUID,
	SURFTYPE_GLASS,
	SURFTYPE_PLASTIC,
	SURFTYPE_RICOCHET,
	SURFTYPE_10,
	SURFTYPE_11,
	SURFTYPE_12,
	SURFTYPE_13,
	SURFTYPE_14,
	SURFTYPE_15
} surfTypes_t;

// surface flags
typedef enum {
	SURF_TYPE_BIT0 = BIT( 0 ), // encodes the material type (metal, flesh, concrete, etc.)
	SURF_TYPE_BIT1 = BIT( 1 ), // "
	SURF_TYPE_BIT2 = BIT( 2 ), // "
	SURF_TYPE_BIT3 = BIT( 3 ), // "
	SURF_TYPE_MASK = ( 1 << NUM_SURFACE_BITS ) - 1,

	SURF_NODAMAGE = BIT( 4 ),	 // never give falling damage
	SURF_SLICK = BIT( 5 ),	 // effects game physics
	SURF_COLLISION = BIT( 6 ),	 // collision surface
	SURF_LADDER = BIT( 7 ),	 // player can climb up this surface
	SURF_NOIMPACT = BIT( 8 ),	 // don't make missile explosions
	SURF_NOSTEPS = BIT( 9 ),	 // no footstep sounds
	SURF_DISCRETE = BIT( 10 ), // not clipped or merged by utilities
	SURF_NOFRAGMENT = BIT( 11 ), // dmap won't cut surface at each bsp boundary
	SURF_NULLNORMAL = BIT( 12 ), // renderbump will draw this surface as 0x80 0x80 0x80, which won't collect light from any angle
	SURF_OCCLUSION = BIT( 13 ), // RB: occluder surface
} surfaceFlags_t;


class idImage;

class idMaterial {
public:
	bool SurfaceCastsShadow() const;
	bool IsDrawn() const;
	bool IsDiscrete() const;
    const char* GetName() const;
    bool NoFragment() const;
    int GetContentFlags() const;
    bool ReceivesLightingOnBackSides() const;
    bool IsAmbientLight() const;
    bool LightEffectsBackSides() const;
    bool ReceivesLighting() const;
    void SetSort( float s ) const;
	bool IsFogLight() const;
	materialCoverage_t	 Coverage() const { return MC_BAD; }
	idImage *GetEditorImage( ) const { return nullptr; }
	const cullType_t	 GetCullType( ) const { return CT_TWO_SIDED; }

	float				 GetEditorAlpha( ) const { return 1.0f; }
	bool				ShouldCreateBackSides( ) const { return false; }
	const int			 GetSurfaceFlags() const { return surfaceFlags; }

private:
	int						 contentFlags;	// content flags
	int						 surfaceFlags;	// surface flags
	mutable int				 materialFlags; // material flags
};

#define CONTENTS_AREAPORTAL 0x80000000


class idImage {
public:
    const char* GetName() const;
	int				   GetUploadWidth( ) const { return 1; }
	int				   GetUploadHeight( ) const { return 1; }
};


typedef int glIndex_t;


typedef struct srfTriangles_s {
	int					numVerts;
	idDrawVert *verts;
	int					numIndexes;
	glIndex_t *indexes;
} srfTriangles_t;

struct modelSurface_t {
	int				  id;
	const idMaterial *shader;
	srfTriangles_t *geometry;
};

class idRenderModel {
public:
    int NumSurfaces() const;
    const struct modelSurface_t* Surface( int surfaceNum ) const;
	idBounds Bounds( const struct renderEntity_s *ent ) const { return bounds; }
private:
	idBounds						  bounds;
};

class idRenderModelManager {
public:
    idRenderModel* FindModel( const char* modelName );
	idRenderModel* CheckModel( const char* modelName );
};

class idDeclManager {
public:
    const idMaterial* FindMaterial( const char* name, bool makeDefault = true );
};

extern idRenderModelManager* renderModelManager;
extern idDeclManager* declManager;

namespace idLib {
    void RogmapPacifierFilename( const char* msg, const char* title );
    void RogmapPacifierInfo( const char* fmt, ... );
    void RogmapPacifierCompileProgressTotal( int total );
    void ClearWarnings( const char *fmt, ... );
    void SetRefreshOnPrint( bool set );
    void PrintWarnings( void );
}

#define CONTENTS_AREAPORTAL 0x80000000

// Colors
extern idVec4 colorRed;
extern idVec4 colorGreen;
extern idVec4 colorBlue;
extern idVec4 colorYellow;
extern idVec4 colorMagenta;
extern idVec4 colorCyan;
extern idVec4 colorWhite;
extern idVec4 colorPurple;
extern idVec4 colorBlack;
extern idVec4 colorGray;
extern idVec4 colorPink;
extern idVec4 colorDodgerBlue;
extern idVec4 colorDarkKhaki;
extern idVec4 colorAqua;
extern idVec4 colorCrimson;
extern idVec4 colorTeal;
extern idVec4 colorOlive;
extern idVec4 colorCoral;
extern idVec4 colorDarkSlateGray;
extern idVec4 colorBurlyWood;
extern idVec4 colorDarkBlue;
extern idVec4 colorBrown;
extern idVec4 colorLightSeaGreen;
extern idVec4 colorGold;
extern idVec4 colorNavy;
extern idVec4 colorOrange;
extern idVec4 colorDarkGoldenRod;
extern idVec4 colorDarkSalmon;
extern idVec4 colorLightSteelBlue;




// SHADERPARM constants
const int SHADERPARM_RED = 0;
const int SHADERPARM_GREEN = 1;
const int SHADERPARM_BLUE = 2;
const int SHADERPARM_TIMESCALE = 3;
const int SHADERPARM_TIMEOFFSET = 4;
const int SHADERPARM_MODE = 7;


typedef struct renderLight_s {
    idVec3		origin;
    idVec3		target;
    idVec3		up;
    idVec3		right;
    idVec3		start;
    idVec3		end;
    bool		pointLight;
    idVec3		lightCenter;
    idVec3		lightRadius;
    idMat3		axis;
    float		shaderParms[12];
    bool		noShadows;
    bool		noSpecular;
    bool		parallel;
    const idMaterial* shader;
} renderLight_t;

struct idRenderLightLocal {
    const idMaterial* lightShader;
    renderLight_t parms;
    bool baseLightProject;
};

void R_DeriveLightData( idRenderLightLocal *light );
void R_FreeLightDefDerivedData( idRenderLightLocal *light );
void R_RenderLightFrustum( const renderLight_t &light, idPlane *frustum );

class idRenderMatrix {
public:
    static void GetFrustumPlanes( idPlane *planes, bool baseLightProject, bool x, bool y ) {}
};


srfTriangles_t *R_AllocStaticTriSurf( void );
void			R_FreeStaticTriSurf( srfTriangles_t *tri );
void			R_AllocStaticTriSurfVerts( srfTriangles_t *tri, int numVerts );
void			R_AllocStaticTriSurfIndexes( srfTriangles_t *tri, int numIndexes );
void			R_RangeCheckIndexes( srfTriangles_t *tri );
void			R_CreateSilIndexes( srfTriangles_t *tri );
void			R_RemoveDegenerateTriangles( srfTriangles_t *tri );
void			R_FreeStaticTriSurfSilIndexes( srfTriangles_t *tri );

#endif /* !__TOOLS_STUBS_H__ */
