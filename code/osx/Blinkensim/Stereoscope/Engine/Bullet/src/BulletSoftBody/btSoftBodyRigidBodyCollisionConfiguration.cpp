/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "btSoftBodyRigidBodyCollisionConfiguration.h"
#include "btSoftRigidCollisionAlgorithm.h"
#include "btSoftSoftCollisionAlgorithm.h"


btSoftBodyRigidBodyCollisionConfiguration::btSoftBodyRigidBodyCollisionConfiguration(btStackAlloc*	stackAlloc,btPoolAllocator*	persistentManifoldPool,btPoolAllocator*	collisionAlgorithmPool)
:btDefaultCollisionConfiguration(stackAlloc,persistentManifoldPool,collisionAlgorithmPool)
{
	void* mem;

	mem = btAlignedAlloc(sizeof(btSoftSoftCollisionAlgorithm::CreateFunc),16);
	m_softSoftCreateFunc = new(mem) btSoftSoftCollisionAlgorithm::CreateFunc;
	
	mem = btAlignedAlloc(sizeof(btSoftRigidCollisionAlgorithm::CreateFunc),16);
	m_softRigidCreateFunc = new(mem) btSoftRigidCollisionAlgorithm::CreateFunc;
	
	mem = btAlignedAlloc(sizeof(btSoftRigidCollisionAlgorithm::CreateFunc),16);
	m_swappedSoftRigidCreateFunc = new(mem) btSoftRigidCollisionAlgorithm::CreateFunc;
	m_swappedSoftRigidCreateFunc->m_swapped=true;

}

btSoftBodyRigidBodyCollisionConfiguration::~btSoftBodyRigidBodyCollisionConfiguration()
{
	m_softSoftCreateFunc->~btCollisionAlgorithmCreateFunc();
	btAlignedFree(	m_softSoftCreateFunc);

	m_softRigidCreateFunc->~btCollisionAlgorithmCreateFunc();
	btAlignedFree(	m_softRigidCreateFunc);

	m_swappedSoftRigidCreateFunc->~btCollisionAlgorithmCreateFunc();
	btAlignedFree(	m_swappedSoftRigidCreateFunc);


}
	
///creation of soft-soft and soft-rigid, and otherwise fallback to base class implementation
btCollisionAlgorithmCreateFunc* btSoftBodyRigidBodyCollisionConfiguration::getCollisionAlgorithmCreateFunc(int proxyType0,int proxyType1)
{

	///try to handle the softbody interactions first

	if ((proxyType0 == SOFTBODY_SHAPE_PROXYTYPE  ) && (proxyType1==SOFTBODY_SHAPE_PROXYTYPE))
	{
		return	m_softSoftCreateFunc;
	}

	///other can't be also softbody, so assume rigid for now
	if (proxyType0 == SOFTBODY_SHAPE_PROXYTYPE )
	{
		return	m_softRigidCreateFunc;
	}

	///other can't be also softbody, so assume rigid for now
	if (proxyType1 == SOFTBODY_SHAPE_PROXYTYPE )
	{
		return	m_swappedSoftRigidCreateFunc;
	}

	///fallback to the regular rigid collision shape
	return btDefaultCollisionConfiguration::getCollisionAlgorithmCreateFunc(proxyType0,proxyType1);
}