
#pragma once

class ConnectSocket;

#define XFERMODE_IDLE	0
#define XFERMODE_LIST	1
#define XFERMODE_SEND	2
#define XFERMODE_RECEIVE 3
#define XFERMODE_ERROR	4

class DataSocket : public CAsyncSocket
{
public:
	DataSocket(ConnectSocket *pSocket, int nTransferType = 0);
	virtual ~DataSocket();
public:
	void SetRestartOffset(DWORD dwOffset);
	void SetTransferType(int nType, BOOL bWaitForAccept = FALSE);
	void SetData(LPCTSTR lpszData);
	CFile m_File;
	int GetStatus();

	public:
	virtual void OnSend(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnAccept(int nErrorCode);

protected:
	DWORD m_dwRestartOffset;
	BOOL m_bConnected;
	BOOL m_bInitialized;
	int Receive();
	BOOL PrepareReceiveFile(LPCTSTR lpszFilename);
	BOOL PrepareSendFile(LPCTSTR lpszFilename);
	DWORD m_nTotalBytesTransfered;
	DWORD m_nTotalBytesReceive;
	DWORD m_nTotalBytesSend;
	int m_nTransferType;

	CString m_strData;
	int m_nStatus;
	ConnectSocket *m_pConnectSocket;
};

