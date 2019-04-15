
// HttpToolDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CHttpToolDlg dialog
class CHttpToolDlg : public CDialogEx
{
// Construction
public:
	CHttpToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HTTPTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	void Start();
	bool IsStop = false;
	DWORD StartTime, EndTime;
	int   SuccessReqCount = 0;
	int   ErrorReqCount = 0;

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
	afx_msg void OnBnClickedButton1();
	CComboBox m_combo_method;
	CString m_edit_url_value;
	afx_msg void OnBnClickedButtonStart();
	CString str_Edit_ReqBody;
	CString str_Edit_RepBody;
	int m_edit_interval;
	int m_Edit_thread;
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
