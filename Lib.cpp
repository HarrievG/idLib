#include "Lib.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include "Str.h"
#include "StrStatic.h"
#include "Dict.h"
#include "math/Vector.h"
#include "math/Polynomial.h"
#include "math/Simd.h"
#include <SDL3/SDL.h>
#include "CmdSystem.h"
#include "CVarSystem.h"
#include "FileSystem.h"

/*
===============================================================================

Byte order functions

===============================================================================
*/

/*
================
FloatSwap
================
*/
ID_INLINE static float FloatSwap( float f ) {
	union {
		float	f;
		unsigned int u;
	} id_attribute((may_alias)) dat;

	dat.f = f;
	dat.u = SDL_Swap32(dat.u);

	return dat.f;
}

/*
=====================================================================
RevBytesSwap

Reverses byte order in place.

INPUTS
bp       bytes to reverse
elsize   size of the underlying data type
elcount  number of elements to swap

RESULTS
Reverses the byte order in each of elcount elements.
===================================================================== */
ID_INLINE static void RevBytesSwap( void *bp, int elsize, int elcount ) {
	unsigned char *p, *q;

	p = ( unsigned char * ) bp;

	if ( elsize == 2 ) {
		q = p + 1;
		while ( elcount-- ) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			p += 2;
			q += 2;
		}
		return;
	}

	while ( elcount-- ) {
		q = p + elsize - 1;
		while ( p < q ) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			++p;
			--q;
		}
		p += elsize >> 1;
	}
}

/*
=====================================================================
RevBytesSwap

Reverses byte order in place, then reverses bits in those bytes

INPUTS
bp       bitfield structure to reverse
elsize   size of the underlying data type

RESULTS
Reverses the bitfield of size elsize.
===================================================================== */
ID_INLINE static void RevBitFieldSwap( void *bp, int elsize) {
	int i;
	unsigned char *p, t, v;

	LittleRevBytes( bp, elsize, 1 );

	p = (unsigned char *) bp;
	while ( elsize-- ) {
		v = *p;
		t = 0;
		for (i = 7; i; i--) {
			t <<= 1;
			v >>= 1;
			t |= v & 1;
		}
		*p++ = t;
	}
}

/*
================
SixtetsForIntLittle
================
*/
ID_INLINE static void SixtetsForIntLittle( byte *out, int src) {
	byte *b = (byte *)&src;
	out[0] = ( b[0] & 0xfc ) >> 2;
	out[1] = ( ( b[0] & 0x3 ) << 4 ) + ( ( b[1] & 0xf0 ) >> 4 );
	out[2] = ( ( b[1] & 0xf ) << 2 ) + ( ( b[2] & 0xc0 ) >> 6 );
	out[3] = b[2] & 0x3f;
}

/*
================
SixtetsForIntBig
TTimo: untested - that's the version from initial base64 encode
================
*/
ID_INLINE static void SixtetsForIntBig( byte *out, int src) {
	for( int i = 0 ; i < 4 ; i++ ) {
		out[i] = src & 0x3f;
		src >>= 6;
	}
}

/*
================
IntForSixtetsLittle
================
*/
ID_INLINE static int IntForSixtetsLittle( byte *in ) {
	int ret = 0;
	byte *b = (byte *)&ret;
	b[0] |= in[0] << 2;
	b[0] |= ( in[1] & 0x30 ) >> 4;
	b[1] |= ( in[1] & 0xf ) << 4;
	b[1] |= ( in[2] & 0x3c ) >> 2;
	b[2] |= ( in[2] & 0x3 ) << 6;
	b[2] |= in[3];
	return ret;
}

/*
================
IntForSixtetsBig
TTimo: untested - that's the version from initial base64 decode
================
*/
ID_INLINE static int IntForSixtetsBig( byte *in ) {
	int ret = 0;
	ret |= in[0];
	ret |= in[1] << 6;
	ret |= in[2] << 2*6;
	ret |= in[3] << 3*6;
	return ret;
}

/*
==========
Swap_IsBigEndian
==========
*/
bool Swap_IsBigEndian( void ) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	return false;
#else
	return true;
#endif
}

short	BigShort( short l ) {
	return SDL_Swap16BE(l);
}

short	LittleShort( short l ) {
	return SDL_Swap16LE(l);
}

int		BigInt( int l ) {
	return SDL_Swap32BE(l);
}

int		LittleInt( int l ) {
	return SDL_Swap32LE(l);
}

float	BigFloat( float l ) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	return FloatSwap(l);
#else
	return l;
#endif
}

float	LittleFloat( float l ) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	return l;
#else
	return FloatSwap(l);
#endif
}

void	BigRevBytes( void *bp, int elsize, int elcount ) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	RevBytesSwap(bp, elsize, elcount);
#else
	return;
#endif
}

void	LittleRevBytes( void *bp, int elsize, int elcount ){
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	return;
#else
	RevBytesSwap(bp, elsize, elcount);
#endif
}

void	LittleBitField( void *bp, int elsize ){
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	return;
#else
	RevBitFieldSwap(bp, elsize);
#endif
}

void	SixtetsForInt( byte *out, int src) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	SixtetsForIntLittle(out, src);
#else
	SixtetsForIntBig(out, src);
#endif
}

int		IntForSixtets( byte *in ) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	return IntForSixtetsLittle(in);
#else
	return IntForSixtetsBig(in);
#endif
}


/*
===============================================================================

Assertion

===============================================================================
*/

namespace idLib {

#define		MAX_CONSOLE_LINES	32
int			com_numConsoleLines;
idCmdArgs	com_consoleLines[MAX_CONSOLE_LINES];


	void AddStartupCommands()
	{
		// quote every token, so args with semicolons can work
		for( int i = 0; i < com_numConsoleLines; i++ )
		{
			if( !com_consoleLines[i].Argc() )
			{
				continue;
			}
			// directly as tokenized so nothing gets screwed
			cmdSystem->BufferCommandArgs( CMD_EXEC_APPEND, com_consoleLines[i] );
		}
	}


	void ParseCommandLine( int argc, const char* const* argv )
	{
		int i, current_count;

		com_numConsoleLines = 0;
		current_count = 0;
		// API says no program path
		for( i = 0; i < argc; i++ )
		{
			if( idStr::Icmp( argv[ i ], "+connect_lobby" ) == 0 )
			{
				// Handle Steam bootable invites.
				// RB begin
	// #if defined(_WIN32)
	// 			session->HandleBootableInvite( _atoi64( argv[ i + 1 ] ) );
	// #else
	// 			session->HandleBootableInvite( atol( argv[ i + 1 ] ) );
	// #endif
				// RB end
			}
			else if( argv[ i ][ 0 ] == '+' )
			{
				com_numConsoleLines++;
				com_consoleLines[ com_numConsoleLines - 1 ].AppendArg( argv[ i ] + 1 );
			}
			else
			{
				if( !com_numConsoleLines )
				{
					com_numConsoleLines++;
				}
				com_consoleLines[ com_numConsoleLines - 1 ].AppendArg( argv[ i ] );
			}
		}
	}


	/*
	==================
	StartupVariable

	Searches for command line parameters that are set commands.
	If match is not NULL, only that cvar will be looked for.
	That is necessary because cddir and basedir need to be set
	before the filesystem is started, but all other sets should
	be after execing the config and default.
	==================
	*/
	void SetStartupVariable( const char* match )
	{
		int i = 0;
		while(	i < com_numConsoleLines )
		{
			if( strcmp( com_consoleLines[ i ].Argv( 0 ), "set" ) != 0 )
			{
				i++;
				continue;
			}
			const char* s = com_consoleLines[ i ].Argv( 1 );

			if( !match || !idStr::Icmp( s, match ) )
			{
				cvarSystem->SetCVarString( s, com_consoleLines[ i ].Argv( 2 ) );
			}
			i++;
		}
	}

	void WriteConfigToFile( const char *filename ) {
		idFile *f = fileSystem->OpenFileWrite( filename );
		if ( !f ) {
			Printf( "Couldn't write %s.\n", filename );
			return;
		}
		cvarSystem->WriteFlaggedVariables( CVAR_ARCHIVE, "set", f );
		fileSystem->CloseFile( f );
	}

	void Printf( const char *fmt, ... ) {
		va_list argptr;
		va_start( argptr, fmt );
		vprintf( fmt, argptr );
		va_end( argptr );
	}

	void Warning( const char *fmt, ... ) {
		va_list argptr;
		printf( "WARNING: " );
		va_start( argptr, fmt );
		vprintf( fmt, argptr );
		va_end( argptr );
		printf( "\n" );
	}


	void Error( const char *fmt, ... ) {
		va_list argptr;
		printf( "!!=- ERROR -=!! :  " );
		va_start( argptr, fmt );
		vprintf( fmt, argptr );
		va_end( argptr );
		printf( "\n" );
	}

	void FatalError( const char *fmt, ... ) {
		va_list argptr;
		printf( "FATAL ERROR: " );
		va_start( argptr, fmt );
		vprintf( fmt, argptr );
		va_end( argptr );
		printf( "\n" );
		exit( 1 );
	}



	/*
	================
	idLib::Init
	================
	*/
	void Init( ) {
		assert( sizeof( bool ) == 1 );

		//isMainThread = 1;
		//mainThreadInitialized = 1; // note that the thread-local isMainThread is now valid

		// initialize little/big endian conversion
		//Swap_Init( );
		Mem_Init();
		// init string memory allocator
		idStr::InitMemory( );

		// initialize generic SIMD implementation
		SIMD_Init( );

		// initialize math
		idMath::Init( );

		// test idMatX
		// idMatX::Test();

		// test idPolynomial
	#ifdef _DEBUG
		idPolynomial::Test( );
	#endif

		// initialize the dictionary string pools
		idDict::Init( );
	}


}


/*
================
PackColor
================
*/
dword		 PackColor( const idVec4 &color ) {
	byte dx = idMath::Ftob( color.x * 255.0f );
	byte dy = idMath::Ftob( color.y * 255.0f );
	byte dz = idMath::Ftob( color.z * 255.0f );
	byte dw = idMath::Ftob( color.w * 255.0f );
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 ) | ( dw << 24 );
}

/*
================
UnpackColor
================
*/
void UnpackColor( const dword color, idVec4 &unpackedColor ) {
	unpackedColor.Set(
		( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ), ( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ), ( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ), ( ( color >> 24 ) & 255 ) * ( 1.0f / 255.0f ) );
}

/*
================
PackColor
================
*/
dword PackColor( const idVec3 &color ) {
	byte dx = idMath::Ftob( color.x * 255.0f );
	byte dy = idMath::Ftob( color.y * 255.0f );
	byte dz = idMath::Ftob( color.z * 255.0f );
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 );
}

/*
================
UnpackColor
================
*/
void UnpackColor( const dword color, idVec3 &unpackedColor ) {
	unpackedColor.Set( ( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ), ( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ), ( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ) );
}

