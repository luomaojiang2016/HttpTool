
// HttpToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HttpTool.h"
#include "HttpToolDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <thread>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

 const std::wstring HttpMethod[] = {
	L"Get",
	L"Put",
	L"Post",
	L"Delete"
};
 const std::wstring RequestMode[] = {
	 L"循环请求",
	 L"定时请求"
 };


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


// CHttpToolDlg dialog



CHttpToolDlg::CHttpToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_HTTPTOOL_DIALOG, pParent)
	, m_edit_url_value(_T("http://127.0.0.1:7001/v1/api/user"))
	, str_Edit_ReqBody(_T("{}"))
	, str_Edit_RepBody(_T(""))
	, m_edit_interval(0)
	, m_Edit_thread(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHttpToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_MEthod, m_combo_method);
	DDX_Text(pDX, IDC_EDIT_Url, m_edit_url_value);
	DDX_Text(pDX, IDC_EDIT_ReqBody, str_Edit_ReqBody);
	DDX_Text(pDX, IDC_EDIT_RepBody, str_Edit_RepBody);
	DDX_Text(pDX, IDC_EDIT_Interval, m_edit_interval);
	DDX_Text(pDX, IDC_EDIT_Thread, m_Edit_thread);
}

BEGIN_MESSAGE_MAP(CHttpToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CHttpToolDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_Start, &CHttpToolDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_Stop, &CHttpToolDlg::OnBnClickedButtonStop)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CHttpToolDlg message handlers

BOOL CHttpToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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
	for (auto method:HttpMethod) {
		m_combo_method.AddString(method.c_str());
	}
	m_combo_method.SetCurSel(1);
	m_Edit_thread=std::thread::hardware_concurrency()*2;

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHttpToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CHttpToolDlg::OnPaint()
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
HCURSOR CHttpToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
#include "./http/textconv_helper.hpp"
#include "./http/restclient.hpp"

void response_log(RestClient::Response& response)
{
	//response utf8
	std::wstring body = textconv_helper::A2W_(response.body.c_str(), CP_UTF8);
	wprintf(L"body = %s \n", body.c_str());
	printf("code = %d \n", response.code);
	printf("cookie = %s \n", response.cookies.c_str());
	printf("%s %s \n", response.Cookie["BAIDUID"].c_str(), response.get_cookie("BAIDUID").c_str());
	printf("%s %s \n", response.headers["Location"].c_str(), response.get_header("Location").c_str());
	printf("%s %s \n", response.headers["Content-Type"].c_str(), response.get_header("Content-Type").c_str());
	printf("%s %s \n", response.headers["Content-Length"].c_str(), response.get_header("Content-Length").c_str());
}


void one_test()
{
	RestClient::Response r;
	r = RestClient::get("http://www.baidu.com/");
	response_log(r);
	r = RestClient::post("http://www.baidu.com/post", "text/json", "{\"foo\": \"bla\"}");
	response_log(r);
	r = RestClient::put("http://www.baidu.com/put", "text/json", "{\"foo\": \"bla\"}");
	response_log(r);
	r = RestClient::del("http://www.baidu.com/delete");
	response_log(r);
	r = RestClient::head("http://www.baidu.com/");
	response_log(r);
}

void get_test()
{
	RestClient::Request request;

	request.timeout = 3000;
	request.followRedirects = false;
	request.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36";
	request.headers["Cookie"] = "name=value;";

	RestClient::Response response = RestClient::get("http://www.baidu.com", &request);

	response_log(response);
}

void post_test()
{
	RestClient::Request request;
	request.timeout = 3000;
	request.followRedirects = false;
	request.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36";
	request.headers["Cookie"] = "name=value;";

	RestClient::Response response = RestClient::post("http://www.baidu.com/post.php?a=123456", "text/json", "{\"foo\": \"bla\"}", &request);

	response_log(response);
}

void proxy_test()
{
	std::string strProxy = "http=115.29.2.139:80";

	RestClient::Request request;
	request.proxy.proxy = strProxy;
	request.proxy.username = "";
	request.proxy.password = "";

	request.timeout = 3000;
	request.followRedirects = false;
	request.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36";
	request.headers["Cookie"] = "name=value;";

	RestClient::Response response = RestClient::get("http://ipip.yy.com/get_ip_info.php", &request);

	response_log(response);
}
void CHttpToolDlg::OnBnClickedButton1()
{
	UpdateData();
	std::string strProxy = "http=115.29.2.139:80";

	RestClient::Request request;
	request.proxy.proxy = "";
	request.proxy.username = "";
	request.proxy.password = "";

	request.timeout = 3000;
	request.followRedirects = false;
	request.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36";
	request.headers["Cookie"] = "name=value;";
	RestClient::Response response;
	setlocale(LC_ALL, "CHS");
	std::string _Url = CT2A(m_edit_url_value, CP_ACP);
	std::string _Body = CT2A(str_Edit_ReqBody, CP_ACP);
	CString _Method;
	m_combo_method.GetWindowText(_Method);
	if (_Method == L"Get") {
		response = RestClient::get(_Url, &request);
	}
	else if (_Method == L"Delete") {
		response = RestClient::del(_Url, &request);
	}
	else if (_Method == L"Head") {
		response = RestClient::head(_Url, &request);
	}
	else if (_Method == "Post") {
		response = RestClient::post(_Url, "text/json", _Body, &request);
	}
	else if (_Method == "Put") {
		response = RestClient::put(_Url, "text/json", _Body, &request);
	}
	std::string strBody = response.body;
	str_Edit_RepBody = strBody.c_str();

	((CEdit*)GetDlgItem(IDC_EDIT_RepBody))->SetWindowText(str_Edit_RepBody);
	int code = (response.code == 200) ? 200 : GetLastError();
	CString _code;
	_code.Format(_T("%d"), code);
	((CListBox*)GetDlgItem(IDC_LIST_RepHeader))->AddString(_code);
}


BOOL CHttpToolDlg::PreTranslateMessage(MSG* pMsg)

{
	if (pMsg->message == WM_KEYDOWN)  //判断是否有按键按下     
	{
		switch (pMsg->wParam)
		{
		case VK_DOWN:     //表示是方向键中的向下的键               //add handle code here           
			break;
		case VK_RETURN: //Enter按键事件
			//return true;

		default:

			break;

		}

	}
	return CDialog::PreTranslateMessage(pMsg);
}
void CHttpToolDlg::Start() {
//	UpdateData();
	std::string strProxy = "http=115.29.2.139:80";

	RestClient::Request request;
	request.proxy.proxy = "";
	request.proxy.username = "";
	request.proxy.password = "";

	request.timeout = 3000;
	request.followRedirects = false;
	request.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36";
	request.headers["Cookie"] = "name=value;";
	RestClient::Response response;
	while (!IsStop) {

		setlocale(LC_ALL, "CHS");
		std::string _Url = CT2A(m_edit_url_value, CP_ACP);
		std::string _Body = CT2A(str_Edit_ReqBody, CP_ACP);
		CString _Method;
		m_combo_method.GetWindowText(_Method);
		if (_Method == L"Get") {
			response=RestClient::get(_Url, &request);
		}
		else if (_Method == L"Delete") {
			response=RestClient::del(_Url, &request);
		}
		else if (_Method == L"Head") {
			response=RestClient::head(_Url, &request);
		}
		else if (_Method == "Post") {
			response=RestClient::post(_Url, "text/json", _Body, &request);
		}
		std::string strBody = response.body;
		str_Edit_RepBody = strBody.c_str();

		((CEdit*)GetDlgItem(IDC_EDIT_RepBody))->SetWindowText(str_Edit_RepBody);
		int code = (response.code == 200) ? 200 : GetLastError();
		CString _code;
		_code.Format(_T("%d"), code);
		((CListBox*)GetDlgItem(IDC_LIST_RepHeader))->AddString(_code);

		if (response.code == 200) {
			SuccessReqCount++;
		}
		else {
			ErrorReqCount++;
		}
	

	}
}
void CHttpToolDlg::OnBnClickedButtonStart()
{
	UpdateData(TRUE);
	for (int i = 0; i < m_Edit_thread;i++) {
		std::thread(&CHttpToolDlg::Start, this).detach();
	}
	StartTime=GetTickCount();

	SetTimer(1, 1000, NULL);

}


void CHttpToolDlg::OnBnClickedButtonStop()
{
	IsStop = true;
	KillTimer(1);

}


void CHttpToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	EndTime = GetTickCount();

	int DatTime = (EndTime - StartTime) / 1000;

	int QPS = SuccessReqCount / DatTime;

	CString result;
	result.Format(_T("正常请求数量:%d,异常请求数量：%d,QPS:%d"), SuccessReqCount, ErrorReqCount, QPS);

	((CStatic*)GetDlgItem(IDC_STATIC_Result))->SetWindowText(result);

	CDialogEx::OnTimer(nIDEvent);
}
