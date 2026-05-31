#pragma once
#include <afxsock.h>
#include <windows.h>

class CMFCServerDlg;
class CMyServerSocket;

class CMyClientSocket : public CAsyncSocket
{
public:
    CMyClientSocket();
    virtual ~CMyClientSocket();

    virtual void OnConnect(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
    virtual void OnClose(int nErrorCode);

    void SetDialogPointer(CMFCServerDlg* pDlg) { m_pDlg = pDlg; }
    void SetServerSocket(CMyServerSocket* pServer) { m_pServer = pServer; }
    CString GetClientInfo() const { return m_strClientInfo; }
    void SendMessage(const CString& strMessage);

    CMFCServerDlg* m_pDlg;
    CMyServerSocket* m_pServer;
    CString m_strClientInfo; // AA.BB.CC.DD:port
    CString m_strUsername = _T("");

    SYSTEMTIME m_stLoginTime;
    SYSTEMTIME m_stLastActiveTime; // Time at last activity

    DECLARE_DYNAMIC(CMyClientSocket)
};

