/*
Oolong Engine for the iPhone / iPod touch
Copyright (c) 2007-2008 Wolfgang Engel  http://code.google.com/p/oolongengine/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#ifndef FIXEDPOINT_H_
#define FIXEDPOINT_H_

//#if defined(BUILD_OGLES) || defined(BUILD_D3DM)
	#include "FixedPointAPI.h"
//#else
//	#define VERTTYPE float
//	#ifdef FIXEDPOINTENABLE
//		#error Build option not supported: FIXEDPOINTENABLE
//	#endif
//#endif

// Define a 64-bit type for various platforms
#if defined(__int64) || defined(WIN32)
#define INT64BIT __int64
#elif defined(TInt64)
#define INT64BIT TInt64
#else
#define INT64BIT long long int
#endif

typedef struct _LARGE_INTEGER
{
	union
	{
		struct
		{
			unsigned long LowPart;
			long HighPart;
		};
		INT64BIT QuadPart;
	};
} LARGE_INTEGER, *PLARGE_INTEGER;


// Fixed-point macros
#define F2X(f)		( (int) ( (f)*(65536) ) )
//#define F2X(f)		(((int) ( f))<<16)

#define X2F(x)		((float)(x)/65536.0f)
#define XMUL(a,b)	( (int)( ((INT64BIT)(a)*(b)) / 65536 ) )
#define XDIV(a,b)	( (int)( (((INT64BIT)(a))<<16)/(b) ) )
#define _ABS(a)		((a) <= 0 ? -(a) : (a) )

// Define trig table macros
#include "MathTable.h"

// Useful values
#define PIOVERTWOf	(3.1415926535f / 2.0f)
#define PIf			(3.1415926535f)
#define TWOPIf		(3.1415926535f * 2.0f)
#define ONEf		(1.0f)

#define PIOVERTWOx	F2X(PIOVERTWOf)
#define PIx			F2X(PIf)
#define TWOPIx		F2X(TWOPIf)
#define ONEx		F2X(ONEf)

// Fixed-point trig function lookups
#define XCOS(x)		(cos_val[(XMUL(((XDIV((x)<0? -(x):(x), TWOPIx)) & 0x0000FFFF), (NUM_ENTRIES-1)))])
#define XSIN(x)		(sin_val[(XMUL(((XDIV((x)<0 ? PIx-(x):(x), TWOPIx)) & 0x0000FFFF), (NUM_ENTRIES-1)))])
#define XTAN(x)		( (x)<0 ? -tan_val[(XMUL(((XDIV(-(x), TWOPIx)) & 0x0000FFFF), (NUM_ENTRIES-1)))] : tan_val[(XMUL(((XDIV(x, TWOPIx)) & 0x0000FFFF), (NUM_ENTRIES-1)))] )
#define XACOS(x)	(acos_val[XMUL(((((x) + F2X(1.0f))>>1) & 0x0000FFFF), (NUM_ENTRIES-1))])


// Floating-point trig functions lookups (needed by some tools chains that have problems with real math functions) 
#ifdef USETRIGONOMETRICLOOKUPTABLES

	// If trig tables are forced ON in non-fixed-point builds then convert fixed-point trig tables results to float
	#define	FCOS(x)				X2F(XCOS(F2X(x)))
	#define	FSIN(x)				X2F(XSIN(F2X(x)))
	#define	FTAN(x)				X2F(XTAN(F2X(x)))
	#define	FACOS(x)			X2F(XACOS(F2X(x)))

#else

	// Trig abstraction macros default to normal math trig functions for full float mode
	#define	FCOS(x)				((float)cos(x))
	#define	FSIN(x)				((float)sin(x))
	#define	FTAN(x)				((float)tan(x))
	#define	FACOS(x)			((float)acos(x))

#endif

#ifdef FIXEDPOINTENABLE

// this one is defined in FixedPoint.cpp
#define FIXSQRT(x)			((unsigned int)fixsqrt(unsigned int n))
#endif


// Fixed/float macro abstraction
#ifdef FIXEDPOINTENABLE

	// Fixed-point operations, including trig tables 
	#define VERTTYPEMUL(a,b)			XMUL(a,b)
	#define VERTTYPEDIV(a,b)			XDIV(a,b)
	#define VERTTYPEABS(a)				_ABS(a)

	#define f2vt(f) 					F2X(f)
	#define vt2f(x) 					X2F(x)

	#define PIOVERTWO				PIOVERTWOx
	#define PI						PIx
	#define TWOPI					TWOPIx
	#define ONE						ONEx

	#define	COS(x)					XCOS(x)
	#define	SIN(x)					XSIN(x)
	#define	TAN(x)					XTAN(x)
	#define	ACOS(x)					XACOS(x)
	
	#define SQRT(x)					FIXSQRT(x)

#else

	// Floating-point operations
	#define VERTTYPEMUL(a,b)			( (VERTTYPE)((a)*(b)) )
	#define VERTTYPEDIV(a,b)			( (VERTTYPE)((a)/(b)) )
	#define VERTTYPEABS(a)				( (VERTTYPE)(fabs(a)) )

	#define f2vt(x)						(x)
	#define vt2f(x)						(x)

	#define PIOVERTWO				PIOVERTWOf
	#define PI						PIf
	#define TWOPI					TWOPIf
	#define ONE						ONEf

	// If trig tables are forced ON in non-fixed-point builds then convert fixed-point trig tables results to float
	#define	COS(x)					FCOS(x)
	#define	SIN(x)					FSIN(x)
	#define	TAN(x)					FTAN(x)
	#define	ACOS(x)					FACOS(x)
	
	#define SQRT(x)					sqrtf(x)

#endif


// Structure Definitions

//
// Defines the format of a header-object as exported by the MAX
// plugin.
//
typedef struct {
	unsigned int      nNumVertex;
    unsigned int      nNumFaces;
    unsigned int      nNumStrips;
    unsigned int      nFlags;
    unsigned int      nMaterial;
    float             fCenter[3];
    float             *pVertex;
    float             *pUV;
    float             *pNormals;
    float             *pPackedVertex;
    unsigned int      *pVertexColor;
    unsigned int      *pVertexMaterial;
    unsigned short    *pFaces;
    unsigned short    *pStrips;
    unsigned short    *pStripLength;
    struct
    {
        unsigned int  nType;
        unsigned int  nNumPatches;
        unsigned int  nNumVertices;
        unsigned int  nNumSubdivisions;
        float         *pControlPoints;
        float         *pUVs;
    } Patch;
}   HeaderStruct_Mesh;


#ifdef FIXEDPOINTENABLE

//
// Defines the format of a header-object as when converted to
// fixed point.
//
	typedef struct {
		unsigned int      nNumVertex;
		unsigned int      nNumFaces;
		unsigned int      nNumStrips;
		unsigned int      nFlags;
		unsigned int      nMaterial;
		VERTTYPE          fCenter[3];
		VERTTYPE          *pVertex;
		VERTTYPE          *pUV;
		VERTTYPE          *pNormals;
		VERTTYPE          *pPackedVertex;
		unsigned int      *pVertexColor;
		unsigned int      *pVertexMaterial;
		unsigned short    *pFaces;
		unsigned short    *pStrips;
		unsigned short    *pStripLength;
		struct
		{
			unsigned int  nType;				
			unsigned int  nNumPatches;
			unsigned int  nNumVertices;
			unsigned int  nNumSubdivisions;
			float       *pControlPoints;
			float       *pUVs;
		} Patch;
	}   HeaderStruct_Fixed_Mesh;

	typedef HeaderStruct_Fixed_Mesh HeaderStruct_Mesh_Type;
#else
	typedef HeaderStruct_Mesh HeaderStruct_Mesh_Type;
#endif

//
// Converts the data exported by MAX to fixed point when used in OpenGL ES common-lit profile == fixed-point
// Expects a pointer to the object structure in the header file
// returns a directly usable geometry in fixed or float format
// 
HeaderStruct_Mesh_Type* LoadHeaderObject(const void *headerObj);

//
// Releases memory allocated by LoadHeaderObject when the geometry is no longer needed
// expects a pointer returned by LoadHeaderObject
// 
void UnloadHeaderObject(HeaderStruct_Mesh_Type* headerObj);


#endif // FIXEDPOINT_H_
