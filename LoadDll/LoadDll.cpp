// LoadDll.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "LoadDll.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szDllPath[MAX_PATH];


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	hInst=hInstance;
	DialogBox(hInst,MAKEINTRESOURCE(IDD_MainDialog),NULL,MainDialog);
	return GetLastError();
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if(LOWORD(wParam)==IDC_BUTTON1) {
			ShellExecute(NULL,_T("open"),_T("https://github.com/KrxkGit"),NULL,NULL,SW_NORMAL);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

DWORD GetListProcessId(HWND hDlg)
{

	TCHAR sz[MAX_PATH];
	ComboBox_GetText(GetDlgItem(hDlg,IDC_COMBO1),sz,_countof(sz));
	HANDLE hTlHelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
	TCHAR szFormat[MAX_PATH+5];
	__try {
		PROCESSENTRY32 pe;
		pe.dwSize=sizeof(pe);
		for(BOOL b = Process32First(hTlHelp,&pe);b;b=Process32Next(hTlHelp,&pe)) {
			wsprintf(szFormat,_T("%s[%u]"),pe.szExeFile,pe.th32ProcessID);
			if(lstrcmp(sz,szFormat)==0) 
				return pe.th32ProcessID;
		}
	}
	__finally {
		CloseHandle(hTlHelp);
	}
	return -1;
}

VOID OnListProcess(HWND hDlg)
{
	HWND hCombo = GetDlgItem(hDlg,IDC_COMBO1);
	ComboBox_ResetContent(hCombo);
	TCHAR sz[MAX_PATH*2];
	HANDLE hTlHelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
	PROCESSENTRY32 pe;
	pe.dwSize=sizeof(pe);
	for(BOOL b = Process32First(hTlHelp,&pe);b;b=Process32Next(hTlHelp,&pe)) {
		wsprintf(sz,_T("%s[%u]"),pe.szExeFile,pe.th32ProcessID);
		ComboBox_AddString(hCombo,sz);
	}
	CloseHandle(hTlHelp);
}

VOID inline OnInit(HWND hDlg)
{
	SendMessage(hDlg,WM_SETICON,0,(LPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_LOADDLL)));
	OnListProcess(hDlg);
}

VOID OnOpenFile(HWND hDlg)
{
	szDllPath[0]='\0';
	OPENFILENAME ofn={0};
	ofn.hInstance=hInst;
	ofn.lpstrFile=szDllPath;
	ofn.lpstrFilter=_T("动态链接库(*.dll)\0*.dll\0\0");
	ofn.lStructSize=sizeof(ofn);
	ofn.nMaxFile=MAX_PATH;
	GetOpenFileName(&ofn);
	SetDlgItemText(hDlg,IDC_EDIT1,szDllPath);
}


VOID OnLoadDll(HWND hDlg)
{
	if(szDllPath[0]=='\0') {
		MessageBox(hDlg,_T("选择Dll文件？"),NULL,MB_ICONQUESTION | MB_YESNO)==IDYES?OnOpenFile(hDlg):FALSE;
		return;
	}
	/*获取进程Id*/
	DWORD dwProcessId = GetListProcessId(hDlg);
	if(dwProcessId==-1) {
		return;
	}

	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
		FALSE,dwProcessId);
	if(hProcess==NULL) {
		MessageBox(hDlg,_T("打开目标进程错误！"),NULL,MB_ICONERROR);
		return;
	}
	LPVOID p=VirtualAllocEx(hProcess,NULL,sizeof(TCHAR)*MAX_PATH,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
	WriteProcessMemory(hProcess,p,szDllPath,sizeof(TCHAR)*MAX_PATH,NULL);
	HANDLE hThread = CreateRemoteThread(hProcess,0,0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32.dll")),"LoadLibraryW"),p,0,0);
	WaitForSingleObject(hThread,INFINITE);
	VirtualFreeEx(hProcess,p,0,MEM_RELEASE);
	CloseHandle(hProcess);
	MessageBox(hDlg,_T("注入成功！"),_T("LoadDll"),MB_ICONINFORMATION);
}


VOID inline OnCommand(HWND hDlg,WPARAM wParam,LPARAM)
{
	switch (LOWORD(wParam))
	{
	case IDC_BUTTON1:
		OnOpenFile(hDlg);
		break;
	case IDOK:
		OnLoadDll(hDlg);
		break;
	case IDC_BUTTON3:
		OnListProcess(hDlg);
		break;
	case IDCANCEL:
		EndDialog(hDlg,IDCANCEL);
		break;
	case IDM_ABOUT:
		DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUTBOX),hDlg,About);
		break;
	}
}

INT_PTR CALLBACK MainDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		OnInit(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		OnCommand(hDlg,wParam,lParam);
		return TRUE;
	}
	return (INT_PTR)FALSE;
}