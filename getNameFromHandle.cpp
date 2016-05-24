#include <windows.h>
#include <tchar.h>
#include <Psapi.h>
#include "getNameFromHandle.hpp"



using std::string;

string GetFileNameFromHandle(HANDLE hFile) 
{
	TCHAR pszFilename[MAX_PATH+1];
	HANDLE hFileMap;
	

	string strFilename;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

	if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
	{     
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile, 
		NULL, 
		PAGE_READONLY,
		0, 
		1,
		NULL);

	if (hFileMap) 
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem) 
		{
			if (GetMappedFileName (GetCurrentProcess(), 
				pMem, 
				pszFilename,
				MAX_PATH)) 
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[1024];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(1024-1, szTemp)) 
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do 
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH) 
							{
								bFound = _tcsnicmp(pszFilename, szName, 
									uNameLen) == 0;

								if (bFound) 
								{
									strFilename.append(szDrive);
                                    strFilename.append(pszFilename+uNameLen);
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			UnmapViewOfFile(pMem);
		} 

		CloseHandle(hFileMap);
	}

	return strFilename;
}
