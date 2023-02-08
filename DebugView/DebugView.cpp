// DebugView.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "DebugView.h"
#define MAX_LOADSTRING 100
#define DEFAULT_PORT "32170"
#define TEST_STRING "完成连接"

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szClassName[] = _T("DebugView");
NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
DWORD dwProcessId;//被加载的进程句柄ID
//网络全局变量
LPSTR szIPAddress;
SOCKET ConnectSocket = INVALID_SOCKET;
SOCKET ServiceSocket = INVALID_SOCKET;
//DLL模块
HMODULE hDll = NULL;
HHOOK hKeybroadHook = NULL;//快捷键
HWND hDlg = NULL;

// 此代码模块中包含的函数的前向声明:


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	hInst = hInstance;
	__try {
		hDll = LoadLibrary(_T("Running"));
		if (hDll == NULL) {
			MessageBox(NULL, _T("找不到指定DLL\n部分功能不可用！"), _T("Warning"), MB_ICONERROR);
			__leave;
		}
		FARPROC pfn = GetProcAddress(hDll, "KeepRunning");
		pfn == NULL ? NULL : pfn();
	}
	__finally {

	}
	RegisterDlgClass(szClassName);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	HANDLE hWaitFreeDll = CreateEvent(0, TRUE, FALSE, _T("DebugView"));
	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);

	closesocket(ConnectSocket);
	closesocket(ServiceSocket);
	WSACleanup();
	if (hDll != NULL)
	{
		FreeLibrary(hDll);
	}
	//触发事件
	SetEvent(hWaitFreeDll);
	CloseHandle(hWaitFreeDll);
	return GetLastError();
}

VOID RegisterDlgClass(LPTSTR szClassName)
{
	//注册自定义窗口类
	WNDCLASS wc;
	GetClassInfo(hInst, _T("#32770"), &wc);
	wc.lpszClassName = szClassName;
	RegisterClass(&wc);
}

LRESULT CALLBACK KeybroadProc(int code, WPARAM wParam, LPARAM lParam)
{
	LPMSG lpMsg = (LPMSG)lParam;
	if (lpMsg->message == WM_KEYDOWN) {
		switch (LOWORD(lpMsg->wParam))
		{
		case VK_DELETE:
		{ HWND hListBox = GetDlgItem(hDlg, IDC_LIST1);
		DWORD nIndex = ListBox_GetCurSel(hListBox);
		ListBox_DeleteString(hListBox, nIndex);
		ListBox_SetCurSel(hListBox, nIndex - 1);  }
			break;
		case VK_F1: //保存
			SendMessage(hDlg, WM_COMMAND, ID_SAVE, 0);
			break;
		case VK_F2://加载
			SendMessage(hDlg, WM_COMMAND, IDC_LOAD, 0);
			break;
		case VK_F3://清空
			SendMessage(hDlg, WM_COMMAND, IDC_EMPTY, 0);
			break;
		case VK_F5://刷新
			SendMessage(hDlg, WM_COMMAND, IDC_FLUSH, 0);
			break;
		}
	}
	return (CallNextHookEx(hKeybroadHook, code, wParam, lParam));
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		On_DlgInit(hDlg);
		return (INT_PTR)TRUE;
	case WM_SIZE:
		if (LOWORD(wParam) == SIZE_MINIMIZED) {
			Shell_NotifyIcon(NIM_ADD, &nid);
			ShowWindow(hDlg, SW_HIDE);
		}
		break;
	case WM_TRAYICON:
		if (lParam == WM_LBUTTONDOWN)
		{
			Shell_NotifyIcon(NIM_DELETE, &nid);
			ShowWindow(hDlg, SW_SHOWNORMAL);
			SetForegroundWindow(hDlg);
		}
		break;
	case WM_COMMAND:
		On_DlgCommand(hDlg, wParam, lParam);
		if (LOWORD(wParam) == IDCANCEL)
		{
			if (hDll == NULL) {
				EndDialog(hDlg, LOWORD(wParam));//找不到DLL直接退出程序
			}
			FARPROC pfn = GetProcAddress(hDll, "CancelKeepRunning");
			pfn == NULL ? NULL : pfn();
			EndDialog(hDlg, IDCANCEL);
			return (INT_PTR)TRUE;
		}
		break;
	case WM_CLOSE:
		if (MessageBox(NULL, _T("是否安全退出本程序？"), _T("DebugView"), MB_YESNO) == IDYES) {
			PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
		}
		else {
			return TRUE;
		}
		break;
	case WM_COPYDATA:
		LPCTSTR sz = (LPCTSTR)((COPYDATASTRUCT*)lParam)->lpData;
		AddText(GetDlgItem(hDlg, IDC_LIST1), sz);
		if (ConnectSocket != INVALID_SOCKET) {
			send(ConnectSocket, (char*)sz, ((COPYDATASTRUCT*)lParam)->cbData, 0);//发送信息
		}
		break;
	}
	return (INT_PTR)FALSE;
}

VOID On_DlgInit(HWND hDlg)
{
	::hDlg = hDlg;
	HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hDlg, WM_SETICON, 0, (LPARAM)hIcon);

	SetWindowsHookEx(WH_GETMESSAGE, KeybroadProc, hInst, GetCurrentThreadId());//快捷键

	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.hIcon = hIcon;
	nid.hWnd = hDlg;
	nid.uID = 0;
	nid.uCallbackMessage = WM_TRAYICON;
	StringCchPrintf(nid.szTip, _countof(nid.szTip), _T("单击打开DebugView主程序"));

	EnumProcess(GetDlgItem(hDlg, IDC_COMBO1));
}

VOID On_DlgCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == CBN_SELCHANGE) {
		HWND hCombo = GetDlgItem(hDlg, IDC_COMBO1);
		ComboBox_SetCurSel(hCombo, ComboBox_GetCurSel(hCombo));
		SendMessage(hDlg, WM_COMMAND, IDC_LOAD, 0);
	}
	switch (LOWORD(wParam))
	{
	case IDC_EMPTY://清空列表
		SendMessage(GetDlgItem(hWnd, IDC_LIST1), LB_RESETCONTENT, 0, 0);
		break;
	case IDC_LOAD:
		LoadInstance(hWnd);
		break;
	case IDC_FLUSH:
		EnumProcess(GetDlgItem(hWnd, IDC_COMBO1));
		break;
	case ID_SAVE:
		OnSave(hWnd);
		break;
	case IDC_CONNECT:
		OnConnentTo(hWnd);
		break;
	case IDC_WAITTOCONNECT:
		_beginthreadex(NULL, 0, OnWaitToConnent, hWnd, 0, 0);
		break;

	}
}

VOID AddText(HWND hWnd, PCTSTR pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);
	TCHAR sz[1024 * 10];
	_vstprintf_s(sz, _countof(sz), pszFormat, argList);

	TCHAR szClass[MAX_PATH] = _T("");
	GetClassName(hWnd, szClass, _countof(szClass));//获取窗口类型

	if (lstrcmp(szClass, _T("ListBox")) == 0) {
		SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)sz);
	}
	else if (lstrcmp(szClass, _T("ComboBox")) == 0) {
		SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)sz);
	}
	va_end(argList);
}

DWORD GetListProcessId(HWND hDlg)
{

	TCHAR sz[MAX_PATH];
	ComboBox_GetText(GetDlgItem(hDlg, IDC_COMBO1), sz, _countof(sz));
	HANDLE hTlHelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	TCHAR szFormat[MAX_PATH + 5];
	__try {
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(pe);
		for (BOOL b = Process32First(hTlHelp, &pe); b; b = Process32Next(hTlHelp, &pe)) {
			wsprintf(szFormat, _T("%s[%u]"), pe.szExeFile, pe.th32ProcessID);
			if (lstrcmp(sz, szFormat) == 0)
				return pe.th32ProcessID;
		}
	}
	__finally {
		CloseHandle(hTlHelp);
	}
	return -1;
}

VOID LoadInstance(HWND hDlg)
{
	TCHAR sz[MAX_PATH];
	sz[0] = '\0';
	dwProcessId = GetListProcessId(hDlg);


	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, MyCreateRemoteThread, hDlg, 0, 0);
	if (hThread == NULL) {
		AddText(GetDlgItem(hDlg, IDC_LIST1), _T("错误: %u"), GetLastError());
		return;
	}
	//EnableWindow(GetDlgItem(hDlg,IDC_PROCESS_ID),FALSE);
	//EnableWindow(GetDlgItem(hDlg,IDC_LOAD),FALSE);
}


VOID EnumProcess(HWND hWnd)
{
	/*枚举进程*/
	SendMessage(hWnd, CB_RESETCONTENT, 0, 0);
	AddText(hWnd, _T("*******以下为正在运行的所有进程*********"));
	HANDLE hTlHelp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hTlHelp == NULL) {
		return;
	}
	PROCESSENTRY32 pes;
	pes.dwSize = sizeof(PROCESSENTRY32);
	BOOL bmore = Process32First(hTlHelp, &pes);
	for (BOOL bmore = Process32First(hTlHelp, &pes); bmore; bmore = Process32Next(hTlHelp, &pes)) {
		AddText(hWnd, _T("%s[%u]"), pes.szExeFile, pes.th32ProcessID);
	}
	CloseHandle(hTlHelp);
}

UINT CALLBACK MyCreateRemoteThread(LPVOID pParam)
{
	HWND hDlg = (HWND)pParam;
	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
		FALSE, dwProcessId);
	if (hProcess == NULL) {
		AddText(GetDlgItem(hDlg, IDC_LIST1), _T("错误: %u"), GetLastError());
		return FALSE;
	}
	LPTHREAD_START_ROUTINE pfn = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("kernel32")), "LoadLibraryW");
	/*获取DLL绝对路径*/
	TCHAR sz[MAX_PATH] = _T("");
	GetModuleFileName(NULL, sz, _countof(sz));
	int cbSize = _countof(sz);
	for (LPTSTR p = &sz[cbSize]; *p != TEXT('\\'); p--, cbSize--);
	sz[cbSize + 1] = '\0';//截断
	StringCchPrintf(sz, _countof(sz), _T("%s%s"), sz, _T("DebugViewDLL.dll"));
	/*获取DLL绝对路径*/
	PVOID pData = VirtualAllocEx(hProcess, NULL, _countof(sz)*sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, pData, sz, _countof(sz)*sizeof(TCHAR), NULL);
	HANDLE newThread = CreateRemoteThread(hProcess, NULL, 0, pfn, pData, 0, NULL);
	if (newThread == NULL) {
		AddText(GetDlgItem(hDlg, IDC_LIST1), _T("错误: %u"), GetLastError());
	}
	WaitForSingleObject(newThread, INFINITE);//等待载入完成
	VirtualFreeEx(hProcess, pData, 0, MEM_RELEASE);
	return TRUE;
}

VOID OnSave(HWND hWnd)
{
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
	ofn.hwndOwner = hWnd;
	GetSaveFileName(&ofn);

	HANDLE hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	/*HANDLE hFileMap=CreateFileMapping(hFile,0,PAGE_READWRITE,0,FILESIZE,0);
	LPVOID pData=MapViewOfFile(hFileMap,FILE_MAP_WRITE,0,0,0);

	StringCbCatA((LPSTR)pData,FILESIZE,"******************  DebugView生成报告  ******************\r\n");

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
	HWND hListBox = GetDlgItem(hWnd, IDC_LIST1);
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

VOID OnConnentTo(HWND hWndParent/*父窗口*/)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_CONNECTTO), hWndParent, ConnentToDlg);

	addrinfo hints, *result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int iResult = getaddrinfo(szIPAddress, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("获取地址信息失败：%d"), iResult);
		return;
	}
	delete[]szIPAddress;

	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("创建套接字失败：%d"), WSAGetLastError());
		return;
	}
	//连接...
	iResult = connect(ConnectSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("无法连接到服务器：%d"), WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	//发送测试数据
	iResult = send(ConnectSocket, (char*)_T(TEST_STRING), sizeof(_T(TEST_STRING)), 0);
	AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("连接成功"));

	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		return;
	}
	
}

INT_PTR CALLBACK ConnentToDlg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND) {
		if (LOWORD(wParam) == IDOK){
			szIPAddress = new CHAR[50];
			GetWindowTextA(GetDlgItem(hWnd, IDC_ROMOTEIPADDRESS), szIPAddress, 20);
			EndDialog(hWnd, 0);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hWnd, 0);
			return TRUE;
		}
	}
	return FALSE;
}

UINT CALLBACK OnWaitToConnent(LPVOID hWnd)
{
	AddText(GetDlgItem((HWND)hWnd, IDC_LIST1), _T("等待连接中..."));
	addrinfo hints, *result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	TCHAR sz[1024];
	int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	ServiceSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	int s_bind = bind(ServiceSocket, result->ai_addr, result->ai_addrlen);

	freeaddrinfo(result);

	int s_listen = listen(ServiceSocket, 5);//设置监听状态
	if (s_listen == SOCKET_ERROR) {
		AddText(GetDlgItem((HWND)hWnd, IDC_LIST1), _T("监听失败！"));
	}
	SOCKET sockSend = INVALID_SOCKET;

	sockaddr addr;
	int addrlen = sizeof(addr);
	sockSend = accept(ServiceSocket, &addr, &addrlen);//接受连接

	while (TRUE)
	{
		ZeroMemory(sz, sizeof(sz));
		recv(sockSend, (char*)sz, sizeof(sz), 0);
		AddText(GetDlgItem((HWND)hWnd, IDC_LIST1), sz);
	}
	return TRUE;
}