
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

#ifndef __CMDSYSTEM_H__
#define __CMDSYSTEM_H__

#include "CVarSystem.h" // Includes CmdArgs.h, Str.h, Lib.h

/*
===============================================================================

	Console command execution and command text buffering.

===============================================================================
*/

// command flags
typedef enum
{
	CMD_FL_ALL				= -1,
	CMD_FL_CHEAT			= BIT( 0 ),	// command is considered a cheat
	CMD_FL_SYSTEM			= BIT( 1 ),	// system command
	CMD_FL_RENDERER			= BIT( 2 ),	// renderer command
	CMD_FL_SOUND			= BIT( 3 ),	// sound command
	CMD_FL_GAME				= BIT( 4 ),	// game command
	CMD_FL_TOOL				= BIT( 5 )	// tool command
} cmdFlags_t;

// parameters for command buffer stuffing
typedef enum
{
	CMD_EXEC_NOW,						// don't return until completed
	CMD_EXEC_INSERT,					// insert at current position, but don't run yet
	CMD_EXEC_APPEND						// add to end of the command buffer (normal case)
} cmdExecution_t;

// command function
typedef void ( *cmdFunction_t )( const idCmdArgs& args );

// argCompletion_t is defined in CVarSystem.h (actually I defined it there to avoid circular dependency, but it logically belongs here. Original header had it here).
// Since I included CVarSystem.h, I have it.

/*
================================================
idCommandLink
================================================
*/

class idCommandLink
{
public:
	idCommandLink( const char* cmdName, cmdFunction_t function,
				   const char* description, argCompletion_t argCompletion = NULL );
	idCommandLink* 	next;
	const char* 	cmdName_;
	cmdFunction_t	function_;
	const char* 	description_;
	argCompletion_t argCompletion_;
};

// The command system will create commands for all the static definitions
// when it initializes.
idCommandLink* CommandLinks( idCommandLink* cl = NULL );

/*
================================================
The CONSOLE_COMMAND macro
================================================
*/

#if defined ( ID_RETAIL ) && !defined( ID_RETAIL_INTERNAL )
	#define CONSOLE_COMMAND_SHIP			CONSOLE_COMMAND_COMPILE
	#define CONSOLE_COMMAND					CONSOLE_COMMAND_NO_COMPILE
	// We need to disable this warning to get commands that were made friends
	// of classes to compile as inline.
	// warning C4211: nonstandard extension used : redefined extern to static
	//#pragma warning( disable : 4211 )
	// warning C4505: 'xxx' : unreferenced local function has been removed
	//#pragma warning( disable : 4505 )
#else
	#define CONSOLE_COMMAND_SHIP			CONSOLE_COMMAND_COMPILE
	#define CONSOLE_COMMAND					CONSOLE_COMMAND_COMPILE
#endif

// Turn console commands into static inline code, which will cause them to be
// removed from the build.
#define CONSOLE_COMMAND_NO_COMPILE( name, comment, completion ) \
	static inline void name ## _f( const idCmdArgs &args )

#define CONSOLE_COMMAND_COMPILE( name, comment, completion ) \
	void name ## _f( const idCmdArgs &args ); \
	idCommandLink name ## _v( #name, name ## _f, comment, completion  ); \
	void name ## _f( const idCmdArgs &args )

class idCmdSystem
{
public:
	virtual				~idCmdSystem() {}

	virtual void		Init() = 0;
	virtual void		Shutdown() = 0;

	// Registers a command and the function to call for it.
	virtual void		AddCommand( const char* cmdName, cmdFunction_t function, int flags, const char* description, argCompletion_t argCompletion = NULL ) = 0;
	// Removes a command.
	virtual void		RemoveCommand( const char* cmdName ) = 0;
	// Remove all commands with one of the flags set.
	virtual void		RemoveFlaggedCommands( int flags ) = 0;

	// Command and argument completion using callback for each valid string.
	virtual void		CommandCompletion( void( *callback )( const char* s ) ) = 0;
	virtual void		ArgCompletion( const char* cmdString, void( *callback )( const char* s ) ) = 0;

	virtual void		ExecuteCommandText( const char* text ) = 0;
	virtual void		AppendCommandText( const char* text ) = 0;

	// Expose command list for tooling
	virtual int			GetNumCommands( void ) = 0;
	virtual const char *GetCommandName( int index ) = 0;
	virtual const char *GetCommandDescription( int index ) = 0;

	// Adds command text to the command buffer, does not add a final \n
	virtual void		BufferCommandText( cmdExecution_t exec, const char* text ) = 0;
	// Pulls off \n \r or ; terminated lines of text from the command buffer and
	// executes the commands. Stops when the buffer is empty.
	// Normally called once per frame, but may be explicitly invoked.
	virtual void		ExecuteCommandBuffer() = 0;

	// Base for path/file auto-completion.
	virtual void		ArgCompletion_FolderExtension( const idCmdArgs& args, void( *callback )( const char* s ), const char* folder, int stripFolder, ... ) = 0;
	// Base for decl name auto-completion.
	virtual void		ArgCompletion_DeclName( const idCmdArgs& args, void( *callback )( const char* s ), int type ) = 0;

	// Adds to the command buffer in tokenized form ( CMD_EXEC_NOW or CMD_EXEC_APPEND only )
	virtual void		BufferCommandArgs( cmdExecution_t exec, const idCmdArgs& args ) = 0;

	// Setup a reloadEngine to happen on next command run, and give a command to execute after reload
	virtual void		SetupReloadEngine( const idCmdArgs& args ) = 0;
	virtual bool		PostReloadEngine() = 0;

	// Default argument completion functions.
	static void			ArgCompletion_DefFile( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_Boolean( const idCmdArgs& args, void( *callback )( const char* s ) );
	template<int min, int max>
	static void			ArgCompletion_Integer( const idCmdArgs& args, void( *callback )( const char* s ) );
	template<const char** strings>
	static void			ArgCompletion_String( const idCmdArgs& args, void( *callback )( const char* s ) );
	template<int type>
	static void			ArgCompletion_Decl( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_FileName( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_MapName( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_MapNameNoJson( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_ModelName( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_SoundName( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_ImageName( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_VideoName( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_ConfigName( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_SaveGame( const idCmdArgs& args, void( *callback )( const char* s ) );
	static void			ArgCompletion_DemoName( const idCmdArgs& args, void( *callback )( const char* s ) );
};

extern idCmdSystem* 	cmdSystem;

ID_INLINE void idCmdSystem::ArgCompletion_DefFile( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "def/", true, ".def", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_Boolean( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	callback( idStr( args.Argv( 0 ) ) + " 0" );
	callback( idStr( args.Argv( 0 ) ) + " 1" );
}

template<int min, int max> ID_INLINE void idCmdSystem::ArgCompletion_Integer( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	for( int i = min; i <= max; i++ )
	{
		callback( idStr( args.Argv( 0 ) ) + " " + idStr( i ) );
	}
}

template<const char** strings> ID_INLINE void idCmdSystem::ArgCompletion_String( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	for( int i = 0; strings[i]; i++ )
	{
		callback( idStr( args.Argv( 0 ) ) + " " + strings[i] );
	}
}

template<int type> ID_INLINE void idCmdSystem::ArgCompletion_Decl( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_DeclName( args, callback, type );
}

ID_INLINE void idCmdSystem::ArgCompletion_FileName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", true, "", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_MapName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "maps/", true, ".map", ".json", ".gltf", ".glb", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_MapNameNoJson( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "maps/", true, ".map", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_ModelName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "models/", false, ".lwo", ".ase", ".md5mesh", ".ma", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_SoundName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "sound/", false, ".wav", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_ImageName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", false, ".tga", ".dds", ".jpg", ".pcx", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_VideoName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", false, ".bik", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_ConfigName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", true, ".cfg", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_SaveGame( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "SaveGames/", true, ".save", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_DemoName( const idCmdArgs& args, void( *callback )( const char* s ) )
{
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "demos/", true, ".demo", NULL );
}

#endif /* !__CMDSYSTEM_H__ */
