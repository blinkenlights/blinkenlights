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

#ifndef FRUSTUM_H_
#define FRUSTUM_H_

#include "Mathematics.h"
#include "Plane.h"

// 
// Plane class
//
class CFrustum
{
	CFrustum(){}
	~CFrustum(){}
	
public:
	CPlane& GetNearPlane()
	{
		return Plane[4];
	}
	CPlane& GetFarPlane()
	{
		return Plane[5];
	}	
	
	CPlane& GetTopPlane()
	{
		return Plane[2];
	}	
	
	CPlane& GetBottomPlane()
	{
		return Plane[3];
	}	
	
	CPlane& GetLeftPlane()
	{
		return Plane[0];
	}	
	
	CPlane& GetRightPlane()
	{
		return Plane[1];
	}	
		
	// extract planes from view frustum == projection matrix
	// perspective projection only
	void ExtractPlanes(const MATRIX & comboMatrix, bool normalize);
	
private:
	CPlane Plane[6];
};

#endif // FRUSTUM_H_