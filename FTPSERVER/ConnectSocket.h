#pragma once
#include "library.h"
class DataSocket;
class ftpserver;
class ConnectSocket : public CSocket
{
	enum // Token ID's
	{
		TOK_ABOR, TOK_BYE, TOK_CDUP, TOK_CWD,
		TOK_DELE, TOK_DIR, TOK_HELP, TOK_LIST,
		TOK_MKD, TOK_NOOP, TOK_PASS, TOK_PASV, 
		TOK_PORT, TOK_PWD, TOK_QUIT, TOK_REST,
		TOK_RETR, TOK_RMD, TOK_RNFR, TOK_RNTO, 
		TOK_SIZE, TOK_STOR, TOK_SYST, TOK_TYPE, 
		TOK_USER, TOK_ERROR,
	};

public:
	DataSocket *a_DataSocket;
	int m_bPassiveMode;
	int m_nRemotePort;
	CString m_strRemoteHost;



	struct CFTPCommand
	{
		int m_nTokenID;
		char *m_pszName;
		BOOL m_bHasArguments;
		char *m_pszDescription;
	};

	
public:
	BOOL HasConnectionDropped(void);
	void FireStatusMessage(LPCTSTR lpszStatus, int nType);
	BOOL GetRxCommand(CString &command, CString &args);
	BOOL CreateDataConnection(int nTransferType, LPCTSTR lpszData);
	void DestroyDataConnection();

	ConnectSocket();
	virtual ~ConnectSocket();

	void ParseCommand();

	CWinThread *a_Thread;	
	ftpserver * server;

	BOOL m_bLoggedon;
	CString m_strUserName;
	
	BOOL SendResponse(LPCTSTR pstrFormat, ...);

	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnConnect(int nErrorCode);

	protected:
	CStringList m_strCommands;
	void GetRxLine();
	BOOL m_bRenameFile;
	DWORD m_dwRestartOffset;
	CString m_strRenameFile;
	CString m_RxBuffer;
	CString m_strCurrentDir;	
};

