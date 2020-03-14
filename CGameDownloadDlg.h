#pragma once


// CGameDownloadDlg 对话框

class CGameDownloadDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGameDownloadDlg)

public:
	CGameDownloadDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CGameDownloadDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GAMEDOWNLOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
