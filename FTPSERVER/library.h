#ifndef LIBRARY_H
#define LIBRARY_H


#include <afxwin.h>
#include <afxsock.h>
#include <stdio.h>
#include <conio.h>
#include <locale.h>
#include <iostream>
#include <afxmt.h>
#include <afxtempl.h>
#include <afxcmn.h>
#include <afxdtctl.h>	
#include <afxext.h>  
#pragma once
using namespace std;

#define MY_PORT    21

#define FTPSTAT_DOWNLOADSUCCEEDED	2
#define FTPSTAT_UPLOADSUCCEEDED		3
#define FTPSTAT_DOWNLOADFAILED		4
#define FTPSTAT_UPLOADFAILED		5

#define WM_THREADSTART	WM_USER+200
#define WM_THREADCLOSE	WM_USER+201
#define WM_THREADMSG	WM_USER+202
#define WM_ADDTRACELINE WM_USER+203
#define PACKET_SIZE 4096

BOOL MakeSureDirectoryPathExists(LPCTSTR lpszDirPath);
BOOL IsNumeric(char *buff);

#endif // LIBRARY_H

