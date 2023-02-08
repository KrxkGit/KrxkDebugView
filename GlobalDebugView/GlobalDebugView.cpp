// GlobalDebugView.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "GlobalDebugView.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
typedef VOID (*pfnSetHook)(BOOL bInstall,DWORD dwThreadId);
HMODULE hDll=NULL;
pfnSetHook pfn=NULL;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst=hInstance;
	//拦截函数
	hDll=LoadLibrary(_T("GlobalDebugViewDll.dll"));
	if(hDll==NULL) {
		MessageBox(NULL,_T("找不到相关DLL！"),_T("GlobaoDebugView"),MB_OK | MB_ICONERROR);
		ExitProcess(GetLastError());
	}
	pfn=(pfnSetHook)GetProcAddress(hDll,"SetHook");
	//注册窗口类
	WNDCLASS wc;
	GetClassInfo(hInst,_T("#32770"),&wc);
	wc.lpszClassName=_T("GlobalDebugView");
	RegisterClass(&wc);

	DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUTBOX),NULL,CMyDialog::DlgFunc);

	FreeLibrary(hDll);
	return GetLastError();
}






