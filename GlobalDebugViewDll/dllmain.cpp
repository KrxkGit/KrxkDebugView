// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

extern HWND hWnd;
TCHAR szPath[MAX_PATH];
DWORD hCurrentProcessId = NULL;

VOID CALLBACK MyOutputDebugStringW(LPCWSTR sz);


VOID DllInit()
{
	hCurrentProcessId = GetCurrentProcessId();
	GetModuleFileName(NULL,szPath,_countof(szPath));
	hWnd=FindWindow(_T("GlobalDebugView"),NULL);
	TCHAR sz[MAX_PATH*2];
	StringCchPrintf(sz,_countof(sz),_T("DLL被加载！ 应用程序路径：%s"),szPath);
	MyOutputDebugStringW(sz);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DllInit();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		TCHAR sz[MAX_PATH*2];
		StringCchPrintf(sz,_countof(sz),_T("DLL被卸载！ 应用程序路径：%s"),szPath);
		MyOutputDebugStringW(sz);
		break;
	}
	return TRUE;
}

