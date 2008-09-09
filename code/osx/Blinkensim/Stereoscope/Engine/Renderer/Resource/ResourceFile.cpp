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
#include "ResourceFile.h"
#include <stdio.h>

#include "MemoryFile.h"

string CResourceFile::s_DataPath;

void CResourceFile::SetDataPath(const char* const pszDataPath)
{
	s_DataPath = (pszDataPath) ? pszDataPath : "";
}

string CResourceFile::GetDataPath()
{
	return string(s_DataPath);
}

CResourceFile::CResourceFile(const char* const pszFilename) :
	m_bOpen(false),
	m_bMemoryFile(false),
	m_Size(0),
	m_pData(0)
{
	string Path(s_DataPath);
	Path += pszFilename;

	FILE* pFile = fopen(Path.c_str(), "rb");
	if (pFile)
	{
		// Get the file size
		fseek(pFile, 0, SEEK_END);
		m_Size = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		// read the data, append a 0 byte as the data might represent a string
		char* pData = new char[m_Size + 1];
		pData[m_Size] = '\0';
		size_t BytesRead = fread(pData, 1, m_Size, pFile);

		if (BytesRead != m_Size)
		{
			delete [] pData;
			m_Size = 0;
		}
		else
		{
			m_pData = pData;
			m_bOpen = true;
		}
		fclose(pFile);
	}

#if defined(BUILD_OGLES2) || defined(BUILD_OGLES) || defined(BUILD_OGL)
	if (!m_bOpen)
	{
		m_bOpen = m_bMemoryFile = CMemoryFileSystem::GetFile(pszFilename, (const void**)(&m_pData), &m_Size);
	}
#endif
}

CResourceFile::~CResourceFile()
{
	Close();
}

bool CResourceFile::IsOpen() const
{
	return m_bOpen;
}

bool CResourceFile::IsMemoryFile() const
{
	return m_bMemoryFile;
}

size_t CResourceFile::Size() const
{
	return m_Size;
}

const void* CResourceFile::DataPtr() const
{
	return m_pData;
}

const char* CResourceFile::StringPtr() const
{
	return m_pData;
}

void CResourceFile::Close()
{
	if (m_bOpen)
	{
		if (!m_bMemoryFile)
		{
			delete [] (char*)m_pData;
		}
		m_bMemoryFile = false;
		m_bOpen = false;
		m_pData = 0;
		m_Size = 0;
	}
}

#if defined(BUILD_OGLES2) || defined(BUILD_OGLES) || defined(BUILD_OGL)



//
// class CMemoryFileSystem
//
CMemoryFileSystem::CAtExit CMemoryFileSystem::s_AtExit;
CMemoryFileSystem::SFileInfo* CMemoryFileSystem::s_pFileInfo = 0;
int CMemoryFileSystem::s_i32Capacity = 0;
int CMemoryFileSystem::s_i32NumFiles = 0;

//
// Workaround for platforms that
// don't support the atexit() function. This deletes any memory
// file system data.
//
CMemoryFileSystem::CAtExit::~CAtExit()
{
	for (int i = 0; i < CMemoryFileSystem::s_i32NumFiles; ++i)
	{
		if (CMemoryFileSystem::s_pFileInfo[i].bAllocated)
		{
			delete [] (char*)CMemoryFileSystem::s_pFileInfo[i].pszFilename;
			delete [] (char*)CMemoryFileSystem::s_pFileInfo[i].pBuffer;
		}
	}
	delete [] CMemoryFileSystem::s_pFileInfo;
}

CMemoryFileSystem::CMemoryFileSystem(const char* pszFilename, const void* pBuffer, size_t Size, bool bCopy)
{
	RegisterMemoryFile(pszFilename, pBuffer, Size, bCopy);
}

//
// pszFilename		Name of file to register
// pBuffer			Pointer to file data
// Size			File size
// bCopy			Name and data should be copied?
// Registers a block of memory as a file that can be looked up
// by name. 
//
void CMemoryFileSystem::RegisterMemoryFile(const char* pszFilename, const void* pBuffer, size_t Size, bool bCopy)
{
	if (s_i32NumFiles == s_i32Capacity)
	{
		SFileInfo* pFileInfo = new SFileInfo[s_i32Capacity + 10];
		memcpy(pFileInfo, s_pFileInfo, sizeof(SFileInfo) * s_i32Capacity);
		delete [] s_pFileInfo;
		s_pFileInfo = pFileInfo;
		s_i32Capacity += 10;
	}

	s_pFileInfo[s_i32NumFiles].pszFilename = pszFilename;
	s_pFileInfo[s_i32NumFiles].pBuffer = pBuffer;
	if (bCopy)
	{
		char* pszNewFilename = new char[strlen(pszFilename)];
		strcpy(pszNewFilename, pszFilename);
		s_pFileInfo[s_i32NumFiles].pszFilename = pszNewFilename;

		void* pszNewBuffer = new char[Size];
		memcpy(pszNewBuffer, pBuffer, Size);
		s_pFileInfo[s_i32NumFiles].pBuffer = pszNewBuffer;
	}
	s_pFileInfo[s_i32NumFiles].Size = Size;
	s_pFileInfo[s_i32NumFiles].bAllocated = bCopy;
	++s_i32NumFiles;
}

// 
// pszFilename		Name of file to open
// ppBuffer			Pointer to file data
// pSize			File size
// true if the file was found in memory, false otherwise
// Looks up a file in the memory file system by name. Returns a
// pointer to the file data as well as its size on success.
// 
bool CMemoryFileSystem::GetFile(const char* pszFilename, const void** ppBuffer, size_t* pSize)
{
	for (int i = 0; i < s_i32NumFiles; ++i)
	{
		if (strcmp(s_pFileInfo[i].pszFilename, pszFilename) == 0)
		{
			if (ppBuffer) *ppBuffer = s_pFileInfo[i].pBuffer;
			if (pSize) *pSize = s_pFileInfo[i].Size;
			return true;
		}
	}
	return false;
}

//
// The number of registered files
//
int CMemoryFileSystem::GetNumFiles()
{
	return s_i32NumFiles;
}

//
// i32Index		Index of file
// A pointer to the filename of the requested file
// Looks up a file in the memory file system by name. Returns a
// pointer to the file data as well as its size on success.
//
const char* CMemoryFileSystem::GetFilename(int i32Index)
{
	if (i32Index < 0 || i32Index > s_i32NumFiles) return 0;

	return s_pFileInfo[i32Index].pszFilename;
}

//std::string CMemoryFileSystem::DebugOut()
//{
//	std::stringstream Out;
//
//	Out << "MemoryFileSystem Info: Files - " << s_i32NumFiles << "\n";
//	for (int i = 0; i < s_i32NumFiles; ++i)
//	{
//		Out << "\t" << i << " " << s_pFileInfo[i].pszFilename << " (" 
//			<< s_pFileInfo[i].Size << ", " << (int)s_pFileInfo[i].pBuffer << ")\n";
//	}
//
//	return Out.str();
//}

#endif