#pragma once
#include "library.h"
#include "ConnectSocket.h"
#include "ftpserver.h"
class ftpserver;
class ConnectThread : public CWinThread
{
	DECLARE_DYNCREATE(ConnectThread);
public:
    CWnd *m_pWndServer;	
    SOCKET a_Socket;
	ConnectSocket a_ConnectSocket;
	CTime m_LastDataTransferTime;
	CString m_strRemoteHost;
	ConnectThread(void);
	~ConnectThread(void);

	int m_nReceivedBytes;
	int m_nSentBytes;

    static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT uIDEvent, DWORD dwTime);

	void IncReceivedBytes(int nBytes);
	void IncSentBytes(int nBytes);

	void UpdateStatistic(int nType);

	virtual BOOL InitInstance();
	virtual int ExitInstance();	
protected:
	
	UINT m_nTimerID;	
	LRESULT OnThreadMessage(WPARAM wParam, LPARAM lParam);
	//DECLARE_MESSAGE_MAP()

};

