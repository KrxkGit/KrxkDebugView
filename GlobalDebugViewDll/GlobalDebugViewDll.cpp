// GlobalDebugViewDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include"APIHook.h"

HHOOK g_Hook=NULL;
HMODULE g_Exclude=NULL;
HWND hWnd=NULL;
extern TCHAR szPath[MAX_PATH];
extern DWORD hCurrentProcessId;


//API拦截
VOID CALLBACK MyOutputDebugStringW(LPCWSTR sz)
{
	int length = lstrlenW(sz)+25;
	LPTSTR p=new WCHAR[length];
	SYSTEMTIME st;
	GetLocalTime(&st);
	StringCchPrintfW(p,length,_T("[%d] %d:%d:%d %s"),hCurrentProcessId,st.wHour,st.wMinute,st.wSecond,sz);

	COPYDATASTRUCT cs={0};
	cs.dwData=hCurrentProcessId;
	cs.cbData=length*sizeof(WCHAR);
	cs.lpData=(PVOID)p;
	SendMessage(hWnd,WM_COPYDATA,0,(LPARAM)&cs);

	OutputDebugStringW(sz);//调用原版函数
}

VOID CALLBACK MyOutputDebugStringA(LPCSTR sz)
{
	int cbSize=lstrlenA(sz)+1;
	LPWSTR p=(LPWSTR)VirtualAlloc(NULL,cbSize*sizeof(WCHAR),MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
	MultiByteToWideChar(CP_ACP,0,sz,cbSize,p,cbSize);
	MyOutputDebugStringW(p);
	VirtualFree(p,0,MEM_RELEASE);
}

VOID CALLBACK MyExitThread(DWORD dwExitCode)
{
	TCHAR sz[MAX_PATH*2];
	StringCchPrintf(sz,_countof(sz),_T("ExitThread 退出代码:%u"),dwExitCode);
	MyOutputDebugStringW(sz);
	ExitThread(dwExitCode);
}

BOOL CALLBACK MyTerminateProcess(HANDLE hProcess,DWORD dwExitcode)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("TerminateProcess 退出进程句柄:%u 退出代码：%u"),hProcess,dwExitcode);
	MyOutputDebugStringW(sz);
	return TerminateProcess(hProcess,dwExitcode);
}

BOOL CALLBACK MyTerminateThread(HANDLE hThread,DWORD dwExitcode)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("TerminateThread 退出线程句柄:%u 退出代码：%u"),hThread,dwExitcode);
	MyOutputDebugStringW(sz);
	return TerminateThread(hThread,dwExitcode);
}

VOID CALLBACK MyExitProcess(UINT uExitCode)
{	
	TCHAR sz[MAX_PATH*2];
	StringCchPrintf(sz,_countof(sz),_T("ExitProcess 退出代码：%u"),uExitCode);
	MyOutputDebugStringW(sz);
	ExitProcess(uExitCode);
}

BOOL CALLBACK MyEndDialog(HWND hWnd,INT_PTR nResult)
{
	TCHAR sz[MAX_PATH*2];
	StringCchPrintf(sz,_countof(sz),_T("EndDialog 窗口句柄：%u  退出代码：%u"),hWnd,nResult);
	MyOutputDebugStringW(sz);
	return EndDialog(hWnd,nResult);
}

INT CALLBACK MyMessageBoxW(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("MessageBoxW 窗口句柄:%u 消息文本:%s 窗口标题:%s 类型:%u"),
		hWnd,lpText,lpCaption,uType);
	MyOutputDebugStringW(sz);
	return MessageBox(hWnd,lpText,lpCaption,uType);
}

INT CALLBACK MyMessageBoxA(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT uType)
{
	CHAR sz[MAX_PATH];
	StringCchPrintfA(sz,_countof(sz),"MessageBoxA 窗口句柄:%u 消息文本:%s 窗口标题:%s 类型:%u",
		hWnd,lpText,lpCaption,uType);
	MyOutputDebugStringA(sz);
	return MessageBoxA(hWnd,lpText,lpCaption,uType);
}


HANDLE WINAPI MyCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,SIZE_T dwStackSize,LPTHREAD_START_ROUTINE lpStartAddress,
							 LPVOID lpParameter,DWORD dwCreationFlags,LPDWORD lpThreadId)
{
	HANDLE hThread=CreateThread(lpThreadAttributes,dwStackSize,lpStartAddress,lpParameter,
		dwCreationFlags,lpThreadId);
	TCHAR sz[MAX_PATH*2];
	StringCchPrintf(sz,_countof(sz),_T("CreateThread 线程函数:%u 父窗口:%u 标志:%u 新线程句柄:%u"),lpStartAddress,lpParameter,
		dwCreationFlags,hThread);
	MyOutputDebugStringW(sz);
	return hThread;
}
//API拦截

LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	//Nothing to do here;
	return (CallNextHookEx(g_Hook,code,wParam,lParam));
}

static HMODULE ModuleFromAddress(PVOID pv) {

	MEMORY_BASIC_INFORMATION mbi;
	return((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
		? (HMODULE) mbi.AllocationBase : NULL);
}

HANDLE WINAPI MyCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	TCHAR sz[MAX_PATH*3];
	StringCchPrintf(sz,_countof(sz),_T("CreateFileW 文件名:%s 共享模式:%d"),lpFileName,dwShareMode);
	MyOutputDebugStringW(sz);
	return CreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

HANDLE WINAPI MyCreateFileA(LPCSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	CHAR sz[MAX_PATH*3];
	StringCchPrintfA(sz,_countof(sz),"CreateFileA 文件名:%s 共享模式:%d",lpFileName,dwShareMode);
	MyOutputDebugStringA(sz);
	return CreateFileA(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

VOID SetHook(BOOL bInstall,DWORD dwThreadId)
{
	if(bInstall) {
		g_Hook=SetWindowsHookEx(WH_GETMESSAGE,GetMsgProc,ModuleFromAddress(SetHook),dwThreadId);
	}
	else {
		UnhookWindowsHookEx(g_Hook);
		g_Hook=NULL;
	}
}



CAPIHook g_OutputDebugStringW("Kernel32.dll","OutputDebugStringW",(PROC)MyOutputDebugStringW);
CAPIHook g_OutputDebugStringA("Kernel32.dll","OutputDebugStringA",(PROC)MyOutputDebugStringA);
CAPIHook g_TerminnataProcess("Kernel32.dll","TerminateProcess",(PROC)MyTerminateProcess);
CAPIHook g_TerminateThread("Kernel32.dll","TerminateThread",(PROC)MyTerminateThread);
CAPIHook g_CreateFileW("Kernel32.dll","CreateFileW",(PROC)MyCreateFileW);
CAPIHook g_CreateFileA("Kernel32.dll","CreateFileA",(PROC)MyCreateFileA);
CAPIHook g_CreateThread("Kernel32.dll","CreateThread",(PROC)MyCreateThread);

CAPIHook g_EndDialog("User32.dll","EndDialog",(PROC)MyEndDialog);
CAPIHook g_MessageBoxW("User32.dll","MessageBoxW",(PROC)MyMessageBoxW);
CAPIHook g_MessageBoxA("User32.dll","MessageBoxA",(PROC)MyMessageBoxA);

