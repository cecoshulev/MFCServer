#include "pch.h"
#include "CMyClientSocket.h"
#include "MFCServerDlg.h"
#include "CMyServerSocket.h"
#include <ws2tcpip.h>
#include <string.h>

IMPLEMENT_DYNAMIC(CMyClientSocket, CAsyncSocket)

CMyClientSocket::CMyClientSocket()
{
    m_pDlg = nullptr;
    m_pServer = nullptr;
    m_strClientInfo = _T("Unknown:0");
}

CMyClientSocket::~CMyClientSocket()
{
    if (m_pServer)
    {
        m_pServer->RemoveClient(this); // Notify server to remove this client
    }
    Close();
}

void CMyClientSocket::OnReceive(int nErrorCode)
{
    TCHAR szBuffer[1024] = { 0 };
    int nBytesReceived = Receive(szBuffer, sizeof(szBuffer) - 1);

    if (nBytesReceived > 0)
    {
        szBuffer[nBytesReceived] = _T('\0');
        CString strReceived = szBuffer;

        GetLocalTime(&m_stLastActiveTime);

        // If receive USERNAME:
        if ((strReceived.Find(_T("USERNAME:"), 0) == 0) &&
            (m_strUsername.IsEmpty()))   // If once user name is set it can not be changed
        {
            m_strUsername = strReceived.Mid(9);
            if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
                m_pDlg->m_ListClients.SetItemText(0, 0, m_strUsername);
        }
        else // Receive normal message
        {
            if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
                m_pDlg->AddLogMessage(m_strClientInfo + _T(" ") + strReceived);

            if (m_pServer)
            {
                m_pServer->Broadcast(m_strClientInfo + _T(" ") + strReceived);
            }
        }
    }
    CAsyncSocket::OnReceive(nErrorCode);
}

void CMyClientSocket::OnConnect(int nErrorCode)
{
    if (nErrorCode == 0)
    {
        if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
            m_pDlg->AddLogMessage(m_strClientInfo + _T(" connected!"));
    }
    else
    {
        if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
            m_pDlg->AddLogMessage(_T("Connection failed!"));
    }
    CAsyncSocket::OnConnect(nErrorCode);
}

void CMyClientSocket::OnClose(int nErrorCode)
{
    if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
        m_pDlg->AddLogMessage(m_strClientInfo + _T(" ") + m_strUsername + _T(" is disconnected!"));
    CAsyncSocket::OnClose(nErrorCode);
    // 
    int count = m_pDlg->m_ListClients.GetItemCount();

    // Remove from m_ListClients
    for (int i = 0; i < count; i++)
    {
        if (m_strClientInfo == m_pDlg->m_ListClients.GetItemText(i, 1))
            m_pDlg->m_ListClients.DeleteItem(i);
    }

    // Remove client from CList
    m_pServer->RemoveClient(this);
}

void CMyClientSocket::SendMessage(const CString& strMessage)
{
    // Convert CString (Unicode) to UTF-8 for sending
    CStringA strAnsiMessage(CT2A(strMessage, CP_UTF8));
    if (Send(strAnsiMessage, strAnsiMessage.GetLength()) == SOCKET_ERROR)
    {
        int nError = GetLastError();
        if (nError != WSAEWOULDBLOCK)
        {
            if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
                m_pDlg->AddLogMessage(_T("Failed to send message to ") + m_strClientInfo);
        }
    }
}



