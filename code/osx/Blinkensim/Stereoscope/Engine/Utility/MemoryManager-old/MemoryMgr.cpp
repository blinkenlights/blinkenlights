
#include "MemoryMgr.h"
//#include "HeapManager.h"
//#include "HeapFactory.h"
/*
void * operator new (size_t size) 
{
    return operator new (size, HeapFactory::GetDefaultHeap() );
}


void * operator new (size_t size, Heap * pHeap) 
{
    _ASSERT(pHeap != NULL);
    return pHeap->Allocate(size);
}


void operator delete (void * pMem) 
{
    if (pMem != NULL)
        Heap::Deallocate (pMem);    
}

*/
char* StrDup(const char *string)
{
	char* lpReturn=NULL;
    if (string) 
	{
        int wSize = strlen(string)+1;
        lpReturn = new char[wSize];
        if (lpReturn) 
            memcpy( lpReturn, string, wSize );
			
/*#ifdef DEBUG
	char strData[128];
  	sprintf(strData,"%5s %3d %5s", 
            "StrDup", wSize, (char *)string);
	LOG(string(strData), Logger::LOG_BLOK);
#endif
*/
	}
    return(lpReturn);
}





