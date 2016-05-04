#include "ftpserver.h"


BEGIN_MESSAGE_MAP(ftpserver, CWnd)	
	ON_WM_TIMER()
	ON_MESSAGE(WM_THREADSTART, OnThreadStart)
	ON_MESSAGE(WM_THREADCLOSE, OnThreadClose)
	ON_MESSAGE(WM_THREADMSG, OnThreadMessage)
END_MESSAGE_MAP()

ftpserver::ftpserver()
{
	m_nPort = 21;
	m_nMaxUsers = 10;
	m_strWelcomeMessage = "Welcome to Andrey Scherbin's FTP Server";
	m_strGoodbyeMessage = "Bye";
	m_nTimeout = 5;
	m_bRunning = FALSE;
	m_hWnd = NULL;
	m_nConnectionCount = 0;
	m_dwTotalSentBytes = 0;
	m_dwTotalReceivedBytes = 0;
	m_nTotalConnections = 0;
	m_nFilesDownloaded = 0;
	m_nFilesUploaded = 0;
	m_nFailedDownloads = 0;
	m_nFailedUploads = 0;
	m_nSecurityMode = 0;
	m_nStatisticsInterval = 0;

	m_UserManager.Serialize(FALSE);
}
bool ftpserver ::start()
{  
    char buff[1024];
	WSAStartup(0x0202,(WSADATA *) &buff[0]);
	if (m_bRunning)
		return FALSE;	
   
	if (m_ListenSocket.Create(m_nPort))
	{		
		if (m_ListenSocket.Listen())
		{
			m_ListenSocket.m_pWndServer = this;		
			printf("FTP SERVER started on port %d .\n", m_nPort);
			SetTimer(1, m_nStatisticsInterval, NULL);
			return TRUE;
		} 
	}	
	AddTraceLine(0, "FTP Server failed to listen on port %d.", m_nPort);
	return FALSE;	
}


LRESULT ftpserver::OnThreadStart(WPARAM wParam, LPARAM)
{
	m_nConnectionCount++;
	m_nTotalConnections++;
	ConnectThread *pThread = (ConnectThread *)wParam;

	UINT port;

	pThread->a_ConnectSocket.GetPeerName(pThread->m_strRemoteHost, port);
	printf("[%d] Client connected from %s.", pThread->m_nThreadID, pThread->m_strRemoteHost);

	return TRUE;
}


LRESULT ftpserver::OnThreadClose(WPARAM wParam, LPARAM lParam)
{
	m_nConnectionCount--;
	ConnectThread *pThread = (ConnectThread *)wParam;
	printf(0, "[%d] Client disconnected from %s.", pThread->m_nThreadID, pThread->m_strRemoteHost);	
	return TRUE;
}


LRESULT ftpserver::OnThreadMessage(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		case 0:
			m_dwTotalSentBytes += (int)lParam;
			break;
		case 1:
			m_dwTotalReceivedBytes += (int)lParam;
			break;
		case 2:
			switch(lParam)
			{
				case FTPSTAT_DOWNLOADSUCCEEDED:
					m_nFilesDownloaded++;
					break;
				case FTPSTAT_UPLOADSUCCEEDED:
					m_nFilesUploaded++;
					break;
				case FTPSTAT_DOWNLOADFAILED:
					m_nFailedDownloads++;
					break;
				case FTPSTAT_UPLOADFAILED:
					m_nFailedUploads++;
					break;
			}
			break;
		default:
			
			break;
	}
	return TRUE;
}
void ftpserver::OnTimer(UINT nIDEvent) 
{	
	CWnd::OnTimer(nIDEvent);
}

void ftpserver::AddTraceLine(int nType, LPCTSTR pstrFormat, ...)
{
	CString str;

	// format and write the data we were given
	va_list args;
	va_start(args, pstrFormat);
	str.FormatV(pstrFormat, args);	
	cout << str << "\n" ;
}






