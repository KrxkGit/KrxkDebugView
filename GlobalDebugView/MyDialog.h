#pragma once
#include"stdafx.h"
#include"CmnHdr.h"
#include"resource.h"


class CMyDialog
{
public:
	CMyDialog();
	~CMyDialog();
	static INT_PTR CALLBACK DlgFunc(HWND,UINT,WPARAM,LPARAM);
private:
	HWND m_hDlg;
	HMENU m_hMenu;
	BOOL m_IsMenuChecked;
	NOTIFYICONDATA m_nid;
	DWORD m_FilterProcessId;
	static CMyDialog* pClass;
private:
	VOID OnDlgInit();
	VOID OnDlgCommand(WPARAM,LPARAM);
	VOID OnCopyData(LPARAM);
	VOID OnSave();
	static LRESULT CALLBACK KeybroadProc(int code, WPARAM wParam, LPARAM lParam);
	friend INT_PTR CALLBACK ProcessFilterDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM);
};

