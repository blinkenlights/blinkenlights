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
#ifndef RESOURCEFILE_H_
#define RESOURCEFILE_H_

#include <stdlib.h>
#include <string>
using namespace std;


class CResourceFile
{
public:
	static void SetDataPath(const char* pszDataPath);
	static string GetDataPath();

	CResourceFile(const char* pszFilename);
	virtual ~CResourceFile();

	bool IsOpen() const;
	bool IsMemoryFile() const;
	size_t Size() const;

	const void* DataPtr() const;
	
	// returns a null-terminated buffer.
	const char* StringPtr() const;

	void Close();

protected:
	bool m_bOpen;
	bool m_bMemoryFile;
	size_t m_Size;
	const char* m_pData;

	static string s_DataPath;
};

#endif // RESOURCEFILE_H_
