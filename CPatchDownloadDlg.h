#pragma once

#include <list>

#include "xl/DownWrapper.h"
// CPatchDownloadDlg 对话框

#define WM_UPDATE_PROGRESS (WM_USER + 100)
#define WM_PATCH_LOG (WM_USER + 101)

typedef struct version_option {
	int type;
	CString label;
	CString relative_path;
} version_option;

class CPatchDownloadDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPatchDownloadDlg)

public:
	CPatchDownloadDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPatchDownloadDlg();
	BOOL OnInitDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PATCHDOWNLOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
  CEdit input_from;
  CEdit input_to;
	CComboBox type_select;
  CButton button;
  CProgressCtrl progress;
  CEdit log;
	CString log_value;
	afx_msg void OnBnClickedStartButton();
	afx_msg LRESULT OnProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLog(WPARAM wParam, LPARAM lParam);
	CString input_from_value;
	CString input_to_value;

	std::list<version_option> version_options_list;

	BOOL is_downloading = false;

	DownEngine::CDownWrapper dl;
	HANDLE taskHandle;

	void print(const CString&);
	CString static_progress;
	CString static_speed;
	CWinThread* downloadThread;
	CString status_value;
};
