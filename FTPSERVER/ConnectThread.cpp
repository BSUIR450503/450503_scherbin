#include "ConnectThread.h"



IMPLEMENT_DYNCREATE(ConnectThread, CWinThread)

ConnectThread::ConnectThread(void)
{
	m_nReceivedBytes = 0;
	m_nSentBytes = 0;
	m_nTimerID = 0;
	m_LastDataTransferTime = CTime::GetCurrentTime();
}


ConnectThread::~ConnectThread(void)
{
}

BOOL ConnectThread::InitInstance()
{	try
	{
	ftpserver *pWnd = (ftpserver *)m_pWndServer;
	a_ConnectSocket.server = pWnd;
    a_ConnectSocket.a_Thread = this;	
	a_ConnectSocket.SendResponse("220 %s", ((ftpserver *)m_pWndServer)->GetWelcomeMessage());
	m_nTimerID = ::SetTimer(NULL, 0, 3000, TimerProc); 	
	}
	catch(CException *e) 
	{
		e->Delete();		
	}
	return TRUE;	
}

int ConnectThread::ExitInstance()
{  
	ftpserver *pWnd = (ftpserver *)m_pWndServer;
	try
	{		pWnd->m_CriticalSection.Lock();		
		
		POSITION pos = pWnd->m_ThreadList.Find(this);
		if(pos != NULL)
		{
			pWnd->m_ThreadList.RemoveAt(pos);			
		}
		pWnd->m_CriticalSection.Unlock();	
	}
	catch(CException *e) 
	{
		pWnd->m_CriticalSection.Unlock();
		e->Delete();
	}
	return CWinThread::ExitInstance();
}


 VOID CALLBACK ConnectThread::TimerProc(HWND hwnd, UINT uMsg, UINT uIDEvent, DWORD dwTime)
{  
	ConnectThread *pThread = (ConnectThread *)AfxGetThread();

	if (uIDEvent == pThread->m_nTimerID)
	{
		int nConnectionTimeout = ((ftpserver *)pThread->m_pWndServer)->GetTimeout();	
		CTime time = pThread->m_LastDataTransferTime;
		time += CTimeSpan(0, 0, nConnectionTimeout, 0);
		if (time < CTime::GetCurrentTime())
		{
			pThread->a_ConnectSocket.SendResponse((LPCTSTR)"426 Connection timed out, aborting transfer");
			pThread->PostThreadMessage(WM_QUIT,0,0);
		}
	}
} 

 void ConnectThread::IncSentBytes(int nBytes)
{
	m_LastDataTransferTime = CTime::GetCurrentTime();
	m_nSentBytes += nBytes;	
	m_pWndServer->PostMessage(WM_THREADMSG, (WPARAM)0, (LPARAM)nBytes);
}


void ConnectThread::IncReceivedBytes(int nBytes)
{
	m_LastDataTransferTime = CTime::GetCurrentTime();
	m_nReceivedBytes += nBytes;
	m_pWndServer->PostMessage(WM_THREADMSG, (WPARAM)1, (LPARAM)nBytes);
}


void ConnectThread::UpdateStatistic(int nType)
{	
	m_pWndServer->PostMessage(WM_THREADMSG, (WPARAM)2, (LPARAM)nType);
}

/*BEGIN_MESSAGE_MAP(ConnectThread, CWinThread)

	ON_MESSAGE(WM_THREADMSG, OnThreadMessage)
END_MESSAGE_MAP()*/


LRESULT ConnectThread::OnThreadMessage(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		case 0: // destroy data socket
			a_ConnectSocket.DestroyDataConnection();
			break;
		case 1: // quit !
			PostThreadMessage(WM_QUIT,0,0);
			break;
		default:
			break;
	}
	return 0L;
}

