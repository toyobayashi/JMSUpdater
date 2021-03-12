// CPatchDownloadDlg.cpp: 实现文件
//

#include "pch.h"
#include "JMSUpdater.h"
#include "CPatchDownloadDlg.h"
#include "afxdialogex.h"
#include "afxinet.h"

#include <string>
#include "../wz/lib.h"

// CPatchDownloadDlg 对话框

//参数 文件绝对路径 和 命令行
/* static DWORD Execute(LPCTSTR pszExeFile, LPCTSTR pszCmdLine) {
  // 启动子进程
  PROCESS_INFORMATION pi;
  STARTUPINFO si = { sizeof(si) };
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
  BOOL ret = CreateProcess(pszExeFile, (LPTSTR)pszCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
  if (!ret) {
    return -1;
  }

  DWORD dwExitCode;
  
  // 等待子进程的退出
  WaitForSingleObject(pi.hProcess, INFINITE);
  // 获取子进程的退出码
  GetExitCodeProcess(pi.hProcess, &dwExitCode);

  // 关闭子进程的主线程句柄
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  return dwExitCode;
} */

static CString GetStatusString(DOWN_TASK_STATUS status) {
  switch (status) {
  case NOITEM: return _T("空");
  case TSC_ERROR: return _T("错误");
  case TSC_PAUSE: return _T("已暂停");
  case TSC_DOWNLOAD: return _T("下载中");
  case TSC_COMPLETE: return _T("已完成");
  case TSC_STARTPENDING: return _T("正在开始");
  case TSC_STOPPENDING: return _T("正在停止");
  default: return _T("未知");
  }
}

int PreArgHandler(char* fileName, char* baseFullPath, int type, char* outMessage, int outMessageLength);

static void MakePatch(CPatchDownloadDlg* self, const CString& name) {
  WCHAR szModulePath[MAX_PATH] = { 0 };
  GetModuleFileNameW(NULL, szModulePath, MAX_PATH);
  PathRemoveFileSpecW(szModulePath);
  WCHAR wexe[MAX_PATH] = { 0 };
  PathCombineW(wexe, szModulePath, L"NXPatcherLite.exe");
  WCHAR targetdir[MAX_PATH] = { 0 };
  PathCombineW(targetdir, szModulePath, L"data");
  CString exe = wexe;
  CString path = CString(targetdir) + _T("\\") + name;
  CString cmdl = exe + _T(" ") + path;
  CString* make = new CString(_T("正在制作补丁程序..."));
  ::PostMessageW(self->m_hWnd, WM_PATCH_LOG, 0, (LPARAM)make);
  char pathA[MAX_PATH];
  WideCharToMultiByte(CP_ACP, 0, path.GetString(), -1, pathA, MAX_PATH, NULL, NULL);
  // if (Execute(wexe, cmdl.GetString()) == 0) {
  int index = self->type_select.GetCurSel();
  const version_option* option = (version_option*) self->type_select.GetItemData(index);
  char baseFullPath[MAX_PATH] = { 0 };
  WCHAR baseFullPathW[MAX_PATH] = { 0 };
  if (option->relative_path != _T("")) {
    PathCombineW(baseFullPathW, szModulePath, option->relative_path.GetString());
    WideCharToMultiByte(CP_ACP, 0, baseFullPathW, -1, baseFullPath, MAX_PATH, NULL, NULL);
  }
  char msg[512] = { 0 };
  wchar_t msgW[512] = { 0 };
  if (PreArgHandler(pathA, baseFullPath, option->type, msg, sizeof(msg))) {
    CString * compl = new CString(_T("已完成"));
    MultiByteToWideChar(CP_ACP, 0, msg, -1, msgW, 512);
    compl->Append(_T("："));
    compl->Append(CString(msgW));
    ::PostMessageW(self->m_hWnd, WM_PATCH_LOG, 0, (LPARAM)compl);
  } else {
    CString * compl = new CString(_T("制作失败："));
    MultiByteToWideChar(CP_ACP, 0, msg, -1, msgW, 512);
    compl->Append(CString(msgW));
    ::PostMessageW(self->m_hWnd, WM_PATCH_LOG, 0, (LPARAM)compl);
  }
  return;
}

IMPLEMENT_DYNAMIC(CPatchDownloadDlg, CDialogEx)

CPatchDownloadDlg::CPatchDownloadDlg(CWnd* pParent /*=nullptr*/)
  : CDialogEx(IDD_PATCHDOWNLOAD, pParent), log_value(_T(""))
  , input_from_value(_T("")), input_to_value(_T("")), dl(), version_options_list(), taskHandle(NULL), static_progress(_T("")), static_speed(_T(""))
  , downloadThread(NULL), status_value(_T("")) {

  version_options_list.push_back({ 1, _T("默认"), _T("") });
  version_options_list.push_back({ 1, _T("旧国际服"), _T("patchers\\PatcherGMSOld.exe") });
  version_options_list.push_back({ 2, _T("国服"), _T("patchers\\PatcherCMS.exe") });
  version_options_list.push_back({ 2, _T("东南亚服"), _T("patchers\\PatcherSEA.exe") });
}

CPatchDownloadDlg::~CPatchDownloadDlg()
{
  dl.UnEngine();
}

BOOL CPatchDownloadDlg::OnInitDialog() {
  CDialogEx::OnInitDialog();
  progress.SetRange(0, 10000);
  input_from_value = L"393";
  input_to_value = L"394";

  WCHAR szModulePath[MAX_PATH] = { 0 };
  GetModuleFileNameW(NULL, szModulePath, MAX_PATH);
  PathRemoveFileSpecW(szModulePath);

  for (const auto& o : version_options_list) {
    if (o.label == _T("默认")) {
      int index = type_select.InsertString(-1, o.label);
      type_select.SelectString(-1, o.label);
      type_select.SetItemData(index, (DWORD_PTR)&o);
      continue;
    }

    WCHAR path[MAX_PATH] = { 0 };
    PathCombineW(path, szModulePath, o.relative_path.GetString());
    if (PathFileExistsW(path)) {
      int index = type_select.InsertString(-1, o.label);
      type_select.SetItemData(index, (DWORD_PTR)&o);
    }
  }

  UpdateData(0);

  dl.InitEngine();
  return FALSE;
}

void CPatchDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FROM_EDIT, input_from);
  DDX_Control(pDX, IDC_TO_EDIT, input_to);
  DDX_Control(pDX, IDC_TYPE_SELECT, type_select);
  DDX_Control(pDX, IDC_START_BUTTON, button);
  DDX_Control(pDX, IDC_PROGRESS1, progress);
  DDX_Control(pDX, IDC_LOG_EDIT, log);
  DDX_Text(pDX, IDC_FROM_EDIT, input_from_value);
  DDX_Text(pDX, IDC_TO_EDIT, input_to_value);
  // DDX_Text(pDX, IDC_LOG_EDIT, log_value);
  DDX_Text(pDX, IDC_STATIC_PROGRESS, static_progress);
  DDX_Text(pDX, IDC_STATIC_SPEED, static_speed);
  DDX_Text(pDX, IDC_STATIC_STATUS, status_value);
}


BEGIN_MESSAGE_MAP(CPatchDownloadDlg, CDialogEx)
  ON_BN_CLICKED(IDC_START_BUTTON, &CPatchDownloadDlg::OnBnClickedStartButton)
  ON_MESSAGE(WM_UPDATE_PROGRESS, &CPatchDownloadDlg::OnProgress)
  ON_MESSAGE(WM_PATCH_LOG, &CPatchDownloadDlg::OnLog)
  ON_BN_CLICKED(IDC_CHECK_BUTTON, &CPatchDownloadDlg::OnBnClickedCheckButton)
  ON_BN_CLICKED(IDC_CHECK_LATEST, &CPatchDownloadDlg::OnBnClickedCheckLatest)
END_MESSAGE_MAP()

static BOOL checkVersionAvailable(const CString& fromValue, const CString& toValue, CString& msg) {
  CString fromValueStr, toValueStr;
  fromValueStr.Format(_T("%05d"), _ttoi(fromValue));
  toValueStr.Format(_T("%05d"), _ttoi(toValue));
  if (fromValueStr == toValueStr) {
    msg.Format(_T("不支持从 %s 升级到 %s"), fromValue.GetString(), toValue.GetString());
    return FALSE;
  }
  INTERNET_PORT nPort;
  CString strServer, strObject;
  DWORD dwServiceType, statusCode;
  CString strUrl = _T("http://webdown2.nexon.co.jp/maple/patch/patchdir/");
  strUrl += toValueStr;
  strUrl += _T("/Version.info");

  if (!AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort)) {
    msg = _T("解析失败");
    return FALSE;
  }
  CInternetSession session;
  session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000 * 20);
  CHttpConnection* connection = NULL;
  try {
    connection = session.GetHttpConnection(strServer, nPort);
  } catch (CInternetException* e) {
    connection = NULL;
    WCHAR msgtemp[1024];
    e->GetErrorMessage(msgtemp, 1024);
    e->Delete();
    msg = msgtemp;
    session.Close();
    return FALSE;
  }
  if (connection == NULL) {
    session.Close();
    msg = _T("网络连接失败");
    return FALSE;
  }

  CHttpFile* file = connection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
  if (file == NULL) {
    session.Close();
    connection->Close();
    delete connection;
    msg = _T("请求失败");
    return FALSE;
  }

  file->SendRequest();

  file->QueryInfoStatusCode(statusCode);

  if (statusCode == HTTP_STATUS_NOT_FOUND) {
    session.Close();
    connection->Close();
    file->Close();
    delete connection;
    delete file;
    msg.Format(_T("版本不存在：%s"), toValue.GetString());
    return FALSE;
  }

  if (statusCode != HTTP_STATUS_OK) {
    session.Close();
    connection->Close();
    file->Close();
    delete connection;
    delete file;
    CString txt;
    txt.Format(_T("请求失败：%u"), statusCode);
    msg = txt;
    return FALSE;
  }

  UINT nReaded = 0;
  std::string responseText;
  char buf[8096] = { 0 };
  try {
    while ((nReaded = file->Read((void*)buf, 8095)) > 0) {
      buf[nReaded] = '\0';
      responseText += buf;
    }
  } catch (CInternetException* e) {
    WCHAR msgtemp[1024];
    e->GetErrorMessage(msgtemp, 1024);
    e->Delete();
    msg = msgtemp;
    session.Close();
    connection->Close();
    file->Close();
    delete connection;
    delete file;
    return FALSE;
  }
 
  int len = MultiByteToWideChar(CP_UTF8, 0, responseText.c_str(), -1, NULL, 0);
  WCHAR* responseW = new WCHAR[len];
  MultiByteToWideChar(CP_UTF8, 0, responseText.c_str(), -1, responseW, len);
  CString res(responseW);
  delete[] responseW;

  session.Close();
  connection->Close();
  file->Close();
  delete connection;
  delete file;

  CString sub = res;
  int l = 0;
  int rnindex;
  CString left;
  CString right;
  int versioncode = _ttoi(fromValue);
  CString target;
  target.Format(_T("%05d"), versioncode);
  while ((rnindex = sub.Find(_T("\r\n"))) != -1) {
    left = res.Mid(l, rnindex);
    right = res.Right(res.GetLength() - rnindex - l - 2);
    if (left.Find(_T("0x")) != 0 && left == target) {
      return TRUE;
    }
    sub = right;
    l += rnindex + 2;
  }
  
  msg.Format(_T("不支持从 %s 升级到 %s"), fromValue.GetString(), toValue.GetString());
  return FALSE;
}

static void download(void* s) {
  CPatchDownloadDlg* self = (CPatchDownloadDlg*)s;
  CString errmsg;
  BOOL r = checkVersionAvailable(self->input_from_value, self->input_to_value, errmsg);
  if (!r) {
    CString* printmsg = new CString(errmsg);
    ::PostMessageW(self->m_hWnd, WM_PATCH_LOG, 1, (LPARAM)printmsg);
    ::PostMessageW(self->m_hWnd, WM_UPDATE_PROGRESS, 1, 0);
    return;
  }
  CString fromValueStr, toValueStr;
  fromValueStr.Format(_T("%05d"), _ttoi(self->input_from_value));
  toValueStr.Format(_T("%05d"), _ttoi(self->input_to_value));
  if (self->taskHandle != NULL) {
    self->dl.TaskDelete(self->taskHandle);
    // self->dl.TaskStart(self->taskHandle);
  }
  WCHAR szModulePath[MAX_PATH] = { 0 };
  GetModuleFileNameW(NULL, szModulePath, MAX_PATH);
  PathRemoveFileSpecW(szModulePath);

  WCHAR targetdir[MAX_PATH] = { 0 };
  PathCombineW(targetdir, szModulePath, L"data");
  CreateDirectoryW(targetdir, NULL);

  CString name = fromValueStr + _T("to") + toValueStr + _T(".patch");
  CString url = CString(L"https://webdown2.nexon.co.jp/maple/patch/patchdir/")+ toValueStr + _T("/") + name;
  
  if (PathFileExistsW(CString(targetdir) + _T("\\") + name)) {
    MakePatch(self, name);
    ::PostMessageW(self->m_hWnd, WM_UPDATE_PROGRESS, 1, 0);
    return;
  }
  self->taskHandle = self->dl.CreateTaskByURL(
    url.GetString(),
    targetdir,
    name.GetString(),
    TRUE
  );

  self->dl.TaskStart(self->taskHandle);

  DownTaskInfo info;
  self->dl.TaskQueryInfoEx(self->taskHandle, info);

  CString* downloadurl = new CString(_T("开始下载: "));
  downloadurl->operator+=(url);
  ::PostMessageW(self->m_hWnd, WM_PATCH_LOG, 0, (LPARAM)downloadurl);

  BOOL fn = FALSE;
  BOOL sz = FALSE;
  while (true) {
    Sleep(1000);
    DownTaskInfo* info = new DownTaskInfo;
    self->dl.TaskQueryInfoEx(self->taskHandle, *info);

    if (!fn && CString(info->szFilename) != _T("")) {
      CString* filename = new CString(_T("文件名: "));
      filename->operator+=(targetdir);
      filename->operator+=(_T("\\"));
      filename->operator+=(info->szFilename);
      
      ::PostMessageW(self->m_hWnd, WM_PATCH_LOG, 0, (LPARAM)filename);
      
      fn = TRUE;
    }

    if (!sz && info->nTotalSize != 0) {
      CString* size = new CString(_T("文件大小: "));
      double mb = (double)info->nTotalSize / 1000 / 1000;
      CString s;
      s.Format(_T("%.2lf MB"), mb);
      size->operator+=(s);
      ::PostMessageW(self->m_hWnd, WM_PATCH_LOG, 0, (LPARAM)size);
      sz = TRUE;
    }

    ::PostMessageW(self->m_hWnd, WM_UPDATE_PROGRESS, 0, (LPARAM)info);
    if (info->stat == TSC_PAUSE || info->stat == TSC_ERROR || info->stat == TSC_COMPLETE) {
      if (info->stat == TSC_COMPLETE) {
        MakePatch(self, info->szFilename);
      }
      ::PostMessageW(self->m_hWnd, WM_UPDATE_PROGRESS, 1, 0);
      return;
    }
  }
}

// CPatchDownloadDlg 消息处理程序

void CPatchDownloadDlg::OnBnClickedStartButton() {
  UpdateData(1);
  if (input_from_value == "") {
    MessageBox(_T("请输入开始版本"), _T("提示"), MB_ICONWARNING);
    return;
  }
  if (input_to_value == "") {
    MessageBox(_T("请输入目标版本"), _T("提示"), MB_ICONWARNING);
    return;
  }
  button.EnableWindow(0);
  input_from.EnableWindow(0);
  input_to.EnableWindow(0);
  if (!is_downloading) {
    is_downloading = true;
    static_progress = _T("");
    static_speed = _T("");
    status_value = _T("");
    progress.SetPos(0);
    UpdateData(0);
    downloadThread = AfxBeginThread((AFX_THREADPROC)download, this, THREAD_PRIORITY_IDLE);
    button.EnableWindow(1);
    button.SetWindowTextW(L"停止");
  } else {
    is_downloading = false;
    dl.TaskStop(taskHandle);
    this->print(_T("停止操作"));
    button.SetWindowTextW(L"开始");
  }
}

LRESULT CPatchDownloadDlg::OnProgress(WPARAM wParam, LPARAM lParam) {
  if (wParam == 1) {
    button.EnableWindow(1);
    input_from.EnableWindow(1);
    input_to.EnableWindow(1);
    button.SetWindowTextW(L"开始");
    is_downloading = false;
    progress.SetPos(0);
    return 0;
  }
  DownTaskInfo* info = (DownTaskInfo*)lParam;
  progress.SetPos((int)(info->fPercent * 10000));
  CString p;
  p.Format(_T("%0.2lf"), (double)info->fPercent * 100);
  p += _T("%");
  CString n;
  n.Format(_T("%0.2lf"), (double)info->nTotalDownload / 1000 / 1000);
  n += _T(" MB");
  static_progress = n + _T("(") + p + _T(")");
  CString s;
  s.Format(_T("%d KB/s"), info->nSpeed / 1000);
  static_speed = s;
  status_value = GetStatusString(info->stat);
  UpdateData(0);
  delete info;
  return 0;
}

LRESULT CPatchDownloadDlg::OnLog(WPARAM wParam, LPARAM lParam) {
  CString* v = (CString*)lParam;
  this->print(*v);
  if (wParam == 1) {
    MessageBox(*v);
  }
  delete v;
  return 0;
}

void CPatchDownloadDlg::print(const CString& msg) {
  log_value += msg + _T("\r\n");
  log.SetWindowTextW(log_value);
  log.LineScroll(log.GetLineCount());
}


void CPatchDownloadDlg::OnBnClickedCheckButton() {
  OPENFILENAME ofn;
  std::wstring szFile;
  std::wstring currentDirectory;
  std::wstring szFileTitle;
  int result = 1;

  szFileTitle.resize(MAX_PATH);
  szFile.resize(MAX_PATH);
  currentDirectory.resize(MAX_PATH);

  GetCurrentDirectory(currentDirectory.length(), &currentDirectory[0]);

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = &szFile[0];
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = szFile.length();
  ofn.lpstrFilter = _T("WZ (*.wz)\0*.wz");
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = &szFileTitle[0];
  ofn.nMaxFileTitle = szFileTitle.length();
  ofn.lpstrInitialDir = currentDirectory.c_str();
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  result = GetOpenFileName(&ofn);

  if (result) {
    char path[MAX_PATH] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, ofn.lpstrFile, -1, path, MAX_PATH, NULL, NULL);
    
    char ver[128] = { 0 };
    WCHAR verW[128] = { 0 };
    int r = get_wz_version(path, ver, 128);

    MultiByteToWideChar(CP_ACP, 0, ver, -1, verW, 128);
    MessageBox(verW, r == 0 ? _T("版本检测失败") : _T("客户端版本"), r == 0 ? MB_ICONERROR : MB_ICONINFORMATION);
    if (r) {
      this->input_from_value = verW;
      UpdateData(0);
    }
  }
}


void CPatchDownloadDlg::OnBnClickedCheckLatest() {
  ShellExecute(NULL, _T("open"), L"https://maplestory.nexon.co.jp/notice/list/update/", NULL, NULL, SW_SHOWNORMAL);
}
