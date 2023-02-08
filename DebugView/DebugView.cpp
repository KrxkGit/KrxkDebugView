// DebugView.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "DebugView.h"
#define MAX_LOADSTRING 100
#define DEFAULT_PORT "32170"
#define TEST_STRING "�������"

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szClassName[] = _T("DebugView");
NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
DWORD dwProcessId;//�����صĽ��̾��ID
//����ȫ�ֱ���
LPSTR szIPAddress;
SOCKET ConnectSocket = INVALID_SOCKET;
SOCKET ServiceSocket = INVALID_SOCKET;
//DLLģ��
HMODULE hDll = NULL;
HHOOK hKeybroadHook = NULL;//��ݼ�
HWND hDlg = NULL;

// �˴���ģ���а����ĺ�����ǰ������:


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: �ڴ˷��ô��롣
	hInst = hInstance;
	__try {
		hDll = LoadLibrary(_T("Running"));
		if (hDll == NULL) {
			MessageBox(NULL, _T("�Ҳ���ָ��DLL\n���ֹ��ܲ����ã�"), _T("Warning"), MB_ICONERROR);
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
	//�����¼�
	SetEvent(hWaitFreeDll);
	CloseHandle(hWaitFreeDll);
	return GetLastError();
}

VOID RegisterDlgClass(LPTSTR szClassName)
{
	//ע���Զ��崰����
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
		case VK_F1: //����
			SendMessage(hDlg, WM_COMMAND, ID_SAVE, 0);
			break;
		case VK_F2://����
			SendMessage(hDlg, WM_COMMAND, IDC_LOAD, 0);
			break;
		case VK_F3://���
			SendMessage(hDlg, WM_COMMAND, IDC_EMPTY, 0);
			break;
		case VK_F5://ˢ��
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
				EndDialog(hDlg, LOWORD(wParam));//�Ҳ���DLLֱ���˳�����
			}
			FARPROC pfn = GetProcAddress(hDll, "CancelKeepRunning");
			pfn == NULL ? NULL : pfn();
			EndDialog(hDlg, IDCANCEL);
			return (INT_PTR)TRUE;
		}
		break;
	case WM_CLOSE:
		if (MessageBox(NULL, _T("�Ƿ�ȫ�˳�������"), _T("DebugView"), MB_YESNO) == IDYES) {
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
			send(ConnectSocket, (char*)sz, ((COPYDATASTRUCT*)lParam)->cbData, 0);//������Ϣ
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

	SetWindowsHookEx(WH_GETMESSAGE, KeybroadProc, hInst, GetCurrentThreadId());//��ݼ�

	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.hIcon = hIcon;
	nid.hWnd = hDlg;
	nid.uID = 0;
	nid.uCallbackMessage = WM_TRAYICON;
	StringCchPrintf(nid.szTip, _countof(nid.szTip), _T("������DebugView������"));

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
	case IDC_EMPTY://����б�
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
	GetClassName(hWnd, szClass, _countof(szClass));//��ȡ��������

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
		AddText(GetDlgItem(hDlg, IDC_LIST1), _T("����: %u"), GetLastError());
		return;
	}
	//EnableWindow(GetDlgItem(hDlg,IDC_PROCESS_ID),FALSE);
	//EnableWindow(GetDlgItem(hDlg,IDC_LOAD),FALSE);
}


VOID EnumProcess(HWND hWnd)
{
	/*ö�ٽ���*/
	SendMessage(hWnd, CB_RESETCONTENT, 0, 0);
	AddText(hWnd, _T("*******����Ϊ�������е����н���*********"));
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
		AddText(GetDlgItem(hDlg, IDC_LIST1), _T("����: %u"), GetLastError());
		return FALSE;
	}
	LPTHREAD_START_ROUTINE pfn = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("kernel32")), "LoadLibraryW");
	/*��ȡDLL����·��*/
	TCHAR sz[MAX_PATH] = _T("");
	GetModuleFileName(NULL, sz, _countof(sz));
	int cbSize = _countof(sz);
	for (LPTSTR p = &sz[cbSize]; *p != TEXT('\\'); p--, cbSize--);
	sz[cbSize + 1] = '\0';//�ض�
	StringCchPrintf(sz, _countof(sz), _T("%s%s"), sz, _T("DebugViewDLL.dll"));
	/*��ȡDLL����·��*/
	PVOID pData = VirtualAllocEx(hProcess, NULL, _countof(sz)*sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, pData, sz, _countof(sz)*sizeof(TCHAR), NULL);
	HANDLE newThread = CreateRemoteThread(hProcess, NULL, 0, pfn, pData, 0, NULL);
	if (newThread == NULL) {
		AddText(GetDlgItem(hDlg, IDC_LIST1), _T("����: %u"), GetLastError());
	}
	WaitForSingleObject(newThread, INFINITE);//�ȴ��������
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
	ofn.lpstrFilter = _T("�ı��ļ���*.txt��\0*.txt\0\0");
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = _T("*.txt");
	ofn.hInstance = hInst;
	ofn.hwndOwner = hWnd;
	GetSaveFileName(&ofn);

	HANDLE hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	/*HANDLE hFileMap=CreateFileMapping(hFile,0,PAGE_READWRITE,0,FILESIZE,0);
	LPVOID pData=MapViewOfFile(hFileMap,FILE_MAP_WRITE,0,0,0);

	StringCbCatA((LPSTR)pData,FILESIZE,"******************  DebugView���ɱ���  ******************\r\n");

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

VOID OnConnentTo(HWND hWndParent/*������*/)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_CONNECTTO), hWndParent, ConnentToDlg);

	addrinfo hints, *result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int iResult = getaddrinfo(szIPAddress, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("��ȡ��ַ��Ϣʧ�ܣ�%d"), iResult);
		return;
	}
	delete[]szIPAddress;

	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("�����׽���ʧ�ܣ�%d"), WSAGetLastError());
		return;
	}
	//����...
	iResult = connect(ConnectSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("�޷����ӵ���������%d"), WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	//���Ͳ�������
	iResult = send(ConnectSocket, (char*)_T(TEST_STRING), sizeof(_T(TEST_STRING)), 0);
	AddText(GetDlgItem(hWndParent, IDC_LIST1), _T("���ӳɹ�"));

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
	AddText(GetDlgItem((HWND)hWnd, IDC_LIST1), _T("�ȴ�������..."));
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

	int s_listen = listen(ServiceSocket, 5);//���ü���״̬
	if (s_listen == SOCKET_ERROR) {
		AddText(GetDlgItem((HWND)hWnd, IDC_LIST1), _T("����ʧ�ܣ�"));
	}
	SOCKET sockSend = INVALID_SOCKET;

	sockaddr addr;
	int addrlen = sizeof(addr);
	sockSend = accept(ServiceSocket, &addr, &addrlen);//��������

	while (TRUE)
	{
		ZeroMemory(sz, sizeof(sz));
		recv(sockSend, (char*)sz, sizeof(sz), 0);
		AddText(GetDlgItem((HWND)hWnd, IDC_LIST1), sz);
	}
	return TRUE;
}