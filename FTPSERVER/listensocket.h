#ifndef LISTENSOCKET_H
#define LISTENSOCKET_H
#pragma once
#include "library.h"
class ftpserver;

class ListenSocket : public CAsyncSocket
{
public:
    ListenSocket();
	virtual ~ListenSocket();

    CWnd* m_pWndServer;

    virtual void OnAccept(int nErrorCode);
};

#endif // LISTENSOCKET_H

