
// MFCServerDlg.h : header file
//

#pragma once
#include "CMyServerSocket.h"

// CMFCServerDlg dialog
class CMFCServerDlg : public CDialogEx
{
// Construction
public:
	CMFCServerDlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~CMFCServerDlg();


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCSERVER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonKick();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnClose();
	void AddLogMessage(const CString& strMessage);
	void AddListControlEntry(const CString& strMessage);
	void WriteToFile(const CString& strMessage);
	CListCtrl m_ListClients;

private:
	CMyServerSocket m_serverSocket;
	CListBox m_listLog;
	CEdit m_editPort;
public:
	afx_msg void OnClickedButtonStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
