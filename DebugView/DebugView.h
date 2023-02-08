#pragma once

#include "resource.h"

#define WM_TRAYICON WM_APP+1

VOID On_DlgInit(HWND);
VOID On_DlgCommand(HWND,WPARAM,LPARAM);
VOID RegisterDlgClass(LPTSTR);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
VOID LoadInstance(HWND);
VOID AddText(HWND,PCTSTR, ...);
VOID EnumProcess(HWND);
UINT CALLBACK MyCreateRemoteThread(LPVOID pParam);//线程函数
VOID OnSave(HWND hWnd);
DWORD GetListProcessId(HWND hDlg);

VOID OnConnentTo(HWND/*父窗口*/);
INT_PTR CALLBACK ConnentToDlg(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK OnWaitToConnent(LPVOID);

LRESULT CALLBACK KeybroadProc(int code, WPARAM wParam, LPARAM lParam);//快捷键