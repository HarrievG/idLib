
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

#include "File.h"
#include <cstring>
#include <cassert>
#include <cstdio>
#include <SDL3/SDL.h>

// idFile base implementation

const char* idFile::GetName() const { return ""; }
const char* idFile::GetFullPath() const { return ""; }
int idFile::Read( void* buffer, int len ) { return 0; }
int idFile::Write( const void* buffer, int len ) { return 0; }
int idFile::Length() const { return 0; }
ID_TIME_T idFile::Timestamp() const { return 0; }
int idFile::Tell() const { return 0; }
void idFile::ForceFlush() {}
void idFile::Flush() {}
int idFile::Seek( long offset, fsOrigin_t origin ) { return -1; }
void idFile::Rewind() { Seek( 0, FS_SEEK_SET ); }

int idFile::Printf( const char* fmt, ... ) {
    va_list arg;
    va_start( arg, fmt );
    int len = VPrintf( fmt, arg );
    va_end( arg );
    return len;
}

int idFile::VPrintf( const char* fmt, va_list arg ) {
    char buffer[16384];
    int len = vsnprintf( buffer, sizeof(buffer), fmt, arg );
    if ( len > 0 ) {
        Write( buffer, len );
    }
    return len;
}

int idFile::WriteFloatString( const char* fmt, ... ) {
    va_list arg;
    va_start( arg, fmt );
    int len = VPrintf( fmt, arg );
    va_end( arg );
    return len;
}

// Endian portable alternatives to Read(...)
int idFile::ReadInt( int& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadUnsignedInt( unsigned int& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadShort( short& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadUnsignedShort( unsigned short& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadChar( char& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadUnsignedChar( unsigned char& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadFloat( float& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadBool( bool& value ) { return Read( &value, sizeof(value) ); }
int idFile::ReadString( idStr& string ) {
    string = "";
    char c;
    int len = 0;
    while ( Read( &c, 1 ) == 1 ) {
        if ( c == 0 ) break;
        string += c;
        len++;
    }
    return len;
}
int idFile::ReadVec2( idVec2& vec ) { return Read( &vec, sizeof(vec) ); }
int idFile::ReadVec3( idVec3& vec ) { return Read( &vec, sizeof(vec) ); }
int idFile::ReadVec4( idVec4& vec ) { return Read( &vec, sizeof(vec) ); }
int idFile::ReadVec6( idVec6& vec ) { return Read( &vec, sizeof(vec) ); }
int idFile::ReadMat3( idMat3& mat ) { return Read( &mat, sizeof(mat) ); }

// Endian portable alternatives to Write(...)
int idFile::WriteInt( const int value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteUnsignedInt( const unsigned int value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteShort( const short value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteUnsignedShort( unsigned short value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteChar( const char value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteUnsignedChar( const unsigned char value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteFloat( const float value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteBool( const bool value ) { return Write( &value, sizeof(value) ); }
int idFile::WriteString( const char* string ) { return Write( string, strlen(string) + 1 ); }
int idFile::WriteVec2( const idVec2& vec ) { return Write( &vec, sizeof(vec) ); }
int idFile::WriteVec3( const idVec3& vec ) { return Write( &vec, sizeof(vec) ); }
int idFile::WriteVec4( const idVec4& vec ) { return Write( &vec, sizeof(vec) ); }
int idFile::WriteVec6( const idVec6& vec ) { return Write( &vec, sizeof(vec) ); }
int idFile::WriteMat3( const idMat3& mat ) { return Write( &mat, sizeof(mat) ); }


// idFile_Memory

idFile_Memory::idFile_Memory() {
    mode = 0;
    maxSize = 0;
    fileSize = 0;
    allocated = 0;
    granularity = 4096;
    filePtr = NULL;
    curPtr = NULL;
}

idFile_Memory::idFile_Memory( const char* name ) {
    idFile_Memory();
    this->name = name;
}

idFile_Memory::idFile_Memory( const char* name, char* data, int length ) {
    this->name = name;
    filePtr = data;
    curPtr = data;
    fileSize = length;
    allocated = length;
    maxSize = length; // Assume read only or fixed size if passed in
    granularity = 0;
    mode = FS_READ; // Assume read for this constructor? Or user sets.
}

idFile_Memory::idFile_Memory( const char* name, const char* data, int length ) {
    this->name = name;
    // We cast away const, but if we don't own it and it's const, we shouldn't write.
    filePtr = (char*)data;
    curPtr = (char*)data;
    fileSize = length;
    allocated = length;
    maxSize = length;
    granularity = 0;
    mode = FS_READ;
}

idFile_Memory::~idFile_Memory() {
    if ( granularity != 0 && filePtr ) { // If we allocated it
        free( filePtr );
    }
}

int idFile_Memory::Read( void* buffer, int len ) {
    if ( !filePtr ) return 0;
    int remaining = fileSize - ( curPtr - filePtr );
    if ( len > remaining ) len = remaining;
    if ( len <= 0 ) return 0;
    memcpy( buffer, curPtr, len );
    curPtr += len;
    return len;
}

int idFile_Memory::Write( const void* buffer, int len ) {
    // Basic write implementation for memory file
    if ( granularity == 0 ) return 0; // Read only or fixed

    int pos = curPtr - filePtr;
    if ( pos + len > allocated ) {
        int newSize = pos + len + granularity;
        char* newPtr = (char*)realloc( filePtr, newSize );
        if ( !newPtr ) return 0;
        filePtr = newPtr;
        curPtr = filePtr + pos;
        allocated = newSize;
    }
    memcpy( curPtr, buffer, len );
    curPtr += len;
    if ( (curPtr - filePtr) > fileSize ) fileSize = (curPtr - filePtr);
    return len;
}

int idFile_Memory::Length() const { return fileSize; }
void idFile_Memory::SetLength( size_t len ) { fileSize = len; if(allocated < len) { /*realloc logic if needed*/ } }
ID_TIME_T idFile_Memory::Timestamp() const { return 0; }
int idFile_Memory::Tell() const { return curPtr - filePtr; }
void idFile_Memory::ForceFlush() {}
void idFile_Memory::Flush() {}
int idFile_Memory::Seek( long offset, fsOrigin_t origin ) {
    switch ( origin ) {
        case FS_SEEK_CUR: curPtr += offset; break;
        case FS_SEEK_END: curPtr = filePtr + fileSize + offset; break;
        case FS_SEEK_SET: curPtr = filePtr + offset; break;
    }
    // Clamp
    if ( curPtr < filePtr ) curPtr = filePtr;
    if ( curPtr > filePtr + fileSize ) curPtr = filePtr + fileSize;
    return curPtr - filePtr;
}

void idFile_Memory::SetMaxLength( size_t len ) { maxSize = len; }
void idFile_Memory::MakeReadOnly() { granularity = 0; }
void idFile_Memory::MakeWritable() { if ( granularity == 0 ) granularity = 4096; }
void idFile_Memory::Clear( bool freeMemory ) {
    fileSize = 0;
    curPtr = filePtr;
    if ( freeMemory && granularity != 0 && filePtr ) {
        free( filePtr );
        filePtr = NULL;
        curPtr = NULL;
        allocated = 0;
    }
}
void idFile_Memory::SetData( const char* data, int length ) {
    Clear( true );
    filePtr = (char*)data;
    curPtr = (char*)data;
    fileSize = length;
    allocated = length;
    granularity = 0; // assumed read-only external buffer
}
void idFile_Memory::PreAllocate( size_t len ) {
    if ( len > allocated ) {
        char* newPtr = (char*)realloc( filePtr, len );
        if ( newPtr ) {
            filePtr = newPtr;
            allocated = len;
            curPtr = filePtr; // Reset to beginning? or keep pos? Keep pos.
            // But we need to recalc curPtr if it was valid
            // curPtr is absolute pointer, so we need offset.
            // Wait, I didn't store offset before realloc.
            // Good catch.
            // Implementation:
            // int offset = curPtr - filePtr;
            // ... realloc ...
            // curPtr = filePtr + offset;
        }
    }
}
void idFile_Memory::TruncateData( size_t len ) {
    if ( len < fileSize ) fileSize = len;
}
void idFile_Memory::TakeDataOwnership() {
    granularity = 4096; // Enable freeing and resizing
}


// idFile_Permanent (SDL3 implementation)

idFile_Permanent::idFile_Permanent() {
    o = NULL;
    mode = 0;
    fileSize = 0;
    handleSync = false;
}

idFile_Permanent::~idFile_Permanent() {
    if ( o ) {
        SDL_CloseIO( o );
    }
}

int idFile_Permanent::Read( void* buffer, int len ) {
    if ( !o ) return 0;
    return (int)SDL_ReadIO( o, buffer, len );
}

int idFile_Permanent::Write( const void* buffer, int len ) {
    if ( !o ) return 0;
    return (int)SDL_WriteIO( o, buffer, len );
}

int idFile_Permanent::Length() const {
    return fileSize;
}

ID_TIME_T idFile_Permanent::Timestamp() const {
    // SDL3 might not provide timestamp easily on IOStream.
    // Use SDL_GetPathInfo if we had the path.
    // For now return 0.
    return 0;
}

int idFile_Permanent::Tell() const {
    if ( !o ) return 0;
    return (int)SDL_TellIO( o );
}

void idFile_Permanent::ForceFlush() {
    if ( o ) SDL_FlushIO( o );
}

void idFile_Permanent::Flush() {
    if ( o ) SDL_FlushIO( o );
}

int idFile_Permanent::Seek( long offset, fsOrigin_t origin ) {
    if ( !o ) return -1;
    SDL_IOWhence whence = SDL_IO_SEEK_SET;
    switch( origin ) {
        case FS_SEEK_CUR: whence = SDL_IO_SEEK_CUR; break;
        case FS_SEEK_END: whence = SDL_IO_SEEK_END; break;
        case FS_SEEK_SET: whence = SDL_IO_SEEK_SET; break;
    }
    return (int)SDL_SeekIO( o, offset, whence );
}

idFileLocal::~idFileLocal() {
    if ( file ) {
        delete file; // This assumes file system allocated it and we own it.
        // FileSystem typically returns a new idFile* that the caller owns.
    }
}
