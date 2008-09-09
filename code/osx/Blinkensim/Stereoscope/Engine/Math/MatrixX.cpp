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

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "FixedPoint.h"
#include "Matrix.h"
#include "Macros.h"


static const MATRIXx	c_mIdentity = {
	{
	F2X(1.0f), F2X(0.0f), F2X(0.0f), F2X(0.0f),
	F2X(0.0f), F2X(1.0f), F2X(0.0f), F2X(0.0f),
	F2X(0.0f), F2X(0.0f), F2X(1.0f), F2X(0.0f),
	F2X(0.0f), F2X(0.0f), F2X(0.0f), F2X(1.0f)
	}
};


void MatrixIdentityX(MATRIXx &mOut)
{
	mOut.f[ 0]=F2X(1.0f);	mOut.f[ 4]=F2X(0.0f);	mOut.f[ 8]=F2X(0.0f);	mOut.f[12]=F2X(0.0f);
	mOut.f[ 1]=F2X(0.0f);	mOut.f[ 5]=F2X(1.0f);	mOut.f[ 9]=F2X(0.0f);	mOut.f[13]=F2X(0.0f);
	mOut.f[ 2]=F2X(0.0f);	mOut.f[ 6]=F2X(0.0f);	mOut.f[10]=F2X(1.0f);	mOut.f[14]=F2X(0.0f);
	mOut.f[ 3]=F2X(0.0f);	mOut.f[ 7]=F2X(0.0f);	mOut.f[11]=F2X(0.0f);	mOut.f[15]=F2X(1.0f);
}

void MatrixMultiplyX(
	MATRIXx			&mOut,
	const MATRIXx	&mA,
	const MATRIXx	&mB)
{
	MATRIXx mRet;

	/* Perform calculation on a dummy matrix (mRet) */
	mRet.f[ 0] = XMUL(mA.f[ 0], mB.f[ 0]) + XMUL(mA.f[ 1], mB.f[ 4]) + XMUL(mA.f[ 2], mB.f[ 8]) + XMUL(mA.f[ 3], mB.f[12]);
	mRet.f[ 1] = XMUL(mA.f[ 0], mB.f[ 1]) + XMUL(mA.f[ 1], mB.f[ 5]) + XMUL(mA.f[ 2], mB.f[ 9]) + XMUL(mA.f[ 3], mB.f[13]);
	mRet.f[ 2] = XMUL(mA.f[ 0], mB.f[ 2]) + XMUL(mA.f[ 1], mB.f[ 6]) + XMUL(mA.f[ 2], mB.f[10]) + XMUL(mA.f[ 3], mB.f[14]);
	mRet.f[ 3] = XMUL(mA.f[ 0], mB.f[ 3]) + XMUL(mA.f[ 1], mB.f[ 7]) + XMUL(mA.f[ 2], mB.f[11]) + XMUL(mA.f[ 3], mB.f[15]);

	mRet.f[ 4] = XMUL(mA.f[ 4], mB.f[ 0]) + XMUL(mA.f[ 5], mB.f[ 4]) + XMUL(mA.f[ 6], mB.f[ 8]) + XMUL(mA.f[ 7], mB.f[12]);
	mRet.f[ 5] = XMUL(mA.f[ 4], mB.f[ 1]) + XMUL(mA.f[ 5], mB.f[ 5]) + XMUL(mA.f[ 6], mB.f[ 9]) + XMUL(mA.f[ 7], mB.f[13]);
	mRet.f[ 6] = XMUL(mA.f[ 4], mB.f[ 2]) + XMUL(mA.f[ 5], mB.f[ 6]) + XMUL(mA.f[ 6], mB.f[10]) + XMUL(mA.f[ 7], mB.f[14]);
	mRet.f[ 7] = XMUL(mA.f[ 4], mB.f[ 3]) + XMUL(mA.f[ 5], mB.f[ 7]) + XMUL(mA.f[ 6], mB.f[11]) + XMUL(mA.f[ 7], mB.f[15]);

	mRet.f[ 8] = XMUL(mA.f[ 8], mB.f[ 0]) + XMUL(mA.f[ 9], mB.f[ 4]) + XMUL(mA.f[10], mB.f[ 8]) + XMUL(mA.f[11], mB.f[12]);
	mRet.f[ 9] = XMUL(mA.f[ 8], mB.f[ 1]) + XMUL(mA.f[ 9], mB.f[ 5]) + XMUL(mA.f[10], mB.f[ 9]) + XMUL(mA.f[11], mB.f[13]);
	mRet.f[10] = XMUL(mA.f[ 8], mB.f[ 2]) + XMUL(mA.f[ 9], mB.f[ 6]) + XMUL(mA.f[10], mB.f[10]) + XMUL(mA.f[11], mB.f[14]);
	mRet.f[11] = XMUL(mA.f[ 8], mB.f[ 3]) + XMUL(mA.f[ 9], mB.f[ 7]) + XMUL(mA.f[10], mB.f[11]) + XMUL(mA.f[11], mB.f[15]);

	mRet.f[12] = XMUL(mA.f[12], mB.f[ 0]) + XMUL(mA.f[13], mB.f[ 4]) + XMUL(mA.f[14], mB.f[ 8]) + XMUL(mA.f[15], mB.f[12]);
	mRet.f[13] = XMUL(mA.f[12], mB.f[ 1]) + XMUL(mA.f[13], mB.f[ 5]) + XMUL(mA.f[14], mB.f[ 9]) + XMUL(mA.f[15], mB.f[13]);
	mRet.f[14] = XMUL(mA.f[12], mB.f[ 2]) + XMUL(mA.f[13], mB.f[ 6]) + XMUL(mA.f[14], mB.f[10]) + XMUL(mA.f[15], mB.f[14]);
	mRet.f[15] = XMUL(mA.f[12], mB.f[ 3]) + XMUL(mA.f[13], mB.f[ 7]) + XMUL(mA.f[14], mB.f[11]) + XMUL(mA.f[15], mB.f[15]);

	/* Copy result in pResultMatrix */
	mOut = mRet;
}

void MatrixTranslationX(
	MATRIXx	&mOut,
	const int	fX,
	const int	fY,
	const int	fZ)
{
	mOut.f[ 0]=F2X(1.0f);	mOut.f[ 4]=F2X(0.0f);	mOut.f[ 8]=F2X(0.0f);	mOut.f[12]=fX;
	mOut.f[ 1]=F2X(0.0f);	mOut.f[ 5]=F2X(1.0f);	mOut.f[ 9]=F2X(0.0f);	mOut.f[13]=fY;
	mOut.f[ 2]=F2X(0.0f);	mOut.f[ 6]=F2X(0.0f);	mOut.f[10]=F2X(1.0f);	mOut.f[14]=fZ;
	mOut.f[ 3]=F2X(0.0f);	mOut.f[ 7]=F2X(0.0f);	mOut.f[11]=F2X(0.0f);	mOut.f[15]=F2X(1.0f);
}


void MatrixScalingX(
	MATRIXx	&mOut,
	const int	fX,
	const int	fY,
	const int	fZ)
{
	mOut.f[ 0]=fX;				mOut.f[ 4]=F2X(0.0f);	mOut.f[ 8]=F2X(0.0f);	mOut.f[12]=F2X(0.0f);
	mOut.f[ 1]=F2X(0.0f);	mOut.f[ 5]=fY;				mOut.f[ 9]=F2X(0.0f);	mOut.f[13]=F2X(0.0f);
	mOut.f[ 2]=F2X(0.0f);	mOut.f[ 6]=F2X(0.0f);	mOut.f[10]=fZ;				mOut.f[14]=F2X(0.0f);
	mOut.f[ 3]=F2X(0.0f);	mOut.f[ 7]=F2X(0.0f);	mOut.f[11]=F2X(0.0f);	mOut.f[15]=F2X(1.0f);
}


void MatrixRotationXX(
	MATRIXx	&mOut,
	const int	fAngle)
{
	int		fCosine, fSine;

    /* Precompute cos and sin */
#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	fCosine	= XCOS(-fAngle);
    fSine	= XSIN(-fAngle);
#else
	fCosine	= XCOS(fAngle);
    fSine	= XSIN(fAngle);
#endif

	/* Create the trigonometric matrix corresponding to X Rotation */
	mOut.f[ 0]=F2X(1.0f);	mOut.f[ 4]=F2X(0.0f);		mOut.f[ 8]=F2X(0.0f);		mOut.f[12]=F2X(0.0f);
	mOut.f[ 1]=F2X(0.0f);	mOut.f[ 5]=fCosine;				mOut.f[ 9]=fSine;				mOut.f[13]=F2X(0.0f);
	mOut.f[ 2]=F2X(0.0f);	mOut.f[ 6]=-fSine;				mOut.f[10]=fCosine;				mOut.f[14]=F2X(0.0f);
	mOut.f[ 3]=F2X(0.0f);	mOut.f[ 7]=F2X(0.0f);		mOut.f[11]=F2X(0.0f);		mOut.f[15]=F2X(1.0f);
}


void MatrixRotationYX(
	MATRIXx	&mOut,
	const int	fAngle)
{
	int		fCosine, fSine;

	/* Precompute cos and sin */
#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	fCosine	= XCOS(-fAngle);
    fSine	= XSIN(-fAngle);
#else
	fCosine	= XCOS(fAngle);
    fSine	= XSIN(fAngle);
#endif

	/* Create the trigonometric matrix corresponding to Y Rotation */
	mOut.f[ 0]=fCosine;				mOut.f[ 4]=F2X(0.0f);	mOut.f[ 8]=-fSine;				mOut.f[12]=F2X(0.0f);
	mOut.f[ 1]=F2X(0.0f);		mOut.f[ 5]=F2X(1.0f);	mOut.f[ 9]=F2X(0.0f);		mOut.f[13]=F2X(0.0f);
	mOut.f[ 2]=fSine;				mOut.f[ 6]=F2X(0.0f);	mOut.f[10]=fCosine;				mOut.f[14]=F2X(0.0f);
	mOut.f[ 3]=F2X(0.0f);		mOut.f[ 7]=F2X(0.0f);	mOut.f[11]=F2X(0.0f);		mOut.f[15]=F2X(1.0f);
}


void MatrixRotationZX(
	MATRIXx	&mOut,
	const int	fAngle)
{
	int		fCosine, fSine;

	/* Precompute cos and sin */
#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	fCosine = XCOS(-fAngle);
    fSine   = XSIN(-fAngle);
#else
	fCosine = XCOS(fAngle);
    fSine   = XSIN(fAngle);
#endif

	/* Create the trigonometric matrix corresponding to Z Rotation */
	mOut.f[ 0]=fCosine;				mOut.f[ 4]=fSine;				mOut.f[ 8]=F2X(0.0f);	mOut.f[12]=F2X(0.0f);
	mOut.f[ 1]=-fSine;				mOut.f[ 5]=fCosine;				mOut.f[ 9]=F2X(0.0f);	mOut.f[13]=F2X(0.0f);
	mOut.f[ 2]=F2X(0.0f);		mOut.f[ 6]=F2X(0.0f);		mOut.f[10]=F2X(1.0f);	mOut.f[14]=F2X(0.0f);
	mOut.f[ 3]=F2X(0.0f);		mOut.f[ 7]=F2X(0.0f);		mOut.f[11]=F2X(0.0f);	mOut.f[15]=F2X(1.0f);
}



void MatrixTransposeX(
	MATRIXx			&mOut,
	const MATRIXx	&mIn)
{
	MATRIXx	mTmp;

	mTmp.f[ 0]=mIn.f[ 0];	mTmp.f[ 4]=mIn.f[ 1];	mTmp.f[ 8]=mIn.f[ 2];	mTmp.f[12]=mIn.f[ 3];
	mTmp.f[ 1]=mIn.f[ 4];	mTmp.f[ 5]=mIn.f[ 5];	mTmp.f[ 9]=mIn.f[ 6];	mTmp.f[13]=mIn.f[ 7];
	mTmp.f[ 2]=mIn.f[ 8];	mTmp.f[ 6]=mIn.f[ 9];	mTmp.f[10]=mIn.f[10];	mTmp.f[14]=mIn.f[11];
	mTmp.f[ 3]=mIn.f[12];	mTmp.f[ 7]=mIn.f[13];	mTmp.f[11]=mIn.f[14];	mTmp.f[15]=mIn.f[15];

	mOut = mTmp;
}


void MatrixInverseX(
	MATRIXx			&mOut,
	const MATRIXx	&mIn)
{
	MATRIXx	mDummyMatrix;
	int			det_1;
	int			pos, neg, temp;

    /* Calculate the determinant of submatrix A and determine if the
       the matrix is singular as limited by the double precision
       floating-point data representation. */
    pos = neg = 0;
    temp =  XMUL(XMUL(mIn.f[ 0], mIn.f[ 5]), mIn.f[10]);
    if (temp >= 0) pos += temp; else neg += temp;
    temp =  XMUL(XMUL(mIn.f[ 4], mIn.f[ 9]), mIn.f[ 2]);
    if (temp >= 0) pos += temp; else neg += temp;
    temp =  XMUL(XMUL(mIn.f[ 8], mIn.f[ 1]), mIn.f[ 6]);
    if (temp >= 0) pos += temp; else neg += temp;
	temp =  XMUL(XMUL(-mIn.f[ 8], mIn.f[ 5]), mIn.f[ 2]);
    if (temp >= 0) pos += temp; else neg += temp;
    temp =  XMUL(XMUL(-mIn.f[ 4], mIn.f[ 1]), mIn.f[10]);
    if (temp >= 0) pos += temp; else neg += temp;
    temp =  XMUL(XMUL(-mIn.f[ 0], mIn.f[ 9]), mIn.f[ 6]);
    if (temp >= 0) pos += temp; else neg += temp;
    det_1 = pos + neg;

    /* Is the submatrix A singular? */
    if (det_1 == 0)
	{
        /* Matrix M has no inverse */
        printf("Matrix has no inverse : singular matrix\n");
        return;
    }
    else
	{
        /* Calculate inverse(A) = adj(A) / det(A) */
        //det_1 = 1.0 / det_1;
		det_1 = XDIV(F2X(1.0f), det_1);
		mDummyMatrix.f[ 0] =   XMUL(( XMUL(mIn.f[ 5], mIn.f[10]) - XMUL(mIn.f[ 9], mIn.f[ 6]) ), det_1);
		mDummyMatrix.f[ 1] = - XMUL(( XMUL(mIn.f[ 1], mIn.f[10]) - XMUL(mIn.f[ 9], mIn.f[ 2]) ), det_1);
		mDummyMatrix.f[ 2] =   XMUL(( XMUL(mIn.f[ 1], mIn.f[ 6]) - XMUL(mIn.f[ 5], mIn.f[ 2]) ), det_1);
		mDummyMatrix.f[ 4] = - XMUL(( XMUL(mIn.f[ 4], mIn.f[10]) - XMUL(mIn.f[ 8], mIn.f[ 6]) ), det_1);
		mDummyMatrix.f[ 5] =   XMUL(( XMUL(mIn.f[ 0], mIn.f[10]) - XMUL(mIn.f[ 8], mIn.f[ 2]) ), det_1);
		mDummyMatrix.f[ 6] = - XMUL(( XMUL(mIn.f[ 0], mIn.f[ 6]) - XMUL(mIn.f[ 4], mIn.f[ 2]) ), det_1);
		mDummyMatrix.f[ 8] =   XMUL(( XMUL(mIn.f[ 4], mIn.f[ 9]) - XMUL(mIn.f[ 8], mIn.f[ 5]) ), det_1);
		mDummyMatrix.f[ 9] = - XMUL(( XMUL(mIn.f[ 0], mIn.f[ 9]) - XMUL(mIn.f[ 8], mIn.f[ 1]) ), det_1);
		mDummyMatrix.f[10] =   XMUL(( XMUL(mIn.f[ 0], mIn.f[ 5]) - XMUL(mIn.f[ 4], mIn.f[ 1]) ), det_1);

        /* Calculate -C * inverse(A) */
        mDummyMatrix.f[12] = - ( XMUL(mIn.f[12], mDummyMatrix.f[ 0]) + XMUL(mIn.f[13], mDummyMatrix.f[ 4]) + XMUL(mIn.f[14], mDummyMatrix.f[ 8]) );
		mDummyMatrix.f[13] = - ( XMUL(mIn.f[12], mDummyMatrix.f[ 1]) + XMUL(mIn.f[13], mDummyMatrix.f[ 5]) + XMUL(mIn.f[14], mDummyMatrix.f[ 9]) );
		mDummyMatrix.f[14] = - ( XMUL(mIn.f[12], mDummyMatrix.f[ 2]) + XMUL(mIn.f[13], mDummyMatrix.f[ 6]) + XMUL(mIn.f[14], mDummyMatrix.f[10]) );

        /* Fill in last row */
        mDummyMatrix.f[ 3] = F2X(0.0f);
		mDummyMatrix.f[ 7] = F2X(0.0f);
		mDummyMatrix.f[11] = F2X(0.0f);
        mDummyMatrix.f[15] = F2X(1.0f);
	}

   	/* Copy contents of dummy matrix in pfMatrix */
	mOut = mDummyMatrix;
}


void MatrixInverseExX(
	MATRIXx			&mOut,
	const MATRIXx	&mIn)
{
	MATRIXx		mTmp;
	int				*ppfRows[4], pfRes[4], pfIn[20];
	int				i, j;

	for (i = 0; i < 4; ++i)
	{
		ppfRows[i] = &pfIn[i * 5];
	}

	/* Solve 4 sets of 4 linear equations */
	for (i = 0; i < 4; ++i)
	{
		for (j = 0; j < 4; ++j)
		{
			ppfRows[j][0] = c_mIdentity.f[i + 4 * j];
			memcpy(&ppfRows[j][1], &mIn.f[j * 4], 4 * sizeof(float));
		}

		MatrixLinearEqSolveX(pfRes, (int**)ppfRows, 4);

		for(j = 0; j < 4; ++j)
		{
			mTmp.f[i + 4 * j] = pfRes[j];
		}
	}

	mOut = mTmp;
}


void MatrixLookAtLHX(
	MATRIXx			&mOut,
	const VECTOR3x	&vEye,
	const VECTOR3x	&vAt,
	const VECTOR3x	&vUp)
{
	VECTOR3x	f, vUpActual, s, u;
	MATRIXx		t;

	f.x = vEye.x - vAt.x;
	f.y = vEye.y - vAt.y;
	f.z = vEye.z - vAt.z;

	MatrixVec3NormalizeX(f, f);
	MatrixVec3NormalizeX(vUpActual, vUp);
	MatrixVec3CrossProductX(s, f, vUpActual);
	MatrixVec3CrossProductX(u, s, f);

	mOut.f[ 0] = s.x;
	mOut.f[ 1] = u.x;
	mOut.f[ 2] = -f.x;
	mOut.f[ 3] = F2X(0.0f);

	mOut.f[ 4] = s.y;
	mOut.f[ 5] = u.y;
	mOut.f[ 6] = -f.y;
	mOut.f[ 7] = F2X(0.0f);

	mOut.f[ 8] = s.z;
	mOut.f[ 9] = u.z;
	mOut.f[10] = -f.z;
	mOut.f[11] = F2X(0.0f);

	mOut.f[12] = F2X(0.0f);
	mOut.f[13] = F2X(0.0f);
	mOut.f[14] = F2X(0.0f);
	mOut.f[15] = F2X(1.0f);

	MatrixTranslationX(t, -vEye.x, -vEye.y, -vEye.z);
	MatrixMultiplyX(mOut, t, mOut);
}



void MatrixLookAtRHX(
	MATRIXx			&mOut,
	const VECTOR3x	&vEye,
	const VECTOR3x	&vAt,
	const VECTOR3x	&vUp)
{
	VECTOR3x	f, vUpActual, s, u;
	MATRIXx		t;

	f.x = vAt.x - vEye.x;
	f.y = vAt.y - vEye.y;
	f.z = vAt.z - vEye.z;

	MatrixVec3NormalizeX(f, f);
	MatrixVec3NormalizeX(vUpActual, vUp);
	MatrixVec3CrossProductX(s, f, vUpActual);
	MatrixVec3CrossProductX(u, s, f);

	mOut.f[ 0] = s.x;
	mOut.f[ 1] = u.x;
	mOut.f[ 2] = -f.x;
	mOut.f[ 3] = F2X(0.0f);

	mOut.f[ 4] = s.y;
	mOut.f[ 5] = u.y;
	mOut.f[ 6] = -f.y;
	mOut.f[ 7] = F2X(0.0f);

	mOut.f[ 8] = s.z;
	mOut.f[ 9] = u.z;
	mOut.f[10] = -f.z;
	mOut.f[11] = F2X(0.0f);

	mOut.f[12] = F2X(0.0f);
	mOut.f[13] = F2X(0.0f);
	mOut.f[14] = F2X(0.0f);
	mOut.f[15] = F2X(1.0f);

	MatrixTranslationX(t, -vEye.x, -vEye.y, -vEye.z);
	MatrixMultiplyX(mOut, t, mOut);
}


void MatrixPerspectiveFovLHX(
	MATRIXx	&mOut,
	const int	fFOVy,
	const int	fAspect,
	const int	fNear,
	const int	fFar,
	const bool  bRotate)
{
	int		f, fRealAspect;

	if (bRotate)
		fRealAspect = XDIV(F2X(1.0f), fAspect);
	else
		fRealAspect = fAspect;

	f = XDIV(F2X(1.0f), XTAN(XMUL(fFOVy, F2X(0.5f))));

	mOut.f[ 0] = XDIV(f, fRealAspect);
	mOut.f[ 1] = F2X(0.0f);
	mOut.f[ 2] = F2X(0.0f);
	mOut.f[ 3] = F2X(0.0f);

	mOut.f[ 4] = F2X(0.0f);
	mOut.f[ 5] = f;
	mOut.f[ 6] = F2X(0.0f);
	mOut.f[ 7] = F2X(0.0f);

	mOut.f[ 8] = F2X(0.0f);
	mOut.f[ 9] = F2X(0.0f);
	mOut.f[10] = XDIV(fFar, fFar - fNear);
	mOut.f[11] = F2X(1.0f);

	mOut.f[12] = F2X(0.0f);
	mOut.f[13] = F2X(0.0f);
	mOut.f[14] = -XMUL(XDIV(fFar, fFar - fNear), fNear);
	mOut.f[15] = F2X(0.0f);

	if (bRotate)
	{
		MATRIXx mRotation, mTemp = mOut;
		MatrixRotationZX(mRotation, F2X(90.0f*PIf/180.0f));
		MatrixMultiplyX(mOut, mTemp, mRotation);
	}
}



void MatrixPerspectiveFovRHX(
	MATRIXx	&mOut,
	const int	fFOVy,
	const int	fAspect,
	const int	fNear,
	const int	fFar,
	const bool  bRotate)
{
	int		f;

	int fCorrectAspect = fAspect;
	if (bRotate)
	{
		fCorrectAspect = XDIV(F2X(1.0f), fAspect);
	}
	f = XDIV(F2X(1.0f), XTAN(XMUL(fFOVy, F2X(0.5f))));

	mOut.f[ 0] = XDIV(f, fCorrectAspect);
	mOut.f[ 1] = F2X(0.0f);
	mOut.f[ 2] = F2X(0.0f);
	mOut.f[ 3] = F2X(0.0f);

	mOut.f[ 4] = F2X(0.0f);
	mOut.f[ 5] = f;
	mOut.f[ 6] = F2X(0.0f);
	mOut.f[ 7] = F2X(0.0f);

	mOut.f[ 8] = F2X(0.0f);
	mOut.f[ 9] = F2X(0.0f);
	mOut.f[10] = XDIV(fFar + fNear, fNear - fFar);
	mOut.f[11] = F2X(-1.0f);

	mOut.f[12] = F2X(0.0f);
	mOut.f[13] = F2X(0.0f);
	mOut.f[14] = XMUL(XDIV(fFar, fNear - fFar), fNear) << 1;	// Cheap 2x
	mOut.f[15] = F2X(0.0f);

	if (bRotate)
	{
		MATRIXx mRotation, mTemp = mOut;
		MatrixRotationZX(mRotation, F2X(-90.0f*PIf/180.0f));
		MatrixMultiplyX(mOut, mTemp, mRotation);
	}
}


void MatrixOrthoLHX(
	MATRIXx	&mOut,
	const int	w,
	const int	h,
	const int	zn,
	const int	zf,
	const bool  bRotate)
{
	int fCorrectW = w;
	int fCorrectH = h;
	if (bRotate)
	{
		fCorrectW = h;
		fCorrectH = w;
	}
	mOut.f[ 0] = XDIV(F2X(2.0f), fCorrectW);
	mOut.f[ 1] = F2X(0.0f);
	mOut.f[ 2] = F2X(0.0f);
	mOut.f[ 3] = F2X(0.0f);

	mOut.f[ 4] = F2X(0.0f);
	mOut.f[ 5] = XDIV(F2X(2.0f), fCorrectH);
	mOut.f[ 6] = F2X(0.0f);
	mOut.f[ 7] = F2X(0.0f);

	mOut.f[ 8] = F2X(0.0f);
	mOut.f[ 9] = F2X(0.0f);
	mOut.f[10] = XDIV(F2X(1.0f), zf - zn);
	mOut.f[11] = XDIV(zn, zn - zf);

	mOut.f[12] = F2X(0.0f);
	mOut.f[13] = F2X(0.0f);
	mOut.f[14] = F2X(0.0f);
	mOut.f[15] = F2X(1.0f);

	if (bRotate)
	{
		MATRIXx mRotation, mTemp = mOut;
		MatrixRotationZX(mRotation, F2X(-90.0f*PIf/180.0f));
		MatrixMultiplyX(mOut, mRotation, mTemp);
	}
}


void MatrixOrthoRHX(
	MATRIXx	&mOut,
	const int	w,
	const int	h,
	const int	zn,
	const int	zf,
	const bool  bRotate)
{
	int fCorrectW = w;
	int fCorrectH = h;
	if (bRotate)
	{
		fCorrectW = h;
		fCorrectH = w;
	}
	mOut.f[ 0] = XDIV(F2X(2.0f), fCorrectW);
	mOut.f[ 1] = F2X(0.0f);
	mOut.f[ 2] = F2X(0.0f);
	mOut.f[ 3] = F2X(0.0f);

	mOut.f[ 4] = F2X(0.0f);
	mOut.f[ 5] = XDIV(F2X(2.0f), fCorrectH);
	mOut.f[ 6] = F2X(0.0f);
	mOut.f[ 7] = F2X(0.0f);

	mOut.f[ 8] = F2X(0.0f);
	mOut.f[ 9] = F2X(0.0f);
	mOut.f[10] = XDIV(F2X(1.0f), zn - zf);
	mOut.f[11] = XDIV(zn, zn - zf);

	mOut.f[12] = F2X(0.0f);
	mOut.f[13] = F2X(0.0f);
	mOut.f[14] = F2X(0.0f);
	mOut.f[15] = F2X(1.0f);

	if (bRotate)
	{
		MATRIXx mRotation, mTemp = mOut;
		MatrixRotationZX(mRotation, F2X(-90.0f*PIf/180.0f));
		MatrixMultiplyX(mOut, mRotation, mTemp);
	}
}



void MatrixVec3LerpX(
	VECTOR3x		&vOut,
	const VECTOR3x	&v1,
	const VECTOR3x	&v2,
	const int			s)
{
	vOut.x = v1.x + XMUL(s, v2.x - v1.x);
	vOut.y = v1.y + XMUL(s, v2.y - v1.y);
	vOut.z = v1.z + XMUL(s, v2.z - v1.z);
}


int MatrixVec3DotProductX(
	const VECTOR3x	&v1,
	const VECTOR3x	&v2)
{
	return (XMUL(v1.x, v2.x) + XMUL(v1.y, v2.y) + XMUL(v1.z, v2.z));
}


void MatrixVec3CrossProductX(
	VECTOR3x		&vOut,
	const VECTOR3x	&v1,
	const VECTOR3x	&v2)
{
	VECTOR3x result;

	/* Perform calculation on a dummy VECTOR (result) */
    result.x = XMUL(v1.y, v2.z) - XMUL(v1.z, v2.y);
    result.y = XMUL(v1.z, v2.x) - XMUL(v1.x, v2.z);
    result.z = XMUL(v1.x, v2.y) - XMUL(v1.y, v2.x);

	/* Copy result in pOut */
	vOut = result;
}



void MatrixVec3NormalizeX(
	VECTOR3x		&vOut,
	const VECTOR3x	&vIn)
{
	int				f, n;
	VECTOR3x	vTemp;

	/* Scale vector by uniform value */
	n = _ABS(vIn.x) + _ABS(vIn.y) + _ABS(vIn.z);
	vTemp.x = XDIV(vIn.x, n);
	vTemp.y = XDIV(vIn.y, n);
	vTemp.z = XDIV(vIn.z, n);

	/* Calculate x2+y2+z2/sqrt(x2+y2+z2) */
	f = MatrixVec3DotProductX(vTemp, vTemp);
	f = XDIV(F2X(1.0f), F2X(sqrt(X2F(f))));

	/* Multiply vector components by f */
	vOut.x = XMUL(vTemp.x, f);
	vOut.y = XMUL(vTemp.y, f);
	vOut.z = XMUL(vTemp.z, f);
}

void MatrixVec4NormalizeX(
	VECTOR4x		&vOut,
	const VECTOR4x	&vIn)
{
	int				f, n;
	VECTOR4x	vTemp;

	/* Scale vector by uniform value */
	n = _ABS(vIn.x) + _ABS(vIn.y) + _ABS(vIn.z);
	vTemp.x = XDIV(vIn.x, n);
	vTemp.y = XDIV(vIn.y, n);
	vTemp.z = XDIV(vIn.z, n);

	/* Calculate x2+y2+z2/sqrt(x2+y2+z2) */
	f = MatrixVec3DotProductX((VECTOR3x&)vTemp, (VECTOR3x&)vTemp);
	f = XDIV(F2X(1.0f), F2X(sqrt(X2F(f))));

	/* Multiply vector components by f */
	vOut.x = XMUL(vTemp.x, f);
	vOut.y = XMUL(vTemp.y, f);
	vOut.z = XMUL(vTemp.z, f);
	vOut.w = XMUL(vTemp.w, f);
}



int MatrixVec3LengthX(
	const VECTOR3x	&vIn)
{
	int temp;

	temp = XMUL(vIn.x,vIn.x) + XMUL(vIn.y,vIn.y) + XMUL(vIn.z,vIn.z);
	return F2X(sqrt(X2F(temp)));
}


void MatrixQuaternionIdentityX(
	QUATERNIONx		&qOut)
{
	qOut.x = F2X(0.0f);
	qOut.y = F2X(0.0f);
	qOut.z = F2X(0.0f);
	qOut.w = F2X(1.0f);
}


void MatrixQuaternionRotationAxisX(
	QUATERNIONx		&qOut,
	const VECTOR3x	&vAxis,
	const int			fAngle)
{
	int	fSin, fCos;

	fSin = XSIN(fAngle>>1);
	fCos = XCOS(fAngle>>1);

	/* Create quaternion */
	qOut.x = XMUL(vAxis.x, fSin);
	qOut.y = XMUL(vAxis.y, fSin);
	qOut.z = XMUL(vAxis.z, fSin);
	qOut.w = fCos;

	/* Normalise it */
	MatrixQuaternionNormalizeX(qOut);
}


void MatrixQuaternionToAxisAngleX(
	const QUATERNIONx	&qIn,
	VECTOR3x			&vAxis,
	int						&fAngle)
{
	int		fCosAngle, fSinAngle;
	int		temp;

	/* Compute some values */
	fCosAngle	= qIn.w;
	temp		= F2X(1.0f) - XMUL(fCosAngle, fCosAngle);
	fAngle		= XMUL(XACOS(fCosAngle), F2X(2.0f));
	fSinAngle	= F2X(((float)sqrt(X2F(temp))));

	/* This is to avoid a division by zero */
	if (_ABS(fSinAngle)<F2X(0.0005f))
	{
		fSinAngle = F2X(1.0f);
	}

	/* Get axis vector */
	vAxis.x = XDIV(qIn.x, fSinAngle);
	vAxis.y = XDIV(qIn.y, fSinAngle);
	vAxis.z = XDIV(qIn.z, fSinAngle);
}


void MatrixQuaternionSlerpX(
	QUATERNIONx			&qOut,
	const QUATERNIONx	&qA,
	const QUATERNIONx	&qB,
	const int				t)
{
	int		fCosine, fAngle, A, B;

	/* Parameter checking */
	if (t<F2X(0.0f) || t>F2X(1.0f))
	{
		printf("MatrixQuaternionSlerp : Bad parameters\n");
		qOut.x = F2X(0.0f);
		qOut.y = F2X(0.0f);
		qOut.z = F2X(0.0f);
		qOut.w = F2X(1.0f);
		return;
	}

	/* Find sine of Angle between Quaternion A and B (dot product between quaternion A and B) */
	fCosine = XMUL(qA.w, qB.w) +
		XMUL(qA.x, qB.x) + XMUL(qA.y, qB.y) + XMUL(qA.z, qB.z);

	if(fCosine < F2X(0.0f))
	{
		QUATERNIONx qi;

		/*
			<http://www.magic-software.com/Documentation/Quaternions.pdf>

			"It is important to note that the quaternions q and -q represent
			the same rotation... while either quaternion will do, the
			interpolation methods require choosing one over the other.

			"Although q1 and -q1 represent the same rotation, the values of
			Slerp(t; q0, q1) and Slerp(t; q0,-q1) are not the same. It is
			customary to choose the sign... on q1 so that... the angle
			between q0 and q1 is acute. This choice avoids extra
			spinning caused by the interpolated rotations."
		*/
		qi.x = -qB.x;
		qi.y = -qB.y;
		qi.z = -qB.z;
		qi.w = -qB.w;

		MatrixQuaternionSlerpX(qOut, qA, qi, t);
		return;
	}

	fCosine = _MIN(fCosine, F2X(1.0f));
	fAngle = XACOS(fCosine);

	/* Avoid a division by zero */
	if (fAngle==F2X(0.0f))
	{
		qOut = qA;
		return;
	}

	/* Precompute some values */
	A = XDIV(XSIN(XMUL((F2X(1.0f)-t), fAngle)), XSIN(fAngle));
	B = XDIV(XSIN(XMUL(t, fAngle)), XSIN(fAngle));

	/* Compute resulting quaternion */
	qOut.x = XMUL(A, qA.x) + XMUL(B, qB.x);
	qOut.y = XMUL(A, qA.y) + XMUL(B, qB.y);
	qOut.z = XMUL(A, qA.z) + XMUL(B, qB.z);
	qOut.w = XMUL(A, qA.w) + XMUL(B, qB.w);

	/* Normalise result */
	MatrixQuaternionNormalizeX(qOut);
}


void MatrixQuaternionNormalizeX(QUATERNIONx &quat)
{
	QUATERNIONx	qTemp;
	int				f, n;

	/* Scale vector by uniform value */
	n = _ABS(quat.w) + _ABS(quat.x) + _ABS(quat.y) + _ABS(quat.z);
	qTemp.w = XDIV(quat.w, n);
	qTemp.x = XDIV(quat.x, n);
	qTemp.y = XDIV(quat.y, n);
	qTemp.z = XDIV(quat.z, n);

	/* Compute quaternion magnitude */
	f = XMUL(qTemp.w, qTemp.w) + XMUL(qTemp.x, qTemp.x) + XMUL(qTemp.y, qTemp.y) + XMUL(qTemp.z, qTemp.z);
	f = XDIV(F2X(1.0f), F2X(sqrt(X2F(f))));

	/* Multiply vector components by f */
	quat.x = XMUL(qTemp.x, f);
	quat.y = XMUL(qTemp.y, f);
	quat.z = XMUL(qTemp.z, f);
	quat.w = XMUL(qTemp.w, f);
}



void MatrixRotationQuaternionX(
	MATRIXx				&mOut,
	const QUATERNIONx	&quat)
{
	const QUATERNIONx *pQ;

#if defined(BUILD_DX9) || defined(BUILD_D3DM)
	QUATERNIONx qInv;

	qInv.x = -quat.x;
	qInv.y = -quat.y;
	qInv.z = -quat.z;
	qInv.w =  quat.w;

	pQ = &qInv;
#else
	pQ = &quat;
#endif

    /* Fill matrix members */
	mOut.f[0] = F2X(1.0f) - (XMUL(pQ->y, pQ->y)<<1) - (XMUL(pQ->z, pQ->z)<<1);
	mOut.f[1] = (XMUL(pQ->x, pQ->y)<<1) - (XMUL(pQ->z, pQ->w)<<1);
	mOut.f[2] = (XMUL(pQ->x, pQ->z)<<1) + (XMUL(pQ->y, pQ->w)<<1);
	mOut.f[3] = F2X(0.0f);

	mOut.f[4] = (XMUL(pQ->x, pQ->y)<<1) + (XMUL(pQ->z, pQ->w)<<1);
	mOut.f[5] = F2X(1.0f) - (XMUL(pQ->x, pQ->x)<<1) - (XMUL(pQ->z, pQ->z)<<1);
	mOut.f[6] = (XMUL(pQ->y, pQ->z)<<1) - (XMUL(pQ->x, pQ->w)<<1);
	mOut.f[7] = F2X(0.0f);

	mOut.f[8] = (XMUL(pQ->x, pQ->z)<<1) - (XMUL(pQ->y, pQ->w)<<1);
	mOut.f[9] = (XMUL(pQ->y, pQ->z)<<1) + (XMUL(pQ->x, pQ->w)<<1);
	mOut.f[10] = F2X(1.0f) - (XMUL(pQ->x, pQ->x)<<1) - (XMUL(pQ->y, pQ->y)<<1);
	mOut.f[11] = F2X(0.0f);

	mOut.f[12] = F2X(0.0f);
	mOut.f[13] = F2X(0.0f);
	mOut.f[14] = F2X(0.0f);
	mOut.f[15] = F2X(1.0f);
}


void MatrixQuaternionMultiplyX(
	QUATERNIONx			&qOut,
	const QUATERNIONx	&qA,
	const QUATERNIONx	&qB)
{
	VECTOR3x	CrossProduct;

	/* Compute scalar component */
	qOut.w = XMUL(qA.w, qB.w) -
				   (XMUL(qA.x, qB.x) + XMUL(qA.y, qB.y) + XMUL(qA.z, qB.z));

	/* Compute cross product */
	CrossProduct.x = XMUL(qA.y, qB.z) - XMUL(qA.z, qB.y);
	CrossProduct.y = XMUL(qA.z, qB.x) - XMUL(qA.x, qB.z);
	CrossProduct.z = XMUL(qA.x, qB.y) - XMUL(qA.y, qB.x);

	/* Compute result vector */
	qOut.x = XMUL(qA.w, qB.x) + XMUL(qB.w, qA.x) + CrossProduct.x;
	qOut.y = XMUL(qA.w, qB.y) + XMUL(qB.w, qA.y) + CrossProduct.y;
	qOut.z = XMUL(qA.w, qB.z) + XMUL(qB.w, qA.z) + CrossProduct.z;

	/* Normalize resulting quaternion */
	MatrixQuaternionNormalizeX(qOut);
}



void MatrixLinearEqSolveX(
	int			* const pRes,
	int			** const pSrc,
	const int	nCnt)
{
	int		i, j, k;
	int		f;

	if (nCnt == 1)
	{
		//_ASSERT(pSrc[0][1] != 0);
		pRes[0] = XDIV(pSrc[0][0], pSrc[0][1]);
		return;
	}

	// Loop backwards in an attempt avoid the need to swap rows
	i = nCnt;
	while(i)
	{
		--i;

		if(pSrc[i][nCnt] != F2X(0.0f))
		{
			// Row i can be used to zero the other rows; let's move it to the bottom
			if(i != (nCnt-1))
			{
				for(j = 0; j <= nCnt; ++j)
				{
					// Swap the two values
					f = pSrc[nCnt-1][j];
					pSrc[nCnt-1][j] = pSrc[i][j];
					pSrc[i][j] = f;
				}
			}

			// Now zero the last columns of the top rows
			for(j = 0; j < (nCnt-1); ++j)
			{
				//_ASSERT(pSrc[nCnt-1][nCnt] != F2X(0.0f));
				f = XDIV(pSrc[j][nCnt], pSrc[nCnt-1][nCnt]);

				// No need to actually calculate a zero for the final column
				for(k = 0; k < nCnt; ++k)
				{
					pSrc[j][k] -= XMUL(f, pSrc[nCnt-1][k]);
				}
			}

			break;
		}
	}

	// Solve the top-left sub matrix
	MatrixLinearEqSolveX(pRes, pSrc, nCnt - 1);

	// Now calc the solution for the bottom row
	f = pSrc[nCnt-1][0];
	for(k = 1; k < nCnt; ++k)
	{
		f -= XMUL(pSrc[nCnt-1][k], pRes[k-1]);
	}
	//_ASSERT(pSrc[nCnt-1][nCnt] != F2X(0));
	f = XDIV(f, pSrc[nCnt-1][nCnt]);
	pRes[nCnt-1] = f;
}
