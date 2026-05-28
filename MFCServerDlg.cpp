
// MFCServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFCServer.h"
#include "MFCServerDlg.h"
#include "afxdialogex.h"
#include <atlstr.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <fstream>
#include <afx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Copy from stackoverflow.com
SYSTEMTIME operator-(const SYSTEMTIME& pSr, const SYSTEMTIME& pSl)
{
	SYSTEMTIME t_res;
	FILETIME v_ftime;
	ULARGE_INTEGER v_ui;
	__int64 v_right, v_left, v_res;
	SystemTimeToFileTime(&pSr, &v_ftime);
	v_ui.LowPart = v_ftime.dwLowDateTime;
	v_ui.HighPart = v_ftime.dwHighDateTime;
	v_right = v_ui.QuadPart;

	SystemTimeToFileTime(&pSl, &v_ftime);
	v_ui.LowPart = v_ftime.dwLowDateTime;
	v_ui.HighPart = v_ftime.dwHighDateTime;
	v_left = v_ui.QuadPart;

	v_res = v_right - v_left;

	v_ui.QuadPart = v_res;
	v_ftime.dwLowDateTime = v_ui.LowPart;
	v_ftime.dwHighDateTime = v_ui.HighPart;
	FileTimeToSystemTime(&v_ftime, &t_res);
	return t_res;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()



// CMFCServerDlg dialog

CMFCServerDlg::CMFCServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CMFCServerDlg::~CMFCServerDlg()
{
	m_serverSocket.Close();
	WSACleanup();
}

void CMFCServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CLIENTS, m_ListClients);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
}

BEGIN_MESSAGE_MAP(CMFCServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_KICK, &CMFCServerDlg::OnBnClickedButtonKick)
	ON_BN_CLICKED(IDC_BUTTON_START, &CMFCServerDlg::OnClickedButtonStart)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMFCServerDlg message handlers

BOOL CMFCServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// Initialize Winsock
	if (AfxSocketInit() == FALSE)
	{
		AfxMessageBox(_T("Winsock initialization failed!"));
		return FALSE;
	}

	// Set default port
	m_editPort.SetWindowText(_T("9005"));

	// Set the dialog pointer in the server socket
	m_serverSocket.SetDialogPointer(this);

	//m_ListClients.SetExtendedStyle(m_ListClients.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ListClients.SetExtendedStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	m_ListClients.InsertColumn(0, _T("Client name"), LVCFMT_LEFT, 120);
	m_ListClients.InsertColumn(1, _T("IP"), LVCFMT_LEFT, 120);
	m_ListClients.InsertColumn(2, _T("Connected at"), LVCFMT_LEFT, 100);
	m_ListClients.InsertColumn(3, _T("Time online"), LVCFMT_LEFT, 100);

/*
	int index = m_ListClients.InsertItem(0, _T("Client 1"));
	m_ListClients.SetItemText(index, 1, _T("local host"));
	m_ListClients.SetItemText(index, 2, _T("Y"));
	m_ListClients.SetItemText(index, 3, _T("0"));

	index = m_ListClients.InsertItem(1, _T("Client 2"));
	m_ListClients.SetItemText(index, 1, _T("local host"));
	m_ListClients.SetItemText(index, 2, _T("Y"));
	m_ListClients.SetItemText(index, 3, _T("0"));
*/	
	UpdateData(false);

	SetTimer(ID_TIMER_CLIENTS, 1000, NULL);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCServerDlg::OnBnClickedButtonKick()
{
	// TODO: Add your control notification handler code here
	int selectedRow = ((CListCtrl*)GetDlgItem(IDC_LIST_CLIENTS))->GetSelectionMark();

	CString strClientInfo = m_ListClients.GetItemText(selectedRow, 1);

	CMyClientSocket* pClient = m_serverSocket.GetClientByIP(strClientInfo);


	if (pClient)
	{
		// Remove from m_ListClients
		m_ListClients.DeleteItem(selectedRow);

		pClient->SendMessage(_T("I'll kick you!!!"));
		pClient->Close();
		//AfxMessageBox(_T("Success !!!"));

		AddLogMessage(pClient->m_strClientInfo + _T(" ") + pClient->m_strUsername + _T(" is kicked"));

		POSITION pos = m_serverSocket.m_clientList.Find(pClient);
		if (pos != nullptr)
		{
			//AfxMessageBox(_T("Success! I'll remove it"));
			m_serverSocket.RemoveClient(pClient);
		}
		else
		{
			AfxMessageBox(_T("Client not found in the list!"));
		}
	}
	else {
		AfxMessageBox(_T("FAIL !!!"));
	}

	UpdateData(false);
}

void CMFCServerDlg::AddLogMessage(const CString& strMessage)
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	CString strTimeAndMessage;
	
	// Add Time stamp to log message
	strTimeAndMessage.Format(_T("%02d:%02d:%02d %s"), lt.wHour, lt.wMinute, lt.wSecond, (LPCTSTR)strMessage);

	int nIndex = m_listLog.AddString(strTimeAndMessage);
	if (nIndex != LB_ERR)
	{
		m_listLog.SetCurSel(nIndex);
		WriteToFile(strTimeAndMessage);
	}
}

void CMFCServerDlg::AddListControlEntry(const CString& strMessage)
{
	int nIndex = m_ListClients.InsertItem(0, _T(""));
	m_ListClients.SetItemText(nIndex, 1, strMessage);
}

// Create file or append to file a string
void CMFCServerDlg::WriteToFile(const CString& strMessage)
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	CString strFileName;
	CFile file;

	// File name format yyyymmdd_hh00.log
	strFileName.Format(_T("%04d%02d%02d_%02d00.log"), lt.wYear ,lt.wMonth, lt.wDay , lt.wHour);

	// Add Time stamp to log message
	//strWrite.Format(_T("%02d:%02d:%02d %s"), lt.wHour, lt.wMinute, lt.wSecond, (LPCTSTR)strMessage);

	if (file.Open(strFileName, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate))
	{
		file.SeekToEnd();
		CStringA strAnsi(strMessage); // Convert to ANSI (if needed)
		file.Write(strAnsi, strAnsi.GetLength());
		file.Write("\r\n", 2);
		file.Close();
	}
	else
	{
		AfxMessageBox(_T("Failed to open file!"));
	}
}

void CMFCServerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnClose();
}

void CMFCServerDlg::OnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	CString strPort;
	m_editPort.GetWindowText(strPort);
	UINT nPort = _ttoi(strPort);

	if (nPort == 0)
	{
		AfxMessageBox(_T("Invalid port number!"));
		return;
	}

	if (!m_serverSocket.StartListening(nPort))
	{
		AfxMessageBox(_T("Failed to start server!"));
		return;
	}

	// Server is started. Disable button Start
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_START);
	if (pButton != NULL)
	{
		pButton->EnableWindow(FALSE);
	}
}

void CMFCServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CString strTmp;
	CMyClientSocket* pClient;
	int i;
	int count = m_ListClients.GetItemCount();
	SYSTEMTIME curTime, diffTime;

	GetLocalTime(&curTime);

	for (i = 0; i < count; i++)
	{
		pClient = m_serverSocket.GetClientByIP(m_ListClients.GetItemText(i, 1));
		if (pClient)
		{
			diffTime = curTime - pClient->m_stLoginTime;
			strTmp.Format(_T("%02d:%02d"), diffTime.wMinute, diffTime.wSecond);
			m_ListClients.SetItemText(i, 3, strTmp);
			if (diffTime.wMinute >= 10) // Kick client after 10 minutes
			{
				m_ListClients.DeleteItem(i);
				pClient->Close();
				AddLogMessage(pClient->m_strClientInfo + _T(" ") + pClient->m_strUsername + _T(" is timed out"));
				
				POSITION pos = m_serverSocket.m_clientList.Find(pClient);
				if (pos != nullptr)
				{
					m_serverSocket.RemoveClient(pClient);
				}
				else
				{
					AfxMessageBox(_T("Can not remove client from list!"));
				}
			}
		}
	}


	UpdateData(false);

	CDialogEx::OnTimer(nIDEvent);
}
