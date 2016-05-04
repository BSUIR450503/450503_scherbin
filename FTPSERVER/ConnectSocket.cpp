#include "library.h"
#include "ftpserver.h"
#include "ConnectSocket.h"
#include "ConnectThread.h"
#include "DataSocket.h"


ConnectSocket::ConnectSocket()
{
	m_bLoggedon = FALSE;
	m_bRenameFile = FALSE;
	a_DataSocket = NULL;
	m_strRemoteHost = "";
	m_nRemotePort = -1;
	m_dwRestartOffset = 0;
	m_bPassiveMode = FALSE;			
		
}


ConnectSocket::~ConnectSocket()
{
	DestroyDataConnection();

	// tell our thread we have been closed
	AfxGetThread()->PostThreadMessage(WM_QUIT,0,0);

	TRACE0("CConnectSocket destroyed.\n");
}





void ConnectSocket::OnClose(int nErrorCode) 
{
	Close();
	// destroy connection
	a_Thread->PostThreadMessage(WM_THREADMSG, 1, 0);
	TRACE("CConnectSocket() OnClose()\n");

	CSocket::OnClose(nErrorCode);
}


#define BUFFERSIZE 4096

void ConnectSocket::OnReceive(int nErrorCode) 
{
	TCHAR buff[BUFFERSIZE];

	int nRead = Receive(buff, BUFFERSIZE);
	switch (nRead)
	{
		case 0:
			Close();
			break;

		case SOCKET_ERROR:
			if (GetLastError() != WSAEWOULDBLOCK) 
			{
				TCHAR szError[256];
				wsprintf(szError, "OnReceive error: %d", GetLastError());
				
				AfxMessageBox (szError);
			}
			break;

		default:
			if (nRead != SOCKET_ERROR && nRead != 0)
			{
				//((ConnectThread *)AfxGetThread())->IncReceivedBytes(nRead);

				// terminate the string
				buff[nRead] = 0; 
				m_RxBuffer += CString(buff);
				
				GetRxLine();
			}	
			break;
	}
	CSocket::OnReceive(nErrorCode);
}

BOOL ConnectSocket::GetRxCommand(CString &strCommand, CString &strArguments)
{
	if (!m_strCommands.IsEmpty())
	{
		CString strBuff = m_strCommands.RemoveHead();
		int nIndex = strBuff.Find(" ");
		if (nIndex != -1)
		{
			CString strPassword = strBuff;
			strPassword.MakeUpper();
			// make password invisible
			if (strPassword.Left(5) == "PASS ")
			{
				for (int i=5; i < strPassword.GetLength(); i++)
				{
					strPassword.SetAt(i, '*');
				}
				FireStatusMessage(strPassword, 1);
			}
			else
			{
				FireStatusMessage(strBuff, 1);
			}
			strCommand = strBuff.Left(nIndex);
			strArguments = strBuff.Mid(nIndex+1);
		}
		else
		{
			FireStatusMessage(strBuff, 1);
			strCommand = strBuff;
		}

		if (strCommand != "")
		{
			strCommand.MakeUpper();

			// who screwed up ???
			if (strCommand.Right(4) == "ABOR")
			{
				strCommand = "ABOR";
			}
			
			TRACE2("COMMAND: %s, ARGS: %s\n", strCommand, strArguments);
			return TRUE;
		}
	}
	return FALSE;
}

void ConnectSocket::GetRxLine()
{
	CString strTemp;
	int nIndex;

	while(!m_RxBuffer.IsEmpty())
	{
		nIndex = m_RxBuffer.Find("\r\n");
		if (nIndex != -1)
		{
			strTemp = m_RxBuffer.Left(nIndex);
			m_RxBuffer = m_RxBuffer.Mid(nIndex + 2);
			if (!strTemp.IsEmpty())
			{
				m_strCommands.AddTail(strTemp);
				// parse and execute command
				ParseCommand();
			}
		}
		else
			break;
	}
}

void ConnectSocket::OnConnect(int nErrorCode) 
{
	CSocket::OnConnect(nErrorCode);
}


BOOL ConnectSocket::HasConnectionDropped(void)
{
	BOOL bConnDropped = FALSE;
	INT iRet = 0;
	BOOL bOK = TRUE;
	
	if (m_hSocket == INVALID_SOCKET)
		return TRUE;

	struct timeval timeout = { 0, 0 };
	fd_set readSocketSet;
	
	FD_ZERO(&readSocketSet);
	FD_SET(m_hSocket, &readSocketSet);
	
	iRet = ::select(0, &readSocketSet, NULL, NULL, &timeout);
	bOK = (iRet > 0);
	
	if(bOK)
	{
		bOK = FD_ISSET(m_hSocket, &readSocketSet);
	}
	
	if(bOK)
	{
		CHAR szBuffer[1] = "";
		iRet = ::recv(m_hSocket, szBuffer, 1, MSG_PEEK);
		bOK = (iRet > 0);
		if(!bOK)
		{
			INT iError = ::WSAGetLastError();
			bConnDropped = (( iError == WSAENETRESET) ||
				(iError == WSAECONNABORTED) ||
				(iError == WSAECONNRESET) ||
				(iError == WSAEINVAL) ||
				(iRet == 0));
		}
	}
    return(bConnDropped);
}

BOOL ConnectSocket::SendResponse(LPCTSTR pstrFormat, ...)
{
	CString str;

	// format arguments and put them in CString
	va_list args;
	va_start(args, pstrFormat);
	str.FormatV(pstrFormat, args);

	// is connection still active ?
	if (HasConnectionDropped())
	{
		FireStatusMessage("Could not send reply, disconnected.", 2);	

		Close();
		// tell our thread we have been closed
		
		// destroy connection
		a_Thread->PostThreadMessage(WM_THREADMSG, 1, 0);
		return FALSE;
	}

	int nBytes = CSocket::Send(str + "\r\n", str.GetLength()+2);
	if (nBytes == SOCKET_ERROR)
	{
		Close();
		FireStatusMessage("Could not send reply, disconnected.", 2);	

		// tell our thread we have been closed
		a_Thread->PostThreadMessage(WM_THREADMSG, 1, 0);

		return FALSE;
	}

	FireStatusMessage(str, 2);
	
	
	return TRUE;
}

void ConnectSocket::ParseCommand()
{
	static CFTPCommand commandList[] = 
	{
		{TOK_ABOR,	"ABOR", FALSE,	"Abort transfer: ABOR"}, 
		{TOK_BYE,	"BYE",  FALSE,	"Logout or break the connection: BYE"},
		{TOK_CDUP,	"CDUP", FALSE,	"Change to parent directory: CDUP"},
		{TOK_CWD,	"CWD",	TRUE,	"Change working directory: CWD [directory-name]"},
		{TOK_DELE,	"DELE", TRUE ,	"Delete file: DELE file-name"},
		{TOK_DIR,	"DIR",  FALSE,	"Get directory listing: DIR [path-name]"},
		{TOK_HELP,	"HELP",  FALSE, "Show help: HELP [command]"},
		{TOK_LIST,	"LIST", FALSE,	"Get directory listing: LIST [path-name]"}, 
		{TOK_MKD,	"MKD",	TRUE,	"Make directory: MKD path-name"},
		{TOK_NOOP,	"NOOP", FALSE,	"Do nothing: NOOP"},
		{TOK_PASS,	"PASS", TRUE,	"Supply a user password: PASS password"},
		{TOK_PASV,	"PASV", FALSE,	"Set server in passive mode: PASV"},
		{TOK_PORT,	"PORT", TRUE,	"Specify the client port number: PORT a0,a1,a2,a3,a4,a5"},
		{TOK_PWD,	"PWD",	FALSE,	"Get current directory: PWD"},
		{TOK_QUIT,	"QUIT",  FALSE,	"Logout or break the connection: QUIT"},
		{TOK_REST,	"REST", TRUE,	"Set restart transfer marker: REST marker"},
		{TOK_RETR,	"RETR", TRUE,	"Get file: RETR file-name"},
		{TOK_RMD,	"RMD",	TRUE,	"Remove directory: RMD path-name"},
		{TOK_RNFR,	"RNFR", TRUE,	"Specify old path name of file to be renamed: RNFR file-name"},
		{TOK_RNTO,	"RNTO", TRUE,	"Specify new path name of file to be renamed: RNTO file-name"},
		{TOK_SIZE,	"SIZE", TRUE,	"Get filesize: SIZE file-name"},
		{TOK_STOR,	"STOR", TRUE,	"Store file: STOR file-name"},
		{TOK_SYST,	"SYST", FALSE,	"Get operating system type: SYST"},
		{TOK_TYPE,	"TYPE", TRUE,	"Set filetype: TYPE [A | I]"},
		{TOK_USER,	"USER", TRUE,	"Supply a username: USER username"},
		{TOK_ERROR,	"",		FALSE,  ""},
	};
	
	// parse command
	CString strCommand, strArguments;
	if (!GetRxCommand(strCommand, strArguments))
	{
		return;
	}

	int nCommand;

	// find command in command list
	for (nCommand = TOK_ABOR; nCommand < TOK_ERROR; nCommand++)
	{
		// found command ?
		if (strCommand == commandList[nCommand].m_pszName)
		{
			// did we expect an argument ?
			if (commandList[nCommand].m_bHasArguments && (strArguments.IsEmpty()))
			{
				SendResponse("501 Syntax error: Invalid number of parameters.");
				return;
			}
			break;			
		}
	}

	if (nCommand == TOK_ERROR)
	{
		// command is not in our list
		SendResponse("501 Syntax error: Command not understood.");
		return;
	}
	
	// no commands are excepted before successfull logged on
	if ((nCommand != TOK_USER && nCommand != TOK_PASS) && !m_bLoggedon)
	{
		SendResponse("530 Please login with USER and PASS.");
		return;
	}

	// proces command
	switch(nCommand)
	{
		// specify username
		case TOK_USER:
		{
			strArguments.MakeLower();
			m_bLoggedon = FALSE;
			m_strUserName = strArguments;

			CString strPeerAddress;
			UINT nPeerPort;
			GetPeerName(strPeerAddress, nPeerPort);

			// tell FTP server a new user has connected
			ConnectThread *pThread = (ConnectThread *)a_Thread;			
			printf("[%d] %s %s\n" ,a_Thread->m_nThreadID, m_strUserName, strPeerAddress);
			SendResponse("331 User name ok, need password.");
		}
		break;

		// specify password
		case TOK_PASS:
		{
			// already logged on ?
			if (m_bLoggedon)
			{
				SendResponse("503 Login with USER first.");
			}
			else
			{
				// now we have user name and password, attempt to login the client
				CUser user;
				// check username
				if (server->m_UserManager.GetUser(m_strUserName, user))
				{
					// check password
					if ((!user.m_strPassword.Compare(strArguments) || user.m_strPassword.IsEmpty()) && !user.m_bAccountDisabled)
					{
						// set home directory of user
						m_strCurrentDir = "/";

						// succesfully logged on
						m_bLoggedon = TRUE;
						SendResponse("230 User successfully logged in.");
						break;
					}
				}
				SendResponse("530 Not logged in, user or password incorrect!");
			}
		}
		break;
		
		// change transfer type
		case TOK_TYPE:
		{
			// let's pretend we did something...
			SendResponse("200 Type set to %s", strArguments);
		}
		break;

		// print current directory
		case TOK_PWD:
		{
			SendResponse("257 \"%s\" is current directory.", m_strCurrentDir);
		}
		break;

		// change to parent directory
		case TOK_CDUP:
			strArguments = "..";
		// change working directory
		case TOK_CWD:
		{
			// try to change to specified directory
			int nResult = server->m_UserManager.ChangeDirectory(m_strUserName, m_strCurrentDir, strArguments);
			switch(nResult)
			{
				case 0:
					SendResponse("250 CWD command successful. \"%s\" is current directory.", m_strCurrentDir);
					break;
				case 1:
					SendResponse("550 CWD command failed. \"%s\": Permission denied.", strArguments);
					break;
				default:
					SendResponse("550 CWD command failed. \"%s\": Directory not found.", strArguments);
					break;
			}
		}
		break; 

		// specify IP and port (PORT a1,a2,a3,a4,p1,p2) -> IP address a1.a2.a3.a4, port p1*256+p2. 
		case TOK_PORT:
		{
			CString strSub;
			int nCount=0;

			while (AfxExtractSubString(strSub, strArguments, nCount++, ','))
			{
				switch(nCount)
				{
					case 1:	// a1
						m_strRemoteHost = strSub;
						m_strRemoteHost += ".";
						break;
					case 2:	// a2
						m_strRemoteHost += strSub;
						m_strRemoteHost += ".";
						break;
					case 3:	// a3
						m_strRemoteHost += strSub;
						m_strRemoteHost += ".";
						break;
					case 4:	// a4
						m_strRemoteHost += strSub;
						break;
					case 5:	// p1
						m_nRemotePort = 256*atoi(strSub);
						break;
					case 6:	// p2
						m_nRemotePort += atoi(strSub);
						break;
				}
			}
			m_bPassiveMode = FALSE;
			SendResponse("200 Port command successful.");
			break;
		}
		
		// switch to passive mode
		case TOK_PASV:
		{
			// delete existing datasocket
			DestroyDataConnection();

			// create new data socket
			a_DataSocket = new DataSocket(this, -1);

			if (!a_DataSocket->Create())
			{
				DestroyDataConnection();	
				SendResponse("421 Failed to create socket.");
				break;
			}
			// start listening
			a_DataSocket->Listen();
			a_DataSocket->AsyncSelect();
			
			CString strIP, strTmp;
			UINT nPort;
			
			// get our ip address
			GetSockName(strIP, nPort);
			// retrieve port
			a_DataSocket->GetSockName(strTmp, nPort);
			// replace dots 
			strIP.Replace(".",",");
			// tell the client which address/port to connect to
			SendResponse("227 Entering Passive Mode (%s,%d,%d).", strIP, nPort/256, nPort%256);
			m_bPassiveMode = TRUE;
			break;
		} 

		// list current directory (or a specified file/directory)
		case TOK_LIST:
		case TOK_DIR:
		{
			// if not PASV mode, we need a PORT comand first
			if(!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
			{
				SendResponse("503 Bad sequence of commands.");
			}
			else
			{
				// if client did not specify a directory use current dir
				if (strArguments == "")
				{
					strArguments = m_strCurrentDir;
				}
				else
				{
					// check if argument is file or directory
					CString strResult;
					int nResult =server->m_UserManager.CheckFileName(m_strUserName, strArguments, m_strCurrentDir, FTP_LIST, strResult);
					if (nResult == 0)
					{
						strArguments = strResult;
					}
				}
				CString strListing;
				int nResult = server->m_UserManager.GetDirectoryList(m_strUserName, strArguments, strListing);
				switch(nResult)
				{
					case 1:
						SendResponse("550 Permission denied.");
						break;
					case 2:
						SendResponse("550 Directory not found.");
						break;
					default:
						// create socket connection to transfer directory listing
						if (!CreateDataConnection(0, strListing))
						{
							DestroyDataConnection();
						}
						break;
				}
			} 
			break;
		} 
		
		// retrieve file
		case TOK_RETR:
		{
			// if not PASV mode, we need a PORT comand first
			if(!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
			{
				SendResponse("503 Bad sequence of commands.");
				break;
			}
			
			CString strResult;
			int nResult = server->m_UserManager.CheckFileName(m_strUserName, strArguments, m_strCurrentDir, FTP_DOWNLOAD, strResult);
			switch(nResult)
			{
				case 1:
					SendResponse("550 Permission denied.");
					break;
				case 2:
					SendResponse("550 File not found.");
					break;
				default:
					// create socket connection for file transfer
					if (!CreateDataConnection(1, strResult))
					{
						DestroyDataConnection();
					}
					break;
			}
			break;	
		}

		// client wants to upload file
		case TOK_STOR:
		{
			// if not PASV mode, we need a PORT comand first
			if(!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
			{
				SendResponse("503 Bad sequence of commands.");
				break;
			}
			
			CString strResult;
			int nResult = server->m_UserManager.CheckFileName(m_strUserName, strArguments, m_strCurrentDir, FTP_UPLOAD, strResult);
			switch(nResult)
			{
				case 1:
					SendResponse("550 Permission denied.");
					break;
				case 2:
					SendResponse("550 Filename invalid.");
					break;
				default:
					// create socket connection for file transfer
					if (!CreateDataConnection(2, strResult))
					{
						DestroyDataConnection();
					}
					break;
			}
		}
		break;
		
		// get file size
		case TOK_SIZE:
		{
			CString strResult;
			int nResult = server->m_UserManager.CheckFileName(m_strUserName, strArguments, m_strCurrentDir, FTP_DOWNLOAD, strResult);
			switch(nResult)
			{
				case 1:
					SendResponse("550 Permission denied.");
					break;
				case 2:
					SendResponse("550 File not found.");
					break;
				default:
				{
					CFileStatus status;
					CFile::GetStatus(strResult, status);
					SendResponse("213 %d", status.m_size);
					break;
				}
			}
		}
		break;
		
		// delete file
		case TOK_DELE:
		{
			CString strResult;
			int nResult = server->m_UserManager.CheckFileName(m_strUserName, strArguments, m_strCurrentDir, FTP_DELETE, strResult);
			switch(nResult)
			{
				case 1:
					SendResponse("550 Permission denied.");
					break;
				case 2:
					SendResponse("550 File not found.");
					break;
				default:
					// delete the file
					if (!DeleteFile(strResult))
					{
						SendResponse("450 Internal error deleting the file: \"%s\".",  strArguments);
					}
					else
					{
						SendResponse("250 File \"%s\" was deleted successfully.", strArguments);
					}
					break;
			}
		}
		break;
		
		// remove directory
		case TOK_RMD: 
		{
			CString strResult;
			int nResult = server->m_UserManager.CheckDirectory(m_strUserName, strArguments, m_strCurrentDir, FTP_DELETE, strResult);
			switch(nResult)
			{
				case 1:
					SendResponse("550 Permission denied.");
					break;
				case 2:
					SendResponse("550 Directory not found.");
					break;
				default:
					// remove the directory
					if (!RemoveDirectory(strResult))
					{
						if (GetLastError() == ERROR_DIR_NOT_EMPTY)
						{
							SendResponse("550 Directory not empty.");
						}
						else
						{
							SendResponse("450 Internal error deleting the directory.");
						}
					}
					else
					{
						SendResponse("250 Directory deleted successfully.");
					}
					break;
			}
		}
		break;	
		
		// create directory
		case TOK_MKD: 
		{
			CString strResult;
			int nResult = server->m_UserManager.CheckDirectory(m_strUserName, strArguments, m_strCurrentDir, FTP_CREATE_DIR, strResult);
			switch(nResult)
			{
				case 0:
					SendResponse("550 Directory already exists.");
					break;
				case 1:
					SendResponse("550 Can't create directory. Permission denied.");
					break;
				default:
					// create directory structure
					if (!MakeSureDirectoryPathExists(strResult))
					{
						SendResponse("450 Internal error creating the directory.");
					}
					else
					{
						SendResponse("250 Directory created successfully.");
					}
					break;
			}
		}
		break;
		
		// rename file or directory (part 1)
		case TOK_RNFR:
		{
			CString strResult;
			int nResult = server->m_UserManager.CheckFileName(m_strUserName, strArguments, m_strCurrentDir, FTP_RENAME, strResult);
			if (nResult == 0)
			{
				m_strRenameFile = strResult;
				m_bRenameFile = TRUE;
				SendResponse("350 File exists, ready for destination name.");
				break;
			}
			else
			{
				// client wants to rename directory
				nResult = server->m_UserManager.CheckDirectory(m_strUserName, strArguments, m_strCurrentDir, FTP_RENAME, strResult);
				switch(nResult)
				{
					case 0:
						m_strRenameFile = strResult;
						m_bRenameFile = FALSE;
						SendResponse("350 Directory exists, ready for destination name.");
						break;
					case 1:
						SendResponse("550 Permission denied.");
						break;
					default: 
						SendResponse("550 File/directory not found.");
						break;
				}
			}	
		}
		break;

		// rename file or directory (part 2)
		case TOK_RNTO:
		{
			if (m_strRenameFile.IsEmpty())
			{
				SendResponse("503 Bad sequence of commands.");
				break;
			}
			if (m_bRenameFile)
			{
				CString strResult;
				// check destination filename
				int nResult = server->m_UserManager.CheckFileName(m_strUserName, strArguments, m_strCurrentDir, FTP_RENAME, strResult);
				switch(nResult)
				{
					case 0:
						SendResponse("550 File already exists.");
						break;
					case 1:
						SendResponse("550 Permission denied.");
						break;
					default:
						// rename file
						if (!MoveFile(m_strRenameFile, strResult))
						{
							SendResponse("450 Internal error renaming the file: \"%s\".", m_strRenameFile);
						}
						else
						{
							SendResponse("250 File \"%s\" renamed successfully.", m_strRenameFile);
						}
						break;
				}
			}
			else
			{
				CString strResult;
				// check destination directory name
				int nResult = server->m_UserManager.CheckDirectory(m_strUserName, strArguments, m_strCurrentDir, FTP_RENAME, strResult);
				switch(nResult)
				{
					case 0:
						SendResponse("550 Directory already exists.");
						break;
					case 1:
						SendResponse("550 Permission denied.");
						break;
					case 3:
						SendResponse("550 Directory invalid.");
						break;
					default:
						// rename directory
						if (!MoveFile(m_strRenameFile, strResult))
						{
							SendResponse("450 Internal error renaming the directory: \"%s\".", m_strRenameFile);
						}
						else
						{
							SendResponse("250 Directory \"%s\" renamed successfully.", m_strRenameFile);
						}
						break;
				}
			}		
		}
		break;

		// abort transfer
		case TOK_ABOR:
		{
			if (a_DataSocket)
			{
				if (a_DataSocket->GetStatus() != XFERMODE_IDLE)
				{
					SendResponse("426 Data connection closed.");
				}
				// destroy data connection
				a_Thread->PostThreadMessage(WM_THREADMSG, 0, 0);
			}
			SendResponse("226 ABOR command successful.");
			break;
		} 

		// get system info
		case TOK_SYST:
			SendResponse("215 UNIX emulated by Pablo's FTP Server.");
			break;
		
		// close connection
		case TOK_QUIT:
		case TOK_BYE:
		{
			// send goodbye message to client
			ConnectThread *pThread = (ConnectThread *)a_Thread;
			SendResponse("220 %s", ((ftpserver *)pThread->m_pWndServer)->GetGoodbyeMessage());

			Close();
			
			// tell our thread we have been closed
		
			// destroy connection
			a_Thread->PostThreadMessage(WM_THREADMSG, 1, 0);
			break;
		}

		// restart transfer
		case TOK_REST:
		{
			if (!IsNumeric(strArguments.GetBuffer(strArguments.GetLength())))
			{
				strArguments.ReleaseBuffer();
				SendResponse("501 Invalid parameter.");
			}
			else
			{
				strArguments.ReleaseBuffer();
				m_dwRestartOffset = atol(strArguments);
				SendResponse("350 Restarting at %d.", m_dwRestartOffset);
			}
		}
		break;

		// display help
		case TOK_HELP:
			// if client did not specify a command, display all available commands
			if (strArguments == "")
			{
				CString strResponse = "214-The following commands are recognized:\r\n";
				// find command in command list
				for (int i = TOK_ABOR; i < TOK_ERROR; i++)
				{
					strResponse += commandList[i].m_pszName;
					strResponse += "\r\n";
				}
				strResponse += "214 HELP command successful.";
				SendResponse(strResponse);
			}
			else
			{
				int nHelpCmd;
				// find command in command list
				for (nHelpCmd = TOK_ABOR; nHelpCmd < TOK_ERROR; nHelpCmd++)
				{
					// found command ?
					if (strArguments.CompareNoCase(commandList[nHelpCmd].m_pszName) == 0)
					{
						break;			
					}
				}
				if (nHelpCmd != TOK_ERROR)
				{
					// show help about command
					SendResponse("214 %s", commandList[nHelpCmd].m_pszDescription);
				}
				else
				{
					SendResponse("501 Unknown command %s", strArguments);
				}
			}
			break;

		// dummy instruction
		case TOK_NOOP:
			SendResponse("200 OK");
			break;

		default:
			SendResponse("502 Command not implemented - Try HELP.");
			break;
	}
}


void ConnectSocket::FireStatusMessage(LPCTSTR lpszStatus, int nType)
{
	ConnectThread *pThread = (ConnectThread *)a_Thread;
	((ftpserver *)pThread->m_pWndServer)->AddTraceLine(nType, "[%d] %s", a_Thread->m_nThreadID, lpszStatus);
}


BOOL ConnectSocket::CreateDataConnection(int nTransferType, LPCTSTR lpszData)
{
	if (!m_bPassiveMode)
	{
		a_DataSocket = new DataSocket(this, nTransferType);
		if (a_DataSocket->Create())
		{
			a_DataSocket->AsyncSelect();
			a_DataSocket->SetRestartOffset(m_dwRestartOffset);
			a_DataSocket->SetData(lpszData);

			// connect to remote site
			if (a_DataSocket->Connect(m_strRemoteHost, m_nRemotePort) == 0)
			{
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					SendResponse("425 Can't open data connection.");
					return FALSE;
				}
			}
			
			switch(nTransferType)
			{
				case 0:
					SendResponse("150 Opening ASCII mode data connection for directory list."); 
					break;
				case 1:
				case 2:
					SendResponse("150 Opening BINARY mode data connection for file transfer.");
					break;
			}
		}
		else
		{
			SendResponse("421 Failed to create data connection socket.");
			return FALSE;
		}
	}
	else
	{
		a_DataSocket->SetRestartOffset(m_dwRestartOffset);
		a_DataSocket->SetData(lpszData);
		a_DataSocket->SetTransferType(nTransferType, TRUE);
	}
	return TRUE;
}


void ConnectSocket::DestroyDataConnection()
{
	if (!a_DataSocket)
		return;

	delete a_DataSocket;

	// reset transfer status
	a_DataSocket = NULL;
	m_strRemoteHost = "";
	m_nRemotePort = -1;
	m_dwRestartOffset = 0;
	m_bPassiveMode = FALSE;
}
