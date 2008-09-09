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
// Description: Vector, Matrix and quaternion functions for floating and fixed
//               point math. The general matrix format used is directly compatible
//               with, for example, both DirectX and OpenGL. For the reasons why,
//               read this:
//               http://research.microsoft.com/~hollasch/cgindex/math/matrix/column-vec.html
//

#ifndef MATRIX_H_
#define MATRIX_H_

#include "Vector.h"
#include "Quaternion.h"

#define MAT00 0
#define MAT01 1
#define MAT02 2
#define MAT03 3
#define MAT10 4
#define MAT11 5
#define MAT12 6
#define MAT13 7
#define MAT20 8
#define MAT21 9
#define MAT22 10
#define MAT23 11
#define MAT30 12
#define MAT31 13
#define MAT32 14
#define MAT33 15

/*
struct Matrix4x4 
{ 
    // The elements of the 4x4 matrix are stored in 
    // column-major order (see "OpenGL Programming Guide", 
    // 3rd edition, pp 106, glLoadMatrix). 
    float   _11, _21, _31, _41; 
    float   _12, _22, _32, _42; 
    float   _13, _23, _33, _43; 
    float   _14, _24, _34, _44; 
}; 
*/

#define _11 0
#define _12 1
#define _13 2
#define _14 3
#define _21 4
#define _22 5
#define _23 6
#define _24 7
#define _31 8
#define _32 9
#define _33 10
#define _34 11
#define _41 12
#define _42 13
#define _43 14
#define _44 15



//
// 4x4 floating point matrix
//
class MATRIXf
{
public:
    float* operator [] ( const int Row )
	{
		return &f[Row<<2];
	}
	float f[16];	/*!< Array of float */
};

//
// 4x4 fixed point matrix
//
class MATRIXx
{
public:
    int* operator [] ( const int Row )
	{
		return &f[Row<<2];
	}
	int f[16];
};

#ifdef FIXEDPOINTENABLE
typedef MATRIXx			MATRIX;
#define MatrixIdentity					MatrixIdentityX
#define MatrixMultiply					MatrixMultiplyX
#define MatrixTranslation				MatrixTranslationX
#define MatrixScaling					MatrixScalingX
#define MatrixRotationX					MatrixRotationXX
#define MatrixRotationY					MatrixRotationYX
#define MatrixRotationZ					MatrixRotationZX
#define MatrixTranspose					MatrixTransposeX
#define MatrixInverse					MatrixInverseX
#define MatrixInverseEx					MatrixInverseExX
#define MatrixLookAtLH					MatrixLookAtLHX
#define MatrixLookAtRH					MatrixLookAtRHX
#define MatrixPerspectiveFovLH			MatrixPerspectiveFovLHX
#define MatrixPerspectiveFovRH			MatrixPerspectiveFovRHX
#define MatrixOrthoLH					MatrixOrthoLHX
#define MatrixOrthoRH					MatrixOrthoRHX
#define MatrixVec3Lerp					MatrixVec3LerpX
#define MatrixVec3DotProduct			MatrixVec3DotProductX
#define MatrixVec3CrossProduct			MatrixVec3CrossProductX
#define MatrixVec3Normalize				MatrixVec3NormalizeX
#define MatrixVec4Normalize				MatrixVec4NormalizeX
#define MatrixVec3Length				MatrixVec3LengthX
#define MatrixQuaternionIdentity		MatrixQuaternionIdentityX
#define MatrixQuaternionRotationAxis	MatrixQuaternionRotationAxisX
#define MatrixQuaternionToAxisAngle		MatrixQuaternionToAxisAngleX
#define MatrixQuaternionSlerp			MatrixQuaternionSlerpX
#define MatrixQuaternionNormalize		MatrixQuaternionNormalizeX
#define MatrixRotationQuaternion		MatrixRotationQuaternionX
#define MatrixQuaternionMultiply		MatrixQuaternionMultiplyX
#define MatrixLinearEqSolve				MatrixLinearEqSolveX
#else
typedef MATRIXf			MATRIX;
#define MatrixIdentity					MatrixIdentityF
#define MatrixMultiply					MatrixMultiplyF
#define MatrixTranslation				MatrixTranslationF
#define MatrixScaling					MatrixScalingF
#define MatrixRotationX					MatrixRotationXF
#define MatrixRotationY					MatrixRotationYF
#define MatrixRotationZ					MatrixRotationZF
#define MatrixTranspose					MatrixTransposeF
#define MatrixInverse					MatrixInverseF
#define MatrixInverseEx					MatrixInverseExF
#define MatrixLookAtLH					MatrixLookAtLHF
#define MatrixLookAtRH					MatrixLookAtRHF
#define MatrixPerspectiveFovLH			MatrixPerspectiveFovLHF
#define MatrixPerspectiveFovRH			MatrixPerspectiveFovRHF
#define MatrixOrthoLH					MatrixOrthoLHF
#define MatrixOrthoRH					MatrixOrthoRHF
#define MatrixVec3Lerp					MatrixVec3LerpF
#define MatrixVec3DotProduct			MatrixVec3DotProductF
#define MatrixVec3CrossProduct			MatrixVec3CrossProductF
#define MatrixVec3Normalize				MatrixVec3NormalizeF
#define MatrixVec4Normalize				MatrixVec4NormalizeF
#define MatrixVec3Length				MatrixVec3LengthF
#define MatrixQuaternionIdentity		MatrixQuaternionIdentityF
#define MatrixQuaternionRotationAxis	MatrixQuaternionRotationAxisF
#define MatrixQuaternionToAxisAngle		MatrixQuaternionToAxisAngleF
#define MatrixQuaternionSlerp			MatrixQuaternionSlerpF
#define MatrixQuaternionNormalize		MatrixQuaternionNormalizeF
#define MatrixRotationQuaternion		MatrixRotationQuaternionF
#define MatrixQuaternionMultiply		MatrixQuaternionMultiplyF
#define MatrixLinearEqSolve				MatrixLinearEqSolveF
#endif

//
// Reset matrix to identity
// outputs the identity matrix
//
void MatrixIdentityF(MATRIXf &mOut);

//
// Resets matrix to identity
// outpus the identity matrix
//
void MatrixIdentityX(MATRIXx &mOut);

//
// Multiply mA by mB and assign the result to mOut
// (mOut = p1 * p2). A copy of the result matrix is done in
// the function because mOut can be a parameter mA or mB.
//
void MatrixMultiplyF(
	MATRIXf			&mOut,
	const MATRIXf	&mA,
	const MATRIXf	&mB);
	
//
// Multiply mA by mB and assign the result to mOut
// (mOut = p1 * p2). A copy of the result matrix is done in
// the function because mOut can be a parameter mA or mB.
// The fixed-point shift could be performed after adding
// all four intermediate results together however this might
// cause some overflow issues.
//
void MatrixMultiplyX(
	MATRIXx			&mOut,
	const MATRIXx	&mA,
	const MATRIXx	&mB);

//
// Build a translation matrix mOut using fX, fY and fZ.
//
void MatrixTranslationF(
	MATRIXf	&mOut,
	const float	fX,
	const float	fY,
	const float	fZ);
	
//
// Build a translation matrix mOut using fX, fY and fZ.
//
void MatrixTranslationX(
	MATRIXx	&mOut,
	const int	fX,
	const int	fY,
	const int	fZ);

//
// Build a scale matrix mOut using fX, fY and fZ.
//
void MatrixScalingF(
	MATRIXf	&mOut,
	const float fX,
	const float fY,
	const float fZ);

//
// Build a scale matrix mOut using fX, fY and fZ.
//
void MatrixScalingX(
	MATRIXx	&mOut,
	const int	fX,
	const int	fY,
	const int	fZ);


//
// Create an X rotation matrix mOut.
//
void MatrixRotationXF(
	MATRIXf	&mOut,
	const float fAngle);

// 
// Create an X rotation matrix mOut.
//
void MatrixRotationXX(
	MATRIXx	&mOut,
	const int	fAngle);

//
// Create an Y rotation matrix mOut.
//
void MatrixRotationYF(
	MATRIXf	&mOut,
	const float fAngle);

// 
// Create an Y rotation matrix mOut.
//
void MatrixRotationYX(
	MATRIXx	&mOut,
	const int	fAngle);

//
// Create an Z rotation matrix mOut.
//
void MatrixRotationZF(
	MATRIXf	&mOut,
	const float fAngle);

// 
// Create an Z rotation matrix mOut.
//
void MatrixRotationZX(
	MATRIXx	&mOut,
	const int	fAngle);


//
// Compute the transpose matrix of mIn.
//
void MatrixTransposeF(
	MATRIXf			&mOut,
	const MATRIXf	&mIn);

//
// Compute the transpose matrix of mIn.
//
void MatrixTransposeX(
	MATRIXx			&mOut,
	const MATRIXx	&mIn);

//
// Compute the inverse matrix of mIn.
//	The matrix must be of the form :
//	A 0
//	C 1
// Where A is a 3x3 matrix and C is a 1x3 matrix.
//
void MatrixInverseF(
	MATRIXf			&mOut,
	const MATRIXf	&mIn);

//
// Compute the inverse matrix of mIn.
//	The matrix must be of the form :
//	A 0
//	C 1
// Where A is a 3x3 matrix and C is a 1x3 matrix.
//
void MatrixInverseX(
	MATRIXx			&mOut,
	const MATRIXx	&mIn);

//
// Compute the inverse matrix of mIn.
// Uses a linear equation solver and the knowledge that M.M^-1=I.
// Use this fn to calculate the inverse of matrices that
// MatrixInverse() cannot.
//
void MatrixInverseExF(
	MATRIXf			&mOut,
	const MATRIXf	&mIn);

//
// Compute the inverse matrix of mIn.
// Uses a linear equation solver and the knowledge that M.M^-1=I.
// Use this fn to calculate the inverse of matrices that
// MatrixInverse() cannot.
//
void MatrixInverseExX(
	MATRIXx			&mOut,
	const MATRIXx	&mIn);

//
// Create a look-at view matrix.
//
void MatrixLookAtLHF(
	MATRIXf			&mOut,
	const VECTOR3f	&vEye,
	const VECTOR3f	&vAt,
	const VECTOR3f	&vUp);

//
// Create a look-at view matrix.
//
void MatrixLookAtLHX(
	MATRIXx			&mOut,
	const VECTOR3x	&vEye,
	const VECTOR3x	&vAt,
	const VECTOR3x	&vUp);

// 
// Create a look-at view matrix.
//
void MatrixLookAtRHF(
	MATRIXf			&mOut,
	const VECTOR3f	&vEye,
	const VECTOR3f	&vAt,
	const VECTOR3f	&vUp);

// 
// Create a look-at view matrix.
//
void MatrixLookAtRHX(
	MATRIXx			&mOut,
	const VECTOR3x	&vEye,
	const VECTOR3x	&vAt,
	const VECTOR3x	&vUp);

void MatrixPerspectiveFovLHF(
	MATRIXf	&mOut,
	const float	fFOVy,
	const float	fAspect,
	const float	fNear,
	const float	fFar,
	const bool  bRotate = false);

void MatrixPerspectiveFovLHX(
	MATRIXx	&mOut,
	const int	fFOVy,
	const int	fAspect,
	const int	fNear,
	const int	fFar,
	const bool  bRotate = false);

void MatrixPerspectiveFovRHF(
	MATRIXf	&mOut,
	const float	fFOVy,
	const float	fAspect,
	const float	fNear,
	const float	fFar,
	const bool  bRotate = false);


void MatrixPerspectiveFovRHX(
	MATRIXx	&mOut,
	const int	fFOVy,
	const int	fAspect,
	const int	fNear,
	const int	fFar,
	const bool  bRotate = false);

void MatrixOrthoLHF(
	MATRIXf	&mOut,
	const float w,
	const float h,
	const float zn,
	const float zf,
	const bool  bRotate = false);

void MatrixOrthoLHX(
	MATRIXx	&mOut,
	const int	w,
	const int	h,
	const int	zn,
	const int	zf,
	const bool  bRotate = false);



void MatrixOrthoRHF(
	MATRIXf	&mOut,
	const float w,
	const float h,
	const float zn,
	const float zf,
	const bool  bRotate = false);

void MatrixOrthoRHX(
	MATRIXx	&mOut,
	const int	w,
	const int	h,
	const int	zn,
	const int	zf,
	const bool  bRotate = false);


void MatrixVec3LerpF(
	VECTOR3f		&vOut,
	const VECTOR3f	&v1,
	const VECTOR3f	&v2,
	const float			s);

void MatrixVec3LerpX(
	VECTOR3x		&vOut,
	const VECTOR3x	&v1,
	const VECTOR3x	&v2,
	const int			s);


float MatrixVec3DotProductF(
	const VECTOR3f	&v1,
	const VECTOR3f	&v2);

int MatrixVec3DotProductX(
	const VECTOR3x	&v1,
	const VECTOR3x	&v2);



void MatrixVec3CrossProductF(
	VECTOR3f		&vOut,
	const VECTOR3f	&v1,
	const VECTOR3f	&v2);


void MatrixVec3CrossProductX(
	VECTOR3x		&vOut,
	const VECTOR3x	&v1,
	const VECTOR3x	&v2);


void MatrixVec3NormalizeF(
	VECTOR3f		&vOut,
	const VECTOR3f	&vIn);

void MatrixVec3NormalizeX(
	VECTOR3x		&vOut,
	const VECTOR3x	&vIn);
	
void MatrixVec4NormalizeF(
	VECTOR4f		&vOut,
	const VECTOR4f	&vIn);

void MatrixVec4NormalizeX(
	VECTOR4x		&vOut,
	const VECTOR4x	&vIn);

float MatrixVec3LengthF(
	const VECTOR3f	&vIn);


int MatrixVec3LengthX(
	const VECTOR3x	&vIn);


void MatrixQuaternionIdentityF(
	QUATERNIONf		&qOut);


void MatrixQuaternionIdentityX(
	QUATERNIONx		&qOut);


void MatrixQuaternionRotationAxisF(
	QUATERNIONf		&qOut,
	const VECTOR3f	&vAxis,
	const float			fAngle);

void MatrixQuaternionRotationAxisX(
	QUATERNIONx		&qOut,
	const VECTOR3x	&vAxis,
	const int			fAngle);


void MatrixQuaternionToAxisAngleF(
	const QUATERNIONf	&qIn,
	VECTOR3f			&vAxis,
	float					&fAngle);


void MatrixQuaternionToAxisAngleX(
	const QUATERNIONx	&qIn,
	VECTOR3x			&vAxis,
	int						&fAngle);


void MatrixQuaternionSlerpF(
	QUATERNIONf			&qOut,
	const QUATERNIONf	&qA,
	const QUATERNIONf	&qB,
	const float				t);


void MatrixQuaternionSlerpX(
	QUATERNIONx			&qOut,
	const QUATERNIONx	&qA,
	const QUATERNIONx	&qB,
	const int				t);

void MatrixQuaternionNormalizeF(QUATERNIONf &quat);
//
// Original quaternion is scaled down prior to be normalized in
// order to avoid overflow issues.
//
void MatrixQuaternionNormalizeX(QUATERNIONx &quat);

//
// Create rotation matrix from submitted quaternion.
// Assuming the quaternion is of the form [X Y Z W]:
//
//						|       2     2									|
//						| 1 - 2Y  - 2Z    2XY - 2ZW      2XZ + 2YW		 0	|
//						|													|
//						|                       2     2					|
//					M = | 2XY + 2ZW       1 - 2X  - 2Z   2YZ - 2XW		 0	|
//						|													|
//						|                                      2     2		|
//						| 2XZ - 2YW       2YZ + 2XW      1 - 2X  - 2Y	 0	|
//						|													|
//						|     0			   0			  0          1  |
//
void MatrixRotationQuaternionF(
	MATRIXf				&mOut,
	const QUATERNIONf	&quat);
//
// Create rotation matrix from submitted quaternion.
// Assuming the quaternion is of the form [X Y Z W]:
//
//						|       2     2									|
//						| 1 - 2Y  - 2Z    2XY - 2ZW      2XZ + 2YW		 0	|
//						|													|
//						|                       2     2					|
//					M = | 2XY + 2ZW       1 - 2X  - 2Z   2YZ - 2XW		 0	|
//						|													|
//						|                                      2     2		|
//						| 2XZ - 2YW       2YZ + 2XW      1 - 2X  - 2Y	 0	|
//						|													|
//						|     0			   0			  0          1  |
//
void MatrixRotationQuaternionX(
	MATRIXx				&mOut,
	const QUATERNIONx	&quat);


void MatrixQuaternionMultiplyF(
	QUATERNIONf			&qOut,
	const QUATERNIONf	&qA,
	const QUATERNIONf	&qB);
// 
// Multiply quaternion A with quaternion B and return the
// result in qOut.
// Input quaternions must be normalized.
//
void MatrixQuaternionMultiplyX(
	QUATERNIONx			&qOut,
	const QUATERNIONx	&qA,
	const QUATERNIONx	&qB);

//
// Solves 'nCnt' simultaneous equations of 'nCnt' variables.
// pRes should be an array large enough to contain the
// results: the values of the 'nCnt' variables.
// This fn recursively uses Gaussian Elimination.
//
void MatrixLinearEqSolveF(
	float		* const pRes,
	float		** const pSrc,
	const int	nCnt);

//
// Solves 'nCnt' simultaneous equations of 'nCnt' variables.
// pRes should be an array large enough to contain the
// results: the values of the 'nCnt' variables.
// This fn recursively uses Gaussian Elimination.
//
void MatrixLinearEqSolveX(
	int			* const pRes,
	int			** const pSrc,
	const int	nCnt);

#endif // MATRIX_H_

