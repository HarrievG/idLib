
#include "precompiled.h"
#pragma hdrstop

#include "stubs.h"

// Stubs for idMaterial
const char* idMaterial::GetName() const { return "stub_material"; }
bool idMaterial::NoFragment() const { return false; }
int idMaterial::GetContentFlags() const { return 0; }
bool idMaterial::ReceivesLightingOnBackSides() const { return false; }
bool idMaterial::IsAmbientLight() const { return false; }
bool idMaterial::LightEffectsBackSides() const { return false; }
bool idMaterial::ReceivesLighting() const { return true; }
void idMaterial::SetSort( float s ) const {}
bool idMaterial::SurfaceCastsShadow() const { return false; }
bool idMaterial::IsDrawn() const { return true; }
bool idMaterial::IsDiscrete() const { return false; }
bool idMaterial::IsFogLight() const { return false; }
// Stubs for idImage
const char* idImage::GetName() const { return "stub_image"; }

// Stubs for R_* functions
srfTriangles_t *R_AllocStaticTriSurf( void ) {
    return new srfTriangles_t();
}

void R_FreeStaticTriSurf( srfTriangles_t *tri ) {
    if ( tri ) {
        delete[] tri->verts;
        delete[] tri->indexes;
        delete tri;
    }
}

void R_AllocStaticTriSurfVerts( srfTriangles_t *tri, int numVerts ) {
    tri->verts = new idDrawVert[numVerts];
    tri->numVerts = numVerts;
}

void R_AllocStaticTriSurfIndexes( srfTriangles_t *tri, int numIndexes ) {
    tri->indexes = new glIndex_t[numIndexes];
    tri->numIndexes = numIndexes;
}

void R_RangeCheckIndexes( srfTriangles_t *tri ) {}
void R_CreateSilIndexes( srfTriangles_t *tri ) {}
void R_RemoveDegenerateTriangles( srfTriangles_t *tri ) {}
void R_FreeStaticTriSurfSilIndexes( srfTriangles_t *tri ) {}

// Stubs for Managers
idRenderModelManager* renderModelManager = nullptr;
idDeclManager* declManager = nullptr;

namespace idLib {
    void RogmapPacifierFilename( const char* msg, const char* title ) {}
    void RogmapPacifierInfo( const char* fmt, ... ) {}
    void RogmapPacifierCompileProgressTotal( int total ) {}
    void ClearWarnings( const char *fmt, ... ) {}
    void SetRefreshOnPrint( bool set ) {}
    void PrintWarnings( void ) {}
}

int idRenderModel::NumSurfaces() const { return 0; }
const struct modelSurface_t* idRenderModel::Surface( int surfaceNum ) const { return nullptr; }

idRenderModel* idRenderModelManager::FindModel( const char* modelName ) { return nullptr; }
idRenderModel* idRenderModelManager::CheckModel( const char* modelName ) { return nullptr; }

const idMaterial* idDeclManager::FindMaterial( const char* name, bool makeDefault ) { return nullptr; }

void R_DeriveLightData( idRenderLightLocal *light ) {}
void R_FreeLightDefDerivedData( idRenderLightLocal *light ) {}
void R_RenderLightFrustum( const renderLight_t &light, idPlane *frustum ) {}

void RunAAS_f( const class idCmdArgs &args ) {}
