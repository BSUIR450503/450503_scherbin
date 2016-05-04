#ifndef FTPSERVER_H
#define FTPSERVER_H

#include "library.h"
#include "listensocket.h"
#include "ConnectThread.h"
#include "UserManager.h"

#pragma once
#pragma warning 

class ConnectThread;
class ftpserver : public CWnd
{
	friend ConnectSocket;
public:
	CUserManager m_UserManager;
    int nclients;   
    ListenSocket m_ListenSocket;
	int		m_nTimeout;
	CCriticalSection m_CriticalSection;
    CTypedPtrList<CObList, ConnectThread*> m_ThreadList;
public:
    ftpserver();
    bool start();
	int GetTimeout() { return m_nTimeout; }

	int		m_nPort;
	int		m_nMaxUsers;
	CString	m_strWelcomeMessage;
	CString	m_strGoodbyeMessage;	
	BOOL	m_bRunning;

	DWORD m_dwTotalReceivedBytes;
	DWORD m_dwTotalSentBytes;
	int	m_nConnectionCount;
	int m_nTotalConnections;
	int m_nFilesDownloaded;
	int m_nFilesUploaded;
	int m_nFailedDownloads;
	int m_nFailedUploads;
	void AddTraceLine(int nType, LPCTSTR pstrFormat, ...);

	CString GetWelcomeMessage() { return m_strWelcomeMessage; };
	CString GetGoodbyeMessage() { return m_strGoodbyeMessage; };

protected:
	int m_nSecurityMode;
	int m_nStatisticsInterval;	
	afx_msg void OnTimer(UINT nIDEvent);	
	LRESULT OnThreadClose(WPARAM wParam, LPARAM lParam);
	LRESULT OnThreadStart(WPARAM wParam, LPARAM);
	LRESULT OnThreadMessage(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#endif // FTPSERVER_H