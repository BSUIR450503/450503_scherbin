#include "library.h"

BOOL MakeSureDirectoryPathExists(LPCTSTR lpszDirPath)
{
	CString strDirPath = lpszDirPath;
	
	int nPos = 0;
   
	while((nPos = strDirPath.Find('\\', nPos+1)) != -1) 
	{
		CreateDirectory(strDirPath.Left(nPos), NULL);
	}
	return CreateDirectory(strDirPath, NULL);
}

BOOL IsNumeric(char *buff)
{
	// validate data
	char *ptr = buff;
	while(*ptr)
	{
		if (isdigit(*ptr))
		{
			ptr++;
		}
		else
			return FALSE;
	}
	return TRUE;
}