#include "stdafx.h"
#include "MyDialog.h"

HHOOK hKeybroadHook = NULL;//快捷键
extern HINSTANCE hInst;
typedef VOID(*pfnSetHook)(BOOL bInstall, DWORD dwThreadId);
extern pfnSetHook pfn;
CMyDialog* CMyDialog::pClass = NULL;
#define WM_TRAYICON WM_APP+2

INT_PTR CALLBACK ProcessFilterDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
{
	switch (message)
	{
	case WM_INITDIALOG:
		if (CMyDialog::pClass->m_FilterProcessId != 0) {
			SetDlgItemInt(hDlg, IDC_PID, CMyDialog::pClass->m_FilterProcessId, FALSE);
		}
		SetFocus(GetDlgItem(hDlg, IDC_PID));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			CMyDialog::pClass->m_FilterProcessId = GetDlgItemInt(hDlg, IDC_PID, NULL, FALSE);
			break;
		}
		if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, IDOK);
		}
	}
	return FALSE;
}


CMyDialog::CMyDialog()
{
	m_IsMenuChecked = FALSE;
	ZeroMemory(&m_nid, sizeof(m_nid));
	m_nid.cbSize = sizeof(m_nid);
	m_FilterProcessId = 0;
	pClass = this;
}

INT_PTR CALLBACK CMyDialog::DlgFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		pClass = new CMyDialog;
		pClass->m_hDlg = hDlg;
		pClass->OnDlgInit();
		return (INT_PTR)TRUE;
	case WM_SIZE:
		if (LOWORD(wParam) == SIZE_MINIMIZED) {
			Shell_NotifyIcon(NIM_ADD, &pClass->m_nid);
			ShowWindow(hDlg, SW_HIDE);
		}
		return (INT_PTR)TRUE;
	case WM_TRAYICON:
		if (lParam == WM_LBUTTONDOWN)
		{
			Shell_NotifyIcon(NIM_DELETE, &pClass->m_nid);
			ShowWindow(hDlg, SW_SHOWNORMAL);
			SetForegroundWindow(hDlg);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		pClass->OnDlgCommand(wParam, lParam);
		return (INT_PTR)TRUE;
	case WM_COPYDATA:
		pClass->OnCopyData(lParam);
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}

LRESULT CALLBACK CMyDialog::KeybroadProc(int code, WPARAM wParam, LPARAM lParam)
{
	LPMSG lpMsg = (LPMSG)lParam;
	if (lpMsg->message == WM_KEYDOWN) {
		switch (LOWORD(lpMsg->wParam))
		{
		case VK_DELETE:
		{ HWND hListBox = GetDlgItem(pClass->m_hDlg, IDC_LIST1);
		DWORD nIndex = ListBox_GetCurSel(hListBox);
		ListBox_DeleteString(hListBox, nIndex);
		ListBox_SetCurSel(hListBox, nIndex - 1);  }
			break;
		case VK_F1: //保存
			SendMessage(pClass->m_hDlg, WM_COMMAND, IDC_SAVE, 0);
			break;
		case VK_F2://筛选进程
			SendMessage(pClass->m_hDlg, WM_COMMAND, IDC_FILTER_PROCESS, 0);
			break;
		case VK_F5://自动监视
			SendMessage(pClass->m_hDlg, WM_COMMAND, ID_WATCH, 0);
			break;
		}
	}
	return (CallNextHookEx(hKeybroadHook, code, wParam, lParam));
}

VOID CMyDialog::OnDlgInit()
{
	pClass->m_hMenu = GetMenu(m_hDlg);
	chSETDLGICONS(m_hDlg, IDI_ICON1);
	SetWindowsHookEx(WH_GETMESSAGE, KeybroadProc, hInst, GetCurrentThreadId());//快捷键

	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	m_nid.hWnd = m_hDlg;
	m_nid.uID = 0;
	m_nid.uCallbackMessage = WM_TRAYICON;
	StringCchPrintf(m_nid.szTip, _countof(m_nid.szTip), _T("单击打开GlobalDebugView主程序"));
}

VOID CMyDialog::OnDlgCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDC_SAVE:
		OnSave();
		break;
	case IDC_EMPTY:
		SendMessage(GetDlgItem(m_hDlg, IDC_LIST1), LB_RESETCONTENT, 0, 0);
		break;
	case ID_WATCH:
		if (!m_IsMenuChecked) {
			CheckMenuItem(m_hMenu, ID_WATCH, MF_CHECKED);
			m_IsMenuChecked = TRUE;
			pfn(TRUE, 0);
		}
		else {
			CheckMenuItem(m_hMenu, ID_WATCH, MF_UNCHECKED);
			m_IsMenuChecked = FALSE;
			pfn(FALSE, 0);

		}
		break;
	case IDC_FILTER_PROCESS:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_FilterProcess), m_hDlg, ProcessFilterDlgProc);
		break;
	}

	if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL){
		EndDialog(m_hDlg, LOWORD(wParam));
	}
}

VOID CMyDialog::OnCopyData(LPARAM lParam)
{
	COPYDATASTRUCT* pCopyDataStruct = (COPYDATASTRUCT*)lParam;
	if (m_FilterProcessId == 0/*未设置筛选进程ID*/ || m_FilterProcessId == pCopyDataStruct->dwData/*与筛选进程ID相同*/) {
		SendMessage(GetDlgItem(m_hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)pCopyDataStruct->lpData);
	}
}

VOID CMyDialog::OnSave()
{
	/*#define FILESIZE 1024*1024*1024*/
	TCHAR szPath[MAX_PATH];
	szPath[0] = '\0';
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szPath;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrFilter = _T("文本文件（*.txt）\0*.txt\0\0");
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = _T("*.txt");
	ofn.hInstance = hInst;
	ofn.hwndOwner = m_hDlg;
	GetSaveFileName(&ofn);

	HANDLE hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	/*HANDLE hFileMap=CreateFileMapping(hFile,0,PAGE_READWRITE,0,FILESIZE,0);
	LPVOID pData=MapViewOfFile(hFileMap,FILE_MAP_WRITE,0,0,0);

	StringCbCatA((LPSTR)pData,FILESIZE,"******************  GlobalDebugView生成报告  ******************\r\n");

	HWND hWndList=GetDlgItem(m_hDlg,IDC_LIST1);

	int i=(int)SendMessage(hWndList,LB_GETCOUNT,0L,0L);
	TCHAR szW[MAX_PATH]={0};
	CHAR szA[MAX_PATH]={0};
	for(int temp=0;temp<i;temp++) {
	SendMessage(hWndList,LB_GETTEXT,temp,(LPARAM)szW);
	WideCharToMultiByte(0,0,szW,-1,szA,_countof(szA),0,0);
	StringCbCatA((LPSTR)pData,FILESIZE,szA);
	StringCbCatA((LPSTR)pData,FILESIZE,"\r\n");
	}

	LONGLONG size=(lstrlenA((LPSTR)pData)+1)-sizeof("\r\n");
	LARGE_INTEGER li;
	li.QuadPart=size;

	UnmapViewOfFile(pData);
	CloseHandle(hFileMap);

	SetFilePointerEx(hFile,li,NULL,FILE_BEGIN);
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	#undef FILESIZE*/
	CHAR szLine[] = "\r\n";
	HWND hListBox = GetDlgItem(m_hDlg, IDC_LIST1);
	TCHAR szW[MAX_PATH * 2];
	CHAR szA[MAX_PATH * 2];
	for (int max = ListBox_GetCount(hListBox), i = 0; i < max; i++) {
		ListBox_GetText(hListBox, i, szW);
		WideCharToMultiByte(CP_ACP, 0, szW, _countof(szW), szA, _countof(szA), 0, 0);
		WriteFile(hFile, szA, lstrlenA(szA), 0, 0);
		WriteFile(hFile, szLine, _countof(szLine), 0, 0);
	}
	CloseHandle(hFile);
}

CMyDialog::~CMyDialog()
{
	delete pClass;
}