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
#ifndef _MODELPOD_H_
#define _MODELPOD_H_

#include "Mathematics.h"
#include "Geometry.h"
#include "Macros.h"
#include "Memory.h"

#include "BoneBatch.h"

/****************************************************************************
** Defines
****************************************************************************/
#define MODELPOD_VERSION	("AB.POD.2.0")

// MODELPOD Scene Flags
#define MODELPODSF_FIXED	(0x00000001)	// Fixed-point 16.16 data (otherwise float)

/****************************************************************************
** Enumerations
****************************************************************************/
enum EPODLight {
	ePODPoint,
	ePODDirectional
};

/****************************************************************************
** Structures
****************************************************************************/
class CPODData {
public:
	EDataType	eType;		/*!< Type of data stored */
	unsigned int	n;			/*!< Number of values per vertex */
	unsigned int	nStride;	/*!< Distance in bytes from one array entry to the next */
	unsigned char	*pData;		/*!< Actual data (array of values); if mesh is interleaved, this is an OFFSET from pInterleaved */
};

struct SPODCamera {
	int			nIdxTarget;			/*!< Index of the target object */
	VERTTYPE	fFOV;				/*!< Field of view */
	VERTTYPE	fFar;				/*!< Far clip plane */
	VERTTYPE	fNear;				/*!< Near clip plane */
	VERTTYPE	*pfAnimFOV;			/*!< 1 VERTTYPE per frame of animation. */
};

struct SPODLight {
	int			nIdxTarget;		/*!< Index of the target object */
	VERTTYPE	pfColour[3];	/*!< Light colour (0.0f -> 1.0f for each channel) */
	EPODLight	eType;			/*!< Light type (point, directional etc.) */
};

struct SPODMesh {
	unsigned int		nNumVertex;		/*!< Number of vertices in the mesh */
	unsigned int		nNumFaces;		/*!< Number of triangles in the mesh */
	unsigned int		nNumUVW;		/*!< Number of texture coordinate channels per vertex */
	CPODData			sFaces;			/*!< List of triangle indices */
	unsigned int		*pnStripLength;	/*!< If mesh is stripped: number of tris per strip. */
	unsigned int		nNumStrips;		/*!< If mesh is stripped: number of strips, length of pnStripLength array. */
	CPODData			sVertex;		/*!< List of vertices (x0, y0, z0, x1, y1, z1, x2, etc...) */
	CPODData			sNormals;		/*!< List of vertex normals (Nx0, Ny0, Nz0, Nx1, Ny1, Nz1, Nx2, etc...) */
	CPODData			sTangents;		/*!< List of vertex tangents (Tx0, Ty0, Tz0, Tx1, Ty1, Tz1, Tx2, etc...) */
	CPODData			sBinormals;		/*!< List of vertex binormals (Bx0, By0, Bz0, Bx1, By1, Bz1, Bx2, etc...) */
	CPODData			*psUVW;			/*!< List of UVW coordinate sets; size of array given by 'nNumUVW' */
	CPODData			sVtxColours;	/*!< A colour per vertex */
	CPODData			sBoneIdx;		/*!< nNumBones*nNumVertex ints (Vtx0Idx0, Vtx0Idx1, ... Vtx1Idx0, Vtx1Idx1, ...) */
	CPODData			sBoneWeight;	/*!< nNumBones*nNumVertex floats (Vtx0Wt0, Vtx0Wt1, ... Vtx1Wt0, Vtx1Wt1, ...) */

	unsigned char		*pInterleaved;	/*!< Interleaved vertex data */

	CBoneBatches	sBoneBatches;	/*!< Bone tables */
};

struct SPODNode {
	int			nIdx;				/*!< Index into mesh, light or camera array, depending on which object list contains this Node */
	char		*pszName;			/*!< Name of object */
	int			nIdxMaterial;		/*!< Index of material used on this mesh */

	int			nIdxParent;			/*!< Index into MeshInstance array; recursively apply ancestor's transforms after this instance's. */
	VERTTYPE	pfPosition[3];		/*!< Position in World coordinates */
	VERTTYPE	pfRotation[4];		/*!< Rotation in World coordinates */
	VERTTYPE	pfScale[3];			/*!< Scale in World coordinates */

	VERTTYPE	*pfAnimPosition;	/*!< 3 floats per frame of animation. */
	VERTTYPE	*pfAnimRotation;	/*!< 4 floats per frame of animation. */
	VERTTYPE	*pfAnimScale;		/*!< 7 floats per frame of animation. */
};

struct SPODTexture {
	char	*pszName;			/*!< File-name of texture */
};

struct SPODMaterial {
	char		*pszName;			/*!< Name of material */
	int			nIdxTexDiffuse;		/*!< Idx into textures for diffuse texture */
	VERTTYPE	fMatOpacity;		/*!< Material opacity (used with vertex alpha ?) */
	VERTTYPE	pfMatAmbient[3];	/*!< Ambient RGB value */
	VERTTYPE	pfMatDiffuse[3];	/*!< Diffuse RGB value */
	VERTTYPE	pfMatSpecular[3];	/*!< Specular RGB value */
	VERTTYPE	fMatShininess;		/*!< Material shininess */
	char		*pszEffectFile;		/*!< Name of effect file */
	char		*pszEffectName;		/*!< Name of effect in the effect file */
};

struct SPODScene 
{
	VERTTYPE	pfColourBackground[3];		/*!< Background colour */
	VERTTYPE	pfColourAmbient[3];			/*!< Background colour */

	unsigned int	nNumCamera;				/*!< The length of the array pCamera */
	SPODCamera		*pCamera;				/*!< Camera nodes array */

	unsigned int	nNumLight;				/*!< The length of the array pLight */
	SPODLight		*pLight;				/*!< Light nodes array */

	unsigned int	nNumMesh;				/*!< The length of the array pMesh */
	SPODMesh		*pMesh;					/*!< Mesh array. Meshes may be instanced several times in a scene; i.e. multiple Nodes may reference any given mesh. */

	unsigned int	nNumNode;		/*!< Number of items in the array pNode */
	unsigned int	nNumMeshNode;	/*!< Number of items in the array pNode which are objects */
	SPODNode		*pNode;			/*!< Node array. Sorted as such: objects, lights, cameras, Everything Else (bones, helpers etc) */

	unsigned int	nNumTexture;
	SPODTexture		*pTexture;

	unsigned int	nNumMaterial;
	SPODMaterial	*pMaterial;

	unsigned int	nNumFrame;		/*!< Number of frames of animation */
	unsigned int	nFlags;			/*!< MODELPODSF_* bit-flags */
};

struct SPODImpl;	// Internal implementation data

class CPODScene : public SPODScene{
public:
	/*!***************************************************************************
	 @Function		Constructor
	 @Description	Constructor for CPODScene class
	*****************************************************************************/
	CPODScene();

	/*!***************************************************************************
	 @Function		Destructor
	 @Description	Destructor for CPODScene class
	*****************************************************************************/
	~CPODScene();

	/*!***************************************************************************
	 @Function		ReadFromFile
	 @Input			pszFileName		Filename to load
	 @Output		pszExpOpt		String in which to place exporter options
	 @Input			count			Maximum number of characters to store.
	 @Return		TRUE if successful, FALSE if not
	 @Description	Loads the specified ".POD" file; returns the scene in
					pScene. This structure must later be destroyed with
					ModelPODDestroy() to prevent memory leaks.
					".POD" files are exported from 3D Studio MAX using a
					PowerVR plugin.
					If pszExpOpt is NULL, the scene is loaded; otherwise the
					scene is not loaded and pszExpOpt is filled in.
	*****************************************************************************/
	bool ReadFromFile(
		const char		* const pszFileName,
		char			* const pszExpOpt = NULL,
		const size_t	count = 0);

	/*!***************************************************************************
	 @Function		ReadFromMemory
	 @Input			scene			Scene data from the header file
	 @Return		TRUE if successful, FALSE if not
	 @Description	Sets the scene data from the supplied data structure. Use
					when loading from .H files.
	*****************************************************************************/
	bool ReadFromMemory(
		const SPODScene &scene);

#ifdef WIN32
	/*!***************************************************************************
	 @Function		ReadFromResource
	 @Input			pszName			Name of the resource to load from
	 @Return		TRUE if successful, FALSE if not
	 @Description	Loads the specified ".POD" file; returns the scene in
					pScene. This structure must later be destroyed with
					ModelPODDestroy() to prevent memory leaks.
					".POD" files are exported from 3D Studio MAX using a
					PowerVR plugin.
	*****************************************************************************/
	bool ReadFromResource(
		const TCHAR * const pszName);
#endif

	/*!***********************************************************************
	 @Function		InitImpl
	 @Description	Used by the Read*() fns to initialise implementation
					details. Should also be called by applications which
					manually build data in the POD structures for rendering;
					in this case call it after the data has been created.
					Otherwise, do not call this function.
	*************************************************************************/
	bool InitImpl();

	/*!***********************************************************************
	 @Function		FlushCache
	 @Description	Clears the matrix cache; use this if necessary when you
					edit the position or animation of a node.
	*************************************************************************/
	void FlushCache();
	/*!***************************************************************************
	 @Function		Destroy
	 @Description	Frees the memory allocated to store the scene in pScene.
	*****************************************************************************/
	void Destroy();

	/*!***************************************************************************
	 @Function		SetFrame
	 @Input			fFrame			Frame number
	 @Description	Set the animation frame for which subsequent Get*() calls
					should return data.
	*****************************************************************************/
	void SetFrame(
		const VERTTYPE fFrame);

	/*!***************************************************************************
	 @Function		GetRotationMatrix
	 @Output		mOut			Rotation matrix
	 @Input			node			Node to get the rotation matrix from
	 @Description	Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
	*****************************************************************************/
	void GetRotationMatrix(
		 MATRIX		&mOut,
		const SPODNode	&node) const;

	/*!***************************************************************************
	 @Function		GetScalingMatrix
	 @Output		mOut			Scaling matrix
	 @Input			node			Node to get the rotation matrix from
	 @Description	Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
	*****************************************************************************/
	void GetScalingMatrix(
		 MATRIX		&mOut,
		const SPODNode	&node) const;

	/*!***************************************************************************
	 @Function		GetTranslation
	 @Output		V				Translation vector
	 @Input			node			Node to get the translation vector from
	 @Description	Generates the translation vector for the given Mesh
					Instance. Uses animation data.
	*****************************************************************************/
	void GetTranslation(
		 VECTOR3		&V,
		const SPODNode	&node) const;

	/*!***************************************************************************
	 @Function		GetTranslationMatrix
	 @Output		mOut			Translation matrix
	 @Input			node			Node to get the translation matrix from
	 @Description	Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
	*****************************************************************************/
	void GetTranslationMatrix(
		 MATRIX		&mOut,
		const SPODNode	&node) const;

	/*!***************************************************************************
	 @Function		GetWorldMatrixNoCache
	 @Output		mOut			World matrix
	 @Input			node			Node to get the world matrix from
	 @Description	Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
	*****************************************************************************/
	void GetWorldMatrixNoCache(
		 MATRIX		&mOut,
		const SPODNode	&node) const;

	/*!***************************************************************************
	 @Function		GetWorldMatrix
	 @Output		mOut			World matrix
	 @Input			node			Node to get the world matrix from
	 @Description	Generates the world matrix for the given Mesh Instance;
					applies the parent's transform too. Uses animation data.
	*****************************************************************************/
	void GetWorldMatrix(
		 MATRIX		&mOut,
		const SPODNode	&node) const;

	/*!***************************************************************************
	 @Function		GetBoneWorldMatrix
	 @Output		mOut			Bone world matrix
	 @Input			NodeMesh		Mesh to take the bone matrix from
	 @Input			NodeBone		Bone to take the matrix from
	 @Description	Generates the world matrix for the given bone.
	*****************************************************************************/
	void GetBoneWorldMatrix(
		 MATRIX		&mOut,
		const SPODNode	&NodeMesh,
		const SPODNode	&NodeBone);

	/*!***************************************************************************
	 @Function		GetCamera
	 @Output		vFrom			Position of the camera
	 @Output		vTo				Target of the camera
	 @Output		vUp				Up direction of the camera
	 @Input			nIdx			Camera number
	 @Return		Camera horizontal FOV
	 @Description	Calculate the From, To and Up vectors for the given
					camera. Uses animation data.
					Note that even if the camera has a target, *pvTo is not
					the position of that target. *pvTo is a position in the
					correct direction of the target, one unit away from the
					camera.
	*****************************************************************************/
	VERTTYPE GetCamera(
		 VECTOR3			&vFrom,
		 VECTOR3			&vTo,
		 VECTOR3			&vUp,
		const unsigned int	nIdx) const;

	/*!***************************************************************************
	 @Function		GetCameraPos
	 @Output		vFrom			Position of the camera
	 @Output		vTo				Target of the camera
	 @Input			nIdx			Camera number
	 @Return		Camera horizontal FOV
	 @Description	Calculate the position of the camera and its target. Uses
					animation data.
					If the queried camera does not have a target, *pvTo is
					not changed.
	*****************************************************************************/
	VERTTYPE GetCameraPos(
		 VECTOR3			&vFrom,
		 VECTOR3			&vTo,
		const unsigned int	nIdx) const;

	/*!***************************************************************************
	 @Function		GetLight
	 @Output		vPos			Position of the light
	 @Output		vDir			Direction of the light
	 @Input			nIdx			Light number
	 @Description	Calculate the position and direction of the given Light.
					Uses animation data.
	*****************************************************************************/
	void GetLight(
		 VECTOR3			&vPos,
		 VECTOR3			&vDir,
		const unsigned int	nIdx) const;

	/*!***************************************************************************
	 @Function		CreateSkinIdxWeight
	 @Output		pIdx				Four bytes containing matrix indices for vertex (0..255) (D3D: use UBYTE4)
	 @Output		pWeight				Four bytes containing blend weights for vertex (0.0 .. 1.0) (D3D: use D3DCOLOR)
	 @Input			nVertexBones		Number of bones this vertex uses
	 @Input			pnBoneIdx			Pointer to 'nVertexBones' indices
	 @Input			pfBoneWeight		Pointer to 'nVertexBones' blend weights
	 @Description	Creates the matrix indices and blend weights for a boned
					vertex. Call once per vertex of a boned mesh.
	*****************************************************************************/
	bool CreateSkinIdxWeight(
		char			* const pIdx,
		char			* const pWeight,
		const int		nVertexBones,
		const int		* const pnBoneIdx,
		const VERTTYPE	* const pfBoneWeight);

	/*!***************************************************************************
	 @Function		SavePOD
	 @Input			pszFilename		Filename to save to
	 @Input			pszExpOpt		A string containing the options used by the exporter
	 @Description	Save a binary POD file (.POD).
	*****************************************************************************/
	bool SavePOD(const char * const pszFilename, const char * const pszExpOpt = 0);

	/*!***********************************************************************
	 @Function		SaveH
	 @Input			pszFilename		Filename to save to
	 @Input			pszExpOpt		A string containing the options used by the exporter
	 @Description	Save a header file (.H).
	*************************************************************************/
	bool SaveH(const char * const pszFilename, const char * const pszExpOpt = 0);

private:
	SPODImpl	*m_pImpl;	/*!< Internal implementation data */
	
//private:
//    DECLARE_HEAP;

};

/****************************************************************************
** Declarations
****************************************************************************/

/*!***************************************************************************
 @Function		 ModelPODDataTypeSize
 @Input			type		Type to get the size of
 @Return		Size of the data element
 @Description	Returns the size of each data element.
*****************************************************************************/
size_t ModelPODDataTypeSize(const EDataType type);

/*!***************************************************************************
 @Function		ModelPODDataTypeComponentCount
 @Input			type		Type to get the number of components from
 @Return		number of components in the data element
 @Description	Returns the number of components in a data element.
*****************************************************************************/
size_t ModelPODDataTypeComponentCount(const EDataType type);

/*!***************************************************************************
 @Function		ModelPODDataStride
 @Input			data		Data elements
 @Return		Size of the vector elements
 @Description	Returns the size of the vector of data elements.
*****************************************************************************/
size_t ModelPODDataStride(const CPODData &data);

/*!***************************************************************************
 @Function		ModelPODDataConvert
 @Modified		data		Data elements to convert
 @Input			eNewType	New type of elements
 @Input			nCnt		Number of elements
 @Description	Convert the format of the array of vectors.
*****************************************************************************/
void ModelPODDataConvert(CPODData &data, const unsigned int nCnt, const EDataType eNewType);

/*!***************************************************************************
 @Function		ModelPODDataShred
 @Modified		data		Data elements to modify
 @Input			nCnt		Number of elements
 @Input			nMask		Channel masks
 @Description	Reduce the number of dimensions in 'data' using the channel
				masks in 'nMask'.
*****************************************************************************/
void ModelPODDataShred(CPODData &data, const unsigned int nCnt, const unsigned int nMask);

/*!***************************************************************************
 @Function		ModelPODToggleInterleaved
 @Modified		mesh		Mesh to modify
 @Description	Switches the supplied mesh to or from interleaved data format.
*****************************************************************************/
void ModelPODToggleInterleaved(SPODMesh &mesh);

/*!***************************************************************************
 @Function		ModelPODDeIndex
 @Modified		mesh		Mesh to modify
 @Description	De-indexes the supplied mesh. The mesh must be
				Interleaved before calling this function.
*****************************************************************************/
void ModelPODDeIndex(SPODMesh &mesh);

/*!***************************************************************************
 @Function		ModelPODToggleStrips
 @Modified		mesh		Mesh to modify
 @Description	Converts the supplied mesh to or from strips.
*****************************************************************************/
void ModelPODToggleStrips(SPODMesh &mesh);

/*!***************************************************************************
 @Function		ModelPODCountIndices
 @Input			mesh		Mesh
 @Return		Number of indices used by mesh
 @Description	Counts the number of indices of a mesh
*****************************************************************************/
unsigned int ModelPODCountIndices(const SPODMesh &mesh);

/*!***************************************************************************
 @Function		ModelPODToggleFixedPoint
 @Modified		s		Scene to modify
 @Description	Switch all non-vertex data between fixed-point and
				floating-point.
*****************************************************************************/
void ModelPODToggleFixedPoint(SPODScene &s);


#endif