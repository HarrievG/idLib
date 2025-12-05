
/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "FileSystem.h"
#include "CmdSystem.h"
#include "CVarSystem.h"
#include <SDL3/SDL.h>
#include <memory>

// Globals
idCVar fs_basepath( "fs_basepath", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar fs_savepath( "fs_savepath", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar fs_game( "fs_game", "", CVAR_SYSTEM | CVAR_INIT | CVAR_SERVERINFO, "mod path" );
idCVar fs_game_base( "fs_game_base", "mod_icedhellfire", CVAR_SYSTEM | CVAR_INIT | CVAR_SERVERINFO, "alternate mod path, searched after the main fs_game path, before the basedir" );
idCVar fs_copyfiles( "fs_copyfiles", "0", CVAR_SYSTEM | CVAR_INIT | CVAR_BOOL, "Copy every file touched to fs_savepath" );
idCVar fs_debug( "fs_debug", "0", CVAR_SYSTEM | CVAR_INTEGER, "", 0, 2 );

#ifndef TAG_IDFILE
#define TAG_IDFILE 0
#endif

struct searchpath_t
{
	idStr	path;		// c:\doom
	idStr	gamedir;	// base
};

class idFileSystemLocal : public idFileSystem
{
public:
	idFileSystemLocal();

	virtual void			Init();
	virtual void			Restart();
	virtual void			Shutdown( bool reloading );
	virtual bool			IsInitialized() const;
	virtual idFileList* 	ListFiles( const char* relativePath, const char* extension, bool sort = false, bool fullRelativePath = false, const char* gamedir = NULL );
	virtual idFileList* 	ListFilesTree( const char* relativePath, const char* extension, bool sort = false,  bool allowSubdirsForResourcePaks = false, const char* gamedir = NULL );
	virtual void			FreeFileList( idFileList* fileList );
	virtual const char* 	OSPathToRelativePath( const char* OSPath );
	virtual const char* 	RelativePathToOSPath( const char* relativePath, const char* basePath );
	virtual const char* 	BuildOSPath( const char* base, const char* game, const char* relativePath );
	virtual const char* 	BuildOSPath( const char* base, const char* relativePath );
	virtual void			CreateOSPath( const char* OSPath );
	virtual int				ReadFile( const char* relativePath, void** buffer, ID_TIME_T* timestamp );
	virtual void			FreeFile( void* buffer );
	virtual int				WriteFile( const char* relativePath, const void* buffer, int size, const char* basePath = "fs_savepath" );
	virtual void			RemoveFile( const char* relativePath );
	virtual	bool			RemoveDir( const char* relativePath );
	virtual bool			RenameFile( const char* relativePath, const char* newName, const char* basePath = "fs_savepath" );
	virtual idFile* 		OpenFileReadFlags( const char* relativePath, int searchFlags, bool allowCopyFiles = true, const char* gamedir = NULL );
	virtual idFile* 		OpenFileRead( const char* relativePath, bool allowCopyFiles = true, const char* gamedir = NULL );
	virtual idFile* 		OpenFileReadMemory( const char* relativePath, bool allowCopyFiles = true, const char* gamedir = NULL );
	virtual idFile* 		OpenFileWrite( const char* relativePath, const char* basePath = "fs_savepath" );
	virtual idFile* 		OpenFileAppend( const char* relativePath, bool sync = false, const char* basePath = "fs_basepath" );
	virtual idFile* 		OpenFileByMode( const char* relativePath, fsMode_t mode );
	virtual idFile* 		OpenExplicitFileRead( const char* OSPath );
	virtual idFile* 		OpenExplicitFileWrite( const char* OSPath );
	virtual void			CloseFile( idFile* f );
	virtual void			FindDLL( const char* basename, char dllPath[ MAX_OSPATH ] );
	virtual void			CopyFile( const char* fromOSPath, const char* toOSPath );
	virtual findFile_t		FindFile( const char* path );
	virtual bool			FilenameCompare( const char* s1, const char* s2 ) const;
	virtual int				GetFileLength( const char* relativePath );
	virtual bool			IsFolder( const char* relativePath, const char* basePath = "fs_basepath" );
	// resource tracking
	virtual void			EnableBackgroundCache( bool enable );
	virtual bool			InProductionMode();
	virtual bool			UsingResourceFiles();
	virtual bool			IsBinaryModel( const idStr& resName ) const;
	virtual bool			IsSoundSample( const idStr& resName ) const;
	virtual void			FreeResourceBuffer();

	static void				Dir_f( const idCmdArgs& args );
	static void				Path_f( const idCmdArgs& args );

private:
    idList<searchpath_t>	searchPaths;
    idStr					gameFolder;

    void					SetupGameDirectories( const char* gameName );
    void					AddGameDirectory( const char* path, const char* dir );
    void                    Startup();
    void                    GetExtensionList( const char* extension, idStrList& extensionList ) const;
    int						GetFileList( const char* relativePath, const idStrList& extensions, idStrList& list, idHashIndex& hashIndex, bool fullRelativePath, bool allowSubdirsForResourcePaks, const char* gamedir );
    int						GetFileListTree( const char* relativePath, const idStrList& extensions, idStrList& list, idHashIndex& hashIndex, bool allowSubdirsForResourcePaks, const char* gamedir );
    int						ListOSFiles( const char* directory, const char* extension, idStrList& list );

    // SDL3 Helper
    SDL_IOStream*           OpenOSFile( const char* name, fsMode_t mode );
    int                     DirectFileLength( SDL_IOStream* o );
};

idFileSystemLocal	fileSystemLocal;
idFileSystem* 		fileSystem = &fileSystemLocal;

// Stubbed/Empty implementations for methods we aren't fully supporting yet
void idFileSystemLocal::EnableBackgroundCache( bool enable ) {}
bool idFileSystemLocal::InProductionMode() { return false; }
bool idFileSystemLocal::UsingResourceFiles() { return false; }
bool idFileSystemLocal::IsBinaryModel( const idStr& resName ) const { return false; }
bool idFileSystemLocal::IsSoundSample( const idStr& resName ) const { return false; }
void idFileSystemLocal::FreeResourceBuffer() {}
void idFileSystemLocal::FindDLL( const char* basename, char dllPath[ MAX_OSPATH ] ) {}
void idFileSystemLocal::CopyFile( const char* fromOSPath, const char* toOSPath ) {
    SDL_CopyFile( fromOSPath, toOSPath );
}
findFile_t idFileSystemLocal::FindFile( const char* path ) {
    idFile* f = OpenFileRead( path );
    if (f) {
        CloseFile(f);
        return FIND_YES;
    }
    return FIND_NO;
}
bool idFileSystemLocal::FilenameCompare( const char* s1, const char* s2 ) const {
    return idStr::Icmp( s1, s2 ) != 0;
}
int idFileSystemLocal::GetFileLength( const char* relativePath ) {
    idFile* f = OpenFileRead( relativePath );
    if (f) {
        int len = f->Length();
        CloseFile(f);
        return len;
    }
    return -1;
}
bool idFileSystemLocal::IsFolder( const char* relativePath, const char* basePath ) {
    const char* path = RelativePathToOSPath( relativePath, basePath );
    SDL_PathInfo info;
    if ( SDL_GetPathInfo( path, &info ) == 0 ) {
        return info.type == SDL_PATHTYPE_DIRECTORY;
    }
    return false;
}

idFileSystemLocal::idFileSystemLocal()
{
}

void idFileSystemLocal::Init() {
	if ( fs_basepath.GetString()[0] == '\0' )
	{
        const char *base = SDL_GetBasePath();
		fs_basepath.SetString( base ? base : "." );
        if ( base ) {
            SDL_free( (void*)base );
        }
	}
	if ( fs_savepath.GetString()[0] == '\0' )
	{
        const char *pref = SDL_GetPrefPath( "idSoftware", "Doom3" ); // Example org/app
		fs_savepath.SetString( pref ? pref : "." );
        if ( pref ) {
            SDL_free( (void*)pref );
        }
	}

	Startup();
}

void idFileSystemLocal::Startup()
{
    idLib::Printf( "------ Initializing File System ------\n" );
    SetupGameDirectories( "base" );

    cmdSystem->AddCommand( "dir", Dir_f, CMD_FL_SYSTEM, "lists a folder", idCmdSystem::ArgCompletion_FileName );
    cmdSystem->AddCommand( "path", Path_f, CMD_FL_SYSTEM, "lists search paths" );

    idLib::Printf( "file system initialized.\n" );
}

void idFileSystemLocal::Shutdown( bool reloading ) {
    searchPaths.Clear();
}

void idFileSystemLocal::Restart() {
    Shutdown( true );
    Startup();
}

bool idFileSystemLocal::IsInitialized() const {
    return searchPaths.Num() > 0;
}

void idFileSystemLocal::SetupGameDirectories( const char* gameName ) {
	if( fs_basepath.GetString()[0] )
	{
		AddGameDirectory( fs_basepath.GetString(), gameName );
	}
	if( fs_savepath.GetString()[0] )
	{
		AddGameDirectory( fs_savepath.GetString(), gameName );
	}
}

void idFileSystemLocal::AddGameDirectory( const char* path, const char* dir ) {
    searchpath_t& search = searchPaths.Alloc();
	search.path = path;
	search.gamedir = dir;
	gameFolder = dir;
}

// Path Construction

const char* idFileSystemLocal::BuildOSPath( const char* base, const char* game, const char* relativePath ) {
	static char OSPath[MAX_STRING_CHARS];
	idStr newPath;

    if ( relativePath[0] == '/' || relativePath[1] == ':' ) {
        return relativePath;
    }

	idStr strBase = base;
	strBase.StripTrailing( '/' );
	strBase.StripTrailing( '\\' );
	sprintf( newPath, "%s/%s/%s", strBase.c_str(), game, relativePath );
	idStr::Copynz( OSPath, newPath, sizeof( OSPath ) );
	return OSPath;
}

const char* idFileSystemLocal::BuildOSPath( const char* base, const char* relativePath ) {
    return BuildOSPath( base, gameFolder, relativePath );
}

const char* idFileSystemLocal::RelativePathToOSPath( const char* relativePath, const char* basePath ) {
	const char* path = cvarSystem->GetCVarString( basePath );
	if( !path[0] )
	{
		path = fs_savepath.GetString();
	}
	return BuildOSPath( path, gameFolder, relativePath );
}

const char* idFileSystemLocal::OSPathToRelativePath( const char* OSPath ) {
    return OSPath;
}

// File Operations (SDL3)

SDL_IOStream* idFileSystemLocal::OpenOSFile( const char* name, fsMode_t mode ) {
    const char* modeStr = "rb";
    if ( mode == FS_WRITE ) modeStr = "wb";
    else if ( mode == FS_APPEND ) modeStr = "ab";

    return SDL_IOFromFile( name, modeStr );
}

int idFileSystemLocal::DirectFileLength( SDL_IOStream* o ) {
    if (!o) return 0;
    Sint64 len = SDL_GetIOSize( o );
    return (int)len;
}

idFile* idFileSystemLocal::OpenExplicitFileRead( const char* OSPath ) {
    SDL_IOStream* io = OpenOSFile( OSPath, FS_READ );
    if ( !io ) return NULL;

    idFile_Permanent* f = new idFile_Permanent();
    // Assuming friend access works as declared in File.h
    f->o = io;
    f->mode = ( 1 << FS_READ );
    f->fileSize = DirectFileLength( io );
    f->fullPath = OSPath;

    return f;
}

idFile* idFileSystemLocal::OpenExplicitFileWrite( const char* OSPath ) {
    SDL_IOStream* io = OpenOSFile( OSPath, FS_WRITE );
    if ( !io ) return NULL;

    idFile_Permanent* f = new idFile_Permanent();
    f->o = io;
    f->mode = ( 1 << FS_WRITE );
    f->fileSize = 0;
    f->fullPath = OSPath;

    return f;
}

void idFileSystemLocal::CloseFile( idFile* f ) {
    delete f;
}

int idFileSystemLocal::ReadFile( const char* relativePath, void** buffer, ID_TIME_T* timestamp ) {
    idFile* f = OpenFileRead( relativePath, false );
    if ( !f ) return -1;

    int len = f->Length();
    if ( timestamp ) *timestamp = f->Timestamp();

    if ( !buffer ) {
        CloseFile( f );
        return len;
    }

    byte* buf = (byte*)malloc( len + 1 );
    f->Read( buf, len );
    buf[len] = 0;
    *buffer = buf;

    CloseFile( f );
    return len;
}

void idFileSystemLocal::FreeFile( void* buffer ) {
    free( buffer );
}

int idFileSystemLocal::WriteFile( const char* relativePath, const void* buffer, int size, const char* basePath ) {
    idFile* f = OpenFileWrite( relativePath, basePath );
    if ( !f ) return -1;

    int res = f->Write( buffer, size );
    CloseFile( f );
    return res;
}

void idFileSystemLocal::RemoveFile( const char* relativePath ) {
    idStr fullPath = RelativePathToOSPath( relativePath, "fs_savepath" );
    SDL_RemovePath( fullPath );
}

bool idFileSystemLocal::RemoveDir( const char* relativePath ) {
    idStr fullPath = RelativePathToOSPath( relativePath, "fs_savepath" );
    return SDL_RemovePath( fullPath ) == 0;
}

bool idFileSystemLocal::RenameFile( const char* relativePath, const char* newName, const char* basePath ) {
    idStr oldPath = RelativePathToOSPath( relativePath, basePath );
    idStr newPath = RelativePathToOSPath( newName, basePath );
    return SDL_RenamePath( oldPath, newPath ) == 0;
}

void idFileSystemLocal::CreateOSPath( const char* OSPath ) {
    idStr path = OSPath;
    path.StripFilename();
    SDL_CreateDirectory( path );
}

idFile* idFileSystemLocal::OpenFileWrite( const char* relativePath, const char* basePath ) {
    const char* osPath = RelativePathToOSPath( relativePath, basePath );
    CreateOSPath( osPath );
    return OpenExplicitFileWrite( osPath );
}

idFile* idFileSystemLocal::OpenFileAppend( const char* relativePath, bool sync, const char* basePath ) {
    const char* osPath = RelativePathToOSPath( relativePath, basePath );
    CreateOSPath( osPath );
    SDL_IOStream* io = OpenOSFile( osPath, FS_APPEND );
    if ( !io ) return NULL;

    idFile_Permanent* f = new idFile_Permanent();
    f->o = io;
    f->mode = ( 1 << FS_WRITE ) | ( 1 << FS_APPEND );
    f->fileSize = DirectFileLength( io );
    f->fullPath = osPath;

    return f;
}

idFile* idFileSystemLocal::OpenFileRead( const char* relativePath, bool allowCopyFiles, const char* gamedir ) {
    return OpenFileReadFlags( relativePath, 0, allowCopyFiles, gamedir );
}

idFile* idFileSystemLocal::OpenFileReadMemory( const char* relativePath, bool allowCopyFiles, const char* gamedir ) {
    idFile* f = OpenFileReadFlags( relativePath, 0, allowCopyFiles, gamedir );
    if ( !f ) return NULL;

    int len = f->Length();
    char* buf = (char*)malloc( len + 1 );
    f->Read( buf, len );
    buf[len] = 0;

    idFile_Memory* mem = new idFile_Memory( relativePath, buf, len );
    mem->TakeDataOwnership();
    delete f;
    return mem;
}

idFile* idFileSystemLocal::OpenFileReadFlags( const char* relativePath, int searchFlags, bool allowCopyFiles, const char* gamedir ) {
    for( int i = 0; i < searchPaths.Num(); i++ ) {
        if ( gamedir && idStr::Icmp( gamedir, searchPaths[i].gamedir ) != 0 ) continue;

        idStr path = BuildOSPath( searchPaths[i].path, searchPaths[i].gamedir, relativePath );
        idFile* f = OpenExplicitFileRead( path );
        if ( f ) return f;
    }
    return NULL;
}

idFile* idFileSystemLocal::OpenFileByMode( const char* relativePath, fsMode_t mode ) {
    if ( mode == FS_READ ) return OpenFileRead( relativePath );
    if ( mode == FS_WRITE ) return OpenFileWrite( relativePath );
    if ( mode == FS_APPEND ) return OpenFileAppend( relativePath );
    return NULL;
}

void idFileSystemLocal::GetExtensionList( const char* extension, idStrList& extensionList ) const
{
	int s, e, l;

	l = idStr::Length( extension );
	s = 0;
	while( 1 )
	{
		e = idStr::FindChar( extension, '|', s, l );
		if( e != -1 )
		{
			extensionList.Append( idStr( extension, s, e ) );
			s = e + 1;
		}
		else
		{
			extensionList.Append( idStr( extension, s, l ) );
			break;
		}
	}
}

static SDL_EnumerationResult SDLCALL EnumCallback(void *userdata, const char *dirname, const char *fname) {
    idStrList* sysFiles = (idStrList*)userdata;
    sysFiles->Append( fname );
    return SDL_ENUM_CONTINUE;
}

int	idFileSystemLocal::ListOSFiles( const char* directory, const char* extension, idStrList& list )
{
    idStrList sysFiles;
    SDL_EnumerateDirectory( directory, EnumCallback, &sysFiles );

    // Filter by extension
    for( int i = 0; i < sysFiles.Num(); i++ ) {
        if ( extension[0] == '/' && extension[1] == 0 ) {
            // Check if it's a directory
            idStr fullPath = directory;
            fullPath.AppendPath( sysFiles[i] );
            if ( IsFolder( fullPath, "" ) ) { // Need relative path for IsFolder logic?
                // IsFolder uses RelativePathToOSPath.
                // Here we have absolute path (if directory is absolute).
                // Or path relative to base.
                // SDL_GetPathInfo needs full path or relative to CWD.
                // Assuming directory is valid path for SDL.
                SDL_PathInfo info;
                if ( SDL_GetPathInfo( fullPath, &info ) == 0 && info.type == SDL_PATHTYPE_DIRECTORY ) {
                    list.Append( sysFiles[i] );
                }
            }
        }
        else if ( idStr::CheckExtension( sysFiles[i], extension ) ) {
            list.Append( sysFiles[i] );
        }
    }

    return list.Num();
}

// ... GetFileList ...
int idFileSystemLocal::GetFileList( const char* relativePath, const idStrList& extensions, idStrList& list, idHashIndex& hashIndex, bool fullRelativePath, bool allowSubdirsForResourcePaks, const char* gamedir )
{
	if( !IsInitialized() )
	{
		idLib::FatalError( "Filesystem call made without initialization\n" );
	}

	if( !extensions.Num() )
	{
		return 0;
	}

	if( !relativePath )
	{
		return 0;
	}

	// search through the path, one element at a time, adding to list
	for( int sp = searchPaths.Num() - 1; sp >= 0; sp-- )
	{
		if( gamedir != NULL && gamedir[0] != 0 )
		{
			if( searchPaths[sp].gamedir != gamedir )
			{
				continue;
			}
		}

		idStr netpath = BuildOSPath( searchPaths[sp].path, searchPaths[sp].gamedir, relativePath );

		for( int i = 0; i < extensions.Num(); i++ )
		{

			// scan for files in the filesystem
			idStrList sysFiles;
			ListOSFiles( netpath, extensions[i], sysFiles );

			// if we are searching for directories, remove . and ..
			if( extensions[i][0] == '/' && extensions[i][1] == 0 )
			{
				sysFiles.Remove( "." );
				sysFiles.Remove( ".." );
			}

			for( int j = 0; j < sysFiles.Num(); j++ )
			{
				// unique the match
				if( fullRelativePath )
				{
					idStr work = relativePath;
					work += "/";
					work += sysFiles[j];
					// idFileSystemLocal::AddUnique( work, list, hashIndex ); // AddUnique is not static, but defined in original file.
                    // Wait, AddUnique was a member. I need to add it to class definition.
                    // Or implement inline.
                    // Implementing inline here:
                    int k, hashKey;
                    const char* name = work.c_str();
                    hashKey = hashIndex.GenerateKey( name );
                    bool found = false;
                    for( k = hashIndex.First( hashKey ); k >= 0; k = hashIndex.Next( k ) )
                    {
                        if( list[k].Icmp( name ) == 0 )
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        k = list.Append( name );
                        hashIndex.Add( hashKey, k );
                    }
				}
				else
				{
					// Inline AddUnique
                    const char* name = sysFiles[j];
                    int k, hashKey;
                    hashKey = hashIndex.GenerateKey( name );
                    bool found = false;
                    for( k = hashIndex.First( hashKey ); k >= 0; k = hashIndex.Next( k ) )
                    {
                        if( list[k].Icmp( name ) == 0 )
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        k = list.Append( name );
                        hashIndex.Add( hashKey, k );
                    }
				}
			}
		}
	}

	return list.Num();
}

idFileList* idFileSystemLocal::ListFiles( const char* relativePath, const char* extension, bool sort, bool fullRelativePath, const char* gamedir )
{
	idHashIndex hashIndex( 4096, 4096 );
	idStrList extensionList;

	idFileList* fileList = new idFileList;
	fileList->basePath = relativePath;

	GetExtensionList( extension, extensionList );

	GetFileList( relativePath, extensionList, fileList->list, hashIndex, fullRelativePath, false, gamedir );

	if( sort )
	{
		fileList->list.SortWithTemplate( idStr::idSort_PathStr() );
	}

	return fileList;
}

int idFileSystemLocal::GetFileListTree( const char* relativePath, const idStrList& extensions, idStrList& list, idHashIndex& hashIndex, bool allowSubdirsForResourcePaks, const char* gamedir )
{
	int i;
	idStrList slash, folders;
    slash.SetGranularity( 16 );
    folders.SetGranularity( 128 );
	idHashIndex folderHashIndex( 1024, 128 );

	// recurse through the subdirectories
	slash.Append( "/" );
	GetFileList( relativePath, slash, folders, folderHashIndex, true, allowSubdirsForResourcePaks, gamedir );
	for( i = 0; i < folders.Num(); i++ )
	{
		if( folders[i][0] == '.' )
		{
			continue;
		}
		if( folders[i].Icmp( relativePath ) == 0 )
		{
			continue;
		}
		GetFileListTree( folders[i], extensions, list, hashIndex, allowSubdirsForResourcePaks, gamedir );
	}

	// list files in the current directory
	GetFileList( relativePath, extensions, list, hashIndex, true, allowSubdirsForResourcePaks, gamedir );

	return list.Num();
}

idFileList* idFileSystemLocal::ListFilesTree( const char* relativePath, const char* extension, bool sort,  bool allowSubdirsForResourcePaks, const char* gamedir )
{
	idHashIndex hashIndex( 4096, 4096 );
	idStrList extensionList;

	idFileList* fileList = new idFileList();
	fileList->basePath = relativePath;
	fileList->list.SetGranularity( 4096 );

	GetExtensionList( extension, extensionList );

	GetFileListTree( relativePath, extensionList, fileList->list, hashIndex, allowSubdirsForResourcePaks, gamedir );

	if( sort )
	{
		fileList->list.SortWithTemplate( idStr::idSort_PathStr() );
	}

	return fileList;
}

void idFileSystemLocal::FreeFileList( idFileList* fileList )
{
	delete fileList;
}

void idFileSystemLocal::Dir_f( const idCmdArgs& args ) {
    // Stub
    idLib::Printf("Dir command\n");
}

void idFileSystemLocal::Path_f( const idCmdArgs& args ) {
    // Stub
    idLib::Printf("Path command\n");
}
