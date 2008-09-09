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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Mathematics.h"
#include "Geometry.h"
#include "Macros.h"


/****************************************************************************
** Defines
****************************************************************************/

/****************************************************************************
** Macros
****************************************************************************/
#define MAX_VERTEX_OUT (3*nVtxNum)

#define _MIN_(X,Y) (((X)<(Y))?(X):(Y))
#define _MAX_(X,Y) (((X)>(Y))?(X):(Y))

//#define CLAMP(V,N,X) (_MIN_((X), _MAX_((V), (N))))

/****************************************************************************
** Structures
****************************************************************************/

/****************************************************************************
** Constants
****************************************************************************/

/****************************************************************************
** Local function definitions
****************************************************************************/

/*****************************************************************************
** Functions
*****************************************************************************/

/*!***************************************************************************
 @Function			DataTypeRead
 @Output			pV
 @Input				pData
 @Input				eType
 @Input				nCnt
 @Description		Read a vector
*****************************************************************************/
void DataTypeRead(
	VECTOR4f		* const pV,
	const void			* const pData,
	const EDataType	eType,
	const int			nCnt)
{
	int		i;
	float	*pOut = (float*)pV;

	pV->x = 0;
	pV->y = 0;
	pV->z = 0;
	pV->w = 1;

	switch(eType)
	{
	default:
		_ASSERT(false);
		break;

	case EPODDataFloat:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = ((float*)pData)[i];
		break;

	case EPODDataFixed16_16:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = ((int*)pData)[i] * 1.0f / (float)(1 << 16);
		break;

	case EPODDataInt:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = (float)((int*)pData)[i];
		break;

	case EPODDataByte:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = (float)((char*)pData)[i];
		break;

	case EPODDataByteNorm:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = (float)((char*)pData)[i] / (float)((1 << 7)-1);
		break;

	case EPODDataUnsignedByte:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = (float)((unsigned char*)pData)[i];
		break;

	case EPODDataShort:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = (float)((short*)pData)[i];
		break;

	case EPODDataShortNorm:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = (float)((short*)pData)[i] / (float)((1 << 15)-1);
		break;

	case EPODDataUnsignedShort:
		for(i = 0; i < nCnt; ++i)
			pOut[i] = (float)((unsigned short*)pData)[i];
		break;

	case EPODDataRGBA:
		{
			unsigned int dwVal = *(unsigned int*)pData;
			unsigned char v[4];

			v[0] = dwVal >> 24;
			v[1] = dwVal >> 16;
			v[2] = dwVal >>  8;
			v[3] = dwVal >>  0;

			for(i = 0; i < 4; ++i)
				pOut[i] = 1.0f / 255.0f * (float)v[i];
		}
		break;

	case EPODDataARGB:
	case EPODDataD3DCOLOR:
		{
			unsigned int dwVal = *(unsigned int*)pData;
			unsigned char v[4];

			v[0] = dwVal >> 16;
			v[1] = dwVal >>  8;
			v[2] = dwVal >>  0;
			v[3] = dwVal >> 24;

			for(i = 0; i < 4; ++i)
				pOut[i] = 1.0f / 255.0f * (float)v[i];
		}
		break;

	case EPODDataUBYTE4:
		{
			unsigned int dwVal = *(unsigned int*)pData;
			unsigned char v[4];

			v[0] = dwVal >>  0;
			v[1] = dwVal >>  8;
			v[2] = dwVal >> 16;
			v[3] = dwVal >> 24;

			for(i = 0; i < 4; ++i)
				pOut[i] = v[i];
		}
		break;

	case EPODDataDEC3N:
		{
			int dwVal = *(int*)pData;
			int v[4];

			v[0] = (dwVal << 22) >> 22;
			v[1] = (dwVal << 12) >> 22;
			v[2] = (dwVal <<  2) >> 22;
			v[3] = 0;

			for(i = 0; i < 3; ++i)
				pOut[i] = (float)v[i] * (1.0f / 511.0f);
		}
		break;
	}
}

void DataTypeRead(
	unsigned int		* const pV,
	const void			* const pData,
	const EDataType	eType)
{
	switch(eType)
	{
	default:
		_ASSERT(false);
		break;

	case EPODDataUnsignedShort:
		*pV = *(unsigned short*)pData;
		break;
	}
}

/*!***************************************************************************
 @Function			DataTypeWrite
 @Output			pOut
 @Input				eType
 @Input				nCnt
 @Input				pV
 @Description		Write a vector
*****************************************************************************/
void DataTypeWrite(
	void				* const pOut,
	const EDataType	eType,
	const int			nCnt,
	const VECTOR4f	* const pV)
{
	int		i;
	float	*pData = (float*)pV;

	switch(eType)
	{
	default:
		_ASSERT(false);
		break;

	case EPODDataDEC3N:
		{
			int v[3];

			for(i = 0; i < nCnt; ++i)
			{
				v[i] = (int)(pData[i] * 511.0f);
				v[i] = CLAMP(v[i], -511, 511);
				v[i] &= 0x000003ff;
			}

			for(; i < 3; ++i)
			{
				v[i] = 0;
			}

			*(unsigned int*)pOut = (v[0] << 0) | (v[1] << 10) | (v[2] << 20);
		}
		break;

	case EPODDataARGB:
	case EPODDataD3DCOLOR:
		{
			unsigned char v[4];

			for(i = 0; i < nCnt; ++i)
				v[i] = (unsigned char)CLAMP(pData[i] * 255.0f, 0.0f, 255.0f);

			for(; i < 4; ++i)
				v[i] = 0;

			*(unsigned int*)pOut = (v[3] << 24) | (v[0] << 16) | (v[1] << 8) | v[2];
		}
		break;

	case EPODDataRGBA:
		{
			unsigned char v[4];

			for(i = 0; i < nCnt; ++i)
				v[i] = (unsigned char)CLAMP(pData[i] * 255.0f, 0.0f, 255.0f);

			for(; i < 4; ++i)
				v[i] = 0;

			*(unsigned int*)pOut = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
		}
		break;

	case EPODDataUBYTE4:
		{
			unsigned char v[4];

			for(i = 0; i < nCnt; ++i)
				v[i] = (unsigned char)CLAMP(pData[i], 0.0f, 255.0f);

			for(; i < 4; ++i)
				v[i] = 0;

			*(unsigned int*)pOut = (v[3] << 24) | (v[2] << 16) | (v[1] << 8) | v[0];
		}
		break;

	case EPODDataFloat:
		for(i = 0; i < nCnt; ++i)
			((float*)pOut)[i] = pData[i];
		break;

	case EPODDataFixed16_16:
		for(i = 0; i < nCnt; ++i)
			((int*)pOut)[i] = (int)(pData[i] * (float)(1 << 16));
		break;

	case EPODDataInt:
		for(i = 0; i < nCnt; ++i)
			((int*)pOut)[i] = (int)pData[i];
		break;

	case EPODDataByte:
		for(i = 0; i < nCnt; ++i)
			((char*)pOut)[i] = (char)pData[i];
		break;

	case EPODDataByteNorm:
		for(i = 0; i < nCnt; ++i)
			((char*)pOut)[i] = (char)(pData[i] * (float)((1 << 7)-1));
		break;

	case EPODDataUnsignedByte:
		for(i = 0; i < nCnt; ++i)
			((unsigned char*)pOut)[i] = (unsigned char)pData[i];
		break;

	case EPODDataShort:
		for(i = 0; i < nCnt; ++i)
			((short*)pOut)[i] = (short)pData[i];
		break;

	case EPODDataShortNorm:
		for(i = 0; i < nCnt; ++i)
			((short*)pOut)[i] = (short)(pData[i] * (float)((1 << 15)-1));
		break;

	case EPODDataUnsignedShort:
		for(i = 0; i < nCnt; ++i)
			((unsigned short*)pOut)[i] = (unsigned short)pData[i];
		break;
	}
}

void DataTypeWrite(
	void				* const pOut,
	const EDataType	eType,
	const unsigned int	V)
{
	switch(eType)
	{
	default:
		_ASSERT(false);
		break;

	case EPODDataUnsignedShort:
		*(unsigned short*)pOut = V;
	}
}

/*!***************************************************************************
 @Function			VertexTangentBinormal
 @Output			pvTan
 @Output			pvBin
 @Input				pvNor
 @Input				pfPosA
 @Input				pfPosB
 @Input				pfPosC
 @Input				pfTexA
 @Input				pfTexB
 @Input				pfTexC
 @Description		Calculates the tangent and binormal vectors for
					vertex 'A' of the triangle defined by the 3 supplied
					3D position coordinates (pfPosX) and 2D texture
					coordinates (pfTexX).
*****************************************************************************/
void VertexTangentBinormal(
	VECTOR3f		* const pvTan,
	VECTOR3f		* const pvBin,
	const VECTOR3f	* const pvNor,
	const float			* const pfPosA,
	const float			* const pfPosB,
	const float			* const pfPosC,
	const float			* const pfTexA,
	const float			* const pfTexB,
	const float			* const pfTexC)
{
	VECTOR3f BaseVector1, BaseVector2, AlignedVector;

	if(MatrixVec3DotProductF(*pvNor, *pvNor) == 0)
	{
		pvTan->x = 0;
		pvTan->y = 0;
		pvTan->z = 0;
		pvBin->x = 0;
		pvBin->y = 0;
		pvBin->z = 0;
		return;
	}

	/* BaseVectors are A-B and A-C. */
	BaseVector1.x = pfPosB[0] - pfPosA[0];
	BaseVector1.y = pfPosB[1] - pfPosA[1];
	BaseVector1.z = pfPosB[2] - pfPosA[2];

	BaseVector2.x = pfPosC[0] - pfPosA[0];
	BaseVector2.y = pfPosC[1] - pfPosA[1];
	BaseVector2.z = pfPosC[2] - pfPosA[2];

	if (pfTexB[0]==pfTexA[0] && pfTexC[0]==pfTexA[0])
	{
		// Degenerate tri
//		_ASSERT(0);
		pvTan->x = 0;
		pvTan->y = 0;
		pvTan->z = 0;
		pvBin->x = 0;
		pvBin->y = 0;
		pvBin->z = 0;
	}
	else
	{
		/* Calc the vector that follows the V direction (it is not the tangent vector)*/
		if(pfTexB[0]==pfTexA[0]) {
			AlignedVector = BaseVector1;
			if((pfTexB[1] - pfTexA[1]) < 0) {
				AlignedVector.x = -AlignedVector.x;
				AlignedVector.y = -AlignedVector.y;
				AlignedVector.z = -AlignedVector.z;
			}
		} else if(pfTexC[0]==pfTexA[0]) {
			AlignedVector = BaseVector2;
			if((pfTexC[1] - pfTexA[1]) < 0) {
				AlignedVector.x = -AlignedVector.x;
				AlignedVector.y = -AlignedVector.y;
				AlignedVector.z = -AlignedVector.z;
			}
		} else {
			float fFac;

			fFac = -(pfTexB[0] - pfTexA[0]) / (pfTexC[0] - pfTexA[0]);

			/* This is the vector that follows the V direction (it is not the tangent vector)*/
			AlignedVector.x = BaseVector1.x + BaseVector2.x * fFac;
			AlignedVector.y = BaseVector1.y + BaseVector2.y * fFac;
			AlignedVector.z = BaseVector1.z + BaseVector2.z * fFac;

			if(((pfTexB[1] - pfTexA[1]) + (pfTexC[1] - pfTexA[1]) * fFac) < 0) {
				AlignedVector.x = -AlignedVector.x;
				AlignedVector.y = -AlignedVector.y;
				AlignedVector.z = -AlignedVector.z;
			}
		}

		MatrixVec3NormalizeF(AlignedVector, AlignedVector);

		/* The Tangent vector is perpendicular to the plane defined by vAlignedVector and the Normal. */
		MatrixVec3CrossProductF(*pvTan, *pvNor, AlignedVector);

		/* The Binormal vector is the vector perpendicular to the Normal and Tangent (and
		that follows the vAlignedVector direction) */
		MatrixVec3CrossProductF(*pvBin, *pvTan, *pvNor);

		_ASSERT(MatrixVec3DotProductF(*pvBin, AlignedVector) > 0.0f);

		// Worry about wrapping; this is esentially a 2D cross product on texture coords
		if((pfTexC[0]-pfTexA[0])*(pfTexB[1]-pfTexA[1]) < (pfTexC[1]-pfTexA[1])*(pfTexB[0]-pfTexA[0])) {
			pvTan->x = -pvTan->x;
			pvTan->y = -pvTan->y;
			pvTan->z = -pvTan->z;
		}

		/* Normalize results */
		MatrixVec3NormalizeF(*pvTan, *pvTan);
		MatrixVec3NormalizeF(*pvBin, *pvBin);

		_ASSERT(MatrixVec3DotProductF(*pvNor, *pvNor) > 0.9f);
		_ASSERT(MatrixVec3DotProductF(*pvTan, *pvTan) > 0.9f);
		_ASSERT(MatrixVec3DotProductF(*pvBin, *pvBin) > 0.9f);
	}
}

/*!***************************************************************************
 @Function			VertexGenerateTangentSpace
 @Output			pnVtxNumOut			Output vertex count
 @Output			pVtxOut				Output vertices (program must free() this)
 @Modified			pwIdx				input AND output; index array for triangle list
 @Input				nVtxNum				Input vertex count
 @Input				pVtx				Input vertices
 @Input				nStride				Size of a vertex (in bytes)
 @Input				nOffsetPos			Offset in bytes to the vertex position
 @Input				eTypePos			Data type of the position
 @Input				nOffsetNor			Offset in bytes to the vertex normal
 @Input				eTypeNor			Data type of the normal
 @Input				nOffsetTex			Offset in bytes to the vertex texture coordinate to use
 @Input				eTypeTex			Data type of the texture coordinate
 @Input				nOffsetTan			Offset in bytes to the vertex tangent
 @Input				eTypeTan			Data type of the tangent
 @Input				nOffsetBin			Offset in bytes to the vertex binormal
 @Input				eTypeBin			Data type of the binormal
 @Input				nTriNum				Number of triangles
 @Input				fSplitDifference	Split a vertex if the DP3 of tangents/binormals are below this (range -1..1)
 @Return			false if there was a problem.
 @Description		Calculates the tangent space for all supplied vertices.
					Writes tangent and binormal vectors to the output
					vertices, copies all other elements from input vertices.
					Will split vertices if necessary - i.e. if two triangles
					sharing a vertex want to assign it different
					tangent-space matrices. The decision whether to split
					uses fSplitDifference - of the DP3 of two desired
					tangents or two desired binormals is higher than this,
					the vertex will be split.
*****************************************************************************/
bool VertexGenerateTangentSpace(
	int				* const pnVtxNumOut,
	char			** const pVtxOut,
	unsigned short	* const pwIdx,
	const int		nVtxNum,
	const char		* const pVtx,
	const int		nStride,
	const int		nOffsetPos,
	EDataType	eTypePos,
	const int		nOffsetNor,
	EDataType	eTypeNor,
	const int		nOffsetTex,
	EDataType	eTypeTex,
	const int		nOffsetTan,
	EDataType	eTypeTan,
	const int		nOffsetBin,
	EDataType	eTypeBin,
	const int		nTriNum,
	const float		fSplitDifference)
{
	const int cnMaxSharedVtx = 32;
	struct SVtxData
	{
		int				n;							// Number of items in following arrays, AKA number of tris using this vtx
		VECTOR3f	pvTan[cnMaxSharedVtx];		// Tangent (one per triangle referencing this vtx)
		VECTOR3f	pvBin[cnMaxSharedVtx];		// Binormal (one per triangle referencing this vtx)
		int				pnTri[cnMaxSharedVtx];		// Triangle index (one per triangle referencing this vtx)
	};
	SVtxData		*psVtxData;		// Array of desired tangent spaces per vertex
	SVtxData		*psTSpass;		// Array of *different* tangent spaces desired for current vertex
	int				nTSpassLen;
	SVtxData		*psVtx, *psCmp;
	int				nVert, nCurr, i, j;	// Loop counters
	int				nIdx0, nIdx1, nIdx2;
	float			pfPos0[4], pfPos1[4], pfPos2[4];
	float			pfTex0[4], pfTex1[4], pfTex2[4];
	float			pfNor0[4], pfNor1[4], pfNor2[4];
	unsigned short	*pwIdxNew;		// New index array, this will be copied over the input array

	// Initialise the outputs
	*pnVtxNumOut	= 0;
	*pVtxOut		= (char*)malloc(MAX_VERTEX_OUT * nStride);
	if(!*pVtxOut)
	{
		return false;
	}

	// Allocate some work space
	pwIdxNew		= (unsigned short*)malloc(nTriNum * 3 * sizeof(*pwIdxNew));
	_ASSERT(pwIdxNew);
	psVtxData		= (SVtxData*)calloc(nVtxNum, sizeof(*psVtxData));
	_ASSERT(psVtxData);
	psTSpass		= (SVtxData*)calloc(cnMaxSharedVtx, sizeof(*psTSpass));
	_ASSERT(psTSpass);
	if(!pwIdxNew || !psVtxData || !psTSpass)
	{
		return false;
	}

	for(nCurr = 0; nCurr < nTriNum; ++nCurr) {
		nIdx0 = pwIdx[3*nCurr+0];
		nIdx1 = pwIdx[3*nCurr+1];
		nIdx2 = pwIdx[3*nCurr+2];

		_ASSERT(nIdx0 < nVtxNum);
		_ASSERT(nIdx1 < nVtxNum);
		_ASSERT(nIdx2 < nVtxNum);

		if(nIdx0 == nIdx1 || nIdx1 == nIdx2 || nIdx0 == nIdx2) {
			_RPT0(_CRT_WARN,"GenerateTangentSpace(): Degenerate triangle found.\n");
			return false;
		}

		if(
			psVtxData[nIdx0].n >= cnMaxSharedVtx ||
			psVtxData[nIdx1].n >= cnMaxSharedVtx ||
			psVtxData[nIdx2].n >= cnMaxSharedVtx)
		{
			_RPT0(_CRT_WARN,"GenerateTangentSpace(): Too many tris sharing a vtx.\n");
			return false;
		}

		DataTypeRead((VECTOR4f*)pfPos0, (char*)&pVtx[nIdx0 * nStride] + nOffsetPos, eTypePos, 3);
		DataTypeRead((VECTOR4f*)pfPos1, (char*)&pVtx[nIdx1 * nStride] + nOffsetPos, eTypePos, 3);
		DataTypeRead((VECTOR4f*)pfPos2, (char*)&pVtx[nIdx2 * nStride] + nOffsetPos, eTypePos, 3);

		DataTypeRead((VECTOR4f*)pfNor0, (char*)&pVtx[nIdx0 * nStride] + nOffsetNor, eTypeNor, 3);
		DataTypeRead((VECTOR4f*)pfNor1, (char*)&pVtx[nIdx1 * nStride] + nOffsetNor, eTypeNor, 3);
		DataTypeRead((VECTOR4f*)pfNor2, (char*)&pVtx[nIdx2 * nStride] + nOffsetNor, eTypeNor, 3);

		DataTypeRead((VECTOR4f*)pfTex0, (char*)&pVtx[nIdx0 * nStride] + nOffsetTex, eTypeTex, 3);
		DataTypeRead((VECTOR4f*)pfTex1, (char*)&pVtx[nIdx1 * nStride] + nOffsetTex, eTypeTex, 3);
		DataTypeRead((VECTOR4f*)pfTex2, (char*)&pVtx[nIdx2 * nStride] + nOffsetTex, eTypeTex, 3);

		VertexTangentBinormal(
			&psVtxData[nIdx0].pvTan[psVtxData[nIdx0].n],
			&psVtxData[nIdx0].pvBin[psVtxData[nIdx0].n],
			(VECTOR3f*)pfNor0,
			pfPos0, pfPos1, pfPos2,
			pfTex0, pfTex1, pfTex2);

		VertexTangentBinormal(
			&psVtxData[nIdx1].pvTan[psVtxData[nIdx1].n],
			&psVtxData[nIdx1].pvBin[psVtxData[nIdx1].n],
			(VECTOR3f*)pfNor1,
			pfPos1, pfPos2, pfPos0,
			pfTex1, pfTex2, pfTex0);

		VertexTangentBinormal(
			&psVtxData[nIdx2].pvTan[psVtxData[nIdx2].n],
			&psVtxData[nIdx2].pvBin[psVtxData[nIdx2].n],
			(VECTOR3f*)pfNor2,
			pfPos2, pfPos0, pfPos1,
			pfTex2, pfTex0, pfTex1);

		psVtxData[nIdx0].pnTri[psVtxData[nIdx0].n] = nCurr;
		psVtxData[nIdx1].pnTri[psVtxData[nIdx1].n] = nCurr;
		psVtxData[nIdx2].pnTri[psVtxData[nIdx2].n] = nCurr;

		++psVtxData[nIdx0].n;
		++psVtxData[nIdx1].n;
		++psVtxData[nIdx2].n;
	}

	// Now let's go through the vertices calculating avg tangent-spaces; create new vertices if necessary
	for(nVert = 0; nVert < nVtxNum; ++nVert) {
		psVtx = &psVtxData[nVert];

		// Start out with no output vertices required for this input vertex
		nTSpassLen = 0;

		// Run through each desired tangent space for this vertex
		for(nCurr = 0; nCurr < psVtx->n; ++nCurr) {
			// Run through the possible vertices we can share with to see if we match
			for(i = 0; i < nTSpassLen; ++i) {
				psCmp = &psTSpass[i];

				// Check all the shared vertices which match
				for(j = 0; j < psCmp->n; ++j) {
					if(MatrixVec3DotProductF(psVtx->pvTan[nCurr], psCmp->pvTan[j]) < fSplitDifference)
						break;
					if(MatrixVec3DotProductF(psVtx->pvBin[nCurr], psCmp->pvBin[j]) < fSplitDifference)
						break;
				}

				// Did all the existing vertices match?
				if(j == psCmp->n) {
					// Yes, so add to list
					_ASSERT(psCmp->n < cnMaxSharedVtx);
					psCmp->pvTan[psCmp->n] = psVtx->pvTan[nCurr];
					psCmp->pvBin[psCmp->n] = psVtx->pvBin[nCurr];
					psCmp->pnTri[psCmp->n] = psVtx->pnTri[nCurr];
					++psCmp->n;
					break;
				}
			}

			if(i == nTSpassLen) {
				// We never found another matching matrix, so let's add this as a different one
				_ASSERT(nTSpassLen < cnMaxSharedVtx);
				psTSpass[nTSpassLen].pvTan[0] = psVtx->pvTan[nCurr];
				psTSpass[nTSpassLen].pvBin[0] = psVtx->pvBin[nCurr];
				psTSpass[nTSpassLen].pnTri[0] = psVtx->pnTri[nCurr];
				psTSpass[nTSpassLen].n = 1;
				++nTSpassLen;
			}
		}

		// OK, now we have 'nTSpassLen' different desired matrices, so we need to add that many to output
		_ASSERT(nTSpassLen >= 1);
		for(nCurr = 0; nCurr < nTSpassLen; ++nCurr) {
			psVtx = &psTSpass[nCurr];

			memset(&pfPos0, 0, sizeof(pfPos0));
			memset(&pfPos1, 0, sizeof(pfPos1));

			for(i = 0; i < psVtx->n; ++i) {
				// Sum the tangent & binormals, so we can average them
				pfPos0[0] += psVtx->pvTan[i].x;
				pfPos0[1] += psVtx->pvTan[i].y;
				pfPos0[2] += psVtx->pvTan[i].z;

				pfPos1[0] += psVtx->pvBin[i].x;
				pfPos1[1] += psVtx->pvBin[i].y;
				pfPos1[2] += psVtx->pvBin[i].z;

				// Update triangle indices to use this vtx
				if(pwIdx[3 * psVtx->pnTri[i] + 0] == nVert) {
					pwIdxNew[3 * psVtx->pnTri[i] + 0] = *pnVtxNumOut;

				} else if(pwIdx[3 * psVtx->pnTri[i] + 1] == nVert) {
					pwIdxNew[3 * psVtx->pnTri[i] + 1] = *pnVtxNumOut;

				} else if(pwIdx[3 * psVtx->pnTri[i] + 2] == nVert) {
					pwIdxNew[3 * psVtx->pnTri[i] + 2] = *pnVtxNumOut;

				} else {
					_ASSERT(0);
				}
			}

			MatrixVec3NormalizeF(*(VECTOR3f*)pfPos0, *(VECTOR3f*)pfPos0);
			MatrixVec3NormalizeF(*(VECTOR3f*)pfPos1, *(VECTOR3f*)pfPos1);

			if(*pnVtxNumOut >= MAX_VERTEX_OUT) {
				_RPT0(_CRT_WARN,"VertexGenerateTangentSpace() ran out of working space! (Too many split vertices)\n");
				return false;
			}
			memcpy(&(*pVtxOut)[(*pnVtxNumOut) * nStride], &pVtx[nVert*nStride], nStride);
			DataTypeWrite((char*)&(*pVtxOut)[(*pnVtxNumOut) * nStride] + nOffsetTan, eTypeTan, 3, (VECTOR4f*)pfPos0);
			DataTypeWrite((char*)&(*pVtxOut)[(*pnVtxNumOut) * nStride] + nOffsetBin, eTypeBin, 3, (VECTOR4f*)pfPos1);

			++*pnVtxNumOut;
		}
	}

	free(psTSpass);
	free(psVtxData);

	*pVtxOut = (char*)realloc(*pVtxOut, *pnVtxNumOut * nStride);
	_ASSERT(*pVtxOut);

	memcpy(pwIdx, pwIdxNew, nTriNum * 3 * sizeof(*pwIdxNew));
	free(pwIdxNew);

	_RPT3(_CRT_WARN, "GenerateTangentSpace(): %d tris, %d vtx in, %d vtx out\n", nTriNum, nVtxNum, *pnVtxNumOut);
	_ASSERT(*pnVtxNumOut >= nVtxNum);

	return true;
}

/****************************************************************************
** Local functions
****************************************************************************/

/*****************************************************************************
 End of file (Vertex.cpp)
*****************************************************************************/
