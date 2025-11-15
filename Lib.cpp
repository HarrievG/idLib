#include "Lib.h"
#include "Str.h"
#include "Dict.h"
#include "math/Vector.h"
#include "math/Polynomial.h"
#include "math/Simd.h"
#include <SDL3/SDL.h>


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
	register unsigned char *p, *q;

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

void AssertFailed( const char *file, int line, const char *expression ) {
	//idLib::sys->DebugPrintf( "\n\nASSERTION FAILED!\n%s(%d): '%s'\n", file, line, expression );
#ifdef _MSC_VER
	__debugbreak();
	//if (dev_fatalAssert.GetBool())
		_exit(1);
#elif defined(__unix__)
	// __builtin_trap() causes an illegal instruction which is kinda ugly.
	// especially if you'd like to be able to continue after the assertion during debugging
	raise(SIGTRAP); // this will break into the debugger.
#elif defined( __GNUC__ )
	__builtin_trap();
	_exit(1);
#endif

}