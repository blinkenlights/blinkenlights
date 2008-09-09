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
#ifndef VECTOR_H_
#define VECTOR_H_

typedef struct _VECTOR2F
{
	float x;	/*!< x coordinate */
	float y;	/*!< y coordinate */
   inline void set(float _x, float _y) { x = _x; y = _y; }
} VECTOR2f;


typedef struct
{
	int x;	/*!< x coordinate */
	int y;	/*!< y coordinate */
} VECTOR2x;

typedef struct _VECTOR3f
{
	float x;	/*!< x coordinate */
	float y;	/*!< y coordinate */
	float z;	/*!< z coordinate */
   inline void set(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
   inline const _VECTOR3f operator-(const _VECTOR3f& rhs) const{ _VECTOR3f tmp = { x - rhs.x, y - rhs.y, z - rhs.z }; return tmp; }
   inline const _VECTOR3f operator+(const _VECTOR3f& rhs) const{ _VECTOR3f tmp = { x + rhs.x, y + rhs.y, z + rhs.z }; return tmp; }
   inline const _VECTOR3f operator*(float rhs) const { _VECTOR3f tmp = { x * rhs, y * rhs, z * rhs }; return tmp; }
   inline float lenSquared() const { return x*x + y*y + z*z; }
} VECTOR3f;

typedef struct
{
	int x;	/*!< x coordinate */
	int y;	/*!< y coordinate */
	int z;	/*!< z coordinate */
} VECTOR3x;


typedef struct
{
	float x;	/*!< x coordinate */
	float y;	/*!< y coordinate */
	float z;	/*!< z coordinate */
	float w;	/*!< w coordinate */
} VECTOR4f;


typedef struct
{
	int x;	/*!< x coordinate */
	int y;	/*!< y coordinate */
	int z;	/*!< z coordinate */
	int w;	/*!< w coordinate */
} VECTOR4x;


#ifdef FIXEDPOINTENABLE
typedef VECTOR2x     VECTOR2;
typedef VECTOR3x     VECTOR3;
typedef VECTOR4x     VECTOR4;
#else
typedef VECTOR2f     VECTOR2;
typedef VECTOR3f     VECTOR3;
typedef VECTOR4f     VECTOR4;
#endif


#endif // VECTOR_H_