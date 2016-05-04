#include "ftpserver.h"

  int main(int argc, char* argv[])
  {
	
    printf("FTP SERVER \n");
    ftpserver server;	
    server.start();
    HWND hwndMain; 
    HWND hwndDlgModeless = NULL; 
    MSG msg;
    BOOL bRet; 
    HACCEL haccel;
	CUser andrey("andrey","2");
	CDirectory way("C:","C:");
	andrey.m_DirectoryArray.Add(way);
	server.m_UserManager.m_UserArray.Add(andrey);
	
   while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
		
    if (bRet == -1)
    {
        // handle the error and possibly exit
    }
    else
    {
        if (hwndDlgModeless == (HWND) NULL || 
                !IsDialogMessage(hwndDlgModeless, &msg) && 
                !TranslateAccelerator(hwndMain, haccel, 
                    &msg)) 
        { 
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    } 
    } 
	while(1)
	{}
	return 0;
  }


