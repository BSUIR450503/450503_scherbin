#include "listensocket.h"
#include "ftpserver.h"
#include "ConnectThread.h"


ListenSocket::ListenSocket()
{
	m_pWndServer = NULL;
}

ListenSocket::~ListenSocket()
{
}

void ListenSocket::OnAccept(int nErrorCode) 
{	CSocket sockit;	
	Accept(sockit);	
	ConnectThread* pThread = (ConnectThread*)AfxBeginThread(RUNTIME_CLASS(ConnectThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		sockit.Close();		
		return;
	}
	ftpserver *pWnd = (ftpserver*)m_pWndServer;
	pWnd->m_CriticalSection.Lock();
    pWnd->m_ThreadList.AddTail(pThread);
	pWnd->m_CriticalSection.Unlock();	
	pThread->m_pWndServer = m_pWndServer;
    pThread->a_Socket = sockit.Detach();
	pThread->a_ConnectSocket.Attach(pThread->a_Socket);	
	pThread->ResumeThread();
	CAsyncSocket::OnAccept(nErrorCode);
}
