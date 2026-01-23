
#include "CVarSystem.h"
#include "CmdSystem.h"
#include "FileSystem.h"
#include "Lib.h"
#include "Str.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "tools/compiler_public.h"

// Mocking common if needed, but we used idLib::Printf/Warning/FatalError.

// We use a simple check macro that exits if false
#define CHECK( x ) if ( !(x) ) { std::cerr << "TEST FAILED: " << #x << " at " << __FILE__ << ":" << __LINE__ << std::endl; exit(1); }


// Register a CVar
idCVar testCVar( "test_cvar", "10", CVAR_SYSTEM | CVAR_INTEGER | CVAR_ARCHIVE, "A test cvar" );

void TestCommand( const idCmdArgs& args ) {
    idLib::Printf("Test Command Executed with %d args.\n", args.Argc());
    for( int i=0; i<args.Argc(); i++ ) {
        idLib::Printf("Arg %d: %s\n", i, args.Argv(i));
    }
}

void TestIdStr() {
    idLib::Printf("\n--- Extended idStr Testing ---\n");

    // Constructors
    idStr s1;
    idStr s2("Hello");
    idStr s3 = "World";
    idStr s4(3.14f);
    idStr s5(123);
    idStr s6(true);

    CHECK( s1.Length() == 0 );
    CHECK( s2.Length() == 5 );
    CHECK( s3 == "World" );
    CHECK( s5 == "123" );

    idLib::Printf("Constructors passed.\n");

    // Concatenation
    idStr s7 = s2 + " " + s3;
    CHECK( s7 == "Hello World" );
    s7 += "!";
    CHECK( s7 == "Hello World!" );

    idLib::Printf("Concatenation passed: %s\n", s7.c_str());

    // Comparison
    CHECK( s2.Cmp("Hello") == 0 );
    CHECK( s2.Icmp("hello") == 0 );
    CHECK( s2 != s3 );

    idLib::Printf("Comparison passed.\n");

    // Search and Replace
    idStr s8 = "The quick brown fox";
    CHECK( s8.Find('q') == 4 );
    s8.Replace("brown", "red");
    CHECK( s8 == "The quick red fox" );

    idLib::Printf("Find/Replace passed: %s\n", s8.c_str());

    // Path manipulation
    idStr path = "base/maps/test.map";
    idStr ext;
    path.ExtractFileExtension(ext);
    CHECK( ext == "map" );

    idStr name;
    path.ExtractFileName(name);
    CHECK( name == "test.map" );

    idStr base;
    path.ExtractFileBase(base);
    CHECK( base == "test" );

    path.StripFileExtension();
    CHECK( path == "base/maps/test" );

    idLib::Printf("Path manipulation passed.\n");

    // Formatting
    idStr fmt;
	sprintf(fmt, "Value: %d", 42);
    CHECK( fmt == "Value: 42" );

    idLib::Printf("Formatting passed: %s\n", fmt.c_str());

    idLib::Printf("idStr tests complete.\n");
}

int main( int argc, char* argv[] ) {
    idLib::Printf("Initializing systems...\n");

	idLib::Init();

    // Initialize Cmd System
    cmdSystem->Init();

    // Initialize CVar System
    cvarSystem->Init();

    // Ensure static cvars are registered (sets staticVars to UINTPTR_MAX so subsequent cvars register immediately)
    idCVar::RegisterStaticVars();

	cmdSystem->ExecuteCommandText( "set fs_savepath C:/Users/06162/Saved Games/id Software/RBDOOM 3 BFG");

    // Initialize File System
    fileSystem->Init();
	
	

	idLib::Printf("Test CVar initial value: %d\n", testCVar.GetInteger());

	cmdSystem->ExecuteCommandText( "exec default.cfg");
	cmdSystem->ExecuteCommandBuffer( );
	cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );

    if ( testCVar.GetInteger() != 10 ) {
        idLib::Printf("testCVar.GetInteger() = %d (expected 10,was it already changed?). String value: '%s'\n", testCVar.GetInteger(), testCVar.GetString());
    }

    idLib::Printf("Test CVar initial value: %d\n", testCVar.GetInteger());

    // Register a command
    cmdSystem->AddCommand( "testcmd", TestCommand, CMD_FL_SYSTEM, "A test command" );

    // Execute a command
    cmdSystem->ExecuteCommandText( "testcmd arg1 arg2" );

    // Change CVar via command
    cmdSystem->ExecuteCommandText( "set test_cvar 2000" );

    CHECK( testCVar.GetInteger() == 2000 );
    idLib::Printf("Test CVar new value: %d\n", testCVar.GetInteger());

	cmdSystem->ExecuteCommandText( "path");

    // List files
    idLib::Printf("Listing files in root:\n");
    cmdSystem->ExecuteCommandText( "dir . " );
	cmdSystem->ExecuteCommandText( "dir / / " );

    // Run extended idStr tests
    TestIdStr();

	idLib::Printf( "- CVARS: \n" );
	int numCVars = cvarSystem->GetNumCVars( );
	for ( int i = 0; i < numCVars; i++ ) {
		idCVar *cvar = cvarSystem->GetCVarByIndex( i );
		if ( !cvar ) continue;

		//if (!m_cvarFilter.PassFilter(cvar->GetName())) continue;

		idLib::Printf( "%s \n", cvar->GetName( ) );
	}
	idLib::Printf( "- COMMANDS: \n" );
	int numCmds = cmdSystem->GetNumCommands( );
	for ( int i = 0; i < numCmds; i++ ) {
		const char *name = cmdSystem->GetCommandName( i );
		const char *desc = cmdSystem->GetCommandDescription( i );

		//if ( !name || !m_cmdFilter.PassFilter( name ) ) continue;

		idLib::Printf( "%s \n  -> %s \n", name , desc );
	}

	idLib::WriteConfigToFile( "default.cfg" );
    // Verify dmap linkage
    idCmdArgs args;

	args.AppendArg("dmap" );
	args.AppendArg("obj" );
	args.AppendArg("glview" );
	args.AppendArg("test.map" );
    Rogmap_f( args ); // Just verifying it links, don't actually run it without args


	

	// Shutdown
	fileSystem->Shutdown( false );
	cmdSystem->Shutdown( );
	cvarSystem->Shutdown();

    idLib::Printf("Systems shutdown.\n");

    std::cout << "[Test] SUCCESS: All checks passed." << std::endl;
    return 0;
}
