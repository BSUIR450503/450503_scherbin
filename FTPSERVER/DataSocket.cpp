#include "library.h"
#include "DataSocket.h"
#include "ConnectSocket.h"
#include "ConnectThread.h"


DataSocket::DataSocket(ConnectSocket *pSocket, int nTransferType)
{
	m_nTransferType = nTransferType;
	m_pConnectSocket = pSocket;
	m_nStatus = XFERMODE_IDLE;
	m_strData = "";
	m_File.m_hFile = NULL;
	m_bConnected = FALSE;
	m_dwRestartOffset = 0;
	m_bInitialized = FALSE;
}


DataSocket::~DataSocket()
{
	m_bConnected = FALSE;
	TRACE0("CDataSocket destroyed.\n");
}


void DataSocket::OnSend(int nErrorCode) 
{
	CAsyncSocket::OnSend(nErrorCode);
	switch(m_nStatus)
	{
		case XFERMODE_LIST:
		{
			while (m_nTotalBytesTransfered < m_nTotalBytesSend)
			{
				DWORD dwRead;
				int dwBytes;

				CString strData;
				
				dwRead = m_strData.GetLength();
				
				if (dwRead <= PACKET_SIZE)
				{
					strData = m_strData;
				}
				else
				{
					strData = m_strData.Left(PACKET_SIZE);
					dwRead = strData.GetLength();
				}
				
				if ((dwBytes = Send(strData, dwRead)) == SOCKET_ERROR)
				{
					if (GetLastError() == WSAEWOULDBLOCK) 
					{
						Sleep(0);
						break;
					}
					else
					{
						TCHAR szError[256];
						wsprintf(szError, "Server Socket failed to send: %d", GetLastError());

						// close the data connection.
						Close();

						m_nTotalBytesSend = 0;
						m_nTotalBytesTransfered = 0;

						// change status
						m_nStatus = XFERMODE_IDLE;

						m_pConnectSocket->SendResponse("426 Connection closed; transfer aborted.");

						// destroy this socket
						AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
					}
				}
				else
				{
					m_nTotalBytesTransfered += dwBytes;

					m_strData = m_strData.Mid(dwBytes);
					
					//((ConnectThread *)AfxGetThread())->IncSentBytes(dwBytes);
				}
			}
			if (m_nTotalBytesTransfered == m_nTotalBytesSend)
			{
                // close the data connection.
                Close();

                m_nTotalBytesSend = 0;
                m_nTotalBytesTransfered = 0;

                // change status
				m_nStatus = XFERMODE_IDLE;

				// tell the client the transfer is complete.
                m_pConnectSocket->SendResponse("226 Transfer complete",5);
				// destroy this socket
				//AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
			}
			break;
		}
		case XFERMODE_SEND:
		{
			while (m_nTotalBytesTransfered < m_nTotalBytesSend)
			{
				// allocate space to store data
				byte data[PACKET_SIZE];
				
				m_File.Seek(m_nTotalBytesTransfered, CFile::begin);

				DWORD dwRead = m_File.Read(data, PACKET_SIZE);
    
				int dwBytes;

				if ((dwBytes = Send(data, dwRead)) == SOCKET_ERROR)
				{
					if (GetLastError() == WSAEWOULDBLOCK) 
					{
						Sleep(0);
						break;
					}
					else
					{
						TCHAR szError[256];
						wsprintf(szError, "Server Socket failed to send: %d", GetLastError());

						// close file.
						m_File.Close();
						m_File.m_hFile = NULL;

						// close the data connection.
						Close();

						m_nTotalBytesSend = 0;
						m_nTotalBytesTransfered = 0;

						// change status
						m_nStatus = XFERMODE_IDLE;

						m_pConnectSocket->SendResponse("426 Connection closed; transfer aborted.");

						// destroy this socket
						AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);

						// download failed
						((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_DOWNLOADFAILED);
					}
				}
				else
				{
					m_nTotalBytesTransfered += dwBytes;
					
					//((ConnectThread *)AfxGetThread())->IncSentBytes(dwBytes);
				}
			}
			if (m_nTotalBytesTransfered == m_nTotalBytesSend)
			{
				// close file.
                m_File.Close();
				m_File.m_hFile = NULL;

                // close the data connection.
                Close();

                m_nTotalBytesSend = 0;
                m_nTotalBytesTransfered = 0;

                // change status
				m_nStatus = XFERMODE_IDLE;

				// tell the client the transfer is complete.
				m_pConnectSocket->SendResponse("226 Transfer complete");
				// destroy this socket
				//AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
				// download successfull
				//((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_DOWNLOADSUCCEEDED);
			}
			break;
		}
//		default:
//			break;
	}
}


void DataSocket::OnConnect(int nErrorCode) 
{
	if (nErrorCode)
	{
		m_nStatus = XFERMODE_ERROR;
		m_pConnectSocket->SendResponse("425 Can't open data connection.");
		// destroy this socket
		AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
	}
	else
	{
		switch (m_nTransferType)
		{
			case 0:	// List Directory
				m_nStatus = XFERMODE_LIST;
				m_bConnected = TRUE;
				OnSend(0);
				break;
			case 1:	// Send File
				if (PrepareSendFile(m_strData))
				{
					m_nStatus = XFERMODE_SEND;
					m_bConnected = TRUE;
				}
				else
				{
					Close();
				}
				break;
			case 2:	// Receive File
				if (PrepareReceiveFile(m_strData))
				{
					m_nStatus = XFERMODE_RECEIVE;
					m_bConnected = TRUE;
				}
				else
				{
					Close();
					m_pConnectSocket->SendResponse("450 can't access file.");
					// destroy this socket
					AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
					// upload failed
					((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
				}
				break;
		}
	}
	CAsyncSocket::OnConnect(nErrorCode);
}



void DataSocket::OnClose(int nErrorCode) 
{
	TRACE0("CDataSocket() OnClose()\n");
	if (m_pConnectSocket)
	{
		// shutdown sends
		ShutDown(1);

		if (m_nStatus == XFERMODE_RECEIVE)
		{
			while(Receive() != 0)
			{
				// receive remaining data				
			}
		}
		else
		{
			/*m_pConnectSocket->SendResponse("426 Connection closed; transfer aborted.");
			// destroy this socket
			AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
			// upload failed
			((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);*/
		}
	}
	m_nStatus = XFERMODE_IDLE;	
	m_bConnected = FALSE;
	CAsyncSocket::OnClose(nErrorCode);
}


void DataSocket::OnAccept(int nErrorCode) 
{
	// Accept the connection using a temp CSocket object.
	CAsyncSocket tmpSocket;
	Accept(tmpSocket);
	
	SOCKET socket = tmpSocket.Detach();
	Close();

	Attach(socket);

	m_bConnected = TRUE;

	if (!m_bInitialized)
		SetTransferType(m_nTransferType);
	
	CAsyncSocket::OnAccept(nErrorCode);
}


int DataSocket::GetStatus()
{
	return m_nStatus;
}


BOOL DataSocket::PrepareSendFile(LPCTSTR lpszFilename)
{
	// close file if it's already open
	if (m_File.m_hFile != NULL)
	{
		m_File.Close();
	}

	// open source file
	if (!m_File.Open(lpszFilename, CFile::modeRead | CFile::typeBinary))
	{
		return FALSE;
	}
	m_nTotalBytesSend = m_File.GetLength();
	if (m_dwRestartOffset < m_nTotalBytesSend)
		m_nTotalBytesTransfered = m_dwRestartOffset;
	else
		m_nTotalBytesTransfered = 0;

	return TRUE;
}


BOOL DataSocket::PrepareReceiveFile(LPCTSTR lpszFilename)
{
	// close file if it's already open
	if (m_File.m_hFile != NULL)
	{
		m_File.Close();
	}

	// open destination file
	if (!m_File.Open(lpszFilename, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite))
	{
		return FALSE;
	}
	m_nTotalBytesReceive = 0;
	m_nTotalBytesTransfered = 0;

	if (m_dwRestartOffset)
	{
		m_File.SetLength(m_dwRestartOffset);
		m_File.SeekToEnd();
	}
	return TRUE;
}


void DataSocket::OnReceive(int nErrorCode) 
{
	CAsyncSocket::OnReceive(nErrorCode);
	
	Receive();
}


int DataSocket::Receive()
{
	int nRead = 0;
	
	if (m_nStatus == XFERMODE_RECEIVE)
	{
		if (m_File.m_hFile == NULL)
			return 0;

		byte data[PACKET_SIZE];
		nRead = CAsyncSocket::Receive(data, PACKET_SIZE);

		switch(nRead)
		{
			case 0:
			{
				m_File.Close();
				m_File.m_hFile = NULL;
				Close();
				// tell the client the transfer is complete.
				m_pConnectSocket->SendResponse("226 Transfer complete");
				// destroy this socket
				AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
				// upload succesfull
				((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADSUCCEEDED);
				break;
			}
			case SOCKET_ERROR:
			{
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					m_File.Close();
					m_File.m_hFile = NULL;
					Close();
					m_pConnectSocket->SendResponse("426 Connection closed; transfer aborted.");
					// destroy this socket
					AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
					// upload failed
					((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);

				}
				break;
			}
			default:
			{
				//((ConnectThread *)AfxGetThread())->IncReceivedBytes(nRead);

				TRY
				{
					m_File.Write(data, nRead);
				}
				CATCH_ALL(e)
				{
					m_File.Close();
					m_File.m_hFile = NULL;
					Close();
					m_pConnectSocket->SendResponse("450 Can't access file.");
					// destroy this socket
					AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
					// upload failed
					((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
					return 0;
				}
				END_CATCH_ALL;
				break;
			}
		}
	}
	return nRead;
}

void DataSocket::SetData(LPCTSTR lpszData)
{
	m_strData = lpszData;
	m_nTotalBytesSend = m_strData.GetLength();
	m_nTotalBytesTransfered = 0;
}


void DataSocket::SetTransferType(int nType, BOOL bWaitForAccept)
{
	m_nTransferType = nType; 

	if (bWaitForAccept && !m_bConnected)
	{
		m_bInitialized = FALSE;
		return;
	}

	if (m_bConnected && m_nTransferType != -1)
		m_pConnectSocket->SendResponse("150 Connection accepted");

	m_bInitialized = TRUE;

	switch(m_nTransferType)
	{
		case 0:	// List Directory
			m_nStatus = XFERMODE_LIST;
			OnSend(0);
			break;
		case 1:	// Send File
			if (PrepareSendFile(m_strData))
			{
				m_nStatus = XFERMODE_SEND;
				m_bConnected = TRUE;
				OnSend(0);
			}
			else
			{
				Close();
			}
			break;
		case 2:	// Receive File
			if (PrepareReceiveFile(m_strData))
			{
				m_nStatus = XFERMODE_RECEIVE;
				m_bConnected = TRUE;
				OnSend(0);
			}
			else
			{
				Close();
				m_pConnectSocket->SendResponse("450 Can't access file.");
				// destroy this socket
				AfxGetThread()->PostThreadMessage(WM_THREADMSG, 0, 0);
				// upload failed
				((ConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
			}
			break;
		default:
			m_bInitialized = FALSE;
			break;
	}
}

void DataSocket::SetRestartOffset(DWORD dwOffset)
{
	m_dwRestartOffset = dwOffset;
}
