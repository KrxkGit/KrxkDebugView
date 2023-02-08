// DebugViewDLL.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "DebugViewDLL.h"


TCHAR szPath[MAX_PATH];//Ӧ�ó���·��
HMODULE g_Exclude=NULL;
HWND hWnd=NULL;
DWORD hCurrentProcessId=0;
HMODULE hInstDll=NULL;

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, PVOID fImpLoad) 
{
	switch (fdwReason) 
	{
	case DLL_PROCESS_ATTACH:
		::hInstDll=hInstDll;
		DllInit();//��ʼ��DLL
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		TCHAR sz[MAX_PATH*2]=L"";
		StringCchPrintf(sz,_countof(sz),_T("DLL��ж�أ�\t\tӦ�ó���·����%s"),szPath);
		MyOutputDebugStringW(sz);
		break;
	}
	return(TRUE);
}

DWORD CALLBACK FreeDllThread(LPVOID pWM)
{
	HANDLE hWaitFreeDll = OpenEvent(SYNCHRONIZE,FALSE,_T("DebugView"));
	WaitForSingleObject(hWaitFreeDll,INFINITE);
	CloseHandle(hWaitFreeDll);
	FreeLibraryAndExitThread(hInstDll,GetLastError());
}

//API����
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

	OutputDebugStringW(sz);//����ԭ�溯��
}

VOID CALLBACK MyOutputDebugStringA(LPCSTR sz)
{
	int cbSize=lstrlenA(sz)+1;
	LPWSTR p=(LPWSTR)VirtualAlloc(NULL,cbSize*sizeof(WCHAR),MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
	MultiByteToWideChar(0,0,sz,cbSize,p,cbSize);
	MyOutputDebugStringW(p);
	VirtualFree(p,0,MEM_RELEASE);
}

VOID CALLBACK MyExitThread(DWORD dwExitCode)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("ExitThread �˳����룺%u"),dwExitCode);
	MyOutputDebugStringW(sz);
	ExitThread(dwExitCode);
}

BOOL CALLBACK MyTerminateProcess(HANDLE hProcess,DWORD dwExitcode)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("TerminateProcess. �˳����̾����%u.  �˳����룺%u"),hProcess,dwExitcode);
	MyOutputDebugStringW(sz);
	return TerminateProcess(hProcess,dwExitcode);
}

BOOL CALLBACK MyTerminateThread(HANDLE hThread,DWORD dwExitcode)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("TerminateThread. �˳��߳̾����%u.  �˳����룺%u"),hThread,dwExitcode);
	MyOutputDebugStringW(sz);
	return TerminateThread(hThread,dwExitcode);
}

HANDLE WINAPI MyCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	TCHAR sz[MAX_PATH*3];
	StringCchPrintf(sz,_countof(sz),_T("CreateFileW �ļ���:%s ����ģʽ:%d"),lpFileName,dwShareMode);
	MyOutputDebugStringW(sz);
	return CreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

HANDLE WINAPI MyCreateFileA(LPCSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	CHAR sz[MAX_PATH*3];
	StringCchPrintfA(sz,_countof(sz),"CreateFileA �ļ���:%s ����ģʽ:%d",lpFileName,dwShareMode);
	MyOutputDebugStringA(sz);
	return CreateFileA(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

VOID CALLBACK MyExitProcess(UINT uExitCode)
{	
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("ExitProcess �˳����룺%u"),uExitCode);
	MyOutputDebugStringW(sz);
	ExitProcess(uExitCode);
}

BOOL CALLBACK MyEndDialog(HWND hWnd,INT_PTR nResult)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("EndDialog ���ھ����%u  �˳����룺%u"),hWnd,nResult);
	MyOutputDebugStringW(sz);
	return EndDialog(hWnd,nResult);
}

INT CALLBACK MyMessageBoxW(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType)
{
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("MessageBoxW  ���ھ����%u  ��Ϣ�ı���%s  ���ڱ��⣺%s    ���ͣ�%u"),
		hWnd,lpText,lpCaption,uType);
	MyOutputDebugStringW(sz);
	return MessageBox(hWnd,lpText,lpCaption,uType);
}

INT CALLBACK MyMessageBoxA(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT uType)
{
	CHAR sz[MAX_PATH];
	StringCchPrintfA(sz,_countof(sz),"MessageBoxA  ���ھ����%u  ��Ϣ�ı���%s  ���ڱ��⣺%s    ���ͣ�%u",
		hWnd,lpText,lpCaption,uType);
	MyOutputDebugStringA(sz);
	return MessageBoxA(hWnd,lpText,lpCaption,uType);
}

/*DWORD CALLBACK MyGetLastError()
{
	DWORD dwError=GetLastError();
	TCHAR sz[MAX_PATH];
	HLOCAL hlocal;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,dwError,0,(LPWSTR)&hlocal,0,0);
	//StringCchPrintf(sz,_countof(sz),_T("GetLastError ������룺%u  ��Ϣ�ı���%s"),dwError,hlocal);
	wsprintf(sz,_T("GetLastError ������룺%u  ��Ϣ�ı���%s"),dwError,hlocal);
	MyOutputDebugStringW(sz);
	return dwError;
}*/

HANDLE WINAPI MyCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,SIZE_T dwStackSize,LPTHREAD_START_ROUTINE lpStartAddress,
			LPVOID lpParameter,DWORD dwCreationFlags,LPDWORD lpThreadId)
{
	HANDLE hThread=CreateThread(lpThreadAttributes,dwStackSize,lpStartAddress,lpParameter,
								dwCreationFlags,lpThreadId);
	TCHAR sz[MAX_PATH];
	StringCchPrintf(sz,_countof(sz),_T("CreateThread �̺߳���:%u ������:%u ��־:%u ���߳̾��:%u"),lpStartAddress,lpParameter,
		dwCreationFlags,hThread);
	MyOutputDebugStringW(sz);
	return hThread;
}
//API����


static HMODULE ModuleFromAddress(PVOID pv) {

   MEMORY_BASIC_INFORMATION mbi;
   return((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
      ? (HMODULE) mbi.AllocationBase : NULL);
}

VOID DllInit()
{
	CreateThread(0,0,FreeDllThread,0,0,0);//�ȴ�ж��Dll
	hCurrentProcessId=GetCurrentProcessId();
	GetModuleFileName(NULL,szPath,_countof(szPath));
	hWnd=FindWindow(_T("DebugView"),NULL);
	TCHAR sz[MAX_PATH*2];
	StringCchPrintf(sz,_countof(sz),_T("DLL�����أ�\tӦ�ó���·����%s"),szPath);
	MyOutputDebugStringW(sz);
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

