
#include "pch.h"
#include "framework.h"
#include "JMSUpdater.h"
#include "MainDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MainDlg::MainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_JMSDOWNLOADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void MainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, tab);
}

BEGIN_MESSAGE_MAP(MainDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &MainDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()

BOOL MainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	tab.InsertItem(0, L"补丁制作");
	// tab.InsertItem(1, L"tab2");

	patchtab.Create(IDD_PATCHDOWNLOAD, &tab);
	gametab.Create(IDD_GAMEDOWNLOAD, &tab);

	// 调整子对话框大小及位置
	CRect rc;
	tab.GetClientRect(&rc);
	CRect rcTabItem;
	tab.GetItemRect(0, rcTabItem);
	rc.top += rcTabItem.Height() + 4;
	rc.left += 2;
	rc.bottom -= 3;
	rc.right -= 4;
	patchtab.MoveWindow(&rc);
	gametab.MoveWindow(&rc);
	// m_Dlg3.MoveWindow(&rc);

	// 默认标签选中
	patchtab.ShowWindow(SW_SHOW);
	tab.SetCurFocus(0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void MainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR MainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void MainDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) {
	switch (tab.GetCurSel()) {
	case 0:
		patchtab.ShowWindow(SW_SHOW);
		gametab.ShowWindow(SW_HIDE);
		patchtab.SetFocus();
		break;
	case 1:
		gametab.ShowWindow(SW_SHOW);
		patchtab.ShowWindow(SW_HIDE);
		gametab.SetFocus();
		break;
	default:
		break;
	}
	*pResult = 0;
}
