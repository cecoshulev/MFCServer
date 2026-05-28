#include "pch.h"
#include "CMyServerSocket.h"
#include "CMyClientSocket.h"
#include "MFCServerDlg.h"
//#include <string>
#include <atlstr.h>
#include <windows.h>

IMPLEMENT_DYNAMIC(CMyServerSocket, CAsyncSocket)

CMyServerSocket::CMyServerSocket()
{
    m_pDlg = nullptr;
}

CMyServerSocket::~CMyServerSocket()
{
    // Clean up all clients
    POSITION pos = m_clientList.GetHeadPosition();
    while (pos != nullptr)
    {
        CMyClientSocket* pClient = m_clientList.GetNext(pos);
        delete pClient;
    }
    m_clientList.RemoveAll();
    Close();
}

BOOL CMyServerSocket::StartListening(UINT nPort)
{
    if (!Socket())
    {
        if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
            m_pDlg->AddLogMessage(_T("Failed to create socket!"));
        return FALSE;
    }

    if (!Bind(nPort))
    {
        if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
            m_pDlg->AddLogMessage(_T("Failed to bind socket!"));
        return FALSE;
    }

    if (!Listen())
    {
        if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
            m_pDlg->AddLogMessage(_T("Failed to listen on socket!"));
        return FALSE;
    }

    if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
        m_pDlg->AddLogMessage(_T("Server started. Waiting for connections..."));

    return TRUE;
}

void CMyServerSocket::OnAccept(int nErrorCode)
{
    if (nErrorCode == 0)
    {
        // Create a new client socket
        CMyClientSocket* pClientSocket = new CMyClientSocket();

        // Accept the connection and get the client's address
        SOCKADDR_IN clientAddr;
        int nAddrLen = sizeof(clientAddr);
        if (Accept(*pClientSocket, (SOCKADDR*)&clientAddr, &nAddrLen))
        {
            // Set the client's address immediately
            char szIP[INET_ADDRSTRLEN] = { 0 };
            if (inet_ntop(AF_INET, &(clientAddr.sin_addr), szIP, INET_ADDRSTRLEN) != nullptr)
            {
                // Force 127.0.0.1 for loopback
                if (strcmp(szIP, "0.0.0.0") == 0 || strcmp(szIP, "127.0.0.1") == 0)
                {
                    pClientSocket->m_strClientInfo.Format(_T("127.0.0.1:%d"), ntohs(clientAddr.sin_port));
                }
                else
                {
                    pClientSocket->m_strClientInfo.Format(_T("%S:%d"), szIP, ntohs(clientAddr.sin_port));
                }
            }
            else
            {
                pClientSocket->m_strClientInfo = _T("127.0.0.1:0");
            }

            // Set the dialog and server pointers
            pClientSocket->SetDialogPointer(m_pDlg);
            pClientSocket->SetServerSocket(this);
            m_clientList.AddTail(pClientSocket);

            if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
                m_pDlg->AddLogMessage(pClientSocket->m_strClientInfo + _T(" connected!"));
            if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
                m_pDlg->AddListControlEntry(pClientSocket->m_strClientInfo);

            // Add onConnect tile
            GetLocalTime(&pClientSocket->m_stLoginTime);
            CString strTmp;

            GetLocalTime(&pClientSocket->m_stLoginTime);
            strTmp.Format(_T(" %02d:%02d:%02d"), pClientSocket->m_stLoginTime.wHour, pClientSocket->m_stLoginTime.wMinute, pClientSocket->m_stLoginTime.wSecond);
            if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
                m_pDlg->m_ListClients.SetItemText(0, 2, strTmp);
        }
        else
        {
            delete pClientSocket;
            if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
                m_pDlg->AddLogMessage(_T("Failed to accept connection!"));
        }
    }

/*
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    CString strTmp;

    
    strTmp.Format(_T("Current time %02d:%02d"), lt.wHour, lt.wMinute);
    if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
        m_pDlg->AddLogMessage(strTmp);
*/
    CAsyncSocket::OnAccept(nErrorCode);
}

void CMyServerSocket::Broadcast(const CString& strMessage)
{
    POSITION pos = m_clientList.GetHeadPosition();
    while (pos != nullptr)
    {
        CMyClientSocket* pClient = m_clientList.GetNext(pos);
        if (pClient && ::IsWindow(pClient->m_pDlg->m_hWnd)) // Check if client is valid
        {
            pClient->SendMessage(strMessage); // Send the message to the client
        }
    }
}

void CMyServerSocket::RemoveClient(CMyClientSocket* pClient)
{
    POSITION pos = m_clientList.Find(pClient);
    if (pos != nullptr)
    {
        m_clientList.RemoveAt(pos);
    }
}

void CMyServerSocket::OnClose(int nErrorCode)
{
    if (m_pDlg && ::IsWindow(m_pDlg->m_hWnd))
        m_pDlg->AddLogMessage(_T("Server socket closed!"));
    CAsyncSocket::OnClose(nErrorCode);
}

CMyClientSocket* CMyServerSocket::GetClientByIP(const CString& strIP)
{
    POSITION pos = m_clientList.GetHeadPosition();
    while (pos != nullptr)
    {
        CMyClientSocket* pClient = m_clientList.GetNext(pos);
        if (pClient && pClient->m_strClientInfo.Find(strIP) == 0) // Check if the client's info starts with the given IP
        {
            return pClient;
        }
    }
    return nullptr; // Client not found
}
