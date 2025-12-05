
#include "CVarSystem.h"
#include "CmdSystem.h"
#include "FileSystem.h"
#include "Lib.h"
#include <cstdio>

// Mocking common if needed, but we used idLib::Printf/Warning/FatalError.

void TestCommand( const idCmdArgs& args ) {
    idLib::Printf("Test Command Executed with %d args.\n", args.Argc());
    for( int i=0; i<args.Argc(); i++ ) {
        idLib::Printf("Arg %d: %s\n", i, args.Argv(i));
    }
}

int main( int argc, char* argv[] ) {
    idLib::Printf("Initializing systems...\n");

    // Initialize CVar System
    cvarSystem->Init();

    // Initialize Cmd System
    cmdSystem->Init();

    // Initialize File System
    fileSystem->Init();

    // Register a CVar
    idCVar testCVar( "test_cvar", "10", CVAR_SYSTEM | CVAR_INTEGER, "A test cvar" );

    idLib::Printf("Test CVar initial value: %d\n", testCVar.GetInteger());

    // Register a command
    cmdSystem->AddCommand( "testcmd", TestCommand, CMD_FL_SYSTEM, "A test command" );

    // Execute a command
    cmdSystem->ExecuteCommandText( "testcmd arg1 arg2" );

    // Change CVar via command
    cmdSystem->ExecuteCommandText( "set test_cvar 20" );

    idLib::Printf("Test CVar new value: %d\n", testCVar.GetInteger());

    // List files
    idLib::Printf("Listing files in root:\n");
    cmdSystem->ExecuteCommandText( "dir /" );

    // Shutdown
    fileSystem->Shutdown(false);
    cmdSystem->Shutdown();
    cvarSystem->Shutdown();

    idLib::Printf("Systems shutdown.\n");

    return 0;
}
