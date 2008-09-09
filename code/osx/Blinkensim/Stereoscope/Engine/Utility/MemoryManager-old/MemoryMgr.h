
#ifndef MEMORYMGR_H_
#define MEMORYMGR_H_

#include "Macros.h"
//#include "Log.h"

//#include "HeapFactory.h"
#include <cstddef>
#include <algorithm>
#include <iostream>
#include <cstddef>

// this is commented out because it seems like the file new for the 4.01 compiler already declares this
//void * operator new (size_t size);
/*
void * operator new (size_t size, Heap * pHeap);
void operator delete (void * pMem, size_t size); 
*/
using namespace std;

template <typename T>
T *reallocEM(T *array, size_t old_size, size_t new_size)
{
   T *temp = new T[new_size];

   delete [] array;
   
#ifdef DEBUG
	char strData[128];
  	sprintf(strData,"%5s %3d %5s %3d", 
            "New memory piece", (int)new_size, "Old memory piece", (int)old_size);
//	LOG(string(strData), Logger::LOG_BLOK);
#endif

   return copy(array, array + old_size, temp);
}


template <typename T>
bool SafeAlloc(T* &ptr, size_t cnt)
{
	_ASSERT(!ptr);
	if(cnt)
	{
		ptr = new T[cnt * sizeof(T)];
		_ASSERT(ptr);
		if(!ptr)
			return false;
	}
#ifdef DEBUG
	char strData[128];
  	sprintf(strData,"%5s %3d", 
            "Memory", (int)cnt);
//	LOG(string(strData), Logger::LOG_BLOK);
#endif

	return true;
}

template <typename T>
void SafeRealloc(T* &ptr, size_t cnt)
{
   // simulate what realloc would do
   // allocate enough so that the old and the new memory have space
   T *temp = new T[cnt * sizeof(T)];
   
   size_t old_size = sizeof(*ptr);
   
   // delete old memory
   delete [] ptr;
   
   // copy temp starting whereever ptr has started before
   copy(ptr, ptr + old_size, temp);

#ifdef DEBUG
	char strData[128];
  	sprintf(strData,"%5s %3d %5s %3d", 
            "New memory piece", (int)cnt, "Old memory piece", (int)old_size);
//	LOG(string(strData), Logger::LOG_BLOK);
#endif
 	_ASSERT(ptr);
}

char* StrDup(const char *string);

/*
#define DECLARE_HEAP \
    public: \
        static void * operator new(size_t size); \
        static void operator delete(void * p, size_t size); \
    private: \
        static Heap * s_pHeap; 


#define DEFINE_HEAP(className,heapName) \
    Heap * className::s_pHeap = NULL; \
    void * className::operator new(size_t size) { \
        if (s_pHeap==NULL)  \
            s_pHeap = HeapFactory::CreateHeap(heapName); \
        return ::operator new(size, s_pHeap); \
    } \
    void className::operator delete(void * p, size_t size) { \
        ::operator delete(p); \
    }


#define DEFINE_HIERARCHICALHEAP(className,heapName,parentName) \
    Heap * className::s_pHeap = NULL; \
    void * className::operator new(size_t size) { \
        if (s_pHeap==NULL)  \
            s_pHeap = HeapFactory::CreateHeap(heapName,parentName); \
        return ::operator new(size, s_pHeap); \
    } \
    void className::operator delete(void * p, size_t size) { \
        ::operator delete(p); \
    }
*/
#endif
