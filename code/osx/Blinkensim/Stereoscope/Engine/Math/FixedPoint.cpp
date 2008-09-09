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
#include <math.h>
#include <string.h>
#include "FixedPoint.h"

// only want to use this if the target should use fixed point math
#ifdef FIXEDPOINTENABLE

// Converts model floating point data to fixed point
HeaderStruct_Fixed_Mesh *CreateFixedObjectMesh(HeaderStruct_Mesh *mesh)
{
	HeaderStruct_Fixed_Mesh *new_mesh = new HeaderStruct_Fixed_Mesh;

	new_mesh->fCenter[0] = F2X(mesh->fCenter[0]);
	new_mesh->fCenter[1] = F2X(mesh->fCenter[1]);
	new_mesh->fCenter[2] = F2X(mesh->fCenter[2]);


	new_mesh->nNumVertex = mesh->nNumVertex;
	new_mesh->nNumFaces = mesh->nNumFaces;
	new_mesh->nNumStrips = mesh->nNumStrips;
	new_mesh->nMaterial = mesh->nMaterial;

	if(mesh->nNumVertex)
	{
		new_mesh->pVertex = new VERTTYPE[mesh->nNumVertex*3];
		for(unsigned int i = 0; i < mesh->nNumVertex*3; i++)		// each vertex is 3 floats
			new_mesh->pVertex[i] = F2X(mesh->pVertex[i]);
	}
	else
	{
		new_mesh->pVertex = 0;
		new_mesh->nNumVertex = 0;
	}

	if(mesh->pUV)
	{
		new_mesh->pUV = new VERTTYPE[mesh->nNumVertex*2];
		for(unsigned int i = 0; i < mesh->nNumVertex*2; i++)		// UVs come in pairs of floats
			new_mesh->pUV[i] = F2X(mesh->pUV[i]);
	}
	else
		new_mesh->pUV = 0;

	if(mesh->pNormals)
	{
		new_mesh->pNormals = new VERTTYPE[mesh->nNumVertex*3];
		for(unsigned int i = 0; i < mesh->nNumVertex*3; i++)		// each normal is 3 floats
			new_mesh->pNormals[i] = F2X(mesh->pNormals[i]);
	}
	else
	{
		new_mesh->pNormals = 0;
	}

	/*
	 * Format of packedVerts is
	 *		Position
	 *		Normal / Colour
	 *		UVs
	 */

#define MF_NORMALS 1
#define MF_VERTEXCOLOR 2
#define MF_UV 3

	if(mesh->pPackedVertex)
	{
		unsigned int nPackedVertSize = mesh->nNumVertex * 3 +
					(mesh->nFlags & MF_NORMALS		? mesh->nNumVertex * 3 : 0) +
					(mesh->nFlags & MF_VERTEXCOLOR	? mesh->nNumVertex * 3 : 0) +
					(mesh->nFlags & MF_UV			? mesh->nNumVertex * 2 : 0);

		new_mesh->pPackedVertex = new VERTTYPE[nPackedVertSize];
		for(unsigned int i = 0; i < nPackedVertSize; i++)
			new_mesh->pPackedVertex[i] = F2X(mesh->pPackedVertex[i]);
	}
	else
		new_mesh->pPackedVertex = 0;

	// simply copy reference to all properties which do not need conversion (indicies)

	new_mesh->pVertexColor				= mesh->pVertexColor;
	new_mesh->pVertexMaterial			= mesh->pVertexMaterial;
	new_mesh->pFaces					= mesh->pFaces;
	new_mesh->pStrips					= mesh->pStrips;
	new_mesh->pStripLength				= mesh->pStripLength;

	// we're leaving the patch stuff alone

	new_mesh->Patch.nType				= mesh->Patch.nType;
	new_mesh->Patch.nNumPatches			= mesh->Patch.nNumPatches;
	new_mesh->Patch.nNumVertices		= mesh->Patch.nNumVertices;
	new_mesh->Patch.nNumSubdivisions	= mesh->Patch.nNumSubdivisions;
	new_mesh->Patch.pControlPoints		= mesh->Patch.pControlPoints;
	new_mesh->Patch.pUVs				= mesh->Patch.pUVs;

	return new_mesh;
}

/* Release memory allocated in CreateFixedObjectMesh() */
void FreeFixedObjectMesh(HeaderStruct_Fixed_Mesh* mesh)
{

	delete[] mesh->pVertex;
	delete[] mesh->pUV;
	delete[] mesh->pNormals;
	delete[] mesh->pPackedVertex;

	delete mesh;
}

#endif


#ifdef FIXEDPOINTENABLE
//
// this follows http://www.finesse.demon.co.uk/steven/sqrt.html
// 4 cycle/bit C routine
// Wilco Dijkstra also provided the following C code which produces optimised ARM code which takes 4 cycles per bit:

// fixed-point square root
#define ITER1(N) \
    tr = root + (1 << (N)); \
    if (n >= tr << (N))   \
    {   n -= tr << (N);   \
        root |= 2 << (N); \
    }

unsigned int fixsqrt (unsigned int n)
{
    unsigned int root;
	unsigned int tr;
	root = 0;
	
    ITER1 (15);    ITER1 (14);    ITER1 (13);    ITER1 (12);
    ITER1 (11);    ITER1 (10);    ITER1 ( 9);    ITER1 ( 8);
    ITER1 ( 7);    ITER1 ( 6);    ITER1 ( 5);    ITER1 ( 4);
    ITER1 ( 3);    ITER1 ( 2);    ITER1 ( 1);    ITER1 ( 0);
	
    return root >> 1;
}
#endif

//
// Converts the data exported by MAX to fixed point when used in OpenGL ES common-lit profile == fixed-point
// Expects a pointer to the object structure in the header file
// returns a directly usable geometry in fixed or float format
// 
HeaderStruct_Mesh_Type *LoadHeaderObject(const void *headerObj)
{
#ifdef FIXEDPOINTENABLE
	return (HeaderStruct_Mesh_Type*) CreateFixedObjectMesh((HeaderStruct_Mesh *) headerObj);
#else
	HeaderStruct_Mesh_Type *new_mesh = new HeaderStruct_Mesh_Type;
	memcpy (new_mesh,headerObj,sizeof(HeaderStruct_Mesh_Type));
	return (HeaderStruct_Mesh_Type*) new_mesh;
#endif
}
//
// Releases memory allocated by LoadHeaderObject when the geometry is no longer needed
// expects a pointer returned by LoadHeaderObject
// 
void UnloadHeaderObject(HeaderStruct_Mesh_Type* headerObj)
{
#ifdef FIXEDPOINTENABLE
	FreeFixedObjectMesh(headerObj);
#else
	delete headerObj;
#endif
}
