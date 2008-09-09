
#include "HeapFactory.h"
#include "HeapManager.h"
#include <cstddef>
#include <stdio.h>
#include <string.h>

#include "Log.h"

Heap * HeapFactory::s_pRootHeap    = NULL;
Heap * HeapFactory::s_pDefaultHeap = NULL;
Heap HeapFactory::s_heaps[];


void HeapFactory::Initialize()
{   
    for (int i=0; i < MAXHEAPS; ++i)
        s_heaps[i].Initialize ();

    s_pRootHeap    = CreateNewHeap("Root");
    s_pDefaultHeap = CreateHeap("Default");
}

Heap * HeapFactory::GetRootHeap ()
{
    if (s_pRootHeap == NULL)
        Initialize();
    return s_pRootHeap;
}


Heap * HeapFactory::GetDefaultHeap ()
{
    if (s_pDefaultHeap == NULL)
        Initialize();
    return s_pDefaultHeap;
}



Heap * HeapFactory::CreateHeap (const char * name, const char * parent)
{
    if (s_pRootHeap == NULL)
        Initialize();

    Heap * pParent = FindHeap(parent);
    if (pParent == NULL)
    {
        pParent = CreateNewHeap(parent);
        pParent->AttachTo (s_pRootHeap);
    }

    Heap * pHeap = FindHeap(name);
    if (pHeap == NULL)
        pHeap = CreateNewHeap(name);

    pHeap->AttachTo (pParent);
    return pHeap;
}



Heap * HeapFactory::CreateHeap (const char * name)
{
    return CreateHeap(name, "Root");
}


Heap * HeapFactory::FindHeap (const char * name)
{
    for (int i=0; i < MAXHEAPS; ++i)
    {
        Heap * pHeap = &s_heaps[i];
		// instead if stricmp we have to use strcasecmp here: both are case-insensitive string comparison
        if (pHeap->IsActive() && !strcasecmp(name, pHeap->GetName()))
            return pHeap;
    }

    return NULL;
}


Heap * HeapFactory::CreateNewHeap (const char * name)
{
    for (int i=0; i < MAXHEAPS; ++i)
    {
        Heap * pHeap = &s_heaps[i];
        if (!pHeap->IsActive())
        {
            pHeap->Activate (name);
            return pHeap;
        }
    }

    return NULL;
}



void HeapFactory::PrintInfo ()
{

    // printf ("MEMORY INFORMATION\n");
	INDENT("MEMORY INFORMATION", Logger::LOG_BLOK);
    // printf ("                            Local                 Total\n");
//	LOG("                              Local                Total ", Logger::LOG_BLOK);
	char strData[128];
	sprintf(strData,"%16s %9s %9s %5s  %9s %9s %5s", 
            " ", 
            " ", "Local", " ",
            " ", "Total", " ");
	LOG(string(strData), Logger::LOG_BLOK);


    // printf ("Name                  Memory  Peak  Inst   Memory  Peak  Inst\n");
	//LOG("Name                  Memory  Peak  Inst   Memory  Peak  Inst ", Logger::LOG_BLOK);
	//char strData[128];
	sprintf(strData,"%16s %9s %9s %5s  %9s %9s %5s", 
            "Heap Name", 
            "Memory", "Peak", "Inst",
            "Memory", "Peak", "Inst");
	LOG(string(strData), Logger::LOG_BLOK);


    GetRootHeap()->PrintTreeInfo(0);
    /*
    for (int i=0; i < MAXHEAPS; ++i)
    {
        if (s_heaps[i].IsActive())
            s_heaps[i].PrintInfo();
    }
    */
    //printf ("\n");
//	UNDENT("\n", Logger::LOG_BLOK);
}



int HeapFactory::GetMemoryBookmark () 
{
    return Heap::GetMemoryBookmark();
}


void HeapFactory::ReportMemoryLeaks (int nBookmark) 
{
    ReportMemoryLeaks (nBookmark, GetMemoryBookmark ());
}


void HeapFactory::ReportMemoryLeaks (int nBookmark1, int nBookmark2) 
{
	UNDENT("\n", Logger::LOG_BLOK);
    int nLeaks = 0;
    for (int i=0; i < MAXHEAPS; ++i)
        if (s_heaps[i].IsActive())
            nLeaks += s_heaps[i].ReportMemoryLeaks(nBookmark1, nBookmark2);

    if (nLeaks > 0)
	{
		char strData[128];
		sprintf(strData,"%d memory leaks detected", nLeaks);
		LOG(string(strData), Logger::LOG_CRIT);
	}
    else
		LOG("No memory leaks detected. ", Logger::LOG_DATA);
}
