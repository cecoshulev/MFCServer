#pragma once
#include <afxsock.h>
#include <afxtempl.h>
#include "CMyClientSocket.h"

class CMFCServerDlg;
class CMyClientSocket;

class CMyServerSocket : public CAsyncSocket
{
public:
    CMyServerSocket();
    virtual ~CMyServerSocket();

    virtual void OnAccept(int nErrorCode);
    virtual void OnClose(int nErrorCode);

    BOOL StartListening(UINT nPort);
    void SetDialogPointer(CMFCServerDlg* pDlg) { m_pDlg = pDlg; }
    void Broadcast(const CString& strMessage);
    void RemoveClient(CMyClientSocket* pClient);

    CMFCServerDlg* m_pDlg;
    CList<CMyClientSocket*, CMyClientSocket*> m_clientList;

    CMyClientSocket* GetClientByIP(const CString& strIP);

    DECLARE_DYNAMIC(CMyServerSocket)
};
