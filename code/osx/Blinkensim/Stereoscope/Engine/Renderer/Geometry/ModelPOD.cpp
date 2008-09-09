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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Resource.h"

#include "ModelPOD.h"

/****************************************************************************
** Defines
****************************************************************************/
#define MODELPOD_TAG_MASK			(0x80000000)
#define MODELPOD_TAG_START			(0x00000000)
#define MODELPOD_TAG_END			(0x80000000)

#define CFAH		(1024)

//DEFINE_HEAP(CPODScene, "Model POD");


/****************************************************************************
** Enumerations
****************************************************************************/
enum EPODFileName
{
	ePODFileVersion				= 1000,
	ePODFileScene,
	ePODFileExpOpt,

	ePODFileColourBackground	= 2000,
	ePODFileColourAmbient,
	ePODFileNumCamera,
	ePODFileNumLight,
	ePODFileNumMesh,
	ePODFileNumNode,
	ePODFileNumMeshNode,
	ePODFileNumTexture,
	ePODFileNumMaterial,
	ePODFileNumFrame,
	ePODFileCamera,		// Will come multiple times
	ePODFileLight,		// Will come multiple times
	ePODFileMesh,		// Will come multiple times
	ePODFileNode,		// Will come multiple times
	ePODFileTexture,	// Will come multiple times
	ePODFileMaterial,	// Will come multiple times
	ePODFileFlags,

	ePODFileMatName				= 3000,
	ePODFileMatIdxTexDiffuse,
	ePODFileMatOpacity,
	ePODFileMatAmbient,
	ePODFileMatDiffuse,
	ePODFileMatSpecular,
	ePODFileMatShininess,
	ePODFileMatEffectFile,
	ePODFileMatEffectName,

	ePODFileTexName				= 4000,

	ePODFileNodeIdx				= 5000,
	ePODFileNodeName,
	ePODFileNodeIdxMat,
	ePODFileNodeIdxParent,
	ePODFileNodePos,
	ePODFileNodeRot,
	ePODFileNodeScale,
	ePODFileNodeAnimPos,
	ePODFileNodeAnimRot,
	ePODFileNodeAnimScale,

	ePODFileMeshNumVtx			= 6000,
	ePODFileMeshNumFaces,
	ePODFileMeshNumUVW,
	ePODFileMeshFaces,
	ePODFileMeshStripLength,
	ePODFileMeshNumStrips,
	ePODFileMeshVtx,
	ePODFileMeshNor,
	ePODFileMeshTan,
	ePODFileMeshBin,
	ePODFileMeshUVW,			// Will come multiple times
	ePODFileMeshVtxCol,
	ePODFileMeshBoneIdx,
	ePODFileMeshBoneWeight,
	ePODFileMeshInterleaved,
	ePODFileMeshBoneBatches,
	ePODFileMeshBoneBatchBoneCnts,
	ePODFileMeshBoneBatchOffsets,
	ePODFileMeshBoneBatchBoneMax,
	ePODFileMeshBoneBatchCnt,

	ePODFileLightIdxTgt			= 7000,
	ePODFileLightColour,
	ePODFileLightType,

	ePODFileCamIdxTgt			= 8000,
	ePODFileCamFOV,
	ePODFileCamFar,
	ePODFileCamNear,
	ePODFileCamAnimFOV,

	ePODFileDataType			= 9000,
	ePODFileN,
	ePODFileStride,
	ePODFileData
};

/****************************************************************************
** Structures
****************************************************************************/
struct SPODImpl
{
	VERTTYPE	fFrame;		/*!< Frame number */
	VERTTYPE	fBlend;		/*!< Frame blend	(AKA fractional part of animation frame number) */
	int			nFrame;		/*!< Frame number (AKA integer part of animation frame number) */

	VERTTYPE	*pfCache;		/*!< Cache indicating the frames at which the matrix cache was filled */
	MATRIX	*pWmCache;		/*!< Cache of world matrices */
	MATRIX	*pWmZeroCache;	/*!< Pre-calculated frame 0 matrices */

	bool		bFromMemory;	/*!< Was the mesh data loaded from memory? */

#ifdef _DEBUG
	__int64	nWmTotal, nWmCacheHit, nWmZeroCacheHit;
	float	fHitPerc, fHitPercZero;
#endif
};


/****************************************************************************
** Local code: File writing
****************************************************************************/

/*!***************************************************************************
 @Function			WriteFileSafe
 @Input				pFile
 @Input				lpBuffer
 @Input				nNumberOfBytesToWrite
 @Return			true if successful
 @Description		Writes data to a file, checking return codes.
*****************************************************************************/
static bool WriteFileSafe(FILE *pFile, const void * const lpBuffer, const unsigned int nNumberOfBytesToWrite)
{
	if(nNumberOfBytesToWrite)
	{
		size_t count = fwrite(lpBuffer, nNumberOfBytesToWrite, 1, pFile);
		return count == 1;
	}
	return true;
}

/*!***************************************************************************
 @Function			WriteMarker
 @Input				pFile
 @Input				nName
 @Input				bEnd
 @Input				nLen
 Return				true if successful
 @Description		Write a marker to a POD file. If bEnd if false, it's a
					beginning marker, otherwise it's an end marker.
*****************************************************************************/
static bool WriteMarker(
	FILE				* const pFile,
	const unsigned int	nName,
	const bool			bEnd,
	const unsigned int	nLen = 0)
{
	unsigned int pnData[2];

	_ASSERT((nName & ~MODELPOD_TAG_MASK) == nName);
	pnData[0] = nName | (bEnd ? MODELPOD_TAG_END : MODELPOD_TAG_START);
	pnData[1] = nLen;

	return WriteFileSafe(pFile, pnData, sizeof(pnData));
}

/*!***************************************************************************
 @Function			WriteData
 @Input				pFile
 @Input				nName
 @Input				pData
 @Input				nLen
 @Return			true if successful
 @Description		Write nLen bytes of data from pData, bracketed by an nName
					begin/end markers.
*****************************************************************************/
static bool WriteData(
	FILE				* const pFile,
	const unsigned int	nName,
	const void			* const pData,
	const unsigned int	nLen)
{
	if(pData)
	{
		_ASSERT(nLen);
		if(!WriteMarker(pFile, nName, false, nLen)) return false;
		if(!WriteFileSafe(pFile, pData, nLen)) return false;
		if(!WriteMarker(pFile, nName, true, 0)) return false;
	}
	return true;
}

/*!***************************************************************************
 @Function			WriteData
 @Input				pFile
 @Input				nName
 @Input				n
 @Return			true if successful
 @Description		Write the value n, bracketed by an nName begin/end markers.
*****************************************************************************/
template <typename T>
static bool WriteData(
	FILE				* const pFile,
	const unsigned int	nName,
	const T				&n)
{
	return WriteData(pFile, nName, (void*)&n, (unsigned int)sizeof(T));
}
static bool WriteCPODData(
	FILE				* const pFile,
	const unsigned int	nName,
	const CPODData		&n,
	const unsigned int	nEntries,
	const bool			bValidData)
{
	if(!WriteMarker(pFile, nName, false)) return false;
	if(!WriteData(pFile, ePODFileDataType, n.eType)) return false;
	if(!WriteData(pFile, ePODFileN, n.n)) return false;
	if(!WriteData(pFile, ePODFileStride, n.nStride)) return false;
	if(bValidData)
	{
		if(!WriteData(pFile, ePODFileData, n.pData, nEntries * n.nStride)) return false;
	}
	else
	{
		if(!WriteData(pFile, ePODFileData, n.pData)) return false;
	}
	if(!WriteMarker(pFile, nName, true)) return false;
	return true;
}

/*!***************************************************************************
 @Function			WritePOD
 @Output			The file referenced by pFile
 @Input				s
 @Input				pszExpOpt
 @Return			true if successful
 @Description		Write a POD file
*****************************************************************************/
static bool WritePOD(
	FILE			* const pFile,
	const char		* const pszExpOpt,
	const SPODScene	&s)
{
	unsigned int i, j;

	// Save: file version
	{
		char *pszVersion = MODELPOD_VERSION;

		if(!WriteData(pFile, ePODFileVersion, pszVersion, (unsigned int)strlen(pszVersion) + 1)) return false;
	}

	// Save: exporter options
	if(pszExpOpt && *pszExpOpt)
	{
		if(!WriteData(pFile, ePODFileExpOpt, pszExpOpt, (unsigned int)strlen(pszExpOpt) + 1)) return false;
	}

	// Save: scene descriptor
	if(!WriteMarker(pFile, ePODFileScene, false)) return false;

	{
		if(!WriteData(pFile, ePODFileColourBackground,	s.pfColourBackground)) return false;
		if(!WriteData(pFile, ePODFileColourAmbient,		s.pfColourAmbient)) return false;
		if(!WriteData(pFile, ePODFileNumCamera,			s.nNumCamera)) return false;
		if(!WriteData(pFile, ePODFileNumLight,			s.nNumLight)) return false;
		if(!WriteData(pFile, ePODFileNumMesh,			s.nNumMesh)) return false;
		if(!WriteData(pFile, ePODFileNumNode,			s.nNumNode)) return false;
		if(!WriteData(pFile, ePODFileNumMeshNode,		s.nNumMeshNode)) return false;
		if(!WriteData(pFile, ePODFileNumTexture,		s.nNumTexture)) return false;
		if(!WriteData(pFile, ePODFileNumMaterial,		s.nNumMaterial)) return false;
		if(!WriteData(pFile, ePODFileNumFrame,			s.nNumFrame)) return false;
		if(!WriteData(pFile, ePODFileFlags,				s.nFlags)) return false;

		// Save: cameras
		for(i = 0; i < s.nNumCamera; ++i) {
			if(!WriteMarker(pFile, ePODFileCamera, false)) return false;
			if(!WriteData(pFile, ePODFileCamIdxTgt,		s.pCamera[i].nIdxTarget)) return false;
			if(!WriteData(pFile, ePODFileCamFOV,		s.pCamera[i].fFOV)) return false;
			if(!WriteData(pFile, ePODFileCamFar,		s.pCamera[i].fFar)) return false;
			if(!WriteData(pFile, ePODFileCamNear,		s.pCamera[i].fNear)) return false;
			if(!WriteData(pFile, ePODFileCamAnimFOV,	s.pCamera[i].pfAnimFOV, s.nNumFrame * sizeof(*s.pCamera[i].pfAnimFOV))) return false;
			if(!WriteMarker(pFile, ePODFileCamera, true)) return false;
		}

		// Save: lights
		for(i = 0; i < s.nNumLight; ++i) {
			if(!WriteMarker(pFile, ePODFileLight, false)) return false;
			if(!WriteData(pFile, ePODFileLightIdxTgt,	s.pLight[i].nIdxTarget)) return false;
			if(!WriteData(pFile, ePODFileLightColour,	s.pLight[i].pfColour)) return false;
			if(!WriteData(pFile, ePODFileLightType,		s.pLight[i].eType)) return false;
			if(!WriteMarker(pFile, ePODFileLight, true)) return false;
		}

		// Save: materials
		for(i = 0; i < s.nNumMaterial; ++i) {
			if(!WriteMarker(pFile, ePODFileMaterial, false)) return false;
			if(!WriteData(pFile, ePODFileMatName,			s.pMaterial[i].pszName, (unsigned int)strlen(s.pMaterial[i].pszName)+1)) return false;
			if(!WriteData(pFile, ePODFileMatIdxTexDiffuse,	s.pMaterial[i].nIdxTexDiffuse)) return false;
			if(!WriteData(pFile, ePODFileMatOpacity,		s.pMaterial[i].fMatOpacity)) return false;
			if(!WriteData(pFile, ePODFileMatAmbient,		s.pMaterial[i].pfMatAmbient)) return false;
			if(!WriteData(pFile, ePODFileMatDiffuse,		s.pMaterial[i].pfMatDiffuse)) return false;
			if(!WriteData(pFile, ePODFileMatSpecular,		s.pMaterial[i].pfMatSpecular)) return false;
			if(!WriteData(pFile, ePODFileMatShininess,		s.pMaterial[i].fMatShininess)) return false;
			if(!WriteData(pFile, ePODFileMatEffectFile,		s.pMaterial[i].pszEffectFile, s.pMaterial[i].pszEffectFile ? ((unsigned int)strlen(s.pMaterial[i].pszEffectFile)+1) : 0)) return false;
			if(!WriteData(pFile, ePODFileMatEffectName,		s.pMaterial[i].pszEffectName, s.pMaterial[i].pszEffectName ? ((unsigned int)strlen(s.pMaterial[i].pszEffectName)+1) : 0)) return false;
			if(!WriteMarker(pFile, ePODFileMaterial, true)) return false;
		}

		// Save: meshes
		for(i = 0; i < s.nNumMesh; ++i) {
			if(!WriteMarker(pFile, ePODFileMesh, false)) return false;

			if(!WriteData(pFile, ePODFileMeshNumVtx,			s.pMesh[i].nNumVertex)) return false;
			if(!WriteData(pFile, ePODFileMeshNumFaces,			s.pMesh[i].nNumFaces)) return false;
			if(!WriteData(pFile, ePODFileMeshNumUVW,			s.pMesh[i].nNumUVW)) return false;
			if(!WriteData(pFile, ePODFileMeshStripLength,		s.pMesh[i].pnStripLength, s.pMesh[i].nNumStrips * sizeof(*s.pMesh[i].pnStripLength))) return false;
			if(!WriteData(pFile, ePODFileMeshNumStrips,			s.pMesh[i].nNumStrips)) return false;
			if(!WriteData(pFile, ePODFileMeshInterleaved,		s.pMesh[i].pInterleaved, s.pMesh[i].nNumVertex * s.pMesh[i].sVertex.nStride)) return false;
			if(!WriteData(pFile, ePODFileMeshBoneBatchBoneMax,	s.pMesh[i].sBoneBatches.nBatchBoneMax)) return false;
			if(!WriteData(pFile, ePODFileMeshBoneBatchCnt,		s.pMesh[i].sBoneBatches.nBatchCnt)) return false;
			if(!WriteData(pFile, ePODFileMeshBoneBatches,		s.pMesh[i].sBoneBatches.pnBatches, s.pMesh[i].sBoneBatches.nBatchBoneMax * s.pMesh[i].sBoneBatches.nBatchCnt * sizeof(*s.pMesh[i].sBoneBatches.pnBatches))) return false;
			if(!WriteData(pFile, ePODFileMeshBoneBatchBoneCnts,	s.pMesh[i].sBoneBatches.pnBatchBoneCnt, s.pMesh[i].sBoneBatches.nBatchCnt * sizeof(*s.pMesh[i].sBoneBatches.pnBatchBoneCnt))) return false;
			if(!WriteData(pFile, ePODFileMeshBoneBatchOffsets,	s.pMesh[i].sBoneBatches.pnBatchOffset, s.pMesh[i].sBoneBatches.nBatchCnt * sizeof(*s.pMesh[i].sBoneBatches.pnBatchOffset))) return false;

			if(!WriteCPODData(pFile, ePODFileMeshFaces,			s.pMesh[i].sFaces,		ModelPODCountIndices(s.pMesh[i]), true)) return false;
			if(!WriteCPODData(pFile, ePODFileMeshVtx,			s.pMesh[i].sVertex,		s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;
			if(!WriteCPODData(pFile, ePODFileMeshNor,			s.pMesh[i].sNormals,	s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;
			if(!WriteCPODData(pFile, ePODFileMeshTan,			s.pMesh[i].sTangents,	s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;
			if(!WriteCPODData(pFile, ePODFileMeshBin,			s.pMesh[i].sBinormals,	s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;
			for(j = 0; j < s.pMesh[i].nNumUVW; ++j)
				if(!WriteCPODData(pFile, ePODFileMeshUVW,		s.pMesh[i].psUVW[j],	s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;
			if(!WriteCPODData(pFile, ePODFileMeshVtxCol,		s.pMesh[i].sVtxColours, s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;
			if(!WriteCPODData(pFile, ePODFileMeshBoneIdx,		s.pMesh[i].sBoneIdx,	s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;
			if(!WriteCPODData(pFile, ePODFileMeshBoneWeight,	s.pMesh[i].sBoneWeight,	s.pMesh[i].nNumVertex, s.pMesh[i].pInterleaved == 0)) return false;

			if(!WriteMarker(pFile, ePODFileMesh, true)) return false;
		}

		// Save: node
		for(i = 0; i < s.nNumNode; ++i) {
			if(!WriteMarker(pFile, ePODFileNode, false)) return false;
			if(!WriteData(pFile, ePODFileNodeIdx,		s.pNode[i].nIdx)) return false;
			if(!WriteData(pFile, ePODFileNodeName,		s.pNode[i].pszName, (unsigned int)strlen(s.pNode[i].pszName)+1)) return false;
			if(!WriteData(pFile, ePODFileNodeIdxMat,	s.pNode[i].nIdxMaterial)) return false;
			if(!WriteData(pFile, ePODFileNodeIdxParent,	s.pNode[i].nIdxParent)) return false;
			if(!WriteData(pFile, ePODFileNodePos,		s.pNode[i].pfPosition)) return false;
			if(!WriteData(pFile, ePODFileNodeRot,		s.pNode[i].pfRotation)) return false;
			if(!WriteData(pFile, ePODFileNodeScale,		s.pNode[i].pfScale)) return false;
			if(!WriteData(pFile, ePODFileNodeAnimPos,	s.pNode[i].pfAnimPosition,	s.nNumFrame * 3 * sizeof(*s.pNode[i].pfAnimPosition))) return false;
			if(!WriteData(pFile, ePODFileNodeAnimRot,	s.pNode[i].pfAnimRotation,	s.nNumFrame * 4 * sizeof(*s.pNode[i].pfAnimRotation))) return false;
			if(!WriteData(pFile, ePODFileNodeAnimScale,	s.pNode[i].pfAnimScale,		s.nNumFrame * 7 * sizeof(*s.pNode[i].pfAnimScale))) return false;
			if(!WriteMarker(pFile, ePODFileNode, true)) return false;
		}

		// Save: texture
		for(i = 0; i < s.nNumTexture; ++i) {
			if(!WriteMarker(pFile, ePODFileTexture, false)) return false;
			if(!WriteData(pFile, ePODFileTexName,		s.pTexture[i].pszName, (unsigned int)strlen(s.pTexture[i].pszName)+1)) return false;
			if(!WriteMarker(pFile, ePODFileTexture, true)) return false;
		}
	}
	if(!WriteMarker(pFile, ePODFileScene, true)) return false;

	return true;
}

/****************************************************************************
** Local code: File reading
****************************************************************************/
static bool ReadCPODData(
	CPODData			&s,
	CStreamResource				&src,
	const unsigned int	nSpec,
	const bool			bValidData)
{
	unsigned int nName, nLen;

	while(src.ReadMarker(nName, nLen))
	{
		if(nName == (nSpec | MODELPOD_TAG_END))
			return true;

		switch(nName)
		{
		case ePODFileDataType:	if(!src.Read(s.eType)) return false;					break;
		case ePODFileN:			if(!src.Read(s.n)) return false;						break;
		case ePODFileStride:	if(!src.Read(s.nStride)) return false;					break;
		case ePODFileData:		if(bValidData) { if(!src.ReadAfterAlloc(s.pData, nLen)) return false; } else { if(!src.Read(s.pData)) return false; }	break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

static bool ReadCamera(
	SPODCamera	&s,
	CStreamResource		&src)
{
	unsigned int nName, nLen;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileCamera | MODELPOD_TAG_END:			return true;

		case ePODFileCamIdxTgt:		if(!src.Read(s.nIdxTarget)) return false;					break;
		case ePODFileCamFOV:		if(!src.Read(s.fFOV)) return false;							break;
		case ePODFileCamFar:		if(!src.Read(s.fFar)) return false;							break;
		case ePODFileCamNear:		if(!src.Read(s.fNear)) return false;						break;
		case ePODFileCamAnimFOV:	if(!src.ReadAfterAlloc(s.pfAnimFOV, nLen)) return false;	break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

static bool ReadLight(
	SPODLight	&s,
	CStreamResource		&src)
{
	unsigned int nName, nLen;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileLight | MODELPOD_TAG_END:			return true;

		case ePODFileLightIdxTgt:	if(!src.Read(s.nIdxTarget)) return false;	break;
		case ePODFileLightColour:	if(!src.Read(s.pfColour)) return false;		break;
		case ePODFileLightType:		if(!src.Read(s.eType)) return false;		break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

static bool ReadMaterial(
	SPODMaterial	&s,
	CStreamResource			&src)
{
	unsigned int nName, nLen;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileMaterial | MODELPOD_TAG_END:			return true;

		case ePODFileMatName:			if(!src.ReadAfterAlloc(s.pszName, nLen)) return false;			break;
		case ePODFileMatIdxTexDiffuse:	if(!src.Read(s.nIdxTexDiffuse)) return false;					break;
		case ePODFileMatOpacity:		if(!src.Read(s.fMatOpacity)) return false;						break;
		case ePODFileMatAmbient:		if(!src.Read(s.pfMatAmbient)) return false;						break;
		case ePODFileMatDiffuse:		if(!src.Read(s.pfMatDiffuse)) return false;						break;
		case ePODFileMatSpecular:		if(!src.Read(s.pfMatSpecular)) return false;					break;
		case ePODFileMatShininess:		if(!src.Read(s.fMatShininess)) return false;					break;
		case ePODFileMatEffectFile:		if(!src.ReadAfterAlloc(s.pszEffectFile, nLen)) return false;	break;
		case ePODFileMatEffectName:		if(!src.ReadAfterAlloc(s.pszEffectName, nLen)) return false;	break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

static bool ReadMesh(
	SPODMesh	&s,
	CStreamResource		&src)
{
	unsigned int	nName, nLen;
	unsigned int	nUVWs=0;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileMesh | MODELPOD_TAG_END:
			if(nUVWs != s.nNumUVW) return false;
			return true;

		case ePODFileMeshNumVtx:			if(!src.Read(s.nNumVertex)) return false;													break;
		case ePODFileMeshNumFaces:			if(!src.Read(s.nNumFaces)) return false;													break;
		case ePODFileMeshNumUVW:			if(!src.Read(s.nNumUVW)) return false;	if(!SafeAlloc(s.psUVW, s.nNumUVW)) return false;	break;
		case ePODFileMeshStripLength:		if(!src.ReadAfterAlloc(s.pnStripLength, nLen)) return false;								break;
		case ePODFileMeshNumStrips:			if(!src.Read(s.nNumStrips)) return false;													break;
		case ePODFileMeshInterleaved:		if(!src.ReadAfterAlloc(s.pInterleaved, nLen)) return false;									break;
		case ePODFileMeshBoneBatches:		if(!src.ReadAfterAlloc(s.sBoneBatches.pnBatches, nLen)) return false;						break;
		case ePODFileMeshBoneBatchBoneCnts:	if(!src.ReadAfterAlloc(s.sBoneBatches.pnBatchBoneCnt, nLen)) return false;					break;
		case ePODFileMeshBoneBatchOffsets:	if(!src.ReadAfterAlloc(s.sBoneBatches.pnBatchOffset, nLen)) return false;					break;
		case ePODFileMeshBoneBatchBoneMax:	if(!src.Read(s.sBoneBatches.nBatchBoneMax)) return false;									break;
		case ePODFileMeshBoneBatchCnt:		if(!src.Read(s.sBoneBatches.nBatchCnt)) return false;										break;

		case ePODFileMeshFaces:			if(!ReadCPODData(s.sFaces, src, ePODFileMeshFaces, true)) return false;			break;
		case ePODFileMeshVtx:			if(!ReadCPODData(s.sVertex, src, ePODFileMeshVtx, s.pInterleaved == 0)) return false;			break;
		case ePODFileMeshNor:			if(!ReadCPODData(s.sNormals, src, ePODFileMeshNor, s.pInterleaved == 0)) return false;			break;
		case ePODFileMeshTan:			if(!ReadCPODData(s.sTangents, src, ePODFileMeshTan, s.pInterleaved == 0)) return false;			break;
		case ePODFileMeshBin:			if(!ReadCPODData(s.sBinormals, src, ePODFileMeshBin, s.pInterleaved == 0)) return false;			break;
		case ePODFileMeshUVW:			if(!ReadCPODData(s.psUVW[nUVWs++], src, ePODFileMeshUVW, s.pInterleaved == 0)) return false;		break;
		case ePODFileMeshVtxCol:		if(!ReadCPODData(s.sVtxColours, src, ePODFileMeshVtxCol, s.pInterleaved == 0)) return false;		break;
		case ePODFileMeshBoneIdx:		if(!ReadCPODData(s.sBoneIdx, src, ePODFileMeshBoneIdx, s.pInterleaved == 0)) return false;		break;
		case ePODFileMeshBoneWeight:	if(!ReadCPODData(s.sBoneWeight, src, ePODFileMeshBoneWeight, s.pInterleaved == 0)) return false;	break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

static bool ReadNode(
	SPODNode	&s,
	CStreamResource		&src)
{
	unsigned int nName, nLen;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileNode | MODELPOD_TAG_END:			return true;

		case ePODFileNodeIdx:		if(!src.Read(s.nIdx)) return false;								break;
		case ePODFileNodeName:		if(!src.ReadAfterAlloc(s.pszName, nLen)) return false;			break;
		case ePODFileNodeIdxMat:	if(!src.Read(s.nIdxMaterial)) return false;						break;
		case ePODFileNodeIdxParent:	if(!src.Read(s.nIdxParent)) return false;						break;
		case ePODFileNodePos:		if(!src.Read(s.pfPosition)) return false;						break;
		case ePODFileNodeRot:		if(!src.Read(s.pfRotation)) return false;						break;
		case ePODFileNodeScale:		if(!src.Read(s.pfScale)) return false;							break;
		case ePODFileNodeAnimPos:	if(!src.ReadAfterAlloc(s.pfAnimPosition, nLen)) return false;	break;
		case ePODFileNodeAnimRot:	if(!src.ReadAfterAlloc(s.pfAnimRotation, nLen)) return false;	break;
		case ePODFileNodeAnimScale:	if(!src.ReadAfterAlloc(s.pfAnimScale, nLen)) return false;		break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

static bool ReadTexture(
	SPODTexture	&s,
	CStreamResource		&src)
{
	unsigned int nName, nLen;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileTexture | MODELPOD_TAG_END:			return true;

		case ePODFileTexName:		if(!src.ReadAfterAlloc(s.pszName, nLen)) return false;			break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

static bool ReadScene(
	SPODScene	&s,
	CStreamResource		&src)
{
	unsigned int nName, nLen;
	unsigned int nCameras=0, nLights=0, nMaterials=0, nMeshes=0, nTextures=0, nNodes=0;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileScene | MODELPOD_TAG_END:
			if(nCameras		!= s.nNumCamera) return false;
			if(nLights		!= s.nNumLight) return false;
			if(nMaterials	!= s.nNumMaterial) return false;
			if(nMeshes		!= s.nNumMesh) return false;
			if(nTextures	!= s.nNumTexture) return false;
			if(nNodes		!= s.nNumNode) return false;
			return true;

		case ePODFileColourBackground:	if(!src.Read(s.pfColourBackground)) return false;	break;
		case ePODFileColourAmbient:		if(!src.Read(s.pfColourAmbient)) return false;		break;
		case ePODFileNumCamera:			if(!src.Read(s.nNumCamera)) return false;			if(!SafeAlloc(s.pCamera, s.nNumCamera)) return false;		break;
		case ePODFileNumLight:			if(!src.Read(s.nNumLight)) return false;			if(!SafeAlloc(s.pLight, s.nNumLight)) return false;			break;
		case ePODFileNumMesh:			if(!src.Read(s.nNumMesh)) return false;				if(!SafeAlloc(s.pMesh, s.nNumMesh)) return false;			break;
		case ePODFileNumNode:			if(!src.Read(s.nNumNode)) return false;				if(!SafeAlloc(s.pNode, s.nNumNode)) return false;			break;
		case ePODFileNumMeshNode:		if(!src.Read(s.nNumMeshNode)) return false;			break;
		case ePODFileNumTexture:		if(!src.Read(s.nNumTexture)) return false;			if(!SafeAlloc(s.pTexture, s.nNumTexture)) return false;		break;
		case ePODFileNumMaterial:		if(!src.Read(s.nNumMaterial)) return false;			if(!SafeAlloc(s.pMaterial, s.nNumMaterial)) return false;	break;
		case ePODFileNumFrame:			if(!src.Read(s.nNumFrame)) return false;			break;
		case ePODFileFlags:				if(!src.Read(s.nFlags)) return false;				break;

		case ePODFileCamera:	if(!ReadCamera(s.pCamera[nCameras++], src)) return false;		break;
		case ePODFileLight:		if(!ReadLight(s.pLight[nLights++], src)) return false;			break;
		case ePODFileMaterial:	if(!ReadMaterial(s.pMaterial[nMaterials++], src)) return false;	break;
		case ePODFileMesh:		if(!ReadMesh(s.pMesh[nMeshes++], src)) return false;			break;
		case ePODFileNode:		if(!ReadNode(s.pNode[nNodes++], src)) return false;				break;
		case ePODFileTexture:	if(!ReadTexture(s.pTexture[nTextures++], src)) return false;	break;

		default:
			if(!src.Skip(nLen)) return false;
		}
	}
	return false;
}

/*!***************************************************************************
 @Function			Read
 @Output			pS
 @Input				src
 @Output			pszExpOpt
 @Input				count
 @Description		Loads the specified ".POD" file; returns the scene in
					pScene. This structure must later be destroyed with
					ModelPODDestroy() to prevent memory leaks.
					".POD" files are exported from 3D Studio MAX using a
					PowerVR plugin.
*****************************************************************************/
static bool Read(
	SPODScene		* const pS,
	CStreamResource			&src,
	char			* const pszExpOpt,
	const size_t	count)
{
	unsigned int	nName, nLen;
	bool			bVersionOK = false, bDone = false;

	while(src.ReadMarker(nName, nLen))
	{
		switch(nName)
		{
		case ePODFileVersion:
			{
				char *pszVersion = NULL;
				if(nLen != strlen(MODELPOD_VERSION)+1) return false;
				if(!SafeAlloc(pszVersion, nLen)) return false;
				if(!src.Read(pszVersion, nLen)) return false;
				if(strcmp(pszVersion, MODELPOD_VERSION) != 0) return false;
				bVersionOK = true;
				delete(pszVersion);
			}
			continue;

		case ePODFileScene:
			if(pS)
			{
				if(!ReadScene(*pS, src))
					return false;
				bDone = true;
			}
			continue;

		case ePODFileExpOpt:
			if(pszExpOpt)
			{
				if(!src.Read(pszExpOpt, _MIN(nLen, count))) return false;
				return true;
			}
			break;

		case ePODFileScene | MODELPOD_TAG_END:
			return bVersionOK == true && bDone == true;
		}

		// Unhandled data, skip it
		if(!src.Skip(nLen))
			return false;
	}

	/*
		Convert data to fixed or float point as this build desires
	*/
#ifdef  FIXEDPOINTENABLE
	if(!(pS->nFlags & MODELPODSF_FIXED))
		ModelPODToggleFixedPoint(*pS);
#else
	if(pS->nFlags & MODELPODSF_FIXED)
		ModelPODToggleFixedPoint(*pS);
#endif

	return bVersionOK == true && bDone == true;
}

/****************************************************************************
** Class: CPODScene
****************************************************************************/

/*!***************************************************************************
 @Function			ReadFromFile
 @Input				pszFileName		Filename to load
 @Output			pszExpOpt		String in which to place exporter options
 @Input				count			Maximum number of characters to store.
 @Return			TRUE if successful, FALSE if not
 @Description		Loads the specified ".POD" file; returns the scene in
					pScene. This structure must later be destroyed with
					ModelPODDestroy() to prevent memory leaks.
					".POD" files are exported from 3D Studio MAX using a
					PowerVR plugin.
					If pszExpOpt is NULL, the scene is loaded; otherwise the
					scene is not loaded and pszExpOpt is filled in.
*****************************************************************************/
bool CPODScene::ReadFromFile(
	const char		* const pszFileName,
	char			* const pszExpOpt,
	const size_t	count)
{
	CSourceStream src;

	if(!src.Init(pszFileName))
		return false;

	memset(this, 0, sizeof(*this));
	if(!Read(pszExpOpt ? NULL : this, src, pszExpOpt, count))
		return false;
	if(!InitImpl())
		return false;
	return true;
}

/*!***************************************************************************
 @Function			ReadFromMemory
 @Input				scene			Scene data from the header file
 @Return			TRUE if successful, FALSE if not
 @Description		Sets the scene data from the supplied data structure. Use
					when loading from .H files.
*****************************************************************************/
bool CPODScene::ReadFromMemory(
	const SPODScene &scene)
{
	memset(this, 0, sizeof(*this));
	*(SPODScene*)this = scene;

	if(!InitImpl())
		return false;
	m_pImpl->bFromMemory = true;
	return true;
}

#ifdef WIN32
/*!***************************************************************************
 @Function			ReadFromResource
 @Input				pszName			Name of the resource to load from
 @Return			TRUE if successful, FALSE if not
 @Description		Loads the specified ".POD" file; returns the scene in
					pScene. This structure must later be destroyed with
					ModelPODDestroy() to prevent memory leaks.
					".POD" files are exported from 3D Studio MAX using a
					PowerVR plugin.
*****************************************************************************/
bool CPODScene::ReadFromResource(
	const TCHAR * const pszName)
{
	CSourceResource src;

	if(!src.Init(pszName))
		return false;

	memset(this, 0, sizeof(*this));
	if(!Read(this, src, NULL, 0))
		return false;
	if(!InitImpl())
		return false;
	return true;
}
#endif /* WIN32 */

/*!***********************************************************************
 @Function		InitImpl
 @Description	Used by the Read*() fns to initialise implementation
				details. Should also be called by applications which
				manually build data in the POD structures for rendering;
				in this case call it after the data has been created.
				Otherwise, do not call this function.
*************************************************************************/
bool CPODScene::InitImpl()
{
	// Allocate space for implementation data
	m_pImpl = new SPODImpl;
	if(!m_pImpl)
		return false;

	// Zero implementation data
	memset(m_pImpl, 0, sizeof(*m_pImpl));

#ifdef _DEBUG
	m_pImpl->nWmTotal = 0;
#endif

	// Allocate world-matrix cache
	m_pImpl->pfCache		= new VERTTYPE[nNumNode];
	m_pImpl->pWmCache		= new  MATRIX[nNumNode];
	m_pImpl->pWmZeroCache	= new  MATRIX[nNumNode];
	FlushCache();

	return true;
}

/*!***********************************************************************
 @Function		FlushCache
 @Description	Clears the matrix cache; use this if necessary when you
				edit the position or animation of a node.
*************************************************************************/
void CPODScene::FlushCache()
{
	// Pre-calc frame zero matrices
	SetFrame(0);
	for(unsigned int i = 0; i < nNumNode; ++i)
		GetWorldMatrixNoCache(m_pImpl->pWmZeroCache[i], pNode[i]);

	// Load cache with frame-zero data
	memcpy(m_pImpl->pWmCache, m_pImpl->pWmZeroCache, nNumNode * sizeof(*m_pImpl->pWmCache));
	memset(m_pImpl->pfCache, 0, nNumNode * sizeof(*m_pImpl->pfCache));
}

/*!***************************************************************************
 @Function			Constructor
 @Description		Initializes the pointer to scene data to NULL
*****************************************************************************/
CPODScene::CPODScene() : m_pImpl(NULL)
{}

/*!***************************************************************************
 @Function			Destructor
 @Description		Frees the memory allocated to store the scene in pScene.
*****************************************************************************/
CPODScene::~CPODScene()
{
	Destroy();
}

/*!***************************************************************************
 @Function			Destroy
 @Description		Frees the memory allocated to store the scene in pScene.
*****************************************************************************/
void CPODScene::Destroy()
{
	unsigned int	i;

	if(m_pImpl != NULL)
	{
		/*
			Only attempt to free this memory if it was actually allocated at
			run-time, as opposed to compiled into the app.
		*/
		if(!m_pImpl->bFromMemory)
		{

			for(i = 0; i < nNumCamera; ++i)
				delete(pCamera[i].pfAnimFOV);
			delete(pCamera);

			delete(pLight);

			for(i = 0; i < nNumMaterial; ++i)
			{
				delete(pMaterial[i].pszName);
				delete(pMaterial[i].pszEffectFile);
				delete(pMaterial[i].pszEffectName);
			}
			delete(pMaterial);

			for(i = 0; i < nNumMesh; ++i) {
				delete(pMesh[i].sFaces.pData);
				delete(pMesh[i].pnStripLength);
				if(pMesh[i].pInterleaved)
				{
					delete(pMesh[i].pInterleaved);
				}
				else
				{
					delete(pMesh[i].sVertex.pData);
					delete(pMesh[i].sNormals.pData);
					delete(pMesh[i].sTangents.pData);
					delete(pMesh[i].sBinormals.pData);
					for(unsigned int j = 0; j < pMesh[i].nNumUVW; ++j)
						delete(pMesh[i].psUVW[j].pData);
					delete(pMesh[i].sVtxColours.pData);
					delete(pMesh[i].sBoneIdx.pData);
					delete(pMesh[i].sBoneWeight.pData);
				}
				delete(pMesh[i].psUVW);
				pMesh[i].sBoneBatches.Release();
			}
			delete(pMesh);

			for(i = 0; i < nNumNode; ++i) {
				delete(pNode[i].pszName);
				delete(pNode[i].pfAnimPosition);
				delete(pNode[i].pfAnimRotation);
				delete(pNode[i].pfAnimScale);
			}
			delete(pNode);

			for(i = 0; i < nNumTexture; ++i)
				delete(pTexture[i].pszName);
			delete(pTexture);
		}

		// Free the working space used by the implementation
		delete [] m_pImpl->pfCache;
		delete [] m_pImpl->pWmCache;
		delete [] m_pImpl->pWmZeroCache;
		delete m_pImpl;
	}

	memset(this, 0, sizeof(*this));
}

/*!***************************************************************************
 @Function			SetFrame
 @Input				fFrame			Frame number
 @Description		Set the animation frame for which subsequent Get*() calls
					should return data.
*****************************************************************************/
void CPODScene::SetFrame(
	const VERTTYPE fFrame)
{
	if(nNumFrame) {
		/*
			Limit animation frames.

			Example: If there are 100 frames of animation, the highest frame
			number allowed is 98, since that will blend between frames 98 and
			99. (99 being of course the 100th frame.)
		*/
		_ASSERT(fFrame <= f2vt((float)(nNumFrame-1)));
		m_pImpl->nFrame = (int)vt2f(fFrame);
		m_pImpl->fBlend = fFrame - f2vt(m_pImpl->nFrame);
	}
	else
	{
		m_pImpl->fBlend = 0;
		m_pImpl->nFrame = 0;
	}

	m_pImpl->fFrame = fFrame;
}

/*!***************************************************************************
 @Function			GetRotationMatrix
 @Output			mOut			Rotation matrix
 @Input				node			Node to get the rotation matrix from
 @Description		Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
*****************************************************************************/
void CPODScene::GetRotationMatrix(
	 MATRIX		&mOut,
	const SPODNode	&node) const
{
	 QUATERNION	q;

	if(node.pfAnimRotation)
	{
		 MatrixQuaternionSlerp(
			q,
			( QUATERNION&)node.pfAnimRotation[4*m_pImpl->nFrame],
			( QUATERNION&)node.pfAnimRotation[4*(m_pImpl->nFrame+1)], m_pImpl->fBlend);
		 MatrixRotationQuaternion(mOut, q);
	}
	else
	{
		 MatrixRotationQuaternion(mOut, *( QUATERNION*)node.pfRotation);
	}
}

/*!***************************************************************************
 @Function			GetScalingMatrix
 @Output			mOut			Scaling matrix
 @Input				node			Node to get the rotation matrix from
 @Description		Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
*****************************************************************************/
void CPODScene::GetScalingMatrix(
	 MATRIX		&mOut,
	const SPODNode	&node) const
{
	 VECTOR3 v;

	if(node.pfAnimScale)
	{
		 MatrixVec3Lerp(
			v,
			( VECTOR3&)node.pfAnimScale[7*(m_pImpl->nFrame+0)],
			( VECTOR3&)node.pfAnimScale[7*(m_pImpl->nFrame+1)], m_pImpl->fBlend);
		 MatrixScaling(mOut, v.x, v.y, v.z);
	}
	else
	{
		 MatrixScaling(mOut, node.pfScale[0], node.pfScale[1], node.pfScale[2]);
	}
}

/*!***************************************************************************
 @Function			GetTranslation
 @Output			V				Translation vector
 @Input				node			Node to get the translation vector from
 @Description		Generates the translation vector for the given Mesh
					Instance. Uses animation data.
*****************************************************************************/
void CPODScene::GetTranslation(
	 VECTOR3		&V,
	const SPODNode	&node) const
{
	if(node.pfAnimPosition) {
		 MatrixVec3Lerp(
			V,
			( VECTOR3&)node.pfAnimPosition[3 * (m_pImpl->nFrame+0)],
			( VECTOR3&)node.pfAnimPosition[3 * (m_pImpl->nFrame+1)], m_pImpl->fBlend);
	} else {
		V = *( VECTOR3*)node.pfPosition;
	}
}

/*!***************************************************************************
 @Function			GetTranslationMatrix
 @Output			mOut			Translation matrix
 @Input				node			Node to get the translation matrix from
 @Description		Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
*****************************************************************************/
void CPODScene::GetTranslationMatrix(
	 MATRIX		&mOut,
	const SPODNode	&node) const
{
	 VECTOR3 v;

	if(node.pfAnimPosition)
	{
		 MatrixVec3Lerp(
			v,
			( VECTOR3&)node.pfAnimPosition[3*(m_pImpl->nFrame+0)],
			( VECTOR3&)node.pfAnimPosition[3*(m_pImpl->nFrame+1)], m_pImpl->fBlend);
		 MatrixTranslation(mOut, v.x, v.y, v.z);
	}
	else
	{
		 MatrixTranslation(mOut,
			node.pfPosition[0],
			node.pfPosition[1],
			node.pfPosition[2]);
	}
}

/*!***************************************************************************
 @Function			GetWorldMatrixNoCache
 @Output			mOut			World matrix
 @Input				node			Node to get the world matrix from
 @Description		Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
*****************************************************************************/
void CPODScene::GetWorldMatrixNoCache(
	 MATRIX		&mOut,
	const SPODNode	&node) const
{
	 MATRIX mTmp;

	// Scale
	GetScalingMatrix(mOut, node);

	// Rotation
	GetRotationMatrix(mTmp, node);
	 MatrixMultiply(mOut, mOut, mTmp);

	// Translation
	GetTranslationMatrix(mTmp, node);
	 MatrixMultiply(mOut, mOut, mTmp);

	// Do we have to worry about a parent?
	if(node.nIdxParent < 0)
		return;

	// Apply parent's transform too.
	GetWorldMatrixNoCache(mTmp, pNode[node.nIdxParent]);
	 MatrixMultiply(mOut, mOut, mTmp);
}

/*!***************************************************************************
 @Function			GetWorldMatrix
 @Output			mOut			World matrix
 @Input				node			Node to get the world matrix from
 @Description		Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
*****************************************************************************/
void CPODScene::GetWorldMatrix(
	 MATRIX		&mOut,
	const SPODNode	&node) const
{
	unsigned int nIdx;

#ifdef _DEBUG
	++m_pImpl->nWmTotal;
	m_pImpl->fHitPerc = (float)m_pImpl->nWmCacheHit / (float)m_pImpl->nWmTotal;
	m_pImpl->fHitPercZero = (float)m_pImpl->nWmZeroCacheHit / (float)m_pImpl->nWmTotal;
#endif

	// Calculate a node index
	nIdx = (unsigned int)(&node - pNode);

	// There is a dedicated cache for frame 0 data
	if(m_pImpl->fFrame == 0)
	{
		mOut = m_pImpl->pWmZeroCache[nIdx];
#ifdef _DEBUG
		++m_pImpl->nWmZeroCacheHit;
#endif
		return;
	}

	// Has this matrix been calculated & cached?
	if(m_pImpl->fFrame == m_pImpl->pfCache[nIdx])
	{
		mOut = m_pImpl->pWmCache[nIdx];
#ifdef _DEBUG
		++m_pImpl->nWmCacheHit;
#endif
		return;
	}

	GetWorldMatrixNoCache(mOut, node);

	// Cache the matrix
	m_pImpl->pfCache[nIdx]	= m_pImpl->fFrame;
	m_pImpl->pWmCache[nIdx]	= mOut;
}

/*!***************************************************************************
 @Function			GetBoneWorldMatrix
 @Output			mOut			Bone world matrix
 @Input				NodeMesh		Mesh to take the bone matrix from
 @Input				NodeBone		Bone to take the matrix from
 @Description		Generates the world matrix for the given bone.
*****************************************************************************/
void CPODScene::GetBoneWorldMatrix(
	 MATRIX		&mOut,
	const SPODNode	&NodeMesh,
	const SPODNode	&NodeBone)
{
	 MATRIX	mTmp;
	VERTTYPE	fFrame;

	fFrame = m_pImpl->fFrame;

	SetFrame(0);

	// Transform by object matrix
	GetWorldMatrix(mOut, NodeMesh);

	// Back transform bone from frame 0 position
	GetWorldMatrix(mTmp, NodeBone);
	 MatrixInverse(mTmp, mTmp);
	 MatrixMultiply(mOut, mOut, mTmp);

	// The bone origin should now be at the origin

	SetFrame(fFrame);

	// Transform bone into frame fFrame position
	GetWorldMatrix(mTmp, NodeBone);
	 MatrixMultiply(mOut, mOut, mTmp);
}

/*!***************************************************************************
 @Function			GetCamera
 @Output			vFrom			Position of the camera
 @Output			vTo				Target of the camera
 @Output			vUp				Up direction of the camera
 @Input				nIdx			Camera number
 @Return			Camera horizontal FOV
 @Description		Calculate the From, To and Up vectors for the given
					camera. Uses animation data.
					Note that even if the camera has a target, *pvTo is not
					the position of that target. *pvTo is a position in the
					correct direction of the target, one unit away from the
					camera.
*****************************************************************************/
VERTTYPE CPODScene::GetCamera(
	 VECTOR3			&vFrom,
	 VECTOR3			&vTo,
	 VECTOR3			&vUp,
	const unsigned int	nIdx) const
{
	 MATRIX		mTmp;
	VERTTYPE		*pfData;
	SPODCamera		*pCam;
	const SPODNode	*pNd;

	_ASSERT(nIdx < nNumCamera);

	// Camera nodes are after the mesh and light nodes in the array
	pNd = &pNode[nNumMeshNode + nNumLight + nIdx];

	pCam = &pCamera[pNd->nIdx];

	GetWorldMatrix(mTmp, *pNd);

	// View position is 0,0,0,1 transformed by world matrix
	vFrom.x = mTmp.f[12];
	vFrom.y = mTmp.f[13];
	vFrom.z = mTmp.f[14];

	// View direction is 0,-1,0,1 transformed by world matrix
	vTo.x = -mTmp.f[4] + mTmp.f[12];
	vTo.y = -mTmp.f[5] + mTmp.f[13];
	vTo.z = -mTmp.f[6] + mTmp.f[14];

#ifdef BUILD_DX9
	/*
		When you rotate the camera from "straight forward" to "straight down", in
		D3D the UP vector will be [0, 0, 1]
	*/
	vUp.x = mTmp.f[ 8];
	vUp.y = mTmp.f[ 9];
	vUp.z = mTmp.f[10];
#endif

#if defined(BUILD_OGL) || defined(BUILD_OGLES) || defined(BUILD_OGLES2)
	/*
		When you rotate the camera from "straight forward" to "straight down", in
		OpenGL the UP vector will be [0, 0, -1]
	*/
	vUp.x = -mTmp.f[ 8];
	vUp.y = -mTmp.f[ 9];
	vUp.z = -mTmp.f[10];
#endif

	/*
		Find & calculate FOV value
	*/
	if(pCam->pfAnimFOV) {
		pfData = &pCam->pfAnimFOV[m_pImpl->nFrame];

		return pfData[0] + m_pImpl->fBlend * (pfData[1] - pfData[0]);
	} else {
		return pCam->fFOV;
	}
}

/*!***************************************************************************
 @Function			GetCameraPos
 @Output			vFrom			Position of the camera
 @Output			vTo				Target of the camera
 @Input				nIdx			Camera number
 @Return			Camera horizontal FOV
 @Description		Calculate the position of the camera and its target. Uses
					animation data.
					If the queried camera does not have a target, *pvTo is
					not changed.
*****************************************************************************/
VERTTYPE CPODScene::GetCameraPos(
	 VECTOR3			&vFrom,
	 VECTOR3			&vTo,
	const unsigned int	nIdx) const
{
	 MATRIX		mTmp;
	VERTTYPE		*pfData;
	SPODCamera		*pCam;
	const SPODNode	*pNd;

	_ASSERT(nIdx < nNumCamera);

	// Camera nodes are after the mesh and light nodes in the array
	pNd = &pNode[nNumMeshNode + nNumLight + nIdx];

	// View position is 0,0,0,1 transformed by world matrix
	GetWorldMatrix(mTmp, *pNd);
	vFrom.x = mTmp.f[12];
	vFrom.y = mTmp.f[13];
	vFrom.z = mTmp.f[14];

	pCam = &pCamera[pNd->nIdx];
	if(pCam->nIdxTarget >= 0)
	{
		// View position is 0,0,0,1 transformed by world matrix
		GetWorldMatrix(mTmp, pNode[pCam->nIdxTarget]);
		vTo.x = mTmp.f[12];
		vTo.y = mTmp.f[13];
		vTo.z = mTmp.f[14];
	}

	/*
		Find & calculate FOV value
	*/
	if(pCam->pfAnimFOV) {
		pfData = &pCam->pfAnimFOV[m_pImpl->nFrame];

		return pfData[0] + m_pImpl->fBlend * (pfData[1] - pfData[0]);
	} else {
		return pCam->fFOV;
	}
}

/*!***************************************************************************
 @Function			GetLight
 @Output			vPos			Position of the light
 @Output			vDir			Direction of the light
 @Input				nIdx			Light number
 @Description		Calculate the position and direction of the given Light.
					Uses animation data.
*****************************************************************************/
void CPODScene::GetLight(
	 VECTOR3			&vPos,
	 VECTOR3			&vDir,
	const unsigned int	nIdx) const
{
	 MATRIX		mTmp;
	const SPODNode	*pNd;

	_ASSERT(nIdx < nNumLight);

	// Light nodes are after the mesh nodes in the array
	pNd = &pNode[nNumMeshNode + nIdx];

	GetWorldMatrix(mTmp, *pNd);

	// View position is 0,0,0,1 transformed by world matrix
	vPos.x = mTmp.f[12];
	vPos.y = mTmp.f[13];
	vPos.z = mTmp.f[14];

	// View direction is 0,-1,0,0 transformed by world matrix
	vDir.x = -mTmp.f[4];
	vDir.y = -mTmp.f[5];
	vDir.z = -mTmp.f[6];
}

/*!***************************************************************************
 @Function			CreateSkinIdxWeight
 @Output			pIdx				Four bytes containing matrix indices for vertex (0..255) (D3D: use UBYTE4)
 @Output			pWeight				Four bytes containing blend weights for vertex (0.0 .. 1.0) (D3D: use D3DCOLOR)
 @Input				nVertexBones		Number of bones this vertex uses
 @Input				pnBoneIdx			Pointer to 'nVertexBones' indices
 @Input				pfBoneWeight		Pointer to 'nVertexBones' blend weights
 @Description		Creates the matrix indices and blend weights for a boned
					vertex. Call once per vertex of a boned mesh.
*****************************************************************************/
bool CPODScene::CreateSkinIdxWeight(
	char			* const pIdx,			// Four bytes containing matrix indices for vertex (0..255) (D3D: use UBYTE4)
	char			* const pWeight,		// Four bytes containing blend weights for vertex (0.0 .. 1.0) (D3D: use D3DCOLOR)
	const int		nVertexBones,			// Number of bones this vertex uses
	const int		* const pnBoneIdx,		// Pointer to 'nVertexBones' indices
	const VERTTYPE	* const pfBoneWeight)	// Pointer to 'nVertexBones' blend weights
{
	int i, nSum;
	int nIdx[4];
	int nWeight[4];

	for(i = 0; i < nVertexBones; ++i)
	{
		nIdx[i]		= pnBoneIdx[i];
		nWeight[i]	= (int)vt2f((VERTTYPEMUL(f2vt(255.0f), pfBoneWeight[i])));

		if(nIdx[i] > 255)
		{
			_RPT0(_CRT_WARN, "Too many bones (highest index is 255).\n");
			return false;
		}

		nWeight[i]	= max(nWeight[i], 0);
		nWeight[i]	= min(nWeight[i], 255);
	}

	for(; i < 4; ++i)
	{
		nIdx[i]		= 0;
		nWeight[i]	= 0;
	}

	if(nVertexBones)
	{
		// It's important the weights sum to 1
		nSum = 0;
		for(i = 0; i < 4; ++i)
			nSum += nWeight[i];

		if(!nSum)
			return false;

		_ASSERT(nSum <= 255);

		i = 0;
		while(nSum < 255)
		{
			if(nWeight[i]) {
				++nWeight[i];
				++nSum;
			}

			if(++i > 3)
				i = 0;
		}

		_ASSERT(nSum == 255);
	}

#ifdef BUILD_DX9
	*(unsigned int*)pIdx = D3DCOLOR_ARGB(nIdx[3], nIdx[2], nIdx[1], nIdx[0]);					// UBYTE4 is WZYX
	*(unsigned int*)pWeight = D3DCOLOR_RGBA(nWeight[0], nWeight[1], nWeight[2], nWeight[3]);	// D3DCOLORs are WXYZ
#endif

#if defined(BUILD_OGL) || defined(BUILD_OGLES) || defined(BUILD_OGLES2)
	// Return indices and weights as bytes
	for(i = 0; i < 4; ++i)
	{
		pIdx[i]		= nIdx[i];
		pWeight[i]	= nWeight[i];
	}
#endif

	return true;
}

/*!***************************************************************************
 @Function			SavePOD
 @Input				pszFilename		Filename to save to
 @Input				pszExpOpt		A string containing the options used by the exporter
 @Description		Save a binary POD file (.POD).
*****************************************************************************/
bool CPODScene::SavePOD(const char * const pszFilename, const char * const pszExpOpt)
{
	FILE	*pFile;
	bool	bRet;

	pFile = fopen(pszFilename, "wb+");
	if(!pFile)
		return false;

	bRet = WritePOD(pFile, pszExpOpt, *this);

	// Done
	fclose(pFile);
	return bRet;
}

/****************************************************************************
** Code
****************************************************************************/

/*!***************************************************************************
 @Function			ModelPODDataTypeSize
 @Input				type		Type to get the size of
 @Return			Size of the data element
 @Description		Returns the size of each data element.
*****************************************************************************/
size_t ModelPODDataTypeSize(const EDataType type)
{
	switch(type)
	{
	default:
		_ASSERT(false);
		return 0;
	case EPODDataFloat:
		return sizeof(float);
	case EPODDataInt:
		return sizeof(int);
	case EPODDataShort:
	case EPODDataShortNorm:
	case EPODDataUnsignedShort:
		return sizeof(unsigned short);
	case EPODDataRGBA:
		return sizeof(unsigned int);
	case EPODDataARGB:
		return sizeof(unsigned int);
	case EPODDataD3DCOLOR:
		return sizeof(unsigned int);
	case EPODDataUBYTE4:
		return sizeof(unsigned int);
	case EPODDataDEC3N:
		return sizeof(unsigned int);
	case EPODDataFixed16_16:
		return sizeof(unsigned int);
	case EPODDataUnsignedByte:
	case EPODDataByte:
	case EPODDataByteNorm:
		return sizeof(unsigned char);
	}
}

/*!***************************************************************************
@Function			ModelPODDataTypeComponentCount
@Input				type		Type to get the number of components from
@Return				number of components in the data element
@Description		Returns the number of components in a data element.
*****************************************************************************/
size_t ModelPODDataTypeComponentCount(const EDataType type)
{
	switch(type)
	{
	default:
		_ASSERT(false);
		return 0;

	case EPODDataFloat:
	case EPODDataInt:
	case EPODDataShort:
	case EPODDataShortNorm:
	case EPODDataUnsignedShort:
	case EPODDataFixed16_16:
	case EPODDataByte:
	case EPODDataByteNorm:
	case EPODDataUnsignedByte:
		return 1;

	case EPODDataDEC3N:
		return 3;

	case EPODDataRGBA:
	case EPODDataARGB:
	case EPODDataD3DCOLOR:
	case EPODDataUBYTE4:
		return 4;
	}
}

/*!***************************************************************************
 @Function			ModelPODDataStride
 @Input				data		Data elements
 @Return			Size of the vector elements
 @Description		Returns the size of the vector of data elements.
*****************************************************************************/
size_t ModelPODDataStride(const CPODData &data)
{
	return ModelPODDataTypeSize(data.eType) * data.n;
}

/*!***************************************************************************
 @Function			ModelPODDataConvert
 @Modified			data		Data elements to convert
 @Input				eNewType	New type of elements
 @Input				nCnt		Number of elements
 @Description		Convert the format of the array of vectors.
*****************************************************************************/
void ModelPODDataConvert(CPODData &data, const unsigned int nCnt, const EDataType eNewType)
{
	 VECTOR4f	v;
	unsigned int	i;
	CPODData		old;

	if(!data.pData || data.eType == eNewType)
		return;

	old = data;

	switch(eNewType)
	{
	case EPODDataFloat:
	case EPODDataInt:
	case EPODDataUnsignedShort:
	case EPODDataFixed16_16:
	case EPODDataUnsignedByte:
	case EPODDataShort:
	case EPODDataShortNorm:
	case EPODDataByte:
	case EPODDataByteNorm:
		data.n = old.n * ModelPODDataTypeComponentCount(old.eType);
		break;
	case EPODDataRGBA:
	case EPODDataARGB:
	case EPODDataD3DCOLOR:
	case EPODDataUBYTE4:
	case EPODDataDEC3N:
		data.n = 1;
		break;
	default:
		break;
	}

	data.eType = eNewType;
	data.nStride = (unsigned int)ModelPODDataStride(data);

	// If the old & new strides are identical, we can convert it in place
	if(old.nStride != data.nStride)
	{
		data.pData = new unsigned char[data.nStride * nCnt];
	}

	for(i = 0; i < nCnt; ++i)
	{
		DataTypeRead(&v, old.pData + i * old.nStride, old.eType, old.n);
		DataTypeWrite(data.pData + i * data.nStride, eNewType, data.n * ModelPODDataTypeComponentCount(data.eType), &v);
	}

	if(old.nStride != data.nStride)
	{
		delete(old.pData);
	}
}

static int BitCount(unsigned int n)
{
	int nRet = 0;
	while(n)
	{
		if(n & 0x01)
			++nRet;

		n >>= 1;
	}
	return nRet;
}

/*!***************************************************************************
 @Function			ModelPODDataShred
 @Modified			data		Data elements to modify
 @Input				nCnt		Number of elements
 @Input				nMask		Channel masks
 @Description		Reduce the number of dimensions in 'data' using the channel
					masks in 'nMask'.
*****************************************************************************/
void ModelPODDataShred(CPODData &data, const unsigned int nCnt, const unsigned int nMask)
{
	CPODData		old;
	 VECTOR4f	v;
	unsigned int	i, j, nCh;

	if(!data.pData)
		return;

	old = data;

	// Count the number of output channels
	data.n = BitCount(nMask);
	if(data.n > old.n)
		data.n = old.n;

	// Allocate output memory
	data.nStride = (unsigned int)ModelPODDataStride(data);
	data.pData = new unsigned char[data.nStride * nCnt];

	for(i = 0; i < nCnt; ++i)
	{
		// Read the vector
		DataTypeRead(&v, old.pData + i * old.nStride, old.eType, old.n);

		// Shred the vector
		nCh = 0;
		for(j = 0; j < 4; ++j)
		{
			if(nMask & (1 << j))
			{
				((unsigned int*)&v)[nCh] = ((unsigned int*)&v)[j];
				++nCh;
			}
		}

		for(; nCh < 4; ++nCh)
			((unsigned int*)&v)[nCh] = 0;

		// Write the vector
		DataTypeWrite((char*)data.pData + i * data.nStride, data.eType, data.n * ModelPODDataTypeComponentCount(data.eType), &v);
	}

	delete(old.pData);
}

/*!***************************************************************************
 @Function			InterleaveArray
 @Output
 @Modified
 @Input
 @Description
*****************************************************************************/
static void InterleaveArray(
	char			* const pInterleaved,
	CPODData		&data,
	const int		nNumVertex,
	const size_t	nStride,
	size_t			&nOffset)
{
	if(!data.nStride)
		return;

	for(int i = 0; i < nNumVertex; ++i)
		memcpy(pInterleaved + i * nStride + nOffset, (char*)data.pData + i * data.nStride, data.nStride);
	delete(data.pData);
	data.nStride	= (unsigned int)nStride;
	data.pData		= (unsigned char*)nOffset;
	nOffset += (int)ModelPODDataStride(data);
}

/*!***************************************************************************
 @Function			DeinterleaveArray
 @Output
 @Modified
 @Input
 @Description
*****************************************************************************/
static void DeinterleaveArray(
	CPODData			&data,
	const void			* const pInter,
	const int			nNumVertex)
{
	unsigned int	nSrcStride	= data.nStride;
	unsigned int	nDestStride	= (unsigned int)ModelPODDataStride(data);
	const char		*pSrc		= (char*)pInter + (size_t)data.pData;

	if(!nSrcStride)
		return;

	data.pData = 0;
	SafeAlloc(data.pData, nDestStride * nNumVertex);
	data.nStride	= nDestStride;

	for(int i = 0; i < nNumVertex; ++i)
		memcpy((char*)data.pData + i * nDestStride, pSrc + i * nSrcStride, nDestStride);
}

/*!***************************************************************************
 @Function		ModelPODToggleInterleaved
 @Modified		mesh		Mesh to modify
 @Description	Switches the supplied mesh to or from interleaved data format.
*****************************************************************************/
void ModelPODToggleInterleaved(SPODMesh &mesh)
{
	unsigned int i;

	if(!mesh.nNumVertex)
		return;

	if(mesh.pInterleaved)
	{
		/*
			De-interleave
		*/
		DeinterleaveArray(mesh.sVertex, mesh.pInterleaved, mesh.nNumVertex);
		DeinterleaveArray(mesh.sNormals, mesh.pInterleaved, mesh.nNumVertex);
		DeinterleaveArray(mesh.sTangents, mesh.pInterleaved, mesh.nNumVertex);
		DeinterleaveArray(mesh.sBinormals, mesh.pInterleaved, mesh.nNumVertex);

		for(i = 0; i < mesh.nNumUVW; ++i)
			DeinterleaveArray(mesh.psUVW[i], mesh.pInterleaved, mesh.nNumVertex);

		DeinterleaveArray(mesh.sVtxColours, mesh.pInterleaved, mesh.nNumVertex);
		DeinterleaveArray(mesh.sBoneIdx, mesh.pInterleaved, mesh.nNumVertex);
		DeinterleaveArray(mesh.sBoneWeight, mesh.pInterleaved, mesh.nNumVertex);
		delete(mesh.pInterleaved);
	}
	else
	{
		size_t nStride, nOffset;

		/*
			Interleave
		*/

		// Calculate how much data the interleaved array must store
		nStride = ModelPODDataStride(mesh.sVertex);
		nStride += ModelPODDataStride(mesh.sNormals);
		nStride += ModelPODDataStride(mesh.sTangents);
		nStride += ModelPODDataStride(mesh.sBinormals);

		for(i = 0; i < mesh.nNumUVW; ++i)
			nStride += ModelPODDataStride(mesh.psUVW[i]);

		nStride += ModelPODDataStride(mesh.sVtxColours);
		nStride += ModelPODDataStride(mesh.sBoneIdx);
		nStride += ModelPODDataStride(mesh.sBoneWeight);

		// Allocate interleaved array
		SafeAlloc(mesh.pInterleaved, mesh.nNumVertex * nStride);

		// Interleave the data
		nOffset = 0;
		InterleaveArray((char*)mesh.pInterleaved, mesh.sVertex, mesh.nNumVertex, nStride, nOffset);
		InterleaveArray((char*)mesh.pInterleaved, mesh.sNormals, mesh.nNumVertex, nStride, nOffset);
		InterleaveArray((char*)mesh.pInterleaved, mesh.sTangents, mesh.nNumVertex, nStride, nOffset);
		InterleaveArray((char*)mesh.pInterleaved, mesh.sBinormals, mesh.nNumVertex, nStride, nOffset);
		InterleaveArray((char*)mesh.pInterleaved, mesh.sVtxColours, mesh.nNumVertex, nStride, nOffset);

		for(i = 0; i < mesh.nNumUVW; ++i)
			InterleaveArray((char*)mesh.pInterleaved, mesh.psUVW[i], mesh.nNumVertex, nStride, nOffset);

		InterleaveArray((char*)mesh.pInterleaved, mesh.sBoneIdx, mesh.nNumVertex, nStride, nOffset);
		InterleaveArray((char*)mesh.pInterleaved, mesh.sBoneWeight, mesh.nNumVertex, nStride, nOffset);
	}
}

/*!***************************************************************************
 @Function			ModelPODDeIndex
 @Modified			mesh		Mesh to modify
 @Description		De-indexes the supplied mesh. The mesh must be
					Interleaved before calling this function.
*****************************************************************************/
void ModelPODDeIndex(SPODMesh &mesh)
{
	unsigned char *pNew = 0;

	if(!mesh.pInterleaved || !mesh.nNumVertex)
		return;

	_ASSERT(mesh.nNumVertex && mesh.nNumFaces);

	// Create a new vertex list
	mesh.nNumVertex = ModelPODCountIndices(mesh);
	SafeAlloc(pNew, mesh.sVertex.nStride * mesh.nNumVertex);

	// Deindex the vertices
	for(unsigned int i = 0; i < mesh.nNumVertex; ++i)
		memcpy(pNew + i * mesh.sVertex.nStride, (char*)mesh.pInterleaved + ((unsigned short*)mesh.sFaces.pData)[i] * mesh.sVertex.nStride, mesh.sVertex.nStride);

	// Replace the old vertex list
	delete(mesh.pInterleaved);
	mesh.pInterleaved = pNew;

	// Get rid of the index list
	delete(mesh.sFaces.pData);
	mesh.sFaces.n		= 0;
	mesh.sFaces.nStride	= 0;
}

/*!***************************************************************************
 @Function			ModelPODToggleStrips
 @Modified			mesh		Mesh to modify
 @Description		Converts the supplied mesh to or from strips.
*****************************************************************************/
void ModelPODToggleStrips(SPODMesh &mesh)
{
	CPODData	old;
	size_t	nIdxSize, nTriStride;

	if(!mesh.nNumFaces)
		return;

	_ASSERT(mesh.sFaces.n == 1);
	nIdxSize	= ModelPODDataTypeSize(mesh.sFaces.eType);
	nTriStride	= ModelPODDataStride(mesh.sFaces) * 3;

	old					= mesh.sFaces;
	mesh.sFaces.pData	= 0;
	SafeAlloc(mesh.sFaces.pData, nTriStride * mesh.nNumFaces);

	if(mesh.nNumStrips)
	{
		unsigned int nListIdxCnt, nStripIdxCnt;

		/*
			Convert to list
		*/
		nListIdxCnt		= 0;
		nStripIdxCnt	= 0;
		for(unsigned int i = 0; i < mesh.nNumStrips; ++i)
		{
			for(unsigned int j = 0; j < mesh.pnStripLength[i]; ++j)
			{
				if(j)
				{
					_ASSERT(j == 1); // Because this will surely break with any other number

					memcpy(
						(char*)mesh.sFaces.pData	+ nIdxSize * nListIdxCnt,
						(char*)old.pData			+ nIdxSize * (nStripIdxCnt - 1),
						nIdxSize);
					nListIdxCnt += 1;

					memcpy(
						(char*)mesh.sFaces.pData	+ nIdxSize * nListIdxCnt,
						(char*)old.pData			+ nIdxSize * (nStripIdxCnt - 2),
						nIdxSize);
					nListIdxCnt += 1;

					memcpy(
						(char*)mesh.sFaces.pData	+ nIdxSize * nListIdxCnt,
						(char*)old.pData			+ nIdxSize * nStripIdxCnt,
						nIdxSize);
					nListIdxCnt += 1;

					nStripIdxCnt += 1;
				}
				else
				{
					memcpy(
						(char*)mesh.sFaces.pData	+ nIdxSize * nListIdxCnt,
						(char*)old.pData			+ nIdxSize * nStripIdxCnt,
						nTriStride);

					nStripIdxCnt += 3;
					nListIdxCnt += 3;
				}
			}
		}

		_ASSERT(nListIdxCnt == mesh.nNumFaces*3);
		delete(mesh.pnStripLength);
		mesh.nNumStrips = 0;
	}
	else
	{
		int		nIdxCnt;
		unsigned int n0, n1, n2;
		unsigned int p0, p1, p2;

		/*
			Convert to strips
		*/
		mesh.pnStripLength	= (unsigned int*)calloc(mesh.nNumFaces, sizeof(*mesh.pnStripLength));
		mesh.nNumStrips		= 0;
		nIdxCnt				= 0;

		n0 = 0;
		n1 = 0;
		n2 = 0;

		for(unsigned int i = 0; i < mesh.nNumFaces; ++i)
		{
			p0 = n0;
			p1 = n1;
			p2 = n2;

			DataTypeRead(&n0, (char*)old.pData + (3 * i + 0) * old.nStride, old.eType);
			DataTypeRead(&n1, (char*)old.pData + (3 * i + 1) * old.nStride, old.eType);
			DataTypeRead(&n2, (char*)old.pData + (3 * i + 2) * old.nStride, old.eType);

			if(mesh.pnStripLength[mesh.nNumStrips])
			{
				if(mesh.pnStripLength[mesh.nNumStrips] & 0x01)
				{
					if(p1 == n1 && p2 == n0)
					{
						DataTypeWrite((char*)mesh.sFaces.pData + nIdxCnt * mesh.sFaces.nStride, mesh.sFaces.eType, n2);
						++nIdxCnt;
						mesh.pnStripLength[mesh.nNumStrips] += 1;
						continue;
					}
				}
				else
				{
					if(p2 == n1 && p0 == n0)
					{
						DataTypeWrite((char*)mesh.sFaces.pData + nIdxCnt * mesh.sFaces.nStride, mesh.sFaces.eType, n2);
						++nIdxCnt;
						mesh.pnStripLength[mesh.nNumStrips] += 1;
						continue;
					}
				}

				++mesh.nNumStrips;
			}

			/*
				Start of strip, copy entire triangle
			*/
			DataTypeWrite((char*)mesh.sFaces.pData + nIdxCnt * mesh.sFaces.nStride, mesh.sFaces.eType, n0);
			++nIdxCnt;
			DataTypeWrite((char*)mesh.sFaces.pData + nIdxCnt * mesh.sFaces.nStride, mesh.sFaces.eType, n1);
			++nIdxCnt;
			DataTypeWrite((char*)mesh.sFaces.pData + nIdxCnt * mesh.sFaces.nStride, mesh.sFaces.eType, n2);
			++nIdxCnt;

			mesh.pnStripLength[mesh.nNumStrips] += 1;
		}

		if(mesh.pnStripLength[mesh.nNumStrips])
			++mesh.nNumStrips;

		SafeRealloc(mesh.sFaces.pData, nIdxCnt * nIdxSize);
		mesh.pnStripLength	= (unsigned int*)reallocEM(mesh.pnStripLength, sizeof(mesh.pnStripLength), sizeof(*mesh.pnStripLength) * mesh.nNumStrips);
	}

	delete(old.pData);
}

/*!***************************************************************************
 @Function		ModelPODCountIndices
 @Input			mesh		Mesh
 @Return		Number of indices used by mesh
 @Description	Counts the number of indices of a mesh
*****************************************************************************/
unsigned int ModelPODCountIndices(const SPODMesh &mesh)
{
	if(mesh.nNumStrips)
	{
		unsigned int i, n = 0;

		for(i = 0; i < mesh.nNumStrips; ++i)
			n += mesh.pnStripLength[i] + 2;

		return n;
	}

	return mesh.nNumFaces * 3;
}

static void FloatToFixed(int * const pn, const float * const pf, unsigned int n)
{
	if(!pn || !pf) return;
	while(n)
	{
		--n;
		pn[n] = (int)(pf[n] * (float)(1<<16));
	}
}
static void FixedToFloat(float * const pf, const int * const pn, unsigned int n)
{
	if(!pn || !pf) return;
	while(n)
	{
		--n;
		pf[n] = (float)pn[n] / (float)(1<<16);
	}
}

/*!***************************************************************************
 @Function		ModelPODToggleFixedPoint
 @Modified		s		Scene to modify
 @Description	Switch all non-vertex data between fixed-point and
				floating-point.
*****************************************************************************/
void ModelPODToggleFixedPoint(SPODScene &s)
{
	unsigned int i;

	if(s.nFlags & MODELPODSF_FIXED)
	{
		/*
			Convert to floating-point
		*/
		for(i = 0; i < s.nNumCamera; ++i)
		{
			FixedToFloat((float*)&s.pCamera[i].fFOV, (int*)&s.pCamera[i].fFOV, 1);
			FixedToFloat((float*)&s.pCamera[i].fFar, (int*)&s.pCamera[i].fFar, 1);
			FixedToFloat((float*)&s.pCamera[i].fNear, (int*)&s.pCamera[i].fNear, 1);
			FixedToFloat((float*)s.pCamera[i].pfAnimFOV, (int*)s.pCamera[i].pfAnimFOV, s.nNumFrame);
		}

		for(i = 0; i < s.nNumLight; ++i)
		{
			FixedToFloat((float*)&s.pLight[i].pfColour, (int*)&s.pLight[i].pfColour, 3);
		}

		for(i = 0; i < s.nNumNode; ++i)
		{
			FixedToFloat((float*)s.pNode[i].pfPosition,		(int*)s.pNode[i].pfPosition,	3);
			FixedToFloat((float*)s.pNode[i].pfRotation,		(int*)s.pNode[i].pfRotation,	4);
			FixedToFloat((float*)s.pNode[i].pfScale,		(int*)s.pNode[i].pfScale,		3);
			FixedToFloat((float*)s.pNode[i].pfAnimPosition,	(int*)s.pNode[i].pfAnimPosition,	3 * s.nNumFrame);
			FixedToFloat((float*)s.pNode[i].pfAnimRotation,	(int*)s.pNode[i].pfAnimRotation,	4 * s.nNumFrame);
			FixedToFloat((float*)s.pNode[i].pfAnimScale,	(int*)s.pNode[i].pfAnimScale,		7 * s.nNumFrame);
		}

		for(i = 0; i < s.nNumMaterial; ++i)
		{
			FixedToFloat((float*)&s.pMaterial[i].fMatOpacity,	(int*)&s.pMaterial[i].fMatOpacity,		1);
			FixedToFloat((float*)s.pMaterial[i].pfMatAmbient,	(int*)s.pMaterial[i].pfMatAmbient,		3);
			FixedToFloat((float*)s.pMaterial[i].pfMatDiffuse,	(int*)s.pMaterial[i].pfMatDiffuse,		3);
			FixedToFloat((float*)s.pMaterial[i].pfMatSpecular,	(int*)s.pMaterial[i].pfMatSpecular,		3);
			FixedToFloat((float*)&s.pMaterial[i].fMatShininess,	(int*)&s.pMaterial[i].fMatShininess,	1);
		}

		FixedToFloat((float*)s.pfColourBackground,	(int*)s.pfColourBackground,	3);
		FixedToFloat((float*)s.pfColourAmbient,		(int*)s.pfColourAmbient,	3);
	}
	else
	{
		/*
			Convert to Fixed-point
		*/
		for(i = 0; i < s.nNumCamera; ++i)
		{
			FloatToFixed((int*)&s.pCamera[i].fFOV, (float*)&s.pCamera[i].fFOV, 1);
			FloatToFixed((int*)&s.pCamera[i].fFar, (float*)&s.pCamera[i].fFar, 1);
			FloatToFixed((int*)&s.pCamera[i].fNear, (float*)&s.pCamera[i].fNear, 1);
			FloatToFixed((int*)s.pCamera[i].pfAnimFOV, (float*)s.pCamera[i].pfAnimFOV, s.nNumFrame);
		}

		for(i = 0; i < s.nNumLight; ++i)
		{
			FloatToFixed((int*)&s.pLight[i].pfColour, (float*)&s.pLight[i].pfColour, 3);
		}

		for(i = 0; i < s.nNumNode; ++i)
		{
			FloatToFixed((int*)s.pNode[i].pfPosition,		(float*)s.pNode[i].pfPosition,	3);
			FloatToFixed((int*)s.pNode[i].pfRotation,		(float*)s.pNode[i].pfRotation,	4);
			FloatToFixed((int*)s.pNode[i].pfScale,		(float*)s.pNode[i].pfScale,		3);
			FloatToFixed((int*)s.pNode[i].pfAnimPosition,	(float*)s.pNode[i].pfAnimPosition,	3 * s.nNumFrame);
			FloatToFixed((int*)s.pNode[i].pfAnimRotation,	(float*)s.pNode[i].pfAnimRotation,	4 * s.nNumFrame);
			FloatToFixed((int*)s.pNode[i].pfAnimScale,	(float*)s.pNode[i].pfAnimScale,		7 * s.nNumFrame);
		}

		for(i = 0; i < s.nNumMaterial; ++i)
		{
			FloatToFixed((int*)&s.pMaterial[i].fMatOpacity,	(float*)&s.pMaterial[i].fMatOpacity,		1);
			FloatToFixed((int*)s.pMaterial[i].pfMatAmbient,	(float*)s.pMaterial[i].pfMatAmbient,		3);
			FloatToFixed((int*)s.pMaterial[i].pfMatDiffuse,	(float*)s.pMaterial[i].pfMatDiffuse,		3);
			FloatToFixed((int*)s.pMaterial[i].pfMatSpecular,	(float*)s.pMaterial[i].pfMatSpecular,		3);
			FloatToFixed((int*)&s.pMaterial[i].fMatShininess,	(float*)&s.pMaterial[i].fMatShininess,	1);
		}

		FloatToFixed((int*)s.pfColourBackground,	(float*)s.pfColourBackground,	3);
		FloatToFixed((int*)s.pfColourAmbient,		(float*)s.pfColourAmbient,	3);
	}

	// Done
	s.nFlags ^= MODELPODSF_FIXED;
}

/*****************************************************************************
 End of file (ModelPOD.cpp)
*****************************************************************************/
