// CGameDownloadDlg.cpp: 实现文件
//

#include "pch.h"
#include "JMSUpdater.h"
#include "CGameDownloadDlg.h"
#include "afxdialogex.h"


// CGameDownloadDlg 对话框

IMPLEMENT_DYNAMIC(CGameDownloadDlg, CDialogEx)

CGameDownloadDlg::CGameDownloadDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GAMEDOWNLOAD, pParent)
{

}

CGameDownloadDlg::~CGameDownloadDlg()
{
}

void CGameDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGameDownloadDlg, CDialogEx)
END_MESSAGE_MAP()

